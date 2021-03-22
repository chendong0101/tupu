/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _PROPERTY_UTILS_H
#define _PROPERTY_UTILS_H

#include <mutex>
#include <atomic>
#include <map>
#include <string>
#include <vector>
#include "workflow/WFTaskFactory.h"

using namespace std;

class PropertyIdService
{
private:
    int db;
    map<string, int32_t> m_propertyMap;
    std::mutex m_mutex;
    bool m_ready;
    volatile atomic<int32_t> m_maxId;
    volatile atomic<int32_t> m_subtask_count;

private:
    PropertyIdService() : db(1), m_ready(false), m_maxId(0), m_subtask_count(0){};
    void redisGetPropertyIdsCallback(vector<string> keys, WFRedisTask *task);
    void scan(string cursor);

public:
    void init();
    bool isReady() { return m_ready; }
    int32_t getPropertyId(string propertyName);

public:
    virtual ~PropertyIdService() = default;
    static PropertyIdService *getInstance()
    {
        static PropertyIdService instance;
        return &instance;
    }
};

#endif