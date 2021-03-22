/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef TaskContext_H_
#define TaskContext_H_

#include <map>
#include <chrono>
#include <stack>
#include <string>
#include <sstream>
#include <vector>
#include "workflow/WFFacilities.h"
#include "message/entity.pb.h"
#include "task.h"

using namespace std::chrono;
using namespace std;

struct TaskContext
{
	~TaskContext() {
		delete currentTask;
	}
	// params
	string sql;
	string queryStr;
	string resFormat;
	bool forceQuery;
	string encoding;
	bool forwardSearch;
	bool fatResult;

	// middle-state
	SubTask *proxyTask;
	milliseconds es_start;
	milliseconds es_end;

	stack<Task *> taskStack;
	Task *currentTask;

	string toString() {
    	milliseconds end_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		stringstream ss;
		ss << "params:["<< "queryStr=" << queryStr << "&"
		  << "sql=" << sql << "&"
		  << "forceQuery=" << forceQuery << "&"
		  << "encoding=" << encoding << "&"
		  << "forwardSearch=" << forwardSearch << "],"
		  << "[Sogou-Observer"
		  << ",cost=" << to_string(end_time.count() - es_start.count())
		  << ",es_cost=" << to_string(es_end.count() - es_start.count())
		  << ",recall_size=" << to_string(currentTask->entity_ids.size())
		  << ",Owner=OP,Type=tupu_sql_server]";
		return ss.str();
	}
};


#endif
