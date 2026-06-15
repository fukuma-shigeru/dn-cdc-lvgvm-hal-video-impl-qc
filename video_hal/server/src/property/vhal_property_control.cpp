/*******************************************************************************
    機能名称    ：  プロパティ制御モジュール
    ファイル名称：  vhal_property_control.cpp
*******************************************************************************/
#include "vhal_property_control.h"

#include <iostream>
#include <stdexcept>
#include <unordered_set>

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_round_cast.h"
#include "vhal_ivi_controller.h"
#include "vhal_main_control.h"
#include "vhal_micon_most_video_info.h"
#include "vhal_sysdb_control.h"
#include "vhal_gpio_control.h"
#include "vhal_layout_mng.h"
#include "vhal_event_item_screen_shot_event_micon.h"
#include "vhal_mute.h"
#include "vhal_capture_camera.h"
#include "vhal_capture_hdmi.h"
#include "vhal_capture_micon_control.h"
#include "vhal_micon_comm_control.h"
#include "vhal_color_mng.h"
#include "vhal_event_item_add_output.h"
#include "vhal_movie_control.h"
#include "vhal_screen_shot.h"
#include "vhal_observer_control.h"
#include "vhal_hdcp_auth_mng.h"
#include "vhal_color_recv_item_quality.h"
#include "vhal_environment_mng.h"
#include "vhal_event_item_power_state.h"
#include "vhal_str_mng.h"
#include "vhal_debug_system.h"
#include "vhal_event_item_hud_screen_event.h"

extern "C"
{
#include "spf_timer_public.h"
#include "sif_util.h"
}

namespace videohal
{

struct VhalRectanglePropNames CVhalPropertyControl::front_output_control_{
	VHAL_PROP_VPATH_FRONT_CTL_OUTPUT_X, VHAL_PROP_VPATH_FRONT_CTL_OUTPUT_Y, VHAL_PROP_VPATH_FRONT_CTL_OUTPUT_W, VHAL_PROP_VPATH_FRONT_CTL_OUTPUT_H
};
struct VhalRectanglePropNames CVhalPropertyControl::front_output_status_{
	VHAL_PROP_VPATH_FRONT_STS_OUTPUT_X, VHAL_PROP_VPATH_FRONT_STS_OUTPUT_Y, VHAL_PROP_VPATH_FRONT_STS_OUTPUT_W, VHAL_PROP_VPATH_FRONT_STS_OUTPUT_H
};
struct VhalRectanglePropNames CVhalPropertyControl::rear_output_control_{
	VHAL_PROP_VPATH_REAR_CTL_OUTPUT_X, VHAL_PROP_VPATH_REAR_CTL_OUTPUT_Y, VHAL_PROP_VPATH_REAR_CTL_OUTPUT_W, VHAL_PROP_VPATH_REAR_CTL_OUTPUT_H
};
struct VhalRectanglePropNames CVhalPropertyControl::rear_output_status_{
	VHAL_PROP_VPATH_REAR_STS_OUTPUT_X, VHAL_PROP_VPATH_REAR_STS_OUTPUT_Y, VHAL_PROP_VPATH_REAR_STS_OUTPUT_W, VHAL_PROP_VPATH_REAR_STS_OUTPUT_H
};
struct VhalRectanglePropNames CVhalPropertyControl::camera_output_control_{
	VHAL_PROP_VPATH_CAMERA_CTL_OUTPUT_X, VHAL_PROP_VPATH_CAMERA_CTL_OUTPUT_Y, VHAL_PROP_VPATH_CAMERA_CTL_OUTPUT_W, VHAL_PROP_VPATH_CAMERA_CTL_OUTPUT_H
};
struct VhalRectanglePropNames CVhalPropertyControl::camera_output_status_{
	VHAL_PROP_VPATH_CAMERA_STS_OUTPUT_X, VHAL_PROP_VPATH_CAMERA_STS_OUTPUT_Y, VHAL_PROP_VPATH_CAMERA_STS_OUTPUT_W, VHAL_PROP_VPATH_CAMERA_STS_OUTPUT_H
};
struct VhalRectanglePropNames CVhalPropertyControl::icluster_output_control_{
	VHAL_PROP_VPATH_ICLUSTER_CTL_OUTPUT_X, VHAL_PROP_VPATH_ICLUSTER_CTL_OUTPUT_Y, VHAL_PROP_VPATH_ICLUSTER_CTL_OUTPUT_W, VHAL_PROP_VPATH_ICLUSTER_CTL_OUTPUT_H
};
struct VhalRectanglePropNames CVhalPropertyControl::icluster_output_status_{
	VHAL_PROP_VPATH_ICLUSTER_STS_OUTPUT_X, VHAL_PROP_VPATH_ICLUSTER_STS_OUTPUT_Y, VHAL_PROP_VPATH_ICLUSTER_STS_OUTPUT_W, VHAL_PROP_VPATH_ICLUSTER_STS_OUTPUT_H
};
/* クリッピング設定 */
struct VhalClippingPropNames CVhalPropertyControl::front_clipping_control_{
	VHAL_PROP_VPATH_FRONT_CTL_CLIP_ENABLE, VHAL_PROP_VPATH_FRONT_CTL_CLIP_X, VHAL_PROP_VPATH_FRONT_CTL_CLIP_Y, VHAL_PROP_VPATH_FRONT_CTL_CLIP_W, VHAL_PROP_VPATH_FRONT_CTL_CLIP_H
};
struct VhalClippingPropNames CVhalPropertyControl::front_clipping_status_{
	VHAL_PROP_VPATH_FRONT_STS_CLIP_ENABLE, VHAL_PROP_VPATH_FRONT_STS_CLIP_X, VHAL_PROP_VPATH_FRONT_STS_CLIP_Y, VHAL_PROP_VPATH_FRONT_STS_CLIP_W, VHAL_PROP_VPATH_FRONT_STS_CLIP_H
};
struct VhalClippingPropNames CVhalPropertyControl::icluster_clipping_control_{
	VHAL_PROP_VPATH_ICLUSTER_CTL_CLIP_ENABLE, VHAL_PROP_VPATH_ICLUSTER_CTL_CLIP_X, VHAL_PROP_VPATH_ICLUSTER_CTL_CLIP_Y, VHAL_PROP_VPATH_ICLUSTER_CTL_CLIP_W, VHAL_PROP_VPATH_ICLUSTER_CTL_CLIP_H
};
struct VhalClippingPropNames CVhalPropertyControl::icluster_clipping_status_{
	VHAL_PROP_VPATH_ICLUSTER_STS_CLIP_ENABLE, VHAL_PROP_VPATH_ICLUSTER_STS_CLIP_X, VHAL_PROP_VPATH_ICLUSTER_STS_CLIP_Y, VHAL_PROP_VPATH_ICLUSTER_STS_CLIP_W, VHAL_PROP_VPATH_ICLUSTER_STS_CLIP_H
};
/* ヒーコンエリア設定 */
struct VhalRectanglePropNames CVhalPropertyControl::front_heacon_control_{
	VHAL_PROP_HEACON_FRONT_CTL_OUTPUT_X, VHAL_PROP_HEACON_FRONT_CTL_OUTPUT_Y, VHAL_PROP_HEACON_FRONT_CTL_OUTPUT_W, VHAL_PROP_HEACON_FRONT_CTL_OUTPUT_H
};
struct VhalRectanglePropNames CVhalPropertyControl::front_heacon_status_{
	VHAL_PROP_HEACON_FRONT_STS_OUTPUT_X, VHAL_PROP_HEACON_FRONT_STS_OUTPUT_Y, VHAL_PROP_HEACON_FRONT_STS_OUTPUT_W, VHAL_PROP_HEACON_FRONT_STS_OUTPUT_H
};

/*****************************************************************************
 クラス名称：CVhalPropertyControlOutputListener
 処理概要  ：出力デバイス検知クラス（検知後、内部イベント発行）
*****************************************************************************/
class CVhalPropertyControlOutputListener : public wlrenderer::CWaylandRendererOutputListener {
public:
	CVhalPropertyControlOutputListener(CVhalEventRoute* const p_event_route, CVhalLayoutManager* const p_layout_mng) noexcept
		:p_event_route_(p_event_route)
		,p_layout_mng_(p_layout_mng)
	{
	}
	~CVhalPropertyControlOutputListener(void) override = default;
  	CVhalPropertyControlOutputListener(const CVhalPropertyControlOutputListener& src) = delete;
	CVhalPropertyControlOutputListener& operator=(const CVhalPropertyControlOutputListener& src) & = delete;
	CVhalPropertyControlOutputListener(CVhalPropertyControlOutputListener&& src) = delete;
	CVhalPropertyControlOutputListener& operator=(CVhalPropertyControlOutputListener&& src) & = delete;

	void NotifyOutputAdded(const int32_t width, const int32_t height, const std::string& model, const std::string& make) override
	{
		std::unique_ptr<CVhalEventItemAddOutput> p_add_output{std::make_unique<CVhalEventItemAddOutput>()};
		p_add_output->SetName(std::string("add output"));
		p_add_output->SetData(p_layout_mng_, width, height, model, make);
		const int32_t ret{p_event_route_->WriteEvent(p_add_output.get())};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGEW("WriteEvent error. ret=%d.", ret);
		}
		else
		{
			(void)p_add_output.release();
		}
	}

private:
	CVhalEventRoute* p_event_route_;
	CVhalLayoutManager* p_layout_mng_;
};

/*****************************************************************************
 クラス名称：CVhalPropertyControlEventListener
 処理概要  ：PropertyControlイベントリスナクラス
*****************************************************************************/
class CVhalPropertyControlEventListener
	: public CVhalLayoutManagerEventListenerBase
	, public CVhalCaptureReceiveEventListenerBase
	, public CVhalCaptureTabReceiveEventListenerBase
	, public CVhalMiscEventListenerBase
	, public CVhalHdcpAuthRsltListenerBase
	, public CVhalFileObserverEventListenerBase
	, public CVhalSysdbEventListenerBase
	, public CVhalMiconMostReceiveEventListenerBase
	, public CVhalMuteListenerBase
	, public CVhalScreenShotReceiveEventListenerBase {
public:
	CVhalPropertyControlEventListener(CVhalPropertyControl* const p_propertyControl, CVhalPropertyControlCamera* const p_camera_control) noexcept
		:p_propertyControl_(p_propertyControl)
		,p_camera_control_(p_camera_control)
	{
	}
	~CVhalPropertyControlEventListener(void) override = default;
  	CVhalPropertyControlEventListener(const CVhalPropertyControlEventListener& src) = delete;
	CVhalPropertyControlEventListener& operator=(const CVhalPropertyControlEventListener& src) & = delete;
	CVhalPropertyControlEventListener(CVhalPropertyControlEventListener&& src) = delete;
	CVhalPropertyControlEventListener& operator=(CVhalPropertyControlEventListener&& src) & = delete;

	void NotifyVpathAvailable(const std::vector<std::string>& videopath_names, const VideoOutputTarget output_target, const bool created) const noexcept override
	{
		const int32_t ret{p_propertyControl_->UpdateVpathStatusAvailable(videopath_names)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("UpdateVpathStatusAvailable error ret=%d.", ret);
		}

		if (true == created)
		{
			p_propertyControl_->UpdateVpathStatusCurrentHdmi(videopath_names, output_target);
		}
	}

	void NotifyScreenAvailable(const int32_t screen_id, const bool display, const int32_t width, const int32_t height) const noexcept override
	{
		const int32_t ret{p_propertyControl_->UpdateSettingStatusDisplay(screen_id, display, width, height)};
		if( VHAL_SUCCESS != ret )
		{
			VHAL_LOGW("UpdateSettingStatusDisplay ret=%d.", ret);
		}
	}

	void NotifyScreenShotResult(const int32_t result, const ScreenShotType type) const noexcept override
	{
		if (type == ScreenShotType::SCREEN)
		{
			p_propertyControl_->UpdateScreenShotStatus(result);
		}
		else if (type == ScreenShotType::SURFACE)
		{
			p_propertyControl_->UpdateSurfaceScreenShotStatus(result);
		}
		else
		{
			VHAL_LOGE("parameter error. type=%d", static_cast<int32_t>(type));
		}
	}

	void NotifyScreenShotMiconResult(const int32_t result) const noexcept override
	{
		p_propertyControl_->UpdateScreenShotStatus(result);
	}

	void NotifyMovieResult(const int32_t result) const noexcept override
	{
		p_propertyControl_->UpdateMovieStartResult(result);
	}

	void NotifyTabReceiveCameraSync(const VhalTabCamSync sync) const noexcept override
	{
		/* カメラ同期処理 */
		p_camera_control_->ActionCameraSync(sync);

		/* カメラ映像キャプチャ動作設定 */
		p_camera_control_->ActionCameraControl(sync);
	}

	void NotifyTabSetCameraInputSize(const int32_t disp) const noexcept override
	{
		p_camera_control_->SetCameraInputSize(disp);
	}

	void NotifyReceiveCameraControl(void) const noexcept override
	{
		/* カメラ映像キャプチャ動作設定 */
		p_camera_control_->ActionCameraControl();
	}

	void NotifyReceiveCaptureStandby2(const captureInputType input_type, const bool status) const noexcept override
	{
		/* カメラ時のみ */
		if (captureInputType::VHAL_CAPTURE_INPUT_CAMERA == input_type)
		{
			p_camera_control_->ActionCameraCaptureStandby2(status);
		}
	}

	void NotifyReceiveCaptureFrame(void) const noexcept override
	{
		p_camera_control_->SetCameraCaptureFrame();
	}

	void NotifyReceiveCameraDisplayStatus(const VhalCameraDisplayStatus current_cam_disp_status) const noexcept override
	{
		/* カメラ映像パス設定可否判定用 */
		std::vector<std::string> videopath_name_list{};
		videopath_name_list.push_back(VHAL_PATH_CAMERA);

		if (VhalCameraDisplayStatus::kReady == current_cam_disp_status)
		{
			/* キャプチャ設定(動画状態を確認,必要時はタイマ起動) */
			bool is_capture{false};
			(void)p_propertyControl_->OpenCameraCapture(true, true, is_capture);

			/* 映像キャプチャ準備状態通知2:Hi */
			NotifyReceiveCaptureStandby2(captureInputType::VHAL_CAPTURE_INPUT_CAMERA, true);

			/* キャプチャ設定済みの場合のみ通知 */
			if(true == is_capture)
			{
				/* カメラ映像パス設定可否判定通知 */
				NotifyVpathAvailable(videopath_name_list, VIDEO_OUTPUT_TARGET_CAMERA, false);
			}
		}
		else
		{
			/* 映像キャプチャ準備状態通知2:Lo */
			NotifyReceiveCaptureStandby2(captureInputType::VHAL_CAPTURE_INPUT_CAMERA, false);
			/* カメラ映像パス設定可否判定通知 */
			NotifyVpathAvailable(videopath_name_list, VIDEO_OUTPUT_TARGET_CAMERA, false);
		}
	}

	/*****************************************************************************
	 処理概要：	カメラキャプチャ設定
	 引数    ：	const bool movie_check	(i)動画再生開始中の状態確認有無
	           	bool& is_capture		(o)false:キャプチャ未設定, true:キャプチャ設定済
	 戻り値  ：	処理結果
	      		VHAL_ERR			キャプチャ失敗
	      		VHAL_SUCCESS		正常処理
	*****************************************************************************/
	int32_t NotifyReceiveOpenCameraCapture(const bool movie_check, bool& is_capture) const noexcept override
	{
		/* カメラキャプチャ設定(本API内でタイマ起動は行わない) */
		const int32_t result{p_propertyControl_->OpenCameraCapture(movie_check, false, is_capture)};

		/* キャプチャ設定済の場合 */
		if(true == is_capture)
		{
			/* カメラ映像パス設定可否判定用 */
			std::vector<std::string> videopath_name_list{};
			videopath_name_list.push_back(VHAL_PATH_CAMERA);

			/* カメラ映像パス設定可否判定通知 */
			NotifyVpathAvailable(videopath_name_list, VIDEO_OUTPUT_TARGET_CAMERA, false);
		}

		return result;
	}

	/* HDCP認証結果プロパティ更新 */
	void NotifyHdcpAuthResult(const hdcpAuthType type, const CVhalHdcpAuthRsltData &hdcp_result) const noexcept override
	{
		if (hdcpAuthType::HDCP_AUTH_TYPE_CDISP == type)
		{
			p_propertyControl_->UpdateHdcpAuthResultCdisp(hdcp_result);
		}
		if (hdcpAuthType::HDCP_AUTH_TYPE_RSE == type)
		{
			p_propertyControl_->UpdateHdcpAuthResultRse(hdcp_result);
		}
	}

	/* HDCP認証結果クリア */
	void NotifyHdcpAuthClear(void) const noexcept override
	{
		p_propertyControl_->UpdateHdcpAuthClear();
	}

	/* カメラ映像切替完了 */
	void NotifyCameraChanged(const int32_t result) const noexcept override
	{
		p_propertyControl_->UpdateCameraPathStatus(result);
	}
		
	void NotifyReceiveMostMuteRearDispResult(const bool mute, const enum MostUpdateType update_type) const noexcept override
	{
		/* MOST 後席ディスプレイ全体のMUTE結果Update（コールバック） */
		p_propertyControl_->UpdateMostMuteRearDispStatus(mute, update_type);
	}

	void NotifySettingConnectedHdmiDevice(const VhalHdmiState state) const noexcept override
	{
		/* BEVでは未使用 */
	}
		
	void NotifyReceiveMostRsePathResult(const uint32_t result, const enum MostUpdateType update_type) const noexcept override
	{
		/* MOST RSE映像パス切替結果Update（コールバック） */
		p_propertyControl_->UpdateMostRsePathStatus(result, update_type);
	}

	/* HDMIビデオフォーマット切替時 */
	void NotifyChangeHdmiVideoFormat(const int32_t state) const noexcept override
	{
		p_propertyControl_->ChangeHdmiVideoFormat(state);
	}

	void NotifyUpdateFilePropertyHdmi(const int32_t state) const noexcept override
	{
		p_propertyControl_->UpdateFilePropertyHdmi(state);
	}

	/* MISC_HDMIオーディオフォーマット切替時 */
	void NotifyChangeHdmiAudioFormat(const int32_t audio_format) const noexcept override
	{
		p_propertyControl_->ChangeHdmiAudioFormat(audio_format);
	}

	/* スクリーンショット応答受信 */
	void NotifyReceiveScreenShotMiconResponse(const VhalScreenShotResult result) const noexcept override
	{
		p_propertyControl_->NotifyScreenShotMiconResponse(result);
	}

private:
	CVhalPropertyControl* p_propertyControl_;
	CVhalPropertyControlCamera* p_camera_control_;
};


/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalPropertyControl::CVhalPropertyControl(void)
	:p_main_(nullptr)
	,p_event_route_(nullptr)
	,p_sysdbctrl_(nullptr)
	,p_gpioctrl_(nullptr)
	,p_layout_mng_(nullptr)
	,p_mute_(nullptr)
	,p_capture_control_(nullptr)
	,p_renderer_(nullptr)
	,p_micon_comm_control_(nullptr)
	,p_color_mng_(nullptr)
	,p_output_listener_(nullptr)
	,p_propertyControl_listener_(nullptr)
	,p_micon_misc_receiver_(nullptr)
	,p_capture_tab_receiver_(nullptr)
	,p_camera_mode_send_item_(nullptr)
	,p_movie_control_(nullptr)
	,p_fileobserver_control_(nullptr)
	,p_screen_shot_micon_(nullptr)
	,p_screen_shot_(nullptr)
	,p_micon_most_video_info_(nullptr)
	,p_observer_control_(nullptr)
	,p_hdcp_auth_mng_(nullptr)
	,p_hdcp_misc_receiver_(nullptr)
	,p_color_adjust_misc_receiver_(nullptr)
	,p_environment_mng_(nullptr)
	,p_screenshot_receiver_(nullptr)
	,publish_order_property_{}
	,property_entries_{}
	,p_hud_screen_receiver_(nullptr)
	,p_hud_screen_controller_(nullptr)
{
 	VHAL_LOGV("CVhalPropertyControl is created. this=%p", this);


	/* プロパティ名        属性        型        actionイベント発行条件 */
	/* プロパティ取得特殊処理        プロパティ更新処理  */
 	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_CTL_CURRENT,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_STR,   VHAL_EVENT_ACTION_WRITE,
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionVpathFrontCurrent(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_STS_CURRENT,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_REAR_CTL_CURRENT,				VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_STR,   VHAL_EVENT_ACTION_WRITE,
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionVpathRearCurrent(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_REAR_STS_CURRENT,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_CAMERA_CTL_CURRENT,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_STR,   VHAL_EVENT_ACTION_WRITE,
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionVpathCameraCurrent(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_CAMERA_STS_CURRENT,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_ICLUSTER_CTL_CURRENT,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_STR,   VHAL_EVENT_ACTION_WRITE,
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionVpathIClusterCurrent(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_ICLUSTER_STS_CURRENT,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_CTL_OUTPUT_ON,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL,  VHAL_EVENT_ACTION_CHANGE,
	nullptr, [this] (VhalPropertyEntry * const p_entry) -> int32_t { return ActionVpathFrontOutput(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_CTL_OUTPUT_X,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_CTL_OUTPUT_Y,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_CTL_OUTPUT_W,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_CTL_OUTPUT_H,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_STS_OUTPUT_X,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_STS_OUTPUT_Y,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_STS_OUTPUT_W,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_STS_OUTPUT_H,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_REAR_CTL_OUTPUT_ON,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL,  VHAL_EVENT_ACTION_CHANGE,
	nullptr, [this] (VhalPropertyEntry * const p_entry) -> int32_t { return ActionVpathRearOutput(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_REAR_CTL_OUTPUT_X,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_REAR_CTL_OUTPUT_Y,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_REAR_CTL_OUTPUT_W,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_REAR_CTL_OUTPUT_H,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_REAR_STS_OUTPUT_X,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_REAR_STS_OUTPUT_Y,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_REAR_STS_OUTPUT_W,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_REAR_STS_OUTPUT_H,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_CAMERA_CTL_OUTPUT_ON,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL,  VHAL_EVENT_ACTION_CHANGE,	
	nullptr, [this] (VhalPropertyEntry * const p_entry) -> int32_t { return ActionVpathCameraOutput(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_CAMERA_CTL_OUTPUT_X,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_CAMERA_CTL_OUTPUT_Y,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_CAMERA_CTL_OUTPUT_W,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_CAMERA_CTL_OUTPUT_H,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_CAMERA_STS_OUTPUT_X,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_CAMERA_STS_OUTPUT_Y,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_CAMERA_STS_OUTPUT_W,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_CAMERA_STS_OUTPUT_H,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_ICLUSTER_CTL_OUTPUT_ON,		VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL,  VHAL_EVENT_ACTION_CHANGE,	
	nullptr, [this] (VhalPropertyEntry * const p_entry) -> int32_t { return ActionVpathIClusterOutput(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_ICLUSTER_CTL_OUTPUT_X,		VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_ICLUSTER_CTL_OUTPUT_Y,		VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_ICLUSTER_CTL_OUTPUT_W,		VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_ICLUSTER_CTL_OUTPUT_H,		VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_ICLUSTER_STS_OUTPUT_X,		VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_ICLUSTER_STS_OUTPUT_Y,		VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_ICLUSTER_STS_OUTPUT_W,		VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_ICLUSTER_STS_OUTPUT_H,		VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_REAR_CTL_RSE_DISP,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_STR,   VHAL_EVENT_ACTION_WRITE,	
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionVpathRearRseDisp(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_REAR_STS_RSE_DISP,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_REAR_CTL_RSE_NOTIFY,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL,  VHAL_EVENT_ACTION_WRITE,	
	nullptr, [this] (VhalPropertyEntry * const p_entry) -> int32_t { return ActionVpathRearRseNotify(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_REAR_STS_RSE_NOTIFY,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_CTL_CLIP_ON,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL,  VHAL_EVENT_ACTION_CHANGE,	
	nullptr, [this] (VhalPropertyEntry * const p_entry) -> int32_t { return ActionVpathFrontClipping(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_CTL_CLIP_ENABLE,		VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL,  VHAL_EVENT_ACTION_CHANGE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_CTL_CLIP_X,				VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_CTL_CLIP_Y,				VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_CTL_CLIP_W,				VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_CTL_CLIP_H,				VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_STS_CLIP_ENABLE,		VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_BOOL,  VHAL_EVENT_ACTION_NONE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_STS_CLIP_X,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_STS_CLIP_Y,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_STS_CLIP_W,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_STS_CLIP_H,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_ICLUSTER_CTL_CLIP_ON,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_CHANGE,
	nullptr, [this] (VhalPropertyEntry * const p_entry) -> int32_t { return ActionVpathIClusterClipping(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_ICLUSTER_CTL_CLIP_ENABLE,		VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_CHANGE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_ICLUSTER_CTL_CLIP_X,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_CHANGE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_ICLUSTER_CTL_CLIP_Y,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_CHANGE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_ICLUSTER_CTL_CLIP_W,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_CHANGE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_ICLUSTER_CTL_CLIP_H,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_CHANGE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_ICLUSTER_STS_CLIP_ENABLE,		VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_NONE,		
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_ICLUSTER_STS_CLIP_X,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,		
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_ICLUSTER_STS_CLIP_Y,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,		
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_ICLUSTER_STS_CLIP_W,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,		
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_ICLUSTER_STS_CLIP_H,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,		
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_CTL_WIDE,				VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_CHANGE,	
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionVpathFrontWide(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_STS_WIDE,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,		
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_REAR_CTL_WIDE,				VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_CHANGE,	
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionVpathRearWide(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_REAR_STS_WIDE,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,		
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_CTL_VISIBLE,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_CHANGE,	
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionVpathFrontVisible(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_STS_VISIBLE,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_NONE,		
	[this] (VhalPropertyEntry * const p_entry) -> int32_t { return GenerateFrontVisible(p_entry); }, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_REAR_CTL_VISIBLE,				VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_CHANGE,	
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionVpathRearVisible(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_REAR_STS_VISIBLE,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_NONE,	
	[this] (VhalPropertyEntry * const p_entry) -> int32_t { return GenerateRearVisible(p_entry); }, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_ICLUSTER_CTL_VISIBLE,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_CHANGE,	
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionVpathIClusterVisible(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_ICLUSTER_STS_VISIBLE,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_NONE,	
	[this] (VhalPropertyEntry * const p_entry) -> int32_t { return GenerateIClusterVisible(p_entry); }, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_CTL_OPACITY,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_CHANGE,	
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionVpathFrontOpacity(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_STS_OPACITY,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_REAR_CTL_OPACITY,				VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_CHANGE,	
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionVpathRearOpacity(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_REAR_STS_OPACITY,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_ICLUSTER_CTL_OPACITY,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_CHANGE,	
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionVpathIClusterOpacity(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_ICLUSTER_STS_OPACITY,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VSRC_FRONT_CTL_ID,					VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_WRITE,	
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionVsrcFrontId(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_VSRC_FRONT_STS_ID,					VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VSRC_FRONT_CTL_BRIGHT,				VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_WRITE,
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionVsrcFrontBright(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_VSRC_FRONT_CTL_CONTRAST,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_WRITE,	
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionVsrcFrontContrast(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_VSRC_FRONT_STS_BRIGHT,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VSRC_FRONT_STS_CONTRAST,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VSRC_FRONT_CTL_IMG_ADJ_UPD,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_CHANGE,	
	nullptr, [this] (VhalPropertyEntry * const p_entry) -> int32_t { return ActionVsrcImgAdjUpdate(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_VSRC_FRONT_CTL_FORCE_HMI_IMG_ADJ,	VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_CHANGE, 
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionVsrcForceHmiImgAdj(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_VSRC_FRONT_STS_FORCE_HMI_IMG_ADJ,	VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_NONE,   
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VSRC_FRONT_CTL_FORCE_MULTISENSORY_IMG_ADJ, VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_CHANGE,
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionVsrcForceMultisensoryImgAdj(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_VSRC_FRONT_STS_FORCE_MULTISENSORY_IMG_ADJ, VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_NONE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_MUTE_FRONT_CTL_DISP,				VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_CHANGE,	
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionMuteFrontDisp(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_MUTE_FRONT_CTL_VIDEO,				VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_CHANGE,	
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionMuteFrontVideo(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_MUTE_FRONT_CTL_BACK,				VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_CHANGE,	
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionMuteFrontBacklight(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_MUTE_REAR_CTL_DISP,					VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_WRITE,	
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionMuteRearDisp(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_MUTE_REAR_CTL_VIDEO,				VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_CHANGE,	
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionMuteRearVideo(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_MUTE_FRONT_STS_DISP,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_MUTE_FRONT_STS_VIDEO,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_MUTE_FRONT_STS_BACK,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_MUTE_REAR_STS_DISP,					VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_MUTE_REAR_STS_VIDEO,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_FAUTH_CTL_RSE,						VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_CHANGE,	
	nullptr, [this] (VhalPropertyEntry * const p_entry) -> int32_t { return ActionHdcpAuthRse(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_FAUTH_CTL_SPRT,						VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_CHANGE,	
	nullptr, [this] (VhalPropertyEntry * const p_entry) -> int32_t { return ActionHdcpAuthSeparateDisp(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_FAUTH_CTL_ICLUSTER,					VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_CHANGE,	
	nullptr, [this] (VhalPropertyEntry * const p_entry) -> int32_t { return ActionHdcpAuthICluster(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_FAUTH_STS_RSE_RSLT,					VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_FAUTH_STS_RSE_DEV_CNT,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_FAUTH_STS_RSE_RCV_ID0,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_FAUTH_STS_RSE_RCV_ID1,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_FAUTH_STS_RSE_RCV_ID2,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_FAUTH_STS_SPRT_RSLT,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_FAUTH_STS_SPRT_DEV_CNT,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_FAUTH_STS_SPRT_RCV_ID0,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_IVI_LAYOUT_CTL_CONF_FILE,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_STR,  VHAL_EVENT_ACTION_WRITE,	
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionIviLayoutConfFile(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_IVI_LAYOUT_STS_CONF_FILE,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_IVI_SCRN_STS_PROPS,					VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_STR,  VHAL_EVENT_ACTION_NONE,	 
	[this] (VhalPropertyEntry * const p_entry) -> int32_t { return GenerateIviScreenInfo(p_entry); },  nullptr});
	(void)property_entries_.emplace(VHAL_PROP_IVI_LAYER_STS_PROPS,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_STR,  VHAL_EVENT_ACTION_NONE,	 
	[this] (VhalPropertyEntry * const p_entry) -> int32_t { return GenerateIviLayerInfo(p_entry); },   nullptr});
	(void)property_entries_.emplace(VHAL_PROP_IVI_SURF_STS_PROPS,					VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_STR,  VHAL_EVENT_ACTION_NONE,	 
	[this] (VhalPropertyEntry * const p_entry) -> int32_t { return GenerateIviSurfaceInfo(p_entry); }, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_IVI_LAYER_CTL_ORDER,				VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_CHANGE,	
	[this] (VhalPropertyEntry * const p_entry) -> int32_t { return GenerateIviLayerOrder(p_entry); }, [this] (VhalPropertyEntry * const p_entry) { return ActionIviLayerOrder(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_IVI_LAYER_STS_ORDER,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,	 
	[this] (VhalPropertyEntry * const p_entry) -> int32_t { return GenerateIviLayerOrder(p_entry); }, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_CAP_SCRN_CTL_BITMAP,				VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_WRITE,	
	nullptr, [this] (VhalPropertyEntry * const p_entry) -> int32_t { return ActionScreenCapture(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_CAP_SCRN_STS_BITMAP,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_CAP_SURF_CTL_BITMAP,				VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_WRITE,	
	nullptr, [this] (VhalPropertyEntry * const p_entry) -> int32_t { return ActionSurfaceScreenCapture(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_CAP_SURF_STS_BITMAP,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_MOVIE_FRONT_CTL_PREPARE,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL,  VHAL_EVENT_ACTION_CHANGE,	
	nullptr, [this] (VhalPropertyEntry * const p_entry) -> int32_t { return ActionMovieFrontPrepare(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_MOVIE_FRONT_CTL_FILE_PATH,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_STR,   VHAL_EVENT_ACTION_CHANGE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_MOVIE_FRONT_CTL_OUTPUT_X,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_MOVIE_FRONT_CTL_OUTPUT_Y,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_MOVIE_FRONT_CTL_OUTPUT_W,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_MOVIE_FRONT_CTL_OUTPUT_H,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_MOVIE_FRONT_CTL_REPEAT,				VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL,  VHAL_EVENT_ACTION_CHANGE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_MOVIE_FRONT_CTL_CLEAR,				VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL,  VHAL_EVENT_ACTION_CHANGE,	
	nullptr, [this] (VhalPropertyEntry * const p_entry) -> int32_t { return ActionMovieFrontClear(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_MOVIE_FRONT_CTL_START,				VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL,  VHAL_EVENT_ACTION_CHANGE,	
	nullptr, [this] (VhalPropertyEntry * const p_entry) -> int32_t { return ActionMovieFrontStart(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_MOVIE_FRONT_CTL_CANCEL,				VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL,  VHAL_EVENT_ACTION_CHANGE,	
	nullptr, [this] (VhalPropertyEntry * const p_entry) -> int32_t { return ActionMovieFrontCancel(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_MOVIE_FRONT_STS_RESULT,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_SETTING_CTL_DISP,					VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,	
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionSettingDisplayType(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_SETTING_STS_DISP,					VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_SETTING_CTL_CNCT_CAMERA,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,	
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionSettingConnectedCamera(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_SETTING_STS_CNCT_CAMERA,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_SETTING_CTL_CNCT_RSE,				VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,	
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionSettingConnectedRSE(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_SETTING_STS_CNCT_RSE,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_SETTING_CTL_CNCT_SPRT,				VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,	
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionSettingConnectedSeparateDisp(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_SETTING_STS_CNCT_SPRT,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_SETTING_CTL_DAY_NIGHT,				VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_CHANGE,
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionSettingDayNight(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_SETTING_STS_DAY_NIGHT,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_SETTING_CTL_THEME_COLOR,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_WRITE,	
	nullptr, [this] (const VhalPropertyEntry * const p_entry) -> int32_t { return ActionSettingThemeColor(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_SETTING_STS_THEME_COLOR,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_SETTING_STS_CNCT_HDMI,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,   VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_SETTING_STS_DISP_FRONT,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_BOOL,  VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_SETTING_STS_DISP_REAR,				VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_BOOL,  VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_SETTING_STS_DISP_ICLUSTER,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_BOOL,  VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_FRONT_STS_AVAILABLE,		 	VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_NONE,	
	[this] (VhalPropertyEntry * const p_entry) -> int32_t { return GenerateVpathFrontAvailable(p_entry); },    nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_REAR_STS_AVAILABLE,		 	VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_NONE,	
	[this] (VhalPropertyEntry * const p_entry) -> int32_t { return GenerateVpathRearAvailable(p_entry); },     nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_CAMERA_STS_AVAILABLE,		 	VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_NONE,	
	[this] (VhalPropertyEntry * const p_entry) -> int32_t { return GenerateVpathCameraAvailable(p_entry); },   nullptr});
	(void)property_entries_.emplace(VHAL_PROP_VPATH_ICLUSTER_STS_AVAILABLE,	 	VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_NONE,	
	[this] (VhalPropertyEntry * const p_entry) -> int32_t { return GenerateVpathIClusterAvailable(p_entry); }, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_SETTING_STS_DISP_FRONT_WIDTH,	 	VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_SETTING_STS_DISP_FRONT_HEIGHT,	 	VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_SETTING_STS_DISP_REAR_WIDTH,		VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_SETTING_STS_DISP_REAR_HEIGHT,	 	VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_SETTING_STS_DISP_ICLUSTER_WIDTH,	VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_SETTING_STS_DISP_ICLUSTER_HEIGHT,	VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE,	
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_HEACON_FRONT_CTL_OUTPUT_ON,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_BOOL, VHAL_EVENT_ACTION_CHANGE, 
	nullptr, [this] (VhalPropertyEntry * const p_entry) -> int32_t { return ActionHeaconAreaSize(p_entry); }});
	(void)property_entries_.emplace(VHAL_PROP_HEACON_FRONT_CTL_OUTPUT_X,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_CHANGE, 
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_HEACON_FRONT_CTL_OUTPUT_Y,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_CHANGE, 
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_HEACON_FRONT_CTL_OUTPUT_W,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_CHANGE, 
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_HEACON_FRONT_CTL_OUTPUT_H,			VhalPropertyEntry{VHAL_ATTR_RW,    VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_CHANGE, 
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_HEACON_FRONT_STS_OUTPUT_X,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE, 
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_HEACON_FRONT_STS_OUTPUT_Y,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE, 
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_HEACON_FRONT_STS_OUTPUT_W,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE, 
	nullptr, nullptr});
	(void)property_entries_.emplace(VHAL_PROP_HEACON_FRONT_STS_OUTPUT_H,			VhalPropertyEntry{VHAL_ATTR_RONLY, VHAL_DATA_TYPE_NUM,  VHAL_EVENT_ACTION_NONE, 
	nullptr, nullptr});
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalPropertyControl::~CVhalPropertyControl(void)
{
	VHAL_LOGV("CVhalPropertyControl is deleted. this=%p", this);
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ：	CVhalMainControl	*p_main		(i)メインコントロールインスタンスポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_****		異常終了
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-N-162
*****************************************************************************/
int32_t CVhalPropertyControl::Initialize(CVhalMainControl * const p_main)
{
	if (nullptr == p_main)
	{
		return VHAL_ERR_PARAM;
	}

	p_main_ = p_main;
	/* [TODO]共通の別の場所に移動（プロセスとして1回コール） */
	/* タイマ機能初期化 */
	int32_t ret{spf_timer_init()};
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-162",fail_ret)};
//	if(true == fail)
//	{
//		ret = fail_ret;
//	}
//#endif
	if (0 != ret)
	{
		VHAL_LOGE("spf_timer_init error ret=%d", ret);
		return ret;
	}

	p_event_route_ = std::make_unique<CVhalEventRoute>();
	ret = p_event_route_->Initialize();
	if (0 > ret)
	{
		VHAL_LOGE("EventRoute Initialize error. ret=%d", ret);
		return ret;
	}
	ret = p_main->RegisterEventSource(p_event_route_.get());
	if (0 > ret)
	{
		VHAL_LOGE("RegisterEventSource error. ret=%d", ret);
		return ret;
	}

	/* Observerクラス初期化 */
	p_observer_control_ = std::make_unique<CVhalObserverControl>();
	ret = p_observer_control_->Initialize();
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGW("CVhalObserverControl::Initialize() error. ret=%d", ret);
		p_observer_control_->Finalize();
		p_observer_control_ = nullptr;
		return ret;
	}

	/* FileObserverクラス初期化 */
	p_fileobserver_control_ = std::make_unique<CVhalFileObserverControl>();
	camera_control_.InitializeFileObserver(p_fileobserver_control_.get());
	ret = p_fileobserver_control_->Initialize(p_main_, p_observer_control_.get());
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGW("CVhalFileObserverControl::Initialize() error. ret=%d", ret);
		p_fileobserver_control_->Finalize();
		p_fileobserver_control_ = nullptr;
		return ret;
	}

	/* sysdbクラス初期化 */
	p_sysdbctrl_ = std::make_unique<CVhalSysdbControl>();
	ret = p_sysdbctrl_->Initialize(p_main_);
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGW("CVhalSysdbControl::Initialize() error. ret=%d", ret);
		p_sysdbctrl_->Finalize();
		p_sysdbctrl_ = nullptr;
		return ret;
	}

	/* GPIOクラス初期化 */
	p_gpioctrl_ = std::make_unique<CVhalGpioControl>();
	ret = p_gpioctrl_->Initialize();
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGW("CVhalGpioControl::Initialize() error. ret=%d", ret);
		p_gpioctrl_ = nullptr;
		return ret;
	}

	p_layout_mng_ = std::make_unique<CVhalLayoutManager>();
	ret = camera_control_.Initialize(p_sysdbctrl_.get(), p_layout_mng_.get(), p_gpioctrl_.get());
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGW("CVhalPropertyControlCamera::Initialize() error. ret=%d", ret);
	}

	p_propertyControl_listener_ = std::make_unique<CVhalPropertyControlEventListener>(this, &camera_control_);
	ret = p_layout_mng_->Initialize(p_main_);
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGW("CVhalLayoutManager::Initialize() error. ret=%d", ret);
		return ret;
	}
	ret = p_layout_mng_->RegisterEventListener(p_propertyControl_listener_.get());
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGW("CVhalLayoutManager::RegisterEventListener() error. ret=%d", ret);
		return ret;
	}

	ret = p_fileobserver_control_->RegisterEventListener(p_propertyControl_listener_.get());
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGW("CVhalFileObserverControl::RegisterEventListener() error. ret=%d", ret);
		return ret;
	}

	ret = p_sysdbctrl_->RegisterEventListener(p_propertyControl_listener_.get());
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGW("CVhalSysdbControl::RegisterEventListener() error. ret=%d", ret);
		return ret;
	}

	p_hdcp_auth_mng_ = std::make_unique<CVhalHdcpAuthManager>();
	ret = p_hdcp_auth_mng_->Initialize();
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("CVhalHdcpAuthManager::Initialize() error. ret=%d", ret);
		p_hdcp_auth_mng_ = nullptr;
		return ret;
	}

	/* HDMI制御初期化 */
	ret = hdmi_control_.Initialize(p_sysdbctrl_.get(), p_hdcp_auth_mng_.get());
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGW("CVhalPropertyControlHdmi::Initialize() error. ret=%d", ret);
	}

	/* HDMI接続状態の現状値の取得とプロパティ更新 */
	ChgConnInfoHdmi(DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECTING, true);

	p_movie_control_ = std::make_unique<CVhalMovieControl>();
	ret = p_movie_control_->Initialize(p_main_, this, p_layout_mng_.get());
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGW("CVhalLayoutManager::Initialize() error. ret=%d", ret);
		return ret;
	}

	p_output_listener_ = std::make_unique<CVhalPropertyControlOutputListener>(p_event_route_.get(), p_layout_mng_.get());
	p_renderer_ = std::make_unique<wlrenderer::CWaylandRenderer>();
	ret = p_renderer_->Initialize();
	if (WL_RENDERER_SUCCESS != ret)
	{
		VHAL_LOGW("CWaylandRenderer::Initialize() error. ret=%d", ret);
		return ret;
	}
	ret = p_renderer_->RegisterOutputListener(p_output_listener_.get());
	if (WL_RENDERER_SUCCESS != ret)
	{
		VHAL_LOGW("CWaylandRenderer::RegisterOutputListener() error. ret=%d", ret);
		return ret;
	}

	p_capture_control_ = std::make_unique<CVhalCaptureControl>();
	ret = p_capture_control_->Initialize(p_renderer_.get(), p_main_, this, p_observer_control_.get());
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGW("CVhalCaptureControl::Initialize() error. ret=%d", ret);
		return ret;
	}
	ret = p_capture_control_->RegisterEventListener(p_propertyControl_listener_.get());
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGW("CVhalCaptureReceiver::RegisterEventListener() error. ret=%d", ret);
		return ret;
	}

	p_micon_comm_control_ = std::make_unique<CVhalMiconCommControl>();
	ret = p_micon_comm_control_->Initialize(p_main_, p_layout_mng_.get(), p_sysdbctrl_.get());
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGW("CVhalMiconCommControl::Initialize() error. ret=%d", ret);
		/* NG時も処理継続 */
	}
	else
	{
		/* マイコンレシーバー生成(Misc) */
		p_micon_misc_receiver_ = std::make_unique<CVhalMiconMiscReceiver>();
		ret = p_micon_misc_receiver_->RegisterEventListener(p_propertyControl_listener_.get());
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGW("CVhalMiconMiscReceiver::RegisterEventListener() error. ret=%d", ret);
			return ret;
		}
		ret = p_micon_comm_control_->RegistryReceiver(p_micon_misc_receiver_.get());
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGW("CVhalMiconCommControl::RegistryReceiver(p_micon_misc_receiver_) error. ret=%d", ret);
			return ret;
		}

		/* キャプチャレシーバー生成(Capture) */
		p_capture_tab_receiver_ = std::make_unique<CVhalCaptureTabReceiver>();
		ret = camera_control_.InitializeCaptureTabReceiver(p_capture_tab_receiver_.get());
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGW("CVhalPropertyControlCamera::InitializeCaptureTabReceiver() error. ret=%d", ret);
		}
		ret = p_capture_tab_receiver_->RegisterEventListener(p_propertyControl_listener_.get());
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGW("CVhalCaptureTabReceiver::RegisterEventListener() error. ret=%d", ret);
			return ret;
		}
		ret = p_micon_comm_control_->RegistryReceiver(p_capture_tab_receiver_.get());
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGW("CVhalMiconCommControl::RegistryReceiver(p_capture_tab_receiver_) error. ret=%d", ret);
			return ret;
		}

		/* カメラ種別判別要求送信 */
		ret = SendCameraType();
		if (VHAL_SUCCESS != ret)
		{
			/* 下位層(CVhalMiconCommMiscControl)での失敗 */
			/* 他VideoHAL機能は動作可能なため、処理を継続する(NGリターンしない) */
			VHAL_LOGE("SendCameraType() error. ret=%d", ret);
		}

		/* HDCP認証通信イベントレシーバー生成 */
		p_hdcp_misc_receiver_ = std::make_unique<CVhalHdcpMiscReceiver>();
		ret = p_hdcp_misc_receiver_->RegisterEventListener(p_propertyControl_listener_.get());
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGW("CVhalHdcpMiscReceiver::RegisterEventListener() error. ret=%d", ret);
			return ret;
		}
		ret = p_micon_comm_control_->RegistryReceiver(p_hdcp_misc_receiver_.get());
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGW("CVhalMiconCommControl::RegistryReceiver(p_hdcp_misc_receiver_) error. ret=%d", ret);
			return ret;
		}

		/*画質モード応答レシーバー生成*/
		p_color_adjust_misc_receiver_ = std::make_unique<CVhalColorAdjustMiscReceiver>();
		ret = p_micon_comm_control_->RegistryReceiver(p_color_adjust_misc_receiver_.get());
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGW("CVhalMiconCommControl::RegistryReceiver(p_color_adjust_misc_receiver_) error. ret=%d", ret);
			return ret;
		}

		/*スクリーンショットレシーバー生成*/
		p_screenshot_receiver_ = std::make_unique<CVhalScreenShotReceiver>();
		ret = p_screenshot_receiver_->RegisterEventListener(p_propertyControl_listener_.get());
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGW("CVhalScreenShotReceiver::RegisterEventListener() error. ret=%d", ret);
			return ret;
		}
		ret = p_micon_comm_control_->RegistryReceiver(p_screenshot_receiver_.get());
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGW("CVhalMiconCommControl::RegistryReceiver(p_screenshot_receiver_) error. ret=%d", ret);
			return ret;
		}

		/*HUD歪み補正レシーバー生成*/
		p_hud_screen_receiver_ = std::make_unique<CVhalHudScreenReceiver>();
		if (nullptr == p_hud_screen_receiver_)
		{
			VHAL_LOGW("CVhalHudScreenReceiver::make_unique error.");
			return VHAL_ERR_MEMORY;
		}
		ret = p_micon_comm_control_->RegistryReceiver(p_hud_screen_receiver_.get());
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGW("CVhalMiconCommControl::RegistryReceiver(p_hud_screen_receiver_) error. ret=%d", ret);
			return ret;
		}
	}

	p_micon_most_video_info_ = std::make_unique<CVhalMiconMostVideoInfo>();
	ret = p_micon_most_video_info_->Initialize(p_micon_comm_control_.get(), p_propertyControl_listener_.get());
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGW("CVhalMiconMostVideoInfo::Initialize() error. ret=%d", ret);
	}

	p_color_mng_ = std::make_unique<CVhalColorManager>();
	ret = p_color_mng_->Initialize(p_micon_comm_control_.get(),p_color_adjust_misc_receiver_.get());
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGW("CVhalColorManager::Initialize() error. ret=%d", ret);
		return ret;
	}

	/* カメラ映像モード 定期送信 */
	p_camera_mode_send_item_ = std::make_unique<CVhalCameraModeSendItem>();
	ret = SendIntervalCameraMode(true);
	if (VHAL_SUCCESS != ret)
	{
		/* エラーログは対象関数内で出力 */
		return ret;
	}

	p_environment_mng_ = std::make_unique<CVhalEnvironmentManager>();
	ret = p_environment_mng_->Initialize();
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGW("CVhalEnvironmentManager Initialize error. ret=%d", ret);
		return ret;
	}

	/* 画面キャプチャ制御生成 */
	p_screen_shot_micon_ = std::make_unique<CVhalScreenShotMicon>();
	if (nullptr != p_screen_shot_micon_)
	{
		ret = p_screen_shot_micon_->Initialize(p_main_, p_layout_mng_.get(), p_micon_comm_control_.get());
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGW("CVhalScreenShotMicon::Initialize() error. ret=%d", ret);
			return ret;
		}	
		else
		{
			ret = p_screen_shot_micon_->RegisterEventListener(p_propertyControl_listener_.get());
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGW("CVhalScreenShotMicon::RegisterEventListener() error. ret=%d", ret);
				return ret;
			}
		}
	}

	/* サーフェスキャプチャ イベントスレッド起動 */
	p_screen_shot_ = std::make_unique<CVhalScreenShot>();
	ret = p_screen_shot_->Initialize(p_main_, p_layout_mng_.get());
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGW("CVhalScreenShot::Initialize() error. ret=%d", ret);
	}

	for (auto itr_prop = property_entries_.begin(); itr_prop != property_entries_.end(); ++itr_prop)
	{
		VhalPropertyEntry &entry{itr_prop->second};

		if (VHAL_DATA_TYPE_STR == entry.GetType())
		{
			entry.AllocateStrPrev("");
			entry.AllocateStrCurrent("");
		}

		/* 特別にデフォルト値設定が必要なもの */
		if ((itr_prop->first.compare(VHAL_PROP_MUTE_FRONT_CTL_BACK) == 0) ||
			(itr_prop->first.compare(VHAL_PROP_MUTE_FRONT_STS_BACK) == 0))
		{
			entry.SetBoolCurrent(true);
		}
		else if ((itr_prop->first.compare(VHAL_PROP_SETTING_CTL_DAY_NIGHT) == 0) ||
			     (itr_prop->first.compare(VHAL_PROP_SETTING_STS_DAY_NIGHT) == 0))
		{
			entry.SetNumCurrent(static_cast<int64_t>(VHAL_SETTING_NIGHT));
		}
		else if ((itr_prop->first.compare(VHAL_PROP_SETTING_CTL_THEME_COLOR) == 0) ||
			     (itr_prop->first.compare(VHAL_PROP_SETTING_STS_THEME_COLOR) == 0))
		{
			entry.SetNumCurrent(VHAL_THEME_COLOR_AUTO_DARK);
		}
		else if ((itr_prop->first.compare(VHAL_PROP_VSRC_FRONT_CTL_ID) == 0) ||
			     (itr_prop->first.compare(VHAL_PROP_VSRC_FRONT_STS_ID) == 0))
		{
			entry.SetNumCurrent(static_cast<int64_t>(VHAL_VSRC_ID_OTHER));
		}
		else if (itr_prop->first.compare(VHAL_PROP_FAUTH_STS_RSE_RSLT) == 0)
		{
			int32_t const num_current{p_hdcp_auth_mng_->GetHdcpInitStatus()};
			entry.SetNumCurrent(static_cast<int64_t>(num_current));
		}
		else if ((itr_prop->first.compare(VHAL_PROP_SETTING_CTL_CNCT_RSE) == 0) ||
			     (itr_prop->first.compare(VHAL_PROP_SETTING_STS_CNCT_RSE) == 0))
		{
			entry.SetNumCurrent(VHAL_CONNECTED_RSE_INVALID);
		}
		else if ((itr_prop->first.compare(VHAL_PROP_SETTING_CTL_CNCT_CAMERA) == 0) ||
			     (itr_prop->first.compare(VHAL_PROP_SETTING_STS_CNCT_CAMERA) == 0))
		{
			entry.SetNumCurrent(VHAL_CONNECTED_CAMERA_INVALID);
		}
		else
		{
			/* 処理なし */
		}
	}

	/* 画質調整STEP値を初期値に設定 */
	ret = UpdateStatusColorStep(COLOR_STEP_TYPE_ALL, false);
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("UpdateStatusColorStep error. ret=%d", ret);
	}

	/* レイヤ構成ファイル読み込み */
	ret = p_layout_mng_->InitLayout("/opt/dc-ivi-pf/share/layer_config.json");
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("InitLayout error. ret=%d vpath=opt/dc-ivi-pf/share/layer_config.json", ret);
		return ret;
	}

	/* Muteの初期化はレイアウトが決まってから */
	constexpr bool mute_init{true};
	p_mute_ = std::make_unique<CVhalMute>();
	ret = p_mute_->Initialize(p_main_, p_layout_mng_.get(), p_gpioctrl_.get(), p_renderer_.get(), p_micon_most_video_info_.get(), mute_init);
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGW("CVhalMute Initialize error. ret=%d", ret);
	}
	else
	{
		ret = camera_control_.InitializeMute(p_mute_.get());
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGW("CVhalPropertyControlCamera::InitializeMute() error. ret=%d", ret);
		}
		ret = p_mute_->RegisterEventListener(p_propertyControl_listener_.get());
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGW("CVhalMute::RegisterEventListener() error. ret=%d", ret);
		}
	}

	/* HUD表示制御生成はレイアウトが決定してからおこなう */
	p_hud_screen_controller_ = std::make_unique<CVhalHudScreenController>();
	if (nullptr == p_hud_screen_controller_)
	{
		VHAL_LOGW("CVhalHudScreenController::make_unique error.");
		return VHAL_ERR_MEMORY;
	}
	else
	{
		ret = p_hud_screen_controller_->Initialize(p_layout_mng_.get(), p_renderer_.get());
		if (VHAL_SUCCESS == ret)
		{
			/* HUD表示制御をHUDレシーバーに登録 */
			ret = p_hud_screen_receiver_->RegisterHudScreenController(p_hud_screen_controller_.get());
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGW("CVhalHudScreenReceiver::RegisterHudScreenController() error. ret=%d", ret);
				return ret;
			}
		}
		else
		{
			VHAL_LOGW("CVhalHudScreenController::Initialize() error. ret=%d", ret);
			return ret;
		}
	}

	for (auto itr_prop = property_entries_.begin(); itr_prop != property_entries_.end(); ++itr_prop)
	{
		if ((itr_prop->first.compare(VHAL_PROP_MUTE_FRONT_CTL_DISP) == 0) ||
			(itr_prop->first.compare(VHAL_PROP_MUTE_FRONT_STS_DISP) == 0))
		{
			VhalPropertyEntry &entry{itr_prop->second};
			entry.SetBoolCurrent(mute_init);
		}
	}

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	終了処理（内部リソースの解放）
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControl::Finalize(void)
{
	for (auto itr_prop = property_entries_.begin(); itr_prop != property_entries_.end(); ++itr_prop)
	{
		VhalPropertyEntry &entry{itr_prop->second};

		if (VHAL_DATA_TYPE_STR == entry.GetType())
		{
			if (true == entry.StrPrevIsAllocated())
			{
				entry.FreeStrPrev();
			}
			if (true == entry.StrCurrentIsAllocated())
			{
				entry.FreeStrCurrent();
			}
		}
	}

	if (nullptr != p_screen_shot_micon_)
	{
		p_screen_shot_micon_->Finalize();
		p_screen_shot_micon_ = nullptr;
	}

	if (nullptr != p_screen_shot_)
	{
		p_screen_shot_->Finalize();
		p_screen_shot_ = nullptr;
	}

	/* HUD制御終了 */
	if (nullptr != p_hud_screen_controller_)
	{
		p_hud_screen_controller_->Finalize();
		p_hud_screen_controller_ = nullptr;
	}

	/* HDMI制御終了 */
	hdmi_control_.Stop();
	hdmi_control_.Finalize();

	/* カメラ制御終了 */
	camera_control_.Finalize();

	if (nullptr != p_capture_control_)
	{
		/* CaptureControlの終了処理 */
		p_capture_control_->Finalize();
		p_capture_control_ = nullptr;
	}

	if (nullptr != p_color_mng_)
	{
		p_color_mng_->Finalize();
		p_color_mng_ = nullptr;
	}

	if (nullptr != p_micon_most_video_info_)
	{
		p_micon_most_video_info_->Finalize();
		p_micon_most_video_info_ = nullptr;
	}

	if (nullptr != p_micon_comm_control_)
	{
		/* HDCP認証通信イベントレシーバー破棄 */
		if (nullptr != p_hdcp_misc_receiver_)
		{
			const int32_t ret{p_micon_comm_control_->ClearReceiver(p_hdcp_misc_receiver_.get())};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("CVhalMiconCommControl::ClearReceiver error. ret=%d", ret);
			}
			p_hdcp_misc_receiver_->ClearEventListener();
			p_hdcp_misc_receiver_ = nullptr;
		}

		/* 画質モード応答レシーバー破棄 */
		if (nullptr != p_color_adjust_misc_receiver_)
		{
			const int32_t ret{p_micon_comm_control_->ClearReceiver(p_color_adjust_misc_receiver_.get())};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("CVhalMiconCommControl::ClearReceiver error. ret=%d", ret);
			}
			p_color_adjust_misc_receiver_ = nullptr;
		}

		if (nullptr != p_capture_tab_receiver_)
		{
			const int32_t ret{p_micon_comm_control_->ClearReceiver(p_capture_tab_receiver_.get())};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("CVhalMiconCommControl::ClearReceiver error. ret=%d", ret);
			}
			p_capture_tab_receiver_->ClearEventListener();
			p_capture_tab_receiver_ = nullptr;
		}

		if (nullptr != p_micon_misc_receiver_)
		{
			const int32_t ret{p_micon_comm_control_->ClearReceiver(p_micon_misc_receiver_.get())};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("CVhalMiconCommControl::ClearReceiver error. ret=%d", ret);
			}
			p_micon_misc_receiver_->ClearEventListener();
			p_micon_misc_receiver_ = nullptr;
		}
		
		if (nullptr != p_camera_mode_send_item_)
		{
			const int32_t ret{p_micon_comm_control_->SendIntervalEnd(p_camera_mode_send_item_.get())};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("SendIntervalEnd error. ret=%d", ret);
			}
			p_camera_mode_send_item_ = nullptr;

		}

		/* スクリーンショット応答レシーバー破棄 */
		if (nullptr != p_screenshot_receiver_)
		{
			const int32_t ret{p_micon_comm_control_->ClearReceiver(p_screenshot_receiver_.get())};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("CVhalMiconCommControl::ClearReceiver error. ret=%d", ret);
			}
			p_screenshot_receiver_->ClearEventListener();
			p_screenshot_receiver_ = nullptr;
		}

		/* HUD歪み補正レシーバー破棄 */
		if (nullptr != p_hud_screen_receiver_)
		{
			const int32_t ret{p_micon_comm_control_->ClearReceiver(p_hud_screen_receiver_.get())};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("CVhalMiconCommControl::ClearReceiver error. ret=%d", ret);
			}
			p_hud_screen_receiver_->ClearHudScreenController();
			p_hud_screen_receiver_ = nullptr;
		}

		p_micon_comm_control_->Finalize();
		p_micon_comm_control_ = nullptr;
	}

	if (nullptr != p_mute_)
	{
		p_mute_->ClearEventListener();
		p_mute_ = nullptr;
	}

	if (nullptr != p_renderer_)
	{
		p_renderer_->ClearOutputListener();
		p_renderer_ = nullptr;
	}

	if (nullptr != p_output_listener_)
	{
		p_output_listener_ = nullptr;
	}

	if (nullptr != p_movie_control_)
	{
		p_movie_control_ = nullptr;
	}

	if (nullptr != p_hdcp_auth_mng_)
	{
		p_hdcp_auth_mng_ = nullptr;
	}

	if (nullptr != p_layout_mng_)
	{
		p_layout_mng_->ClearEventListener();
		p_layout_mng_ = nullptr;
	}

	if (nullptr != p_propertyControl_listener_)
	{
		p_propertyControl_listener_ = nullptr;
	}

	if (nullptr != p_event_route_)
	{
		p_main_->ClearEventSource(p_event_route_.get());
		p_event_route_ = nullptr;
	}

	if (nullptr != p_gpioctrl_)
	{
		p_gpioctrl_ = nullptr;
	}

	if (nullptr != p_sysdbctrl_)
	{
		p_sysdbctrl_->ClearEventListener();
		p_sysdbctrl_ = nullptr;
	}

	if (nullptr != p_fileobserver_control_)
	{
		p_fileobserver_control_->ClearEventListener();
		p_fileobserver_control_ = nullptr;
	}

	if (nullptr != p_observer_control_)
	{
		p_observer_control_ = nullptr;
	}

	if (nullptr != p_environment_mng_)
	{
		p_environment_mng_.reset(nullptr);
	}

	/* [TODO]共通の別の場所に移動（プロセスとして1回コール） */
	/* タイマ機能終了 */
	spf_timer_deinit();
}

/*****************************************************************************
 処理概要：	プロパティ値の取得（整数型）
 引数    ：	const std::string	&name	(i)プロパティ名
           	int64_t				&value	(o)プロパティ値
 戻り値  ：	処理結果
           		VHAL_ERR_PROPERTY_ENTRY	プロパティエントリー不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::GetValueNumber(const std::string &name, int64_t &value)
{
	int32_t result{VHAL_SUCCESS};
	struct VhalPropertyEntry *p_entry{nullptr};

	p_entry = GetPropertyEntry(name, VHAL_DATA_TYPE_NUM, VHAL_ATTR_READ);
	if (nullptr == p_entry)
	{
		result = VHAL_ERR_PROPERTY_ENTRY;
	}
	else
	{
		if (false == p_entry->HasGenerate())
		{
			value = p_entry->GetNumCurrent();
			VHAL_LOGI("method:GET name:%s, value:%ld", name.c_str(), value);
		}
		else
		{
			std::string query{};
			const auto pos = name.find('?');
			if (pos != std::string::npos)
			{
				query = name.substr(pos + 1U);
				p_entry->SetQuery(&query);
			}

			result = p_entry->Generate(p_entry);

			p_entry->SetQuery(nullptr);
			value = p_entry->GetNumCurrent();
			VHAL_LOGI("method:GET name:%s, query:%s, value:%ld", name.c_str(), query.c_str(), value);
			p_entry->SetNumCurrent(0);
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	プロパティ値の取得（bool型）
 引数    ：	const std::string	&name	(i)プロパティ名
           	bool				&value	(o)プロパティ値
 戻り値  ：	処理結果
           		VHAL_ERR_PROPERTY_ENTRY	プロパティエントリー不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::GetValueBool(const std::string &name, bool &value)
{
	int32_t result{VHAL_SUCCESS};
	struct VhalPropertyEntry *p_entry{nullptr};

	p_entry = GetPropertyEntry(name, VHAL_DATA_TYPE_BOOL, VHAL_ATTR_READ);
	if (nullptr == p_entry)
	{
		result = VHAL_ERR_PROPERTY_ENTRY;
	}
	else
	{
		if (false == p_entry->HasGenerate())
		{
			value = p_entry->GetBoolCurrent();
			VHAL_LOGI("method:GET name:%s, value:%s", name.c_str(), value==true?"true":"false");
		}
		else
		{
			std::string query{};
			const auto pos = name.find('?');
			if (pos != std::string::npos)
			{
				query = name.substr(pos + 1U);
				p_entry->SetQuery(&query);
			}

			result = p_entry->Generate(p_entry);

			p_entry->SetQuery(nullptr);
			value = p_entry->GetBoolCurrent();
			VHAL_LOGI("method:GET name:%s, query:%s, value:%s", name.c_str(), query.c_str(), value==true?"true":"false");
			p_entry->SetBoolCurrent(false);
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	プロパティ値の取得（文字列）
 引数    ：	const std::string	&name	(i)プロパティ名
           	std::string			&value	(o)プロパティ値
 戻り値  ：	処理結果
           		VHAL_ERR_PROPERTY_ENTRY	プロパティエントリー不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::GetValueString(const std::string &name, std::string &value)
{
	int32_t result{VHAL_SUCCESS};
	struct VhalPropertyEntry *p_entry{nullptr};

	p_entry = GetPropertyEntry(name, VHAL_DATA_TYPE_STR, VHAL_ATTR_READ);
	if (nullptr == p_entry)
	{
		result = VHAL_ERR_PROPERTY_ENTRY;
	}
	else
	{
		if (false == p_entry->HasGenerate())
		{
			value = p_entry->GetStrCurrent();
			VHAL_LOGI("method:GET name:%s, value:%s", name.c_str(), value.c_str());
		}
		else
		{
			std::string query{};
			const auto pos = name.find('?');
			if (pos != std::string::npos)
			{
				query = name.substr(pos + 1U);
				p_entry->SetQuery(&query);
			}

			result = p_entry->Generate(p_entry);

			p_entry->SetQuery(nullptr);

			if (true == p_entry->StrCurrentIsAllocated())
			{
				value = p_entry->GetStrCurrent();
				VHAL_LOGI("method:GET name:%s, query:%s, value:%s", name.c_str(), query.c_str(), value.c_str());

				p_entry->FreeStrCurrent();
			}
			else
			{
				VHAL_LOGI("method:GET name:%s, query:%s", name.c_str(), query.c_str());
			}
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	プロパティ値の設定（整数型）
 引数    ：	const std::string	&name	(i)プロパティ名
           	const int64_t		&value	(i)プロパティ値
 戻り値  ：	処理結果
           		VHAL_ERR_PROPERTY_ENTRY	プロパティエントリー不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::SetValueNumber(const std::string &name, const int64_t &value)
{
	struct VhalPropertyEntry *p_entry{nullptr};

	p_entry = GetPropertyEntry(name, VHAL_DATA_TYPE_NUM, VHAL_ATTR_WRITE);
	if (nullptr == p_entry)
	{
		return VHAL_ERR_PROPERTY_ENTRY;
	}

	VHAL_LOGI("method:SET name:%s, current=%ld, value:%ld, event_action=%d", name.c_str(), p_entry->GetNumCurrent(), value, p_entry->GetEventAction());

	int32_t result{VHAL_SUCCESS};

	if ((static_cast<int32_t>(VHAL_EVENT_ACTION_WRITE) == p_entry->GetEventAction()) ||
		((static_cast<int32_t>(VHAL_EVENT_ACTION_CHANGE) == p_entry->GetEventAction()) && (value != p_entry->GetNumCurrent())))
	{
		p_entry->SetNumPrev(p_entry->GetNumCurrent());
		p_entry->SetNumCurrent(value);

		p_entry->SetUpdated(true);
		if (true == p_entry->HasAction())
		{
			std::string query{};
			const auto pos = name.find('?');
			if (pos != std::string::npos)
			{
				query = name.substr(pos + 1U);
				p_entry->SetQuery(&query);
			}

			VHAL_LOGV("[%s] action function call", name.c_str());
			result = p_entry->Action(p_entry);

			p_entry->SetQuery(nullptr);
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	プロパティ値の設定（bool型）
 引数    ：	const std::string	&name	(i)プロパティ名
           	const bool			&value	(i)プロパティ値
 戻り値  ：	処理結果
           		VHAL_ERR_PROPERTY_ENTRY	プロパティエントリー不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::SetValueBool(const std::string &name, const bool &value)
{
	struct VhalPropertyEntry *p_entry{nullptr};

	p_entry = GetPropertyEntry(name, VHAL_DATA_TYPE_BOOL, VHAL_ATTR_WRITE);
	if (nullptr == p_entry)
	{
		return VHAL_ERR_PROPERTY_ENTRY;
	}

	VHAL_LOGI("method:SET name=%s, current=%s, value=%s, event_action=%d", name.c_str(), p_entry->GetBoolCurrent()==true?"true":"false", value==true?"true":"false", p_entry->GetEventAction());

	int32_t result{VHAL_SUCCESS};

	if ((static_cast<int32_t>(VHAL_EVENT_ACTION_WRITE) == p_entry->GetEventAction()) ||
		((static_cast<int32_t>(VHAL_EVENT_ACTION_CHANGE) == p_entry->GetEventAction()) && (value != p_entry->GetBoolCurrent())))
	{
		p_entry->SetBoolPrev(p_entry->GetBoolCurrent());
		p_entry->SetBoolCurrent(value);

		p_entry->SetUpdated(true);
		if (true == p_entry->HasAction())
		{
			std::string query{};
			const auto pos = name.find('?');
			if (pos != std::string::npos)
			{
				query = name.substr(pos + 1U);
				p_entry->SetQuery(&query);
			}

			VHAL_LOGV("[%s] action function call", name.c_str());
			result = p_entry->Action(p_entry);

			p_entry->SetQuery(nullptr);
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	プロパティ値の設定（文字列）
 引数    ：	const std::string	&name	(i)プロパティ名
           	const std::string	&value	(i)プロパティ値
 戻り値  ：	処理結果
           		VHAL_ERR_PROPERTY_ENTRY	プロパティエントリー不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::SetValueString(const std::string &name, const std::string &value)
{
	struct VhalPropertyEntry *p_entry{nullptr};

	p_entry = GetPropertyEntry(name, VHAL_DATA_TYPE_STR, VHAL_ATTR_WRITE);
	if (nullptr == p_entry)
	{
		return VHAL_ERR_PROPERTY_ENTRY;
	}

	VHAL_LOGI("method:SET name=%s, current=%s, value=%s, event_action=%d", name.c_str(), p_entry->GetStrCurrent().c_str(), value.c_str(), p_entry->GetEventAction());

	int32_t result{VHAL_SUCCESS};

	if ((static_cast<int32_t>(VHAL_EVENT_ACTION_WRITE) == p_entry->GetEventAction()) ||
		((static_cast<int32_t>(VHAL_EVENT_ACTION_CHANGE) == p_entry->GetEventAction()) && (0 != value.compare(p_entry->GetStrCurrent()))))
	{
		p_entry->SetStrPrev(p_entry->GetStrCurrent());
		p_entry->SetStrCurrent(value);

		p_entry->SetUpdated(true);
		if (true == p_entry->HasAction())
		{
			VHAL_LOGV("[%s] action function call", name.c_str());
			result = p_entry->Action(p_entry);
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	プロパティ値の設定完了。複数SetValueの最後に呼び出す。
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControl::SetValueComplete(void)
{
	std::vector<std::string> names{};
	VHAL_LOGV_IN();

	/* updatedなプロパティ名を集める  */
	for (auto &property_entry : property_entries_)
	{
		VhalPropertyEntry &entry{property_entry.second};

		if (true == entry.GetUpdated())
		{
			names.push_back(property_entry.first);
			entry.SetUpdated(false);
		}
	}

	if (0U != publish_order_property_.size())
	{
		(void)names.insert(names.cend(), publish_order_property_.begin(), publish_order_property_.end());
		publish_order_property_.clear();
	}

	if (0U != names.size())
	{
		p_main_->UpdatedProperties(names);
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	プロパティエントリーの取得
 引数    ：	const std::string	&name	(i)プロパティ名
           	uint32_t			type	(i)データ型
           	uint32_t			attr	(i)アクセス属性
 戻り値  ：	プロパティエントリーポインタ
*****************************************************************************/
struct VhalPropertyEntry *CVhalPropertyControl::GetPropertyEntry(const std::string &name, const uint32_t type, const uint32_t attr)
{
	VHAL_LOGV_IN();

	/* クエリー部分を除外してプロパティエントリーの検索する。 */
	std::string search_name{};

	const auto pos = name.find('?');
	if (pos == std::string::npos)
	{
		search_name = name;
	}
	else
	{
		search_name = name.substr(0U, pos);
	}

	const auto itr_map = property_entries_.find(search_name);
	if (itr_map == property_entries_.end())
	{
		VHAL_LOGE("property [%s] is not supported.", name.c_str());
		return nullptr;
	}
	struct VhalPropertyEntry &entry{itr_map->second};

	if (pos != std::string::npos)
	{
		if ((false == entry.HasGenerate()) && (false == entry.HasAction()))
		{
			VHAL_LOGW("property [%s] does not support query. It is discarded.", name.c_str());
		}
	}

	if (entry.GetType() != type)
	{
		VHAL_LOGE("property [%s] type mismatch.", name.c_str());
		return nullptr;
	}

	if ((VHAL_ATTR_ANY != attr) && (0U == (entry.GetAttr() & attr)))
	{
		VHAL_LOGE("property [%s] attribute error.", name.c_str());
		return nullptr;
	}

	struct VhalPropertyEntry * const p_entry{&entry};

	VHAL_LOGV_OUT();
	return p_entry;
}


/*****************************************************************************
 処理概要：	前席映像パス切替機能Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionVpathFrontCurrent(const struct VhalPropertyEntry * const p_entry)
{
	int32_t ret{VHAL_SUCCESS};
	uint32_t result{VHAL_VPATH_STS_SUCCESS};
	bool available_hdmi{false};
	std::string front_path{};

	VHAL_LOGV_IN();
	VHAL_LOGI("front vpath [%s] -> [%s]", p_entry->GetStrPrev().c_str(), p_entry->GetStrCurrent().c_str());

	ret = p_layout_mng_->GetFrontPath(front_path);
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("GetFrontPath error.");
		result = VHAL_VPATH_STS_FAILED;
	}
	else if (0 != front_path.compare(p_entry->GetStrCurrent()))
	{
		/* 映像pathチェック */
		const bool available{IsValidVideoPathAvailable(p_entry->GetStrCurrent(), VIDEO_OUTPUT_TARGET_FRONT)};
		if (false == available)
		{
			VHAL_LOGE("IsValidVideoPathAvailable error.");
			result = VHAL_VPATH_STS_FAILED;
		}
		else
		{
			/* HDMI切り替え判定 */
			if ("HDMI" == p_entry->GetStrCurrent())
			{
				/* サーフェス生成状態取得 */
				available_hdmi = p_layout_mng_->IsValidSurfaceAvailable(p_entry->GetStrCurrent(), VIDEO_OUTPUT_TARGET_FRONT);

				/* HDMI再生可能状態をチェック */
				int32_t connect{hdmi_control_.kUnknown};
				int32_t format{hdmi_control_.kUnknown};
				uint32_t width{0U};
				uint32_t height{0U};
				const bool hdmi_available{hdmi_control_.Available(connect, format, width, height)};
				if (true == hdmi_available)
				{
					ret = hdmi_control_.Play(CVhalPropertyControlHdmi::CTRL_TARGET_FRONT, p_layout_mng_.get(), p_capture_control_.get(), width, height);
					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("hdmi play error. ret=%d", ret);
						result = VHAL_VPATH_STS_FAILED;
					}
				}
				else
				{
					VHAL_LOGE("Invalid HDMI Status. connect=%d, format=%d[width=%d, height=%d]", connect, format, width, height);
					result = VHAL_VPATH_STS_FAILED;
				}
			}
			else if ("HDMI" == p_entry->GetStrPrev())
			{
				hdmi_control_.Stop(CVhalPropertyControlHdmi::CTRL_TARGET_FRONT);
			}
			else
			{
				/* 処理なし */
			}
		}

		if (VHAL_VPATH_STS_SUCCESS == result)
		{
			/* 前席映像パス切替 */
			ret = p_layout_mng_->SetFrontPath(p_entry->GetStrCurrent());
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("SetFrontPath error. ret=%d vpath=%s", ret, p_entry->GetStrCurrent().c_str());
				result = VHAL_VPATH_STS_FAILED;
			}
			else
			{
				/* 接続されているRSE種別取得(MOST) */
				int32_t connected_rse{VHAL_CONNECTED_RSE_INVALID};
				p_micon_comm_control_->GetConnectedRse(connected_rse);
				if ((VHAL_CONNECTED_RSE_FULL == connected_rse) || (VHAL_CONNECTED_RSE_DOP == connected_rse))
				{
					/* フルRSE 又は用品RSE接続時に有効 */
					/* 前席映像パス設定(MOST) */
					ret = p_micon_most_video_info_->SetFrontPath(p_entry->GetStrCurrent().c_str());
					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("CVhalMiconMostVideoInfo::SetFrontPath() error. ret=%d", ret);
					}
				}

				/* 前席映像パスの設定(画質調整) */
				const bool set_path{p_color_mng_->SetFrontPath(p_entry->GetStrCurrent())};
				if (true == set_path)
				{
					ret = p_color_mng_->ApplyAdjustment(false);
					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("ApplyAdjustment error. ret=%d", ret);
					}
				}

				/* 映像パスに合わせて入力（クリッピング）サイズプロパティも更新する */
				ret = VHAL_SUCCESS;
				if (!p_entry->GetStrCurrent().empty())
				{
					ret = p_layout_mng_->ClearFrontClippingSize();
				}
				
				if (VHAL_SUCCESS == ret)
				{
					struct VhalClippingPropValues clipping_current{};
					clipping_current.enable = kDefaultClippingEnable;
					clipping_current.rect[VHAL_RECTANGLE_X] = 0;
					clipping_current.rect[VHAL_RECTANGLE_Y] = 0;
					clipping_current.rect[VHAL_RECTANGLE_W] = 0;
					clipping_current.rect[VHAL_RECTANGLE_H] = 0;
					VHAL_LOGI("  src rect %d(%d,%d,%d,%d)", clipping_current.enable, clipping_current.rect[VHAL_RECTANGLE_X], clipping_current.rect[VHAL_RECTANGLE_Y], clipping_current.rect[VHAL_RECTANGLE_W], clipping_current.rect[VHAL_RECTANGLE_H]);
					ret = UpdateVpathClipping(front_clipping_control_, clipping_current);
					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("Front UpdateVpathClipping error. ret=%d", ret);
					}
					ret = UpdateVpathClipping(front_clipping_status_, clipping_current);
					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("Front UpdateVpathClipping error. ret=%d", ret);
					}
				}
				else
				{
					VHAL_LOGE("ClearFrontClippingSize error. ret=%d", ret);
				}

				/* 映像パスに合わせて出力サイズプロパティも更新する */
				int32_t screen_id{};
				ret = p_layout_mng_->GetScreenIdFront(screen_id);
				if (VHAL_SUCCESS == ret)
				{
					struct VhalRectanglePropValues output_current{};
					output_current.rect[VHAL_RECTANGLE_X] = 0;
					output_current.rect[VHAL_RECTANGLE_Y] = 0;
					ret = p_layout_mng_->GetScreenSize(screen_id, output_current.rect[VHAL_RECTANGLE_W], output_current.rect[VHAL_RECTANGLE_H]);
					if (VHAL_SUCCESS == ret)
					{
						VHAL_LOGI("  rect (%d,%d,%d,%d)", output_current.rect[0], output_current.rect[1], output_current.rect[2], output_current.rect[3]);

						ret = p_layout_mng_->SetFrontDestRectangle(output_current.rect[0], output_current.rect[1], output_current.rect[2], output_current.rect[3]);
						if (VHAL_SUCCESS == ret)
						{
							ret = UpdateVpathOutput(front_output_status_, output_current);
							if (VHAL_SUCCESS != ret)
							{
								VHAL_LOGE("Front UpdateVpathOutput error. ret=%d", ret);
							}
						}
						else
						{
							VHAL_LOGE("SetFrontDestRectangle error. ret=%d", ret);
						}
					}
					else
					{
						VHAL_LOGE("GetScreenSize error. ret=%d", ret);
					}
				}
				else
				{
					VHAL_LOGE("GetScreenIdFront error. ret=%d", ret);
				}

				/* 映像パスに合わせて 映像可視状態のプロパティを更新する */
				ret = VHAL_SUCCESS;
				if (!p_entry->GetStrCurrent().empty())
				{
					ret = p_layout_mng_->SetFrontVisibility(kDefaultVisibility);

				}

				if (VHAL_SUCCESS == ret)
				{
					ret = UpdateStatusVisible(p_entry->GetStrCurrent(), VIDEO_OUTPUT_TARGET_FRONT, true);
					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("Front UpdateStatusVisible error. ret=%d", ret);
					}
				}
				else
				{
					VHAL_LOGE("SetFrontVisibility error. ret=%d", ret);
				}

				/* 映像不透明度状態のプロパティを更新する */
				uint32_t opacity{kDefaultOpacity};
				if ("HDMI" == p_entry->GetStrCurrent())
				{
					opacity = 100U;		/* HDMIは表示される前提（サーフェス生成前の可能性があるため） */
					ret = VHAL_SUCCESS;
				}
				else
				{
					ret = VHAL_SUCCESS;
					if (!p_entry->GetStrCurrent().empty())
					{
						ret = p_layout_mng_->SetFrontOpacity(opacity);
					}

					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("SetFrontOpacity error. ret=%d", ret);
					}
				}
				if (VHAL_SUCCESS == ret)
				{
					struct VhalPropertyEntry* p_status{GetPropertyEntry(VHAL_PROP_VPATH_FRONT_CTL_OPACITY, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
					if (nullptr != p_status)
					{
						p_status->SetNumCurrent(static_cast<int64_t>(opacity));
						p_status->SetUpdated(true);
					}
					p_status = GetPropertyEntry(VHAL_PROP_VPATH_FRONT_STS_OPACITY, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);
					if (nullptr != p_status)
					{
						p_status->SetNumCurrent(static_cast<int64_t>(opacity));
						p_status->SetUpdated(true);
					}
				}

				/* ワイド設定 状態プロパティ更新 */
				ret = VHAL_SUCCESS;
				if (!p_entry->GetStrCurrent().empty())
				{
					ret = p_layout_mng_->SetFrontWideMode(kDefaultWideMode);
				}
				
				if (VHAL_SUCCESS == ret)
				{
					ret = UpdateStatusWideMode(VIDEO_OUTPUT_TARGET_FRONT);
					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("Front UpdateStatusWideMode error. ret=%d", ret);
					}
				}
				else
				{
					VHAL_LOGE("SetFrontWideMode error. ret=%d", ret);
				}
			}
		}

	}
	else
	{
		/* HDMI切り替え判定（HDMIからHDMI） */
		/* PRBL-764:映像パス切替結果更新のため、サーフェス生成状態を取得する */
		if (0 == (p_entry->GetStrCurrent()).compare("HDMI"))
		{
			/* サーフェス生成状態取得 */
			available_hdmi = p_layout_mng_->IsValidSurfaceAvailable(p_entry->GetStrCurrent(), VIDEO_OUTPUT_TARGET_FRONT);

			/* HDMI再生可能状態をチェック */
			int32_t connect{hdmi_control_.kUnknown};
			int32_t format{hdmi_control_.kUnknown};
			uint32_t width{0U};
			uint32_t height{0U};
			const bool hdmi_available{hdmi_control_.Available(connect, format, width, height)};
			if (false == hdmi_available)
			{
				VHAL_LOGE("Invalid HDMI Status. connect=%d, format=%d[width=%d, height=%d]", connect, format, width, height);
				result = VHAL_VPATH_STS_FAILED;
			}
		}
	}

	/* 映像パス切替結果更新 */
	bool vpath_status_update{true};
	if ((VHAL_VPATH_STS_SUCCESS == result) && (0 == (p_entry->GetStrCurrent()).compare("HDMI")))
	{
		/* HDMIの場合は更新しない。ただしサーフェス生成済みであれば更新する */
		vpath_status_update = false;
		if (true == available_hdmi)
		{
			/* HDMIサーフェス生成済み */
			vpath_status_update = true;
		}
	}
	else
	{
		/* その他（HDMI以外または異常）の場合は更新する */
		vpath_status_update = true;
	}
	if (vpath_status_update)
	{
		VHAL_LOGD("update_name=%s, result=%d", VHAL_PROP_VPATH_FRONT_STS_CURRENT.c_str(), result);
		struct VhalPropertyEntry* p_status{GetPropertyEntry(VHAL_PROP_VPATH_FRONT_STS_CURRENT, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
		if (nullptr != p_status)
		{
			p_status->SetNumCurrent(static_cast<int64_t>(result));
			/* 通知処理の順序を指定するためにSetUpdated()ではなく、publish_order_property_に設定する */
			publish_order_property_.push_back(VHAL_PROP_VPATH_FRONT_STS_CURRENT);
		}

		/* HDMI切替失敗したら、HDMI接続を未接続にする */
		if ((VHAL_VPATH_STS_SUCCESS != result) && (0 == (p_entry->GetStrCurrent()).compare("HDMI")))
		{
			p_status = GetPropertyEntry(VHAL_PROP_SETTING_STS_CNCT_HDMI, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);
			if (nullptr != p_status)
			{
				p_status->SetNumCurrent(VHAL_CONNECTED_HDMI_NONE);
				/* 通知処理の順序を指定するためにSetUpdated()ではなく、publish_order_property_に設定する */
				/* VHAL_PROP_VPATH_FRONT_STS_CURRENTより後に通知を実施する */
				publish_order_property_.push_back(VHAL_PROP_SETTING_STS_CNCT_HDMI);
			}
		}
	}
	else
	{
		VHAL_LOGD("update_name=%s(Wait. NotifyCreatedSurface)", VHAL_PROP_VPATH_FRONT_STS_CURRENT.c_str());
	}

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	後席映像パス切替機能Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionVpathRearCurrent(const struct VhalPropertyEntry * const p_entry)
{
	int32_t ret{VHAL_SUCCESS};
	uint32_t result{VHAL_VPATH_STS_SUCCESS};
	bool available_hdmi{false};
	std::string rear_path{};

	VHAL_LOGV_IN();
	VHAL_LOGI("rear vpath [%s] -> [%s]", p_entry->GetStrPrev().c_str(), p_entry->GetStrCurrent().c_str());

	ret = p_layout_mng_->GetRearPath(rear_path);
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("GetRearPath error.");
		result = VHAL_VPATH_STS_FAILED;
	}
	else if (0 != rear_path.compare(p_entry->GetStrCurrent()))
	{
		/* 映像pathチェック */
		const bool available{IsValidVideoPathAvailable(p_entry->GetStrCurrent(), VIDEO_OUTPUT_TARGET_REAR)};
		if (false == available)
		{
			VHAL_LOGE("IsValidVideoPathAvailable error.");
			result = VHAL_VPATH_STS_FAILED;
		}
		else
		{
			/* 後席スクリーン有効チェック */
			int32_t screen_id{};
			ret = p_layout_mng_->GetScreenIdRear(screen_id);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("GetScreenIdRear error. ret=%d", ret);
				result = VHAL_VPATH_STS_FAILED;
			}
			else
			{
				const bool rear_available{p_layout_mng_->IsScreenAvailable(screen_id)};
				if (false == rear_available)
				{
					VHAL_LOGE("rear screen is not available.");
					result = VHAL_VPATH_STS_FAILED;
				}
				else
				{
					/* HDMI切り替え判定 */
					if ("HDMI" == p_entry->GetStrCurrent())
					{
						/* サーフェス生成状態取得 */
						available_hdmi = p_layout_mng_->IsValidSurfaceAvailable(p_entry->GetStrCurrent(), VIDEO_OUTPUT_TARGET_REAR);

						/* HDMI再生可否判定 */
						int32_t connect{hdmi_control_.kUnknown};
						int32_t format{hdmi_control_.kUnknown};
						uint32_t width{0U};
						uint32_t height{0U};
						const bool rear_hdmi_available{hdmi_control_.Available(connect, format, width, height)};
						if (true == rear_hdmi_available)
						{
							ret = hdmi_control_.Play(CVhalPropertyControlHdmi::CTRL_TARGET_REAR, p_layout_mng_.get(), p_capture_control_.get(), width, height);
							if (VHAL_SUCCESS != ret)
							{
								VHAL_LOGE("hdmi play error. ret=%d", ret);
								result = VHAL_VPATH_STS_FAILED;
							}
						}
						else
						{
							VHAL_LOGE("Invalid HDMI Status. connect=%d, format=%d[width=%d, height=%d]", connect, format, width, height);
							result = VHAL_VPATH_STS_FAILED;
						}
					}
					else if ("HDMI" == p_entry->GetStrPrev())
					{
						hdmi_control_.Stop(CVhalPropertyControlHdmi::CTRL_TARGET_REAR);
					}
					else
					{
						/* 処理なし */
					}

					if (VHAL_VPATH_STS_SUCCESS == result)
					{
						/* 後席映像パス切替処理 */
						ret = p_layout_mng_->SetRearPath(p_entry->GetStrCurrent());
						if (VHAL_SUCCESS != ret)
						{
							VHAL_LOGE("SetRearPath error. ret=%d vpath=%s", ret, p_entry->GetStrCurrent().c_str());
							result = VHAL_VPATH_STS_FAILED;
						}
						else
						{

							/* 後席映像パス設定(MOST) */
							p_micon_most_video_info_->SetRearPath(p_entry->GetStrCurrent());

							/* 映像パスに合わせて出力サイズも更新する */
							struct VhalRectanglePropValues output_current{};
							output_current.rect[VHAL_RECTANGLE_X] = 0;
							output_current.rect[VHAL_RECTANGLE_Y] = 0;
							ret = p_layout_mng_->GetScreenSize(screen_id, output_current.rect[VHAL_RECTANGLE_W], output_current.rect[VHAL_RECTANGLE_H]);
							if (VHAL_SUCCESS == ret)
							{
								VHAL_LOGI("  rect (%d,%d,%d,%d)", output_current.rect[0], output_current.rect[1], output_current.rect[2], output_current.rect[3]);

								ret = p_layout_mng_->SetRearDestRectangle(output_current.rect[0], output_current.rect[1], output_current.rect[2], output_current.rect[3]);
								if (VHAL_SUCCESS == ret)
								{
									ret = UpdateVpathOutput(rear_output_status_, output_current);
									if (VHAL_SUCCESS != ret)
									{
										VHAL_LOGE("Rear UpdateVpathOutput error. ret=%d", ret);
									}
								}
								else
								{
									VHAL_LOGE("SetRearDestRectangle error. ret=%d", ret);
								}
							}
							else
							{
								VHAL_LOGE("GetScreenSize error. ret=%d", ret);
							}
					

							/* 映像パスに合わせて 映像可視状態のプロパティを更新する */
							ret = VHAL_SUCCESS;
							if (!p_entry->GetStrCurrent().empty())
							{
								ret = p_layout_mng_->SetRearVisibility(kDefaultVisibility);
							}

							if (VHAL_SUCCESS == ret)
							{
								ret = UpdateStatusVisible(p_entry->GetStrCurrent(), VIDEO_OUTPUT_TARGET_REAR, true);
								if (VHAL_SUCCESS != ret)
								{
									VHAL_LOGE("Rear UpdateStatusVisible error. ret=%d", ret);
								}
							}
							else
							{
								VHAL_LOGE("SetRearVisibility error. ret=%d", ret);
							}

							/* 映像不透明度状態のプロパティを更新する */
							uint32_t opacity{kDefaultOpacity};
							if ("HDMI" == p_entry->GetStrCurrent())
							{
								opacity = 100U;		/* HDMIは表示される前提（サーフェス生成前の可能性があるため） */
								ret = VHAL_SUCCESS;
							}
							else
							{
								ret = VHAL_SUCCESS;
								if (!p_entry->GetStrCurrent().empty())
								{
									ret = p_layout_mng_->SetRearOpacity(opacity);
								}

								if (VHAL_SUCCESS != ret)
								{
									VHAL_LOGE("SetRearOpacity error. ret=%d", ret);
								}
							}
							if (VHAL_SUCCESS == ret)
							{
								struct VhalPropertyEntry* p_status{GetPropertyEntry(VHAL_PROP_VPATH_REAR_CTL_OPACITY, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
								if (nullptr != p_status)
								{
									p_status->SetNumCurrent(static_cast<int64_t>(opacity));
									p_status->SetUpdated(true);
								}
								p_status = GetPropertyEntry(VHAL_PROP_VPATH_REAR_STS_OPACITY, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);
								if (nullptr != p_status)
								{
									p_status->SetNumCurrent(static_cast<int64_t>(opacity));
									p_status->SetUpdated(true);
								}
							}
	
							/* ワイド設定 状態プロパティ更新 */
							ret = VHAL_SUCCESS;
							if (!p_entry->GetStrCurrent().empty())
							{
								ret = p_layout_mng_->SetRearWideMode(kDefaultWideMode);
							}

							if (VHAL_SUCCESS == ret)
							{
								ret = UpdateStatusWideMode(VIDEO_OUTPUT_TARGET_REAR);
								if (VHAL_SUCCESS != ret)
								{
									VHAL_LOGE("Rear UpdateStatusWideMode error. ret=%d", ret);
								}
							}
							else
							{
								VHAL_LOGE("SetRearWideMode error. ret=%d", ret);
							}
						}
					}
				}
			}
		}
	}
	else
	{
		/* HDMI切り替え判定（HDMIからHDMI） */
		/* PRBL-764:映像パス切替結果更新のため、サーフェス生成状態を取得する */
		if (0 == (p_entry->GetStrCurrent()).compare("HDMI"))
		{
			/* サーフェス生成状態取得 */
			available_hdmi = p_layout_mng_->IsValidSurfaceAvailable(p_entry->GetStrCurrent(), VIDEO_OUTPUT_TARGET_REAR);
			
			/* HDMI再生可能状態をチェック */
			int32_t connect{hdmi_control_.kUnknown};
			int32_t format{hdmi_control_.kUnknown};
			uint32_t width{0U};
			uint32_t height{0U};
			const bool hdmi_available{hdmi_control_.Available(connect, format, width, height)};
			if (false == hdmi_available)
			{
				VHAL_LOGE("Invalid HDMI Status. connect=%d, format=%d[width=%d, height=%d]", connect, format, width, height);
				result = VHAL_VPATH_STS_FAILED;
			}
		}
	}

	/* 映像パス切替結果更新 */
	bool vpath_status_update{true};
	if ((VHAL_VPATH_STS_SUCCESS == result) && (0 == (p_entry->GetStrCurrent()).compare("HDMI")))
	{
		/* HDMIの場合は更新しない。ただしサーフェス生成済みであれば更新する */
		vpath_status_update = false;
		if (true == available_hdmi)
		{
			/* HDMIサーフェス生成済み */
			vpath_status_update = true;
		}
	}
	else
	{
		/* その他（HDMI以外または異常）の場合は更新する */
		vpath_status_update = true;
	}
	if (vpath_status_update)
	{
		VHAL_LOGD("update_name=%s, result=%d", VHAL_PROP_VPATH_REAR_STS_CURRENT.c_str(), result);
		struct VhalPropertyEntry* p_status{GetPropertyEntry(VHAL_PROP_VPATH_REAR_STS_CURRENT, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
		if (nullptr != p_status)
		{
			p_status->SetNumCurrent(static_cast<int64_t>(result));
			/* 通知処理の順序を指定するためにSetUpdated()ではなく、publish_order_property_に設定する */
			publish_order_property_.push_back(VHAL_PROP_VPATH_REAR_STS_CURRENT);
		}

		/* HDMI切替失敗したら、HDMI接続を未接続にする */
		if ((VHAL_VPATH_STS_SUCCESS != result) && (0 == (p_entry->GetStrCurrent()).compare("HDMI")))
		{
			p_status = GetPropertyEntry(VHAL_PROP_SETTING_STS_CNCT_HDMI, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);
			if (nullptr != p_status)
			{
				p_status->SetNumCurrent(VHAL_CONNECTED_HDMI_NONE);
				/* 通知処理の順序を指定するためにSetUpdated()ではなく、publish_order_property_に設定する */
				/* VHAL_PROP_VPATH_REAR_STS_CURRENTより後に通知を実施する */
				publish_order_property_.push_back(VHAL_PROP_SETTING_STS_CNCT_HDMI);
			}
		}
	}
	else
	{
		VHAL_LOGD("update_name=%s(Wait. NotifyCreatedSurface)", VHAL_PROP_VPATH_REAR_STS_CURRENT.c_str());
	}

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	カメラ映像パス切替機能Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionVpathCameraCurrent(const struct VhalPropertyEntry * const p_entry)
{
	int32_t	ret{VHAL_SUCCESS};
	uint32_t result{VHAL_VPATH_STS_SUCCESS};
	bool	vpath_status_update{true};
	std::string camera_path{};

	VHAL_LOGV_IN();

	VHAL_LOGI("camera vpath [%s] -> [%s]", p_entry->GetStrPrev().c_str(), p_entry->GetStrCurrent().c_str());

	ret = p_layout_mng_->GetCameraPath(camera_path);
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("GetCameraPath error. ret=%d", ret);
		result = VHAL_VPATH_STS_FAILED;
	}
	else if (0 != camera_path.compare(p_entry->GetStrCurrent()))
	{
		bool	to_camera{false};

		/* カメラ映像pathチェック */
		const bool available{IsValidVideoPathAvailable(p_entry->GetStrCurrent(), VIDEO_OUTPUT_TARGET_CAMERA)};
		if (false == available)
		{
			VHAL_LOGE("IsValidVideoPathAvailable error.");
			result = VHAL_VPATH_STS_FAILED;
		}
		else
		{
			uint32_t sleep_time{64000U};	/* PRBL-644 画質調整の遅延 */

			/* カメラ映像へ切り替え時 */
			if (!p_entry->GetStrCurrent().empty())
			{
				VhalLogCameraPath(__builtin_FUNCTION(), true);

				/* カメラ種別設定処理 */
				ret = SettingCameraCapture();
				if (VHAL_SUCCESS == ret)
				{
					/* カメラ切り替えMUTE対応 */
					if (nullptr != p_mute_)
					{
						p_mute_->SetMuteCameraPath(p_entry->GetStrCurrent());
					}

					to_camera = true;
					/* カメラ映像パス切替処理 */
					ret = camera_control_.SetCameraPath(p_entry->GetStrCurrent());
					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("SetCameraPath error. rer=%d", ret);
						result = VHAL_VPATH_STS_FAILED;
						(void)sleep_time;			/* MISRA C++-2008 Rule 3-4-1対策 */
					}
					else
					{
						/* PRBL-767 画質調整の遅延(カメラ映像への切替120ms) */
						sleep_time = 120000U;
					}
				}
				else
				{
					VHAL_LOGE("Failed to configure camera capturing.");
					result = VHAL_VPATH_STS_FAILED;
					(void)sleep_time;				/* MISRA C++-2008 Rule 3-4-1対策 */
				}
			}
			else
			{
				/* カメラ表示中(センターディスプレイ) */
				VhalLogCameraPath(__builtin_FUNCTION(), false);

				/* カメラ切り替えMUTE対応 */
				if (nullptr != p_mute_)
				{
					p_mute_->SetMuteCameraPath(p_entry->GetStrCurrent());
				}

				/* カメラ映像キャプチャ停止を行う */
				ret = camera_control_.SetCameraPath(p_entry->GetStrCurrent());
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("SettingCameraPath error. ret=%d path=%s", ret, p_entry->GetStrCurrent().c_str());
				}
			}

			if (result == VHAL_VPATH_STS_SUCCESS)
			{
				/* カメラ映像パス切替処理 */
				ret = p_layout_mng_->SetCameraPath(p_entry->GetStrCurrent(), true);
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("SetCameraPath error. ret=%d vpath=%s", ret, p_entry->GetStrCurrent().c_str());
					result = VHAL_VPATH_STS_FAILED;
					(void)sleep_time;				/* MISRA C++-2008 Rule 3-4-1対策 */
				}
				else
				{
					/* PRBL-644/767 画質調整の遅延(カメラ映像への切替120ms/カメラ映像からの切替64ms) */
					ret = usleep(sleep_time);
					if (0 != ret)
					{
						VHAL_LOGE("usleep error. ret=%d", ret);
					}

					/* PRBL-654 カメラ解除時のみRGB画質調整を挟む*/
					if (p_entry->GetStrCurrent().empty())
					{
						/* RGB画質調整用に前席映像パス設定をクリア */
						(void)p_color_mng_->SetFrontPath("");
						/* カメラ映像パスの設定(画質調整) */
						p_color_mng_->SetCameraPath(p_entry->GetStrCurrent());
						ret = p_color_mng_->ApplyAdjustment(false);
						if (VHAL_SUCCESS != ret)
						{
							VHAL_LOGE("ApplyAdjustment error. ret=%d", ret);
						}

						std::string path{};
						ret = p_layout_mng_->GetFrontPath(path);
						if (VHAL_SUCCESS != ret)
						{
							VHAL_LOGE("GetFrontPath error. ret=%d", ret);
						}
						else if (!path.empty())
						{
							/* 前席映像パスの再設定(画質調整) */
							const bool set_path{p_color_mng_->SetFrontPath(path)};
							if (true == set_path)
							{
								ret = p_color_mng_->ApplyAdjustment(false);
								if (VHAL_SUCCESS != ret)
								{
									VHAL_LOGE("ApplyAdjustment error. ret=%d", ret);
								}
							}
						}
						else
						{
							/* 処理なし */
						}
					}
					else
					{
						/* カメラ映像パスの設定(画質調整) */
						p_color_mng_->SetCameraPath(p_entry->GetStrCurrent());
						ret = p_color_mng_->ApplyAdjustment(false);
						if (VHAL_SUCCESS != ret)
						{
							VHAL_LOGE("ApplyAdjustment error. ret=%d", ret);
						}
					}

					/* カメラ映像モード通知の設定 */
					uint8_t CameraVisualMode{kCameraVisualModeOther};
					if (0 == p_entry->GetStrCurrent().compare(VHAL_PATH_CAMERA))
					{
						CameraVisualMode = kCameraVisualModeCamera;
					}

					p_camera_mode_send_item_->SendCameraVisualMode(CameraVisualMode);
					ret = p_micon_comm_control_->Send(*p_camera_mode_send_item_);
					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("CVhalMiconCommControl::Send() error. ret=%d", ret);
					}

					/* 映像パスに合わせて出力サイズも更新する */
					struct VhalRectanglePropValues output_current;
					ret = p_layout_mng_->GetCameraDestRectangle(output_current.rect[VHAL_RECTANGLE_X], output_current.rect[VHAL_RECTANGLE_Y], output_current.rect[VHAL_RECTANGLE_W], output_current.rect[VHAL_RECTANGLE_H]);
					if (VHAL_SUCCESS == ret)
					{
						VHAL_LOGI("  rect (%d,%d,%d,%d)", output_current.rect[VHAL_RECTANGLE_X], output_current.rect[VHAL_RECTANGLE_Y], output_current.rect[VHAL_RECTANGLE_W], output_current.rect[VHAL_RECTANGLE_H]);

						ret = UpdateVpathOutput(camera_output_status_, output_current);
						if (VHAL_SUCCESS != ret)
						{
							VHAL_LOGE("Camera UpdateVpathOutput error. ret=%d", ret);
						}
					}
					else
					{
						VHAL_LOGE("GetCameraDestRectangle error. ret=%d", ret);
					}
				}
			}
		}

		/* カメラに切り替えようとしたが途中でエラーが発生していれば、カメラキャプチャ処理を停止 */
		if (VHAL_VPATH_STS_SUCCESS != result)
		{
			if (true == to_camera)
			{
				/* カメラキャプチャ処理を停止 */
				const std::string	nopath{""};
				ret = camera_control_.SetCameraPath(nopath);
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("SetCameraPath error. ret=%d path=%s", ret, nopath.c_str());
				}
			}
			/* 同期異常MUTEを解除 */
			if (nullptr != p_mute_)
			{
				p_mute_->SetMuteCameraPathFailed();
			}
		}
		else
		{
			/* 切り替え処理を正常に行った場合はMUTE解除後に切り替え完了通知を出す */
			vpath_status_update = false;
		}
	}
	else
	{
		/* 処理なし */
	}

	if (true == vpath_status_update)
	{
		struct VhalPropertyEntry*	p_status{nullptr};
		p_status = GetPropertyEntry(VHAL_PROP_VPATH_CAMERA_STS_CURRENT, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);
		if (nullptr != p_status)
		{
			p_status->SetNumCurrent(static_cast<int64_t>(result));
			p_status->SetUpdated(true);
		}
	}

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	Instrument Cluster 映像パス切替機能Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionVpathIClusterCurrent(const struct VhalPropertyEntry * const p_entry)
{
	int32_t						ret{VHAL_SUCCESS};
	uint32_t					result{VHAL_VPATH_STS_SUCCESS};
	struct VhalPropertyEntry*	p_status{nullptr};
	std::string ic_path{};

	VHAL_LOGV_IN();
	VHAL_LOGI("instrument cluster vpath [%s] -> [%s]", p_entry->GetStrPrev().c_str(), p_entry->GetStrCurrent().c_str());

	ret = p_layout_mng_->GetIClusterPath(ic_path);
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("GetIClusterPath error. ret=%d", ret);
		result = VHAL_VPATH_STS_FAILED;
	}
	else if (0 != ic_path.compare(p_entry->GetStrCurrent()))
	{
		/* Instrument Cluster映像pathチェック */
		const bool available{IsValidVideoPathAvailable(p_entry->GetStrCurrent(), VIDEO_OUTPUT_TARGET_IC)};
		if (false == available)
		{
			VHAL_LOGE("IsValidVideoPathAvailable error.");
			result = VHAL_VPATH_STS_FAILED;
		}

		/* Instrument Clusterスクリーン有効チェック */
		int32_t screen_id{};
		ret = p_layout_mng_->GetScreenIdICluster(screen_id);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetScreenIdICluster error. ret=%d", ret);
			result = VHAL_VPATH_STS_FAILED;
		}

		const bool ic_available{p_layout_mng_->IsScreenAvailable(screen_id)};
		if (false == ic_available)
		{
			VHAL_LOGE("ICluster screen is not available.");
			result = VHAL_VPATH_STS_FAILED;
		}

		if (VHAL_VPATH_STS_SUCCESS == result)
		{
			std::string	setting_path{};
			ret = p_layout_mng_->GetIClusterPath(setting_path);
			if (VHAL_SUCCESS != ret)
			{
				result = VHAL_VPATH_STS_FAILED;
				VHAL_LOGE("GetIClusterPath error. ret=%d", ret);
			}
			else if (0 == p_entry->GetStrCurrent().compare(setting_path))
			{
				VHAL_LOGI("Path no Change.");
			}
			else
			{
				/* Instrument Cluster 映像パス切替 */
				ret = p_layout_mng_->SetIClusterPath(p_entry->GetStrCurrent());
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("SetIClusterPath error. ret=%d vpath=%s", ret, p_entry->GetStrCurrent().c_str());
					result = VHAL_VPATH_STS_FAILED;
				}
				else
				{
					/* 映像パスに合わせて入力（クリッピング）サイズプロパティも更新する */
					ret = VHAL_SUCCESS;
					if (!p_entry->GetStrCurrent().empty())
					{
						ret = p_layout_mng_->ClearIClusterClippingSize();
					}

					if (VHAL_SUCCESS == ret)
					{
						struct VhalClippingPropValues clipping_current{};
						clipping_current.enable = kDefaultClippingEnable;
						clipping_current.rect[VHAL_RECTANGLE_X] = 0;
						clipping_current.rect[VHAL_RECTANGLE_Y] = 0;
						clipping_current.rect[VHAL_RECTANGLE_W] = 0;
						clipping_current.rect[VHAL_RECTANGLE_H] = 0;
						VHAL_LOGI("  src rect %d(%d,%d,%d,%d)", clipping_current.enable, clipping_current.rect[VHAL_RECTANGLE_X], clipping_current.rect[VHAL_RECTANGLE_Y], clipping_current.rect[VHAL_RECTANGLE_W], clipping_current.rect[VHAL_RECTANGLE_H]);
						ret = UpdateVpathClipping(icluster_clipping_control_, clipping_current);
						if (VHAL_SUCCESS != ret)
						{
							VHAL_LOGE("Front UpdateVpathClipping error. ret=%d", ret);
						}
						ret = UpdateVpathClipping(icluster_clipping_status_, clipping_current);
						if (VHAL_SUCCESS != ret)
						{
							VHAL_LOGE("Front UpdateVpathClipping error. ret=%d", ret);
						}
					}
					else
					{
						VHAL_LOGE("ClearIClusterClippingSize error. ret=%d", ret);
					}

					/* 映像パスに合わせて出力サイズも更新する */
					struct VhalRectanglePropValues output_current{};
					output_current.rect[VHAL_RECTANGLE_X] = 0;
					output_current.rect[VHAL_RECTANGLE_Y] = 0;
					ret = p_layout_mng_->GetScreenSize(screen_id, output_current.rect[VHAL_RECTANGLE_W], output_current.rect[VHAL_RECTANGLE_H]);
					if (VHAL_SUCCESS == ret)
					{
						VHAL_LOGI("  rect (%d,%d,%d,%d)", output_current.rect[0], output_current.rect[1], output_current.rect[2], output_current.rect[3]);

						ret = p_layout_mng_->SetIClusterDestRectangle(output_current.rect[0], output_current.rect[1], output_current.rect[2], output_current.rect[3]);
						if (VHAL_SUCCESS == ret)
						{
							ret = UpdateVpathOutput(icluster_output_status_, output_current);
							if (VHAL_SUCCESS != ret)
							{
								VHAL_LOGE("Instrument Cluster UpdateVpathOutput error. ret=%d", ret);
							}
						}
						else
						{
							VHAL_LOGE("SetIClusterDestRectangle error. ret=%d", ret);
						}
					}
					else
					{
						VHAL_LOGE("GetScreenSize error. ret=%d", ret);
					}

					/* 映像パスに合わせて 映像可視状態のプロパティを更新する */
					ret = VHAL_SUCCESS;
					if (!p_entry->GetStrCurrent().empty())
					{
						ret = p_layout_mng_->SetIClusterVisibility(kDefaultVisibility);
					}

					if (VHAL_SUCCESS == ret)
					{
						ret = UpdateStatusVisible(p_entry->GetStrCurrent(), VIDEO_OUTPUT_TARGET_IC, true);
						if (VHAL_SUCCESS != ret)
						{
							VHAL_LOGE("IC UpdateStatusVisible error. ret=%d", ret);
						}
					}
					else
					{
						VHAL_LOGE("SetIClusterVisibility error. ret=%d", ret);
					}

					/* 映像不透明度状態のプロパティを更新する */
					constexpr uint32_t opacity{kDefaultOpacity};
					ret = VHAL_SUCCESS;
					if (!p_entry->GetStrCurrent().empty())
					{
						ret = p_layout_mng_->SetIClusterOpacity(opacity);
					}
					
					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("SetIClusterOpacity error. ret=%d", ret);
					}
					else
					{
						p_status = GetPropertyEntry(VHAL_PROP_VPATH_ICLUSTER_CTL_OPACITY, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);
						if (nullptr != p_status)
						{
							p_status->SetNumCurrent(static_cast<int64_t>(opacity));
							p_status->SetUpdated(true);
						}
						p_status = GetPropertyEntry(VHAL_PROP_VPATH_ICLUSTER_STS_OPACITY, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);
						if (nullptr != p_status)
						{
							p_status->SetNumCurrent(static_cast<int64_t>(opacity));
							p_status->SetUpdated(true);
						}
					}
				}
			}
		}
	}
	else
	{
		/* 処理なし */
	}

	p_status = GetPropertyEntry(VHAL_PROP_VPATH_ICLUSTER_STS_CURRENT, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);
	if (nullptr != p_status)
	{
		p_status->SetNumCurrent(static_cast<int64_t>(result));
		p_status->SetUpdated(true);
	}

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	カメラ映像設定処理関数
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS		処理成功
           		VHAL_SUCCESS以外	処理失敗
*****************************************************************************/
int32_t CVhalPropertyControl::SettingCameraCapture(void)
{
	int32_t ret{VHAL_SUCCESS};
	int64_t cameraType{VHAL_CONNECTED_CAMERA_INVALID};

	VHAL_LOGV_IN();

	/* カメラ種別を再確認 */
	ret = GetValueNumber(VHAL_PROP_SETTING_STS_CNCT_CAMERA, cameraType);
	VHAL_LOGI("camera Type[%ld]", cameraType);

	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("GetValueNumber error. ret=%d", ret);
		ret = VHAL_ERR;
	}
	else
	{
		ret = camera_control_.SettingtCameraCapture(p_capture_control_.get(), cameraType, true);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SettingtCameraCapture error. ret=%d", ret);
		}
	}

	VHAL_LOGV_OUT();
	return ret;
}

/*****************************************************************************
 処理概要：	前席映像パス設定可否判定Generate登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::GenerateVpathFrontAvailable(struct VhalPropertyEntry * const p_entry)
{
	std::map<std::string, std::string> params{};
	bool available{false};

	VHAL_LOGI("query[%s]", p_entry->GetQuery().c_str());

	const int32_t ret{ParseQueryString(p_entry->GetQuery(), params)};
	if (VHAL_SUCCESS == ret)
	{
		const auto itr_vpath = params.find("vpath");
		if (itr_vpath != params.end())
		{
			available = IsValidVideoPathAvailable(itr_vpath->second, VIDEO_OUTPUT_TARGET_FRONT);
		}
	}

	p_entry->SetBoolCurrent(available);
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	後席映像パス設定可否判定Generate登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::GenerateVpathRearAvailable(struct VhalPropertyEntry * const p_entry)
{
	std::map<std::string, std::string> params{};
	bool available{false};

	VHAL_LOGI("query[%s]", p_entry->GetQuery().c_str());

	const int32_t ret{ParseQueryString(p_entry->GetQuery(), params)};
	if (VHAL_SUCCESS == ret)
	{
		const auto itr_vpath = params.find("vpath");
		if (itr_vpath != params.end())
		{
			available = IsValidVideoPathAvailable(itr_vpath->second, VIDEO_OUTPUT_TARGET_REAR);
		}
	}

	p_entry->SetBoolCurrent(available);
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	カメラ映像パス設定可否判定Generate登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::GenerateVpathCameraAvailable(struct VhalPropertyEntry * const p_entry)
{
	std::map<std::string, std::string> params{};
	bool available{false};

	VHAL_LOGI("query[%s]", p_entry->GetQuery().c_str());

	const int32_t ret{ParseQueryString(p_entry->GetQuery(), params)};
	if (VHAL_SUCCESS == ret)
	{
		const auto itr_vpath = params.find("vpath");
		if (itr_vpath != params.end())
		{
			available = IsValidVideoPathAvailable(itr_vpath->second, VIDEO_OUTPUT_TARGET_CAMERA);
		}
	}

	p_entry->SetBoolCurrent(available);
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	Instrument Cluster 映像パス設定可否判定Generate登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::GenerateVpathIClusterAvailable(struct VhalPropertyEntry * const p_entry)
{
	std::map<std::string, std::string> params{};
	bool available{false};

	VHAL_LOGI("query[%s]", p_entry->GetQuery().c_str());

	const int32_t ret{ParseQueryString(p_entry->GetQuery(), params)};
	if (VHAL_SUCCESS == ret)
	{
		const auto itr_vpath = params.find("vpath");
		if (itr_vpath != params.end())
		{
			available = IsValidVideoPathAvailable(itr_vpath->second, VIDEO_OUTPUT_TARGET_IC);
		}
	}

	p_entry->SetBoolCurrent(available);
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	前席映像出力サイズ設定機能Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionVpathFrontOutput(struct VhalPropertyEntry * const p_entry)
{
	VHAL_LOGV_IN();

	int32_t result{VHAL_SUCCESS};

	do
	{
		std::string path{};
		int32_t ret{p_layout_mng_->GetFrontPath(path)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetFrontPath error. ret=%d", ret);
			result = VHAL_ERR_PARAM;
			break;			
		}
		else if (path.empty())
		{
			VHAL_LOGE("Invalid params. path empty");
			result = VHAL_ERR_PARAM;
			break;
		}
		else
		{
			/* 処理なし */
		}

		struct VhalRectanglePropValues output_current{};
		struct VhalRectanglePropValues output_prev{};
		ret = GetVpathOutputControl(front_output_control_, output_current, output_prev);
		if (VHAL_SUCCESS == ret)
		{
			int32_t screen_id{};
			ret = p_layout_mng_->GetScreenIdFront(screen_id);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("GetScreenIdFront error. ret=%d screen_id=%d", ret, screen_id);
				result = VHAL_ERR_PARAM;
				break;
			}

			if (false == IsValidOutputSize(screen_id, {output_current.rect[0], output_current.rect[1], output_current.rect[2], output_current.rect[3]}))
			{
				VHAL_LOGE("Invalid params. rect (%d,%d,%d,%d)", output_current.rect[0], output_current.rect[1], output_current.rect[2], output_current.rect[3]);
				result = VHAL_ERR_PARAM;
				break;
			}

			/* 出力サイズ設定処理 */
			struct VhalPropertyEntry * const p_control{GetPropertyEntry(VHAL_PROP_VPATH_FRONT_CTL_CURRENT, VHAL_DATA_TYPE_STR, VHAL_ATTR_ANY)};
			if (nullptr != p_control)
			{
				VHAL_LOGI("Front[%s] rect (%d,%d,%d,%d)->(%d,%d,%d,%d)", p_control->GetStrCurrent().c_str(),
						output_prev.rect[0], output_prev.rect[1], output_prev.rect[2], output_prev.rect[3],
						output_current.rect[0], output_current.rect[1], output_current.rect[2], output_current.rect[3]);
			}

			ret = p_layout_mng_->SetFrontDestRectangle(output_current.rect[0], output_current.rect[1], output_current.rect[2], output_current.rect[3]);
			if (VHAL_SUCCESS == ret)
			{
				ret = UpdateVpathOutput(front_output_status_, output_current);
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("Front UpdateVpathOutput error. ret=%d", ret);
				}
			}
			else
			{
				VHAL_LOGE("SetFrontDestRectangle error. ret=%d", ret);
			}
		}
		else
		{
			VHAL_LOGE("GetVpathOutputControl error. ret=%d", ret);
		}
	} while(false);

	p_entry->SetBoolCurrent(false);
	p_entry->SetUpdated(false);

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	後席映像出力サイズ設定機能Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionVpathRearOutput(struct VhalPropertyEntry * const p_entry)
{
	VHAL_LOGV_IN();

	int32_t result{VHAL_SUCCESS};

	do
	{
		std::string path{};
		int32_t ret{p_layout_mng_->GetRearPath(path)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetRearPath error. ret=%d", ret);
			result = VHAL_ERR_PARAM;
			break;
		}
		else if (path.empty())
		{
			VHAL_LOGE("Invalid params. path empty");
			result = VHAL_ERR_PARAM;
			break;
		}
		else
		{
			/* 処理なし */
		}

		struct VhalRectanglePropValues output_current{};
		struct VhalRectanglePropValues output_prev{};
		ret = GetVpathOutputControl(rear_output_control_, output_current, output_prev);
		if (VHAL_SUCCESS == ret)
		{
			int32_t screen_id{};
			ret = p_layout_mng_->GetScreenIdRear(screen_id);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("GetScreenIdRear error. ret=%d screen_id=%d", ret, screen_id);
				result = VHAL_ERR_PARAM;
				break;
			}

			if (false == IsValidOutputSize(screen_id, {output_current.rect[0], output_current.rect[1], output_current.rect[2], output_current.rect[3]}))
			{
				VHAL_LOGE("Invalid params. rect (%d,%d,%d,%d)", output_current.rect[0], output_current.rect[1], output_current.rect[2], output_current.rect[3]);
				result = VHAL_ERR_PARAM;
				break;
			}

			/* 出力サイズ設定処理 */
			struct VhalPropertyEntry * const p_control{GetPropertyEntry(VHAL_PROP_VPATH_REAR_CTL_CURRENT, VHAL_DATA_TYPE_STR, VHAL_ATTR_ANY)};
			if (nullptr != p_control)
			{
				VHAL_LOGI("Rear[%s] rect (%d,%d,%d,%d)->(%d,%d,%d,%d)", p_control->GetStrCurrent().c_str(),
				output_prev.rect[0], output_prev.rect[1], output_prev.rect[2], output_prev.rect[3],
				output_current.rect[0], output_current.rect[1], output_current.rect[2], output_current.rect[3]);
			}

			ret = p_layout_mng_->SetRearDestRectangle(output_current.rect[0], output_current.rect[1], output_current.rect[2], output_current.rect[3]);
			if (VHAL_SUCCESS == ret)
			{
				ret = UpdateVpathOutput(rear_output_status_, output_current);
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("Rear UpdateVpathOutput error. ret=%d", ret);
				}
			}
			else
			{
				VHAL_LOGE("SetRearDestRectangle error. ret=%d", ret);
			}
		}
		else
		{
			VHAL_LOGE("GetVpathOutputControl error. ret=%d", ret);
		}
	} while(false);

	p_entry->SetBoolCurrent(false);
	p_entry->SetUpdated(false);

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	カメラ映像出力サイズ設定機能Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionVpathCameraOutput(struct VhalPropertyEntry * const p_entry)
{
	VHAL_LOGV_IN();

	int32_t result{VHAL_SUCCESS};

	struct VhalRectanglePropValues output_current{};
	struct VhalRectanglePropValues output_prev{};
	int32_t ret{GetVpathOutputControl(camera_output_control_, output_current, output_prev)};
	if (VHAL_SUCCESS == ret)
	{
		int32_t screen_id{};
		ret = p_layout_mng_->GetScreenIdFront(screen_id);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetScreenIdFront error. ret=%d screen_id=%d", ret, screen_id);
			result = VHAL_ERR_PARAM;
		}
		else
		{
			if (false == IsValidOutputSize(screen_id, {output_current.rect[0], output_current.rect[1], output_current.rect[2], output_current.rect[3]}))
			{
				VHAL_LOGE("Invalid params. rect (%d,%d,%d,%d)", output_current.rect[0], output_current.rect[1], output_current.rect[2], output_current.rect[3]);
				result = VHAL_ERR_PARAM;
			}
			else
			{
				/* 出力サイズ設定処理 */
				struct VhalPropertyEntry * const p_control{GetPropertyEntry(VHAL_PROP_VPATH_CAMERA_CTL_CURRENT, VHAL_DATA_TYPE_STR, VHAL_ATTR_ANY)};
				if (nullptr != p_control)
				{
					VHAL_LOGI("Camera[%s] rect (%d,%d,%d,%d)->(%d,%d,%d,%d)", p_control->GetStrCurrent().c_str(),
						output_prev.rect[0], output_prev.rect[1], output_prev.rect[2], output_prev.rect[3],
						output_current.rect[0], output_current.rect[1], output_current.rect[2], output_current.rect[3]);				
				}

				/* ワイド設定がNORMALならINVALIDとする */
				uint32_t wide_mode{VHAL_WIDE_MODE_NORMAL};
				volatile bool reset_wide_mode{false};
				ret = p_layout_mng_->GetCameraWideMode(wide_mode);
				if (VHAL_SUCCESS == ret)
				{
					/* 初回出力サイズ設定が行われるまでワイド設定はNORMAL */
					if (VHAL_WIDE_MODE_NORMAL == wide_mode)
					{
						/* 以降はワイド設定をINVALIDとする */
						ret = p_layout_mng_->SetCameraWideMode(VHAL_WIDE_MODE_INVALID);
						if (VHAL_SUCCESS != ret)
						{
							VHAL_LOGE("SetCameraWideMode error. ret=%d", ret);
						}
						else
						{
							/* 矩形の設定に失敗した場合はワイド設定をNORMALに戻す */
							reset_wide_mode = true;
						}
					}
				}
				else
				{
					VHAL_LOGE("GetCameraWideMode error. ret=%d", ret);
				}

				ret = p_layout_mng_->SetCameraDestRectangle(output_current.rect[0], output_current.rect[1], output_current.rect[2], output_current.rect[3]);
				if (VHAL_SUCCESS == ret)
				{
					ret = UpdateVpathOutput(camera_output_status_, output_current);
					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("Camera UpdateVpathOutput error. ret=%d", ret);
					}
				}
				else
				{
					VHAL_LOGE("SetCameraDestRectangle error. ret=%d", ret);

					/* 初回出力サイズ設定に失敗した場合はワイド設定をNORMALに戻す */
					if (true == reset_wide_mode)
					{
						ret = p_layout_mng_->SetCameraWideMode(VHAL_WIDE_MODE_NORMAL);
						if (VHAL_SUCCESS != ret)
						{
							VHAL_LOGE("SetCameraWideMode error. ret=%d", ret);
						}
					}
				}
			}
		}
	}
	else
	{
		VHAL_LOGE("GetVpathOutputControl error. ret=%d", ret);
	}

	p_entry->SetBoolCurrent(false);
	p_entry->SetUpdated(false);

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	Instrument Cluster 映像出力サイズ設定機能Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionVpathIClusterOutput(struct VhalPropertyEntry * const p_entry)
{
	VHAL_LOGV_IN();

	int32_t result{VHAL_SUCCESS};

	do
	{
		std::string path{};
		int32_t ret{p_layout_mng_->GetIClusterPath(path)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetIClusterPath error. ret=%d", ret);
			result = VHAL_ERR_PARAM;
			break;			
		}
		else if (path.empty())
		{
			VHAL_LOGE("Invalid params. path empty");
			result = VHAL_ERR_PARAM;
			break;
		}
		else
		{
			/* 処理なし */
		}

		struct VhalRectanglePropValues output_current{};
		struct VhalRectanglePropValues output_prev{};
		ret = GetVpathOutputControl(icluster_output_control_,  output_current, output_prev);
		if (VHAL_SUCCESS == ret)
		{
			int32_t screen_id{};
			ret = p_layout_mng_->GetScreenIdICluster(screen_id);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("GetScreenIdICluster error. ret=%d screen_id=%d", ret, screen_id);
				result = VHAL_ERR_PARAM;
			}

			if (false == IsValidOutputSize(screen_id, {output_current.rect[0], output_current.rect[1], output_current.rect[2], output_current.rect[3]}))
			{
				VHAL_LOGE("Invalid params. rect (%d,%d,%d,%d)", output_current.rect[0], output_current.rect[1], output_current.rect[2], output_current.rect[3]);
				result = VHAL_ERR_PARAM;
				break;
			}

			/* 出力サイズ設定処理 */
			struct VhalPropertyEntry * const p_control{GetPropertyEntry(VHAL_PROP_VPATH_ICLUSTER_CTL_CURRENT, VHAL_DATA_TYPE_STR, VHAL_ATTR_ANY)};
			if (nullptr != p_control)
			{
				VHAL_LOGI("Instrument Cluster[%s] rect (%d,%d,%d,%d)->(%d,%d,%d,%d)", p_control->GetStrCurrent().c_str(),
					output_prev.rect[0], output_prev.rect[1], output_prev.rect[2], output_prev.rect[3],
					output_current.rect[0], output_current.rect[1], output_current.rect[2], output_current.rect[3]);
			}

			ret = p_layout_mng_->SetIClusterDestRectangle(output_current.rect[0], output_current.rect[1], output_current.rect[2], output_current.rect[3]);
			if (VHAL_SUCCESS == ret)
			{
				ret = UpdateVpathOutput(icluster_output_status_, output_current);
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("Instrument Cluster UpdateVpathOutput error. ret=%d", ret);
				}
			}
			else
			{
				VHAL_LOGE("SetIClusterDestRectangle error. ret=%d", ret);
			}
		}
		else
		{
			VHAL_LOGE("GetVpathOutputControl error. ret=%d", ret);
		}
	} while (false);

	p_entry->SetBoolCurrent(false);
	p_entry->SetUpdated(false);

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	出力サイズプロパティ更新処理
 引数    ：	std::vector<std::string>	&prop_name		(i)プロパティ名
           	std::vector<int32_t>		&current		(i)変更後の出力サイズ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_ERR_PROPERTY_ENTRY	プロパティエントリー不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::UpdateVpathOutput(
	const struct VhalRectanglePropNames &prop_name, const struct VhalRectanglePropValues &current)
{
	VHAL_LOGV_IN();

	for (uint32_t i{0U}; i < prop_name.GetRect().size(); ++i)
	{
		struct VhalPropertyEntry * const p_status{GetPropertyEntry(prop_name.GetRect(i), VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
		if (nullptr == p_status)
		{
			return VHAL_ERR_PROPERTY_ENTRY;
		}
	
		if (p_status->GetNumCurrent() != current.rect[i])
		{
			p_status->SetNumCurrent(static_cast<int64_t>(current.rect[i]));
			p_status->SetUpdated(true);
		}
	}

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	クリッピングプロパティ取得処理
 引数    ：	struct VhalClippingPropNames	&control	コントロールプロパティ名
           	struct VhalClippingPropValues	&current	変更後のクリッピング設定値
           	struct VhalClippingPropValues	&prev		変更前のクリッピング設定値
 戻り値  ：	処理結果
           		VHAL_ERR_PROPERTY_ENTRY	プロパティエントリー不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::GetVpathOutputControl(
	const struct VhalRectanglePropNames &control,
	struct VhalRectanglePropValues &current, struct VhalRectanglePropValues &prev)
{
	VHAL_LOGV_IN();

	/* 出力サイズ */
	for (uint32_t i{0U}; i < control.GetRect().size(); ++i)
	{
		struct VhalPropertyEntry * const p_control{GetPropertyEntry(control.GetRect(i), VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
		if (nullptr == p_control)
		{
			return VHAL_ERR_PROPERTY_ENTRY;
		}

		current.rect[i] = static_cast<int32_t>(p_control->GetNumCurrent());
		prev.rect[i] = static_cast<int32_t>(p_control->GetNumPrev());
	}

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	後席専用モード向け映像パス切替機能Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionVpathRearRseDisp(const struct VhalPropertyEntry * const p_entry)
{	
	VHAL_LOGV_IN();
	VHAL_LOGI("rse disp [%s] -> [%s]", p_entry->GetStrPrev().c_str(), p_entry->GetStrCurrent().c_str());

	/* 後席専用映像パス設定(MOST) */
	const int32_t ret_path{p_micon_most_video_info_->SetRearRseDispPath(p_entry->GetStrCurrent())};
	if (VHAL_SUCCESS != ret_path)
	{
		VHAL_LOGE("p_micon_most_video_info_->SetRearRseDispPath() error. ret=%d", ret_path);
	}

	/* 結果更新 */
	struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_VPATH_REAR_STS_RSE_DISP, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
	if (nullptr != p_status)
	{
		uint32_t result{VHAL_VPATH_STS_SUCCESS};
		if (VHAL_SUCCESS != ret_path)
		{
			result = VHAL_VPATH_STS_FAILED;
		}
		p_status->SetNumCurrent(static_cast<int64_t>(result));
		p_status->SetUpdated(true);
	}

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	後席専用映像反映通知Action登録関数Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionVpathRearRseNotify(struct VhalPropertyEntry * const p_entry)
{
	VHAL_LOGV_IN();
	VHAL_LOGI("rse notify [%d] -> [%d]", p_entry->GetBoolPrev(), p_entry->GetBoolCurrent());

	/* 接続されているRSE種別取得(MOST) */
	int32_t connected_rse{VHAL_CONNECTED_RSE_INVALID};
	p_micon_comm_control_->GetConnectedRse(connected_rse);
	if ((VHAL_CONNECTED_RSE_FULL == connected_rse) || (VHAL_CONNECTED_RSE_DOP == connected_rse))
	{
		/* フルRSE 又は用品RSE接続時に有効 */
		/* 後席専用映像パス設定(MOST) */
		const int32_t ret{p_micon_most_video_info_->Reflect()};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("p_micon_most_video_info_->Reflect() error. ret=%d", ret);

			/* 結果更新(FAILED) */
			struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_VPATH_REAR_STS_RSE_NOTIFY, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
			if (nullptr != p_status)
			{
				constexpr uint32_t result{VHAL_VPATH_STS_FAILED};
				p_status->SetNumCurrent(static_cast<int64_t>(result));
				p_status->SetUpdated(true);
			}
		}
	}
	else
	{
		/* RSE未接続 */
		VHAL_LOGW("connected_rse=%d", connected_rse);

		/* 結果更新(FAILED) */
		struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_VPATH_REAR_STS_RSE_NOTIFY, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
		if (nullptr != p_status)
		{
			constexpr uint32_t result{VHAL_VPATH_STS_FAILED};
			p_status->SetNumCurrent(static_cast<int64_t>(result));
			p_status->SetUpdated(true);
		}
	}

	/* one-shot event */
	p_entry->SetBoolCurrent(false);
	p_entry->SetUpdated(false);

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	前席映像のクリッピング機能Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionVpathFrontClipping(struct VhalPropertyEntry * const p_entry)
{
	VHAL_LOGV_IN();

	int32_t result{VHAL_SUCCESS};

	do
	{
		std::string path{};
		int32_t ret{p_layout_mng_->GetFrontPath(path)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetFrontPath error. ret=%d", ret);
			result = VHAL_ERR_PARAM;
			break;			
		}
		else if (path.empty())
		{
			VHAL_LOGE("Invalid params. path empty");
			result = VHAL_ERR_PARAM;
			break;
		}
		else
		{
			/* 処理なし */
		}

		/* クリッピングプロパティ取得 */
		struct VhalClippingPropValues clipping_prev{};
		struct VhalClippingPropValues clipping_current{};
		ret = GetVpathClippingControl(front_clipping_control_, clipping_current, clipping_prev);
		if (VHAL_SUCCESS == ret)
		{
			struct VhalPropertyEntry * const p_control{GetPropertyEntry(VHAL_PROP_VPATH_FRONT_CTL_CURRENT, VHAL_DATA_TYPE_STR, VHAL_ATTR_ANY)};
			if (nullptr != p_control)
			{
				VHAL_LOGI("Front[%s] rect %d(%d,%d,%d,%d)->%d(%d,%d,%d,%d)", p_control->GetStrCurrent().c_str(),
					clipping_prev.enable, clipping_prev.rect[VHAL_RECTANGLE_X], clipping_prev.rect[VHAL_RECTANGLE_Y], clipping_prev.rect[VHAL_RECTANGLE_W], clipping_prev.rect[VHAL_RECTANGLE_H],
					clipping_current.enable, clipping_current.rect[VHAL_RECTANGLE_X], clipping_current.rect[VHAL_RECTANGLE_Y], clipping_current.rect[VHAL_RECTANGLE_W], clipping_current.rect[VHAL_RECTANGLE_H]);
			}

			/* クリッピング設定処理 */
			ret = p_layout_mng_->SetFrontSourceRectangle(clipping_current.enable, clipping_current.rect[VHAL_RECTANGLE_X], clipping_current.rect[VHAL_RECTANGLE_Y], clipping_current.rect[VHAL_RECTANGLE_W], clipping_current.rect[VHAL_RECTANGLE_H]);
			if (VHAL_ERR_PARAM == ret)
			{
				VHAL_LOGE("SetFrontSourceRectangle error. ret=%d", ret);
				result = VHAL_ERR_PARAM;
				break;
			}
			else if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("SetFrontSourceRectangle error. ret=%d", ret);
				result = ret;
				break;
			}
			else
			{
				/* 処理なし */
			}

			/* 最新値でクリッピングプロパティを更新（クリッピング無効へ切り替え時、オリジナルサイズへ設定するため） */
			ret = p_layout_mng_->GetFrontSourceRectangle(clipping_current.enable, clipping_current.rect[VHAL_RECTANGLE_X], clipping_current.rect[VHAL_RECTANGLE_Y], clipping_current.rect[VHAL_RECTANGLE_W], clipping_current.rect[VHAL_RECTANGLE_H]);
			if (VHAL_SUCCESS == ret)
			{
				VHAL_LOGI("  src rect %d(%d,%d,%d,%d)", clipping_current.enable, clipping_current.rect[VHAL_RECTANGLE_X], clipping_current.rect[VHAL_RECTANGLE_Y], clipping_current.rect[VHAL_RECTANGLE_W], clipping_current.rect[VHAL_RECTANGLE_H]);

				ret = UpdateVpathClipping(front_clipping_control_, clipping_current);
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("Front UpdateVpathClipping error. ret=%d", ret);
				}
				ret = UpdateVpathClipping(front_clipping_status_, clipping_current);
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("Front UpdateVpathClipping error. ret=%d", ret);
				}
			}
			else
			{
				VHAL_LOGE("GetFrontSourceRectangle error. ret=%d", ret);
			}
		}
		else
		{
			VHAL_LOGE("Front GetVpathClippingControl error. ret=%d", ret);
		}
	} while (false);

	p_entry->SetBoolCurrent(false);
	p_entry->SetUpdated(false);

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	InstrumentCluster映像のクリッピング機能Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionVpathIClusterClipping(struct VhalPropertyEntry * const p_entry)
{
	VHAL_LOGV_IN();

	int32_t result{VHAL_SUCCESS};

	do
	{
		std::string path{};
		int32_t ret{p_layout_mng_->GetIClusterPath(path)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetIClusterPath error. ret=%d", ret);
			result = VHAL_ERR_PARAM;
			break;
		}
		else if (path.empty())
		{
			VHAL_LOGE("Invalid params. path empty");
			result = VHAL_ERR_PARAM;
			break;
		}
		else
		{
			/* 処理なし */
		}

		/* クリッピングプロパティ取得 */
		struct VhalClippingPropValues clipping_prev{};
		struct VhalClippingPropValues clipping_current{};
		ret = GetVpathClippingControl(icluster_clipping_control_, clipping_current, clipping_prev);
		if (VHAL_SUCCESS == ret)
		{
			struct VhalPropertyEntry * const p_control{GetPropertyEntry(VHAL_PROP_VPATH_ICLUSTER_CTL_CURRENT, VHAL_DATA_TYPE_STR, VHAL_ATTR_ANY)};
			if (nullptr != p_control)
			{
				VHAL_LOGI("Instrument Cluster[%s] rect %d(%d,%d,%d,%d)->%d(%d,%d,%d,%d)", p_control->GetStrCurrent().c_str(),
					clipping_prev.enable, clipping_prev.rect[VHAL_RECTANGLE_X], clipping_prev.rect[VHAL_RECTANGLE_Y], clipping_prev.rect[VHAL_RECTANGLE_W], clipping_prev.rect[VHAL_RECTANGLE_H],
					clipping_current.enable, clipping_current.rect[VHAL_RECTANGLE_X], clipping_current.rect[VHAL_RECTANGLE_Y], clipping_current.rect[VHAL_RECTANGLE_W], clipping_current.rect[VHAL_RECTANGLE_H]);
			}

			/* クリッピング設定処理 */
			ret = p_layout_mng_->SetIClusterSourceRectangle(clipping_current.enable, clipping_current.rect[VHAL_RECTANGLE_X], clipping_current.rect[VHAL_RECTANGLE_Y], clipping_current.rect[VHAL_RECTANGLE_W], clipping_current.rect[VHAL_RECTANGLE_H]);
			if (VHAL_ERR_PARAM == ret)
			{
				VHAL_LOGE("SetIClusterSourceRectangle error. ret=%d", ret);
				result = VHAL_ERR_PARAM;
				break;
			}
			else if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("SetIClusterSourceRectangle error. ret=%d", ret);
			}
			else
			{
				/* 処理なし */
			}

			/* 最新値でクリッピングプロパティを更新（クリッピング無効へ切り替え時、オリジナルサイズへ設定するため） */
			ret = p_layout_mng_->GetIClusterSourceRectangle(clipping_current.enable, clipping_current.rect[VHAL_RECTANGLE_X], clipping_current.rect[VHAL_RECTANGLE_Y], clipping_current.rect[VHAL_RECTANGLE_W], clipping_current.rect[VHAL_RECTANGLE_H]);
			if (VHAL_SUCCESS == ret)
			{
				VHAL_LOGI("  src rect %d(%d,%d,%d,%d)", clipping_current.enable, clipping_current.rect[VHAL_RECTANGLE_X], clipping_current.rect[VHAL_RECTANGLE_Y], clipping_current.rect[VHAL_RECTANGLE_W], clipping_current.rect[VHAL_RECTANGLE_H]);

				ret = UpdateVpathClipping(icluster_clipping_control_, clipping_current);
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("Instrument Cluster UpdateVpathClipping error. ret=%d", ret);
				}
				ret = UpdateVpathClipping(icluster_clipping_status_, clipping_current);
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("Instrument Cluster UpdateVpathClipping error. ret=%d", ret);
				}

			}
			else
			{
				VHAL_LOGE("GetIClusterSourceRectangle error. ret=%d", ret);
			}
		}
		else
		{
			VHAL_LOGE("Instrument Cluster GetVpathClippingControl error. ret=%d", ret);
		}
	} while (false);

	p_entry->SetBoolCurrent(false);
	p_entry->SetUpdated(false);

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	クリッピングプロパティ更新処理
引数	：	struct VhalClippingPropNames	&prop_name	(i)コントロールプロパティ名
           	struct VhalClippingPropValues	&current	(i)変更後のクリッピング設定値
 戻り値  ：	処理結果
           		VHAL_ERR_PROPERTY_ENTRY	プロパティエントリー不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::UpdateVpathClipping(
	const struct VhalClippingPropNames &prop_name, 
	const struct VhalClippingPropValues &current)
{
	VHAL_LOGV_IN();


	/* クリッピング有効フラグ */
	struct VhalPropertyEntry * p_entry{GetPropertyEntry(prop_name.GetEnable(), VHAL_DATA_TYPE_BOOL, VHAL_ATTR_ANY)};
	if (nullptr == p_entry)
	{
		return VHAL_ERR_PROPERTY_ENTRY;
	}
	if (p_entry->GetBoolCurrent() != current.enable)
	{
		p_entry->SetBoolCurrent(current.enable);
		p_entry->SetUpdated(true);
	}

	/* クリッピングサイズ */
	for (uint32_t i{0U}; i < prop_name.GetRect().size(); ++i)
	{
		p_entry = GetPropertyEntry(prop_name.GetRect(i), VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);
		if (nullptr == p_entry)
		{
			return VHAL_ERR_PROPERTY_ENTRY;
		}

		if (p_entry->GetNumCurrent() != current.rect[i])
		{
			p_entry->SetNumCurrent(static_cast<int64_t>(current.rect[i]));
			p_entry->SetUpdated(true);
		}
	}

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	クリッピングプロパティ取得処理
 引数    ：	struct VhalClippingPropNames	&control	コントロールプロパティ名
           	struct VhalClippingPropValues	&current	変更後のクリッピング設定値
           	struct VhalClippingPropValues	&prev		変更前のクリッピング設定値
 戻り値  ：	処理結果
           		VHAL_ERR_PROPERTY_ENTRY	プロパティエントリー不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::GetVpathClippingControl(
	const struct VhalClippingPropNames &control,
	struct VhalClippingPropValues &current, struct VhalClippingPropValues &prev)
{
	VHAL_LOGV_IN();

	/* クリッピング有効フラグ */
	const struct VhalPropertyEntry * p_control{GetPropertyEntry(control.GetEnable(), VHAL_DATA_TYPE_BOOL, VHAL_ATTR_ANY)};
	if (nullptr == p_control)
	{
		return VHAL_ERR_PROPERTY_ENTRY;
	}

	current.enable = p_control->GetBoolCurrent();
	prev.enable = p_control->GetBoolPrev();

	/* クリッピングサイズ */
	for (uint32_t i{0U}; i < control.GetRect().size(); ++i)
	{
		p_control = GetPropertyEntry(control.GetRect(i), VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);
		if (nullptr == p_control)
		{
			return VHAL_ERR_PROPERTY_ENTRY;
		}

		current.rect[i] = static_cast<int32_t>(p_control->GetNumCurrent());
		prev.rect[i] = static_cast<int32_t>(p_control->GetNumPrev());
	}

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	短形設定プロパティ取得処理（共通）
 引数    ：	struct VhalRectanglePropNames	&control_prop		コントロールプロパティ名
         	struct VhalRectanglePropNames	&status_prop		ステータスプロパティ名
           	struct VhalRectanglePropValues	&current_values		変更後の短形設定値
           	struct VhalRectanglePropValues	&prev_values		変更前の短形設定値
 戻り値  ：	処理結果
           		VHAL_ERR_PROPERTY_ENTRY	プロパティエントリー不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::GetRectangleControlCommon(
	const struct VhalRectanglePropNames &control_prop, const struct VhalRectanglePropNames &status_prop, struct VhalRectanglePropValues &current_values, struct VhalRectanglePropValues &prev_values)
{
	VHAL_LOGV_IN();
	int32_t ret{VHAL_SUCCESS};

	/* サイズ */
	bool done{false};
	for (uint32_t i{0U}; (!done) && (i < control_prop.GetRect().size()); (void)++i)
	{
		struct VhalPropertyEntry * const p_control{GetPropertyEntry(control_prop.GetRect(i), VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
		if (nullptr == p_control)
		{
			ret = VHAL_ERR_PROPERTY_ENTRY;
			done = true;
		}
		else
		{
			struct VhalPropertyEntry * const p_status{GetPropertyEntry(status_prop.GetRect(i), VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
			if (nullptr == p_status)
			{
				ret = VHAL_ERR_PROPERTY_ENTRY;
				done = true;
			}
			else
			{
				current_values.rect[i]	= static_cast<int32_t>(p_control->GetNumCurrent());
				prev_values.rect[i]		= static_cast<int32_t>(p_status->GetNumCurrent());
			}
		}

	}

	VHAL_LOGV_OUT("ret=%d", ret);
	return ret;
}

/*****************************************************************************
 処理概要：	短形状態プロパティ更新処理（共通）
 引数    ：	struct VhalRectanglePropNames	&status_prop		ステータスプロパティ名
           	struct VhalRectanglePropValues	&current_values		短形設定値
 戻り値  ：	処理結果
           		VHAL_ERR_PROPERTY_ENTRY	プロパティエントリー不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::UpdateRectangleStatusCommon(
	const struct VhalRectanglePropNames &status_prop, struct VhalRectanglePropValues &current_values)
{
	VHAL_LOGV_IN();
	int32_t ret{VHAL_SUCCESS};

	/* サイズ */
	for (uint32_t i{0U}; i < status_prop.GetRect().size(); (void)++i)
	{
		struct VhalPropertyEntry * const p_status{GetPropertyEntry(status_prop.GetRect(i), VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
		if (nullptr == p_status)
		{
			ret = VHAL_ERR_PROPERTY_ENTRY;
			break;
		}
	
		if (static_cast<int32_t>(p_status->GetNumCurrent()) != current_values.rect[i])
		{
			p_status->SetNumCurrent(static_cast<int64_t>(current_values.rect[i]));
			p_status->SetUpdated(true);
		}
	}

	VHAL_LOGV_OUT("ret=%d", ret);
	return ret;
}

/*****************************************************************************
 処理概要：	前席映像のワイド設定機能Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionVpathFrontWide(const struct VhalPropertyEntry * const p_entry)
{
	int32_t result{VHAL_SUCCESS};
	VHAL_LOGV_IN();
	VHAL_LOGI("front wide [%ld] -> [%ld]", p_entry->GetNumPrev(), p_entry->GetNumCurrent());

	std::string path{};
	int32_t ret{p_layout_mng_->GetFrontPath(path)};
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("GetFrontPath error. ret=%d", ret);
		result = VHAL_ERR_PARAM;
	}
	else if (path.empty())
	{
		VHAL_LOGE("Invalid params. path empty");
		result = VHAL_ERR_PARAM;
	}
	else
	{
		/* ワイド設定処理 */
		ret = p_layout_mng_->SetFrontWideMode(I64ToUI32(p_entry->GetNumCurrent()));
		if (VHAL_ERR_PARAM == ret)
		{
			VHAL_LOGE("SetFrontWide error. ret=%d", ret);
			result = VHAL_ERR_PARAM;
		}
		else if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetFrontWide error. ret=%d", ret);
		}
		else
		{
			/* 処理なし */
		}

		if (VHAL_SUCCESS == result)
		{
			struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_VPATH_FRONT_STS_WIDE, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
			if (nullptr != p_status)
			{
				p_status->SetNumCurrent(p_entry->GetNumCurrent());
				p_status->SetUpdated(true);
			}
		}
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	後席映像のワイド設定機能Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionVpathRearWide(const struct VhalPropertyEntry * const p_entry)
{
	int32_t result{VHAL_SUCCESS};
	VHAL_LOGV_IN();
	VHAL_LOGI("rear wide [%ld] -> [%ld]", p_entry->GetNumPrev(), p_entry->GetNumCurrent());

	std::string path{};
	int32_t ret{p_layout_mng_->GetRearPath(path)};
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("GetRearPath error. ret=%d", ret);
		result = VHAL_ERR_PARAM;
	}
	else if (path.empty())
	{
		VHAL_LOGE("Invalid params. path empty");
		result = VHAL_ERR_PARAM;
	}
	else
	{
		/* ワイド設定処理 */
		ret = p_layout_mng_->SetRearWideMode(I64ToUI32(p_entry->GetNumCurrent()));
		if (VHAL_ERR_PARAM == ret)
		{
			VHAL_LOGE("SetRearWideMode error. ret=%d", ret);
			result = VHAL_ERR_PARAM;
		}
		else if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetRearWideMode error. ret=%d", ret);
		}
		else
		{
			/* 処理なし */
		}

		if (result == VHAL_SUCCESS) {
			struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_VPATH_REAR_STS_WIDE, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
			if (nullptr != p_status)
			{
				p_status->SetNumCurrent(p_entry->GetNumCurrent());
				p_status->SetUpdated(true);
			}
		}
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	前席映像の可視状態設定機能Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionVpathFrontVisible(const struct VhalPropertyEntry * const p_entry)
{
	int32_t result{VHAL_SUCCESS};
	VHAL_LOGV_IN();
	VHAL_LOGI("front visible [%d] -> [%d]", p_entry->GetBoolPrev(), p_entry->GetBoolCurrent());

	std::string path{};
	int32_t ret{p_layout_mng_->GetFrontPath(path)};
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("GetFrontPath error. ret=%d", ret);
		result = VHAL_ERR_PARAM;
	}
	else if (path.empty())
	{
		VHAL_LOGE("Invalid params. path empty");
		result = VHAL_ERR_PARAM;
	}
	else
	{
		/* 可視状態設定処理 */
		ret = p_layout_mng_->SetFrontVisibility(p_entry->GetBoolCurrent());
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetFrontVisibility error. ret=%d", ret);
			result = ret;
		}

		struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_VPATH_FRONT_STS_VISIBLE, VHAL_DATA_TYPE_BOOL, VHAL_ATTR_ANY)};
		if (nullptr != p_status)
		{
			p_status->SetBoolCurrent(p_entry->GetBoolCurrent());
			p_status->SetUpdated(true);
		}
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	前席映像の可視状態取得機能Generate登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	なし
*****************************************************************************/
int32_t CVhalPropertyControl::GenerateFrontVisible(struct VhalPropertyEntry * const p_entry)
{
	int32_t ret{VHAL_SUCCESS};
	bool visibility{};
	
	ret = p_layout_mng_->GetFrontVisibility(visibility);
	if (VHAL_SUCCESS == ret)
	{
		p_entry->SetBoolCurrent(visibility);
	}
	
	return ret;
}

/*****************************************************************************
 処理概要：	後席映像の可視状態設定機能Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionVpathRearVisible(const struct VhalPropertyEntry * const p_entry)
{
	int32_t result{VHAL_SUCCESS};
	VHAL_LOGV_IN();
	VHAL_LOGI("rear visible [%d] -> [%d]", p_entry->GetBoolPrev(), p_entry->GetBoolCurrent());

	std::string path{};
	int32_t ret{p_layout_mng_->GetRearPath(path)};
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("GetRearPath error. ret=%d", ret);
		result = VHAL_ERR_PARAM;
	}
	else if (path.empty())
	{
		VHAL_LOGE("Invalid params. path empty");
		result = VHAL_ERR_PARAM;
	}
	else
	{
		/* 可視状態設定処理 */
		ret = p_layout_mng_->SetRearVisibility(p_entry->GetBoolCurrent());
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetRearVisibility error. ret=%d", ret);
			result = VHAL_ERR_PARAM;
		}

		struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_VPATH_REAR_STS_VISIBLE, VHAL_DATA_TYPE_BOOL, VHAL_ATTR_ANY)};
		if (nullptr != p_status)
		{
			p_status->SetBoolCurrent(p_entry->GetBoolCurrent());
			p_status->SetUpdated(true);
		}
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	後席映像の可視状態取得機能Generate登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	なし
*****************************************************************************/
int32_t CVhalPropertyControl::GenerateRearVisible(struct VhalPropertyEntry * const p_entry)
{
	int32_t ret{VHAL_SUCCESS};
	bool visibility{};
	
	ret = p_layout_mng_->GetRearVisibility(visibility);
	if (VHAL_SUCCESS == ret)
	{
		p_entry->SetBoolCurrent(visibility);
	}
	
	return ret;
}

/*****************************************************************************
 処理概要：	Instrument Cluster 映像の可視状態設定機能Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionVpathIClusterVisible(const struct VhalPropertyEntry * const p_entry)
{
	int32_t result{VHAL_SUCCESS};
	VHAL_LOGV_IN();
	VHAL_LOGI("instrument cluster visible [%d] -> [%d]", p_entry->GetBoolPrev(), p_entry->GetBoolCurrent());

	std::string path{};
	int32_t ret{p_layout_mng_->GetIClusterPath(path)};
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("GetIClusterPath error. ret=%d", ret);
		result = VHAL_ERR_PARAM;
	}
	else if (path.empty())
	{
		VHAL_LOGE("Invalid params. path empty");
		result = VHAL_ERR_PARAM;
	}
	else
	{
		/* 可視状態設定処理 */
		ret = p_layout_mng_->SetIClusterVisibility(p_entry->GetBoolCurrent());
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetIClusterVisibility error. ret=%d", ret);
			result = VHAL_ERR_PARAM;
		}

		struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_VPATH_ICLUSTER_STS_VISIBLE, VHAL_DATA_TYPE_BOOL, VHAL_ATTR_ANY)};
		if (nullptr != p_status)
		{
			p_status->SetBoolCurrent(p_entry->GetBoolCurrent());
			p_status->SetUpdated(true);
		}
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	Instrument Cluster 映像の可視状態取得機能Generate登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	なし
*****************************************************************************/
int32_t CVhalPropertyControl::GenerateIClusterVisible(struct VhalPropertyEntry * const p_entry)
{
	int32_t ret{VHAL_SUCCESS};
	bool visibility{};
	
	ret = p_layout_mng_->GetIClusterVisibility(visibility);
	if (VHAL_SUCCESS == ret)
	{
		p_entry->SetBoolCurrent(visibility);
	}
	
	return ret;
}

/*****************************************************************************
 処理概要：	前席映像の不透明度設定機能Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionVpathFrontOpacity(const struct VhalPropertyEntry * const p_entry)
{
	int32_t result{VHAL_SUCCESS};
	VHAL_LOGV_IN();
	VHAL_LOGI("front opacity [%ld] -> [%ld]", p_entry->GetNumPrev(), p_entry->GetNumCurrent());

	std::string path{};
	int32_t ret{p_layout_mng_->GetFrontPath(path)};
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("GetFrontPath error. ret=%d", ret);
		result = VHAL_ERR_PARAM;
	}
	else if (path.empty())
	{
		VHAL_LOGE("Invalid params. path empty");
		result = VHAL_ERR_PARAM;
	}
	else
	{
		/* 不透明度設定処理 */
		ret = p_layout_mng_->SetFrontOpacity(I64ToUI32(p_entry->GetNumCurrent()));
		if (VHAL_ERR_PARAM == ret)
		{
			VHAL_LOGE("SetFrontOpacity error. ret=%d", ret);
			result = VHAL_ERR_PARAM;
		}
		else if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetFrontOpacity error. ret=%d", ret);
			result = ret;
		}
		else
		{
			/* 処理なし */
		}

		if (VHAL_SUCCESS == result)
		{
			struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_VPATH_FRONT_STS_OPACITY, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
			if (nullptr != p_status)
			{
				p_status->SetNumCurrent(p_entry->GetNumCurrent());
				p_status->SetUpdated(true);
			}
		}
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	後席映像の不透明度設定機能Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionVpathRearOpacity(const struct VhalPropertyEntry * const p_entry)
{
	int32_t result{VHAL_SUCCESS};
	VHAL_LOGV_IN();
	VHAL_LOGI("rear opacity [%ld] -> [%ld]", p_entry->GetNumPrev(), p_entry->GetNumCurrent());

	std::string path{};
	int32_t ret{p_layout_mng_->GetRearPath(path)};
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("GetRearPath error. ret=%d", ret);
		result = VHAL_ERR_PARAM;
	}
	else if (path.empty())
	{
		VHAL_LOGE("Invalid params. path empty");
		result = VHAL_ERR_PARAM;
	}
	else
	{
		/* 不透明度設定処理 */
		ret = p_layout_mng_->SetRearOpacity(I64ToUI32(p_entry->GetNumCurrent()));
		if (VHAL_ERR_PARAM == ret)
		{
			VHAL_LOGE("SetRearOpacity error. ret=%d", ret);
			result = VHAL_ERR_PARAM;
		}
		else if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetRearOpacity error. ret=%d", ret);
		}
		else
		{
			/* 処理なし */
		}

		if (VHAL_SUCCESS == result)
		{
			struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_VPATH_REAR_STS_OPACITY, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
			if (nullptr != p_status)
			{
				p_status->SetNumCurrent(p_entry->GetNumCurrent());
				p_status->SetUpdated(true);
			}
		}
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	Instrument Cluster 映像の不透明度設定機能Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionVpathIClusterOpacity(const struct VhalPropertyEntry * const p_entry)
{
	int32_t result{VHAL_SUCCESS};
	VHAL_LOGV_IN();
	VHAL_LOGI("instrument cluster opacity [%ld] -> [%ld]", p_entry->GetNumPrev(), p_entry->GetNumCurrent());

	std::string path{};
	int32_t ret{p_layout_mng_->GetIClusterPath(path)};
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("GetIClusterPath error. ret=%d", ret);
		result = VHAL_ERR_PARAM;
	}
	else if (path.empty())
	{
		VHAL_LOGE("Invalid params. path empty");
		result = VHAL_ERR_PARAM;
	}
	else
	{
		/* 不透明度設定処理 */
		ret = p_layout_mng_->SetIClusterOpacity(I64ToUI32(p_entry->GetNumCurrent()));
		if (VHAL_ERR_PARAM == ret)
		{
			VHAL_LOGE("SetIClusterOpacity error. ret=%d", ret);
			result = VHAL_ERR_PARAM;
		}
		else if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetIClusterOpacity error. ret=%d", ret);
			result = ret;
		}
		else
		{
			/* 処理なし */
		}

		if (VHAL_SUCCESS == result)
		{
			struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_VPATH_ICLUSTER_STS_OPACITY, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
			if (nullptr != p_status)
			{
				p_status->SetNumCurrent(p_entry->GetNumCurrent());
				p_status->SetUpdated(true);
			}
		}
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	映像ソース設定機能Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionVsrcFrontId(const struct VhalPropertyEntry * const p_entry)
{
	int32_t result{VHAL_SUCCESS};
	VHAL_LOGV_IN();
	VHAL_LOGI("Vsrc Front ID [%ld] -> [%ld]", p_entry->GetNumPrev(), p_entry->GetNumCurrent());

	do
	{
		/* 映像ソース設定処理 */
		int32_t ret{p_color_mng_->SetVideoSource(I64ToUI32(p_entry->GetNumCurrent()))};
		if (VHAL_ERR_PARAM == ret)
		{
			VHAL_LOGE("SetVideoSource error. ret=%d", ret);
			result = VHAL_ERR_PARAM;
			break;
		}
		else if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetVideoSource error. ret=%d", ret);
		}
		else
		{
			/* 処理なし */
		}
		
		/* 映像ソースを切り替えたら、現在のStep値(status)も更新が必要 */
		/* 画質調整STEP 状態プロパティ設定 */
		ret = UpdateStatusColorStep(COLOR_STEP_TYPE_ALL);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("UpdateStatusColorStep error. ret=%d", ret);
		}
	
		struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_VSRC_FRONT_STS_ID, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
		if (nullptr != p_status)
		{
			/* 設定値変化チェック */
			if (p_status->GetNumCurrent() != p_entry->GetNumCurrent())
			{
				p_status->SetNumCurrent(p_entry->GetNumCurrent());
				p_status->SetUpdated(true);
			}
		}
	} while (false);

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	画質調整機能（明るさStep設定要求）Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionVsrcFrontBright(const struct VhalPropertyEntry * const p_entry)
{
	int32_t result{VHAL_SUCCESS};
	VHAL_LOGV_IN();
	VHAL_LOGI("Vsrc Front Brightness [%ld]", p_entry->GetNumCurrent());

	do
	{
		/* 明るさStep設定処理 */
		int32_t ret{p_color_mng_->SetCurrentColorStepBrightness(I64ToI32(p_entry->GetNumCurrent()))};
		if (VHAL_ERR_PARAM == ret)
		{
			VHAL_LOGE("SetCurrentColorStepBrightness error. ret=%d", ret);
			result = VHAL_ERR_PARAM;
			break;
		}
		else if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetCurrentColorStepBrightness error. ret=%d", ret);
		}
		else
		{
			/* 処理なし */
		}
		/* 画質調整設定適用 */
		ret = p_color_mng_->ApplyAdjustment(true);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("ApplyAdjustment error. ret=%d", ret);
		}

		/* 設定した値で現在のStep値(status)を更新 */
		/* 画質調整STEP 状態プロパティ設定 */
		ret = UpdateStatusColorStep(COLOR_STEP_TYPE_BRIGHTNESS);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("UpdateStatusColorStep error. ret=%d", ret);
		}
	} while (false);

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	画質調整機能（コントラストStep設定要求）Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionVsrcFrontContrast(const struct VhalPropertyEntry * const p_entry)
{
	int32_t result{VHAL_SUCCESS};
	VHAL_LOGV_IN();
	VHAL_LOGI("Vsrc Front Contrast [%ld]", p_entry->GetNumCurrent());

	do
	{
		/* コントラストStep設定処理 */
		int32_t ret{p_color_mng_->SetCurrentColorStepContrast(I64ToI32(p_entry->GetNumCurrent()))};
		if (VHAL_ERR_PARAM == ret)
		{
			VHAL_LOGE("SetCurrentColorStepContrast error. ret=%d", ret);
			result = VHAL_ERR_PARAM;
			break;
		}
		else if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetCurrentColorStepContrast error. ret=%d", ret);
		}
		else
		{
			/* 処理なし */
		}
		/* 画質調整設定適用 */
		ret = p_color_mng_->ApplyAdjustment(true);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("ApplyAdjustment error. ret=%d", ret);
		}

		/* 設定した値で現在のStep値(status)を更新 */
		/* 画質調整STEP 状態プロパティ設定 */
		ret = UpdateStatusColorStep(COLOR_STEP_TYPE_CONTRAST);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("UpdateStatusColorStep error. ret=%d", ret);
		}
	} while (false);

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	画質調整機能（別体ディスプレイへの画質調整値更新要求）Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionVsrcImgAdjUpdate(struct VhalPropertyEntry * const p_entry)
{
	VHAL_LOGV_IN();

	/* 別体ディスプレイへの画質調整値更新要求 */



	p_entry->SetBoolCurrent(false);
	p_entry->SetUpdated(false);

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	画質調整機能（強制HMI画質適用設定）Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionVsrcForceHmiImgAdj(const struct VhalPropertyEntry * const p_entry)
{
	struct VhalPropertyEntry *p_status{nullptr};

	VHAL_LOGV_IN();
	VHAL_LOGI("front [%d]", p_entry->GetBoolCurrent());

	/* 画質調整 強制HMI画質適用設定処理 */
	p_color_mng_->SetForceHmiEnable(p_entry->GetBoolCurrent());
	/* 画質モード通知が送信開始されるまでは送信をガードする */
	const bool first_send{p_color_mng_->IsFirstSend()};
	if (false == first_send)
	{
		const int32_t ret{p_color_mng_->ApplyAdjustment(false)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("ApplyAdjustment error. ret=%d", ret);
		}
	}

	p_status = GetPropertyEntry(VHAL_PROP_VSRC_FRONT_STS_FORCE_HMI_IMG_ADJ, VHAL_DATA_TYPE_BOOL, VHAL_ATTR_READ);
	if (nullptr != p_status)
	{
		p_status->SetBoolCurrent(p_entry->GetBoolCurrent());
		p_status->SetUpdated(true);	/* callback通知有り */
	}
	else
	{
		VHAL_LOGE("GetPropertyEntry error p_status=%p", p_status);
	}

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	画質調整機能（強制多感覚連携画質適用設定）Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionVsrcForceMultisensoryImgAdj(const struct VhalPropertyEntry * const p_entry)
{
	struct VhalPropertyEntry *p_status{nullptr};

	VHAL_LOGV_IN();
	VHAL_LOGI("front [%d]", p_entry->GetBoolCurrent());

	/* 画質調整 強制多感覚連携画質適用設定処理 */
	p_color_mng_->SetForceMultisensoryEnable(p_entry->GetBoolCurrent());
	/* 画質モード通知が送信開始されるまでは送信をガードする */
	const bool first_send{p_color_mng_->IsFirstSend()};
	if (false == first_send)
	{
		bool color_update{false};
		/* 多感覚連携 true */
		if (true == p_entry->GetBoolCurrent())
		{
			std::string front_path{};
			const int32_t ret{p_layout_mng_->GetFrontPath(front_path)};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("GetFrontPath error. ret=%d", ret);
			}
			else
			{
				/* 前席映像パス = 多感覚連携 */
				if (VHAL_PATH_MULTISENSORY == front_path)
				{
					color_update = true;
				}
			}
		}
		/* 多感覚連携 false */
		else
		{
			color_update = true;
		}

		/* 画質モード通知を送信する */
		if (true == color_update)
		{
			const int32_t ret{p_color_mng_->ApplyAdjustment(false)};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("ApplyAdjustment error. ret=%d", ret);
			}
		}
	}

	p_status = GetPropertyEntry(VHAL_PROP_VSRC_FRONT_STS_FORCE_MULTISENSORY_IMG_ADJ, VHAL_DATA_TYPE_BOOL, VHAL_ATTR_READ);
	if (nullptr != p_status)
	{
		p_status->SetBoolCurrent(p_entry->GetBoolCurrent());
		p_status->SetUpdated(true);	/* callback通知有り */
	}
	else
	{
		VHAL_LOGE("GetPropertyEntry error p_status=%p", p_status);
	}

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	MUTE機能（前席ディスプレイ全体のMUTE要求）Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionMuteFrontDisp(const struct VhalPropertyEntry * const p_entry)
{
	VHAL_LOGV_IN();
	VHAL_LOGI("Mute Front Disp [%d] -> [%d]", p_entry->GetBoolPrev(), p_entry->GetBoolCurrent());

	int32_t ret{VHAL_SUCCESS};
	/* Mute設定処理 */
	if (nullptr == p_color_mng_)
	{
		VHAL_LOGW("CVhalColorMng is not created yet.");
		ret = VHAL_ERR;
	}
	else
	{
		ret = p_color_mng_->SetMuteFrontDisp(p_entry->GetBoolCurrent());
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetMuteFrontDisp error. ret=%d", ret);
		}
		else
		{
			/* 前席ディスプレイ全体のMUTE処理 */
			if (nullptr == p_mute_)
			{
				VHAL_LOGW("CVhalMute is not created yet.");
				ret = VHAL_ERR;
			}
			else
			{
				/* バックライト設定値取得 */
				struct VhalPropertyEntry * const p_backlight_status{GetPropertyEntry(VHAL_PROP_MUTE_FRONT_STS_BACK, VHAL_DATA_TYPE_BOOL, VHAL_ATTR_ANY)};
				if (nullptr == p_backlight_status)
				{
					VHAL_LOGW("p_backlight_status is not found.");
					ret = VHAL_ERR;
				}
				else
				{
					const bool backlight_status{p_backlight_status->GetBoolCurrent()};
					if (true == p_entry->GetBoolCurrent())
					{
						if (true == backlight_status)
						{
							/* Muteサーフェスを表示する場合、画質モード通知を先に実施する必要がある
							(BEVS3CDC_システムシーケンス_映像 "電源ステートによる映像MUTE"に合わせる) */
							/* 画質調整設定適用 */
							ret = p_color_mng_->ApplyAdjustment(false);
							if (VHAL_SUCCESS != ret)
							{
								VHAL_LOGE("ApplyAdjustment error. ret=%d", ret);
							}
							else
							{
								/* 前席ディスプレイMuteサーフェス表示 */
								ret = p_mute_->SetMuteFrontDisp(p_entry->GetBoolCurrent());
								if (VHAL_SUCCESS != ret)
								{
									VHAL_LOGE("SetMuteFrontDisp error. ret=%d", ret);
								}
							}
						}
					}
					else
					{
						if (true == backlight_status)
						{
							/* Muteサーフェスを非表示にする場合、画質モード通知より先に実施する必要がある
							(BEVS3CDC_システムシーケンス_映像 "電源ステートによる映像MUTE"に合わせる) */
							/* 前席ディスプレイMuteサーフェス非表示 */
							ret = p_mute_->SetMuteFrontDisp(p_entry->GetBoolCurrent());
							if (VHAL_SUCCESS != ret)
							{
								VHAL_LOGE("SetMuteFrontDisp error. ret=%d", ret);
							}
							else
							{
								/* 画質調整設定適用 */
								ret = p_color_mng_->ApplyAdjustment(false);
								if (VHAL_SUCCESS != ret)
								{
									VHAL_LOGE("ApplyAdjustment error. ret=%d", ret);
								}
							}
						}
					}
				}
			}
		}
	}
	struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_MUTE_FRONT_STS_DISP, VHAL_DATA_TYPE_BOOL, VHAL_ATTR_ANY)};
	if (nullptr != p_status)
	{
		p_status->SetBoolCurrent(p_entry->GetBoolCurrent());
		p_status->SetUpdated(true);
	}

	VHAL_LOGV_OUT();
	return ret;
}

/*****************************************************************************
 処理概要：	MUTE機能（前席映像面のMUTE要求）Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionMuteFrontVideo(const struct VhalPropertyEntry * const p_entry)
{
	VHAL_LOGV_IN();
	VHAL_LOGI("Mute Front Video [%d] -> [%d]", p_entry->GetBoolPrev(), p_entry->GetBoolCurrent());

	/* 前席映像面のMUTE処理 */
	if (nullptr == p_mute_)
	{
		VHAL_LOGW("CVhalMute is not created yet.");
	}
	else
	{
		p_mute_->SetMuteFrontVideo(p_entry->GetBoolCurrent());
	}

	struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_MUTE_FRONT_STS_VIDEO, VHAL_DATA_TYPE_BOOL, VHAL_ATTR_ANY)};
	if (nullptr != p_status)
	{
		p_status->SetBoolCurrent(p_entry->GetBoolCurrent());
		p_status->SetUpdated(true);
	}

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	MUTE機能（前席バックライト設定変更要求）Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionMuteFrontBacklight(const struct VhalPropertyEntry * const p_entry)
{
	VHAL_LOGV_IN();
	VHAL_LOGI("Mute Front Backlight [%d] -> [%d]", p_entry->GetBoolPrev(), p_entry->GetBoolCurrent());

	int32_t ret{VHAL_SUCCESS};
	/* バックライト設定処理 */
	if (nullptr == p_color_mng_)
	{
		VHAL_LOGW("CVhalColorMng is not created yet.");
		ret = VHAL_ERR;
	}
	else
	{
		ret = p_color_mng_->SetBackLight(p_entry->GetBoolCurrent());
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetBackLight error. ret=%d", ret);
		}
		else
		{
			/* 前席ディスプレイ全体のMUTE処理 */
			if (nullptr == p_mute_)
			{
				VHAL_LOGW("CVhalMute is not created yet.");
				ret = VHAL_ERR;
			}
			else
			{
				/* Mute設定値取得 */
				struct VhalPropertyEntry * const p_mute_status{GetPropertyEntry(VHAL_PROP_MUTE_FRONT_STS_DISP, VHAL_DATA_TYPE_BOOL, VHAL_ATTR_ANY)};
				if (nullptr == p_mute_status)
				{
					VHAL_LOGW("p_mute_status is not found.");
					ret = VHAL_ERR;
				}
				else
				{
					const bool mute_status{p_mute_status->GetBoolCurrent()};
					if (true == p_entry->GetBoolCurrent())
					{
						if (false == mute_status)
						{
							/* Muteサーフェスを非表示にする場合、画質モード通知より先に実施する必要がある
							(BEVS3CDC_システムシーケンス_映像 "電源ステートによる映像MUTE"に合わせる) */
							/* 前席ディスプレイMuteサーフェス非表示 (引数を反転させる) */
							ret = p_mute_->SetMuteFrontDisp(!p_entry->GetBoolCurrent());
							if (VHAL_SUCCESS != ret)
							{
								VHAL_LOGE("SetMuteFrontDisp error. ret=%d", ret);
							}
							else
							{
								/* 画質調整設定適用 */
								ret = p_color_mng_->ApplyAdjustment(false);
								if (VHAL_SUCCESS != ret)
								{
									VHAL_LOGE("ApplyAdjustment error. ret=%d", ret);
								}
							}
						}
					}
					else
					{
						if (false == mute_status)
						{
							/* Muteサーフェスを表示する場合、画質モード通知を先に実施する必要がある
							(BEVS3CDC_システムシーケンス_映像 "電源ステートによる映像MUTE"に合わせる) */
							/* 画質調整設定適用 */
							ret = p_color_mng_->ApplyAdjustment(false);
							if (VHAL_SUCCESS != ret)
							{
								VHAL_LOGE("ApplyAdjustment error. ret=%d", ret);
							}
							else
							{
								/* 前席ディスプレイMuteサーフェス表示 (引数を反転させる) */
								ret = p_mute_->SetMuteFrontDisp(!p_entry->GetBoolCurrent());
								if (VHAL_SUCCESS != ret)
								{
									VHAL_LOGE("SetMuteFrontDisp error. ret=%d", ret);
								}
							}
						}
					}
				}
			}
		}
	}
	struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_MUTE_FRONT_STS_BACK, VHAL_DATA_TYPE_BOOL, VHAL_ATTR_ANY)};
	if (nullptr != p_status)
	{
		p_status->SetBoolCurrent(p_entry->GetBoolCurrent());
		p_status->SetUpdated(true);
	}

	VHAL_LOGV_OUT();
	return ret;
}

/*****************************************************************************
 処理概要：	MUTE機能（後席ディスプレイ全体のMUTE要求）Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionMuteRearDisp(const struct VhalPropertyEntry * const p_entry)
{
	int32_t result{VHAL_SUCCESS};
	VHAL_LOGV_IN();
	VHAL_LOGI("Mute Rear Disp [%d] -> [%d]", p_entry->GetBoolPrev(), p_entry->GetBoolCurrent());

	/* 後席スクリーン有効チェック */
	int32_t screen_id{};
	const int32_t ret{p_layout_mng_->GetScreenIdRear(screen_id)};
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("GetScreenIdRear error. ret=%d", ret);
		result = VHAL_ERR;
	}
	else
	{
		const bool rear_available{p_layout_mng_->IsScreenAvailable(screen_id)};
		if (false == rear_available)
		{
			VHAL_LOGE("rear screen is not available.");
			result = VHAL_ERR;
		}
		else
		{
			/* 後席ディスプレイ全体のMUTE処理 */
			if (nullptr == p_mute_)
			{
				VHAL_LOGW("CVhalMute is not created yet.");
			}
			else
			{
				p_mute_->SetMuteRearDisp(p_entry->GetBoolCurrent());
			}

			/* 接続されているRSE種別取得(MOST) */
			int32_t connected_rse{VHAL_CONNECTED_RSE_INVALID};
			p_micon_comm_control_->GetConnectedRse(connected_rse);
			if (VHAL_CONNECTED_RSE_DOP != connected_rse)
			{
				/* 用品RSE時は後席ディスプレイ全体MUTE設定(MOST)終了後に実施 */
				struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_MUTE_REAR_STS_DISP, VHAL_DATA_TYPE_BOOL, VHAL_ATTR_ANY)};
				if (nullptr != p_status)
				{
					p_status->SetBoolCurrent(p_entry->GetBoolCurrent());
					p_status->SetUpdated(true);
				}
			}
		}
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	MUTE機能（後席映像面のMUTE要求）Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionMuteRearVideo(const struct VhalPropertyEntry * const p_entry)
{
	int32_t result{VHAL_SUCCESS};
	VHAL_LOGV_IN();
	VHAL_LOGI("Mute Rear Video [%d] -> [%d]", p_entry->GetBoolPrev(), p_entry->GetBoolCurrent());

	/* 後席スクリーン有効チェック */
	int32_t screen_id{};
	const int32_t ret{p_layout_mng_->GetScreenIdRear(screen_id)};
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("GetScreenIdRear error. ret=%d", ret);
		result = VHAL_ERR;
	}
	else
	{
		const bool rear_available{p_layout_mng_->IsScreenAvailable(screen_id)};
		if (false == rear_available)
		{
			VHAL_LOGE("rear screen is not available.");
			result = VHAL_ERR;
		}
		else
		{
			/* 後席映像面のMUTE処理 */
			if (nullptr == p_mute_)
			{
				VHAL_LOGW("CVhalMute is not created yet.");
			}
			else
			{
				p_mute_->SetMuteRearVideo(p_entry->GetBoolCurrent());
			}

			struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_MUTE_REAR_STS_VIDEO, VHAL_DATA_TYPE_BOOL, VHAL_ATTR_ANY)};
			if (nullptr != p_status)
			{
				p_status->SetBoolCurrent(p_entry->GetBoolCurrent());
				p_status->SetUpdated(true);
			}
		}
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	HDCP認証機能（HDCP(RSE) first認証開始要求）Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_****		エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionHdcpAuthRse(struct VhalPropertyEntry * const p_entry)
{
	int32_t	ret{VHAL_SUCCESS};
	VHAL_LOGV_IN();

	do
	{
		if (nullptr == p_hdcp_auth_mng_)
		{
			VHAL_LOGE("Invalid params. p_hdcp_auth_mng_=%p", p_hdcp_auth_mng_.get());
			ret = VHAL_ERR_PARAM;
			break;
		}

		/* HDCP(RSE) first認証開始要求 */
		ret = p_hdcp_auth_mng_->StartAuthRse();
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("StartAuth error. ret=%d", ret);
		}
	} while (false);

	p_entry->SetBoolCurrent(false);
	p_entry->SetUpdated(false);

	VHAL_LOGV_OUT();
	return ret;
}

/*****************************************************************************
 処理概要：	HDCP認証機能（HDCP(別体ディスプレイ) first認証開始要求）Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionHdcpAuthSeparateDisp(struct VhalPropertyEntry * const p_entry)
{
	VHAL_LOGV_IN();

	/* HDCP(別体ディスプレイ) first認証開始要求 */




	p_entry->SetBoolCurrent(false);
	p_entry->SetUpdated(false);

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	HDCP認証機能（HDCP(Instrument Cluster) first認証開始要求）Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionHdcpAuthICluster(struct VhalPropertyEntry * const p_entry)
{
	VHAL_LOGV_IN();

	/* HDCP(Instrument Cluster) first認証開始要求 */




	p_entry->SetBoolCurrent(false);
	p_entry->SetUpdated(false);

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	iviレイアウト情報設定機能Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionIviLayoutConfFile(const struct VhalPropertyEntry * const p_entry)
{
	VHAL_LOGV_IN();
	VHAL_LOGI("Layout Configuration File [%s]", p_entry->GetStrCurrent().c_str());
/* MISRA C++-2008 Rule 16-2-1 #if 0	*/ /* 動画再生暫定対応により起動直後にレイアウトファイル読み込み */
	/* iviレイアウト情報設定 */
	// int32_t ret = p_layout_mng_->InitLayout(p_entry->GetStrCurrent());
	// if (VHAL_SUCCESS != ret)
	// {
	// 	VHAL_LOGE("InitLayout error. ret=%d vpath=%s", ret, p_entry->GetStrCurrent().c_str());
	// 	result = VHAL_IVI_LAYOUT_STS_FAILED;
	// }

// MISRA C++-2008 Rule 16-2-1 #endif
	struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_IVI_LAYOUT_STS_CONF_FILE, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
	if (nullptr != p_status)
	{
		constexpr uint32_t result{VHAL_IVI_LAYOUT_STS_SUCCESS};
		p_status->SetNumCurrent(static_cast<int64_t>(result));
		p_status->SetUpdated(true);
	}
// MISRA C++-2008 Rule 16-2-1 #if 0
	/* Muteの初期化はレイアウトが決まってから */
	// p_mute_ = new CVhalMute;
	// ret = p_mute_->Initialize(p_layout_mng_, p_gpioctrl_, p_renderer_);
	// if (VHAL_SUCCESS != ret)
	// {
	// 	VHAL_LOGW("CVhalMute Initialize error. ret=%d", ret);
	// }
	// camera_control_.InitializeMute(p_mute_);
// MISRA C++-2008 Rule 16-2-1 #endif
	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}


/*****************************************************************************
 処理概要：	iviプロパティ取得機能（スクリーン）Generate登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
           		VHAL_ERR				異常終了
*****************************************************************************/
int32_t CVhalPropertyControl::GenerateIviScreenInfo(struct VhalPropertyEntry * const p_entry)
{
	int32_t result{VHAL_SUCCESS};
	std::map<std::string, std::string> params{};

	const int32_t ret{ParseQueryString(p_entry->GetQuery(), params)};
	if (VHAL_SUCCESS == ret)
	{
		p_entry->AllocateStrCurrent((p_layout_mng_->GetIviScreenInfo(params)));
	}
	
	if ("null" == p_entry->GetStrCurrent())
	{
		result = VHAL_ERR;
	}
	
	return result;
}

/*****************************************************************************
 処理概要：	iviプロパティ取得機能（レイヤー）Generate登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
           		VHAL_ERR				異常終了
*****************************************************************************/
int32_t CVhalPropertyControl::GenerateIviLayerInfo(struct VhalPropertyEntry * const p_entry)
{
	int32_t result{VHAL_SUCCESS};
	std::map<std::string, std::string> params{};

	const int32_t ret{ParseQueryString(p_entry->GetQuery(), params)};
	if (VHAL_SUCCESS == ret)
	{
		p_entry->AllocateStrCurrent((p_layout_mng_->GetIviLayerInfo(params)));
	}
	
	if ("null" == p_entry->GetStrCurrent())
	{
		result = VHAL_ERR;
	}
	
	return result;
}

/*****************************************************************************
 処理概要：	iviプロパティ取得機能（サーフェス）Generate登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
           		VHAL_ERR				異常終了
*****************************************************************************/
int32_t CVhalPropertyControl::GenerateIviSurfaceInfo(struct VhalPropertyEntry * const p_entry)
{
	int32_t result{VHAL_SUCCESS};
	std::map<std::string, std::string> params{};

	const int32_t ret{ParseQueryString(p_entry->GetQuery(), params)};
	if (VHAL_SUCCESS == ret)
	{
		p_entry->AllocateStrCurrent((p_layout_mng_->GetIviSurfaceInfo(params)));
	}
	
	if ("null" == p_entry->GetStrCurrent())
	{
		result = VHAL_ERR;
	}
	
	return result;
}

/*****************************************************************************
 処理概要：	iviレイヤー優先度変更機能Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionIviLayerOrder(struct VhalPropertyEntry * const p_entry)
{
	int32_t result{VHAL_SUCCESS};
	std::map<std::string, std::string> params{};

	VHAL_LOGV_IN();

	int32_t ret{ParseQueryString(p_entry->GetQuery(), params)};
	if (VHAL_SUCCESS == ret)
	{
		bool changed{false};
		ret = p_layout_mng_->SetIviLayerOrder(params, p_entry->GetNumCurrent(), changed);
		if (VHAL_SUCCESS == ret)
		{
			/* 設定値変化で更新ありとする */
			if (true == changed)
			{
				struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_IVI_LAYER_STS_ORDER, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
				if (nullptr != p_status)
				{
					p_status->SetUpdated(true);
				}
			}
		}
		else
		{
			VHAL_LOGE("SetIviLayerOrder error. ret=%d", ret);
			result = VHAL_ERR;
		}
		p_entry->SetNumCurrent(-1);
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	iviレイヤー優先度変更機能Generate登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::GenerateIviLayerOrder(struct VhalPropertyEntry * const p_entry)
{
	VHAL_LOGV_IN();

	std::map<std::string, std::string> params{};

	int32_t ret{ParseQueryString(p_entry->GetQuery(), params)};
	if (VHAL_SUCCESS == ret)
	{
		int64_t value{};
		ret = p_layout_mng_->GetIviLayerOrder(params, value);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetIviLayerOrder error. ret=%d", ret);
		}
		else
		{
			p_entry->SetNumCurrent(value);
		}
	}

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	クエリー文字列のパース
 引数    ：	const std::string					&query	(i)クエリー文字列
           	std::map<std::string, std::string>	&params	(o)パラメータ名と値のmap
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
****************************************************************************/
int32_t CVhalPropertyControl::ParseQueryString(const std::string &query, std::map<std::string, std::string> &params)
{
	VHAL_LOGV_IN();
	constexpr int32_t ret{VHAL_SUCCESS};
	size_t pos{0U};
	size_t next{0U};
	size_t equal{0U};
	std::string pair{};

	do
	{
		next = static_cast<size_t>(query.find('&', pos));
		if (next == std::string::npos)
		{
			pair = query.substr(pos);
		}
		else
		{
			if (next >= pos)
			{
				pair = query.substr(pos, next - pos);
				pos = next + 1U;
			}
		}

		equal = static_cast<size_t>(pair.find('='));
		if (equal == std::string::npos)
		{
			/* 名前と値がそろっていないパラメータは無視する */
			VHAL_LOGD("skip param [%s] which does not have '='.", pair.c_str());
		}
		else
		{
			const auto key = pair.substr(0U, equal);
			const auto value = pair.substr(equal + 1U);

			VHAL_LOGV("param key[%s] value[%s]", key.c_str(), value.c_str());
			params[key] = value;
		}

	} while (next != std::string::npos);

	VHAL_LOGV_OUT();
	return ret;
}

/*****************************************************************************
 処理概要：	画面キャプチャ取得機能Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
           		VHAL_ERR     			処理エラー
*****************************************************************************/
int32_t CVhalPropertyControl::ActionScreenCapture(struct VhalPropertyEntry * const p_entry)
{
	int32_t result{VHAL_CAPTURE_STS_FAILED};
	int32_t ret{VHAL_SUCCESS};
	std::map<std::string, std::string> params{};

	VHAL_LOGV_IN();
	VHAL_LOGI("query[%s]", p_entry->GetQuery().c_str());

	/* スクリーンショット取得要求のパラメータがtrue */
	bool request{p_entry->GetBoolCurrent()};
	if (true == request)
	{
		if (nullptr != p_screen_shot_micon_)
		{
			/* スクリーンショット取得要求中か確認 */
			std::string filepath{};
			p_screen_shot_micon_->GetDestFilePath(filepath);
			/* 格納先ファイルパスが無い場合は、 取得要求中でない */
			if (true == filepath.empty())
			{
				if (VHAL_SUCCESS == ParseQueryString(p_entry->GetQuery(), params))
				{
					/* path */
					const auto itr_path = params.find("path");
					if (itr_path == params.end())
					{
						VHAL_LOGE("parameter[path] is not found");
					}
					else
					{
						/* スクリーンショット格納先ファイルパス取得 */
						const std::string dest_path{itr_path->second};
						/* id優先で使う */
						const auto itr_id = params.find("ivi_id");
						if (itr_id != params.end())
						{
							VHAL_LOGD("ivi_id=%s", itr_id->second.c_str());
							try {
								const int32_t ivi_id{ std::stoi(itr_id->second) };
								if (VHAL_SUCCESS == p_screen_shot_micon_->SendScreenShotRequest(ivi_id, dest_path))
								{
									result = VHAL_CAPTURE_STS_SUCCESS;
								}
							} catch (const std::invalid_argument& e) {
								/* 数値として解釈できない場合 */
								VHAL_LOGE("Invalid argument: cannot convert ivi_id '%s' to integer: %s", itr_id->second.c_str(), e.what());
							} catch (const std::out_of_range& e) {
								/* 範囲外の値の場合 */
								VHAL_LOGE("Out of range: ivi_id '%s' value too large: %s", itr_id->second.c_str(), e.what());
							}
						}
						else
						{
							/* name */
							const auto itr_name = params.find("name");
							if (itr_name == params.end())
							{
								VHAL_LOGE("parameter[ivi_id/name] is not found");
							}
							else
							{
								VHAL_LOGD("name=%s", itr_name->second.c_str());
								/* スクリーン名からivi_idを取得 */
								int32_t ivi_id{};
								if (VHAL_SUCCESS != p_layout_mng_->GetScreenIdCommon(itr_name->second, ivi_id))
								{
									VHAL_LOGE("GetScreenIdCommon(name=%s) error.", itr_name->second.c_str());
								}
								else
								{
									if ((VHAL_SUCCESS == p_screen_shot_micon_->SendScreenShotRequest(ivi_id, dest_path)))
									{
										result = VHAL_CAPTURE_STS_SUCCESS;
									}
								}
							}
						}
					}
				}

				if (VHAL_CAPTURE_STS_SUCCESS != result)
				{
					/* パラメータ不正/異常の場合はStatus更新 */
					struct VhalPropertyEntry *p_status{nullptr};
					p_status = GetPropertyEntry(VHAL_PROP_CAP_SCRN_STS_BITMAP, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);
					if (nullptr != p_status)
					{
						p_status->SetNumCurrent(static_cast<int64_t>(result));
						p_status->SetUpdated(true);
					}
				}
				ret = VHAL_SUCCESS;
			}
			else
			{
				VHAL_LOGE("ScreenCapture already in progress.");
				ret = VHAL_ERR;
			}
		}
		else
		{
			VHAL_LOGE("p_screen_shot_micon_ is nullptr.");
			ret = VHAL_ERR;
		}
	}

	p_entry->SetBoolCurrent(false);
	p_entry->SetUpdated(false);

	VHAL_LOGV_OUT();
	return ret;
}

/*****************************************************************************
 処理概要：	サーフェスキャプチャ取得機能Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionSurfaceScreenCapture(struct VhalPropertyEntry * const p_entry)
{
	int32_t result{VHAL_CAPTURE_STS_FAILED};
	int32_t ret{VHAL_SUCCESS};
	std::map<std::string, std::string> params{};

	VHAL_LOGV_IN();
	VHAL_LOGI("query[%s]", p_entry->GetQuery().c_str());

	ret = ParseQueryString(p_entry->GetQuery(), params);
	if (VHAL_SUCCESS == ret)
	{
		/* path */
		const auto itr_path = params.find("path");
		if (itr_path == params.end())
		{
			VHAL_LOGE("parameter[path] is not found");
		}
		else
		{
			struct VhalScreenShotData screenshot_data{};
			screenshot_data.SetFilePath(itr_path->second);
			screenshot_data.SetType(ScreenShotType::SURFACE);

			/* id優先で使う */
			const auto itr_id = params.find("ivi_id");
			if (itr_id != params.end())
			{
				VHAL_LOGD("ivi_id=%s", itr_id->second.c_str());
				screenshot_data.SetIviId(std::stoi(itr_id->second));
				ret = p_screen_shot_->Notify(screenshot_data);
				if (VHAL_SUCCESS == ret)
				{
					result = VHAL_CAPTURE_STS_SUCCESS;
				}
			}
			else
			{
				/* name */
				const auto itr_name = params.find("name");
				if (itr_name == params.end())
				{
					VHAL_LOGE("parameter[ivi_id/name] is not found");
				}
				else
				{
					VHAL_LOGD("name=%s", itr_name->second.c_str());
					/* サーフェス名からivi_idを取得 */
					int32_t surface_id{};
					ret = p_layout_mng_->GetSurfaceIdCommon(itr_name->second, surface_id);
					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("GetSurfaceIdCommon(name=%s) error. ret=%d", itr_name->second.c_str(), ret);
					}
					else
					{
						screenshot_data.SetIviId(surface_id);
						ret = p_screen_shot_->Notify(screenshot_data);
						if (VHAL_SUCCESS == ret)
						{
							result = VHAL_CAPTURE_STS_SUCCESS;
						}
					}
				}
			}
		}
	}

	if (VHAL_CAPTURE_STS_SUCCESS != result)
	{
		/* パラメータ不正/異常の場合はStatus更新 */
		struct VhalPropertyEntry *p_status{nullptr};
		p_status = GetPropertyEntry(VHAL_PROP_CAP_SURF_STS_BITMAP, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);
		if (nullptr != p_status)
		{
			p_status->SetNumCurrent(static_cast<int64_t>(result));
			p_status->SetUpdated(true);
		}
	}

	/* For the one-shot event, the property value must be set as false after the action is performed */
	p_entry->SetBoolCurrent(false);
	p_entry->SetUpdated(false);

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	汎用動画再生機能（動画再生パラメータ設定）Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionMovieFrontPrepare(struct VhalPropertyEntry * const p_entry)
{
	std::vector<std::string> rect_prop{{VHAL_PROP_MOVIE_FRONT_CTL_OUTPUT_X, VHAL_PROP_MOVIE_FRONT_CTL_OUTPUT_Y, VHAL_PROP_MOVIE_FRONT_CTL_OUTPUT_W, VHAL_PROP_MOVIE_FRONT_CTL_OUTPUT_H}};
	int32_t result{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_MOVIE_FRONT_STS_RESULT, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
	if (nullptr != p_status)
	{
		CVhalMovieControl::MovieParameter param{};
		struct VhalPropertyEntry *p_control{GetPropertyEntry(VHAL_PROP_MOVIE_FRONT_CTL_FILE_PATH, VHAL_DATA_TYPE_STR, VHAL_ATTR_ANY)};
		
		if (nullptr != p_control)
		{
			param.SetFilePath(p_control->GetStrCurrent());
		}
	
		int32_t rect_current[4]{};
		for (uint32_t i{0U}; i < rect_prop.size(); ++i)
		{
			p_control = GetPropertyEntry(rect_prop[i], VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);
			if (nullptr != p_control)
			{
				rect_current[i] = static_cast<int32_t>(p_control->GetNumCurrent());
			}
		}
	
		param.SetX(static_cast<uint32_t>(rect_current[0]));
		param.SetY(static_cast<uint32_t>(rect_current[1]));
		param.SetWidth(static_cast<uint32_t>(rect_current[2]));
		param.SetHeight(static_cast<uint32_t>(rect_current[3]));
		
		constexpr int32_t screen_id{0};
		int32_t screen_width{0};
		int32_t screen_height{0};
		int32_t ret{p_layout_mng_->GetScreenSize(screen_id, screen_width, screen_height)};
		if (VHAL_SUCCESS == ret)
		{
			ret = p_movie_control_->PrepareFrontMovie(static_cast<uint32_t>(screen_width), static_cast<uint32_t>(screen_height), param);
			if (VHAL_SUCCESS == ret)
			{
				const int32_t movie_status{p_movie_control_->GetFrontMovieStatus()};
				if (p_status->GetNumCurrent() != movie_status)
				{
					p_status->SetNumCurrent(static_cast<int64_t>(movie_status));
					p_status->SetUpdated(true);
				}
			}
			else
			{
				VHAL_LOGE("PrepareFrontMovie error. ret=%d", ret);
				result = ret;
			}
		}
		else
		{
			VHAL_LOGE("GetScreenSize error screenid=%d ret=%d.", screen_id, ret);
			result = ret;
		}
	}
	p_entry->SetBoolCurrent(false);
	p_entry->SetUpdated(false);

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	汎用動画再生機能（設定した動画再生パラメータクリア）Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionMovieFrontClear(struct VhalPropertyEntry * const p_entry)
{
	std::vector<std::string> rect_prop{{VHAL_PROP_MOVIE_FRONT_CTL_OUTPUT_X, VHAL_PROP_MOVIE_FRONT_CTL_OUTPUT_Y, VHAL_PROP_MOVIE_FRONT_CTL_OUTPUT_W, VHAL_PROP_MOVIE_FRONT_CTL_OUTPUT_H}};
	int32_t ret{VHAL_SUCCESS};

	VHAL_LOGV_IN();
	struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_MOVIE_FRONT_STS_RESULT, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
	if (nullptr != p_status)
	{
		/* 内部パラメータクリア */
		ret = p_movie_control_->ClearFrontMovie();
		if (VHAL_SUCCESS == ret)
		{
			/* プロパティ値のクリア */
			struct VhalPropertyEntry *p_control{GetPropertyEntry(VHAL_PROP_MOVIE_FRONT_CTL_FILE_PATH, VHAL_DATA_TYPE_STR, VHAL_ATTR_ANY)};
			if (nullptr != p_control)
			{
				p_control->SetStrCurrent("");
				p_control->SetUpdated(true);
			}
		
			for (uint32_t i{0U}; i < rect_prop.size(); ++i)
			{
				p_control = GetPropertyEntry(rect_prop[i], VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);
				if (nullptr != p_control)
				{
					p_control->SetNumCurrent(0);
					p_control->SetUpdated(true);
				}
			}

			const int32_t movie_status{p_movie_control_->GetFrontMovieStatus()};
			if (p_status->GetNumCurrent() != movie_status)
			{
				p_status->SetNumCurrent(static_cast<int64_t>(movie_status));
				p_status->SetUpdated(true);
			}
		}
	}
	p_entry->SetBoolCurrent(false);
	p_entry->SetUpdated(false);

	VHAL_LOGV_OUT();
	return ret;
}

/*****************************************************************************
 処理概要：	汎用動画再生機能（動画再生開始）Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionMovieFrontStart(struct VhalPropertyEntry * const p_entry)
{
	int32_t ret{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_MOVIE_FRONT_STS_RESULT, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};	
	if (nullptr != p_status)
	{
		ret = p_movie_control_->StartFrontMovie();
		const int32_t movie_status{p_movie_control_->GetFrontMovieStatus()};
		if (p_status->GetNumCurrent() != movie_status)
		{
			/*  動画再生中への遷移はconfigureのタイミングとする */
			if (VHAL_MOVIE_STS_PLAYING != movie_status)
			{
				p_status->SetNumCurrent(static_cast<int64_t>(movie_status));
				p_status->SetUpdated(true);
			}
		}
	}
	p_entry->SetBoolCurrent(false);
	p_entry->SetUpdated(false);

	VHAL_LOGV_OUT();
	return ret;
}

/*****************************************************************************
 処理概要：	汎用動画再生機能（動画再生中止）Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionMovieFrontCancel(struct VhalPropertyEntry * const p_entry)
{
	int32_t ret{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_MOVIE_FRONT_STS_RESULT, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
	if (nullptr != p_status)
	{
		ret = p_movie_control_->CancelFrontMovie();
		const int32_t movie_status{p_movie_control_->GetFrontMovieStatus()};
		if (p_status->GetNumCurrent() != movie_status)
		{
			p_status->SetNumCurrent(static_cast<int64_t>(movie_status));
			p_status->SetUpdated(true);
		}		
	}
	p_entry->SetBoolCurrent(false);
	p_entry->SetUpdated(false);

	VHAL_LOGV_OUT();
	return ret;
}

/*****************************************************************************
 処理概要：	汎用動画再生結果格納関数
 引数    ：	int32_t result	(i)動画再生結果
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControl::UpdateMovieStartResult(const int32_t result)
{
	struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_MOVIE_FRONT_STS_RESULT, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
	if (nullptr != p_status)
	{
		std::vector<std::string> names{};
		switch(result)
		{
			case VHAL_MOVIE_STS_PLAYING:
			case VHAL_MOVIE_STS_FINISHED:
			case VHAL_MOVIE_STS_FAILED:
				p_status->SetNumCurrent(static_cast<int64_t>(result));

				names.push_back(VHAL_PROP_MOVIE_FRONT_STS_RESULT);
				p_main_->UpdatedProperties(names);
				break;
		
			case VHAL_MOVIE_STS_NONE:
			case VHAL_MOVIE_STS_READY:
			case VHAL_MOVIE_STS_CANCELED:
				VHAL_LOGE("unexpected transition status=%d.",result);
				break;			
			
			default:
				VHAL_LOGE("status error status=%d.",result);
				break;
		}
	}
}

/*****************************************************************************
 処理概要：	VideoHAL動作設定機能（車両ディスプレイ情報）Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionSettingDisplayType(const struct VhalPropertyEntry * const p_entry)
{
	VHAL_LOGV_IN();
	VHAL_LOGI("vehicle display type [%ld] -> [%ld]", p_entry->GetNumPrev(), p_entry->GetNumCurrent());

	/* 車両ディスプレイ情報設定処理 */



	struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_SETTING_STS_DISP, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
	if (nullptr != p_status)
	{
		p_status->SetNumCurrent(p_entry->GetNumCurrent());
		p_status->SetUpdated(true);
	}

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	VideoHAL動作設定機能（接続されているカメラ種別）Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
                VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionSettingConnectedCamera(const struct VhalPropertyEntry * const p_entry)
{
	int64_t	cameraType{VHAL_CONNECTED_CAMERA_NONE};
	int32_t result{VHAL_SUCCESS};
	bool cameratype_valid{true};

	const std::vector<int32_t> camera_connected_type{{
		VHAL_CONNECTED_CAMERA_INVALID,
		VHAL_CONNECTED_CAMERA_NONE,
		VHAL_CONNECTED_CAMERA_ABGM,
		VHAL_CONNECTED_CAMERA_DBGM,
		VHAL_CONNECTED_CAMERA_DPVM_NORMAL,
		VHAL_CONNECTED_CAMERA_DPVM_WIDE,
		VHAL_CONNECTED_CAMERA_DPVM_FULLHD,
		VHAL_CONNECTED_CAMERA_SBGM_CAA,
		VHAL_CONNECTED_CAMERA_BGM_ADU,
		VHAL_CONNECTED_CAMERA_SIM,
		VHAL_CONNECTED_CAMERA_PVM,
		VHAL_CONNECTED_CAMERA_MTM,
		VHAL_CONNECTED_CAMERA_PVM_METER,
		VHAL_CONNECTED_CAMERA_MTM_METER
	}};

	VHAL_LOGV_IN();

	const auto find_result = std::find(camera_connected_type.begin(), camera_connected_type.end(), p_entry->GetNumCurrent());
	if (find_result == camera_connected_type.end())
	{
		VHAL_LOGE("error camera Type[%ld]", p_entry->GetNumCurrent());
		cameratype_valid = false;
		result = VHAL_ERR_PARAM;
	}
	else
	{
		/* カメラ種別を取得 */
		int32_t	ret{VHAL_SUCCESS};
		ret = GetValueNumber(VHAL_PROP_SETTING_STS_CNCT_CAMERA, cameraType);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("Camera type[%ld] error. ret=%d", cameraType, ret);
			/* 現在のカメラ種別が取得出来なくても処理を進める */
		}

		VHAL_LOGI("connected camera type [%ld] -> [%ld]", cameraType, p_entry->GetNumCurrent());
		/* 接続されているカメラ種別変更 */
		if (cameraType != p_entry->GetNumCurrent())
		{
			cameraType = p_entry->GetNumCurrent();

			/* キャプチャ設定の有無を取得 */
			bool camera_set{true};
			const bool is_capture{camera_control_.IsCameraCapture()};
			/* キャプチャ未設定の場合 */
			if (false == is_capture)
			{
				VhalCameraDisplayStatus current_cam_disp_status{};
				ret = p_fileobserver_control_->GetCameraDisplayStatus(current_cam_disp_status);
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("GetCameraDisplayStatus error. ret=%d", ret);
					camera_set = false;
					cameratype_valid = false;
					result = VHAL_ERR_PARAM;
				}
				else if (VhalCameraDisplayStatus::kReady != current_cam_disp_status)
				{
					/* カメラ制御権がReady以外の場合、キャプチャ設定は実施しない */
					camera_set = false;
				}
				else
				{
					/* カメラ制御権がReadyの場合、本API内でキャプチャ設定を実施する */
					/* Settingカメラキャプチャはこの箇所では実施しない */
					// ret = SettingCameraCapture();
					// if (VHAL_SUCCESS != ret)
					// {
					// 	VHAL_LOGE("SettingCameraCapture error. ret=%d", ret);
					// 	camera_set = false;
					// }
				}
			}

			if (true == camera_set)
			{
				/* カメラ設定済なら設定解除を実施する */
				camera_control_.ReleaseCameraCapture();

				ret = camera_control_.SettingtCameraCapture(p_capture_control_.get(), cameraType, false);
				if (VHAL_SUCCESS != ret)
				{
					if (VHAL_CONNECTED_CAMERA_NONE < cameraType)
					{
						VHAL_LOGE("SettingtCameraCapture error. ret=%d", ret);
						cameraType = VHAL_CONNECTED_CAMERA_NONE;
						result = VHAL_ERR_PARAM;
					}
				}
				else
				{
					/* 実行状態(カメラパス)を取得する */
					std::string	path{};
					(void)p_layout_mng_->GetCameraPath(path);
					int32_t	disp{VHAL_CAMERA_DISPLAY_UNKNOWN};
					if (!path.empty())
					{
						disp = VHAL_CAMERA_DISPLAY_CENTER;
					}

					/* カメラ映像表示 */
					if (VHAL_CAMERA_DISPLAY_CENTER == disp)
					{
						ret = camera_control_.SetCameraPath(path);
						if (VHAL_SUCCESS != ret)
						{
							VHAL_LOGE("SetCameraPath error. ret=%d path=%s", ret, path.c_str());
						}
					}

					/* 映像パス設定可否判定状態Update（コールバック） */
					struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_VPATH_CAMERA_STS_AVAILABLE, VHAL_DATA_TYPE_BOOL, VHAL_ATTR_ANY)};
					if (nullptr != p_status)
					{
						p_status->SetUpdated(true);
					}
				}
			}
		}
	}

	if(true == cameratype_valid)
	{
		struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_SETTING_STS_CNCT_CAMERA, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
		if (nullptr != p_status)
		{
			p_status->SetNumCurrent(cameraType);
			p_status->SetUpdated(true);
		}
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	VideoHAL動作設定機能（接続されているRSE種別）Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionSettingConnectedRSE(const struct VhalPropertyEntry * const p_entry)
{
	int32_t result{VHAL_SUCCESS};
	VHAL_LOGV_IN();
	VHAL_LOGI("connected RSE type [%ld] -> [%ld]", p_entry->GetNumPrev(), p_entry->GetNumCurrent());

	do
	{
		if ((VHAL_CONNECTED_RSE_INVALID > p_entry->GetNumCurrent()) ||
		    (VHAL_CONNECTED_RSE_FULL < p_entry->GetNumCurrent()))
		{
			VHAL_LOGE("Invalid params. connected RSE type =[%ld]", p_entry->GetNumCurrent());
			result = VHAL_ERR_PARAM;
			break;
		}

		/* 接続されているRSE種別設定処理 */
		p_layout_mng_->SetConnectedRse(I64ToI32(p_entry->GetNumCurrent()));

		if (nullptr != p_hdcp_auth_mng_)
		{
			p_hdcp_auth_mng_->SetConnectedRse(I64ToI32(p_entry->GetNumCurrent()));
		}

		/* 接続されているRSE種別設定(MOST) */
		p_micon_comm_control_->SetConnectedRse(I64ToI32(p_entry->GetNumCurrent()));
		if ((VHAL_CONNECTED_RSE_FULL == I64ToI32(p_entry->GetNumCurrent())) || (VHAL_CONNECTED_RSE_DOP == I64ToI32(p_entry->GetNumCurrent())))
		{
			/* フルRSE 又は用品RSE接続時に有効 */
			/* Most初期化済み状態取得 */
			const bool initialized_most{p_micon_comm_control_->IsInitializedMost()};
			if (false == initialized_most)
			{
				/* RSE接続（初回） */
				/* MOST初期化 */
				int32_t ret{p_micon_comm_control_->InitializeMost()};
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGW("InitializeMost() error. ret=%d", ret);
				}
				else
				{
					const bool ret_cold_boot{p_sysdbctrl_->IsColdBoot()};
					/* コールドブート(+B起動)状態 */
					if (true == ret_cold_boot)
					{
						/* 後席専用映像パス設定(MOST) */
						ret = p_micon_most_video_info_->SetConnectedRse(MostConnectedState::kConnectedFirst);
						if (VHAL_SUCCESS != ret)
						{
							VHAL_LOGE("p_micon_most_video_info_->SetConnectedRse(kConnectedFirst) error. ret=%d", ret);
						}
					}
				}
			}
			else
			{
				/* RSE接続（脱落復帰） */
				/* 後席専用映像パス設定(MOST) */
				const int32_t ret{p_micon_most_video_info_->SetConnectedRse(MostConnectedState::kReConnected)};
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("p_micon_most_video_info_->SetConnectedRse(kReConnected) error. ret=%d", ret);
				}
			}
		}
		else
		{
			/* RSE脱落 */
			/* 後席専用映像パス設定(MOST) */
			const int32_t ret{p_micon_most_video_info_->SetConnectedRse(MostConnectedState::kDisConnected)};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("p_micon_most_video_info_->SetConnectedRse(kDisConnected) error. ret=%d", ret);
			}
		}

		/* FULLRSE接続時MUTE OFF設定 */
		if (VHAL_CONNECTED_RSE_FULL == p_entry->GetNumCurrent())
		{
			p_mute_->SetMuteRearDisp(false);
		}

		struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_SETTING_STS_CNCT_RSE, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
		if (nullptr != p_status)
		{
			p_status->SetNumCurrent(p_entry->GetNumCurrent());
			p_status->SetUpdated(true);
		}
	} while (false);

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	VideoHAL動作設定機能（別体ディスプレイ接続状態）Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionSettingConnectedSeparateDisp(const struct VhalPropertyEntry * const p_entry)
{
	VHAL_LOGV_IN();
	VHAL_LOGI("connected separate display type [%ld] -> [%ld]", p_entry->GetNumPrev(), p_entry->GetNumCurrent());

	/* 別体ディスプレイ接続状態設定処理 */



	struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_SETTING_STS_CNCT_SPRT, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
	if (nullptr != p_status)
	{
		p_status->SetNumCurrent(p_entry->GetNumCurrent());
		p_status->SetUpdated(true);
	}

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	VideoHAL動作設定機能（昼夜モード）Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionSettingDayNight(const struct VhalPropertyEntry * const p_entry)
{
	int32_t result{VHAL_SUCCESS};
	VHAL_LOGV_IN();
	VHAL_LOGI("day or night mode [%ld] -> [%ld]", p_entry->GetNumPrev(), p_entry->GetNumCurrent());

	do
	{	
		/* 昼夜モード設定処理 */
		int32_t ret{p_color_mng_->SetDayNight(I64ToI32(p_entry->GetNumCurrent()))};
		if (VHAL_ERR_PARAM == ret)
		{
			VHAL_LOGE("SetDayNight error. ret=%d", ret);
			result = VHAL_ERR_PARAM;
			break;
		}
		else if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetDayNight error. ret=%d", ret);
		}
		else
		{
			/* 処理なし */
		}

		/* 映像パス未設定時、画質モード通知は不要のため送信をガードする */
		const CVhalColorRecord::ColorType color_type{p_color_mng_->GetVideoPathColorType()};
		if (CVhalColorRecord::ColorType::COLOR_TYPE_DAYNIGHT == color_type)
		{
			/* 画質調整設定適用 */
			ret = p_color_mng_->ApplyAdjustment(true);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("ApplyAdjustment error. ret=%d", ret);
			}
		}

		/* 昼夜モードを切り替えたら、現在のStep値(status)も更新が必要 */
		/* 画質調整STEP 状態プロパティ設定 */
		ret = UpdateStatusColorStep(COLOR_STEP_TYPE_ALL);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("UpdateStatusColorStep error. ret=%d", ret);
		}

		struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_SETTING_STS_DAY_NIGHT, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
		if (nullptr != p_status)
		{
			p_status->SetNumCurrent(p_entry->GetNumCurrent());
			p_status->SetUpdated(true);
		}
	} while (false);

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	VideoHAL動作設定機能（テーマカラー）Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionSettingThemeColor(const struct VhalPropertyEntry * const p_entry)
{
	int32_t result{VHAL_SUCCESS};

	VHAL_LOGV_IN();
	VHAL_LOGI("theme color [%ld] -> [%ld]", p_entry->GetNumPrev(), p_entry->GetNumCurrent());

	do
	{
		/* テーマカラー設定処理 */
		int32_t ret{p_color_mng_->SetThemeColor(I64ToI32(p_entry->GetNumCurrent()))};
		if (VHAL_ERR_PARAM == ret)
		{
			VHAL_LOGE("SetThemeColor error. ret=%d", ret);
			result = VHAL_ERR_PARAM;
			break;
		}
		else if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetThemeColor error. ret=%d", ret);
		}
		else
		{
			/* 処理なし */
		}
		/* 画質調整設定適用 */
		ret = p_color_mng_->ApplyAdjustment(false);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("ApplyAdjustment error. ret=%d", ret);
		}

		/* テーマカラーを切り替えたら、現在のStep値(status)も更新が必要 */
		/* 画質調整STEP 状態プロパティ設定 */
		ret = UpdateStatusColorStep(COLOR_STEP_TYPE_ALL);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("UpdateStatusColorStep error. ret=%d", ret);
		}
	
		struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_SETTING_STS_THEME_COLOR, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
		if (nullptr != p_status)
		{
			/* 設定値変化チェック */
			if (p_status->GetNumCurrent() != p_entry->GetNumCurrent())
			{
				p_status->SetNumCurrent(p_entry->GetNumCurrent());
				p_status->SetUpdated(true);
			}
		}
	} while (false);

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	ヒーコンエリアサイズ機能（ヒーコンエリアサイズ設定）Action登録関数
 引数    ：	struct VhalPropertyEntry	*p_entry	プロパティエントリーポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ActionHeaconAreaSize(struct VhalPropertyEntry * const p_entry)
{
	VHAL_LOGV_IN();
	int32_t ret{VHAL_SUCCESS};
	int32_t result{VHAL_SUCCESS};

	/* 短形設定プロパティ取得処理（共通） */
	struct VhalRectanglePropValues heacon_current{};
	struct VhalRectanglePropValues heacon_prev{};
	ret = GetRectangleControlCommon(front_heacon_control_, front_heacon_status_, heacon_current, heacon_prev);
	if (VHAL_SUCCESS == ret)
	{
		constexpr int32_t screen_id{0}; /* 前席 */
		int32_t screen_width{0};
		int32_t screen_height{0};
		ret = p_layout_mng_->GetScreenSize(screen_id, screen_width, screen_height);
		if (VHAL_SUCCESS == ret)
		{
			/* ヒーコンエリア設定値の有効範囲チェック */
			/*  X：0固定 */
			/*  Y：0以上、画面解像度縦未満 */
			/*  W：0又は、画面解像度横 */
			/*  H：0以上、画面解像度縦以下 */
			if ( (0 == heacon_current.rect[VHAL_RECTANGLE_X]) &&
				 ((0 <= heacon_current.rect[VHAL_RECTANGLE_Y]) && (screen_height > heacon_current.rect[VHAL_RECTANGLE_Y])) &&
				 ((0 == heacon_current.rect[VHAL_RECTANGLE_W]) || (screen_width == heacon_current.rect[VHAL_RECTANGLE_W])) &&
				 ((0 <= heacon_current.rect[VHAL_RECTANGLE_H]) && (screen_height >= heacon_current.rect[VHAL_RECTANGLE_H])) )
			{
				/* ヒーコンエリア無効/有効切替設定チェック */
				/*  無効設定（WとH：どちらかが0） */
				if ( ((0 == heacon_current.rect[VHAL_RECTANGLE_W]) || (0 == heacon_current.rect[VHAL_RECTANGLE_H])) ||
				/*  有効設定（Y+Hは画面解像度縦） */
					 (screen_height == (heacon_current.rect[VHAL_RECTANGLE_Y] + heacon_current.rect[VHAL_RECTANGLE_H])) )
				{
					VHAL_LOGI("rect (%d,%d,%d,%d)->(%d,%d,%d,%d)",
						heacon_prev.rect[VHAL_RECTANGLE_X], heacon_prev.rect[VHAL_RECTANGLE_Y], heacon_prev.rect[VHAL_RECTANGLE_W], heacon_prev.rect[VHAL_RECTANGLE_H],
						heacon_current.rect[VHAL_RECTANGLE_X], heacon_current.rect[VHAL_RECTANGLE_Y], heacon_current.rect[VHAL_RECTANGLE_W], heacon_current.rect[VHAL_RECTANGLE_H]);

					/* ヒーコン短形設定処理 */
					p_color_mng_->SetHeaconRectangle(static_cast<int32_t>(heacon_current.rect[VHAL_RECTANGLE_X]), static_cast<int32_t>(heacon_current.rect[VHAL_RECTANGLE_Y]), static_cast<int32_t>(heacon_current.rect[VHAL_RECTANGLE_W]), static_cast<int32_t>(heacon_current.rect[VHAL_RECTANGLE_H]));

					/* 短形状態プロパティ更新処理（共通） */
					ret = UpdateRectangleStatusCommon(front_heacon_status_, heacon_current);
					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("UpdateRectangleStatusCommon error. ret=%d", ret);
					}
				}
				else
				{
					/* ヒーコンエリア無効/有効切替不正 */
					VHAL_LOGE("heacon switch error. heacon_rect=(%d,%d,%d,%d), screen=(%dx%d)",
						heacon_current.rect[VHAL_RECTANGLE_X], heacon_current.rect[VHAL_RECTANGLE_Y], heacon_current.rect[VHAL_RECTANGLE_W], heacon_current.rect[VHAL_RECTANGLE_H],
						screen_width, screen_height);
					result = VHAL_ERR_PARAM;
				}
			}
			else
			{
				/* ヒーコンエリア設定値が有効範囲外 */
				VHAL_LOGE("heacon size error. heacon_rect=(%d,%d,%d,%d), screen=(%dx%d)",
					heacon_current.rect[VHAL_RECTANGLE_X], heacon_current.rect[VHAL_RECTANGLE_Y], heacon_current.rect[VHAL_RECTANGLE_W], heacon_current.rect[VHAL_RECTANGLE_H],
					screen_width, screen_height);
				result = VHAL_ERR_PARAM;
			}
		}
		else
		{
			VHAL_LOGE("GetScreenSize error. ret=%d", ret);
		}
	}
	else
	{
		VHAL_LOGE("GetRectangleControlCommon error. ret=%d", ret);
	}

	/* one-shot event */
	p_entry->SetBoolCurrent(false);				/* プロパティ値は自動的にfalse */
	p_entry->SetUpdated(false);					/* callback通知無し */

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	HDMI接続状態更新実施
 引数    ：	int32_t state		(i)HDMI接続状態SysDB値
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControl::UpdateFilePropertyHdmi(const int32_t state)
{
	VHAL_LOGV_IN();

	int32_t	ret{VHAL_SUCCESS};
	ret = p_sysdbctrl_->SetValue(VHAL_SYSDB_PATH_CONN_INFO_HDMI, state);
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("SetValue(VHAL_SYSDB_PATH_CONN_INFO_HDMI) error. ret=%d", ret);
	}

	/* HDMI接続状態更新処理 */
	ChgConnInfoHdmi(state, false);

	/* HDMI接続済でない場合はビデオフォーマットとオーディオフォーマットを初期化 */
	if (DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECT != state)
	{
		/* HDMIビデオフォーマット情報を初期化 */
		int32_t		video_fmt{SYSINFO_CONN_INFO_VIDEO_FORMAT_INVALID};
		ret = p_sysdbctrl_->GetValue(VHAL_SYSDB_PATH_HDMI_VIDEO_FORMAT, &video_fmt);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetValue(%s) error. ret=%d", VHAL_SYSDB_PATH_HDMI_VIDEO_FORMAT.c_str(), ret);
		}
		/* 「未確定」以外ならば、「未確定」に更新 */
		if (SYSINFO_CONN_INFO_VIDEO_FORMAT_INVALID != video_fmt)
		{
			ret = p_sysdbctrl_->SetValue(VHAL_SYSDB_PATH_HDMI_VIDEO_FORMAT, SYSINFO_CONN_INFO_VIDEO_FORMAT_INVALID);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("SetValue(%s) error. ret=%d", VHAL_SYSDB_PATH_HDMI_VIDEO_FORMAT.c_str(), ret);
			}
		}

		/* HDMIオーディオフォーマット情報を初期化 */
		int32_t		audio_fmt{SYSINFO_CONN_INFO_AUDIO_FORMAT_INVALID};
		ret = p_sysdbctrl_->GetValue(VHAL_SYSDB_PATH_HDMI_AUDIO_FORMAT, &audio_fmt);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetValue(%s) error. ret=%d", VHAL_SYSDB_PATH_HDMI_AUDIO_FORMAT.c_str(), ret);
		}
		/* 「未確定」以外ならば、「未確定」に更新 */
		if (SYSINFO_CONN_INFO_AUDIO_FORMAT_INVALID != audio_fmt)
		{
			ret = p_sysdbctrl_->SetValue(VHAL_SYSDB_PATH_HDMI_AUDIO_FORMAT, SYSINFO_CONN_INFO_AUDIO_FORMAT_INVALID);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("SetValue(%s) error. ret=%d", VHAL_SYSDB_PATH_HDMI_AUDIO_FORMAT.c_str(), ret);
			}
		}

		/* HDCP認証キー書込み済フラグクリア */
		if (nullptr != p_capture_control_)
		{
			p_capture_control_->ClearHdcpAuthKeyWriteStatus();
		}
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	HDMI接続状態Update関数
 引数    ：	int32_t state				(i)HDMI接続状態SysDB値
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControl::UpdateSettingConnectedHdmiDevice(const int32_t state)
{
	VHAL_LOGV_IN();
	VHAL_LOGI("HDMI connected status connect=%d", state);

	struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_SETTING_STS_CNCT_HDMI, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
	if (nullptr == p_status)
	{
		VHAL_LOGE("UpdateSettingConnectedHdmiDevice() error.");
	}
	else
	{
		int64_t status{VHAL_CONNECTED_HDMI_NONE};
		int32_t connect{state};
		int32_t format{hdmi_control_.kUnknown};
		uint32_t width{0U};
		uint32_t height{0U};
		const bool available{hdmi_control_.Available(connect, format, width, height)};
		if (true == available)
		{
			status = VHAL_CONNECTED_HDMI_READY;
		}
		else
		{
			VHAL_LOGW("Invalid HDMI Status. connect=%d, format=%d[width=%d, height=%d]", connect, format, width, height);
		}

		if (p_status->GetNumCurrent() == status)
		{
			VHAL_LOGD("HDMI connect status(%lx) is not changed.", status);
		}
		else
		{
			/* HDMI接続状態変化がある時のみ再生・停止処理を行う */
			int32_t ret{VHAL_SUCCESS};
			ret = UpdateHdmiCaptureControl(status, width, height);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("UpdateHdmiCaptureControl error. ret=%d", ret);
			}

			p_status->SetNumCurrent(status);

			/* HDMI接続状態プロパティ更新通知*/
			std::vector<std::string> names{};
			names.push_back(VHAL_PROP_SETTING_STS_CNCT_HDMI);
			p_main_->UpdatedProperties(names);

			/* 映像パス設定可否判定状態Update（コールバック） */
			std::vector<std::string> videopath_names{};
			videopath_names.push_back(std::string("HDMI"));
			ret = UpdateVpathStatusAvailable(videopath_names);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("UpdateVpathStatusAvailable error. ret=%d", ret);
			}
		}
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	HDMIビデオフォーマット切替
 引数    ：	int32_t state		(i) 解像度値
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControl::ChangeHdmiVideoFormat(const int32_t state)
{
	VHAL_LOGV_IN();
	/* 切替時間計測用ログ[START] */
	VHAL_LOGI("change hdmi video format start state=%d", state);

	int32_t	ret{p_sysdbctrl_->SetValue(VHAL_SYSDB_PATH_HDMI_VIDEO_FORMAT, state)};
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("SetValue(VHAL_SYSDB_PATH_HDMI_VIDEO_FORMAT) error. ret=%d", ret);
	}

	struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_SETTING_STS_CNCT_HDMI, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
	if (nullptr == p_status)
	{
		VHAL_LOGE("ChangeHdmiVideoFormat() error.");
	}
	else
	{
		if (SYSINFO_CONN_INFO_VIDEO_FORMAT_ERROR != state)
		{
			int32_t ret{VHAL_SUCCESS};

			/* HDMI再生可能状態をチェック */
			int64_t status{VHAL_CONNECTED_HDMI_NONE};
			int32_t connect{hdmi_control_.kUnknown};
			int32_t format{state};
			uint32_t	width{0U};
			uint32_t	height{0U};
			const bool available{hdmi_control_.Available(connect, format, width, height)};
			if (true == available)
			{
				status = VHAL_CONNECTED_HDMI_READY;
			}
			else
			{
				VHAL_LOGW("Invalid HDMI Status. connect=%d, format=%d[width=%d, height=%d]", connect, format, width, height);
			}

			/* HDMIビデオフォーマット切替はHDMI接続状態変化に関わらず再生・停止処理を行う */
			ret = UpdateHdmiCaptureControl(status, width, height);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("UpdateHdmiCaptureControl error. ret=%d", ret);
			}

			/* HDMI接続状態プロパティはHDMI再生可否変更時またはビデオフォーマットが仕様外[80h]の場合に実施 */
			if ((p_status->GetNumCurrent() == status) && (SYSINFO_CONN_INFO_VIDEO_FORMAT_OUTOFSPEC != format))
			{
				VHAL_LOGD("HDMI connect status(%lx) is not changed.", status);
			}
			else
			{
				p_status->SetNumCurrent(status);

				/* HDMI接続状態プロパティ更新通知*/
				std::vector<std::string> names{};
				names.push_back(VHAL_PROP_SETTING_STS_CNCT_HDMI);
				p_main_->UpdatedProperties(names);

				/* 映像パス設定可否判定状態Update（コールバック） */
				std::vector<std::string> videopath_names{};
				videopath_names.push_back(std::string("HDMI"));
				ret = UpdateVpathStatusAvailable(videopath_names);
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("UpdateVpathStatusAvailable error. ret=%d", ret);
				}
			}
		}
	}

	/* 切替時間計測用ログ[END] */
	VHAL_LOGI("change hdmi video format end");
	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	HDMI映像キャプチャ制御更新
 引数    ：	const int64_t	status				(i) HDMI接続状態(VHAL_CONNECTED_HDMI_NONE/VHAL_CONNECTED_HDMI_READY)
           	const uint32_t	width				(i) 解像度幅
           	const uint32_t	height				(i) 解像度高さ
 戻り値  ：	処理結果
           		VHAL_ERR_****			エラー
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::UpdateHdmiCaptureControl(const int64_t status, const uint32_t width, const uint32_t height)
{
	int32_t ret{VHAL_SUCCESS};

	VHAL_LOGV_IN();
	/* HDMIキャプチャ終了 */
	hdmi_control_.Finalize();

	if (VHAL_CONNECTED_HDMI_READY == status)
	{
		/* HDMI再生 */
		ret = PlayHdmiCapture(width, height);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("PlayHdmiCapture() error. ret=%d", ret);
		}
	}
	/*
	   ここでは、上位からの表示状態設定があったわけではないため、サーフェス表示設定や表示状態変化通知は行わない。
	   （サーフェスのconfigure通知受信時にサーフェス表示設定を行う。）
	   表示状態変化通知を行うと、上位が通知をトリガーに表示状態を取得するが、サーフェス生成後の表示状態設定前であれば
	   非表示を返すことになり、上位は非表示を受け取るとHDMI表示部分に対し黒画を被せてしまうため、不要な更新通知を行わ
	   ないようにする。
	*/
		
	VHAL_LOGV_OUT();
	return ret;
}

/*****************************************************************************
 処理概要：	HDMI映像再生開始処理
 引数    ：	const uint32_t	width				(i) 解像度幅
           	const uint32_t	height				(i) 解像度高さ
 戻り値  ：	処理結果
           		VHAL_ERR_****			エラー
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::PlayHdmiCapture(const uint32_t width, const uint32_t height)
{
	int32_t		result{VHAL_SUCCESS};
	int32_t		ret{VHAL_SUCCESS};
	std::string	video_path{};

	VHAL_LOGV_IN();

	/* 前席映像パスがHDMIであれば、前席HDMI再生 */
	ret = p_layout_mng_->GetFrontPath(video_path);
	if (VHAL_SUCCESS == ret)
	{
		if (0 == video_path.compare("HDMI"))
		{
			/* 取得したHDMI解像度にて再生 */
			ret = hdmi_control_.Play(CVhalPropertyControlHdmi::CTRL_TARGET_FRONT, p_layout_mng_.get(), p_capture_control_.get(), width, height);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("hdmi play error. ret=%d", ret);
				result = ret;
			}
		}
	}
	else
	{
		VHAL_LOGE("GetFrontPath error. ret=%d", ret);
		result = ret;
	}

	/* 後席映像パスがHDMIであれば、後席HDMI再生 */
	ret = p_layout_mng_->GetRearPath(video_path);
	if (VHAL_SUCCESS == ret)
	{
		if (0 == video_path.compare("HDMI"))
		{
			/* 取得したHDMI解像度にて再生 */
			ret = hdmi_control_.Play(CVhalPropertyControlHdmi::CTRL_TARGET_REAR, p_layout_mng_.get(), p_capture_control_.get(), width, height);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("hdmi play error. ret=%d", ret);
				result = ret;
			}
		}
	}
	else
	{
		VHAL_LOGE("GetRearPath error. ret=%d", ret);
		result = ret;
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	HDMIオーディオフォーマット書込み
 引数    ：	int32_t audio_format		(i) オーディオフォーマット
 戻り値  ：	なし
 ****************************************************************************/
void CVhalPropertyControl::ChangeHdmiAudioFormat(const int32_t audio_format)
{
	VHAL_LOGV_IN();

	int32_t	ret{VHAL_SUCCESS};
	ret = p_sysdbctrl_->SetValue(VHAL_SYSDB_PATH_HDMI_AUDIO_FORMAT, audio_format);
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("SetValue(VHAL_SYSDB_PATH_HDMI_AUDIO_FORMAT) error. ret=%d", ret);
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	HDMI接続状態更新処理
 引数    ：	int32_t state				(i)HDMI接続状態SysDB値
         ：	bool initialized			(i)初期化時判定
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControl::ChgConnInfoHdmi(const int32_t state, const bool initialized)
{
	VHAL_LOGV_IN();

	int32_t ret{VHAL_SUCCESS};
	int32_t connect_state{state};

	VhalHdmiState hdmi_state{VhalHdmiState::kDisconnect};
	uint32_t retry_num{CVhalFileObserverControl::kRetryMaxNormal};
	uint32_t retry_wait{CVhalFileObserverControl::kRetryWaitNormal};

	/* HDMI接続状態確認 */
	if (true == initialized)
	{
		retry_num = CVhalFileObserverControl::kRetryMaxStartup;
		retry_wait = CVhalFileObserverControl::kRetryWaitStartup;
		ret = p_sysdbctrl_->GetValue(VHAL_SYSDB_PATH_CONN_INFO_HDMI, &connect_state);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SysDB %s Read error. ret=%d", VHAL_SYSDB_PATH_CONN_INFO_HDMI.c_str(), ret);
			connect_state = DAT_CNV_SYSINFO_CONN_INFO_STAT_DISCONNECT;
			hdmi_state = VhalHdmiState::kUnknown;
		}
	}

	if (DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECT == connect_state)
	{
		hdmi_state = VhalHdmiState::kConnect;
	}

	/* HDMI接続状態更新 */
	(void)p_fileobserver_control_->UpdatePropertyHdmi(true, hdmi_state, retry_num, retry_wait);

	/* HDCP認証キー書込み */
	ret = SetHdcpAuthKey(connect_state);
	if (VHAL_SUCCESS != ret)
	{
		/* 失敗時は関数内部でログ出力するため、ここではログレベル低めで出力 */
		VHAL_LOGD("SetHdcpAuthKey failed. ret=%d", ret);
	}

	if (VhalHdmiState::kUnknown != hdmi_state)
	{
		/* HDMI接続状態Update */
		UpdateSettingConnectedHdmiDevice(connect_state);
	}

	VHAL_LOGV_OUT();
}


/*****************************************************************************
 処理概要：	画質調整STEP 状態プロパティ更新
 引数    ：	ColorStepType type	(i) 画質調整STEPタイプ
           	bool update_notify	(i) Update通知（true:あり false:なし）
 戻り値  ：	処理結果
           		VHAL_ERR			異常終了
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::UpdateStatusColorStep(const ColorStepType type, const bool update_notify)
{
	int32_t	ret{0};
	int32_t	result{VHAL_SUCCESS};
	CVhalColorRecord::ColorImgAdjStep colorStep{};

	VHAL_LOGV("type=0x%x", type);

	/* 現在の映像ソースIDのSTEP設定を取得 */
	ret = p_color_mng_->GetCurrentColorStep(colorStep, p_color_mng_->GetVideoSourceId());
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("GetCurrentColorStep error. ret=%d", ret);
		result = VHAL_ERR;
	}
	else
	{
		/* コントラスト */
		if (COLOR_STEP_TYPE_CONTRAST == (COLOR_STEP_TYPE_CONTRAST & type))
		{
			struct VhalPropertyEntry *p_status{GetPropertyEntry(VHAL_PROP_VSRC_FRONT_STS_CONTRAST, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
			if (nullptr != p_status)
			{
				/* 設定値変化チェック */
				if (p_status->GetNumCurrent() != colorStep.contrast_)
				{
					VHAL_LOGD("Status Contrast=%d", colorStep.contrast_);
					p_status->SetNumCurrent(static_cast<int64_t>(colorStep.contrast_));
					if ( true == update_notify )
					{
						p_status->SetUpdated(true);
					}
				}
			}

			/* controlプロパティも合わせて更新 */
			p_status = GetPropertyEntry(VHAL_PROP_VSRC_FRONT_CTL_CONTRAST, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);
			if (nullptr != p_status)
			{
				VHAL_LOGD("Control Contrast=%d", colorStep.contrast_);
				p_status->SetNumCurrent(static_cast<int64_t>(colorStep.contrast_));
			}
		}

		/* 明るさ */
		if (COLOR_STEP_TYPE_BRIGHTNESS == (COLOR_STEP_TYPE_BRIGHTNESS & type))
		{
			struct VhalPropertyEntry *p_status{GetPropertyEntry(VHAL_PROP_VSRC_FRONT_STS_BRIGHT, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
			if (nullptr != p_status)
			{
				/* 設定値変化チェック */
				if (p_status->GetNumCurrent() != colorStep.brightness_)
				{
					VHAL_LOGD("Status Brightness=%d", colorStep.brightness_);
					p_status->SetNumCurrent(static_cast<int64_t>(colorStep.brightness_));
					if ( true == update_notify )
					{
						p_status->SetUpdated(true);
					}
				}
			}

			/* controlプロパティも合わせて更新 */
			p_status = GetPropertyEntry(VHAL_PROP_VSRC_FRONT_CTL_BRIGHT, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);
			if (nullptr != p_status)
			{
				VHAL_LOGD("Control Brightness=%d", colorStep.brightness_);
				p_status->SetNumCurrent(static_cast<int64_t>(colorStep.brightness_));
			}
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	ワイド設定 状態プロパティ更新
 引数    ：	VideoOutputTarget 	output_target	(i)映像出力先
 戻り値  ：	処理結果
           		VHAL_ERR			異常終了
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::UpdateStatusWideMode(const VideoOutputTarget output_target)
{
	uint32_t wide_mode{VHAL_WIDE_MODE_NORMAL};

	VHAL_LOGD("output_target=0x%x", output_target);

	/* 前席映像のワイド設定 */
	if (VIDEO_OUTPUT_TARGET_FRONT == (VIDEO_OUTPUT_TARGET_FRONT & output_target))
	{
		const int32_t ret{p_layout_mng_->GetFrontWideMode(wide_mode)};
		if (VHAL_SUCCESS == ret)
		{
			struct VhalPropertyEntry *p_entry{nullptr};
			VHAL_LOGD("wide_mode=%d", wide_mode);

			/* controlプロパティを更新 */
			p_entry = GetPropertyEntry(VHAL_PROP_VPATH_FRONT_CTL_WIDE, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);
			if (nullptr != p_entry)
			{
				p_entry->SetNumCurrent(static_cast<int64_t>(wide_mode));
			}
			/* statusプロパティを更新 */
			p_entry = GetPropertyEntry(VHAL_PROP_VPATH_FRONT_STS_WIDE, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);
			if (nullptr != p_entry)
			{
				p_entry->SetNumCurrent(static_cast<int64_t>(wide_mode));
			}
		}
	}

	/* 後席映像のワイド設定 */
	if (VIDEO_OUTPUT_TARGET_REAR == (VIDEO_OUTPUT_TARGET_REAR & output_target))
	{
		const int32_t ret{p_layout_mng_->GetRearWideMode(wide_mode)};
		if (VHAL_SUCCESS == ret)
		{
			struct VhalPropertyEntry *p_entry{nullptr};
			VHAL_LOGD("wide_mode=%d", wide_mode);

			/* controlプロパティを更新 */
			p_entry = GetPropertyEntry(VHAL_PROP_VPATH_REAR_CTL_WIDE, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);
			if (nullptr != p_entry)
			{
				p_entry->SetNumCurrent(static_cast<int64_t>(wide_mode));
			}
			/* statusプロパティを更新 */
			p_entry = GetPropertyEntry(VHAL_PROP_VPATH_REAR_STS_WIDE, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);
			if (nullptr != p_entry)
			{
				p_entry->SetNumCurrent(static_cast<int64_t>(wide_mode));
			}
		}
	}

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	可視状態設定 プロパティ更新
 引数    ：	const std::string	&video_path		(i)映像vpath文字列
         ：	const VideoOutputTarget output_target	(i)映像出力先
         ：	const bool update_notify				(i)Update通知（true:あり false:なし）
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_ERR			異常終了
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::UpdateStatusVisible(const std::string &video_path, const VideoOutputTarget output_target, const bool update_notify)
{
	bool visibility{false};
	int32_t ret{VHAL_SUCCESS};
	std::string control_name{};
	std::string status_name{};

	/* 前席 */
	if (0U < (VIDEO_OUTPUT_TARGET_FRONT & output_target))
	{
		control_name = VHAL_PROP_VPATH_FRONT_CTL_VISIBLE;
		status_name = VHAL_PROP_VPATH_FRONT_STS_VISIBLE;
		/* 可視状態取得 */
		ret = p_layout_mng_->GetFrontVisibility(visibility);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetFrontVisibility error. ret=%d", ret);
		}
	}
	/* 後席 */
	else if (0U < (VIDEO_OUTPUT_TARGET_REAR & output_target))
	{
		control_name = VHAL_PROP_VPATH_REAR_CTL_VISIBLE;
		status_name = VHAL_PROP_VPATH_REAR_STS_VISIBLE;
		/* 可視状態取得 */
		ret = p_layout_mng_->GetRearVisibility(visibility);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetRearVisibility error. ret=%d", ret);
		}
	}
	/* IC */
	else if (0U < (VIDEO_OUTPUT_TARGET_IC & output_target))
	{
		control_name = VHAL_PROP_VPATH_ICLUSTER_CTL_VISIBLE;
		status_name = VHAL_PROP_VPATH_ICLUSTER_STS_VISIBLE;
		/* 可視状態取得 */
		ret = p_layout_mng_->GetIClusterVisibility(visibility);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetIClusterVisibility error. ret=%d", ret);
		}
	}
	else
	{
		VHAL_LOGE("invalid output_target=0x%x, video path=%s", output_target, video_path.c_str());
		ret = VHAL_ERR_PARAM;
	}

	/* プロパティ更新 */
	if (VHAL_SUCCESS == ret)
	{
		struct VhalPropertyEntry *p_entry{nullptr};

		VHAL_LOGD("video_path=%s, output_target=0x%x visibility=%d", video_path.c_str(), output_target,visibility);
		/* 映像可視状態のcontrolプロパティを更新 */
		p_entry = GetPropertyEntry(control_name, VHAL_DATA_TYPE_BOOL, VHAL_ATTR_ANY);
		if (nullptr != p_entry)
		{
			p_entry->SetBoolCurrent(visibility);
			if (true == update_notify)
			{
				p_entry->SetUpdated(true);
			}
		}
		/* 映像可視状態のstatusプロパティを更新 */
		p_entry = GetPropertyEntry(status_name, VHAL_DATA_TYPE_BOOL, VHAL_ATTR_ANY);
		if (nullptr != p_entry)
		{
			p_entry->SetBoolCurrent(visibility);
			if (true == update_notify)
			{
				p_entry->SetUpdated(true);
			}
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	映像パス設定可否チェック
 引数    ：	const std::string	&video_path		(i)映像vpath文字列
         ：	VideoOutputTarget 	output_target	(i)映像出力先
 戻り値  ：	処理結果
           		false		設定不可
           		true		設定可
****************************************************************************/
bool CVhalPropertyControl::IsValidVideoPathAvailable(const std::string &video_path, const VideoOutputTarget output_target)
{
	bool available{false};

	/* 映像パスクリアは常に設定可(true) */
	if (video_path.empty())
	{
		available = true;
	}
	else
	{
		/* サポートチェック */
		bool already_path{false};
		bool is_valid{false};
		is_valid = p_layout_mng_->IsValidVideoPathCommon("", video_path, output_target, already_path);
		if (false == is_valid)
		{
			VHAL_LOGE("video_path [%s] is not supported, output_target[0x%x]", video_path.c_str(), output_target);
		}
		else
		{
			/* HDMI：常に設定可(true) */
			if (0 == video_path.compare("HDMI"))
			{
				available = true;
			}
			else if (0 == video_path.compare(VHAL_PATH_CAMERA))
			{
				/* CAMERA：接続状態の場合は設定可(true) */
				/* VideoHAL側でサーフェス作成するため、サーフェス存在確認は不要 */
				int32_t ret;
				int64_t cameraType{VHAL_CONNECTED_CAMERA_NONE};
				/* カメラ種別を取得 */
				ret = GetValueNumber(VHAL_PROP_SETTING_STS_CNCT_CAMERA, cameraType);
				if ((VHAL_SUCCESS != ret) || (VHAL_CONNECTED_CAMERA_NONE >= cameraType))
				{
					VHAL_LOGE("Camera type[%ld] error. ret=%d", cameraType, ret);
				}
				else
				{
					/* カメラ制御権取得 */
					/* Initialの場合は設定不可、Readyの場合は設定可 */
					VhalCameraDisplayStatus current_cam_disp_status{};
					ret = p_fileobserver_control_->GetCameraDisplayStatus(current_cam_disp_status);
					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("GetCameraDisplayStatus error. ret=%d", ret);
					}
					else if (VhalCameraDisplayStatus::kReady != current_cam_disp_status)
					{
						/* nothing to do */
					}
					else
					{
						const bool is_capture_waiting{camera_control_.GetWaitingCapture()};
						/* 動画再生開始中によるキャプチャ設定Wait状態なら設定不可 */
						if (false == is_capture_waiting)
						{
							available = true;
						}
					}
				}
			}
			else
			{
				/* その他の映像パス：サーフェスが存在する場合は設定可(true) */
				bool ret_available;
				ret_available = p_layout_mng_->IsValidSurfaceAvailable(video_path, output_target);
				if (true != ret_available)
				{
					VHAL_LOGI("not found surface. ret=%d", ret_available);
				}
				else
				{
					available = true;
				}
			}
		}
	}

	VHAL_LOGI("check video_path [%s], output_target[0x%x], available=%d", video_path.c_str(), output_target, available);
	return available;
}

/*****************************************************************************
 処理概要：	映像パス設定可否判定状態Update（コールバック）
 引数    ：	const std::vector<std::string>	&videopath_names	(i)映像vpath文字列
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
           		VHAL_ERR_PROPERTY_ENTRY	プロパティエントリー不正
*****************************************************************************/
int32_t CVhalPropertyControl::UpdateVpathStatusAvailable(const std::vector<std::string> &videopath_names)
{
	std::vector<std::string> property_names{};

	for (auto itr_videopath = videopath_names.begin(); itr_videopath != videopath_names.end(); ++itr_videopath)
	{
		const std::string& video_path{*itr_videopath};
		bool is_valid{false};
		bool already_path{false};

		/* 前席映像パス サポートチェック */
		is_valid = p_layout_mng_->IsValidVideoFrontPath(video_path, already_path);
		if (true == is_valid)
		{
			VHAL_LOGI("video path front=%s", video_path.c_str());
			property_names.push_back(VHAL_PROP_VPATH_FRONT_STS_AVAILABLE);
		}

		/* 後席映像パス サポートチェック */
		is_valid = p_layout_mng_->IsValidVideoRearPath(video_path, already_path);
		if (true == is_valid)
		{
			VHAL_LOGI("video path rear=%s", video_path.c_str());
			property_names.push_back(VHAL_PROP_VPATH_REAR_STS_AVAILABLE);
		}

		/* カメラ映像パス サポートチェック */
		is_valid = p_layout_mng_->IsValidVideoCameraPath(video_path, already_path);
		if (true == is_valid)
		{
			VHAL_LOGI("video path camera=%s", video_path.c_str());
			property_names.push_back(VHAL_PROP_VPATH_CAMERA_STS_AVAILABLE);
		}

		/* IC映像パス サポートチェック */
		is_valid = p_layout_mng_->IsValidVideoIClusterPath(video_path, already_path);
		if (true == is_valid)
		{
			VHAL_LOGI("video path ic=%s", video_path.c_str());
			property_names.push_back(VHAL_PROP_VPATH_ICLUSTER_STS_AVAILABLE);
		}
	}

	int32_t		result{VHAL_SUCCESS};

	/* 全て未サポートの映像vpath文字列の場合 */
	if (0U == property_names.size())
	{
		VHAL_LOGE("video path none");
		result = VHAL_ERR_PROPERTY_ENTRY;
	}
	else
	{
		/* 重複要素の除去 */
		std::sort(property_names.begin(), property_names.end());
		(void)property_names.erase(std::unique(property_names.begin(), property_names.end()), property_names.end());
		/* log */
		for (auto itr_property = property_names.begin(); itr_property != property_names.end(); ++itr_property)
		{
			const std::string& property_name{*itr_property};
			VHAL_LOGD("update property_name=%s", property_name.c_str());
		}

  		/* 設定可否判定コールバック */
		p_main_->UpdatedProperties(property_names);
	}

	return result;
}

/*****************************************************************************
 処理概要：	ディスプレイデバイス状態Update（コールバック）
 引数    ：	int32_t		screen_id		(i)スクリーンID
         ：	bool		display			(i)表示状態 true:表示可能, false:表示不可
         ：	int32_t		width			(i)ディスプレイ解像度(幅)
         ：	int32_t		height			(i)ディスプレイ解像度(高さ)
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
           		VHAL_ERR_PROPERTY_ENTRY	プロパティエントリー不正
*****************************************************************************/
int32_t CVhalPropertyControl::UpdateSettingStatusDisplay(const int32_t screen_id, const bool display, const int32_t width, const int32_t height)
{
	int32_t ret{VHAL_SUCCESS};

	/* スクリーンIDチェック */
	if (0 > screen_id)
	{
		VHAL_LOGE("error screen_id=%d, display=%d, width=%d, height=%d", screen_id, display, width, height);
		ret = VHAL_ERR_PROPERTY_ENTRY;
	}
	else
	{
		int32_t front_screen_id{-1};
		int32_t rear_screen_id{-1};
		int32_t ic_screen_id{-1};
		int32_t hud_screen_id{-1};
		const int32_t front_ret{p_layout_mng_->GetScreenIdFront(front_screen_id)};
		const int32_t rear_ret{p_layout_mng_->GetScreenIdRear(rear_screen_id)};
		const int32_t ic_ret{p_layout_mng_->GetScreenIdICluster(ic_screen_id)};
		const int32_t hud_ret{p_layout_mng_->GetScreenIdHud(hud_screen_id)};
		std::string update_name{};
		std::string update_name_width{};
		std::string update_name_height{};
		/* 前席スクリーンID */
		if ((VHAL_SUCCESS == front_ret) && (front_screen_id == screen_id))
		{
			VHAL_LOGI("front screen_id=%d, display=%d, resolution=(%dx%d)", screen_id, display, width, height);
			update_name = VHAL_PROP_SETTING_STS_DISP_FRONT;
			update_name_width = VHAL_PROP_SETTING_STS_DISP_FRONT_WIDTH;
			update_name_height = VHAL_PROP_SETTING_STS_DISP_FRONT_HEIGHT;
			/* 画質調整 ディスプレイ出力矩形を設定 */
			p_color_mng_->SetDisplayRectangle(0, 0, width, height);
		}
		/* 後席スクリーンID */
		else if ((VHAL_SUCCESS == rear_ret) && (rear_screen_id == screen_id))
		{
			VHAL_LOGI("rear screen_id=%d, display=%d, resolution=(%dx%d)", screen_id, display, width, height);
			update_name = VHAL_PROP_SETTING_STS_DISP_REAR;
			update_name_width = VHAL_PROP_SETTING_STS_DISP_REAR_WIDTH;
			update_name_height = VHAL_PROP_SETTING_STS_DISP_REAR_HEIGHT;
			/* MOST 後席ディスプレイ出力矩形を設定 */
			p_micon_most_video_info_->SetRearDisplayRectangle(width, height);
		}
		/* Instrument Cluster スクリーンID */
		else if ((VHAL_SUCCESS == ic_ret) && (ic_screen_id == screen_id))
		{
			VHAL_LOGI("iCluster screen_id=%d, display=%d, resolution=(%dx%d)", screen_id, display, width, height);
			update_name = VHAL_PROP_SETTING_STS_DISP_ICLUSTER;
			update_name_width = VHAL_PROP_SETTING_STS_DISP_ICLUSTER_WIDTH;
			update_name_height = VHAL_PROP_SETTING_STS_DISP_ICLUSTER_HEIGHT;
		}
		/* HUD スクリーンID */
		else if ((VHAL_SUCCESS == hud_ret) && (hud_screen_id == screen_id))
		{
			if (nullptr != p_hud_screen_controller_)
			{
				VHAL_LOGI("HUD screen_id=%d, display=%d, resolution=(%dx%d)", screen_id, display, width, height);
			}
			else
			{
				VHAL_LOGE("p_hud_screen_controller_ is null.");
			}
		}
		else
		{
			/* 未サポートスクリーンID */
			VHAL_LOGI("screen_id [%d] is not supported. display=%d, width=%d, height=%d", screen_id, display, width, height);
			ret = VHAL_ERR_PROPERTY_ENTRY;
		}
		
		if ( VHAL_SUCCESS == ret )
		{
			/* ディスプレイ解像度更新 */
			UpdateDisplayResolution(update_name_width, update_name_height, width, height);
			
			/* 前回値との比較 */
			struct VhalPropertyEntry * const p_status{GetPropertyEntry(update_name, VHAL_DATA_TYPE_BOOL, VHAL_ATTR_ANY)};
			if (nullptr == p_status)
			{
				ret = VHAL_ERR_PROPERTY_ENTRY;
			}
			else
			{
				if (p_status->GetBoolCurrent() == display)
				{
					VHAL_LOGI("display(%d) not changed.", display);
				}
				else
				{
					p_status->SetBoolCurrent(display);

 					/* コールバック */
					std::vector<std::string> property_names{};
					property_names.push_back(std::string(update_name));
					p_main_->UpdatedProperties(property_names);
				}
			}
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	映像パス設定切替結果HDMI Update（コールバック）
 引数    ：	const std::vector<std::string>	&videopath_names	(i)映像vpath文字列
         ：	const VideoOutputTarget output_target				(i)映像出力先
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControl::UpdateVpathStatusCurrentHdmi(const std::vector<std::string> &videopath_names, const VideoOutputTarget output_target)
{
	/* 映像パス設定切替結果HDMI更新 */
	for (auto itr_videopath = videopath_names.begin(); itr_videopath != videopath_names.end(); (void)++itr_videopath)
	{
		const std::string& video_path{*itr_videopath};

		if (0 == video_path.compare("HDMI"))
		{
			volatile uint32_t result{VHAL_VPATH_STS_FAILED};
			std::string update_name{};

			/* 前席HDMI */
			if (0U < (static_cast<uint32_t>(VIDEO_OUTPUT_TARGET_FRONT) & static_cast<uint32_t>(output_target)))
			{
				/* 現在HDMI映像パスかチェック */
				std::string video_path_current{};
				const int32_t ret{p_layout_mng_->GetFrontPath(video_path_current)};
				VHAL_LOGD("video path=%s, output_target=0x%x video_path_current(front)=%s", 
					video_path.c_str(), output_target, video_path_current.c_str());
				if ((VHAL_SUCCESS == ret) && (0 == video_path_current.compare("HDMI")))
				{
					update_name = VHAL_PROP_VPATH_FRONT_STS_CURRENT;
					result = VHAL_VPATH_STS_SUCCESS;
				}
			}
			/* 後席HDMI */
			else if (0U < (static_cast<uint32_t>(VIDEO_OUTPUT_TARGET_REAR) & static_cast<uint32_t>(output_target)))
			{
				/* 現在HDMI映像パスかチェック */
				std::string video_path_current{};
				const int32_t ret{p_layout_mng_->GetRearPath(video_path_current)};
				VHAL_LOGD("video path=%s, output_target=0x%x video_path_current(rear)=%s", 
					video_path.c_str(), output_target, video_path_current.c_str());
				if ((VHAL_SUCCESS == ret) && (0 == video_path_current.compare("HDMI")))
				{
					update_name = VHAL_PROP_VPATH_REAR_STS_CURRENT;
					result = VHAL_VPATH_STS_SUCCESS;
				}
			}
			else
			{
				/* 処理なし */
				VHAL_LOGW("unknown video path=%s, output_target=0x%x", 
					video_path.c_str(), output_target);
				update_name = "";
				result = VHAL_VPATH_STS_FAILED;
			}

			if (!update_name.empty())
			{
				VHAL_LOGD("update_name=%s, result=%d", update_name.c_str(), result);
				struct VhalPropertyEntry *p_status{nullptr};
				p_status = GetPropertyEntry(update_name, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);
				if (nullptr != p_status)
				{
					p_status->SetNumCurrent(static_cast<int64_t>(result));
				}

				/* コールバック */
				std::vector<std::string> property_names{};
				property_names.push_back(std::string(update_name));
				p_main_->UpdatedProperties(property_names);
			}
		}
	}
}

/*****************************************************************************
 処理概要：	画面キャプチャ結果Update（コールバック）
 引数    ：	const int32_t result	(i)処理結果
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControl::UpdateScreenShotStatus(const int32_t result)
{
	const std::string update_name{VHAL_PROP_CAP_SCRN_STS_BITMAP};
	VHAL_LOGI("update_name=%s, result=%d", update_name.c_str(), result);

	struct VhalPropertyEntry *p_status{nullptr};
	p_status = GetPropertyEntry(update_name, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);
	if (nullptr != p_status)
	{
		p_status->SetNumCurrent(static_cast<int64_t>(result));
	}

	/* コールバック */
	std::vector<std::string> property_names{};
	property_names.push_back(std::string(update_name));
	p_main_->UpdatedProperties(property_names);
}

/*****************************************************************************
 処理概要：	サーフェスキャプチャ結果Update（コールバック）
 引数    ：	const int32_t result	(i)処理結果
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControl::UpdateSurfaceScreenShotStatus(const int32_t result)
{
	const std::string update_name{VHAL_PROP_CAP_SURF_STS_BITMAP};
	VHAL_LOGI("update_name=%s, result=%d", update_name.c_str(), result);

	struct VhalPropertyEntry *p_status{nullptr};
	p_status = GetPropertyEntry(update_name, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);
	if (nullptr != p_status)
	{
		p_status->SetNumCurrent(static_cast<int64_t>(result));
	}

	/* コールバック */
	std::vector<std::string> property_names{};
	property_names.push_back(std::string(update_name));
	p_main_->UpdatedProperties(property_names);
}

/*****************************************************************************
 処理概要：	HDCP first認証結果Update（コールバック）
 引数    ：	const CVhalHdcpAuthRsltData&	hdcp_result		(i)HDCP認証結果情報
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControl::UpdateHdcpAuthResultRse(const CVhalHdcpAuthRsltData &hdcp_result)
{
	struct VhalPropertyEntry		*p_status{nullptr};
	const std::vector<std::string>	receiver_id_prop{VHAL_PROP_FAUTH_STS_RSE_RCV_ID0, VHAL_PROP_FAUTH_STS_RSE_RCV_ID1, VHAL_PROP_FAUTH_STS_RSE_RCV_ID2};

	/* HDCPデバイス数更新 */
	p_status = GetPropertyEntry(VHAL_PROP_FAUTH_STS_RSE_DEV_CNT, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);
	if (nullptr != p_status)
	{
		const uint32_t	count{hdcp_result.GetDevsCount()};
		if (p_hdcp_auth_mng_->khdcpAuthMaxDevCnt < count)
		{
			/* デバイス数が3より多い時は3に置き換える */
			VHAL_LOGW("Over Device Count. count=%u", count);
			p_status->SetNumCurrent(static_cast<int64_t>(p_hdcp_auth_mng_->khdcpAuthMaxDevCnt));
		}
		else
		{
			p_status->SetNumCurrent(static_cast<int64_t>(count));
		}
	}

	/* HDCPレシーバーID更新 */
	std::vector<uint64_t> receiver_ids{hdcp_result.GetReceiverIds()};
	auto itr_id = receiver_ids.begin();
	for (auto itr_prop = receiver_id_prop.begin(); itr_prop != receiver_id_prop.end(); (void)++itr_prop)
	{
		const std::string& property_name{*itr_prop};
		p_status = GetPropertyEntry(property_name, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);

		if (itr_id != receiver_ids.end())
		{
			if (nullptr != p_status)
			{
				const uint64_t receiver_id{*itr_id};
				p_status->SetNumCurrent(static_cast<int64_t>(receiver_id));
			}
			(void)++itr_id;
		}
		else
		{
			if (nullptr != p_status)
			{
				p_status->SetNumCurrent(0);
			}
		}
	}

	/* HDCP認証結果更新後にコールバック */
	p_status = GetPropertyEntry(VHAL_PROP_FAUTH_STS_RSE_RSLT, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);
	if (nullptr != p_status)
	{
		p_status->SetNumCurrent(static_cast<int64_t>(hdcp_result.GetResult()));

		/* コールバック */
		std::vector<std::string> property_names{};
		property_names.push_back(std::string(VHAL_PROP_FAUTH_STS_RSE_RSLT));
		p_main_->UpdatedProperties(property_names);
	}

	/* HDCP認証結果保存 */
	if (nullptr != p_capture_control_)
	{
		/* C-Disp_HDCP認証キーの取得 */
		CVhalHdcpAuthRsltData	hdcp_cdisp;
		(void)p_hdcp_auth_mng_->GetHdcpAuthKey(hdcpAuthType::HDCP_AUTH_TYPE_CDISP, hdcp_cdisp);
// (予定)		/* HDCP認証の後段機器情報更新 */
//		(void)p_capture_control_->SetHdcpAuthResultRse(hdcp_result, hdcp_cdisp);
	}
}

/*****************************************************************************
 処理概要：	C-Disp HDCP first認証結果Update（コールバック）
 引数    ：	const CVhalHdcpAuthRsltData 	&hdcp_cdisp		(i)C-Disp_HDCP認証キー情報
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControl::UpdateHdcpAuthResultCdisp(const CVhalHdcpAuthRsltData &hdcp_cdisp)
{
	int32_t result{VHAL_ERR};

	if (nullptr != p_hdcp_auth_mng_)
	{
		result = p_hdcp_auth_mng_->UpdateHdcpAuthResult(hdcpAuthType::HDCP_AUTH_TYPE_CDISP, hdcp_cdisp);
	}

	/* C-Disp_HDCP認証キーに変化あり または C-Disp_HDCP認証失敗時の場合はVHAL_SUCCESS */
	if (VHAL_SUCCESS == result)
	{
		/* 認証に成功しているか調査 */
		if ((SUB_TYPE_HDCP_AUTH_NTY_CDISP == hdcp_cdisp.GetSubType()) &&
			(VHAL_HDCP_FIRST_AUTH_STS_SUCCESS == hdcp_cdisp.GetResult()))
		{
			/* QNXへのHDCP認証キー書込み */
			if (nullptr != p_capture_control_)
			{
				/* HDMI接続情報の取得 */
				int32_t		connect{hdmi_control_.kUnknown};
				int32_t 	ret{p_sysdbctrl_->GetValue(VHAL_SYSDB_PATH_CONN_INFO_HDMI, &connect)};
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("SysDB %s Read error. ret=%d", VHAL_SYSDB_PATH_CONN_INFO_HDMI.c_str(), ret);
				}

				/* HDCP認証キー書込み */
				result = SetHdcpAuthKey(connect);
				if (VHAL_SUCCESS == result)
				{
					/* 前席映像同期面のMUTE-OFF */
					VHAL_LOGI("MuteVideoSync mute-off.");
					p_layout_mng_->SetBrinderSync(VhalMuteFrontReason::VHAL_MUTE_FRONT_REASON_HDMI, false);
				}
			}
		}
		else
		{
			/* 認証失敗時 */
			if (hdmi_control_.IsPlaying(CVhalPropertyControlHdmi::CTRL_TARGET_FRONT))
			{
				/* 前席映像同期面のMUTE-ON */
				VHAL_LOGI("MuteVideoSync mute-on. C-Disp: sub_type=%u result=%d",hdcp_cdisp.GetSubType(), hdcp_cdisp.GetResult());
				p_layout_mng_->SetBrinderSync(VhalMuteFrontReason::VHAL_MUTE_FRONT_REASON_HDMI, true);
			}
		}
	}
	else
	{
		/* C-Disp_HDCP認証キーに変化なしまたは、C-Disp_HDCP認証キー異常の場合は無処理 */
	}
}

/*****************************************************************************
 処理概要：	HDCP認証キー書込み
 引数    ：	int32_t hdmi_state		(i)HDMI接続状態SysDB値
 戻り値  ：	処理結果
           		VHAL_SUCCESS以外		異常終了
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::SetHdcpAuthKey(const int32_t hdmi_state)
{
	int32_t result{VHAL_SUCCESS};
	VHAL_LOGV_IN();

	/* HDMI接続中であるか判定 */
	if (DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECT == hdmi_state)
	{
		/* HDMI接続中の場合 */

		/* キー未書込みであるか取得 */
		const bool setflag_hdcp{p_capture_control_->GetHdcpAuthKeyWriteStatus()};
		if (!setflag_hdcp)
		{
			/* HDCP認証キー(C-Disp)の取得 */
			CVhalHdcpAuthRsltData	hdcp_cdisp;
			(void)p_hdcp_auth_mng_->GetHdcpAuthKey(hdcpAuthType::HDCP_AUTH_TYPE_CDISP, hdcp_cdisp);

			/* HDCP認証キー(RSE)の取得 */
			CVhalHdcpAuthRsltData	hdcp_rse;
			(void)p_hdcp_auth_mng_->GetHdcpAuthKey(hdcpAuthType::HDCP_AUTH_TYPE_RSE, hdcp_rse);

			/* 認証キーのどちらかが認証完了であればHDCP認証キー書込み開始 */
			if (((VHAL_HDCP_FIRST_AUTH_STS_SUCCESS == hdcp_cdisp.GetResult()) ||
				 (VHAL_HDCP_FIRST_AUTH_STS_SUCCESS == hdcp_rse.GetResult())))
			{
				result = p_capture_control_->SetHdcpAuthKey(hdcp_cdisp, hdcp_rse);
			}
			else
			{
				/* C-Disp,RSE両方とも認証非完了 */
				VHAL_LOGI("Neither of the HDCP authentication keys has been completed.");
				result = VHAL_ERR;
			}
		}
		else
		{
			/* 既にHDCP認証キー書込み済 */
			VHAL_LOGI("Already written HDCP authentication key.");
			// result = VHAL_SUCCESS;	// 既にHDCP認証キー書込み済の場合は処理成功扱いとする
		}
	}
	else
	{
		/* HDMI接続中以外 */
		VHAL_LOGI("HDMI unconnected. : hdmi_state=[%d].", hdmi_state);
		result = VHAL_ERR;
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	HDCP認証情報クリアUpdate（コールバック）
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControl::UpdateHdcpAuthClear(void)
{
	if (nullptr != p_hdcp_auth_mng_)
	{
		p_hdcp_auth_mng_->ClearHdcpAuthKey();
	}
}

/*****************************************************************************
 処理概要：	カメラ映像パス結果Update（コールバック）
 引数    ：	const int32_t	result	(i)切替結果
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControl::UpdateCameraPathStatus(const int32_t result)
{
	VHAL_LOGV_IN();

	/* カメラ映像パスの切替結果更新後にコールバック */
	struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_VPATH_CAMERA_STS_CURRENT, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
	if (nullptr != p_status)
	{
		p_status->SetNumCurrent(static_cast<int64_t>(result));

		/* コールバック */
		std::vector<std::string> property_names{};
		property_names.push_back(std::string(VHAL_PROP_VPATH_CAMERA_STS_CURRENT));
		p_main_->UpdatedProperties(property_names);
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	MOST RSE映像パス切替結果Update（コールバック）
 引数    ：	const uint32_t	result	(i)切替結果
         :	const enum MostUpdateType update_type	(i)Updateタイプ
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControl::UpdateMostRsePathStatus(const uint32_t result, const enum MostUpdateType update_type)
{
	VHAL_LOGV_IN("result=%d, update_type=%d", result, update_type);

	/* RSE映像パス切替結果 コールバック */
	struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_VPATH_REAR_STS_RSE_NOTIFY, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};
	if (nullptr != p_status)
	{
		p_status->SetNumCurrent(static_cast<int64_t>(result));

		if (MostUpdateType::kNotifyCallbackRse == update_type)
		{
			/* RSE(ls3ctl)コールバック通知トリガーによる上位への結果通知 */
			std::vector<std::string> property_names{};
			property_names.push_back(std::string(VHAL_PROP_VPATH_REAR_STS_RSE_NOTIFY));
			p_main_->UpdatedProperties(property_names);
		}
		else
		{
			/* プロパティ設定トリガーによる上位への結果通知 */
			/* （プロパティ設定はACK応答後、PUBLISH通知するため、このタイミングで直接PUBLISH通知しない） */
			p_status->SetUpdated(true);
		}
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	MOST 後席ディスプレイ全体のMUTE結果Update（コールバック）
 引数    ：	const bool mute	(i)Mute設定
         :	const enum MostUpdateType update_type	(i)Updateタイプ
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControl::UpdateMostMuteRearDispStatus(const bool mute, const enum MostUpdateType update_type)
{
	VHAL_LOGV_IN("mute=%d, update_type=%d", mute, update_type);

	/* 後席ディスプレイ全体のMUTE結果 コールバック */
	struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_MUTE_REAR_STS_DISP, VHAL_DATA_TYPE_BOOL, VHAL_ATTR_ANY)};
	if (nullptr != p_status)
	{
		p_status->SetBoolCurrent(mute);

		if (MostUpdateType::kNotifyCallbackRse == update_type)
		{
			/* RSE(ls3ctl)コールバック通知トリガーによる上位への結果通知 */
			std::vector<std::string> property_names{};
			property_names.push_back(std::string(VHAL_PROP_MUTE_REAR_STS_DISP));
			p_main_->UpdatedProperties(property_names);
		}
		else
		{
			/* プロパティ設定トリガーによる上位への結果通知 */
			/* （プロパティ設定はACK応答後、PUBLISH通知するため、このタイミングで直接PUBLISH通知しない） */
			p_status->SetUpdated(true);
		}
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	ディスプレイ解像度更新
 引数    ：	const std::string &name_width	(i) 幅プロパティ名
            const std::string &name_hieght	(i) 高さプロパティ名
            const int32_t value_width		(i) 幅の値
            const int32_t value_height		(i) 高さの値
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControl::UpdateDisplayResolution(const std::string &name_width, const std::string &name_height, const int32_t value_width, const int32_t value_height)
{
	struct VhalPropertyEntry *p_status{nullptr};
	std::vector<std::string> names{};

	VHAL_LOGV_IN();

	p_status = GetPropertyEntry(name_width, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);
	if (nullptr != p_status)
	{
		p_status->SetNumCurrent(static_cast<int64_t>(value_width));
		names.push_back(name_width);
	}

	p_status = GetPropertyEntry(name_height, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY);
	if (nullptr != p_status)
	{
		p_status->SetNumCurrent(static_cast<int64_t>(value_height));
		names.push_back(name_height);
	}

	if (0U == names.size())
	{
		VHAL_LOGW("UpdateDisplayResolution failed.");
	}
	else
	{
		p_main_->UpdatedProperties(names);
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	サービス起動状態の設定関数
 引数    ：	int32_t state	(i) サービス起動状態値(DAT_CNV_SERVICE_***)
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_ERR			異常終了
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-R-055
*****************************************************************************/
int32_t CVhalPropertyControl::SetServiceState(const int32_t state)
{
	if ((0 > state) || (DAT_CNV_SERVICE_SUSPEND < state))
	{
		VHAL_LOGE("parameter error. state=%d", state);
		return VHAL_ERR_PARAM;
	}

	int32_t result{VHAL_SUCCESS};
	int32_t ret{VHAL_SUCCESS};

	VHAL_LOGI("state=%d", state);

	/* SYSDBへサービス起動状態書込み */
	ret = p_sysdbctrl_->SetValue(VHAL_SYSDB_PATH_SERVICE_STATE, state);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-055",fail_ret)};
//	if(true == fail)
//	{
//		ret = fail_ret;
//	}
//#endif
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("SetValue error. ret=%d", ret);
		result = VHAL_ERR;
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t state_read{100};
//		ret = p_sysdbctrl_->GetValue(VHAL_SYSDB_PATH_SERVICE_STATE, &state_read);
//		VHAL_LOGI("GetValue. state_read=%d", state_read);
//#endif
	}

	return result;
}

/*****************************************************************************
 処理概要：	画質調整テーブルのクリア（デフォルトデータ）
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR				異常終了
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ClearColorStep(void)
{
	int32_t ret{VHAL_SUCCESS};

	ret = p_color_mng_->ClearColorStep();
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("p_color_mng_->ClearColorStep error. ret=%d", ret);
		ret = VHAL_ERR;
	}

	return ret;
}

/*****************************************************************************
 処理概要：	HDCP認証キー情報のクリア（デフォルトデータ）
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR				異常終了
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::ClearHdcpKey(void)
{
	int32_t ret{VHAL_SUCCESS};

	ret = p_micon_comm_control_->ClearHdcpKey();
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("p_micon_comm_control_->ClearHdcpKey error. ret=%d", ret);
		ret = VHAL_ERR;
	}

	return ret;
}

/*****************************************************************************
 処理概要：	出力サイズ有効値チェック
 引数    ：	const int32_t	screen_id	screen_id	(i)スクリーンID
         ：	const std::vector<int32_t> 	rect		(i)入力矩形
 戻り値  ：	処理結果
           		false		設定値無効
           		true		設定値有効
*****************************************************************************/
bool CVhalPropertyControl::IsValidOutputSize(const int32_t screen_id, const std::vector<int32_t> &rect) const
{
	bool result{true};
	int32_t width{0};
	int32_t height{0};
	const int32_t ret{p_layout_mng_->GetScreenSize(screen_id, width, height)};
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("GetScreenSize error. ret=%d", ret);
		result = false;
	}
	else
	{
		if ((0 > rect[0])  || (width <= rect[0])  ||
			(0 > rect[1])  || (height <= rect[1]) ||
			(0 >= rect[2]) || (width < rect[2])   ||
			(0 >= rect[3]) || (height < rect[3]))
		{
			result = false;
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	キャプチャ再初期化設定
 引数    ：	captureInputType input_type	(i)デバイス種別
 戻り値  ：	なし
 フェールセーフNo：	F-VHAL-R-078
                   	F-VHAL-R-079
                   	F-VHAL-R-080
*****************************************************************************/
void CVhalPropertyControl::ResetCapture(const captureInputType input_type)
{
	VHAL_LOGI("capture reset start input_type=%d", input_type);

	if (captureInputType::VHAL_CAPTURE_INPUT_CAMERA == input_type)
	{
		/* 実行状態(カメラパス)を取得する */
		std::string	path{};
		(void)p_layout_mng_->GetCameraPath(path);
		if (!path.empty())
		{
			/* カメラサーフェスMUTE黒画 */
			p_mute_->SetMuteCameraOn();
		}

		/* カメラキャプチャ終了 */
		camera_control_.Finalize();

		/* カメラキャプチャ設定 */
		int32_t ret{SettingCameraCapture()};
		if(VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SettingCameraCapture error. ret=%d", ret);
		}
		
		if (!path.empty())
		{
			ret = camera_control_.SetCameraPath(path);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("SetCameraPath error. ret=%d path=%s", ret, path.c_str());
			}
			/* カメラサーフェスMUTE黒画解除 */
			p_mute_->SetMuteCameraOff();
		}
	}
	else if (captureInputType::VHAL_CAPTURE_INPUT_HDMI == input_type)
	{
		/* HDMIキャプチャ終了 */
		hdmi_control_.Finalize();
		
		/* HDMI再生可否判定 */
		int32_t connect{hdmi_control_.kUnknown};
		int32_t format{hdmi_control_.kUnknown};
		uint32_t	width{0U};
		uint32_t	height{0U};
		const bool available{hdmi_control_.Available(connect, format, width, height)};
		if (true == available)
		{
		/* HDMI再生 */
			const int32_t ret{PlayHdmiCapture(width, height)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("PlayHdmiCapture() error. ret=%d", ret);
		}
		}
		else
		{
			VHAL_LOGE("Invalid HDMI Status. connect=%d, format=%d[width=%d, height=%d]", connect, format, width, height);
		}
	}
	else
	{
		VHAL_LOGW("Capture reset not required.");
	}

	VHAL_LOGI("capture reset end");
}

/*****************************************************************************
 処理概要：	カメラキャプチャ設定(カメラ制御権設定時にキャプチャ設定を行う際に使用)
 引数    ：	const bool movie_check	(i)動画再生開始中の状態確認有無
           	const bool timer_start	(i)タイマ起動の有無
           	bool& is_capture		(o)false:キャプチャ未設定, true:キャプチャ設定済
 戻り値  ：	処理結果
           		VHAL_ERR			異常処理(キャプチャ設定失敗)
           		VHAL_SUCCESS		正常処理
*****************************************************************************/
int32_t CVhalPropertyControl::OpenCameraCapture(const bool movie_check, const bool timer_start, bool& is_capture)
{
	int32_t result{VHAL_SUCCESS};

	/* wait中解除もれがないようにキャプチャ設定SettingtCameraCapture()にて
	   キャプチャ設定が正常処理された際も解除する */
	/* キャプチャ設定wait中解除 */
	camera_control_.SetWaitingCapture(false);

	/* カメラ種別を取得 */
	int64_t cameraType{VHAL_CONNECTED_CAMERA_NONE};
	int32_t ret{GetValueNumber(VHAL_PROP_SETTING_STS_CNCT_CAMERA, cameraType)};
	if ((VHAL_SUCCESS != ret) || (VHAL_CONNECTED_CAMERA_NONE >= cameraType))
	{
		VHAL_LOGE("Camera type[%ld] error. ret=%d", cameraType, ret);
		result = VHAL_ERR_PARAM;
		is_capture = false;
	}
	else
	{
		is_capture = camera_control_.IsCameraCapture();
		/* キャプチャ未設定の場合 */
		if (false == is_capture)
		{
			VHAL_LOGD("Not OpenCapture[Input_type:%d]", captureInputType::VHAL_CAPTURE_INPUT_CAMERA);

			if (true == movie_check)
			{
				VHAL_LOGD("Check Movie Status");

				struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_MOVIE_FRONT_STS_RESULT, VHAL_DATA_TYPE_NUM, VHAL_ATTR_ANY)};	
				if (nullptr != p_status)
				{
					const int32_t movie_status{p_movie_control_->GetFrontMovieStatus()};
					VHAL_LOGI("Movie Status[%ld],internal[%d]", p_status->GetNumCurrent(), movie_status);

					if (p_status->GetNumCurrent() != movie_status)
					{
						/*  内部ステータスのみ動画再生中のタイミング(動画再生開始中) */
						if (VHAL_MOVIE_STS_PLAYING == movie_status)
						{
							if (true == timer_start)
							{
								/* 900ms後にカメラキャプチャ設定を実施する */
								constexpr uint32_t kWaitTime{900U};		/* 待機時間(ms) 900ms */
								constexpr uint32_t kRetryCount{1U};		/* 繰り返し回数 1回 */
								ret = p_capture_control_->OpenCaptureDelayProcess(captureInputType::VHAL_CAPTURE_INPUT_CAMERA, kWaitTime, kRetryCount);
								if(VHAL_SUCCESS != ret)
								{
									VHAL_LOGE("OpenCaptureDelayProcess error. ret=%d", ret);
									/* タイマー起動に失敗したら即カメラキャプチャ設定を試みる */
								}
								else
								{
									VHAL_LOGD("Waiting OpenCapture[Input_type:%d]", captureInputType::VHAL_CAPTURE_INPUT_CAMERA);
									/* キャプチャ設定wait中 */
									camera_control_.SetWaitingCapture(true);
								}
							}
							else
							{
								VHAL_LOGD("Waiting OpenCapture[Input_type:%d]", captureInputType::VHAL_CAPTURE_INPUT_CAMERA);
								/* タイマモジュール内で再度タイマを起動するため、ここではタイマ起動は行わない */
								/* キャプチャ設定wait中 */
								camera_control_.SetWaitingCapture(true);
							}
						}
					}
				}
				else
				{
					VHAL_LOGE("GetPropertyEntry[%s] error.", VHAL_PROP_MOVIE_FRONT_STS_RESULT.c_str());
					/* プロパティ値取得に失敗したら即カメラキャプチャ設定を試みる */
				}
			}

			const bool is_capture_waiting{camera_control_.GetWaitingCapture()};
			/* キャプチャ設定Wait状態でなければキャプチャ設定を実施 */
			if (false == is_capture_waiting)
			{
				ret = SettingCameraCapture();
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("SettingCameraCapture[Input_type:%d] error. ret=%d", captureInputType::VHAL_CAPTURE_INPUT_CAMERA, ret);
					result = ret;
				}
				else
				{
					is_capture = true;
				}
			}
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	シーン別電源状態遷移時処理 24MM互換用
 引数    ：	const ScenePowerState power_state	(i)電源状態
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControl::ChangePowerState(const ScenePowerState power_state)
{
	if (nullptr == p_mute_)
	{
		VHAL_LOGW("parameter error. p_mute_=%p", p_mute_.get());
	}
	else
	{
		if (ScenePowerState::VHAL_PWR_STS_IN_VEHICLE == power_state)
		{
			const bool mute_current{p_mute_->GetMuteFrontDisp()};
			if (false == mute_current)
			{
				/* 前席ディスプレイMUTE ON設定 */
				p_mute_->SetMuteFrontDisp(true);
			}
		}
	
		if ((ScenePowerState::VHAL_PWR_STS_IN_VEHICLE == power_state)
		 || (ScenePowerState::VHAL_PWR_STS_BG_BOOT    == power_state))
		{
			/* 後席ディスプレイMUTE ON設定 */
			p_mute_->SetMuteRearDisp(true);
		}
		else if (ScenePowerState::VHAL_PWR_STS_NML_BOOT == power_state)
		{
			/* 後席ディスプレイMUTE OFF設定 */
			p_mute_->SetMuteRearDisp(false);
		}
		else
		{
			VHAL_LOGE("Abnormal power state %d", power_state);
		}
	}
}

/*****************************************************************************
 処理概要：	車両電源状態遷移時処理
 引数    ：	const VehiclePowerState power_state	(i)電源状態
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControl::ChangeVehiclePowerState(const VehiclePowerState power_state)
{
	/* カメラ電源状態処理 */
	camera_control_.ActionCameraPowerState(power_state);
}

/*****************************************************************************
 処理概要：	車両電源状態変換処理
 引数    ：	const int32_t special_state			(i)特殊ステート
         ：	const int32_t basic_state			(i)基本ステート
         ：	const int32_t appearance_state		(i)見た目
         ：	VehiclePowerState power_state		(o)電源状態
         ：		※変換失敗時はpower_stateにVehiclePowerState::VHAL_VPWR_STS_MAXを格納する。
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControl::ConvertVehiclePowerState(const int32_t special_state, const int32_t basic_state, const int32_t appearance_state, VehiclePowerState& power_state) const
{
	/* 特殊ステート変換表 */
	const std::unordered_map<int32_t, VehiclePowerState>	special_cnv_table {
		{ DAT_CNV_VEHICLE_PWR_SPE_STS_NOT_APPLICABL	, VehiclePowerState::VHAL_VPWR_STS_NEXT },		/* 未設定 */
		{ DAT_CNV_VEHICLE_PWR_SPE_STS_TRANSPORTING	, VehiclePowerState::VHAL_VPWR_STS_NEXT },		/* 輸送中一部OFF*/
		{ DAT_CNV_VEHICLE_PWR_SPE_STS_OTA			, VehiclePowerState::VHAL_VPWR_STS_VISUAL_ON },	/* OTA */
		{ DAT_CNV_VEHICLE_PWR_SPE_STS_WIRED_REPRO	, VehiclePowerState::VHAL_VPWR_STS_NEXT },		/* 有線リプロ */
		{ DAT_CNV_VEHICLE_PWR_SPE_STS_DIMINISH		, VehiclePowerState::VHAL_VPWR_STS_LIMP_MODE },	/* 縮退走行 */
		{ DAT_CNV_VEHICLE_PWR_SPE_STS_ALL_OFF		, VehiclePowerState::VHAL_VPWR_STS_MAX },		/* 電源ALL OFF */
		{ DAT_CNV_VEHICLE_PWR_SPE_STS_UNDEFINED		, VehiclePowerState::VHAL_VPWR_STS_MAX },		/* 未確定 */
	};

	/* 基本ステート変換表 */
	const std::unordered_map<int32_t, VehiclePowerState>	basic_cnv_table {
		{ DAT_CNV_VEHICLE_PWR_BAS_STS_CHECKING							, VehiclePowerState::VHAL_VPWR_STS_MAX },		/* 判定中 */
		{ DAT_CNV_VEHICLE_PWR_BAS_STS_PARKING							, VehiclePowerState::VHAL_VPWR_STS_BG_BOOT },	/* 駐車中 */
		{ DAT_CNV_VEHICLE_PWR_BAS_STS_RIDING							, VehiclePowerState::VHAL_VPWR_STS_NEXT },		/* 乗車中 */
		{ DAT_CNV_VEHICLE_PWR_BAS_STS_PWON_NORMAL						, VehiclePowerState::VHAL_VPWR_STS_VISUAL_ON },	/* PowerON通常 */
		{ DAT_CNV_VEHICLE_PWR_BAS_STS_PWON_EMERGENCY					, VehiclePowerState::VHAL_VPWR_STS_VISUAL_ON },	/* PowerON緊急停止 */
		{ DAT_CNV_VEHICLE_PWR_BAS_STS_PARKING_HIGH_VOLTAGE				, VehiclePowerState::VHAL_VPWR_STS_BG_BOOT },	/* 駐車中高圧起動 */
		{ DAT_CNV_VEHICLE_PWR_BAS_STS_PARKING_HIGH_VOLTAGE_AND_HEAT_COND, VehiclePowerState::VHAL_VPWR_STS_BG_BOOT },	/* 駐車中高圧・温調起動 */
		{ DAT_CNV_VEHICLE_PWR_BAS_STS_UNDEFINED							, VehiclePowerState::VHAL_VPWR_STS_MAX },		/* 未確定 */
	};

	/* 見た目変換表 */
	const std::unordered_map<int32_t, VehiclePowerState>	appearance_cnv_table {
		{ DAT_CNV_VEHICLE_PWR_APP_STS_APPEARANCE_ON	, VehiclePowerState::VHAL_VPWR_STS_VISUAL_ON },		/* 見た目ON状態 */
		{ DAT_CNV_VEHICLE_PWR_APP_STS_APPEARANCE_OFF, VehiclePowerState::VHAL_VPWR_STS_VISUAL_OFF },	/* 見た目OFF状態 */
		{ DAT_CNV_VEHICLE_PWR_APP_STS_UNDEFINED		, VehiclePowerState::VHAL_VPWR_STS_MAX },			/* 未確定 */
	};

	/* 変換テーブル */
	const std::vector<std::pair<int32_t, const std::unordered_map<int32_t, VehiclePowerState>&>>	vehicle_cnv_table {
		{ special_state		, special_cnv_table },		/* 特殊ステート変換表 */
		{ basic_state		, basic_cnv_table },		/* 基本ステート変換表 */
		{ appearance_state	, appearance_cnv_table },	/* 見た目変換表 */
	};

	/* 各変換テーブルを用いて電源状態を確定 */
	for (auto& table : vehicle_cnv_table)
	{
		power_state = VehiclePowerState::VHAL_VPWR_STS_MAX;
		auto itr = table.second.find(table.first);
		if (itr != table.second.end())
		{
			power_state = itr->second;
		}
		if (VehiclePowerState::VHAL_VPWR_STS_NEXT != power_state)
		{
			break;
		}
	}
}

/*****************************************************************************
 処理概要：	Instrument Clusterディスプレイデバイス状態更新(コールバック)
 引数    ：	const bool result				(i) 更新結果
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControl::UpdateIClusterDisplay(const bool result)
{
	/* 通常起動時にコールバック */
	struct VhalPropertyEntry * const p_status{GetPropertyEntry(VHAL_PROP_SETTING_STS_DISP_ICLUSTER, VHAL_DATA_TYPE_BOOL, VHAL_ATTR_ANY)};
	if (nullptr != p_status)
	{
		p_status->SetBoolCurrent(result);
		/* コールバック */
		std::vector<std::string> property_names{};
		property_names.push_back(std::string(VHAL_PROP_SETTING_STS_DISP_ICLUSTER));
		p_main_->UpdatedProperties(property_names);
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	STR受付処理(PFMスレッド：サスペンド)
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_****	エラー
           		VHAL_SUCCESS	正常終了
 備考    ：	StrStateEvent()関数の★の箇所で相互実行する
           	本関数はPFMスレッドにて実施
*****************************************************************************/
int32_t CVhalPropertyControl::StrStatePfmSuspend(void)
{
	int32_t	result{VHAL_SUCCESS};
	VHAL_LOGV_IN();

	CVhalStrManager&	str_mng{CVhalStrManager::GetInstance()};

	/* SysDBのVM STR移行ステータスが「サスペンド処理完了」になるまで待つ */
	/* ★１の対応先はCVhalSysdbControl::ActionVmStrChange()関数 */
	VHAL_LOGD("Wait SysDB-change.");
	const int32_t ret_wait_sapp{str_mng.Wait(StrWaitState::SuspendApps)};
	VHAL_LOGI("Wait(SuspendApps) ret=%d", ret_wait_sapp);

	/* VM STR移行ステータスが処理成功の場合 */
	if (VHAL_SUCCESS == ret_wait_sapp)
	{
		if (nullptr != p_event_route_)
		{
			/* STRイベントの生成と送信 */
			std::unique_ptr<CVhalEventItemStrState> p_str_state{std::make_unique<CVhalEventItemStrState>()};
			p_str_state->SetName("STR state event");
			p_str_state->SetData(this);
			const int32_t ret{p_event_route_->WriteEvent(p_str_state.get())};
			if (VHAL_SUCCESS == ret)
			{
				/* 所有権を放棄。実体解放はCVhalEventRoute::ExecEvent()にて実施 */
				(void)p_str_state.release();
				/* VideoHALの機能停止待ち　★２ */
				const int32_t ret_wait_svhal{str_mng.Wait(StrWaitState::SuspendVhal)};
				if (VHAL_SUCCESS != ret_wait_svhal)
				{
					/* mainスレッド側で処理を済ませる必要があるため、リカバリ制御は不要 */
					/* 警告ログ出力のみとする(本関数の戻り値は正常) */
					VHAL_LOGW("Wait(SuspendVhal) failed. ret=%d", ret_wait_svhal);
				}
			}
			else
			{
				/* WriteEvent失敗時は所有権を放棄せずスコープ終了時に実体解放する */
				VHAL_LOGEW("WriteEvent failed. ret=%d", ret);
				result = VHAL_ERR;
			}
		}
		else
		{
			VHAL_LOGE("p_event_route_ is null.");
			result = VHAL_ERR;
		}
		VHAL_LOGD("PFM_STR_STS_SUSPEND_START finished.");
	}
	/* VM STR移行ステータスが処理成功以外(=処理失敗)の場合 */
	else
	{
		/* 処理を中断 */
		VHAL_LOGE("Detect DAT_CNV_VM_STR_CHANGE_FAILURE. not suspend.");
		result = VHAL_ERR;
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	STR受付処理(PFMスレッド：レジューム)
 引数    ：	なし
 戻り値  ：	なし
 備考    ：	StrStateEvent()関数の★の箇所で相互実行する
           	本関数はPFMスレッドにて実施
*****************************************************************************/
void CVhalPropertyControl::StrStatePfmResume(void)
{
	VHAL_LOGV_IN();

	CVhalStrManager&	str_mng{CVhalStrManager::GetInstance()};

	/* Suspend状態からの復帰　★３ */
	str_mng.NotifyOne(StrWaitState::ResumeSystem);

	/* 復帰待ち　★４ */
	const int32_t ret{str_mng.Wait(StrWaitState::ResumeVhal)};
	if (VHAL_SUCCESS != ret)
	{
		/* mainスレッド側で処理を済ませる必要があるため、リカバリ制御は不要 */
		/* 警告ログ出力のみとする */
		VHAL_LOGW("Wait(ResumeVhal) failed. ret=%d", ret);
	}
	VHAL_LOGD("PFM_STR_STS_RESUME_START finished.");

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	STRイベント処理
 引数    ：	なし
 戻り値  ：	なし
 フェールセーフNo  ：	F-VHAL-N-338
           		   ：	F-VHAL-N-339
 備考    ：	StrStatePfmSuspend()関数およびStrStatePfmResume()の★の箇所で相互実行する
           	本関数はVideoHALスレッドにて実施
*****************************************************************************/
void CVhalPropertyControl::StrStateEvent(void)
{
	VHAL_LOGV_IN();

	CVhalStrManager&	str_mng{CVhalStrManager::GetInstance()};

	/* VideoHALはサスペンド状態に設定 */
	CVhalStrManager::SetSuspend(true);			/* VideoHALサスペンド状態とする */

	/* STRサスペンド処理実施 */
	StrSuspend();

//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret_sus{0};		/* 0以外を設定すれば応答なし */
//	CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-338",fail_ret_sus);
//	if (0 == fail_ret_sus)
//#endif
	/* pfm.str_state(Suspend開始)処理の待ち状態を完了　★２ */
	str_mng.NotifyOne(StrWaitState::SuspendVhal);

	/* サスペンド処理中に溜まったイベントの破棄 */
	if (nullptr != p_main_)
	{
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM		/* イベント滞留試験用 */
//		for (int32_t i=0; i<fail_ret_sus; ++i)
//		{
//			p_main_->DropEvents();
//			sif_mdelay(1000);
//		}
//#endif
		p_main_->DropEvents();
	}

	/* 基本的には、ここのwait中にSIGSTOPになる見込み */
	/* 復帰は、SIGCONT後のpfm.str_stateのResume開始通知時の見込み　★３ */
	VHAL_LOGI("Suspend VideoHAL...");
	(void)str_mng.Wait(StrWaitState::ResumeSystem);
	VHAL_LOGI("Resume VideoHAL...");

	/* SIGCONTからWait解除までに溜まったイベントの破棄 */
	if (nullptr != p_main_)
	{
		p_main_->DropEvents();
	}

	/* VideoHALはサスペンド処理を解除 */
	CVhalStrManager::SetSuspend(false);			/* VideoHAL非サスペンド状態とする */

	/* STRレジューム処理実施 */
	const int32_t ret{StrResume()};
	if (VHAL_SUCCESS != ret)
	{
		/* レジュームの一部機能失敗のログ */
		VHAL_LOGW("StrResume failed. ret=%d", ret);
	}

//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret_rsm{0};		/* 0以外を設定すればタイムアウト */
//	CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-339",fail_ret_rsm);
//	if (0 == fail_ret_rsm)
//#endif
	/* pfm.str_state(Resume開始)処理の待ち状態を完了　★４ */
	str_mng.NotifyOne(StrWaitState::ResumeVhal);

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	STRサスペンド処理
 引数    ：	なし
 戻り値  ：	なし
 備考    ：	本関数はVideoHALスレッドにて実施
*****************************************************************************/
void CVhalPropertyControl::StrSuspend(void)
{
	int32_t ret{VHAL_SUCCESS};

	/* VideoHALの一部機能の停止 */
	VHAL_LOGI("VideoHAL Finalize(Suspend)");

	/* 本処理は終了系処理のため、基本的に戻り値は確認しない */

	/* 動画停止 */
	if (nullptr != p_movie_control_)
	{
		/* 再生中の場合のみ実施する */
		if (VHAL_MOVIE_STS_PLAYING == p_movie_control_->GetFrontMovieStatus())
		{
			/* 動画再生中止 */
			ret = p_movie_control_->CancelFrontMovie();
			if (VHAL_SUCCESS != ret)
			{
				/* エラーログは対象関数内で出力。ここでは検証用ログ出力 */
				VHAL_LOGV("CancelFrontMovie failed. ret=%d", ret);
			}
		}

		/* クリア状態以外の場合のみ実施する */
		if (VHAL_MOVIE_STS_NONE != p_movie_control_->GetFrontMovieStatus())
		{
			/* 動画再生パラメータクリア */
			ret = p_movie_control_->ClearFrontMovie();
			if (VHAL_SUCCESS != ret)
			{
				/* エラーログは対象関数内で出力。ここでは検証用ログ出力 */
				VHAL_LOGV("ClearFrontMovie failed. ret=%d", ret);
			}
		}
	}

	/* スクリーンショットキャンセル */
	if (nullptr != p_screen_shot_micon_)
	{
		p_screen_shot_micon_->CancelScreenShotRequest();
	}

	/* MiconCommコントロール停止 */
	if (nullptr != p_micon_comm_control_)
	{
		/* 以下の処理の停止を行う */
		/* ・OS間通信 */
		/* ・LS3Ctl */
		/* ・画質モード通知の周期送信 */
		/* ・カメラ映像モード通知の周期送信 */
		p_micon_comm_control_->FinalizeSuspend();
	}

	/* 画面MUTE-ON */
	if (nullptr != p_layout_mng_)
	{
		/* 前席 */
		ret = p_layout_mng_->SetBlinderEnable(VhalBlinderType::VHAL_BLINDER_TYPE_FRONT_DISPLAY, true);
		if (VHAL_SUCCESS != ret)
		{
			/* エラーログは対象関数内で出力。ここでは検証用ログ出力 */
			VHAL_LOGV("SetBlinderEnable(FRONT-DISP) failed. ret=%d", ret);
		}

		/* 後席ディスプレイデバイス状態：vhal.setting.status.display.rear */
		const VhalPropertyEntry* const p_status_rear{GetPropertyEntry(VHAL_PROP_SETTING_STS_DISP_REAR, VHAL_DATA_TYPE_BOOL, VHAL_ATTR_ANY)};
		if (nullptr != p_status_rear)
		{
			const bool is_rear_enabled{p_status_rear->GetBoolCurrent()};
			if (is_rear_enabled)
			{
				ret = p_layout_mng_->SetBlinderEnable(VhalBlinderType::VHAL_BLINDER_TYPE_REAR_DISPLAY, true);
				if (VHAL_SUCCESS != ret)
				{
					/* エラーログは対象関数内で出力。ここでは検証用ログ出力 */
					VHAL_LOGV("SetBlinderEnable(REAR-DISP) failed. ret=%d", ret);
				}
			}
		}

		/* Instrument Clusterディスプレイデバイス状態：vhal.setting.status.display.instrumentcluster */
		const VhalPropertyEntry* const p_status_ic{GetPropertyEntry(VHAL_PROP_SETTING_STS_DISP_ICLUSTER, VHAL_DATA_TYPE_BOOL, VHAL_ATTR_ANY)};
		if (nullptr != p_status_ic)
		{
			const bool is_ic_enabled{p_status_ic->GetBoolCurrent()};
			if (is_ic_enabled)
			{
				ret = p_layout_mng_->SetBlinderEnable(VhalBlinderType::VHAL_BLINDER_TYPE_IC_DISPLAY, true);
				if (VHAL_SUCCESS != ret)
				{
					/* エラーログは対象関数内で出力。ここでは検証用ログ出力 */
					VHAL_LOGV("SetBlinderEnable(IC-DISP) failed. ret=%d", ret);
				}
			}
		}

		/* HUDディスプレイデバイス状態 */
		if (nullptr != p_hud_screen_controller_)
		{
			const bool is_hud_enabled{p_hud_screen_controller_->IsHudScreenEnabled()};
			if (is_hud_enabled)
			{
				ret = p_layout_mng_->SetBlinderEnable(VhalBlinderType::VHAL_BLINDER_TYPE_HUD_DISPLAY, true);
				if (VHAL_SUCCESS != ret)
				{
					/* エラーログは対象関数内で出力。ここでは検証用ログ出力 */
					VHAL_LOGV("SetBlinderEnable(HUD-DISP) failed. ret=%d", ret);
				}
			}
		}
	}

	/* カメラ映像パスクリア */
	ret = StrSuspendPathClearCamera();
	if (VHAL_SUCCESS != ret)
	{
		/* エラーログは対象関数内で出力。ここでは検証用ログ出力 */
		VHAL_LOGV("StrSuspendPathClearCamera failed. ret=%d", ret);
	}

	/* 前席映像パスクリア */
	ret = StrSuspendPathClearFront();
	if (VHAL_SUCCESS != ret)
	{
		/* エラーログは対象関数内で出力。ここでは検証用ログ出力 */
		VHAL_LOGV("StrSuspendPathClearFront failed. ret=%d", ret);
	}

	/* 後席映像パスクリア */
	ret = StrSuspendPathClearRear();
	if (VHAL_SUCCESS != ret)
	{
		/* エラーログは対象関数内で出力。ここでは検証用ログ出力 */
		VHAL_LOGV("StrSuspendPathClearRear failed. ret=%d", ret);
	}

	/* Instrument Cluster映像パスクリア */
	ret = StrSuspendPathClearICluster();
	if (VHAL_SUCCESS != ret)
	{
		/* エラーログは対象関数内で出力。ここでは検証用ログ出力 */
		VHAL_LOGV("StrSuspendPathClearICluster failed. ret=%d", ret);
	}

	/* カメラ種別クリア */
	constexpr int64_t setting_ctl_camera_none{static_cast<int64_t>(VHAL_CONNECTED_CAMERA_NONE)};
	ret = SetValueNumber(VHAL_PROP_SETTING_CTL_CNCT_CAMERA, setting_ctl_camera_none);
	if (VHAL_SUCCESS != ret)
	{
		/* エラーログは対象関数内で出力。ここでは検証用ログ出力 */
		VHAL_LOGV("SetValueNumber(SETTING_CTL_CNCT_CAMERA) failed. ret=%d", ret);
	}

	/* QCarCam終了 */
	if (nullptr != p_capture_control_)
	{
		p_capture_control_->FinalizeQcarcam();
	}

	/* HDMI接続初期化 */
	UpdateFilePropertyHdmi(DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECTING);

	/* 内部リソース再初期化 */
	ReInit();

	/* VideoHALプロパティ初期化 */
	const std::unordered_map<std::string, int64_t>		tbl_num{
		{ VHAL_PROP_SETTING_CTL_DAY_NIGHT	, static_cast<int64_t>(VHAL_SETTING_NIGHT) },
		{ VHAL_PROP_SETTING_STS_DAY_NIGHT	, static_cast<int64_t>(VHAL_SETTING_NIGHT) },
		{ VHAL_PROP_SETTING_CTL_THEME_COLOR	, static_cast<int64_t>(VHAL_THEME_COLOR_AUTO_DARK) },
		{ VHAL_PROP_SETTING_STS_THEME_COLOR	, static_cast<int64_t>(VHAL_THEME_COLOR_AUTO_DARK) },
		{ VHAL_PROP_VSRC_FRONT_CTL_ID		, static_cast<int64_t>(VHAL_VSRC_ID_OTHER) },
		{ VHAL_PROP_VSRC_FRONT_STS_ID		, static_cast<int64_t>(VHAL_VSRC_ID_OTHER) },
		{ VHAL_PROP_SETTING_CTL_CNCT_RSE	, static_cast<int64_t>(VHAL_CONNECTED_RSE_INVALID) },
		{ VHAL_PROP_SETTING_STS_CNCT_RSE	, static_cast<int64_t>(VHAL_CONNECTED_RSE_INVALID) },
		{ VHAL_PROP_SETTING_CTL_CNCT_CAMERA	, static_cast<int64_t>(VHAL_CONNECTED_CAMERA_INVALID) },
		{ VHAL_PROP_SETTING_STS_CNCT_CAMERA	, static_cast<int64_t>(VHAL_CONNECTED_CAMERA_INVALID) },
		{ VHAL_PROP_FAUTH_STS_RSE_RSLT		, static_cast<int64_t>(VHAL_HDCP_FIRST_AUTH_STS_NONE) },
	};
	const std::unordered_map<std::string, bool>			tbl_bool{
		{ VHAL_PROP_MUTE_FRONT_CTL_DISP		, true },
		{ VHAL_PROP_MUTE_FRONT_STS_DISP		, true },
		{ VHAL_PROP_MUTE_FRONT_CTL_BACK		, true },
		{ VHAL_PROP_MUTE_FRONT_STS_BACK		, true },
	};
	const std::unordered_map<std::string, std::string>	tbl_str{};		/* 文字列が対象の固定設定なし */
	const std::vector<std::string>						status_quo{		/* 未初期化プロパティ */
		{ VHAL_PROP_SETTING_STS_DISP_FRONT },
		{ VHAL_PROP_SETTING_STS_DISP_REAR },
		{ VHAL_PROP_SETTING_STS_DISP_ICLUSTER },
		{ VHAL_PROP_SETTING_STS_DISP_FRONT_WIDTH },
		{ VHAL_PROP_SETTING_STS_DISP_FRONT_HEIGHT },
		{ VHAL_PROP_SETTING_STS_DISP_REAR_WIDTH },
		{ VHAL_PROP_SETTING_STS_DISP_REAR_HEIGHT },
		{ VHAL_PROP_SETTING_STS_DISP_ICLUSTER_WIDTH },
		{ VHAL_PROP_SETTING_STS_DISP_ICLUSTER_HEIGHT },
	};
	InitializeProperties(tbl_num, tbl_bool, tbl_str, status_quo);

	/* 画質調整から、プロパティの画質調整STEP値を設定 */
	ret = UpdateStatusColorStep(COLOR_STEP_TYPE_ALL, false);
	if (VHAL_SUCCESS != ret)
	{
		/* エラーログは対象関数内で出力。ここでは検証用ログ出力 */
		VHAL_LOGV("UpdateStatusColorStep failed. ret=%d", ret);
	}

	/* VideoHAL状態のサスペンド登録 */
	ret = SetServiceState(DAT_CNV_SERVICE_SUSPEND);
	if (VHAL_SUCCESS != ret)
	{
		/* 状態通知の失敗は致命ではないため、サスペンド処理は継続（ログのみ） */
		VHAL_LOGW("SetServiceState failed: %d", ret);
	}
}

/*****************************************************************************
 処理概要：	STRレジューム処理
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_****	エラー
           		VHAL_SUCCESS	正常終了
 備考    ：	本関数はVideoHALスレッドにて実施
*****************************************************************************/
int32_t CVhalPropertyControl::StrResume(void)
{
	int32_t	ret{VHAL_SUCCESS};

	/* VideoHALの一部機能の復帰 */
	VHAL_LOGI("VideoHAL Initialize(Resume)");

	/* MiconComm再開処理(CCMは再オープンしない) */
	if (nullptr != p_micon_comm_control_)
	{
		/* 再開(OS間通信, MOST) */
		ret = p_micon_comm_control_->InitializeResume(true);
		if (VHAL_SUCCESS != ret)
		{
			/* エラーログは対象関数内で出力。ここでは検証用ログ出力 */
			VHAL_LOGV("InitializeResume failed. ret=%d", ret);

			/* p_micon_comm_control_->Initialize()と同様、NG時も処理継続 */
		}
		else
		{
			/* カメラ種別判別要求送信 */
			ret = SendCameraType();
			if (VHAL_SUCCESS != ret)
			{
				/* 下位層(CVhalMiconCommMiscControl)での失敗 */
				/* 他VideoHAL機能は動作可能なため、処理を継続する(NGリターンしない) */
				VHAL_LOGE("SendCameraType() error. ret=%d", ret);
			}

			/* 画質モード：周期送信の再開 */
			if (nullptr != p_color_mng_)
			{
				ret = p_color_mng_->ApplyAdjustment(false);
				if (VHAL_SUCCESS != ret)
				{
					/* 画質モード通知送信に失敗しているが、 */
					/* 他VideoHAL機能は動作可能なため、NGリターンしない（プロセス終了させない）。 */
					VHAL_LOGW("ApplyAdjustment error. ret=%d", ret);
				}
			}

			/* カメラ映像モード：周期送信の再開 */
			ret = SendIntervalCameraMode(false);
			if (VHAL_SUCCESS != ret)
			{
				/* エラーログは対象関数内で出力 */
				return ret;
			}

		}
	}

	/* 画面MUTE-OFF */
	if (nullptr != p_layout_mng_)
	{
		/* 前席のMUTE-OFFは実施しない(MUTE-ONの状態から始める) */

		/* 後席ディスプレイデバイス状態：vhal.setting.status.display.rear */
		const VhalPropertyEntry* const p_status_rear{GetPropertyEntry(VHAL_PROP_SETTING_STS_DISP_REAR, VHAL_DATA_TYPE_BOOL, VHAL_ATTR_ANY)};
		if (nullptr != p_status_rear)
		{
			const bool is_rear_enabled{p_status_rear->GetBoolCurrent()};
			if (is_rear_enabled)
			{
				ret = p_layout_mng_->SetBlinderEnable(VhalBlinderType::VHAL_BLINDER_TYPE_REAR_DISPLAY, false);
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGW("SetBlinderEnable(REAR-DISP) failed ret=%d", ret);
					return ret;
				}
			}
		}

		/* Instrument Clusterディスプレイデバイス状態：vhal.setting.status.display.instrumentcluster */
		const VhalPropertyEntry* const p_status_ic{GetPropertyEntry(VHAL_PROP_SETTING_STS_DISP_ICLUSTER, VHAL_DATA_TYPE_BOOL, VHAL_ATTR_ANY)};
		if (nullptr != p_status_ic)
		{
			const bool is_ic_enabled{p_status_ic->GetBoolCurrent()};
			if (is_ic_enabled)
			{
				ret = p_layout_mng_->SetBlinderEnable(VhalBlinderType::VHAL_BLINDER_TYPE_IC_DISPLAY, false);
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGW("SetBlinderEnable(IC-DISP) failed ret=%d", ret);
					return ret;
				}
			}
		}
	}

	/* VideoHAL状態の復帰登録 */
	ret = SetServiceState(DAT_CNV_SERVICE_RUNNING);
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGW("SetServiceState failed: %d", ret);
	}
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	STRサスペンド処理(カメラ映像パスクリア)
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_****	エラー
           		VHAL_SUCCESS	正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::StrSuspendPathClearCamera(void)
{
	std::string video_path{};			/* 映像パス */
	int32_t	ret{VHAL_SUCCESS};			/* 戻り値 */

	/* NULL調査 */
	if ((nullptr == p_layout_mng_) || (nullptr == p_mute_) || (nullptr == p_color_mng_))
	{
		VHAL_LOGW("parameter error. layout[%p] mute[%p] color[%p]", p_layout_mng_.get(), p_mute_.get(), p_color_mng_.get());
		return VHAL_ERR;
	}

	/* カメラ映像パス取得 */
	ret = p_layout_mng_->GetCameraPath(video_path);
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("GetCameraPath error. ret=%d", ret);
		return ret;
	}

	/* 変更有無チェック */
	if (!video_path.empty())
	{
		/* カメラ映像MUTE-ON */
		p_mute_->SetMuteCameraPathOn();

		/* カメラ映像キャプチャ停止 */
		(void)camera_control_.SetCameraPath(kPathEmpty);

		/* カメラ映像パス切替 */
		ret = p_layout_mng_->SetCameraPath(kPathEmpty, true);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetCameraPath error. ret=%d", ret);
			return ret;
		}

		/* 画質調整用の設定 */
		(void)p_color_mng_->SetFrontPath(kPathEmpty);
		p_color_mng_->SetCameraPath(kPathEmpty);

		/* カメラ映像MUTE-OFF */
		p_mute_->SetMuteCameraPathOff();
	}
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	STRサスペンド処理(前席映像パスクリア)
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_****	エラー
           		VHAL_SUCCESS	正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::StrSuspendPathClearFront(void)
{
	std::string video_path{};				/* 映像パス */
	int32_t	ret{VHAL_SUCCESS};				/* 戻り値 */

	/* NULL調査 */
	if ((nullptr == p_layout_mng_) || (nullptr == p_color_mng_))
	{
		VHAL_LOGW("parameter error. layout[%p] color[%p]", p_layout_mng_.get(), p_color_mng_.get());
		return VHAL_ERR;
	}

	/* 映像パス取得 */
	ret = p_layout_mng_->GetFrontPath(video_path);
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("GetFrontPath error. ret=%d", ret);
		return ret;
	}

	/* 変更有無チェック */
	if (!video_path.empty())
	{
		/* HDMI再生停止 */
		if (video_path == kPathHdmi)
		{
			hdmi_control_.Stop(CVhalPropertyControlHdmi::CTRL_TARGET_FRONT);
		}

		/* 前席映像パス切替 */
		ret = p_layout_mng_->SetFrontPath(kPathEmpty);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetFrontPath error. ret=%d", ret);
			return ret;
		}

		/* 画質調整用の設定 */
		(void)p_color_mng_->SetFrontPath(kPathEmpty);

		/* 前席スクリーンID取得 */
		int32_t screen_id{};
		ret = p_layout_mng_->GetScreenIdFront(screen_id);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetScreenIdFront error. ret=%d", ret);
			return ret;
		}

		/* スクリーンサイズ取得 */
		VhalRectanglePropValues output_current{};
		output_current.rect[VHAL_RECTANGLE_X] = 0;
		output_current.rect[VHAL_RECTANGLE_Y] = 0;
		ret = p_layout_mng_->GetScreenSize(screen_id, output_current.rect[VHAL_RECTANGLE_W], output_current.rect[VHAL_RECTANGLE_H]);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetScreenSize error. ret=%d", ret);
			return ret;
		}

		/* 前席出力領域設定 */
		ret = p_layout_mng_->SetFrontDestRectangle(output_current.rect[VHAL_RECTANGLE_X],
												   output_current.rect[VHAL_RECTANGLE_Y],
												   output_current.rect[VHAL_RECTANGLE_W],
												   output_current.rect[VHAL_RECTANGLE_H]);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetFrontDestRectangle error. ret=%d", ret);
			return ret;
		}
	}
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	STRサスペンド処理(後席映像パスクリア)
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_****	エラー
           		VHAL_SUCCESS	正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::StrSuspendPathClearRear(void)
{
	std::string video_path{};			/* 映像パス */
	int32_t	ret{VHAL_SUCCESS};			/* 戻り値 */

	/* NULL調査 */
	if ((nullptr == p_layout_mng_) || (nullptr == p_micon_most_video_info_))
	{
		VHAL_LOGW("parameter error. layout[%p] mostv[%p]", p_layout_mng_.get(), p_micon_most_video_info_.get());
		return VHAL_ERR;
	}

	/* 映像パス取得 */
	ret = p_layout_mng_->GetRearPath(video_path);
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("GetRearPath error. ret=%d", ret);
		return ret;
	}

	/* 変更有無チェック */
	if (!video_path.empty())
	{
		/* 後席スクリーンID取得 */
		int32_t screen_id{};
		ret = p_layout_mng_->GetScreenIdRear(screen_id);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetScreenIdRear error. ret=%d", ret);
			return ret;
		}

		/* 後席スクリーン利用可能か確認 */
		const bool rear_available{p_layout_mng_->IsScreenAvailable(screen_id)};
		if (!rear_available)
		{
			VHAL_LOGI("rear screen is not available.");
			return VHAL_SUCCESS;
		}

		/* HDMI再生停止 */
		if (video_path == kPathHdmi)
		{
			hdmi_control_.Stop(CVhalPropertyControlHdmi::CTRL_TARGET_REAR);
		}

		/* 後席映像パス切替 */
		ret = p_layout_mng_->SetRearPath(kPathEmpty);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetRearPath error. ret=%d", ret);
			return ret;
		}

		/* 後席映像パス切替(MOST) */
		p_micon_most_video_info_->SetRearPath(kPathEmpty);

		/* スクリーンサイズ取得 */
		VhalRectanglePropValues output_current{};
		output_current.rect[VHAL_RECTANGLE_X] = 0;
		output_current.rect[VHAL_RECTANGLE_Y] = 0;
		ret = p_layout_mng_->GetScreenSize(screen_id, output_current.rect[VHAL_RECTANGLE_W], output_current.rect[VHAL_RECTANGLE_H]);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetScreenSize error. ret=%d", ret);
			return ret;
		}

		/* 後席出力領域設定 */
		ret = p_layout_mng_->SetRearDestRectangle(output_current.rect[VHAL_RECTANGLE_X],
												  output_current.rect[VHAL_RECTANGLE_Y],
												  output_current.rect[VHAL_RECTANGLE_W],
												  output_current.rect[VHAL_RECTANGLE_H]);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetRearDestRectangle error. ret=%d", ret);
			return ret;
		}
	}
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	STRサスペンド処理(IC映像パスクリア)
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_****	エラー
           		VHAL_SUCCESS	正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::StrSuspendPathClearICluster(void)
{
	std::string video_path{};			/* 映像パス */
	int32_t	ret{VHAL_SUCCESS};			/* 戻り値 */

	/* NULL調査 */
	if (nullptr == p_layout_mng_)
	{
		VHAL_LOGW("parameter error. layout[%p]", p_layout_mng_.get());
		return VHAL_ERR;
	}

	/* 映像パス取得 */
	ret = p_layout_mng_->GetIClusterPath(video_path);
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("GetIClusterPath error. ret=%d", ret);
		return ret;
	}

	/* 変更有無チェック */
	if (!video_path.empty())
	{
		/* ICスクリーンID取得 */
		int32_t screen_id{};
		ret = p_layout_mng_->GetScreenIdICluster(screen_id);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetScreenIdICluster error. ret=%d", ret);
			return ret;
		}

		/* ICスクリーン利用可能か確認 */
		const bool ic_available{p_layout_mng_->IsScreenAvailable(screen_id)};
		if (!ic_available)
		{
			VHAL_LOGI("ICluster screen is not available.");
			return VHAL_SUCCESS;
		}

		/* IC映像パス切替 */
		ret = p_layout_mng_->SetIClusterPath(kPathEmpty);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetIClusterPath error. ret=%d", ret);
			return ret;
		}

		/* スクリーンサイズ取得 */
		VhalRectanglePropValues output_current{};
		output_current.rect[VHAL_RECTANGLE_X] = 0;
		output_current.rect[VHAL_RECTANGLE_Y] = 0;
		ret = p_layout_mng_->GetScreenSize(screen_id, output_current.rect[VHAL_RECTANGLE_W], output_current.rect[VHAL_RECTANGLE_H]);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetScreenSize error. ret=%d", ret);
			return ret;
		}

		/* IC出力領域設定 */
		ret = p_layout_mng_->SetIClusterDestRectangle(output_current.rect[VHAL_RECTANGLE_X],
													  output_current.rect[VHAL_RECTANGLE_Y],
													  output_current.rect[VHAL_RECTANGLE_W],
													  output_current.rect[VHAL_RECTANGLE_H]);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetIClusterDestRectangle error. ret=%d", ret);
			return ret;
		}
	}
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	内部リソース再初期化処理
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControl::ReInit(void) noexcept
{
	/* HDMI制御再初期化 */
	hdmi_control_.ReInit();

	/* カメラ制御再初期化 */
	camera_control_.ReInit();

	/* Mute情報再初期化 */
	if (nullptr != p_mute_)
	{
		p_mute_->ReInit();
	}

	/* TAB通信イベント再初期化 */
	if (nullptr != p_capture_tab_receiver_)
	{
		p_capture_tab_receiver_->ReInit();
	}

	/* MOST映像情報通知送信モジュール再初期化 */
	if (nullptr != p_micon_most_video_info_)
	{
		p_micon_most_video_info_->ReInit();
	}

	/* 画質調整初期化 (CVhalColorManagerのコンストラクタと同値に設定) */
	/* ちなみにMiconCommコントロール停止中の為、画質モード通知の出力はしない */
	if (nullptr != p_color_mng_)
	{
		p_color_mng_->ReInit();
	}

	/* HUD制御再初期化 */
	if (nullptr != p_hud_screen_controller_)
	{
		p_hud_screen_controller_->ReInit();
	}
}

/*****************************************************************************
 処理概要： プロパティ初期値設定
 引数    ：	std::unordered_map<std::string, int64_t>&		tbl_num			(i)固定値テーブル(数値)
           	std::unordered_map<std::string, bool>&			tbl_bool		(i)固定値テーブル(BOOL)
           	std::unordered_map<std::string, std::string>&	tbl_str			(i)固定値テーブル(文字列)
           	std::vector<std::string>&						status_quo		(i)設定対象外項目(現状維持：最優先)
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControl::InitializeProperties(const std::unordered_map<std::string, int64_t>&     tbl_num, 
												const std::unordered_map<std::string, bool>&        tbl_bool,
												const std::unordered_map<std::string, std::string>& tbl_str, 
												const std::vector<std::string>& status_quo)
{
	const std::unordered_set<std::string> status_quo_set{status_quo.begin(), status_quo.end()};

	/* property_entries_の全項目を対象とする */
	for (auto& elem : property_entries_)
	{
		/* 設定対象外項目であるか判断 */
		bool	is_status_quo{status_quo_set.count(elem.first) > 0U};	/* 存在する場合true */

		/* 設定対象外項目ではない場合 */
		if (!is_status_quo)
		{
			VhalPropertyEntry&	entry{elem.second};

			/* 数値型変数についての調査 */
			if (VHAL_DATA_TYPE_NUM == entry.GetType())
			{
				int64_t		param{0LL};
				auto itr = tbl_num.find(elem.first);
				if (itr != tbl_num.end())
				{
					param = itr->second;
				}
				entry.SetNumCurrent(param);
				entry.SetNumPrev(0LL);
			}
			/* bool型変数についての調査 */
			else if (VHAL_DATA_TYPE_BOOL == entry.GetType())
			{
				bool		param{false};
				auto itr = tbl_bool.find(elem.first);
				if (itr != tbl_bool.end())
				{
					param = itr->second;
				}
				entry.SetBoolCurrent(param);
				entry.SetBoolPrev(false);
			}
			/* 文字列型変数についての調査 */
			else if (VHAL_DATA_TYPE_STR == entry.GetType())
			{
				std::string	param{""};
				auto itr = tbl_str.find(elem.first);
				if (itr != tbl_str.end())
				{
					param = itr->second;
				}
				entry.AllocateStrCurrent(param);
				entry.AllocateStrPrev("");
			}
			/* その他 */
			else
			{
				/* 下記ログが出力されると構成異常のため見直し要 */
				VHAL_LOGE("Invalid VhalPropertyEntry name=[%s] type=[%u]", elem.first.c_str(), entry.GetType());
			}
		}
	}
}

/*****************************************************************************
 処理概要：	カメラ種別判別要求送信
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_****	エラー
           		VHAL_SUCCESS	正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::SendCameraType(void) const
{
	int32_t	result{VHAL_SUCCESS};

	if (nullptr == p_micon_comm_control_)
	{
		VHAL_LOGE("parameter error. micon is null");
		result = VHAL_ERR_PARAM;
	}
	else
	{
		CVhalCaptureTabSendItem tab_send_item{};
		tab_send_item.SetTabCmdCameraTypeNotify();
		result = p_micon_comm_control_->Send(tab_send_item);
	}
	return result;
}

/*****************************************************************************
 処理概要：	カメラ映像モード 定期送信
 引数    ：	bool create_flag ：　true:タイマ生成する  false:タイマ生成しない
 戻り値  ：	処理結果
           		VHAL_ERR_****	エラー
           		VHAL_SUCCESS	正常終了
*****************************************************************************/
int32_t CVhalPropertyControl::SendIntervalCameraMode(const bool create_flag)
{
	int32_t	result{VHAL_SUCCESS};

	/* NULL調査 */
	if ((nullptr == p_micon_comm_control_) || (nullptr == p_camera_mode_send_item_))
	{
		VHAL_LOGW("parameter error. micon[%p] cmode[%p]", p_micon_comm_control_.get(), p_camera_mode_send_item_.get());
		result = VHAL_ERR_PARAM;
	}

	if (VHAL_SUCCESS == result)
	{
		/* カメラ映像モード通知 データ生成 */
		p_camera_mode_send_item_->SendCameraVisualMode(kCameraVisualModeOther);

		if (create_flag)
		{
			constexpr uint32_t camera_mode_msec{700U};
			const int32_t ret{p_micon_comm_control_->CreateIntervalTimer(p_camera_mode_send_item_.get(), camera_mode_msec)};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("CreateIntervalTimer error. ret=%d", ret);
				result = ret;
			}
		}
	}

	if (VHAL_SUCCESS == result)
	{
		const int32_t ret{p_micon_comm_control_->SendIntervalRestart(p_camera_mode_send_item_.get())};
		if (VHAL_SUCCESS != ret)
		{
			/* CCM初期化に失敗した場合、本処理は失敗するがここでエラーにすると */
			/* VideoHAL全体が終了するようになっているため、送信に失敗してもエラーにはしない */
			VHAL_LOGE("SendIntervalRestart error. ret=%d", ret);
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	shutdown処理
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControl::Shutdown(void) const noexcept
{
	VHAL_LOGD("shutdown end.");
}

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalPropertyControlHdmi::CVhalPropertyControlHdmi(void) noexcept
	:p_capture_hdmi_(nullptr)
	,p_sysdbctrl_(nullptr)
	,p_hdcp_auth_mng_(nullptr)
	,p_layout_mng_(nullptr)
{
	(void)is_play_.reset();
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalPropertyControlHdmi::~CVhalPropertyControlHdmi(void)
{
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化処理
 引数    ：	CVhalSysdbControl* p_sysdbctrl	(i)SysDB制御インスタンスポインタ
         ：	CVhalHdcpAuthManager* p_hdcp_auth_mng	(i)HDCP認証インスタンスポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
           		VHAL_ERR_PARAM		引数異常
*****************************************************************************/
int32_t CVhalPropertyControlHdmi::Initialize(CVhalSysdbControl* const p_sysdbctrl, CVhalHdcpAuthManager* const p_hdcp_auth_mng) noexcept
{
	if ((nullptr == p_sysdbctrl) || (nullptr == p_hdcp_auth_mng))
	{
		VHAL_LOGE("parameter error. p_sysdbctrl=%p p_hdcp_auth_mng=%p", p_sysdbctrl, p_hdcp_auth_mng);
		return VHAL_ERR_PARAM;
	}

	p_sysdbctrl_ = p_sysdbctrl;
	p_hdcp_auth_mng_ = p_hdcp_auth_mng;
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	終了処理（HDMI再生中であれば停止）
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControlHdmi::Finalize(void)
{
	Stop();
}

/*****************************************************************************
 処理概要：	再初期化処理
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControlHdmi::ReInit(void) noexcept
{
	/* 事前にHDMI再生停止している為、設定クリア済のはずだが確実に初期化しておく */
	Stop();
}

/*****************************************************************************
 処理概要：	HDMI再生
 引数    ：	CtrlTarget 				target				(i)再生制御対象
           	CVhalLayoutManager* 	p_layout_mng		(i)レイアウト制御インスタンスポインタ
           	CVhalCaptureControl* 	p_capture_control	(i)キャプチャ制御インスタンスポインタ
           	uint32_t 				width				(i)ビデオフォーマット変更後の解像度幅　（初回再生であれば'0'）
           	uint32_t 				height				(i)ビデオフォーマット変更後の解像度高さ（初回再生であれば'0'）
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalPropertyControlHdmi::Play(const CtrlTarget target, CVhalLayoutManager* const  p_layout_mng, CVhalCaptureControl* const p_capture_control, const uint32_t width, const uint32_t height)
{
	if (((CTRL_TARGET_FRONT != target) && (CTRL_TARGET_REAR != target)) ||
		(nullptr == p_layout_mng))
	{
		VHAL_LOGE("parameter error. target=%d p_layout_mng=%p", target, p_layout_mng);
		return VHAL_ERR_PARAM;
	}

 	int32_t ret{VHAL_SUCCESS};

	if ((CTRL_TARGET_FRONT != target) && (CTRL_TARGET_REAR != target))
	{
		VHAL_LOGE("parameter error. target=%d", target);
		ret = VHAL_ERR_PARAM;
	}
	else
	{
		if (nullptr == p_capture_hdmi_)
		{
			p_capture_hdmi_ = std::make_unique<CVhalCaptureHdmi>();
			ret = p_capture_hdmi_->Initialize(p_layout_mng, p_capture_control, width, height);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("Initialize error. ret=%d target=%d", ret, target);
				p_capture_hdmi_->Finalize();
				p_capture_hdmi_ = nullptr;
			}
			else
			{
				(void)is_play_.set(static_cast<std::size_t>(target));
			}
		}
		else
		{
			(void)is_play_.set(static_cast<std::size_t>(target));
		}

		/* HDMI出力先が前席映像 */
		if ((VHAL_SUCCESS == ret) && (CVhalPropertyControlHdmi::CTRL_TARGET_FRONT == target))
		{
			if (nullptr == p_layout_mng_)
			{
				p_layout_mng_ = p_layout_mng;
			}
	
			/* HDMI認証状態の取得 */
			CVhalHdcpAuthRsltData	hdcp_cdisp;
			(void)p_hdcp_auth_mng_->GetHdcpAuthKey(hdcpAuthType::HDCP_AUTH_TYPE_CDISP, hdcp_cdisp);
	
			/* 認証に成功しているか調査 */
			if ((SUB_TYPE_HDCP_AUTH_NTY_CDISP == hdcp_cdisp.GetSubType()) &&
				(VHAL_HDCP_FIRST_AUTH_STS_SUCCESS == hdcp_cdisp.GetResult()))
			{
				/* 前席映像同期面のMUTE-OFF */
				VHAL_LOGI("MuteVideoSync mute-off.");
				p_layout_mng_->SetBrinderSync(VhalMuteFrontReason::VHAL_MUTE_FRONT_REASON_HDMI, false);
			}
			else
			{
				/* 前席映像同期面のMUTE-ON */
				VHAL_LOGI("MuteVideoSync mute-on. C-Disp: sub_type=%u result=%d",hdcp_cdisp.GetSubType(), hdcp_cdisp.GetResult());
				p_layout_mng_->SetBrinderSync(VhalMuteFrontReason::VHAL_MUTE_FRONT_REASON_HDMI, true);
			}
		}

		const std::string play_bit{is_play_.to_string()};
		VHAL_LOGI("HDMI play status target=%d bit=%s", target, play_bit.c_str());
	}
	return ret;
}

/*****************************************************************************
 処理概要：	HDMI停止（前席や後席など全て）
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControlHdmi::Stop(void)
{
	for( uint32_t i{0U}; i<CTRL_TARGET_MAX; ++i )
	{
		Stop(static_cast<CtrlTarget>(i));
	}
}

/*****************************************************************************
 処理概要：	HDMI停止
 引数    ：	CtrlTarget 				target				(i)停止制御対象
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControlHdmi::Stop(const CtrlTarget target)
{
	if ((CTRL_TARGET_FRONT != target) && (CTRL_TARGET_REAR != target))
	{
		VHAL_LOGE("parameter error. target=%d", target);
		return;
	}

	(void)is_play_.reset(static_cast<std::size_t>(target));

	/* 全出力先へのHDMI再生を停止したら、HDMIインスタンス削除 */
	const std::size_t count{is_play_.count()};
	if ((0U == count) && (nullptr != p_capture_hdmi_) )
	{
		p_capture_hdmi_->Finalize();
		p_capture_hdmi_ = nullptr;
	}

	/* HDMI出力先が前席映像 */
	if (CVhalPropertyControlHdmi::CTRL_TARGET_FRONT == target)
	{
		if (nullptr != p_layout_mng_)
		{
			/* 前席映像同期面のMUTE-OFF */
			VHAL_LOGI("MuteVideoSync mute-off.");
			p_layout_mng_->SetBrinderSync(VhalMuteFrontReason::VHAL_MUTE_FRONT_REASON_HDMI, false);
		}
	}

	const std::string play_bit{is_play_.to_string()};
	VHAL_LOGI("HDMI play status target=%d bit=%s", target, play_bit.c_str());
}

/*****************************************************************************
 処理概要：	HDMI再生可否判定
 引数    ：	int32_t&  connect		(i/o)HDMI接続状態値[0～2:SYSDB変化通知時の値, -1:未取得のため本関数内でSYSDEBから値を取得]
           	int32_t&  format		(i/o)解像度値[0～4:SYSDB変化通知時の値, -1:未取得のため本関数内でSYSDEBから値を取得]
           	uint32_t& width			(o)解像度幅
           	uint32_t& height		(o)解像度高さ
 戻り値  ：	処理結果
           		false		失敗(HDMI再生不可)
           		true		成功(HDMI再生可能)
*****************************************************************************/
bool CVhalPropertyControlHdmi::Available(int32_t& connect, int32_t& format, uint32_t& width, uint32_t& height) const
{
	bool result{true};

	/* HDMI接続状態確認 */
	if (kUnknown == connect)
	{
		const int32_t ret{p_sysdbctrl_->GetValue(VHAL_SYSDB_PATH_CONN_INFO_HDMI, &connect)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SysDB %s Read error. ret=%d", VHAL_SYSDB_PATH_CONN_INFO_HDMI.c_str(), ret);
		}
	}

	if (DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECT != connect)
	{
		result = false;
	}
	else
	{
		if (kUnknown == format)
		{
			/* HDMIビデオフォーマット確認 */
			const int32_t ret{p_sysdbctrl_->GetValue(VHAL_SYSDB_PATH_HDMI_VIDEO_FORMAT, &format)};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("SysDB %s Read error. ret=%d", VHAL_SYSDB_PATH_HDMI_VIDEO_FORMAT.c_str(), ret);
			}
		}

		const int32_t ret{p_sysdbctrl_->GetHdmiVideoFormat(format, width, height)};
		if (VHAL_SUCCESS != ret)
		{
			result = false;
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	HDMI再生中調査
 引数    ：	CtrlTarget 				target				(i)対象
 戻り値  ：	bool		true:再生中  false:非再生
*****************************************************************************/
bool CVhalPropertyControlHdmi::IsPlaying(const CtrlTarget target) const
{
	bool	result{false};
	std::size_t		pos{static_cast<std::size_t>(target)};

	if (pos < is_play_.size())
	{
		result = is_play_.test(pos);
	}
	return result;
}

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalPropertyControlCamera::CVhalPropertyControlCamera(void) noexcept
{

}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalPropertyControlCamera::~CVhalPropertyControlCamera(void)
{
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化処理
 引数    ：	CVhalSysdbControl*	p_sysdbctrl		(i)SysDB制御インスタンスポインタ
          	CVhalLayoutManager*	p_layout_mng	(i)IVIイベントリスナインスタンスポインタ
          	CVhalGpioControl*	p_gpioctrl		(i)Gpio制御インスタンスポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalPropertyControlCamera::Initialize(CVhalSysdbControl* const p_sysdbctrl, CVhalLayoutManager* const p_layout_mng, CVhalGpioControl* const p_gpioctrl)
{
	int32_t	ret{VHAL_SUCCESS};

	if ((nullptr == p_sysdbctrl) || (nullptr == p_layout_mng) || (nullptr == p_gpioctrl))
	{
		VHAL_LOGE("parameter error. p_sysdbctrl=%p, p_layout_mng=%p, p_gpioctrl=%p", p_sysdbctrl, p_layout_mng, p_gpioctrl);
		ret = VHAL_ERR_PARAM;
	}
	else
	{
		p_sysdbctrl_  = p_sysdbctrl;
		p_layout_mng_ = p_layout_mng;
		p_gpioctrl_   = p_gpioctrl;
	}
	return ret;
}

/*****************************************************************************
 処理概要：	初期化処理
 引数    ：	CVhalCaptureTabReceiver	*p_capture_tab_receiver	(i)キャプチャTAB2制御インスタンスポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalPropertyControlCamera::InitializeCaptureTabReceiver(CVhalCaptureTabReceiver* const p_capture_tab_receiver)
{
	int32_t	ret{VHAL_SUCCESS};

	if (nullptr == p_capture_tab_receiver)
	{
		VHAL_LOGE("parameter error. p_capture_tab_receiver=%p", p_capture_tab_receiver);
		ret = VHAL_ERR_PARAM;
	}
	else
	{
		p_capture_tab_receiver_ = p_capture_tab_receiver;
	}

	return ret;
}

/*****************************************************************************
 処理概要：	初期化処理
 引数    ：	CVhalMute* p_mute	(i)Mute制御インスタンスポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalPropertyControlCamera::InitializeMute(CVhalMute* const p_mute)
{
	int32_t	ret{VHAL_SUCCESS};

	if (nullptr == p_mute)
	{
		VHAL_LOGE("parameter error. p_mute=%p", p_mute);
		ret = VHAL_ERR_PARAM;
	}
	else
	{
		p_mute_ = p_mute;
	}

	return ret;
}

/*****************************************************************************
 処理概要：	初期化処理
 引数    ：	CVhalFileObserverControl* p_fileob		(i)FileObserver制御インスタンスポインタ
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControlCamera::InitializeFileObserver(CVhalFileObserverControl* const p_fileob) noexcept
{
	p_fileobserver_control_ = p_fileob;
}

/*****************************************************************************
 処理概要：	終了処理
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControlCamera::Finalize(void)
{
	ReleaseCameraCapture();
}

/*****************************************************************************
 処理概要：	再初期化処理
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControlCamera::ReInit(void) noexcept
{
	/* 事前にカメラを停止している為、設定クリア済のはずだが確実に初期化しておく */
	ReleaseCameraCapture();
	camera_cap_stby2_ = false;
	camera_cap_waiting_ = false;
	camera_cap_frame_ = VhalCaptureFrameStatus::kInitial;
	camera_cap_stby2_wait_ = 0U;
	vpower_state_ = VehiclePowerState::VHAL_VPWR_STS_MAX;
}

/*****************************************************************************
 処理概要：	カメラキャプチャ設定
 引数    ：	CVhalCaptureControl*	p_capture_control	(i)キャプチャ制御インスタンスポインタ
           	int64_t					cameraType			(i)カメラ種別
           	bool					error_output		(i)エラー出力有無
 戻り値  ：	処理結果
           		VHAL_ERR_****	エラー
           		VHAL_SUCCESS	正常終了
*****************************************************************************/
int32_t CVhalPropertyControlCamera::SettingtCameraCapture(CVhalCaptureControl* const p_capture_control, const int64_t cameraType, const bool error_output)
{
	int32_t ret{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	do
	{
		if (nullptr == p_capture_camera_)
		{
			if (VHAL_CONNECTED_CAMERA_NONE >= cameraType)
			{
				if ((VHAL_CONNECTED_CAMERA_NONE == cameraType) && (false == error_output))
				{
					VHAL_LOGI("Camera not connected");
				}
				else
				{
					VHAL_LOGE("Camera type[%ld] error.", cameraType);
				}
				ret = VHAL_ERR_PARAM;
				break;
			}

			p_capture_camera_ = std::make_unique<CVhalCaptureCamera>();
			ret = p_capture_camera_->Initialize(p_layout_mng_, p_capture_control);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("Initialize error. ret=%d", ret);
				ReleaseCameraCapture();
			}
			else
			{
				/* キャプチャ設定済のためwait状態解除 */
				/* wait中にカメラ種別[有効]が無効→有効となりキャプチャ設定が正常処理された場合 */
				SetWaitingCapture(false);
			}
		}
	} while (false);

	VHAL_LOGV_OUT();
	return ret;
}

/*****************************************************************************
 処理概要：	カメラキャプチャ設定解除
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControlCamera::ReleaseCameraCapture(void)
{
	VHAL_LOGV_IN();

	/* カメラ設定済なら設定解除を実施する */
	if (nullptr != p_capture_camera_)
	{
		/* 設定解除(キャプチャ中なら停止も実施する。Finalizeの中でやる) */
		p_capture_camera_->Finalize();
		p_capture_camera_ = nullptr;
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	カメラパス設定
 引数    ：	const std::string& path	(i)カメラパス文字列
 戻り値  ：	処理結果
           		VHAL_ERR_****	エラー
           		VHAL_SUCCESS	正常終了
*****************************************************************************/
int32_t CVhalPropertyControlCamera::SetCameraPath(const std::string& path)
{
	int32_t ret{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	/* カメラ映像パス切替(キャプチャ処理) */
	if (nullptr != p_capture_camera_)
	{
		ret = p_capture_camera_->SetPath(path);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetPath error. ret=%d path=%s", ret, path.c_str());
		}
	}

	VHAL_LOGV_OUT();
	return ret;
}

/*****************************************************************************
 処理概要：	カメラ同期処理関数
 引数    ：	const VhalTabCamSync sync	(i)カメラ映像入力同期
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControlCamera::ActionCameraSync(const VhalTabCamSync sync) const
{
	VHAL_LOGV_IN();

	do
	{
		if (nullptr == p_capture_tab_receiver_)
		{
			VHAL_LOGE("parameter error. p_capture_tab_receiver_=%p", p_capture_tab_receiver_);
			break;
		}

		VHAL_LOGD("ReceiveCameraSync(%d)", sync);

		/* Mute(p_mute)の初期化はInitialize()で行われないため、ここで確認する */
		if (nullptr != p_mute_)
		{
			/* 同期状態の場合MUTE設定を通知 */
			if (VhalTabCamSync::kSync == sync)
			{
				/* 同期有り */
				p_mute_->SetMuteCameraSyncOn();
			}
			else
			{
				/* 同期有り以外 */
				p_mute_->SetMuteCameraSyncOff();
			}
		}

		std::string	path{};
		int32_t		disp{VHAL_CAMERA_DISPLAY_UNKNOWN};
		(void)p_layout_mng_->GetCameraPath(path);
		if (false == path.empty())
		{
			disp = VHAL_CAMERA_DISPLAY_CENTER;
		}

		/* カメラ出力中以外の場合は処理無し */
		if (VHAL_CAMERA_DISPLAY_UNKNOWN == disp)
		{
			break;
		}

		/* カメラ映像パス設定時の同期状態に更新がある場合 */
		if (true == p_capture_tab_receiver_->UpdateCameraPathSync(sync))
		{
			VHAL_LOGI("CameraPathSync(%d)", sync);

			/* MUTE処理(同期状態は設定済) */
			if (nullptr != p_mute_)
			{
				p_mute_->SetMuteCamera();
			}
		}
	} while (false);

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	カメラ電源状態処理関数
 引数    ：	const VehiclePowerState	power_state	(i)車両電源ステート状態
 戻り値  ：	なし
 備考　　： 本処理の実施スレッドはPFMのコールバックのみのため複数スレッドによる同期異常はない。
*****************************************************************************/
void CVhalPropertyControlCamera::ActionCameraPowerState(const VehiclePowerState power_state)
{
	VHAL_LOGV_IN();

	VHAL_LOGI("now_power_state=%d bef_power_state=%d", power_state, vpower_state_);

	/* Mute(p_mute)の初期化はInitialize()で行われないため、ここで確認する */
	if ((nullptr == p_mute_) || (nullptr == p_layout_mng_))
	{
		VHAL_LOGE("parameter error. p_mute_=%p p_layout_mng_=%p", p_mute_, p_layout_mng_);
	}
	else
	{
		const bool now_visual{VehiclePowerState::VHAL_VPWR_STS_VISUAL_ON == power_state};		/* 今回見た目オン起動の場合はtrue */
		const bool bef_visual{VehiclePowerState::VHAL_VPWR_STS_VISUAL_ON == vpower_state_};		/* 前回見た目オン起動の場合はtrue */
		bool is_changed{false};

		/* 今回見た目オン起動 かつ 前回見た目オン起動以外 */
		if (now_visual && !bef_visual)
		{
			p_mute_->SetMuteCameraVehiclePowerStateOn();
			is_changed = true;
		}
		/* 今回見た目オン起動以外 かつ 前回見た目オン起動 */
		else if (!now_visual && bef_visual)
		{
			p_mute_->SetMuteCameraVehiclePowerStateOff();
			is_changed = true;
		}
		/* その他 */
		else
		{
			VHAL_LOGD("No processing.");
		}

		/* 見た目オン/オフに関する変更発生の場合 */
		if (is_changed)
		{
			/* カメラが映像表示状態であるか */
			std::string	path{};
			(void)p_layout_mng_->GetCameraPath(path);
			if (false == path.empty())
			{
				/* カメラ面のMUTE設定 */
				p_mute_->SetMuteCamera();
			}

			/* 映像キャプチャ動作設定(車両電源ステート) */
			ActionCameraControl(power_state);
		}

		/* 電源状態の更新 */
		vpower_state_ = power_state;
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	カメラ映像の入力サイズ設定
 引数    ：	const int32_t	disp	(i)表示カメラ種別
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControlCamera::SetCameraInputSize(const int32_t disp)
{
	VHAL_LOGV_IN();

	if (nullptr != p_capture_tab_receiver_)
	{
		int32_t	input_w{0};
		int32_t	input_h{0};
		int32_t	ret{VHAL_SUCCESS};
		ret = p_capture_tab_receiver_->GetCameraInputSize(disp, input_w, input_h);
		if (VHAL_SUCCESS == ret)
		{
			switch (disp)
			{
				case VHAL_CAMERA_DISPLAY_CENTER:
					VHAL_LOGI("SetCameraForceInputSize(%d, %d)", input_w, input_h);
					ret = p_layout_mng_->SetCameraForceInputSize(input_w, input_h);
					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("SetCameraForceInputSize(%d, %d) error. ret=%d", input_w, input_h, ret);
					}
					/* カメラサーフェスに対してSourceRectangle反映 */
					if (nullptr != p_capture_camera_)
					{
						ret = p_layout_mng_->SetCameraForceSourceRectangle(0, 0, input_w, input_h);
						if (VHAL_SUCCESS != ret)
						{
							VHAL_LOGE("SetCameraForceSourceRectangle(%d, %d) error. ret=%d", input_w, input_h, ret);
						}
					}
					break;
				default:
					VHAL_LOGE("Unknown CameraDisplay=%d.", disp);
					break;
			}
		}
		else
		{
			VHAL_LOGE("GetCameraInputSize(%d) error. ret=%d", disp, ret);
		}
	}
	else
	{
		VHAL_LOGE("parameter error. p_capture_tab_receiver_=%p", p_capture_tab_receiver_);
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	映像キャプチャ動作設定(カメラ映像入力同期変更)
 引数    ：	const VhalTabCamSync	sync	(i)カメラ映像入力同期
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControlCamera::ActionCameraControl(const VhalTabCamSync sync) const
{
	VHAL_LOGV_IN("sync=%d", sync);

	ActionCameraControl(sync, vpower_state_);

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	映像キャプチャ動作設定(車両電源ステート)
 引数    ：	const VehiclePowerState	power_state	(i)車両電源ステート状態
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControlCamera::ActionCameraControl(const VehiclePowerState power_state) const
{
	VHAL_LOGV_IN("power_state=%d", power_state);

	do
	{
		if (nullptr == p_capture_tab_receiver_)
		{
			VHAL_LOGE("parameter error. p_capture_tab_receiver_=%p", p_capture_tab_receiver_);
			break;
		}

		/* カメラ同期取得 */
		const VhalTabCamSync	sync{p_capture_tab_receiver_->GetCameraSync()};

		ActionCameraControl(sync, power_state);
	} while (false);

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	映像キャプチャ動作設定(同期状態・電源状態を内部取得して制御実行)
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControlCamera::ActionCameraControl(void) const
{
	VHAL_LOGV_IN("none parameter");

	do
	{
		if (nullptr == p_capture_tab_receiver_)
		{
			VHAL_LOGE("parameter error. p_capture_tab_receiver_=%p", p_capture_tab_receiver_);
			break;
		}

		/* カメラ同期取得 */
		const VhalTabCamSync	sync{p_capture_tab_receiver_->GetCameraSync()};

		ActionCameraControl(sync, vpower_state_);
	} while (false);

	VHAL_LOGV_OUT();
}


/*****************************************************************************
 処理概要：	映像キャプチャ動作設定
 引数    ：	const VhalTabCamSync	sync		(i)カメラ映像入力同期
         ：	const VehiclePowerState	power_state	(i)車両電源ステート状態
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControlCamera::ActionCameraControl(const VhalTabCamSync sync, const VehiclePowerState power_state) const
{
	VHAL_LOGV_IN("sync=%d power_state=%d", sync, power_state);

	do
	{
		/* キャプチャ状態チェック */
		if (nullptr == p_capture_camera_)
		{
			/* カメラ種別未設定のため処理なし */
			VHAL_LOGD("Camera-type none.");
			break;
		}

		bool	camera_start{true};

		/* カメラ同期チェック */
		if (VhalTabCamSync::kSync != sync)
		{
			camera_start = false;
		}

		/* 車両電源ステート状態チェック */
		if (VehiclePowerState::VHAL_VPWR_STS_VISUAL_ON != power_state)
		{
			camera_start = false;
		}

		VHAL_LOGD("ActionCameraControl:status[%d], sync[%d], power[%d]", camera_start, sync, power_state);

		int32_t	ret{VHAL_SUCCESS};
		if (true == camera_start)
		{
			ret = p_capture_camera_->Start();
		}
		else
		{
			ret = p_capture_camera_->Stop();
		}

		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("ActionCameraControl(%d) error. ret=%d", camera_start, ret);
		}
	} while (false);

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	カメラ制御権設定(CAMERA-CAP-STBY2)処理
 引数    ：	const bool	status	(i)true:Hi/false:Lo
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControlCamera::SetCameraCaptureStandby2(const bool status)
{
	VHAL_LOGV_IN("status=%d", status);

	/* パラメータチェック */
	if (nullptr == p_fileobserver_control_)
	{
		VHAL_LOGE("parameter error. p_fileobserver_control_=%p", p_fileobserver_control_);
	}
	else if (camera_cap_stby2_ == status)
	{
		/* 変化無し */
	}
	else if (true == status)
	{
		/* CAMERA-CAP-STBY2=Hi指定 */
		if (VhalCaptureFrameStatus::kReady == camera_cap_frame_)
		{
			/* カメラ表示準備ON */
			/* カメラ制御権取得。Readyの場合は更新 */
			VhalCameraDisplayStatus current_cam_disp_status{};
			int32_t ret{p_fileobserver_control_->GetCameraDisplayStatus(current_cam_disp_status)};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("GetCameraDisplayStatus error. ret=%d", ret);
			}
			else if (VhalCameraDisplayStatus::kReady == current_cam_disp_status)
			{
				ret = p_gpioctrl_->SetValueDirectionLow(HAL_GPIO_O_CAMERA_CAP_STBY2, HAL_GPIO_HIGH);
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("SetValueDirectionLow(STBY2, HIGH) error. ret=%d", ret);
				}
				else
				{
					camera_cap_stby2_ = status;
					camera_cap_frame_ = VhalCaptureFrameStatus::kChanged;
					VHAL_LOGI("CameraCaptureStandby2(%d)", camera_cap_stby2_);
				}
			}
			else {
				/* カメラ制御権状態=Initialの場合は処理無し */
			}
		}
	}
	else
	{
		/* CAMERA-CAP-STBY2=Lo指定 */
		const int32_t ret{p_gpioctrl_->SetValueDirectionLow(HAL_GPIO_O_CAMERA_CAP_STBY2, HAL_GPIO_LOW)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetValueDirectionLow(STBY2, LOW) error. ret=%d", ret);
		}
		else
		{
			camera_cap_stby2_ = status;
			camera_cap_frame_ = VhalCaptureFrameStatus::kInitial;
			VHAL_LOGI("CameraCaptureStandby2(%d)", camera_cap_stby2_);
		}
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	カメラキャプチャ表示準備
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControlCamera::SetCameraCaptureFrame(void)
{
	VHAL_LOGV_IN();

	const std::lock_guard<std::mutex> lock_capture{mtx_standby2_};

	/* パラメータチェック */
	if (nullptr == p_capture_tab_receiver_)
	{
		VHAL_LOGE("parameter error. p_capture_tab_receiver_=%p", p_capture_tab_receiver_);
	}
	else
	{
		std::string	path{};
		bool set_standby2{IsSetCameraCaptureStandby2(path)};
		if (true == set_standby2)
		{
			if (!path.empty())
			{
				if (nullptr != p_mute_)
				{
					bool mute_status{false};
					mute_status = p_mute_->GetMuteCameraPath();
					if (true == mute_status)
					{
						/* カメラ切り替えMUTE状態 */
						camera_cap_stby2_wait_ = 0U;
						set_standby2 = false;
					}
					/* MUTE-OFFが描画に反映されるのを待つ(3フレーム[48ms]) */
					if (kCameraCapStby2WaitCnt >= camera_cap_stby2_wait_)
					{
						camera_cap_stby2_wait_++;
						set_standby2 = false;
					}
				}
			}

			if (true == set_standby2)
			{
				camera_cap_stby2_wait_ = 0U;
				if (VhalCaptureFrameStatus::kInitial == camera_cap_frame_)
				{
					camera_cap_frame_ = VhalCaptureFrameStatus::kReady;
					VHAL_LOGI("Capture frame status=%d", camera_cap_frame_);

					/* 映像キャプチャ準備状態通知2:Hi */
					SetCameraCaptureStandby2(true);
				}
			}
		}
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	カメラ制御権通知
 引数    ：	const bool	status	(i)true/false
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControlCamera::ActionCameraCaptureStandby2(const bool status)
{
	const std::lock_guard<std::mutex> lock_capture{mtx_standby2_};

	if (true == status)
	{
		std::string	path{};
		const bool set_standby2{IsSetCameraCaptureStandby2(path)};
		if (true == set_standby2)
		{
			/* Set CAMERA-CAP-STBY2=High */
			SetCameraCaptureStandby2(status);
		}
	}
	else
	{
		/* Set CAMERA-CAP-STBY2=Low */
		SetCameraCaptureStandby2(status);
	}
}

/*****************************************************************************
 処理概要：	カメラキャプチャ設定有無
 引数    ：	なし
 戻り値  ：	処理結果
           		false		キャプチャ未設定
           		true		キャプチャ設定済
*****************************************************************************/
bool CVhalPropertyControlCamera::IsCameraCapture(void) const noexcept
{
	bool result{false};
	if (nullptr != p_capture_camera_)
	{
		result = true;
	}
	return result;
}

/*****************************************************************************
 処理概要：	カメラ制御権設定可否取得
 引数    ：	tring&	path	(o)映像パス
 戻り値  ：	設定可否(true/false)
*****************************************************************************/
bool CVhalPropertyControlCamera::IsSetCameraCaptureStandby2(std::string& path)
{
	bool	ret_stby2{false};

	if (true == camera_cap_stby2_)
	{
		/* CAMERA-CAP-STBY2=High設定済み) */
	}
	else if (nullptr == p_capture_camera_)
	{
		/* カメラ種別未設定のため設定不可 */
	}
	else if (false == p_capture_camera_->IsCapture())
	{
		/* CAMERA-CAP-STBY=Loのため設定不可 */
	}
	else
	{
		/* カメラ映像パス状態の確認 */
		const int32_t ret{p_layout_mng_->GetCameraPath(path)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetCameraPath error. ret=%d", ret);
		}
		else
		{
			/* カメラ映像表示要求状況の取得 */
			const bool state{p_capture_tab_receiver_->GetCameraCenterDisplayState()};
			if (path.empty())
			{
				if (true == state)
				{
					/* カメラ映像表示要求状況(センターディスプレイ)[ON検知] */
					/* 処理無し */
					if (VhalCaptureFrameStatus::kReady == camera_cap_frame_)
					{
						camera_cap_frame_ = VhalCaptureFrameStatus::kInitial;
						VHAL_LOGI("Capture frame status=%d", camera_cap_frame_);
					}
				}
				else
				{
					/* カメラ映像パス[未設定]
					   カメラ映像表示要求状況(センターディスプレイ)[ON検知以外]
					   →CAMERA-CAP-STBY2=Hi */
					ret_stby2 = true;
				}
			}
			else
			{
				if (false == state)
				{
					/* カメラ映像表示要求状況(センターディスプレイ)[ON検知以外]のため設定不可  */
				}
				else
				{
					ret_stby2 = true;
				}
			}
		}
	}

	return ret_stby2;
}

/*****************************************************************************
 処理概要：	スクリーンショット応答受信
 引数    ：	const VhalScreenShotResult result		(i) 応答結果
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyControl::NotifyScreenShotMiconResponse(const VhalScreenShotResult result) noexcept
{
	if (nullptr != p_screen_shot_micon_)
	{
		p_screen_shot_micon_->NotifyScreenShotMiconResponse(result);
	}
	else
	{
		VHAL_LOGE("p_screen_shot_micon_ is null.");
	}
}

} /* namespace videohal */

