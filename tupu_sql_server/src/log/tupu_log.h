/*
  Copyright (c) 2020 Sogou, Inc.

  Author: Wu Jiaxu (wuujiaxu@sogou-inc.com)
*/

#ifndef __TUPU_LOG_H__
#define __TUPU_LOG_H__

#include <string>
#include <chrono>

#include "log.h"

#define SYS_NOW   std::chrono::system_clock::now
#define GET_NS(x) std::chrono::duration_cast<std::chrono::nanoseconds>((x).time_since_epoch()).count()
#define GET_MS(x) std::chrono::duration_cast<std::chrono::milliseconds>((x).time_since_epoch()).count()
#define GET_S(x)  std::chrono::duration_cast<std::chrono::seconds>((x).time_since_epoch()).count()

#define LOG_DEBUG(format,...) SS_DEBUG((LM_DEBUG, format, ##__VA_ARGS__))
#define LOG_INFO(format,...)  SS_DEBUG((LM_INFO,  format, ##__VA_ARGS__))
#define LOG_ERROR(format,...) SS_DEBUG((LM_ERROR, format, ##__VA_ARGS__))

SS_LOG_MODULE_USE(tupu_sql_server);

namespace tupu
{
namespace log
{

void init(bool debug_mod);
bool is_debug_mode();

/*
class Clocker
{
public:
    Clocker()
    {
        auto now = std::chrono::system_clock::now();
        tt_ = std::chrono::system_clock::to_time_t(now);
        nano_ = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
        localtime_r(&tt_, &tm_);
    }

    ~Clocker() { }

    int year() const { return tm_.tm_year + 1900; }
    int month() const { return tm_.tm_mon + 1; }
    int day() const { return tm_.tm_mday; }
    int hour() const { return tm_.tm_hour; }
    int minute() const { return tm_.tm_min; }
    int second() const { return tm_.tm_sec; }
    int timestamp() const { return tt_; }

private:
    int64_t nano_;
    struct tm tm_;
    time_t tt_;
};
*/

}// namespace log
}// namespace tupu

#endif

