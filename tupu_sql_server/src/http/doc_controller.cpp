/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#include "doc_controller.h"

#include <string>
#include "model/task.h"
#include "model/task_context.h"
#include "service/doc_service.h"
#include "utils/entity_utils.h"

using namespace nlohmann;

void responseDocs(SeriesWork *series) {
    TaskContext *context = (TaskContext *)series->get_context();
    Task *task = context->currentTask;

    proto::FatSearchResult sr;

    for (auto entity_id : task->entity_ids) {
        if (task->entity_map.find(entity_id) != task->entity_map.end()) {
            proto::Entity *add_content = sr.add_entities();
            add_content->CopyFrom(task->entity_map[entity_id]);
        }
    }

    string body;
    if (context->resFormat.compare("json") == 0) {
        body = utils::proto2Json(sr);
    } else {
        body = sr.SerializeAsString();
    }
    static_cast<WFHttpTask*>(context->proxyTask)->get_resp()->append_output_body(body.c_str(), body.size());
    static_cast<WFHttpTask*>(context->proxyTask)->get_resp()->set_http_version("HTTP/1.1");
    static_cast<WFHttpTask*>(context->proxyTask)->get_resp()->set_status_code("200");
    static_cast<WFHttpTask*>(context->proxyTask)->get_resp()->set_reason_phrase("OK");
    static_cast<WFHttpTask*>(context->proxyTask)->get_resp()->add_header_pair("Content-Type", "text/json");
    static_cast<WFHttpTask*>(context->proxyTask)->get_resp()->add_header_pair("Server", "Sogou tupu sql server");
}

void DocCtr::getDocs(WFHttpTask *task, map<string, string> params)
{
    string docId = params["docId"];
    SeriesWork *series = series_of(task);
    TaskContext *context = new TaskContext();
    context->currentTask = new Task();
    context->proxyTask = task;
    context->resFormat = params["resformat"];
    context->forceQuery = params["forceQuery"].compare("1") == 0;
    context->encoding = params["encoding"];
    series->set_context(context);
    series->set_callback([](const SeriesWork *series) {
        delete (TaskContext *)series->get_context();
    });

    DocService::getDocs(series, {docId}, responseDocs);
}
