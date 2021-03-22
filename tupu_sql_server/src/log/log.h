#pragma once

/**
 * SS_LOG logϵͳ���
 *
 * ��Ϊ��ʷԭ��ssplatform5֮ǰ�İ汾��־�����ACE����־�������ϵ�
 * ��ssplatform5��ʼ��������ȫ�����ϵ
 *
 * ��������Ҫʹ��SS_DEBUG�������־��
 *     SS_DEBUG((prio, "format_str", other_args...))
 * ���prio��log�ļ���format_str������printf��ʽ���ַ���
 * log������Ҫ���⼸������Ҫ�̶��������
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
 * SS_LOG�������־ǰ����Ҫ��һ���£�ѡ��ǰԴ�ļ���������־ģ��
 * ����һ��ģ��:
 *   SS_LOG_MODULE_DEF(ģ����)
 * ѡ��ǰԴ�ļ�������ģ��:
 *   SS_LOG_MODULE_USE(ģ����)
 * ���һ��ģ����������.cpp�ļ�����ô���ҽ���һ��.cpp�ļ�����SS_LOG_MODULE_DEF��䣬
 * ͬʱ��ģ���ÿ��.cpp�ļ���Ӧ�ð���SS_LOG_MODULE_USE��䡣
 * ע�⣺��Ҫ��ͷ�ļ���ʹ��SS_LOG_MODULE_DEF��SS_LOG_MODULE_USE
 * �����һ��Ҫ��ͷ�ļ������־����ʹ��SS_LOG_MODULE_DECL����ģ�飬��ʹ��SS_DEBUGX��
 * ��������־��SS_DEBUGX��SS_DEBUG����1������(��1��)����ָ��������־��ʹ�õ�ģ�顣
 * 
 * �������ù���
 * SS_LOG_UTIL �귵�ص�ǰ�߳�Log_Utilʵ��ָ�롣
 *   process_priority_mask �趨�ͻ�ȡ���̷�Χ��log�����������
 *   module_priority_mask �趨�ͻ�ȡģ�鷶Χ��log�����������
 *   load_log_masks ���ⲿ�����ļ������ַ�������log�����������
 *   set_flags/clr_flags �趨һЩlog������Ʋ���
 *      ���ò���VERBOSE VERBOSE_LITE VERBOSE_SS
 * 
 * ��ʵ�õķ����μ�log/mtlogtest.cpp
 *
 */

/**
 * ��־�����ɸѡ
 * ��־�����ɸѡ�� ���̼����̼߳���ģ�鼶 �������빲ͬ����
 * ��ʽ���£�
 *   ��־���� & ( ���̼����� | �̼߳����� ) & ģ�鼶����
 *   ��������0�����������־
 * Ĭ��ֵ��
 *   ���̼����� = ALL
 *   �̼߳����� = 0
 *   ģ�鼶���� = ALL
 */

/**
 * SS_LOG ��־��ʽ����
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

// ���¶�����һ�����log��macro������SS_NLOGGINGʱ����log���
// һ������²�����ʹ��SS_NLOGGING����log������ͨ�������������
// ���磺SS_LOG_UTIL->process_priority_mask(0) ���Խ�ֹ����log���

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

