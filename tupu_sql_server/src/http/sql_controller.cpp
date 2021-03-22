/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#include "sql_controller.h"

#include <string>
#include "message/entity.pb.h"
#include "model/query.h"
#include "model/task.h"
#include "model/task_context.h"
#include "service/sql_scheduler.h"
#include "utils/entity_utils.h"
#include "utils/string_utils.h"
#include "log/tupu_log.h"

void responseCallback(SeriesWork *series) {
    TaskContext *context = (TaskContext *)series->get_context();
    Task *task = context->currentTask;

    if (context->encoding != "utf-8" && context->encoding != "UTF-8")
    {
        for (auto entity_id : task->entity_ids)
        {
            if (task->entity_map.find(entity_id) != task->entity_map.end())
            {
                proto::Entity &entity = task->entity_map[entity_id];
                task->entity_map[entity_id] = utils::encodeEntity(entity, context->encoding);
            }
        }
    }

    string body;
    if (context->fatResult) {
        proto::FatSearchResult sr;
        for (auto entity_id : task->entity_ids)
        {
            if (task->entity_map.find(entity_id) != task->entity_map.end())
            {
                proto::Entity &content = task->entity_map[entity_id];
                proto::Entity *newEntity = sr.add_entities();
                newEntity->MergeFrom(content);
            }
        }
        if (context->resFormat.compare("json") == 0 && context->encoding.compare("utf-8") == 0)
        {
            body = utils::proto2Json(sr);
        }
        else
        {
            body = sr.SerializeAsString();
        }
    } else {
        proto::SearchResult sr;
        for (auto entity_id : task->entity_ids)
        {
            if (task->entity_map.find(entity_id) != task->entity_map.end())
            {
                proto::EntityContent content = utils::entity2EntityContent(task->entity_map[entity_id]);
                proto::EntityContent *newContent = sr.add_entity_contents();
                newContent->MergeFrom(content);
            }
        }
        if (context->resFormat.compare("json") == 0 && context->encoding.compare("utf-8") == 0)
        {
            body = utils::proto2Json(sr);
        }
        else
        {
            body = sr.SerializeAsString();
        }
    }

    static_cast<WFHttpTask*>(context->proxyTask)->get_resp()->append_output_body(body.c_str(), body.size());
    static_cast<WFHttpTask*>(context->proxyTask)->get_resp()->set_http_version("HTTP/1.1");
    static_cast<WFHttpTask*>(context->proxyTask)->get_resp()->set_status_code("200");
    static_cast<WFHttpTask*>(context->proxyTask)->get_resp()->set_reason_phrase("OK");

    static_cast<WFHttpTask*>(context->proxyTask)->get_resp()->add_header_pair("Content-Type", "text/json");
    static_cast<WFHttpTask*>(context->proxyTask)->get_resp()->add_header_pair("Server", "Sogou tupu sql server");
    static_cast<WFHttpTask*>(context->proxyTask)->get_resp()->add_header_pair("Access-Control-Allow-Origin", "*");

    LOG_INFO("%s", context->toString().c_str());
}

void SQLCtr::searchBySQL(WFHttpTask *task, map<string, string> params)
{

    protocol::HttpRequest *req = task->get_req();
    protocol::HttpResponse *resp = task->get_resp();
    SeriesWork *series = series_of(task);

    if (params.find("sql") == params.end()) {
        LOG_ERROR("No sql in params");
        return;
    }
    string sql = params["sql"];

    TaskContext *context = new TaskContext();
	context->proxyTask = task;
    context->sql = sql;
    context->queryStr = params["query"];
    context->resFormat = params["resformat"];
    context->forceQuery = params["forceQuery"].compare("1") == 0;
    context->encoding = params["encoding"];
    context->forwardSearch = params["forwardSearch"].compare("1") == 0;
    context->fatResult = params["fatResult"].compare("1") == 0;

    series->set_context(context);
    series->set_callback([](const SeriesWork *series) {
        delete (TaskContext *)series->get_context();
    });

    SqlScheduler::searchBySql(series, responseCallback);
}
