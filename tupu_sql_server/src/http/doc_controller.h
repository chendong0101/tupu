/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _DOC_CONTROLLER_H
#define _DOC_CONTROLLER_H

#include <map>
#include "workflow/WFFacilities.h"

using namespace std;

class DocCtr {
    public:
        static void getDocs(WFHttpTask *task, map<string, string> params);
};

#endif