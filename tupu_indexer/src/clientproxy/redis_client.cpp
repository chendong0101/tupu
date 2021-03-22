/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#include "redis_client.h"

#include <iostream>
#include <map>
#include <string>
#include <glog/logging.h>
#include <gflags/gflags.h>

using namespace std;
DEFINE_string(hb_redis_url, "", "redis url");
DEFINE_string(js_redis_url, "", "redis url");
DEFINE_string(gd_redis_url, "", "redis url");

namespace redis
{
    struct user_data {
        string key;
    };

    WFRedisTask *createRedisTask(const string &zone, int db, redis_callback_t callback) {
        string url;
        if (zone == "js") {
            url = FLAGS_js_redis_url;
        } else if (zone == "gd") {
            url = FLAGS_gd_redis_url;
        } else {
            url = FLAGS_hb_redis_url;
        }
        if (db > 0) {
            url += "/" + to_string(db);
        }
        return WFTaskFactory::create_redis_task(url, RETRY_MAX, callback);
        }

    void deleteDoc(const string &key, const string &zone, int db, redis_callback_t callback)
    {
        WFRedisTask *task = createRedisTask(zone, db, callback);
        protocol::RedisRequest *req = task->get_req();
        req->set_request("del", {key});
        task->start();
    }

    void deleteDocs(const vector<string> &keys, const string &zone, int db, redis_callback_t callback)
    {
        WFRedisTask *task = createRedisTask(zone, db, callback);
        protocol::RedisRequest *req = task->get_req();
        req->set_request("del", keys);
        task->start();
    }

    void putDoc(const string &key, const string &value, const string &zone, int db, redis_callback_t callback)
    {
        WFRedisTask *task = createRedisTask(zone, db, callback);
        protocol::RedisRequest *req = task->get_req();
        req->set_request("SET", {key, value});
        // user_data *userData = new user_data();
        // userData->key = entity->getEntityId();
        // task->user_data = userData;
        task->start();
    }
    
    void getDoc(const string &key, const string &zone, int db, redis_callback_t callback)
    {
        WFRedisTask *task = createRedisTask(zone, db, callback);
        task->get_req()->set_request("GET", {key});
        task->start();
    }

    void putDocs(const vector<string> &kvs, const string &zone, int db, redis_callback_t callback) {
        WFRedisTask *task = createRedisTask(zone, db, callback);
        task->get_req()->set_request("MSET", kvs);
        task->start();
    }

    void getDocs(const vector<string> &keys, const string &zone, int db, redis_callback_t callback) {
        WFRedisTask *task = createRedisTask(zone, db, callback);
        task->get_req()->set_request("MGET", keys);
        task->start();
    }

    void scan(string cursor, int count, const string &zone, int db, redis_callback_t callback) {
        WFRedisTask *task = createRedisTask(zone, db, callback);
        task->get_req()->set_request("scan", {cursor, "count", to_string(count)});

        task->start();
    }

    void default_callback(WFRedisTask *task)
    {
        // protocol::RedisRequest *req = task->get_req();
        // protocol::RedisResponse *resp = task->get_resp();
        int state = task->get_state();
        int error = task->get_error();
        protocol::RedisValue val;
        if (state != WFT_STATE_SUCCESS) {
            LOG(ERROR) << "redis error: " << state << "\t" << strerror(error);
        }
    }
} // namespace redis
