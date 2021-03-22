/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#include "sql_task.h"

#include <functional>
#include "model/task_context.h"
#include "service/sql_scheduler.h"
#include "workflow/WFFacilities.h"

using namespace std::placeholders;

void SQLTask::dispatch()
{
    this->state = WFT_STATE_SUCCESS;
    SeriesWork *series = series_of(this);
    SqlScheduler::searchBySql(series, bind(&SQLTask::callback, this, _1));
    this->subtask_done();
}
SubTask *SQLTask::done()
{
    SeriesWork *series = series_of(this);
    return series->pop();
}

void SQLTask::proceed()
{
    if (status_ == PROCESS)
    {
        TaskContext *context = new TaskContext();
        context->proxyTask = this;
        context->sql = request_.sql();
        SeriesWork *series = Workflow::create_series_work(this, [](const SeriesWork *series) {
            delete (TaskContext *)series->get_context();
        });
        series->set_context(context);
        series->start();
    }
    else
    {
        GPR_ASSERT(status_ == FINISH);
        delete this;
    }
}

void SQLTask::callback(SeriesWork *series)
{
    TaskContext *context = (TaskContext *)series->get_context();
    Task *task = context->currentTask;

    for (auto entity_id : task->entity_ids)
    {
        if (task->entity_map.find(entity_id) != task->entity_map.end())
        {
            proto::Entity &content = task->entity_map[entity_id];
            proto::Entity *newEntity = reply_.add_entities();
            newEntity->MergeFrom(content);
        }
    }

    status_ = FINISH;
    responder_.Finish(reply_, grpc::Status::OK, this);

    LOG_INFO("%s", context->toString().c_str());
}