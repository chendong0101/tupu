/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _FULL_INDEXER_
#define _FULL_INDEXER_

#include <thread>
#include <vector>
#include <glog/logging.h>
#include "worker_threads.h"

using namespace std;

static vector<string> regexes {"^0.*", "^1.*", "^2.*", "^3.*", "^4.*", "^5.*", "^6.*", "^7.*", 
                               "^8.*", "^9.*", "^a.*", "^b.*", "^c.*", "^d.*", "^e.*", "^f.*"};

class FullIndexer {
    private:
        WorkerThreads *m_workerThreadPool;
        vector<std::thread*> m_threads;

    public:
        FullIndexer(WorkerThreads *workerThreadPool) 
                            : m_workerThreadPool(workerThreadPool) {
            // m_threads.push_back(new std::thread(&FullIndexer::run, this, "d"));
            for (auto r : regexes) {
                m_threads.push_back(new std::thread(&FullIndexer::run, this, r));
            }
        }
        ~FullIndexer() {
            for (std::thread *t : m_threads) {
                if (t->joinable()) {
                    t->join();
                }
            }
            m_workerThreadPool->shutdown();
        }

    private:
        void run(string regex);
};

#endif