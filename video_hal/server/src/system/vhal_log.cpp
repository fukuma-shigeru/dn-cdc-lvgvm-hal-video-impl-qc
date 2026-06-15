/*******************************************************************************
    機能名称    ：  VideoHAL(Server) DLTログモジュール
    ファイル名称：  vhal_log.cpp
*******************************************************************************/
#include "vhal_log.h"
#include "vhal_str_mng.h"

#include <string>

/*****************************************************************************
*	define																	 *
*****************************************************************************/

/*****************************************************************************
*	globals																	 *
*****************************************************************************/
namespace {

DLT_DECLARE_CONTEXT(g_VhalContext);

} /* namespace */

namespace videohal
{

/*****************************************************************************
 処理概要：	DLTログ初期化処理
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void VhalLogInit(void) noexcept
{
	static bool s_VhalDltInitFlag{false};
	const std::string VHAL_DLT_APP_ID{"VDOH"};			/* AppID ※申請ID */
	const std::string VHAL_DLT_CONTEXT_ID{"VDOS"};		/* ContextID ※申請ID */

	/* 多重登録ガード */
	if (false == s_VhalDltInitFlag)
	{
		DLT_REGISTER_APP(VHAL_DLT_APP_ID.c_str(), "VideoHAL Server");

		DLT_REGISTER_CONTEXT(g_VhalContext, VHAL_DLT_CONTEXT_ID.c_str(), "Vhal(Server) log");
		s_VhalDltInitFlag = true;
	}
}

/*****************************************************************************
 処理概要：	DLTログ終了処理
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void VhalLogEnd(void) noexcept
{
	static bool s_VhalDltEndFlag{false};

		/* 多重解除ガード */
	if (false == s_VhalDltEndFlag)
	{
		DLT_UNREGISTER_CONTEXT(g_VhalContext);
		DLT_UNREGISTER_APP();
		s_VhalDltEndFlag = true;
	}
}

/*****************************************************************************
 処理概要 ：	ログ出力関数
 引数    ：	loglevel 	(i)ログレベル
         ：	pName 	    (i)ログ出力関数名
         ：	pline 	    (i)ログ出力行
         ：	format		(i)ログ出力フォーマット
 戻り値  ：	なし
*****************************************************************************/
void VhalLogOut(const DltLogLevelType loglevel, const char* const pName, const int pline, const char *format, ...)
{
	va_list args;
	char logBuf[1024];
	const std::string VHAL_DLT_PREFIX{"[VHALS]"};

	(void)memset(&logBuf[0], 0x00, sizeof(logBuf));
	va_start(args, format);
	(void)vsnprintf(&logBuf[0], sizeof(logBuf), format, args);
	va_end(args);

	DltLogLevelType loglevel_tmp{loglevel};
	if (DLT_LOG_DEFAULT == loglevel_tmp)
	{
		/* DLT_LOG_DEFAULT(ERROR or WARN)の場合はサスペンド状態を確認する */
		if (videohal::CVhalStrManager::GetSuspend())
		{
			loglevel_tmp = DLT_LOG_WARN;
		}
		else
		{
			loglevel_tmp = DLT_LOG_ERROR;
		}
	}

	DLT_LOG(g_VhalContext, loglevel_tmp, DLT_STRING(VHAL_DLT_PREFIX.c_str()), DLT_INT(pline), DLT_STRING(pName), DLT_STRING(&logBuf[0]));
}

/*****************************************************************************
 処理概要：	Camera性能要件 カメラ映像表示完了/カメラ映像非表示完了時のログ出力
 引数    ：	bool mute	(i)true MUTE有効/false MUTE無効
 戻り値  ：	なし
*****************************************************************************/
void VhalLogCameraMute(const char* const pName, const bool mute)
{
	if (true == mute)
	{
		VHAL_LOGI("[%s] #Perf_SoC_CAMERA_HAL_Mute_ON", pName);
	}
	else
	{
		VHAL_LOGI("[%s] #Perf_SoC_CAMERA_HAL_Mute_OFF", pName);
	}
}

/*****************************************************************************
 処理概要：	Camera性能要件 カメラ映像パス切替時のログ出力
 引数    ：	bool camera	(i)true カメラ表示/false カメラ非表示
 戻り値  ：	なし
*****************************************************************************/
void VhalLogCameraPath(const char* const pName, const bool camera)
{
	if (true == camera)
	{
		VHAL_LOGI("[%s] Start Camera #Perf_SoC_CAMERA_HAL_VPathSw_Rcv", pName);
	}
	else
	{
		VHAL_LOGI("[%s] Stop Camera #Perf_SoC_CAMERA_HAL_VPathSw_Rcv", pName);
	}
}

} /* namespace videohal */

extern "C" void VhalLogOut_wrapper(const DltLogLevelType loglevel, const char* const pName, const int pline, const char *format, ...)
{
	va_list args;
	char logBuf[1024];
	const std::string VHAL_DLT_PREFIX{"[VHALS]"};

	(void)memset(&logBuf[0], 0x00, sizeof(logBuf));
	va_start(args, format);
	(void)vsnprintf(&logBuf[0], sizeof(logBuf), format, args);
	va_end(args);
	DLT_LOG(g_VhalContext, loglevel, DLT_STRING(VHAL_DLT_PREFIX.c_str()), DLT_INT(pline), DLT_STRING(pName), DLT_STRING(&logBuf[0]));
}

