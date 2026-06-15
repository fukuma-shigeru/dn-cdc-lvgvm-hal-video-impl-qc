/*******************************************************************************
    機能名称    ：  Wayland描画定義
    ファイル名称：  wl_renderer_define.h
*******************************************************************************/
#ifndef	WL_RENDERER_DEFINE_H
#define	WL_RENDERER_DEFINE_H

/* 関数戻り値 */
#define WL_RENDERER_SUCCESS					(  0)	/* 正常終了 */
#define WL_RENDERER_ERR						( -1)	/* 異常終了 */
#define WL_RENDERER_ERR_PARAM				( -2)	/* パラメータ不正 */
#define WL_RENDERER_ERR_NOT_INITIALIZED		( -3)	/* 未初期化エラー */
#define WL_RENDERER_ERR_LAYOUT_INFO			( -4)	/* レイアウト情報不正 */
#define WL_RENDERER_ERR_CLIENT_DISCONNECTED	( -5)	/* クライアント切断 */
#define WL_RENDERER_ERR_PROPERTY_ENTRY		( -6)	/* プロパティエントリー不正 */
#define WL_RENDERER_ERR_HALCOMM				( -7)	/* HalCommエラー */
#define WL_RENDERER_ERR_EPOLL				( -8)	/* epoll処理エラー */
#define WL_RENDERER_ERR_SOCKET				( -9)	/* socket処理エラー */
#define WL_RENDERER_ERR_WAYLAND				(-10)	/* waylandプロトコルエラー */
#define WL_RENDERER_ERR_ILM_API				(-11)	/* ilm APIエラー */
#define WL_RENDERER_ERR_BUFFER				(-12)	/* バッファエラー */
#define WL_RENDERER_ERR_THREAD				(-13)	/* スレッドエラー */
#define WL_RENDERER_ERR_SYNC				(-14)	/* スレッド間同期エラー */
#define WL_RENDERER_ERR_WAYLAND_CLIENT		(-15)	/* waylandクライアント処理エラー */
#define WL_RENDERER_ERR_FD					(-16)	/* ファイルディスクリプタエラー */
#define WL_RENDERER_ERR_MEMORY				(-17)	/* メモリ確保エラー */

#endif	/* #ifndef	WL_RENDERER_DEFINE_H */

