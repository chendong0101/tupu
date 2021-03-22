// vim:set ts=4 sw=4 et:
/**
 *  \file log utilities
 */

#include "LogUtil.h"
#include "LogImpl.h"
#include "log.h"

#include <ace/Hash_Map_Manager.h>
#include <ace/Thread_Mutex.h>
#include <ace/Null_Mutex.h>
#include <ace/Singleton.h>
#include <ace/SString.h>
#include <ace/OS_NS_time.h>
#include <ace/Log_Msg_Backend.h>
#include <ace/Log_Record.h>
#include <string>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <map>
#include "properties.h"

#include <unistd.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)

#define _T(X) X

ACE_BEGIN_VERSIONED_NAMESPACE_DECL

ACE_INLINE
u_long ACE_Log_Msg::flags (void)
{
    return ACE_Log_Msg::flags_;
}

ACE_END_VERSIONED_NAMESPACE_DECL

using namespace std;

namespace platform {

typedef ACE_Hash_Map_Manager<ACE_TString, Log_Module*, ACE_Thread_Mutex> module_dict_t;
typedef map<tstring, tstring> StringDict;

///////////////////////////////

class Log_Util_Manager
    : ACE_Copy_Disabled
{
    friend class Log_Util;

public:
    
    Log_Util_Manager(void);
    ~Log_Util_Manager(void);

    /// Returns a pointer to the Singleton.
    static Log_Util_Manager *instance (void);

    /// 注册一个module
    int register_module (Log_Module* module);

    /// 查找一个module
    Log_Module* find_module (const ACE_TCHAR* name);

private:
    module_dict_t module_map_;
};

///////////////////////////////

Log_Util_Manager::Log_Util_Manager(void)
{
    // set stderr to O_APPEND
    fcntl(fileno(stderr), F_SETFL, O_APPEND);
}

Log_Util_Manager::~Log_Util_Manager(void)
{
}

Log_Util_Manager* Log_Util_Manager::instance(void)
{
    return ACE_Singleton<Log_Util_Manager, ACE_Thread_Mutex>::instance();
}

int Log_Util_Manager::register_module (Log_Module* module)
{
    if (module_map_.bind(module->module_name(), module) != 0) {
        ACE_ASSERT(false /*regsiter duplicate module*/);
        return -1;
    }
    return 0;
}

Log_Module* Log_Util_Manager::find_module (const ACE_TCHAR* name)
{
    Log_Module* r = NULL;
    if (module_map_.find (name, r) != 0) {
        return NULL;
    }
    return r;
}


///////////////////////////////

const u_long Log_Util::default_priority_mask_ =  LM_SHUTDOWN
                                            | LM_TRACE
                                            | LM_DEBUG
                                            | LM_INFO
                                            | LM_NOTICE
                                            | LM_WARNING
                                            | LM_STARTUP
                                            | LM_ERROR
                                            | LM_CRITICAL
                                            | LM_ALERT
                                            | LM_EMERGENCY
                                            | LM_ONLYSTDERR;

const ACE_TCHAR* Log_Util::priority_names_[] = {
    _T("SHUTDOWN"), 
    _T("TRACE"), 
    _T("DEBUG"), 
    _T("INFO"), 
    _T("NOTICE"), 
    _T("WARNING"), 
    _T("STARTUP"), 
    _T("ERROR"), 
    _T("CRITICAL"), 
    _T("ALERT"), 
    _T("EMERGENCY"), 
    _T("ONLYSTDERR"), 
    NULL
};

const u_long Log_Util::priority_values_[] = {
    LM_SHUTDOWN, 
    LM_TRACE, 
    LM_DEBUG, 
    LM_INFO, 
    LM_NOTICE, 
    LM_WARNING, 
    LM_STARTUP, 
    LM_ERROR, 
    LM_CRITICAL, 
    LM_ALERT, 
    LM_EMERGENCY, 
    LM_ONLYSTDERR,
    0
};

static u_long parse_log_priority_mask(const ACE_TCHAR* maskstring)
{
    const ACE_TCHAR* p = maskstring;
    u_long result = LM_STARTUP|LM_SHUTDOWN|LM_EMERGENCY;
    while (*p) {
        const ACE_TCHAR* q = NULL;
        ACE_TCHAR* t = NULL;
        const ACE_TCHAR* * h = NULL;
        u_long item = 0;

        //  处理分隔符
        p += ACE_OS::strspn(p, _T(" \t,|"));
        if (!*p)
            break;
        q = p + ACE_OS::strcspn(p, _T(" \t,|"));
        if (!q)
            q = p + ACE_OS::strlen(p);

        //  尝试转换成整数
        item = ACE_OS::strtoul(p, &t, 0);
        if (t == q) {
            result |= item;
            p = q;
            continue;
        }

        //  尝试查表识别字符串
        h = &Log_Util::priority_names_[0];
        while (*h) {
            if (ACE_OS::strncmp(*h, p, q-p) == 0) {
                break;
            }
            ++h;
        }
        if (*h != NULL) {
            item = Log_Util::priority_values_[h - &Log_Util::priority_names_[0]];
            result |= item;
            p = q;
            continue;
        }

        //  无法识别的项
        p = q;
        continue;
    }
    return result;
}


int Log_Util::last_error(void)
{
    return errno;
}

Log_Util::Log_Util(void)
: ace_log_(ACE_Log_Msg::instance ())
, current_module_(NULL)
, impl_(new Log_Impl())
{
    this->impl_->batched_priority_mask_ = 0;
    this->impl_->buffer_used_ = 0;
}

Log_Util::~Log_Util(void)
{
    delete impl_;
}

Log_Util* Log_Util::instance(void)
{
    return ACE_TSS_Singleton<Log_Util, ACE_Thread_Mutex>::instance();
}

int Log_Util::open(const char* prog_name, int flags, const char* logger_key)
{
    //ACE_Log_Msg::msg_backend(Log_Util_Smart_Backend_instance);
    //ACE_Log_Msg::msg_backend()->reset();
    //flags |= ACE_Log_Msg::CUSTOM;
    //
    //MODIFY_FLAGS_ALL(flags);

    int r = ACE_LOG_MSG->open(prog_name, flags, logger_key);
 
    ACE_LOG_MSG->set_flags(flags);
    ACE_LOG_MSG->priority_mask(default_priority_mask_);
#if 0
    if (!log_filename.empty()) {
        Log_Util_Smart_Backend::instance_.open(log_filename.c_str());
        Log_Util_Smart_Backend::instance_.flags_ = Log_Util::FILE_SS | Log_Util::STDERR;
    } else {
        Log_Util_Smart_Backend::instance_.flags_ = Log_Util::STDERR;
    }
#endif

    //////////////////////////////////////////////////////////////////////////
    module_dict_t& themap = Log_Util_Manager::instance()->module_map_;
    tstring modules;
    for (module_dict_t::iterator p = themap.begin(); !p.done(); ++p) {
        if (!modules.empty()) {
            modules += _T(", ");
        }
        modules += (*p).ext_id_.c_str();
    }
    ACE_DEBUG((LM_TRACE, _T("Log_Util::load_log_masks(): All Modules: %s\n"), modules.c_str() ));
    return r;
}

int Log_Util::load_log_masks(const ACE_TCHAR* text, const ACE_TCHAR* _prefix)
{
    Properties props;
    props.loadText(text);
    return this->load_log_masks(props, _prefix);
}

int Log_Util::load_log_masks(const Properties& properties, const ACE_TCHAR* _prefix)
{
    using namespace std;
    tstring prefix = _prefix;
    //tstring log_filename;
    tstring priority_mask_string;
    tstring module_default_priority_mask_string;
    u_long priority_mask = default_priority_mask_;
    u_long module_default_priority_mask = default_priority_mask_;
    int tracing = -1;
    int module_default_tracing = -1;
    int verbose = -1;
    const tstring all_modules_prefix = prefix + _T(".Modules.");
    const StringDict allmodules = properties.getPropertiesForPrefix(all_modules_prefix);

    //////////////////////////////////////////////////////////////////////////

    priority_mask_string = properties.getPropertyWithDefault(prefix + _T(".PriorityMask"), _T("*"));
    if (priority_mask_string != _T("*"))
        priority_mask = parse_log_priority_mask(priority_mask_string.c_str());

    tracing = properties.getPropertyAsIntWithDefault(prefix + _T(".Tracing"), -1);

    module_default_priority_mask_string = properties.getPropertyWithDefault(prefix + _T(".Modules.Default.PriorityMask"), _T("*"));
    if (module_default_priority_mask_string != _T("*"))
        module_default_priority_mask = parse_log_priority_mask(module_default_priority_mask_string.c_str());

    module_default_tracing = properties.getPropertyAsIntWithDefault(prefix + _T(".Modules.Default.Tracing"), -1);
    
    priority_mask_string = properties.getPropertyWithDefault(prefix + _T(".PriorityMask"), _T("*"));

    if (priority_mask_string != _T("*"))
        priority_mask = parse_log_priority_mask(priority_mask_string.c_str());

    tracing = properties.getPropertyAsIntWithDefault(prefix + _T(".Tracing"), -1);

    verbose = properties.getPropertyAsIntWithDefault(prefix + _T(".Verbose"), -1);

    //log_filename = properties.getProperty(prefix + _T(".LogFile"));

    //////////////////////////////////////////////////////////////////////////

    this->process_priority_mask(priority_mask);

    switch (tracing) {
        case 0:
            ACE_LOG_MSG->stop_tracing();
            break;
        case 1:
            ACE_LOG_MSG->start_tracing();
            break;
        default:
            break;
    }

    module_dict_t& themap = Log_Util_Manager::instance()->module_map_;
    
    if (module_default_priority_mask_string!= _T("*")) {
        for (module_dict_t::iterator p = themap.begin(); !p.done(); ++p) {
            (*p).int_id_->priority_mask(module_default_priority_mask);
        }
    }

    switch (module_default_tracing) {
        case 0:
            for (module_dict_t::iterator p = themap.begin(); !p.done(); ++p) {
                (*p).int_id_->stop_tracing();
            }
            break;
        case 1:
            for (module_dict_t::iterator p = themap.begin(); !p.done(); ++p) {
                (*p).int_id_->start_tracing();
            }
            break;
        default:
            break;
    }

    switch (verbose) {
        case 0:
            ACE_LOG_MSG->clr_flags(ACE_Log_Msg::VERBOSE | ACE_Log_Msg::VERBOSE_LITE);
            break;
        case 1:
            ACE_LOG_MSG->clr_flags(ACE_Log_Msg::VERBOSE | ACE_Log_Msg::VERBOSE_LITE);
            ACE_LOG_MSG->set_flags(ACE_Log_Msg::VERBOSE_LITE);
            break;
        case 2:
            ACE_LOG_MSG->clr_flags(ACE_Log_Msg::VERBOSE | ACE_Log_Msg::VERBOSE_LITE);
            ACE_LOG_MSG->set_flags(ACE_Log_Msg::VERBOSE);
            break;
        default:
            break;
    }

    //////////////////////////////////////////////////////////////////////////

    for (StringDict::const_iterator p = allmodules.begin(); p != allmodules.end(); ++p) {
        tstring key = p->first;
        size_t pos = key.find(_T('.'), all_modules_prefix.length());
        if (pos == tstring::npos)
            continue;
        tstring module_name = key.substr(all_modules_prefix.length(), pos - all_modules_prefix.length());
        tstring sub_key = key.substr(pos+1);

        if (module_name == _T("Default"))
            continue;

        Log_Module* module = find_module(module_name.c_str());
        if (module == NULL) {
            ACE_DEBUG((LM_WARNING, _T("Log_Util::init(): Unrecognized Module %s\n"), module_name.c_str()));
            continue;
        }

        if (sub_key == _T("Tracing")) {
            switch (ACE_OS::strtoul(p->second.c_str(), NULL, 0)) {
                case 0:
                    module->stop_tracing();
                    break;
                case 1:
                    module->start_tracing();
                    break;
            }
        } else if (sub_key == _T("PriorityMask")) {
            u_long priority_mask = parse_log_priority_mask(p->second.c_str());
            module->priority_mask(priority_mask);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    tstring modules;
    for (module_dict_t::iterator p = themap.begin(); !p.done(); ++p) {
        if (!modules.empty()) {
            modules += _T(", ");
        }
        modules += (*p).ext_id_.c_str();
    }
    ACE_DEBUG((LM_TRACE, _T("Log_Util::load_log_masks(): All Modules: %s\n"), modules.c_str() ));
    return 0;
}


/// 注册一个module
int Log_Util::register_module (Log_Module* module)
{
    return Log_Util_Manager::instance()->register_module(module);
}

/// 查找一个module
Log_Module* Log_Util::find_module (const ACE_TCHAR* name)
{
    return Log_Util_Manager::instance()->find_module(name);
}

ssize_t Log_Util::log (const Log_Module& module,
                            Log_Priority log_priority,
                            const ACE_TCHAR *format_str,
                            ...)
{
    va_list argp;
    va_start (argp, format_str);
    ssize_t result = this->vlog(&module, log_priority, format_str, argp);
    va_end (argp);
    return result;
}

ssize_t Log_Util::log (Log_Priority log_priority,
                            const ACE_TCHAR *format_str,
                            ...)
{
    va_list argp;
    va_start (argp, format_str);
    ssize_t result = this->vlog(this->current_module_, log_priority, format_str, argp);
    va_end (argp);
    return result;
}

ssize_t Log_Util::vlog (const Log_Module* module,
        Log_Priority log_priority,
        const ACE_TCHAR *format_str,
        va_list argp)
{
    static unsigned pid = (unsigned)getpid();

    /**
     * msg_callback is called before filter
     * custom_backend and other log system is called after filter.
     */

    // call msg_callback here
   
    if (!this->log_priority_enabled(module, log_priority)) {
            return 0;
    }

    current_module_ = module;

    //ssize_t result = Log_Impl::output_log(this, module, log_priority, format_str, argp);
    //current_module_ = NULL;
    //return result;

    struct timeval tv;
    gettimeofday(&tv, NULL);
    Log_Record lr(log_priority, tv, gettid());
    lr.module(module);
    
    char format2[MAX_FORMAT_LEN+MAX_LOGPREFIX_LEN];
    char* logbuf = NULL;
    int c;

    c = ss_format_log_preprintf(format2, tv, pid, lr.tid(), format_str, argp);
    if (c >= (int)MAX_FORMAT_LEN) {
        c = MAX_FORMAT_LEN - 1;
        format2[c] = 0;
    }
    if (format2[c-1] != '\n') {
        format2[c] = '\n';
        format2[++c] = 0;
    }

    //c = vasprintf(&logbuf, format2, argp);
    //lr.msg_data(logbuf, c, c+1);
    bool should_flash = false;
    {
        va_list argp_bak;
        va_copy(argp_bak, argp);
        size_t left = sizeof(this->impl_->buffer_) - this->impl_->buffer_used_;
        c = vsnprintf(&this->impl_->buffer_[this->impl_->buffer_used_], left,
                format2, argp);
        if ((size_t)c < left) {
            lr.msg_data(&this->impl_->buffer_[this->impl_->buffer_used_], c, 0);
            this->impl_->buffer_used_ += c+1;
        } else {
            c = vasprintf(&logbuf, format2, argp_bak);
            lr.msg_data(logbuf, c, c+1);
            should_flash = true;
        } 
        va_end(argp_bak);
    }

    // call custom backend here

    // store into batch buffer
    this->impl_->batch_buffer_.push_back(std::move(lr));

    should_flash = should_flash ||
        (this->impl_->batch_buffer_.size() >= MAX_BATCHD_LINES) ||
        !SS_BIT_ENABLED(this->impl_->batched_priority_mask_, log_priority);
    // check flush
    if (should_flash) {
        this->flush();
    }
    return 0;
}

///Get the process mask. 
u_long Log_Util::process_priority_mask ()
{
    return ACE_LOG_MSG->priority_mask(ACE_Log_Msg::PROCESS);
}
     
///Set the process mask, returns original mask. 
u_long Log_Util::process_priority_mask (u_long mask)
{
    return ACE_LOG_MSG->priority_mask(mask, ACE_Log_Msg::PROCESS);
}

///Get the module mask. 
u_long Log_Util::module_priority_mask (const Log_Module& module)
{
    return module.priority_mask_;
}
    
///Set the Log_Priority mask, returns original mask. 
u_long Log_Util::module_priority_mask (Log_Module& module, u_long mask)
{
    u_long old = module.priority_mask_;
    module.priority_mask_ = mask;
    return old;
}

///Return true if the requested priority is enabled. 
bool Log_Util::log_priority_enabled (const Log_Module* module, Log_Priority log_priority) const
{
    if (module && !SS_BIT_ENABLED(module->priority_mask_, log_priority))
        return false;
    return this->ace_log_->log_priority_enabled(log_priority);
}

///转发到ACE_Log_Msg
void Log_Util::conditional_set (const char *file, int line, int op_status, int errnum)
{
    this->ace_log_->conditional_set(file, line, op_status, errnum);
}

void Log_Util::set_flags(u_long f)
{
    this->ace_log_->set_flags(f);
}

void Log_Util::clr_flags(u_long f)
{
    this->ace_log_->clr_flags(f);
}

u_long Log_Util::flags()
{
    u_long f = this->ace_log_->flags();
    return f;
}

void Log_Util::msg_ostream(std::ostream* m, bool delete_ostream)
{
	this->ace_log_->msg_ostream(m, delete_ostream);
}

void Log_Util::msg_ostream(std::ostream* m)
{
	this->ace_log_->msg_ostream(m);
}

std::ostream *Log_Util::msg_ostream(void) const
{
	return this->ace_log_->msg_ostream();
}

int Log_Util::linenum(void) const
{
	return this->ace_log_->linenum();
}

const char* Log_Util::file(void) const
{
	return this->ace_log_->file();
}
    
/// set the mask of batch mode, if prio of one log is in mask,
/// it will be buffered until flush or some log out of mask coming.
/// default mask is 0
void Log_Util::batched_priority_mask(u_long mask)
{
    this->impl_->batched_priority_mask_ = mask;
}

/// return the mask of batched mode
u_long Log_Util::batched_priority_mask() const
{
    return this->impl_->batched_priority_mask_;
}

/// flush batched log of current thread
void Log_Util::flush()
{
    assert(this->impl_->batch_buffer_.size() <= MAX_BATCHD_LINES);
    u_long flags = this->flags();

    if (ACE_BIT_ENABLED(flags, ACE_Log_Msg::STDERR)) {
        char prefix_buffers[MAX_BATCHD_LINES][MAX_LOGPREFIX_LEN];
        struct iovec iovs[MAX_BATCHD_LINES*2];
        int iovs_count = 0;
        for (unsigned i = 0; i < this->impl_->batch_buffer_.size(); ++i)
        {
            auto& lr = this->impl_->batch_buffer_[i];
            int prefix_len = ss_format_log_prefix(&prefix_buffers[i][0], 
                    flags, lr.module(), lr.type(),
                    lr.time_stamp(), lr.tid());
            iovs[iovs_count].iov_base = &prefix_buffers[i][0];
            iovs[iovs_count].iov_len = prefix_len;
            iovs_count++;
            iovs[iovs_count].iov_base = (void*)lr.msg_data();
            iovs[iovs_count].iov_len = lr.msg_data_len();
            iovs_count++;
        }

        size_t buflen = 0;
        const struct iovec *p = iovs;
        int n = 0;

        for (int i = 0; i < iovs_count; i+=2, n+=2) // each log is 2 part
        {
            if (buflen + iovs[i].iov_len + iovs[i+1].iov_len >= PIPE_BUF)
            {
                int c = writev(fileno(stderr), p, n);
                (void)c;

                buflen = 0;
                p = &iovs[i];
                n = 0;
            }

            buflen += iovs[i].iov_len + iovs[i+1].iov_len;
        }

        if (buflen > 0)
        {
            int c = writev(fileno(stderr), p, n);
            (void)c;
        }
    }

    if (ACE_BIT_ENABLED(flags, ACE_Log_Msg::OSTREAM))
    {
        std::ostream *ostream = ACE_LOG_MSG->msg_ostream();
        if (ostream)
        {
            //pthread_mutex_lock(&os_mutex);
            ostream->flush();
            //pthread_mutex_unlock(&os_mutex);
        }
        char prefix_buffer[MAX_LOGPREFIX_LEN];
        for (unsigned i = 0; i < this->impl_->batch_buffer_.size(); ++i)
        {
            auto& lr = this->impl_->batch_buffer_[i];
            int prefix_len = ss_format_log_prefix(&prefix_buffer[0],
                    flags, lr.module(), lr.type(),
                    lr.time_stamp(), lr.tid());
            ostream->write(&prefix_buffer[0], prefix_len);
            ostream->write(lr.msg_data(), lr.msg_data_len());
        }
    }

    if (ACE_BIT_ENABLED(flags, ACE_Log_Msg::CUSTOM))
    { // call custom backend here
        //custom_backend->log(&this->impl_->batch_buffer_[0], this->impl_->batch_buffer_.size());
    }

    this->impl_->batch_buffer_.clear();
    this->impl_->buffer_used_ = 0;
}
} // namespace

namespace platform {

Log_Batch_Holder::Log_Batch_Holder (Log_Priority below)
	: my_batched_prio_mask_( ((below-1ul) | below) & ~(u_long) LM_SHUTDOWN )
	, saved_batched_prio_mask_(SS_LOG_UTIL->batched_priority_mask())
{
	SS_LOG_UTIL->batched_priority_mask(this->my_batched_prio_mask_);
}

Log_Batch_Holder::Log_Batch_Holder (u_long mask, bool)
	: my_batched_prio_mask_(mask)
	, saved_batched_prio_mask_(SS_LOG_UTIL->batched_priority_mask())
{
	SS_LOG_UTIL->batched_priority_mask(this->my_batched_prio_mask_);
}


Log_Batch_Holder::~Log_Batch_Holder ()
{
	SS_LOG_UTIL->batched_priority_mask(this->saved_batched_prio_mask_);
	SS_LOG_UTIL->flush();
}

void Log_Batch_Holder::flush ()
{
	SS_LOG_UTIL->flush();
}

} // namespace