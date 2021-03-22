/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#include "sql_scheduler.h"

#include <functional>
#include <string>
#include <sstream>
#include "model/task.h"
#include "model/task_context.h"
#include "doc_service.h"
#include "simple_sql_service.h"
#include "sql_clause_analyzer.h"

using namespace std::placeholders;
using namespace std;

void dispatchCallback(SeriesWork *series, function<void(SeriesWork *)> callback);

void searchForward(SeriesWork *series, function<void(SeriesWork *)> callback)
{
    TaskContext *context = (TaskContext *)series->get_context();
    Task *task = context->currentTask;
    set<string> select = task->queryRequest.sql_select;
    if (select.size() > 0)
    {
        set<string> entityIds;
        map<string, proto::Entity> newEntityMap;
        for (auto iter = task->entity_map.begin(); iter != task->entity_map.end(); iter++)
        {
            proto::Entity &oldContent = iter->second;
            proto::Entity newContent;
            newContent.set_kgc_id(oldContent.kgc_id());
            newContent.set_entity_id(oldContent.entity_id());
            newContent.set_index(oldContent.index());
            for_each(oldContent.triads().begin(), oldContent.triads().end(), [&](const proto::Triad &triad) {
                string sfield = triad.predicate();
                if (sfield.length() > 8 && sfield.find("webfact_") == 0)
                {
                    sfield = sfield.substr(8);
                }
                else if (sfield.length() > 7 && sfield.find("schema_") == 0)
                {
                    sfield = sfield.substr(7);
                }
                if (select.find(sfield) != select.end())
                {
                    string to_kgc_id = triad.to_kgc_id();
                    if (to_kgc_id != "-1")
                    {
                        entityIds.insert(to_kgc_id);
                    }
                    proto::Triad *newTriad = newContent.add_triads();
                    newTriad->MergeFrom(triad);
                }
            });
            newEntityMap[newContent.kgc_id()] = newContent;
        }
        if (entityIds.size() > 0)
        {
            task->entity_map.clear();
            return DocService::getDocs(series, vector<string>(entityIds.begin(), entityIds.end()), callback);
        }
        task->entity_map = newEntityMap;
    }

    callback(series);
}

void dispatch(SeriesWork *series, function<void(SeriesWork *)> callback) {
    TaskContext *context = (TaskContext *)series->get_context();
    Task *topTask = context->taskStack.top();
    Task *subTask = sql_analyzer::splitSubTask(topTask->sql);
    if (subTask != NULL) {
        context->taskStack.push(subTask);
        return dispatch(series, callback);
    }
    context->currentTask = topTask;
    SqlService::searchBySql(series, topTask->sql, bind(&dispatchCallback, _1, callback));
}

void dispatchCallback(SeriesWork *series, function<void(SeriesWork *)> callback) {
    TaskContext *context = (TaskContext *)series->get_context();
    if (context->taskStack.size() == 1) { 
        if (context->forwardSearch) {
            searchForward(series, callback);
        } else {
            callback(series);
        }
        return;
    }
    Task *task = context->currentTask;
    context->taskStack.pop();
    Task *pTask = context->taskStack.top();
    string whereTmp = pTask->sql.substr(task->start - 6, 6);
    set<string> select = task->queryRequest.sql_select;
    set<string> values;
    if (select.size() > 0)
    {
        set<string> entityIds;
        for (auto iter = task->entity_map.begin(); iter != task->entity_map.end(); iter++)
        {
            proto::Entity &content = iter->second;
            for_each(content.triads().begin(), content.triads().end(), [&](const proto::Triad &triad) {
                string sfield = triad.predicate();
                if (sfield.length() > 8 && sfield.find("webfact_") == 0)
                {
                    sfield = sfield.substr(8);
                }
                else if (sfield.length() > 7 && sfield.find("schema_") == 0)
                {
                    sfield = sfield.substr(7);
                }
                if (select.find(sfield) != select.end())
                {
                    string to_kgc_id = triad.to_kgc_id();
                    if (whereTmp.find("ID") != string::npos)
                    {
                        if (to_kgc_id != "-1")
                        {
                            values.insert(to_kgc_id);
                        }
                    }
                    else
                    {
                        values.insert(triad.value());
                    }
                }
            });
        }
    } else {
        values.insert(task->entity_ids.begin(), task->entity_ids.end());
    }

    ostringstream stream;
    std::copy(values.begin(), values.end(), std::ostream_iterator<std::string>(stream, ","));
    string result = stream.str();

    pTask->sql.replace(task->start, task->end - task->start, result);
    delete task;
    context->currentTask = NULL;
    dispatch(series, callback);
}

void SqlScheduler::searchBySql(SeriesWork *series, function<void(SeriesWork *)> callback)
{
    TaskContext *context = (TaskContext *)series->get_context();
    string sql = context->sql;
    context->es_start = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    Task *task0 = new Task(sql, 0, sql.length());
    context->taskStack.push(task0);
    dispatch(series, callback);
}