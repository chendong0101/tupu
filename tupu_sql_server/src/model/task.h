/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef Task_H_
#define Task_H_

#include <map>
#include <string>
#include <vector>
#include "message/entity.pb.h"
#include "query.h"

using namespace std;

struct Task
{
    string sql;
    size_t start;
    size_t end;

	query::QueryRequest queryRequest;

	// result
	vector<string> entity_ids;
	map<string, proto::Entity> entity_map;

    Task(string sql, size_t start, size_t end) {
        this->sql = sql;
        this->start = start;
        this->end = end;
    }

    Task() {}

};


#endif