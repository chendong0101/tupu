/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#include "http_service.h"

#include <string>
#include <iostream>

#include "utils/string_utils.h"
#include "log/tupu_log.h"

using namespace std;

void defaultHttpFunc(WFHttpTask* task, map<string, string> params) {
    task->get_resp()->set_http_version("HTTP/1.1");
    task->get_resp()->set_status_code("404");
    task->get_resp()->set_reason_phrase("OK");

    task->get_resp()->add_header_pair("Content-Type", "text/json");
    task->get_resp()->add_header_pair("Server", "Sogou tupu sql server");
    task->get_resp()->add_header_pair("Access-Control-Allow-Origin", "*");
}

HttpFunction HttpService::getFunction(string path, string method) {
    string key = path + method;
    if (m_funcMap.find(key) != m_funcMap.end()) {
        return m_funcMap[key];
    }
    LOG_ERROR("unknown request \t%s\t%s", path.c_str(), method.c_str());
    return defaultHttpFunc;
}

void HttpService::registerCtr(string path, string method, HttpFunction httpFunc) {
    string key = path + method;
    if (m_funcMap.find(key) != m_funcMap.end()) {
        LOG_ERROR("duplicate register: %s", key.c_str());
    }
    m_funcMap[key] = httpFunc;
}

map<string, string> processParams(string paramStr) {
    vector<string> paramSegments;
    utils::split(paramStr, paramSegments, "&");
    map<string, string> params;
    for (string param : paramSegments) {
       size_t pos = param.find_first_of("=");
       if (pos != string::npos) {
           string key = param.substr(0, pos);
           string value = param.substr(pos + 1);
           params[key] = utils::urlDecode(value);
       }
    }
    return params;
}

map<string, string> processGetRequest(protocol::HttpRequest *req) {
    string url = req->get_request_uri();
    string paramStr = url.substr(url.find_first_of("?")+1);
    return processParams(paramStr);
}

map<string, string> processPostRequest(protocol::HttpRequest *req) {
    const void *body;
    size_t len;
    req->get_parsed_body(&body, &len);
    string request = static_cast<const char *>(body);
    map<string, string> params = processParams(request);
    string sql_utf8;
    utils::gbk2utf8(params["sql"], sql_utf8);
    params["sql"] = sql_utf8;
    string query_utf8;
    utils::gbk2utf8(params["query"], query_utf8);
    params["query"] = query_utf8;

    return params;
}

string parsePath(string url) {
    size_t pos = url.find_first_of("?");
    if (pos == string::npos) {
        return url;
    }
    return url.substr(0, pos);
}

void HttpService::requestProcess(WFHttpTask *task) {
    protocol::HttpRequest *req = task->get_req();

    string path = parsePath(req->get_request_uri());
    map<string, string> params;
    string method = req->get_method();
    if (method == "GET") {
        params = processGetRequest(req);
    } else if (method == "POST") {
        params = processPostRequest(req);
    }

    getFunction(path, method)(task, params);
}