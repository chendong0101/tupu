
/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef SQL_CLAUSE_ANALYZER_H_
#define SQL_CLAUSE_ANALYZER_H_

#include <string>
#include "model/query.h"
#include "model/task.h"

namespace sql_analyzer
{
    Task *splitSubTask(string sql);
}

#endif