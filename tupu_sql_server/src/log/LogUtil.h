#pragma once
// vim:set ts=4 sw=4 et:
/**
 *  \file log utilities
 */

#ifndef _LOG_UTIL_H_INCLUDED
#define _LOG_UTIL_H_INCLUDED

#include <cstdlib>
#include <cstdarg>
#include <ace/Copy_Disabled.h>
#include <ace/Log_Priority.h>
#include <iosfwd>
#include "properties.h"

ACE_BEGIN_VERSIONED_NAMESPACE_DECL
class ACE_INET_Addr;
class ACE_Log_Msg;
ACE_END_VERSIONED_NAMESPACE_DECL

/// 强制只从stderr输出的log优先级
#define LM_ONLYSTDERR (ACE_Log_Priority(LM_MAX << 1))

typedef unsigned long u_long;

namespace platform {

class Log_Util;
class Log_Module;
class Trace;
class Properties;
using std::ostream;

typedef ACE_Log_Priority Log_Priority;

/// 定义一个log模块
class Log_Module
    : ACE_Copy_Disabled
{
    friend class Log_Util;
    friend class Trace;
public:
    static const size_t MAXMODULENAMELEN = 32;

    /// 构造函数，注册到Log_Util
    Log_Module(Log_Util* log, const ACE_TCHAR* name, u_long mask);

    /// 取module的名字
    const ACE_TCHAR* module_name() const;

    ///Get the priority mask. 
    u_long priority_mask () const;
     
    ///Set the priority mask, returns original mask. 
    u_long priority_mask (u_long mask);

    void enable_debug_messages (Log_Priority priority=LM_DEBUG);
    void disable_debug_messages (Log_Priority priority=LM_DEBUG); 

    // = Control the tracing level.
    /// Determine if tracing is enabled (return == 1) or not (== 0)
    int is_tracing(void) const;

    /// Enable the tracing facility.
    void start_tracing (void);

    /// Disable the tracing facility.
    void stop_tracing (void);

    /// 是否输出详细的trace
    int verbose_tracing() const;

    /// 设置是否输出详细的trace
    int verbose_tracing(int v);

private:
    /// module名字
    const ACE_TCHAR* module_name_;
    /// module的log级别
    u_long priority_mask_;
    /// module是否允许trace
    int enable_tracing_;
    /// trace信息的详细程度
    int verbose_tracing_;

};

class Properties;

/// 在ACE_Log系统的基础上，提供对模块级别的log控制
class Log_Util
    : ACE_Copy_Disabled
{
    friend class Log_Impl;
public:
    //缺省的log级别
    static const u_long default_priority_mask_;
    //
    static const ACE_TCHAR* priority_names_[];
    //
    static const u_long priority_values_[];

    //@see ACE_Log_Msg
    enum {
        /// Write messages to stderr.
        STDERR = 1,
        /// Write messages to the local client logger deamon.
        LOGGER = 2,
        /// Write messages to the ostream * stored in thread-specific
        /// storage.
        OSTREAM = 4,
        /// Write messages to the callback object.
        MSG_CALLBACK = 8,
        /// Display messages in a verbose manner.
        VERBOSE = 16,
        /// Display messages in a less verbose manner (i.e., only print information that can change between calls).
        VERBOSE_LITE = 32,
        /// Do not print messages at all (just leave in thread-specific storage for later inspection).
        SILENT = 64,
        /// Write messages to the system's event log.
        SYSLOG = 128,
        /// Write messages to the user provided backend
        CUSTOM = 256,
        /// SS style verbose output
        VERBOSE_SS = 1 << 20,
    };
    
public:
    // 取当前线程的errno
    static int last_error();

    Log_Util(void);
    ~Log_Util(void);

    /// Returns a pointer to the Singleton.
    static Log_Util *instance (void);

    /// 初始化系统，读配置
    int open(const char* prog_name = 0, int flags = STDERR | VERBOSE_SS, const char* logger_key = 0);

    /**
     *  @brief 从配置项中加载
     *  @param properties       配置文件的内容
     *  @param prefix           指定以prefix开头的配置项是logutil的配置项
     *  配置文件的格式：
     *  1.指定某个模块的log级别，参见LM_...系列参数
     *  xxx.Modules.<ModuleName>.PriorityMask=TRACE,DEBUG,INFO,...
     *  2.指定某个模块是否允许trace
     *  xxx.Modules.<ModuleName>.Tracing=1
     *  xxx.Modules.<ModuleName>.Tracing=0
     *  3.指定模块默认的log级别和tracing
     *  xxx.Modules.Default.PriorityMask=...
     *  xxx.Modules.Default.Tracing=...
     *  4.指定全局的log级别和tracing
     *  xxx.PriorityMask=...
     *  xxx.Tracing=...
     *  5.指定log文件
     *  xxx.LogFile=...
     *  6.指定是否Verbose (0,1,2 分别对应NONE, VERBOSE_LITE, VERBOSE)
     *  xxx.Verbose=...
     *  @{
     */
     int load_log_masks(const Properties& properties, const ACE_TCHAR* prefix = ACE_TEXT("Log"));
     int load_log_masks(const ACE_TCHAR* configtext, const ACE_TCHAR* prefix = ACE_TEXT("Log"));
     ///@}

    /**
     * @brief Format a message to the thread-safe ACE logging mechanism.
     * @param module        模块，省略时，为当前源文件的默认模块
     * @param priority      log级别
     * @param format        log格式文本
     * @param arpg          vprintf格式的参数
     * @see ACE_Log_Msg::log
     * @{
     */
    ssize_t log (const Log_Module& module,
                Log_Priority priority,
                const ACE_TCHAR *format,
                ...);

    ssize_t log (Log_Priority priority,
                const ACE_TCHAR *format,
                ...);
    ssize_t vlog (const Log_Module* module,
                Log_Priority priority,
                const ACE_TCHAR *format,
                va_list argp);
    //@}

    ///Get the process mask. 
    static u_long process_priority_mask ();
     
    ///Set the process mask, returns original mask. 
    static u_long process_priority_mask (u_long mask);

    ///Get the module mask. 
    static u_long module_priority_mask (const Log_Module& module);
     
    ///Set the process mask, returns original mask. 
    static u_long module_priority_mask (Log_Module& module, u_long mask);

    ///Return true if the requested priority is enabled. 
    bool log_priority_enabled (const Log_Module* module, Log_Priority log_priority) const;

    ///设置当前module
    void current_module(const Log_Module& module)
    { this->current_module_ = &module; }

    const Log_Module* current_module() const
    { return this->current_module_; }

    ///取内部的logger
    ACE_Log_Msg* ace_log_msg()
    { return this->ace_log_; }

    ///转发到ACE_Log_Msg
    void conditional_set (const char *file, int line, int op_status, int errnum);

    /// 注册一个module
    static int register_module (Log_Module* module);

    /// 查找一个module
    static Log_Module* find_module (const ACE_TCHAR* name);

    /// set the mask of batch mode, if prio of one log is in mask,
    /// it will be buffered until flush or some log out of mask coming.
    /// default mask is 0
    void batched_priority_mask(u_long);

    /// return the mask of batched mode
    u_long batched_priority_mask() const;

    /// flush batched log of current thread
    void flush(); 

    /**
     * Enable the bits in the logger's options flags.
     */
    void set_flags (u_long f);

    /**
     * Disable the bits in the logger's options flags.
     */
    void clr_flags (u_long f);

    /**
     * Return the bits in the logger's options flags.
     */
    u_long flags (void);

    // = Notice that the following two function is equivalent to
    //   "void msg_ostream (HANDLE)" and "HANDLE msg_ostream (void)"
    //   on Windows CE.  There is no <iostream.h> support on CE.

    /// Update the ostream without overwriting the delete_ostream_ flag.
    void msg_ostream (ACE_OSTREAM_TYPE *);

    /**
     * delete_stream == 1, forces Log_Msg.h to delete the stream in
     * its own ~dtor (assumes control of the stream)
     * use only with proper ostream (eg: fstream), not (cout, cerr)
     */
    void msg_ostream (ACE_OSTREAM_TYPE *, bool delete_ostream);

    /// Get the ostream that is used to print error messages.
    ACE_OSTREAM_TYPE *msg_ostream (void) const;
private:
    ACE_Log_Msg* ace_log_;
    const Log_Module* current_module_;
    class Log_Impl* impl_;

    int linenum () const;
    const char* file () const;

private:
#ifdef ACE_LOG_MSG
    /**
     *  This function is disabled and placed here to prevent the following incorrect usage from being compiled.
     *   DO NOT Call  process_priority_mask(ACE_Log_Msg::PROCESS)
     *   Just call  process_priority_mask() to get mask
     *   Or call  process_priority_mask(LM_INFO|LM_DEBUG|...) to set mask.
     */
    static u_long process_priority_mask (ACE_Log_Msg::MASK_TYPE);
#endif
}; // class Log_Util

/**
 * @class Trace
 *
 * @brief A C++ trace facility that keeps track of which methods are
 * entered and exited.
 *
 * This class is a extension to ACE_Trace. Added module support.
 */
class Trace
{
public:
    // = Initialization and termination methods.

    /// Perform the first part of the trace, which prints out the string
    /// N, the LINE, and the ACE_FILE as the function is entered.
    Trace (
        const Log_Module& module,
        const ACE_TCHAR *n,
        int line = 0,
        const ACE_TCHAR *file = ACE_TEXT (""));

    /// Perform the second part of the trace, which prints out the NAME
    /// as the function is exited.
    ~Trace (void);

    // = Control the tracing level.
    /// Determine if tracing is enabled (return == 1) or not (== 0)
    static int  is_tracing(void);

    /// Enable the tracing facility.
    static void start_tracing (void);

    /// Disable the tracing facility.
    static void stop_tracing (void);

    /// Change the nesting indentation level.
    static void set_nesting_indent (int indent);

    /// Get the nesting indentation level.
    static int get_nesting_indent (void);

private:
    /// Name of the method we are in.
    const ACE_TCHAR *name_;
    const Log_Module& module_;
};

// 用于囤积log批量输出的辅助类
class Log_Batch_Holder
{
	void* operator new (size_t) = delete;
	Log_Batch_Holder (const Log_Batch_Holder&) = delete;
	Log_Batch_Holder& operator= (const Log_Batch_Holder&) = delete;

public:
	Log_Batch_Holder (Log_Priority below);
	Log_Batch_Holder (u_long mask, bool = true);
	~Log_Batch_Holder ();
	void flush();
private:
	u_long my_batched_prio_mask_;
	u_long saved_batched_prio_mask_;
	void* extra1_;
	void* extra2_;
};

} // namespace

#endif //_LOG_UTIL_H_INCLUDED

