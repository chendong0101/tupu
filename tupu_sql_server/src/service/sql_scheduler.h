/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _SQL_SCHEDULER_H_
#define _SQL_SCHEDULER_H_

#include "workflow/WFFacilities.h"

using namespace std;

class SqlScheduler {
    public:
        static void searchBySql(SeriesWork *series, function<void (SeriesWork * task)> callback);
};

#endif