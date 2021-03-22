/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#include "grpc_service.h"

#include <functional>
#include "sql_task.h"

using namespace std::placeholders;

GRPCService::GRPCService()
{
    m_finish = false;
    for (int i = 0; i < 5; i++)
    {
        m_workThreads.push_back(thread(&GRPCService::run, this));
    }
}

GRPCService::~GRPCService() {}

void GRPCService::start()
{
    std::string server_address("0.0.0.0:50051");

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service_);
    cq_ = builder.AddCompletionQueue();
    scq_ = builder.AddCompletionQueue();
    server_ = builder.BuildAndStart();
    LOG_INFO("GRPC Server has started on %d.", server_address.c_str());

    SQLTask *task = new SQLTask();
    service_.Requestsearch(&task->ctx_, &task->request_, &task->responder_, cq_.get(), scq_.get(), task);
    handleRequests();

    LOG_INFO("GRPC Server terminates from %d.", server_address.c_str());
}

void GRPCService::handleRequests()
{
    std::thread t1([&]() {
        void *tag;
        bool ok;
        while (!m_finish)
        {
            cq_->Next(&tag, &ok);
            if (!ok)
            {
                continue;
            }
            submit(tag);
        }
    });

    std::thread t2([&]() {
        void *tag;
        bool ok;
        while (!m_finish)
        {
            scq_->Next(&tag, &ok);
            if (!ok)
            {
                continue;
            }
            SQLTask *task = new SQLTask();
            service_.Requestsearch(&task->ctx_, &task->request_, &task->responder_, cq_.get(), scq_.get(), task);
            submit(tag);
        }
    });
    t1.join();
    t2.join();
}

void GRPCService::handleTask()
{
    while (!m_finish)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition.wait(lock, [&] { return !m_queue.empty() || m_finish; });
        if (m_finish)
        {
            LOG_INFO("grpc worker stopped");
            return;
        }
        void *task = m_queue.front();
        m_queue.pop();
        lock.unlock();
        static_cast<SQLTask *>(task)->proceed();
    }
}

void GRPCService::submit(void *task)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_queue.push(task);
    m_condition.notify_one();
}