/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _INDEXER_MAIN_H_
#define _INDEXER_MAIN_H_

#include <vector>
#include <gflags/gflags.h>
#include <mongocxx/instance.hpp>
#include "worker_threads.h"
#include "property_id_service.h"
#include "full_indexer.h"
#include "realtime_indexer.h"

DECLARE_int32(kafka_shards);

void startFullIndexer() {
    mongocxx::instance instance{}; // This should be done only once.
    PropertyIdService *pis = PropertyIdService::getInstance();
    pis->init();
    
    WorkerThreads *workerThreadPool = new WorkerThreads(4000, 200);
    FullIndexer *fullIndex = new FullIndexer(workerThreadPool);

    delete fullIndex;
    delete workerThreadPool;
}

void startRealtimeIndexer() {
    mongocxx::instance instance{}; // This should be done only once.
    PropertyIdService *pis = PropertyIdService::getInstance();
    pis->init();

    WorkerThreads *workerThreadPool = new WorkerThreads(4000, 100);

    vector<RealtimeIndexer*> ris;
    for (int i = 0; i < FLAGS_kafka_shards; i++) {
        RealtimeIndexer *ri = new RealtimeIndexer(i, workerThreadPool);
        ris.push_back(ri);
    }
    delete workerThreadPool;
}

#endif
