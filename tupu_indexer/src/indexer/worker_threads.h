/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _INDEX_WORKER_THREADS_
#define _INDEX_WORKER_THREADS_

#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "triad.h"

using namespace std;

class WorkerThreads {
    private:
        bool                        m_shutdown;
        queue<vector<Triad>>        m_queue;
        size_t                      m_capacity;
        std::mutex                  m_mutex;
        std::condition_variable     m_condition;
        vector<std::thread>         m_workers;

    public:
        WorkerThreads(size_t capacity, size_t workerCount);
        ~WorkerThreads();

        void run();
        vector<Triad> pop() {
            vector<Triad> job = m_queue.front();
            m_queue.pop();
            return job;
        }
        
        bool isFull() {
            return m_capacity < m_queue.size();
        }

        void submit(const vector<Triad> &job) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_queue.push(job);
            m_condition.notify_one();
        }

        void shutdown() {
            m_shutdown = true;
        }

};

#endif