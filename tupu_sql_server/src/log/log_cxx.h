#pragma once
// vim:set ts=4 sw=4:
/**
 * @file
 * @brief ��ģ��ͼ������log�Ŀ�
 * SS_DEBUG/SS_ERROR ϵ�к��������log����ʹ��֮ǰ������SS_LOG_MODULE_USE
 * ָ����ǰԴ�ļ�������logģ�顣logģ����SS_LOG_MODULE_DEF���塣\n
 * ���磺\n
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
 * // ������ĳ���������module
 * SS_LOG_MODULE_DEF(zipdb);
 * </pre>
 * ��һ�ַ������ڱ���ʱ��-Dָ��ȱʡ��logģ�飬�����Ͳ���дSS_LOG_MODULE_USE��
 * ����
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

//  ���¶�����һ�����log��macro
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
 * ĳЩʱ��Ϊ�����ܣ���Ҫ�ڻ���־Ȼ�������������ʱ��Ҫ�õ�SS_LOG_BATCH���ܡ�
 * ��ʾ������:
 * {
 *    SS_LOG_BATCH_BELOW(LM_DEBUG)
 *    for (...) {
 *      ...
 *      SS_DEBUG((LM_DEBUG, ...));
 *    }
 * }  // ��ʱ��һ������������log
 * 
 */

/// С�ڵ���BELOW����־�����ڻ���ӡ��һ���������ȼ����ߵ���־���߳�����ǰ������Χ�ͻ�һ�������
#define SS_LOG_BATCH_BELOW(BELOW) platform::Log_Batch_Holder ss_log_batch_holder(BELOW)
/// ������MASK����ס����־�����ڻ���ӡ��һ���������ȼ����ߵ���־���߳�����ǰ������Χ�ͻ�һ�������
#define SS_LOG_BATCH_MASK(MASK) platform::Log_Batch_Holder ss_log_batch_holder(MASK, true)
/// ǿ������ڻ�����־
#define SS_LOG_BATCH_FLUSH() ss_log_batch_holder.flush()

/// ��logģ�������Ҷ�Ӧ��logģ�����
#define SS_LOG_MODULE(name) SS_MACRO_JOIN(SSLM_, name)

/// ����һ��ģ��
#define SS_LOG_MODULE_DECL(name) \
    extern ::platform::Log_Module SS_LOG_MODULE(name)

/// ����һ��ģ��
#define SS_LOG_MODULE_DEF(name) \
    ::platform::Log_Module SS_LOG_MODULE(name)(::platform::Log_Util::instance(), ACE_TEXT( #name ), ::platform::Log_Util::default_priority_mask_)

/// ָ����ǰ�ļ�������ģ�飬��SS_LOG_DEFAULT_MODULE��ͻ
#define SS_LOG_MODULE_USE(name) \
    SS_LOG_MODULE_DECL(name); \
    static ::platform::Log_Module & __current_log_module = SS_LOG_MODULE(name)

// ����ȱʡlogģ��
#define SS_LOG_DEFAULT_MODULE_INTL __current_log_module

// ���-DSS_LOG_DEFAULT_MODUL�Ļ�������Ĭ��ģ��
#ifdef SS_LOG_DEFAULT_MODULE
SS_LOG_MODULE_USE(SS_LOG_DEFAULT_MODULE);
#endif

/// ȡSS_Log_Util�ĵ���
#define SS_LOG_UTIL (platform::Log_Util::instance())

//SS_LOG_MODULE_DECL(platform);

#define TEXT(X) X
#define SS_BIT_ENABLED(WORD, BIT) (((WORD) & (BIT)) != 0)

#define __SS_MACRO_JOIN_IMPL(A, B) A##B
// ��2����ʶ��ƴ��
#define SS_MACRO_JOIN(A, B) __SS_MACRO_JOIN_IMPL(A, B)
// ��3����ʶ��ƴ��
#define SS_MACRO_JOIN3(A, B, C) SS_MACRO_JOIN(A, SS_MACRO_JOIN(B,C))