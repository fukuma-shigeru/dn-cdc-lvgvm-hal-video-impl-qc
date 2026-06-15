/*******************************************************************************
    機能名称    ：  Wayland描画 DLTログモジュール
    ファイル名称：  wl_renderer_log.h
*******************************************************************************/
#ifndef	WL_RENDERER_LOG_H
#define	WL_RENDERER_LOG_H

#include <dlt/dlt.h>
#include <cstring>
#include <cstdarg>

#define WLRNDR_LOGF(format, args...)		WlRendererLogOut(DLT_LOG_FATAL, __builtin_FUNCTION(), __LINE__, (format), ## args)
#define WLRNDR_LOGE(format, args...)		WlRendererLogOut(DLT_LOG_ERROR, __builtin_FUNCTION(), __LINE__, (format), ## args)
#define WLRNDR_LOGW(format, args...)		WlRendererLogOut(DLT_LOG_WARN, __builtin_FUNCTION(), __LINE__, (format), ## args)
#define WLRNDR_LOGI(format, args...)		WlRendererLogOut(DLT_LOG_INFO, __builtin_FUNCTION(), __LINE__, (format), ## args)
#define WLRNDR_LOGD(format, args...)		WlRendererLogOut(DLT_LOG_DEBUG, __builtin_FUNCTION(), __LINE__, (format), ## args)
#define WLRNDR_LOGV(format, args...)		WlRendererLogOut(DLT_LOG_VERBOSE, __builtin_FUNCTION(), __LINE__, (format), ## args)

namespace wlrenderer
{
void WlRendererLogInit(void);
void WlRendererLogOut(const DltLogLevelType loglevel, const char* const pName, const int pline, const char *format, ...);
void WlRendererLogEnd(void);
} /* namespace wlrenderer */

#endif	/* #ifndef	WL_RENDERER_LOG_H */

