/*******************************************************************************
    機能名称    ：  VideoHAL(Server) Log wrapperモジュール
    ファイル名称：  vhal_log_wrapper.h
*******************************************************************************/
#ifndef	VHAL_LOG_WRAPPER_H
#define	VHAL_LOG_WRAPPER_H

#include <string.h>
#include <stdarg.h>
#include <dlt/dlt.h>

void VhalLogOut_wrapper(const DltLogLevelType loglevel, const char* const pName, const int pline, const char *format, ...);

#define VHAL_LOGF(format, args...)		VhalLogOut_wrapper(DLT_LOG_FATAL, __builtin_FUNCTION(), __LINE__, (format), ## args)
#define VHAL_LOGE(format, args...)		VhalLogOut_wrapper(DLT_LOG_ERROR, __builtin_FUNCTION(), __LINE__, (format), ## args)
#define VHAL_LOGW(format, args...)		VhalLogOut_wrapper(DLT_LOG_WARN, __builtin_FUNCTION(), __LINE__, (format), ## args)
#define VHAL_LOGI(format, args...)		VhalLogOut_wrapper(DLT_LOG_INFO, __builtin_FUNCTION(), __LINE__, (format), ## args)
#define VHAL_LOGD(format, args...)		VhalLogOut_wrapper(DLT_LOG_DEBUG, __builtin_FUNCTION(), __LINE__, (format), ## args)
#define VHAL_LOGV(format, args...)		VhalLogOut_wrapper(DLT_LOG_VERBOSE, __builtin_FUNCTION(), __LINE__, (format), ## args)

#endif	/* #ifndef	VHAL_LOG_WRAPPER_H */

