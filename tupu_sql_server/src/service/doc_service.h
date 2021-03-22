/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _DOC_SERVICE_H_
#define _DOC_SERVICE_H_

#include <vector>
#include "workflow/WFFacilities.h"

using namespace std;

class DocService {
    public:
        static void getDocs(SeriesWork *series, vector<string> docIds, function<void (SeriesWork * task)> callback);
};

#endif
