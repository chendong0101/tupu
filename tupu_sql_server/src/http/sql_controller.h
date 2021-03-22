/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _SQL_CONTROLLER_H
#define _SQL_CONTROLLER_H

#include <map>
#include "workflow/WFFacilities.h"

using namespace std;

class SQLCtr {
    public:
        static void searchBySQL(WFHttpTask *task, map<string, string> params);

};

#endif