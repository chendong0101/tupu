/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _PROPERTY_ID_SERVICE_
#define _PROPERTY_ID_SERVICE_

#include <atomic>
#include <map>
#include <string>
#include <vector>
#include "workflow/WFTaskFactory.h"

using namespace std;

class PropertyIdService {
    private:
        map<string, int32_t> m_propertyMap;
        std::mutex m_mutex;
        bool m_ready;
        volatile atomic<int32_t> m_maxId;
        volatile atomic<int32_t> m_subtask_count;

    private:
        PropertyIdService() : m_ready(false), m_maxId(0), m_subtask_count(0) {};
        int32_t addNewProperty(string propertyName);
        void redisGetPropertyIdsCallback(vector<string> keys, WFRedisTask* task);
        void scan(string cursor);

    public:
        void init();
        bool isReady() {return m_ready;}
        int32_t getPropertyId(string propertyName);

    public:
        virtual ~PropertyIdService () = default;
        static PropertyIdService *getInstance()
        {
            static PropertyIdService instance;
            return &instance;
        }
};

#endif