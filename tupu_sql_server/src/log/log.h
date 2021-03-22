#pragma once

/**
 * SS_LOG log系统简介
 *
 * 因为历史原因，ssplatform5之前的版本日志输出和ACE的日志输出是耦合的
 * 自ssplatform5开始，两者完全脱离关系
 *
 * 程序中主要使用SS_DEBUG宏输出日志。
 *     SS_DEBUG((prio, "format_str", other_args...))
 * 这里，prio是log的级别，format_str是类似printf格式的字符串
 * log级别主要有这几级，重要程度依次提高
 *   LM_TRACE
 *   LM_DEBUG
 *   LM_INFO
 *   LM_NOTICE
 *   LM_WARNING
 *   LM_ERROR
 *   LM_CRITICAL
 *   LM_ALERT
 *   LM_EMERGENCY
 * 
 * SS_LOG在输出日志前还需要做一件事，选择当前源文件所属的日志模块
 * 定义一个模块:
 *   SS_LOG_MODULE_DEF(模块名)
 * 选择当前源文件所属的模块:
 *   SS_LOG_MODULE_USE(模块名)
 * 如果一个模块里包含多个.cpp文件，那么有且仅有一个.cpp文件包含SS_LOG_MODULE_DEF语句，
 * 同时该模块的每个.cpp文件都应该包含SS_LOG_MODULE_USE语句。
 * 注意：不要在头文件中使用SS_LOG_MODULE_DEF和SS_LOG_MODULE_USE
 * 如果你一定要在头文件输出日志，请使用SS_LOG_MODULE_DECL申明模块，并使用SS_DEBUGX等
 * 语句输出日志。SS_DEBUGX比SS_DEBUG多了1个参数(第1个)用来指出这行日志所使用的模块。
 * 
 * 其他常用功能
 * SS_LOG_UTIL 宏返回当前线程Log_Util实例指针。
 *   process_priority_mask 设定和获取进程范围的log输出级别掩码
 *   module_priority_mask 设定和获取模块范围的log输出级别掩码
 *   load_log_masks 从外部配置文件或者字符串加载log输出级别掩码
 *   set_flags/clr_flags 设定一些log输出控制参数
 *      常用参数VERBOSE VERBOSE_LITE VERBOSE_SS
 * 
 * 简单实用的范例参见log/mtlogtest.cpp
 *
 */

/**
 * 日志的输出筛选
 * 日志的输出筛选由 进程级、线程级、模块级 三组掩码共同控制
 * 公式如下：
 *   日志级别 & ( 进程级掩码 | 线程级掩码 ) & 模块级掩码
 *   如果结果非0，则输出该日志
 * 默认值：
 *   进程级掩码 = ALL
 *   线程级掩码 = 0
 *   模块级掩码 = ALL
 */

/**
 * SS_LOG 日志格式介绍
 * SS_LOG system format log message like "printf", but with extra 
 * substitutions (prefixed by '%', as in printf format strings):
 *  - 'P': print out the current process id
 *  - '@': print a void* pointer (in hexadecimal)
 *  - 'T': print timestamp in hour:minute:sec:usec format (plain option,
 *         i.e. without any flags, prints system supplied timestamp;
 *         with '#' flag added expects ACE_Time_Value* in argument list)
 *  - 'D': print timestamp as Weekday Month day year hour:minute:sec.usec
 *         (plain option, i.e. without any flags, prints system supplied
 *         timestamp; with '#' flag added expects ACE_Time_Value* in
 *         argument list)
 *  - 't': print thread id (1 if single-threaded)i
 *  - ... see printf of libc
 */

#include "log_cxx.h"

// 以下定义了一组输出log的macro，用于SS_NLOGGING时屏蔽log输出
// 一般情况下不建议使用SS_NLOGGING屏蔽log，而是通过程序参数控制
// 比如：SS_LOG_UTIL->process_priority_mask(0) 可以禁止所有log输出

#if defined (SS_NLOGGING)

#define SS_ASSERT(X) assert(X)

#define SS_HEX_DUMPX(X) do {} while (0)
#define SS_ERRORX(X) do {} while (0)
#define SS_DEBUGX(X) do {} while (0)
#define SS_TRACEX(M, T) do {} while (0)

#define SS_HEX_DUMP(X) do {} while (0)
#define SS_ERROR(X) do {} while (0)
#define SS_DEBUG(X) do {} while (0)
#define SS_TRACE(T) do {} while (0)

#endif

