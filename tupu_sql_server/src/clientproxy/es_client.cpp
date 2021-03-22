/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#include <string>
#include <gflags/gflags.h>
#include "workflow/HttpUtil.h"
#include "workflow/WFTaskFactory.h"
#include "workflow/WFFacilities.h"

#include "es_client.h"

DECLARE_string(es_url);

namespace es
{

	void search(SeriesWork *series, json queryJson, http_callback_t callback)
	{
		std::string url = FLAGS_es_url;

		WFHttpTask *task = WFTaskFactory::create_http_task(url, REDIRECT_MAX, RETRY_MAX, callback);
		protocol::HttpRequest *req = task->get_req();
		req->add_header_pair("Accept", "*/*");
		req->add_header_pair("Authorization", "Basic c2podGdyb3VwOnNqaHRncm91cA==");
		req->add_header_pair("Connection", "close");
		req->add_header_pair("Content-Type", "application/json");
		req->set_method("POST");
		req->append_output_body(queryJson.dump());
		series->push_back(task);
	}
} // namespace es