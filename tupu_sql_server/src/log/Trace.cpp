// vim:set ts=4 sw=4 et:
/**
 *  \file log utilities
 */

#include "LogUtil.h"
#include "log.h"
#include <ace/Trace.h>
#include <ace/Log_Msg.h>
#include <ace/Object_Manager_Base.h>
#include <string>

namespace platform {
/// Determine if tracing is enabled (return == 1) or not (== 0)
int Trace::is_tracing(void)
{
    return ACE_Trace::is_tracing();
}

/// Enable the tracing facility.
void Trace::start_tracing (void)
{
    return ACE_Trace::start_tracing();
}

/// Disable the tracing facility.
void Trace::stop_tracing (void)
{
    return ACE_Trace::stop_tracing();
}

/// Change the nesting indentation level.
void set_nesting_indent (int indent)
{
    return ACE_Trace::set_nesting_indent(indent);
}

/// Get the nesting indentation level.
int Trace::get_nesting_indent (void)
{
    return ACE_Trace::get_nesting_indent();
}

Trace::Trace (const Log_Module& module,
                        const ACE_TCHAR *n,
                        int line,
                        const ACE_TCHAR *file)
                        : name_(n)
                        , module_(module)
{
    // If ACE has not yet been initialized, don't try to trace... there's
    // too much stuff not yet initialized.
    if (this->module_.enable_tracing_) {
        if (ACE_Trace::is_tracing() && !ACE_OS_Object_Manager::starting_up ())
        {
            ACE_Log_Msg *lm = ACE_LOG_MSG;
            if (lm->tracing_enabled ()
                && lm->trace_active () == 0)
            {
                lm->trace_active (1);
                if (this->module_.verbose_tracing_) {
                    SS_DEBUGX ((
                        this->module_,
                        LM_TRACE,
                        ACE_TEXT ("%*s(%t) calling %s in file `%s' on line %d\n"),
                        ACE_Trace::get_nesting_indent () * lm->inc (),
                        ACE_TEXT (""),
                        this->name_,
                        file,
                        line));
                } else {
                    SS_DEBUGX ((
                        this->module_,
                        LM_TRACE,
                        ACE_TEXT ("%*s(%t) calling %s\n"),
                        ACE_Trace::get_nesting_indent () * lm->inc (),
                        ACE_TEXT (""),
                        this->name_));
                }
                lm->trace_active (0);
            }
        }
    }
}

// Perform the second part of the trace, which prints out the NAME as
// the function is exited.

Trace::~Trace (void)
{
    // If ACE has not yet been initialized, don't try to trace... there's
    // too much stuff not yet initialized.
    if (this->module_.enable_tracing_) {
        if (ACE_Trace::is_tracing() && !ACE_OS_Object_Manager::starting_up ())
        {
            ACE_Log_Msg *lm = ACE_LOG_MSG;
            if (lm->tracing_enabled ()
                && lm->trace_active () == 0)
            {
                lm->trace_active (1);
                SS_DEBUGX ((
                    this->module_,
                    LM_TRACE,
                    ACE_TEXT ("%*s(%t) leaving %s\n"),
                    ACE_Trace::get_nesting_indent () * lm->dec (),
                    ACE_TEXT (""),
                    this->name_));
                lm->trace_active (0);
            }
        }
    }
}

} // namespace
