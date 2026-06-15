/*******************************************************************************
    機能名称    ：  VideoHAL(Server) DLTログモジュール
    ファイル名称：  vhal_log.h
*******************************************************************************/
#ifndef	VHAL_LOG_H
#define	VHAL_LOG_H

#include <dlt/dlt.h>
#include <cstring>
#include <cstdarg>

#define VHAL_LOGF(format, args...)		videohal::VhalLogOut(DLT_LOG_FATAL, __builtin_FUNCTION(), __LINE__, (format), ## args)
#define VHAL_LOGE(format, args...)		videohal::VhalLogOut(DLT_LOG_ERROR, __builtin_FUNCTION(), __LINE__, (format), ## args)
#define VHAL_LOGW(format, args...)		videohal::VhalLogOut(DLT_LOG_WARN, __builtin_FUNCTION(), __LINE__, (format), ## args)
#define VHAL_LOGI(format, args...)		videohal::VhalLogOut(DLT_LOG_INFO, __builtin_FUNCTION(), __LINE__, (format), ## args)
#define VHAL_LOGD(format, args...)		videohal::VhalLogOut(DLT_LOG_DEBUG, __builtin_FUNCTION(), __LINE__, (format), ## args)
#define VHAL_LOGV(format, args...)		videohal::VhalLogOut(DLT_LOG_VERBOSE, __builtin_FUNCTION(), __LINE__, (format), ## args)
#define VHAL_LOGV_IN(format, args...)	videohal::VhalLogOut(DLT_LOG_VERBOSE, __builtin_FUNCTION(), __LINE__, ("[IN ]" format), ## args)
#define VHAL_LOGV_OUT(format, args...)	videohal::VhalLogOut(DLT_LOG_VERBOSE, __builtin_FUNCTION(), __LINE__, ("[OUT]" format), ## args)

/* 通常時はERROR、サスペンド状態中のみWARNレベルとしてログ出力を実施する。 */
/* ログレベルにDltLogLevelType型以外を使用するとCoverityやMISRAにて警告が発生するためDLT_LOG_DEFAULTを用いているが */
/* DLT_LOG_DEFAULT指定時は内部でERROR/WARNに変換して出力する。 */
#define VHAL_LOGEW(format, args...)		videohal::VhalLogOut(DLT_LOG_DEFAULT, __builtin_FUNCTION(), __LINE__, (format), ## args)

namespace videohal
{
void VhalLogInit(void) noexcept;
void VhalLogOut(const DltLogLevelType loglevel, const char* const pName, const int pline, const char *format, ...);
void VhalLogEnd(void) noexcept;
void VhalLogCameraMute(const char* const pName, const bool mute);
void VhalLogCameraPath(const char* const pName, const bool camera);
}

extern "C" void VhalLogOut_wrapper(const DltLogLevelType loglevel, const char* const pName, const int pline, const char *format, ...);
#endif	/* #ifndef	VHAL_LOG_H */

