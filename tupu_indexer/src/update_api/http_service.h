/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _HTTP_SERIVICE_H
#define _HTTP_SERIVICE_H

#include "workflow/WFFacilities.h"

class HttpService {
    public:
        HttpService();
        virtual ~HttpService() = default;
    
    public:
        void httpProcess(WFHttpTask *task);
};

#endif //_HTTP_SERIVICE_H 