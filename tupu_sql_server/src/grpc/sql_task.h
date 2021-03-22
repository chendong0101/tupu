/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _SQL_TASK_H
#define _SQL_TASK_H

#include <map>
#include <grpcpp/grpcpp.h>
#include <string>

#include "message/entity.pb.h"
#include "message/entity.grpc.pb.h"
#include "workflow/WFFacilities.h"

#include "log/tupu_log.h"

using grpc::ServerAsyncResponseWriter;
using grpc::ServerContext;

using namespace std;

class SQLTask : WFGenericTask
{
public:
    SQLTask() : responder_(&ctx_), status_(PROCESS) {}

    void proceed();
    void callback(SeriesWork *series);

protected:
    virtual void dispatch();
    virtual SubTask *done();

public:
    ServerContext ctx_;
    proto::SearchRequest request_;
    proto::FatSearchResult reply_;
    ServerAsyncResponseWriter<proto::FatSearchResult> responder_;

private:
    enum Status
    {
        PROCESS,
        FINISH
    };
    Status status_;
};

#endif