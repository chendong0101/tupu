/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _ES_CLIENT_H_
#define _ES_CLIENT_H_

#include "thirdparty/json/nlohmann/json.hpp"
#include "workflow/WFFacilities.h"

#define REDIRECT_MAX   0 
#define RETRY_MAX      1 

using namespace::nlohmann;

namespace es
{
    void search(SeriesWork *series, json queryJson, http_callback_t callback);

} // namespace es

#endif