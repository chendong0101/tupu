/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _REDIS_CLIENT_
#define _REDIS_CLIENT_

#include <vector>
#include "workflow/RedisMessage.h"
#include "workflow/WFTaskFactory.h"
#include "workflow/WFFacilities.h"

using namespace std;

#define RETRY_MAX      1 

namespace redis
{
    void default_callback(WFRedisTask *task);

    void deleteDoc(const string &key, const string &zone, int db = 0,
                    redis_callback_t callback = default_callback);

    void deleteDocs(const vector<string> &keys, const string &zone, int db = 0,
                    redis_callback_t callback = default_callback);

    void putDoc(const string &key, const string &value, const string &zone, int db = 0,
                    redis_callback_t callback = default_callback);

    void getDoc(const string &key, const string &zone, int db = 0,
                    redis_callback_t callback = default_callback);

    void putDocs(const vector<string> &kvs, const string &zone, int db = 0,
                    redis_callback_t callback = default_callback);

    void getDocs(const vector<string> &keys, const string &zone, int db = 0,
                    redis_callback_t callback = default_callback);

    void scan(string cursor, int count, const string &zone, int db = 0, 
                    redis_callback_t callback = default_callback);
}

#endif