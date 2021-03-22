/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#include "simple_sql_service.h"

#include <chrono>
#include <map>
#include <algorithm>
#include <atomic>
#include <vector>
#include <functional>
#include "thirdparty/json/nlohmann/json.hpp"

#include "doc_service.h"
#include "simple_sql_analyzer.h"
#include "log/tupu_log.h"
#include "model/task_context.h"
#include "clientproxy/es_client.h"
#include "utils/lru_cache.h"
#include "utils/string_utils.h"

using namespace std::placeholders;
using namespace std;

static atomic_ulong total(0);
static atomic_ulong hit(0);

void sqlPostProcess(SeriesWork *series, function<void(SeriesWork *)> callback)
{
    TaskContext *context = (TaskContext *)series->get_context();
    Task *task = context->currentTask;
    set<string> select = task->queryRequest.sql_select;

    // deal with "select" subsentence
    if (select.size() != 0)
    {
        for (auto iter = task->entity_map.begin(); iter != task->entity_map.end();)
        {
            proto::Entity &content = iter->second;
            set<string> keys;
            for_each(content.triads().begin(), content.triads().end(), [&](const proto::Triad &triad) {
                string sfield = triad.predicate();
                if (sfield.length() > 8 && sfield.find("webfact_") == 0)
                {
                    sfield = sfield.substr(8);
                }
                else if (sfield.length() > 7 && sfield.find("schema_") == 0)
                {
                    sfield = sfield.substr(7);
                }
                keys.insert(sfield);
            });
            if (includes(keys.begin(), keys.end(), select.begin(), select.end()))
            {
                iter++;
            }
            else
            {
                iter = task->entity_map.erase(iter);
            }
        }
    }

    // deal with "limit" subsentence
    map<string, proto::Entity> newEntityMap;
    if (task->entity_map.size() > task->queryRequest.limit)
    {
        int i = 1;
        for (string entityId : task->entity_ids)
        {
            if (task->entity_map.find(entityId) != task->entity_map.end())
            {
                newEntityMap[entityId] = task->entity_map[entityId];
                if (i++ >= task->queryRequest.limit)
                {
                    break;
                }
            }
        }
        task->entity_map = newEntityMap;
    }
    // deal with "index" field
    vector<string> orders;
    utils::split(task->queryRequest.sql_order, orders, "@");
    if (orders.size() == 2)
    {
        string indexField = orders[1];
        if (!indexField.empty())
        {
            for (auto item : task->entity_map)
            {
                string index;
                for_each(item.second.triads().begin(), item.second.triads().end(), 
                                    [&index, &indexField](const proto::Triad &triad) {
                    if (triad.predicate() == indexField)
                    {
                        index = triad.value();
                    }
                });
                item.second.set_index(index);
            }
        }
    }
    callback(series);
}

void dealWithESResponse(SeriesWork *series, const string &esResp, function<void(SeriesWork *)> callback)
{
    TaskContext *context = (TaskContext *)series->get_context();
    context->es_end = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

    vector<string> entityIds;
    json j;
    try
    {
        j = json::parse(esResp.c_str());
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("exception: %s", esResp.c_str());
        DocService::getDocs(series, entityIds, bind(&sqlPostProcess, _1, callback));
    }
    json hits = j["hits"]["hits"];
    for (int i = 0; i < hits.size(); i++)
    {
        std::string entityId = hits[i]["_id"].get<std::string>();
        entityIds.push_back(entityId);
    }
    DocService::getDocs(series, entityIds, bind(&sqlPostProcess, _1, callback));
}

void esCallback(WFHttpTask *task, function<void(SeriesWork *)> callback)
{
    SeriesWork *series = series_of(task);
    TaskContext *context = (TaskContext *)series->get_context();
    protocol::HttpRequest *req = task->get_req();
    protocol::HttpResponse *resp = task->get_resp();
    int state = task->get_state();
    int error = task->get_error();

    if (state != WFT_STATE_SUCCESS)
    {
        LOG_ERROR("ElasticSearch Error: state %d, error: %d", state, error);
        callback(series);
        return;
    }

    const void *body;
    size_t body_len;
    resp->get_parsed_body(&body, &body_len);
    string esResp((char *)body);

    utils::cache::lru_cache<termID64_t, string> *es_cache = utils::cache::lru_cache<termID64_t, string>::getESCache();
    termID64_t key = utils::hash64(context->sql);
    es_cache->insert(key, esResp);

    dealWithESResponse(series, esResp, callback);
}

bool isIDSearch(const string &where) {
    if (where.find("ID =") == string::npos) {
        return false;
    }
    if (where.find(" AND ") != string::npos || where.find(" OR ") != string::npos) {
        return false;
    }

    return true;
}

void SqlService::searchBySql(SeriesWork *series, string sql, function<void(SeriesWork *)> callback)
{
    TaskContext *context = (TaskContext *)series->get_context();
    Task *task = context->currentTask;

    task->queryRequest = sql_analyzer::sqlSplit(sql);

    if (isIDSearch(task->queryRequest.sql_where)) {
        string where = task->queryRequest.sql_where;
        utils::trim(where);
        string ids = where.substr(4);
        utils::trim(ids);
        if (ids[0] == '(' && ids[ids.length() - 1] == ')') {
            ids = ids.substr(1, ids.length() - 2);
        }
        replace(ids.begin(), ids.end(), '"', ' ');
        vector<string> entityIds;
        utils::split(ids, entityIds, ",");
        for (auto &entityId : entityIds) {
            utils::trim(entityId);
        }

        context->es_end = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
        return DocService::getDocs(series, entityIds, bind(&sqlPostProcess, _1, callback));
    }

    utils::cache::lru_cache<termID64_t, string> *es_cache = utils::cache::lru_cache<termID64_t, string>::getESCache();
    termID64_t key = utils::hash64(sql);
    string esResponseStr;
    bool cacheHit = es_cache->get(key, esResponseStr);
    total++;
    if (cacheHit && !context->forceQuery)
    {
        hit++;
        dealWithESResponse(series, esResponseStr, callback);
        if (hit % 10000 == 0)
        {
            LOG_INFO("es hit rate: %f, cache_size: %d", hit * 1.0 / total, es_cache->size());
            total = 0L;
            hit = 0L;
        }
    }
    else
    {
        json queryBody = sql_analyzer::buildESQuery(task->queryRequest);
        LOG_INFO(queryBody.dump().c_str());
        return es::search(series, queryBody, bind(&esCallback, _1, callback));
    }
}