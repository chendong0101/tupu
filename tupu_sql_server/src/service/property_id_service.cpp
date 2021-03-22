/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#include "property_id_service.h"

#include <unistd.h>
#include <iostream>
#include <vector>
#include <functional>

#include "clientproxy/redis_client.h"
#include "log/tupu_log.h"

using namespace std;
using namespace std::placeholders;

void PropertyIdService::redisGetPropertyIdsCallback(vector<string> keys, WFRedisTask* task) {
    protocol::RedisResponse *resp = task->get_resp();
    int state = task->get_state();
    protocol::RedisValue val;

    if (state == WFT_STATE_SUCCESS) {
        resp->get_result(val);
        if (val.is_error())
        {
            LOG_ERROR("Mget Error reply. %d", state);
            this->m_subtask_count--;
            if (this->m_subtask_count == 0) {
                this->m_ready = true;
            }
        }
    } else {
        LOG_ERROR("Mget Error reply. %d", state);
        this->m_subtask_count--;
        if (this->m_subtask_count == 0) {
            this->m_ready = true;
        }
        return;
    }
    if (val.is_array()) {
        std::unique_lock<std::mutex> lock(this->m_mutex);
        for (int i = 0; i < val.arr_size(); i++) {
            if (val[i].is_string()) {
                int32_t pid = stoi(val[i].string_value());
                this->m_propertyMap[keys[i]] = pid;
                if (pid > this->m_maxId) {
                    this->m_maxId = pid;
                }
            }
        }
        this->m_subtask_count--;
        if (this->m_subtask_count == 0) {
            this->m_ready = true;
        }
    }
}

void PropertyIdService::scan(string cursor) {
    m_subtask_count++;
    redis::scan(cursor, 2000, db, [&](WFRedisTask* task) {
        protocol::RedisResponse *resp = task->get_resp();
        int state = task->get_state();
        protocol::RedisValue val;

        if (state == WFT_STATE_SUCCESS) {
            resp->get_result(val);
            if (val.is_error())
            {
                LOG_ERROR("Scan Error reply. %d", state);
                state = WFT_STATE_TASK_ERROR;
            }
        } else {
            LOG_ERROR("Scan Failed. %d", state);
            return;
        }
        if (val.is_array()) {
            vector<string> keys;
            string cursor = val[0].string_value();
            std::unique_lock<std::mutex> lock(this->m_mutex);
            if (val[1].is_array()) {
                for (int i = 0; i < val[1].arr_size(); i++)
                {
                    keys.push_back(val[1][i].string_value());
                }
                redis::mget(NULL, keys, db, bind(&PropertyIdService::redisGetPropertyIdsCallback, this, keys, _1));
            }
            if (cursor.compare("0") != 0) {
                this->scan(cursor);
            }
        }
    });
}

void PropertyIdService::init() {
    scan("0");
    while (!isReady())
    {
        usleep(1000000);
        LOG_INFO("waiting ...");
    }
    LOG_INFO("PropertyIdService finish init, load %d properties.",  (int)m_maxId);
}

int32_t PropertyIdService::getPropertyId(string propertyName) {
    auto it = m_propertyMap.find(propertyName);
    if (it != m_propertyMap.end()) {
        return it->second;
    } else {
        return -1;
    }
}
