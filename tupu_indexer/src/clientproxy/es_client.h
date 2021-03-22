/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef ES_CLIENT_H
#define ES_CLIENT_H

#include <string>
#include <workflow/WFTaskFactory.h>

using namespace std;

#define REDIRECT_MAX   0 
#define RETRY_MAX      1 

namespace es
{
    void es_callback(WFHttpTask *task);

    void index_delete(const string &id);

    void index_put(const string &id, const string &body, http_callback_t callback = es_callback);

} // namespace es

#endif