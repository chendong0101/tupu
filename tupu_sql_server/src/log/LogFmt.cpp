// vim:set ts=4 sw=4 et:
/**
 *  \file log utilities
 */

#define __STDC_FORMAT_MACROS 1
#include "LogUtil.h"
#include "LogRecord.h"
#include "LogImpl.h"
#include "log.h"
#include <string>
#include <fstream>
#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>
#include <assert.h>

#if HAVE_TAKE_ACE

#include <ace/Thread_Mutex.h>
#include <ace/Null_Mutex.h>
#include <ace/Singleton.h>
#include <ace/SString.h>
#include <ace/INET_Addr.h>
#include <ace/OS_NS_time.h>
#include <ace/Log_Msg_Backend.h>
#include <ace/Log_Record.h>
#include <ace/Trace.h>

#endif


using namespace std;

static char const * priority_names[] =
    {
      TEXT ("LM_SHUTDOWN"),
      TEXT ("LM_TRACE"),
      TEXT ("LM_DEBUG"),
      TEXT ("LM_INFO"),
      TEXT ("LM_NOTICE"),
      TEXT ("LM_WARNING"),
      TEXT ("LM_STARTUP"),
      TEXT ("LM_ERROR"),
      TEXT ("LM_CRITICAL"),
      TEXT ("LM_ALERT"),
      TEXT ("LM_EMERGENCY"),
      TEXT ("LM_UNK(04000)"),
      TEXT ("LM_UNK(010000)"),
      TEXT ("LM_UNK(020000)"),
      TEXT ("LM_UNK(040000)"),
      TEXT ("LM_UNK(0100000)"),
      TEXT ("LM_UNK(0200000)"),
      TEXT ("LM_UNK(0400000)"),
      TEXT ("LM_UNK(01000000)"),
      TEXT ("LM_UNK(02000000)"),
      TEXT ("LM_UNK(04000000)"),
      TEXT ("LM_UNK(010000000)"),
      TEXT ("LM_UNK(020000000)"),
      TEXT ("LM_UNK(040000000)"),
      TEXT ("LM_UNK(0100000000)"),
      TEXT ("LM_UNK(0200000000)"),
      TEXT ("LM_UNK(0400000000)"),
      TEXT ("LM_UNK(01000000000)"),
      TEXT ("LM_UNK(02000000000)"),
      TEXT ("LM_UNK(04000000000)"),
      TEXT ("LM_UNK(010000000000)"),
      TEXT ("LM_UNK(020000000000)")
    };

static inline u_long
log2 (u_long num)
{
  u_long log = 0;

  for (; num > 1; ++log)
    num >>= 1;

  return log;
}

static bool __snprintf_cancheck()
{
    char test[2];
    bool can_check = snprintf (test, 1, "x") != -1;
    assert(can_check);
    return can_check;
}
bool snprintf_cancheck = __snprintf_cancheck();


static const char* priority_name (platform::Log_Priority p)
{
  return priority_names[log2((u_long)p)];
}

namespace platform {

static 
const struct tm * fast_localtime_r(time_t time)
{
    static __thread time_t last_time = 0;
    static __thread struct tm last_tm;

    if (last_time != time) {
        last_time = time;
        return localtime_r(&time, &last_tm);
    } else
        return &last_tm;
}

int ss_format_log_prefix(char prefixbuf[MAX_LOGPREFIX_LEN],
        u_long flags, const Log_Module* module, Log_Priority priority,
        const struct timeval& tv, unsigned tid)
{
    char *fp = prefixbuf;
    int fspace = MAX_LOGPREFIX_LEN;
    // 01234567890123456789012345
    // 19891018 14:25:36
    char timebuf[32];
    if (SS_BIT_ENABLED(flags, Log_Util::VERBOSE) || SS_BIT_ENABLED(flags, Log_Util::VERBOSE_SS))
    {
        time_t cur;
        cur = tv.tv_sec;
        strftime(timebuf, 32, "%Y%m%d %H:%M:%S", fast_localtime_r(cur));
    }

    int len = 0;
    if (SS_BIT_ENABLED(flags, Log_Util::VERBOSE) || SS_BIT_ENABLED(flags, Log_Util::VERBOSE_SS))
    {
        len = snprintf(fp, fspace, "[%.20s:%s] [%s] [%04d] ", \
                       module ? module->module_name() : NULL,
                       priority_name(priority), 
                       timebuf, tid);

        fp += len;
    } else if (SS_BIT_ENABLED(flags, Log_Util::VERBOSE_LITE))
    {
        len = snprintf(fp, fspace, "[%.15s:%s] [%s] ", \
                       module ? module->module_name() : NULL,
                       priority_name(priority)+3, 
                       timebuf+5);
        fp += len;
    }
    return len;
}

int ss_format_log_preprintf(char format2[MAX_FORMAT_LEN], 
        const struct timeval& tv, unsigned pid, unsigned tid,
        const char *format, va_list argp)
{
    char *fp = format2;
    int fspace = MAX_FORMAT_LEN;

    while (*format!='\0' && fspace>0)
    {
        if (*format != '%')
        {
            *fp++ = *format++;
            fspace--;
        }
        else 
        {
            format++;
            switch (*format) {
                case '@':
                    {
                        *fp++='%';
                        fspace --;
                        int len = snprintf(fp, fspace, "08x");
                        fspace -= len;
                        fp += len;
                    }
                    break;
                case 'P':
                    {
                        int len = snprintf(fp, fspace, "%d", pid);
                        fspace -= len;
                        fp += len;
                    }
                    break;
                case 't':
                    {
                        int len = snprintf(fp, fspace, "%u", tid);
                        fspace -= len;
                        fp += len;
                    }
                    break;
                case 'T':
                    {
                        time_t sec = time(0);
                        int len = snprintf(fp, fspace, "%ld", sec);
                        fspace -= len;
                        fp += len;
                    }
                    break;
                case 'D':
                    {
                        time_t cur, cur_u;
                        char timebuf[128];
#if defined (WIN32)
                        ACE_Time_Value tv = ACE_OS::gettimeofday();
                        cur = tv.sec();
                        cur_u = tv.usec();
#else
                        struct timeval tv;
                        gettimeofday(&tv, NULL);
                        cur = tv.tv_sec;
                        cur_u = tv.tv_usec;
#endif
                        strftime(timebuf, 128, "%Y-%m-%d %H:%M:%S", fast_localtime_r(cur));
                        int len = snprintf(fp, fspace, "%s.%06ld", timebuf, cur_u);
                        fspace -= len;
                        fp += len;
                    }
                    break;

                default:
                    *fp++ = '%';
                    *fp++ = *format;
                    fspace -= 2;
            }
            format++;
        }
    }
    if (fspace<=0)
        fp = format2 + MAX_FORMAT_LEN - 1;
    *fp = '\0';
    return fp - format2;
}

} // namespace

