/*******************************************************************************
    機能名称    ：  マイコン間通信OPCモジュール(MISC)
    ファイル名称：  vhal_micon_misc_opc.h
*******************************************************************************/
#ifndef	VHAL_MICON_MISC_OPC_H
#define	VHAL_MICON_MISC_OPC_H

extern "C"
{
#include "os_com_brg_lib_public.h"
}

namespace videohal
{

/* データタイプ値 */
static constexpr uint8_t	kDatatypeCamera{static_cast<uint8_t>(OCB_DATA_TYPE_CAM)};		/* 34h */
static constexpr uint8_t	kDatatypeDisplay{static_cast<uint8_t>(OCB_DATA_TYPE_DISP)};		/* 36h */
static constexpr uint8_t	kDatatypeHdmi{static_cast<uint8_t>(OCB_DATA_TYPE_HDMI)};		/* 39h */

/* サブタイプ値 */
enum : uint8_t {
      SUB_TYPE_DISP_MODE_REQ = 0x1U		/*画質モード要求*/
	, SUB_TYPE_DISP_MODE_RSP			/*画質モード応答*/
};

enum : uint8_t {
      SUB_TYPE_DISP_SCREEN_SHOT_REQ = 0x30U		/*スクリーンショット要求*/
	, SUB_TYPE_DISP_SCREEN_SHOT_RSP				/*スクリーンショット応答*/
};

enum : uint8_t {
	  SUB_TYPE_DISP_HUD_FUNC_STATUS = 0x43U		/*HUD機能有無判定結果通知*/
	, SUB_TYPE_DISP_HUD_DISTORTION_CORRECTION	/*HUD歪み補正通知*/
	, SUB_TYPE_DISP_HUD_ROTATION				/*HUD回転パラメータ通知*/
};

enum : uint8_t {
      SUB_TYPE_CAMERA_REQUEST = 0x1U	/*カメラ種別判別要求*/
	, SUB_TYPE_CAMERA_TYPE				/*カメラ種別判別通知*/
	, SUB_TYPE_CAMERA_SYNC				/*同期検知・経路情報通知*/
	, SUB_TYPE_CAMERA_MODE				/*カメラ映像モード通知*/
};

enum : uint8_t {
      SUB_TYPE_HDMI_CONNECT = 0x1U		/*HDMI機器接続状態通知*/
	, SUB_TYPE_HDMI_VIDEO_FORMAT		/*HDMIビデオフォーマット変更通知*/
	, SUB_TYPE_HDMI_AUDIO_FORMAT		/*HDMIオーディオフォーマット変更通知*/
	, SUB_TYPE_HDCP_AUTH_REQ_CDISP		/*C-Disp_HDCP認証要求(未使用)*/
	, SUB_TYPE_HDCP_AUTH_KEY_CLEAR = SUB_TYPE_HDCP_AUTH_REQ_CDISP	/*HDCP認証キークリア*/
	, SUB_TYPE_HDCP_AUTH_NTY_CDISP		/*C-Disp_HDCP認証応答*/
	, SUB_TYPE_HDCP_AUTH_REQ_RSE		/*RSE_HDCP認証要求(予定)*/
	, SUB_TYPE_HDCP_AUTH_NTY_RSE		/*RSE_HDCP認証応答(予定)*/
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_MICON_MISC_OPC_H */
