/*
  Copyright (c) 2020 Sogou, Inc.

  Author: Wu Jiaxu (wuujiaxu@sogou-inc.com)
*/

#include "tupu_log.h"

#include <string>
#include "log.h"

SS_LOG_MODULE_DEF(tupu_sql_server);

namespace tupu
{
namespace log
{

volatile bool g_is_debug = false;

void init(bool debug_mod)
{

    g_is_debug = debug_mod;

    if (debug_mod)
        SS_LOG_UTIL->process_priority_mask(LM_DEBUG | LM_INFO | LM_ERROR);
    else
        SS_LOG_UTIL->process_priority_mask(LM_INFO | LM_ERROR);

    SS_LOG_UTIL->open();
    LOG_INFO("log init success\n");
}

bool is_debug_mode()
{
    return g_is_debug;
}

}// namespace log
}// namespace tupu

