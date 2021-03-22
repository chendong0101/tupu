// vim:set ts=4 sw=4 et:
/**
 *  \file log utilities
 */

#include "LogUtil.h"
#include "log.h"
#include <ace/Log_Msg.h>
#include <ace/OS_NS_strings.h>
#include <ace/OS_NS_string.h>

namespace platform {

/// 构造函数，注册到Log_Util
Log_Module::Log_Module(Log_Util* log, const ACE_TCHAR* name, u_long mask)
: module_name_(name)
, priority_mask_(mask)
, enable_tracing_(false)
, verbose_tracing_(0)
{
    ACE_ASSERT(ACE_OS::strlen(module_name_) < MAXMODULENAMELEN);
    log->register_module(this);
}

const ACE_TCHAR* Log_Module::module_name() const
{
    return module_name_;
}

///Get the priority mask. 
u_long Log_Module::priority_mask () const
{
    return priority_mask_;
}

///Set the priority mask, returns original mask. 
u_long Log_Module::priority_mask (u_long mask)
{
    u_long old = priority_mask_;
    priority_mask_ = mask;
    return old;
}

void Log_Module::enable_debug_messages (Log_Priority priority)
{
    this->priority_mask (this->priority_mask () | priority);
}

void Log_Module::disable_debug_messages (Log_Priority priority)
{
    this->priority_mask (this->priority_mask () & ~priority);
}

// = Control the tracing level.
/// Determine if tracing is enabled (return == 1) or not (== 0)
int  Log_Module::is_tracing(void) const
{
    return enable_tracing_;
}

/// Enable the tracing facility.
void Log_Module::start_tracing (void)
{
    enable_tracing_ = true;
}

/// Disable the tracing facility.
void Log_Module::stop_tracing (void)
{
    enable_tracing_ = false;
}

/// 是否输出详细的trace
int Log_Module::verbose_tracing() const
{
    return this->verbose_tracing_;
}

/// 设置是否输出详细的trace
int Log_Module::verbose_tracing(int v)
{
    int old = this->verbose_tracing_;
    this->verbose_tracing_ = v;
    return old;
}

} //platform

