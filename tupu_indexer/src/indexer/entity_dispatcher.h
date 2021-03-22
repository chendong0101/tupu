/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _ENTITY_DISPATCHER_H_
#define _ENTITY_DISPATCHER_H_

#include <thread>
#include <vector>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>

#include <bsoncxx/builder/stream/document.hpp>
#include <gflags/gflags.h>
#include <mongocxx/uri.hpp>
#include <mongocxx/client.hpp>

#include "worker_threads.h"
#include "triad.h"

using namespace std;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;

DECLARE_string(mongo_uri);

class EntityDispatcher
{
private:
    queue<string> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    vector<thread> threads;
    WorkerThreads *m_workerThreads;

public:
    EntityDispatcher(WorkerThreads *workerThreads) : m_workerThreads(workerThreads)
    {
        for (int i = 0; i < 50; i++)
        {
            threads.push_back(thread(&EntityDispatcher::run, this));
        }
    }
    ~EntityDispatcher()
    {
        m_condition.notify_all();
        for (auto &t : threads)
        {
            if (t.joinable())
            {
                t.join();
            }
        }
    }

    void run()
    {
        mongocxx::uri uri(FLAGS_mongo_uri);
        mongocxx::client client = mongocxx::client(uri);
        while (true)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condition.wait(lock, [&] {
                return !m_queue.empty();
            });
            string entityId = m_queue.front();
            m_queue.pop();
            lock.unlock();
            vector<Triad> entity = getEntityTriads(client, entityId);
            if (entity.size() > 0)
            {
                while (m_workerThreads->isFull())
                {
                    LOG(INFO) << "job buffer has full";
                    usleep(300 * 1000);
                }
                m_workerThreads->submit(entity);
            }
        }
    }

    vector<Triad> getEntityTriads(mongocxx::client &client, const string &entityId)
    {
        LOG(INFO) << "get entity from Mongodb: " << entityId;
        mongocxx::database db = client["tupu"];
        mongocxx::collection coll = db["triad"];
        auto cursor = coll.find(document() << "kgc_id" << entityId << finalize);

        vector<Triad> indexJob;
        for (auto c : cursor)
        {
            Triad t;
            try
            {
                t.parseFromMongo(c);
            }
            catch (const std::exception &e)
            {
                LOG(ERROR) << "parse mongo error";
                continue;
            }

            indexJob.push_back(t);
        }
        return indexJob;
    }

    void submit(const string &job)
    {
        while (m_queue.size() > 4000)
        {
            LOG(INFO) << "job buffer has full";
            usleep(300 * 1000);
        }
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(job);
        m_condition.notify_one();
    }
};

#endif