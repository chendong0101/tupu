/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#include <string>
#include <iostream>
#include <glog/logging.h>
#include <map>
#include "workflow/HttpMessage.h"
#include "workflow/HttpUtil.h"
#include "workflow/WFServer.h"
#include "workflow/WFHttpServer.h"
#include "workflow/WFFacilities.h"

#include "http_service.h"

using namespace::std;

static WFFacilities::WaitGroup wait_group(10);

static void sig_handler(int signo)
{
	wait_group.done();
}

HttpService::HttpService() {

    struct WFServerParams params = HTTP_SERVER_PARAMS_DEFAULT;
    params.max_connections = 8192;
    params.keep_alive_timeout = 3600 * 1000;
    params.request_size_limit = 512 * 1024 * 1024;

    WFHttpServer server(&params, [&](WFHttpTask *server_task) {
        httpProcess(server_task);
    });
    if (server.start(8888) == 0)
	{
		wait_group.wait();
		server.stop();
	}
	else
	{
		perror("Cannot start server");
		exit(1);
	}
}

void HttpService::httpProcess(WFHttpTask *server_task) {
    protocol::HttpRequest *req = server_task->get_req();
    protocol::HttpResponse *resp = server_task->get_resp();
    SeriesWork *series = series_of(server_task);
    protocol::HttpHeaderCursor cursor(req);
    map<string, string> headers;
    string name, value;
    while (cursor.next(name, value))
	{
        headers[name] = value;
        LOG(INFO) << name << ":" << value;
	}

    string url = req->get_request_uri();
}
