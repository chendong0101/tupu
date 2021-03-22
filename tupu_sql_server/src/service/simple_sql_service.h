/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _SIMPLE_SQL_SERVICE_H_
#define _SIMPLE_SQL_SERVICE_H_

#include "workflow/WFFacilities.h"

using namespace std;

class SqlService {
    public:
        static void searchBySql(SeriesWork *series, string sql, function<void (SeriesWork * task)> callback);
};

#endif