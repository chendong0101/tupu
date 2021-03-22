/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _ES_OPLOG_PROCESSOR_H
#define _ES_OPLOG_PROCESSOR_H

#include <functional>
#include <thread>
#include <list>
#include <set>
#include <librdkafka/rdkafkacpp.h>
#include "workflow/WFTaskFactory.h"
#include "oplog_buffer.h"

using namespace std;

typedef function<void()> ProcessorCallback;

class OplogProcessor {
    private:
        OplogBuffer         *m_oplogBuffer;
        std::thread         m_thread;
        atomic_bool         m_block;
        list<Oplog>         m_oplogs;
        set<string>         m_entityIds;
        RdKafka::Producer   *m_producer;
        RdKafka::Topic      *m_topic;
        int32_t             m_partition;

    private:
        void init();
        void run();
    public: 
        OplogProcessor(OplogBuffer *buf) : m_oplogBuffer(buf), m_block(false) {
            init();
            m_thread = std::thread(&OplogProcessor::run, this);
        }
        ~OplogProcessor() {
            m_thread.join();
        }
    private:
        void updateTriadIds();
        void getEntityIds(ProcessorCallback callback);
        void getEntityIdsCallback(ProcessorCallback callbace, WFRedisTask* task);
        void sendMessage2Kafka(ProcessorCallback callback);
        void eraseStatus();

};

#endif // ES_OPLOG_PROCESSOR_H