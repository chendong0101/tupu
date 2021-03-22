/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _REALTIME_INDEXER_H_
#define _REALTIME_INDEXER_H_

#include <thread>
#include <vector>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>

#include <gflags/gflags.h>
#include <librdkafka/rdkafkacpp.h>

#include "worker_threads.h"
#include "entity_dispatcher.h"
#include "triad.h"

using namespace std;

class RealtimeIndexer : public RdKafka::ConsumeCb
{
    private:
        std::thread             m_thread;
        int                     m_partition;
        long                    m_startOffset;
        string                  m_offsetKey;
        EntityDispatcher        *m_entityDispatcher;

    public:
        RealtimeIndexer(int partition, WorkerThreads *wtp) :
                    m_partition(partition) {
            m_entityDispatcher = new EntityDispatcher(wtp);
            init();

            m_thread = std::thread(&RealtimeIndexer::startKafkaListener, this);
        }
        ~RealtimeIndexer() {
            m_thread.join();
        }

    public:
        void consume_cb(RdKafka::Message &msg, void *opaque);

    private:
        void init();

        void startKafkaListener();

};

#endif