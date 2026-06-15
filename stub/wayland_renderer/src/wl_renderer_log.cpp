/*******************************************************************************
    機能名称    ：  Wayland描画 DLTログモジュール
    ファイル名称：  wl_renderer_log.cpp
*******************************************************************************/
#include "wl_renderer_log.h"

#include <string>


/*****************************************************************************
*	define																	 *
*****************************************************************************/

/*****************************************************************************
*	globals																	 *
*****************************************************************************/
namespace {

DLT_DECLARE_CONTEXT(g_WlRndrContext);

} /* namespace */

/*****************************************************************************
*	statics																	 *
*****************************************************************************/

namespace wlrenderer
{

/*****************************************************************************
 処理概要：	DLTログ初期化処理
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void WlRendererLogInit(void)
{
	static bool s_WlRndrDltInitFlag{false};
	const std::string WLRNDR_DLT_ID{"WLRNDR"};

	/* 多重登録ガード */
	if (false == s_WlRndrDltInitFlag)
	{
		DLT_REGISTER_APP(WLRNDR_DLT_ID.c_str(), "Wayland Renderer");

		DLT_REGISTER_CONTEXT(g_WlRndrContext, WLRNDR_DLT_ID.c_str(), "WLRNDR log");
		s_WlRndrDltInitFlag = true;
	}
}

/*****************************************************************************
 処理概要：	DLTログ終了処理
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void WlRendererLogEnd(void)
{
	static bool s_WlRndrDltEndFlag{false};

	/* 多重解除ガード */
	if (false == s_WlRndrDltEndFlag)
	{
		DLT_UNREGISTER_CONTEXT(g_WlRndrContext);
		DLT_UNREGISTER_APP();
		s_WlRndrDltEndFlag = true;
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
void WlRendererLogOut(const DltLogLevelType loglevel, const char* const pName, const int pline, const char *format, ...)
{
	va_list args;
	constexpr uint32_t WLRNDR_DLT_BUF_MAX{1024U};
	char logBuf[WLRNDR_DLT_BUF_MAX];
	const std::string WLRNDR_DLT_PREFIX{"[WLRNDR]"};

	(void)memset(&logBuf[0], 0x00, sizeof(logBuf));
	va_start(args, format);
	(void)vsnprintf(&logBuf[0], sizeof(logBuf), format, args);
	va_end(args);
	DLT_LOG(g_WlRndrContext, loglevel, DLT_STRING(WLRNDR_DLT_PREFIX.c_str()), DLT_INT(pline), DLT_STRING(pName), DLT_STRING(&logBuf[0]));
}

} /* namespace wlrenderer */

