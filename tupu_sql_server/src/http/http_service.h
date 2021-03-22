/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _HTTP_SERIVICE_H
#define _HTTP_SERIVICE_H

#include <gflags/gflags.h>
#include "workflow/WFServer.h"
#include "workflow/WFFacilities.h"
#include "workflow/WFHttpServer.h"

#include "doc_controller.h"
#include "sql_controller.h"
#include "log/tupu_log.h"

using namespace std::placeholders;

DECLARE_int32(server_port);

static WFFacilities::WaitGroup wait_group(1);

typedef function<void (WFHttpTask *, map<string, string>)> HttpFunction;

void defaultHttpFunc(WFHttpTask* task, map<string, string> params);

class HttpService {
    private:
        map<string, HttpFunction> m_funcMap;

    private:
        void registerCtr(string path, string method, HttpFunction httpFunction);
        HttpFunction getFunction(string path, string method);
        void requestProcess(WFHttpTask *task);

    public:
        HttpService() {
            registerCtr("/tupu/doc",            "GET",      DocCtr::getDocs);
            registerCtr("/tupu/sql",            "GET",      SQLCtr::searchBySQL);
            registerCtr("/",                    "POST",     SQLCtr::searchBySQL);
            registerCtr("/sugg/ajaj_json.jsp",  "POST",     SQLCtr::searchBySQL);
        };
        virtual ~HttpService() = default;

        void start() {
            struct WFGlobalSettings settings = GLOBAL_SETTINGS_DEFAULT;
            settings.endpoint_params.max_connections = 8192;
            settings.endpoint_params.connect_timeout = 5 * 1000;
            settings.endpoint_params.response_timeout = 5 * 1000;
            settings.poller_threads = 16;
            settings.handler_threads = 16;
            WORKFLOW_library_init(&settings);

            struct WFServerParams params = HTTP_SERVER_PARAMS_DEFAULT;
            params.max_connections = 8192;
            params.peer_response_timeout = -1;
            params.keep_alive_timeout = 3600 * 1000;
            params.request_size_limit = 512 * 1024 * 1024;

            WFHttpServer server(&params, [this](WFHttpTask *server_task) {
                this->requestProcess(server_task);
            });

            if (server.start(FLAGS_server_port) == 0)
            {
                LOG_INFO("HTTP Server has started on %d", FLAGS_server_port);
                wait_group.wait();
                server.stop();
                LOG_ERROR("HTTP Server has stopped from %d", FLAGS_server_port);
            }
            else
            {
                LOG_ERROR("Cannot start server");
                exit(1);
            }
        }

        void terminate() {
            wait_group.done();
        }
};

#endif //_HTTP_SERIVICE_H 