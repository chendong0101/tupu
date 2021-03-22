/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#include "doc_service.h"

#include <atomic>
#include <functional>
#include <gflags/gflags.h>

#include "clientproxy/redis_client.h"
#include "log/tupu_log.h"
#include "model/task_context.h"
#include "model/task.h"
#include "utils/lru_cache.h"

using namespace std::placeholders;

DECLARE_int32(redis_doc_db);
static atomic_ulong total(0);
static atomic_ulong hit(0);

void docProcessCallback(WFRedisTask *task, function<void (SeriesWork *)> callback);

void DocService::getDocs(SeriesWork *series, vector<string> docIds, function<void (SeriesWork *)> callback) {
    TaskContext *context = (TaskContext *)series->get_context();
    Task *task = context->currentTask;
    task->entity_ids = docIds;

    // find from cache first
    utils::cache::lru_cache<string, proto::Entity> *redis_cache =
        utils::cache::lru_cache<string, proto::Entity>::getRedisCache();
    vector<string> missed_ids;
    for (auto docId : docIds)
    {
        proto::Entity entity; 
        bool cacheHit = redis_cache->get(docId, entity);
        total++;
        if (cacheHit && !context->forceQuery)
        {
            hit++;
            task->entity_map[docId] = entity;
            if (hit % 10000 == 0)
            {
                LOG_INFO("redis cache hit rate: %f, cache_size: %d", hit * 1.0 / total, redis_cache->size());
                hit = 0L;
                total = 0L;
            }
        } else {
            missed_ids.push_back(docId);
        }
    }

    if (missed_ids.size() > 0)
    {
        redis::mget(series, missed_ids, FLAGS_redis_doc_db, bind(&docProcessCallback, _1, callback));
    } else {
        callback(series);
    }
}

void docProcessCallback(WFRedisTask *task, function<void (SeriesWork *)> callback) {
    SeriesWork *series = series_of(task);
    TaskContext *context =
        (TaskContext *)series->get_context();
    protocol::RedisRequest *req = task->get_req();
    protocol::RedisResponse *resp = task->get_resp();
    int state = task->get_state();
    int error = task->get_error();
    protocol::RedisValue results;

    if (state != WFT_STATE_SUCCESS) {
        LOG_ERROR("Redis Error: state %d, error: %d", state, error);
        callback(series);
    } else {
        resp->get_result(results);
    }

    if (results.is_array())
    {
        for (int i = 0; i < results.arr_size(); i++)
        {
            protocol::RedisValue val = results.arr_at(i);
            if (val.is_nil())
            {
                continue;
            }
            if (val.is_string())
            {
                proto::Entity entity;
                if (entity.ParseFromString(val.string_value()))
                {
                    context->currentTask->entity_map[entity.kgc_id()] = entity;
                    // put into cache
                    utils::cache::lru_cache<string, proto::Entity> *redis_cache =
                        utils::cache::lru_cache<string, proto::Entity>::getRedisCache();
                    redis_cache->insert(entity.kgc_id(), entity);
                } else {
                    LOG_ERROR("parse protobuf failed");
                }
            }
        }
        callback(series);
    } else {
        LOG_ERROR("Redis Error: Not a string value. type: %d", results.get_type());
    }
}