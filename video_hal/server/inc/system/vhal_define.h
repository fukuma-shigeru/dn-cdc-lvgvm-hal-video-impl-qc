/*******************************************************************************
    機能名称    ：  VideoHALサーバー定義
    ファイル名称：  vhal_define.h
*******************************************************************************/
#ifndef	VHAL_DEFINE_H
#define	VHAL_DEFINE_H

#include "vhal_ctl_define_public.h"

namespace videohal
{

/* 関数戻り値 */
static constexpr int32_t VHAL_SUCCESS					{  0};	/* 正常終了 */
static constexpr int32_t VHAL_ERR						{ -1};	/* 異常終了 */
static constexpr int32_t VHAL_ERR_PARAM					{ -2};	/* パラメータ不正 */
static constexpr int32_t VHAL_ERR_NOT_INITIALIZED		{ -3};	/* 未初期化エラー */
static constexpr int32_t VHAL_ERR_LAYOUT_INFO			{ -4};	/* レイアウト情報不正 */
static constexpr int32_t VHAL_ERR_CLIENT_DISCONNECTED	{ -5};	/* クライアント切断 */
static constexpr int32_t VHAL_ERR_PROPERTY_ENTRY		{ -6};	/* プロパティエントリー不正 */
static constexpr int32_t VHAL_ERR_HALCOMM				{ -7};	/* HalCommエラー */
static constexpr int32_t VHAL_ERR_EPOLL					{ -8};	/* epoll処理エラー */
static constexpr int32_t VHAL_ERR_SOCKET				{ -9};	/* socket処理エラー */
static constexpr int32_t VHAL_ERR_WAYLAND				{-10};	/* waylandプロトコルエラー */
static constexpr int32_t VHAL_ERR_ILM_API				{-11};	/* ilm APIエラー */
/* MISRA C++-2008 Rule 0-1-4 static constexpr int32_t VHAL_ERR_BUFFER				{-12}; */	/* バッファエラー */
static constexpr int32_t VHAL_ERR_THREAD				{-13};	/* スレッドエラー */
/* MISRA C++-2008 Rule 0-1-4 static constexpr int32_t VHAL_ERR_SYNC					{-14}; */	/* スレッド間同期エラー */
static constexpr int32_t VHAL_ERR_WAYLAND_CLIENT		{-15};	/* waylandクライアント処理エラー */
static constexpr int32_t VHAL_ERR_FD					{-16};	/* ファイルディスクリプタエラー */
static constexpr int32_t VHAL_ERR_MEMORY				{-17};	/* メモリ確保エラー */
static constexpr int32_t VHAL_ERR_RUNTIME				{-18};	/* ランタイムエラー（システムコール等） */
static constexpr int32_t VHAL_ERR_QCARCAM_API			{-19};	/* QCARCAM APIエラー */
static constexpr int32_t VHAL_ERR_SYSDB_API				{-20};	/* SYSDB APIエラー */
static constexpr int32_t VHAL_ERR_TIMER					{-21};	/* timer処理エラー */
static constexpr int32_t VHAL_ERR_GPIO_API				{-22};	/* GPIO APIエラー */
static constexpr int32_t VHAL_ERR_DRM_API				{-23};	/* Drm APIエラー */
static constexpr int32_t VHAL_ERR_TIMEOUT				{-24};	/* タイムアウトエラー */
static constexpr int32_t VHAL_ERR_SUSPEND				{-25};	/* サスペンド状態中 */

/* アクセス属性 */
static constexpr uint32_t VHAL_ATTR_ANY					{0U};			/* 属性チェックなし */
static constexpr uint32_t VHAL_ATTR_READ				{static_cast<uint32_t>(1U<<0U)};		/* 読込(GetValue)可能 */
static constexpr uint32_t VHAL_ATTR_WRITE				{static_cast<uint32_t>(1U<<1U)};		/* 書込(SetValue)可能 */
static constexpr uint32_t VHAL_ATTR_RW					{VHAL_ATTR_READ | VHAL_ATTR_WRITE};
static constexpr uint32_t VHAL_ATTR_RONLY				{VHAL_ATTR_READ};

/* Path name */
static const std::string VHAL_PATH_CAMERA				{"CAMERA"};
static const std::string VHAL_PATH_HDMI					{"HDMI"};
static const std::string VHAL_PATH_CAMERA_IMG_ADJ		{"CAMERA-IMG-ADJ"};
static const std::string VHAL_PATH_MULTISENSORY			{"MULTISENSORY"};

/* Camera Display Type */
static constexpr int32_t VHAL_CAMERA_DISPLAY_CENTER		{ 0};
static constexpr int32_t VHAL_CAMERA_DISPLAY_UNKNOWN	{-1};

/* actionイベント発行条件 */
static constexpr int32_t VHAL_EVENT_ACTION_NONE			{ 0};		/* 発行なし（属性がRead時などイベント発行不要時に指定） */
static constexpr int32_t VHAL_EVENT_ACTION_WRITE		{ 1};		/* 書き込み時（Controlプロパティ前回値の同値判定を行わない） */
static constexpr int32_t VHAL_EVENT_ACTION_CHANGE		{ 2};		/* 変化時（Controlプロパティ前回値の同値判定を行う） */

} /* namespace videohal */

#endif	/* #ifndef	VHAL_DEFINE_H */

