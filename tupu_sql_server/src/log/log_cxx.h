#pragma once
// vim:set ts=4 sw=4:
/**
 * @file
 * @brief 分模块和级别输出log的库
 * SS_DEBUG/SS_ERROR 系列宏用于输出log。在使用之前必须用SS_LOG_MODULE_USE
 * 指定当前源文件所属的log模块。log模块用SS_LOG_MODULE_DEF定义。\n
 * 例如：\n
 * <pre>
 * // ZipDB.cpp
 * #include <Platform/log.h>
 *
 * SS_LOG_MODULE_USE(zipdb);
 *
 * int open_zipdb() {
 *   SS_DEBUG((LM_DEBUG, "Hello! %s\n", "WORLD"))
 *   return 0;
 * }
 * // 必须在某处定义这个module
 * SS_LOG_MODULE_DEF(zipdb);
 * </pre>
 * 另一种方法是在编译时用-D指定缺省的log模块，这样就不用写SS_LOG_MODULE_USE了
 * 例如
 * <pre>
 * g++ -c -DSS_LOG_DEFAULT_MODULE=zipdb zipdb.cpp
 * </pre>
 */

namespace platform {
	class Log_Util;
	class Log_Module;
	class Trace;
} // namespace platform

#include "LogUtil.h"

#define SS_ERROR_RETURN(X, Y) do { SS_ERROR(X); return Y; } while(0)
#define SS_ERROR_BREAK(X) if (1) { SS_ERROR(X); break; } else

//  以下定义了一组输出log的macro
#if !defined (SS_NLOGGING)

#define SS_ASSERT(X) ((X) \
		   ? static_cast<void>(0) \
		   : __ss_assert_fail(__FILE__, __LINE__, #X))

/// Usage: SS_DEBUGX((module, log_priority, format_str, ...));
#define SS_DEBUGX(X) \
  do { \
    int __ace_error = ::platform::Log_Util::last_error (); \
    ::platform::Log_Util *glu___ = ::platform::Log_Util::instance (); \
    glu___->conditional_set (__FILE__, __LINE__, 0, __ace_error); \
    glu___->log X; \
  } while (0)
/// Usage: SS_ERRORX((module, log_priority, format_str, ...));
#define SS_ERRORX(X) \
  do { \
    int __ace_error = ::platform::Log_Util::last_error (); \
    ::platform::Log_Util *glu___ = ::platform::Log_Util::instance (); \
    glu___->conditional_set (__FILE__, __LINE__, -1, __ace_error); \
    glu___->log X; \
  } while (0)

/// Usage: SS_DEBUG((log_priority, format_str, ...));
#define SS_DEBUG(X) \
  do { \
    int __ace_error = ::platform::Log_Util::last_error (); \
    ::platform::Log_Util *glu___ = ::platform::Log_Util::instance (); \
    glu___->current_module (SS_LOG_DEFAULT_MODULE_INTL); \
    glu___->conditional_set (__FILE__, __LINE__, 0, __ace_error); \
    glu___->log X; \
  } while (0)
/// Usage: SS_ERROR((log_priority, format_str, ...));
#define SS_ERROR(X) \
  do { \
    int __ace_error = ::platform::Log_Util::last_error (); \
    ::platform::Log_Util *glu___ = ::platform::Log_Util::instance (); \
    glu___->current_module (SS_LOG_DEFAULT_MODULE_INTL); \
    glu___->conditional_set (__FILE__, __LINE__, -1, __ace_error); \
    glu___->log X; \
  } while (0)
#endif // SS_NLOGGING

/**
 * 某些时候，为了性能，需要囤积日志然后批量输出，此时需要用到SS_LOG_BATCH功能。
 * 简单示例如下:
 * {
 *    SS_LOG_BATCH_BELOW(LM_DEBUG)
 *    for (...) {
 *      ...
 *      SS_DEBUG((LM_DEBUG, ...));
 *    }
 * }  // 此时将一次性输出上面的log
 * 
 */

/// 小于等于BELOW的日志将被囤积打印，一旦遇到优先级更高的日志或者超出当前函数范围就会一次性输出
#define SS_LOG_BATCH_BELOW(BELOW) platform::Log_Batch_Holder ss_log_batch_holder(BELOW)
/// 被掩码MASK覆盖住的日志将被囤积打印，一旦遇到优先级更高的日志或者超出当前函数范围就会一次性输出
#define SS_LOG_BATCH_MASK(MASK) platform::Log_Batch_Holder ss_log_batch_holder(MASK, true)
/// 强行输出囤积的日志
#define SS_LOG_BATCH_FLUSH() ss_log_batch_holder.flush()

/// 从log模块名字找对应的log模块对象
#define SS_LOG_MODULE(name) SS_MACRO_JOIN(SSLM_, name)

/// 声明一个模块
#define SS_LOG_MODULE_DECL(name) \
    extern ::platform::Log_Module SS_LOG_MODULE(name)

/// 定义一个模块
#define SS_LOG_MODULE_DEF(name) \
    ::platform::Log_Module SS_LOG_MODULE(name)(::platform::Log_Util::instance(), ACE_TEXT( #name ), ::platform::Log_Util::default_priority_mask_)

/// 指定当前文件所属的模块，与SS_LOG_DEFAULT_MODULE冲突
#define SS_LOG_MODULE_USE(name) \
    SS_LOG_MODULE_DECL(name); \
    static ::platform::Log_Module & __current_log_module = SS_LOG_MODULE(name)

// 定义缺省log模块
#define SS_LOG_DEFAULT_MODULE_INTL __current_log_module

// 如果-DSS_LOG_DEFAULT_MODUL的话，设置默认模块
#ifdef SS_LOG_DEFAULT_MODULE
SS_LOG_MODULE_USE(SS_LOG_DEFAULT_MODULE);
#endif

/// 取SS_Log_Util的单体
#define SS_LOG_UTIL (platform::Log_Util::instance())

//SS_LOG_MODULE_DECL(platform);

#define TEXT(X) X
#define SS_BIT_ENABLED(WORD, BIT) (((WORD) & (BIT)) != 0)

#define __SS_MACRO_JOIN_IMPL(A, B) A##B
// 将2个标识符拼接
#define SS_MACRO_JOIN(A, B) __SS_MACRO_JOIN_IMPL(A, B)
// 将3个标识符拼接
#define SS_MACRO_JOIN3(A, B, C) SS_MACRO_JOIN(A, SS_MACRO_JOIN(B,C))