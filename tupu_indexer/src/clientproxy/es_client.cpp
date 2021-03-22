/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#include "es_client.h"

#include <string>
#include <glog/logging.h>
#include <gflags/gflags.h>
#include <workflow/HttpUtil.h>

DECLARE_string(es_url);

namespace es
{
	void index_delete(const string &id)
	{
		std::string url = FLAGS_es_url;
		url += id;

		WFHttpTask *task = WFTaskFactory::create_http_task(url, REDIRECT_MAX, RETRY_MAX,
														   es_callback);
		protocol::HttpRequest *req = task->get_req();
		req->add_header_pair("Accept", "*/*");
		req->add_header_pair("Authorization", "Basic c2podGdyb3VwOnNqaHRncm91cA==");
		req->add_header_pair("Connection", "close");
		req->add_header_pair("Content-Type", "application/json");
		req->set_method("DELETE");
		task->start();
	}

	void index_put(const string &id, const string &body, http_callback_t callback)
	{
		std::string url = FLAGS_es_url;
		url += id;

		WFHttpTask *task = WFTaskFactory::create_http_task(url, REDIRECT_MAX, RETRY_MAX, callback);
		protocol::HttpRequest *req = task->get_req();
		req->add_header_pair("Accept", "*/*");
		req->add_header_pair("Authorization", "Basic c2podGdyb3VwOnNqaHRncm91cA==");
		req->add_header_pair("Connection", "close");
		req->add_header_pair("Content-Type", "application/json");
		req->set_method("PUT");
		req->append_output_body(body);
		task->start();
	}

	void es_callback(WFHttpTask *task)
	{
		protocol::HttpRequest *req = task->get_req();
		protocol::HttpResponse *resp = task->get_resp();
		int state = task->get_state();
		int error = task->get_error();

		switch (state)
		{
		case WFT_STATE_SYS_ERROR:
			LOG(ERROR) << "system error: " << strerror(error);
			break;
		case WFT_STATE_DNS_ERROR:
			LOG(ERROR) <<  "DNS error: " << gai_strerror(error);
			break;
		case WFT_STATE_SSL_ERROR:
			LOG(ERROR) << "SSL error: " << error;
			break;
		case WFT_STATE_TASK_ERROR:
			LOG(ERROR) <<  "Task error: " << error;
			break;
		case WFT_STATE_SUCCESS:
			break;
		}

		if (state != WFT_STATE_SUCCESS)
		{
			LOG(ERROR) << "es failed callback: " << state;
			return;
		}
	}
} // namespace es