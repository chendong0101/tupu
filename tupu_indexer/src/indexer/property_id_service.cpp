/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#include "property_id_service.h"

#include <unistd.h>
#include <iostream>
#include <functional>
#include <vector>
#include <glog/logging.h>
#include <gflags/gflags.h>
#include "clientproxy/redis_client.h"

using namespace std;
using namespace std::placeholders;

DECLARE_int32(redis_property_id_db);

void PropertyIdService::redisGetPropertyIdsCallback(vector<string> keys, WFRedisTask* task) {
    protocol::RedisResponse *resp = task->get_resp();
    int state = task->get_state();
    protocol::RedisValue val;

    if (state == WFT_STATE_SUCCESS) {
        resp->get_result(val);
        if (val.is_error())
        {
            LOG(ERROR) << "Mget Error reply.";
            this->m_subtask_count--;
            if (this->m_subtask_count == 0) {
                this->m_ready = true;
            }
        }
    } else {
        LOG(ERROR) <<  "MGet Failed. Press Ctrl-C to exit.";
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
            } else {
                LOG(INFO) << "null" ;
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
    redis::scan(cursor, 1000, "hb", FLAGS_redis_property_id_db, [&](WFRedisTask* task) {
        protocol::RedisResponse *resp = task->get_resp();
        int state = task->get_state();
        protocol::RedisValue val;

        if (state == WFT_STATE_SUCCESS) {
            resp->get_result(val);
            if (val.is_error())
            {
                LOG(ERROR) <<  "Scan Error reply. Need a password?";
                state = WFT_STATE_TASK_ERROR;
            }
        } else {
            LOG(ERROR) <<  "Scan Failed.";
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
                redis::getDocs(keys, "hb", FLAGS_redis_property_id_db, bind(&PropertyIdService::redisGetPropertyIdsCallback, this, keys, _1));
                // redis::deleteDocs(keys, FLAGS_redis_property_id_db, redis::default_callback);
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
        LOG(INFO) << "waiting ..." ;
    }
    LOG(INFO) << "PropertyIdService finish init, load " << m_maxId << " properties." ;
}

int32_t PropertyIdService::addNewProperty(string propertyName) {
    std::unique_lock<std::mutex> lock(m_mutex);
    LOG(INFO) << "add new id" << propertyName ;
    if(m_propertyMap.find(propertyName) == m_propertyMap.end()) {
        m_propertyMap[propertyName] = ++m_maxId;
        redis::putDoc(propertyName, to_string(m_propertyMap[propertyName]), "hb", FLAGS_redis_property_id_db);
    }
    return m_propertyMap[propertyName];
}

int32_t PropertyIdService::getPropertyId(string propertyName) {
    auto it = m_propertyMap.find(propertyName);
    if (it != m_propertyMap.end()) {
        return it->second;
    } else {
        // return addNewProperty(propertyName);
        return -1;
    }
}
