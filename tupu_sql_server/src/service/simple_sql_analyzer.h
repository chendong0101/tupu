/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _SIMPLE_SQL_ANALYZER_H_
#define _SIMPLE_SQL_ANALYZER_H_

#include <string>
#include "model/query.h"
#include "thirdparty/json/nlohmann/json.hpp"

using namespace nlohmann;

namespace sql_analyzer
{
    query::QueryRequest sqlSplit(std::string sql);

    json buildESQuery(const query::QueryRequest &queryRequest);
}

#endif