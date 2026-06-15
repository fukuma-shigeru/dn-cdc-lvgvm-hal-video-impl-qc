/*******************************************************************************
    Function name : VideoHAL client DLT log module
    File name     : vhal_log.h
*******************************************************************************/
#ifndef	VHAL_LOG_H
#define	VHAL_LOG_H

#include <string>
#include <cstdarg>

#include "cockpit_log.hpp"

#define VHAL_DLT_PREFIX	"[VHALC]"

/* Print logs to stdout. Comment out the next line when this module is released */
//#define VHAL_LOG_LOCAL_SETTING

#ifdef VHAL_LOG_LOCAL_SETTING

#define VHAL_LOG_LEVEL_OFF		(0)
#define VHAL_LOG_LEVEL_FATAL	(1)
#define VHAL_LOG_LEVEL_ERROR	(2)
#define VHAL_LOG_LEVEL_WARN		(3)
#define VHAL_LOG_LEVEL_INFO		(4)
#define VHAL_LOG_LEVEL_DEBUG	(5)
#define VHAL_LOG_LEVEL_VERBOSE	(6)

#define VHAL_LOG_LEVEL		VHAL_LOG_LEVEL_VERBOSE

#if ((VHAL_LOG_LEVEL >= VHAL_LOG_LEVEL_FATAL) || VHAL_FORCE_ENABLE_LOGF)
#define VHAL_LOGF(format, ...)		fprintf(stderr, "[F]" VHAL_DLT_PREFIX " %s(%d)" format "\n",__builtin_FUNCTION(),__LINE__,##__VA_ARGS__);
#else 
#define VHAL_LOGF(...)
#endif

#if ((VHAL_LOG_LEVEL >= VHAL_LOG_LEVEL_ERROR) || VHAL_FORCE_ENABLE_LOGE)
#define VHAL_LOGE(format, ...)	 	fprintf(stderr, "[E]" VHAL_DLT_PREFIX " %s(%d)" format "\n",__builtin_FUNCTION(),__LINE__,##__VA_ARGS__);
#else 
#define VHAL_LOGE(...)
#endif

#if ((VHAL_LOG_LEVEL >= VHAL_LOG_LEVEL_WARN) || VHAL_FORCE_ENABLE_LOGW)
#define VHAL_LOGW(format, ...)		fprintf(stderr, "[W]" VHAL_DLT_PREFIX " %s(%d) " format "\n",__builtin_FUNCTION(),__LINE__,##__VA_ARGS__);
#else 
#define VHAL_LOGW(...)
#endif

#if ((VHAL_LOG_LEVEL >= VHAL_LOG_LEVEL_INFO) || VHAL_FORCE_ENABLE_LOGI)
#define VHAL_LOGI(format, ...)		fprintf(stderr, "[I]" VHAL_DLT_PREFIX " %s(%d) " format "\n",__builtin_FUNCTION(),__LINE__,##__VA_ARGS__);
#else 
#define VHAL_LOGI(...)
#endif

#if ((VHAL_LOG_LEVEL >= VHAL_LOG_LEVEL_DEBUG) || VHAL_FORCE_ENABLE_LOGD)
#define VHAL_LOGD(format, ...)		fprintf(stderr, "[D]" VHAL_DLT_PREFIX " %s(%d) " format "\n",__builtin_FUNCTION(),__LINE__,##__VA_ARGS__);
#else 
#define VHAL_LOGD(...)
#endif

#if ((VHAL_LOG_LEVEL >= VHAL_LOG_LEVEL_VERBOSE) || VHAL_FORCE_ENABLE_LOGV)
#define VHAL_LOGV(format, ...)		fprintf(stderr, "[V]" VHAL_DLT_PREFIX " %s(%d) " format "\n", __builtin_FUNCTION(), __LINE__, ##__VA_ARGS__)
#define VHAL_LOGV_IN(format, ...)	fprintf(stderr, "[V]" VHAL_DLT_PREFIX "[IN ] %s:%s(%d) " format "\n", __FILE__, __builtin_FUNCTION(), __LINE__, ##__VA_ARGS__);
#define VHAL_LOGV_OUT(format, ...)	fprintf(stderr, "[V]" VHAL_DLT_PREFIX "[OUT] %s:%s(%d) " format "\n", __FILE__, __builtin_FUNCTION(), __LINE__, ##__VA_ARGS__);
#else 
#define VHAL_LOGV(...)
#define VHAL_LOGV_IN(...)
#define VHAL_LOGV_OUT(...)
#endif

#else /* VHAL_LOG_LOCAL_SETTING */

#define VHAL_LOGF(format, ...)		cockpit::hal::video_hal::LogOut(COCKPIT_LOG_FATAL, VHAL_DLT_PREFIX"[F]", __builtin_FUNCTION(), __LINE__, format, ##__VA_ARGS__)
#define VHAL_LOGE(format, ...)		cockpit::hal::video_hal::LogOut(COCKPIT_LOG_ERROR, VHAL_DLT_PREFIX"[E]",  __builtin_FUNCTION(), __LINE__, format, ##__VA_ARGS__)
#define VHAL_LOGW(format, ...)		cockpit::hal::video_hal::LogOut(COCKPIT_LOG_WARN, VHAL_DLT_PREFIX"[W]",  __builtin_FUNCTION(), __LINE__, format, ##__VA_ARGS__)
#define VHAL_LOGI(format, ...)		cockpit::hal::video_hal::LogOut(COCKPIT_LOG_INFO, VHAL_DLT_PREFIX"[I]",  __builtin_FUNCTION(), __LINE__, format, ##__VA_ARGS__)
#define VHAL_LOGD(format, ...)		cockpit::hal::video_hal::LogOut(COCKPIT_LOG_DEBUG, VHAL_DLT_PREFIX"[D]", __builtin_FUNCTION(), __LINE__, format, ##__VA_ARGS__)
#define VHAL_LOGV(format, ...)		cockpit::hal::video_hal::LogOut(COCKPIT_LOG_VERBOSE, VHAL_DLT_PREFIX"[V]",  __builtin_FUNCTION(), __LINE__, format, ##__VA_ARGS__)
#define VHAL_LOGV_IN(format, ...)	cockpit::hal::video_hal::LogOut(COCKPIT_LOG_VERBOSE, VHAL_DLT_PREFIX"[V]",  __builtin_FUNCTION(), __LINE__, "[IN ]" format, ##__VA_ARGS__)
#define VHAL_LOGV_OUT(format, ...)	cockpit::hal::video_hal::LogOut(COCKPIT_LOG_VERBOSE, VHAL_DLT_PREFIX"[V]",  __builtin_FUNCTION(), __LINE__, "[OUT]" format, ##__VA_ARGS__)

namespace cockpit {
namespace hal {
namespace video_hal {
void LogInit(void);
void LogOut(const CockpitLogLevelType loglevel, const char* const pprefix, const char* const pName, const int32_t pline, const char *format, ...);
void LogEnd(void);

inline static void VHAL_LOG_INIT(void) { LogInit(); }
inline static void VHAL_LOG_END(void) { LogEnd(); }

}  /* namespace video_hal */
}  /* namespace hal */
}  /* namespace cockpit */

#endif /* VHAL_LOG_LOCAL_SETTING */

#endif	/* #ifndef	VHAL_LOG_H */

