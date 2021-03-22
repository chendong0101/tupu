/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef REDIS_CLIENT_
#define REDIS_CLIENT_

#include <string>
#include <vector>
#include "workflow/Workflow.h"
#include "workflow/RedisMessage.h"
#include "workflow/WFTaskFactory.h"
#include "workflow/WFFacilities.h"

using namespace std;

#define RETRY_MAX      1 

namespace redis
{
    void default_callback(WFRedisTask *task);

    void mget(SeriesWork *series, const vector<string> &keys, int db = 0,
                 redis_callback_t callback = default_callback);

    void scan(string cursor, int count, int db = 0, 
                 redis_callback_t callback = default_callback);
}

#endif