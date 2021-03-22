/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#include "redis_client.h"

#include <string>
#include <gflags/gflags.h>
#include "workflow/RedisMessage.h"
#include "workflow/WFTaskFactory.h"
#include "workflow/WFFacilities.h"
#include "log/tupu_log.h"

using namespace std;

DECLARE_string(redis_url);

namespace redis
{
    void redis_callback(WFRedisTask *task);
    void dealWithEntityContent(SeriesWork* serise);

    WFRedisTask *createRedisTask(int db, redis_callback_t callback) {
        string url = FLAGS_redis_url;
        if (db > 0) {
            url += "/" + to_string(db);
        }
        return WFTaskFactory::create_redis_task(url, RETRY_MAX, callback);
    }

    void mget(SeriesWork *series, const vector<string> &keys, int db, redis_callback_t callback) {
        WFRedisTask *task = createRedisTask(db, callback);
        task->get_req()->set_request("MGET", keys);
        if (series != NULL) {
            series->push_back(task);
        } else {
            task->start();
        }
    }

    void scan(string cursor, int count, int db, redis_callback_t callback) {
        WFRedisTask *task = createRedisTask(db, callback);
        task->get_req()->set_request("scan", {cursor, "count", to_string(count)});
        task->start();
    }

    void default_callback(WFRedisTask *task)
    {
        int state = task->get_state();
        int error = task->get_error();
        protocol::RedisValue val;
        if (state != WFT_STATE_SUCCESS)
        {
            LOG_ERROR("redis error: %d\t%s", state, strerror(error));
        }
    }
} // namespace redis
