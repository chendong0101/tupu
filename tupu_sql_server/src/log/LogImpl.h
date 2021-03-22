// vim:set ts=4 sw=4 et:
#pragma once

#include "log.h"
#include "LogUtil.h"
#include "LogRecord.h"
#include <vector>

namespace platform {

// max prefix length is less than 128 
// "[01234567890123456789:LM_0123456789] [20120618 09:00:00] [0123456789]"
static const unsigned MAX_LOGPREFIX_LEN = 128;
// max format string length
static const unsigned MAX_FORMAT_LEN = 2048;
static const unsigned MAX_BATCHD_LINES = 200;
static const unsigned MAX_LOGBUF_LEN = 63<<10;

class Log_Impl {
    friend class Log_Util;
    Log_Impl () = default;
    Log_Impl (const Log_Impl&) = delete;
    Log_Impl& operator= (const Log_Impl&) = delete;
public:
    /// for batched mode
    u_long batched_priority_mask_;

    /// batch buffer
    std::vector<Log_Record> batch_buffer_;

    /// a buffer for avoid malloc
    char buffer_[MAX_LOGBUF_LEN];
    unsigned buffer_used_ ;
};

int ss_format_log_prefix(char prefixbuf[MAX_LOGPREFIX_LEN],
        u_long flags, const Log_Module* module, Log_Priority priority,
        const struct timeval& tv, unsigned tid);

int ss_format_log_preprintf(char format2[MAX_FORMAT_LEN],
        const struct timeval& tv, unsigned pid, unsigned tid,
		const char *format, va_list argp);

} // namespace

