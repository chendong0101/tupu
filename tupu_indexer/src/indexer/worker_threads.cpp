/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#include "worker_threads.h"

#include <vector>
#include <glog/logging.h>
#include <gflags/gflags.h>

#include "clientproxy/redis_client.h"
#include "clientproxy/es_client.h"
#include "entity.h"

using namespace std;
DECLARE_int32(redis_doc_db);

WorkerThreads::WorkerThreads(size_t capacity, size_t workerCount) : m_shutdown(false), m_capacity(capacity) {
    for (int i = 0; i < workerCount; i++) {
        m_workers.push_back(std::thread(&WorkerThreads::run, this));
    }
}

WorkerThreads::~WorkerThreads() {
    m_condition.notify_all();
    for (auto &t : m_workers) {
        if (t.joinable())
        {
            t.join();
        }
    }
}

void WorkerThreads::run() {
    volatile atomic_int subtasks;
    while (true) {
        while(subtasks > 0) {
            usleep(100);
        }
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition.wait(lock, [&] {
            return m_shutdown || !m_queue.empty();
        });
        if (!m_queue.empty()) {
            vector<Triad> jobTriads = pop();
            lock.unlock();
            if (jobTriads.size() < 1)
            {
                continue;
            }
            Triad t = jobTriads[0];
            Entity entity(t.kgc_id, t.entity_id);
            for (Triad t : jobTriads)
            {
                entity.addTriad(t);
            }
            LOG(INFO) << "upsert entity: " << t.kgc_id;
            subtasks = 4;
            redis::putDoc(entity.getKgcId(), entity.serialize(), "hb", FLAGS_redis_doc_db, [&](WFRedisTask *task) {
                int state = task->get_state();
                if (state != WFT_STATE_SUCCESS)
                {
                    LOG(ERROR) << "redis put doc error: hb: " << entity.getKgcId();
                }
                subtasks--;
            });
            redis::putDoc(entity.getKgcId(), entity.serialize(), "js", FLAGS_redis_doc_db, [&](WFRedisTask *task) {
                int state = task->get_state();
                if (state != WFT_STATE_SUCCESS)
                {
                    LOG(ERROR) << "redis put doc error: js: " << entity.getKgcId();
                }
                subtasks--;
            });
            redis::putDoc(entity.getKgcId(), entity.serialize(), "gd", FLAGS_redis_doc_db, [&](WFRedisTask *task) {
                int state = task->get_state();
                if (state != WFT_STATE_SUCCESS)
                {
                    LOG(ERROR) << "redis put doc error: gd: " << entity.getKgcId();
                }
                subtasks--;
            });
            es::index_put(entity.getKgcId(), entity.toIndexJson(), [&](WFHttpTask *task) {
                int state = task->get_state();
                if (state != WFT_STATE_SUCCESS)
                {
                    LOG(ERROR) << "elasticsearch put doc error: " << entity.getKgcId();
                }
                subtasks--;
            });
        }
        else if (m_shutdown)
        {
            LOG(INFO) << std::this_thread::get_id() << " no job left, shutdown.";
            return;
        }
    }
}
