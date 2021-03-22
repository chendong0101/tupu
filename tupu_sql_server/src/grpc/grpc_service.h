/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _GRPC_RUNNER_H
#define _GRPC_RUNNER_H

#include <map>
#include <grpcpp/grpcpp.h>
#include <queue>
#include <string>
#include <signal.h>
#include <thread>

#include "sql_task.h"
#include "message/entity.pb.h"
#include "message/entity.grpc.pb.h"

#include "log/tupu_log.h"

using grpc::CompletionQueue;
using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;

using namespace std;

class GRPCService
{
public:
    GRPCService();
    ~GRPCService();

    void start();
    void handleRequests();
    void handleTask();
    void submit(void *task);

    void terminate()
    {
        m_finish = true;

        m_condition.notify_all();
        for (auto &t : m_workThreads)
        {
            if (t.joinable())
            {
                t.join();
            }
        }
        server_->Shutdown();
        cq_->Shutdown();
        scq_->Shutdown();
    }

private:
    std::unique_ptr<ServerCompletionQueue> cq_;
    std::unique_ptr<ServerCompletionQueue> scq_;
    proto::TupuSearchService::AsyncService service_;
    std::unique_ptr<Server> server_;

    queue<void *> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    vector<thread> m_workThreads;
    volatile bool m_finish;
};

#endif