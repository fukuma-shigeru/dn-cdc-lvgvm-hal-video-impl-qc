/*******************************************************************************
    機能名称    ：  プロパティ制御モジュール
    ファイル名称：  vhal_property_control.h
*******************************************************************************/
#ifndef	VHAL_PROPERTY_CONTROL_H
#define	VHAL_PROPERTY_CONTROL_H

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <bitset>
#include <memory>

#include "wl_renderer_public.h"
#include "vhal_layout_video_setting.h"
#include "vhal_event_route.h"
#include "vhal_fileobserver_control.h"
#include "vhal_capture_control.h"
#include "vhal_capture_micon_control.h"
#include "vhal_micon_most_video_info.h"
#include "vhal_screen_shot_micon.h"
#include "vhal_event_item_hud_screen_event.h"
#include "vhal_hud_screen_controller.h"

namespace videohal
{

class CVhalIviController;
class CVhalMainControl;
class CVhalPropertyControl;
class CVhalPropertyControlOutputListener;
class CVhalSysdbControl;
class CVhalGpioControl;
class CVhalLayoutManager;
class CVhalMute;
class CVhalCaptureControl;
class CVhalCaptureCamera;
class CVhalCaptureHdmi;
class CVhalMiconCommControl;
class CVhalColorManager;
class CVhalPropertyControlEventListener;
class CVhalCaptureTabReceiver;
class CVhalMiconMiscReceiver;
class CVhalMovieControl;
class CVhalScreenShot;
class CVhalMiconMostVideoInfo;
class CVhalObserverControl;
class CVhalHdcpAuthManager;
class CVhalHdcpMiscReceiver;
class CVhalHdcpAuthRsltData;
class CVhalEnvironmentManager;
class CVhalColorAdjustMiscReceiver;
class CVhalScreenHudReceiver;

/* プロパティエントリー構造体 */
struct VhalPropertyEntry
{
public:
	VhalPropertyEntry(const uint32_t attr, const uint32_t type, const int32_t event_action,
						const std::function<int32_t(VhalPropertyEntry *p_entry)> generate,
						const std::function<int32_t(VhalPropertyEntry *p_entry)> action)
	:attr_(attr)
	,type_(type)
	,num_current_(0)
	,num_prev_(0)
	,bool_current_(false)
	,bool_prev_(false)
	,p_str_current_(nullptr)
	,p_str_prev_(nullptr)
	,p_query_{nullptr}
	,updated_{false}
	,event_action_(event_action)
	,function_generate_(generate)
	,action_(action)
	{
	}

	void SetNumCurrent(const int64_t current) noexcept
	{
		num_current_ = current;
	}

	void SetNumPrev(const int64_t prev) noexcept
	{
		num_prev_ = prev;
	}

	void SetBoolCurrent(const bool current) noexcept
	{
		bool_current_ = current;
	}

	void SetBoolPrev(const bool prev) noexcept
	{
		bool_prev_ = prev;
	}

	void SetStrCurrent(const std::string &current) noexcept
	{
		*p_str_current_ = current;
	}

	void SetStrPrev(const std::string &prev) noexcept
	{
		*p_str_prev_ = prev;
	}

	void SetQuery(std::string* const p_query) noexcept
	{
		p_query_ = p_query;
	}

	void SetUpdated(const bool updated) noexcept
	{
		updated_ = updated;
	}

	uint32_t GetAttr(void) const noexcept
	{
		return attr_;
	}

	uint32_t GetType(void) const noexcept
	{
		return type_;
	}

	int64_t GetNumCurrent(void) const noexcept
	{
		return num_current_;
	}

	int64_t GetNumPrev(void) const noexcept
	{
		return num_prev_;
	}

	bool GetBoolCurrent(void) const noexcept
	{
		return bool_current_;
	}

	bool GetBoolPrev(void) const noexcept
	{
		return bool_prev_;
	}

	std::string GetStrCurrent(void) const noexcept
	{
		return *p_str_current_;
	}

	std::string GetStrPrev(void) const noexcept
	{
		return *p_str_prev_;
	}

	std::string GetQuery(void) const noexcept
	{
		return *p_query_;
	}

	bool GetUpdated(void) const noexcept
	{
		return updated_;
	}

	int32_t Generate(VhalPropertyEntry * const p_entry) const
	{
		return function_generate_(p_entry);
	}

	int32_t Action(VhalPropertyEntry * const p_entry) const
	{
		return action_(p_entry);
	}

	int32_t GetEventAction(void) const noexcept
	{
		return event_action_;
	}

	bool HasGenerate(void) const noexcept
	{
		return nullptr != function_generate_;
	}

	bool HasAction(void) const noexcept
	{
		return nullptr != action_;
	}

	void AllocateStrCurrent(std::string current)
	{
		p_str_current_ = std::make_unique<std::string>(current);
	}

	void AllocateStrPrev(std::string prev)
	{
		p_str_prev_ = std::make_unique<std::string>(prev);
	}

	void FreeStrCurrent(void) noexcept
	{
		p_str_current_ = nullptr;
	}

	void FreeStrPrev(void) noexcept
	{
		p_str_prev_ = nullptr;
	}

	bool StrCurrentIsAllocated(void) const noexcept
	{
		return nullptr != p_str_current_;
	}

	bool StrPrevIsAllocated(void) const noexcept
	{
		return nullptr != p_str_prev_;
	}

private:
	uint32_t attr_;
	uint32_t type_;

	int64_t num_current_;
	int64_t num_prev_;
	bool bool_current_;
	bool bool_prev_;
	std::unique_ptr<std::string> p_str_current_;
	std::unique_ptr<std::string> p_str_prev_;

	std::string *p_query_;
	bool updated_;
	int32_t event_action_;					/* actionイベント発行条件 */
	std::function<int32_t(VhalPropertyEntry *p_entry)> function_generate_;
	std::function<int32_t(VhalPropertyEntry *p_entry)> action_;
};

using VhalRectangle = uint32_t;
static constexpr VhalRectangle VHAL_RECTANGLE_X{0U};
static constexpr VhalRectangle VHAL_RECTANGLE_Y{1U};
static constexpr VhalRectangle VHAL_RECTANGLE_W{2U};
static constexpr VhalRectangle VHAL_RECTANGLE_H{3U};

struct VhalClippingPropNames
{
public:
	VhalClippingPropNames(const std::string enable, const std::string &x, const std::string &y, const std::string &w, const std::string &h)
	:enable_{enable}
	,rect_{x, y, w, h}
	{
	}
	std::vector<std::string> GetRect(void) const noexcept
	{
		return rect_;
	}
	std::string GetEnable(void) const noexcept
	{
		return enable_;
	}
	std::string GetRect(const uint32_t i) const noexcept
	{
		return rect_.at(static_cast<uint64_t>(i));
	}
private:
	std::string enable_;
	std::vector<std::string> rect_;
};

struct VhalClippingPropValues
{
	bool enable;
	std::array<int32_t, 4> rect;
};

struct VhalRectanglePropNames
{
	VhalRectanglePropNames(const std::string &x, const std::string &y, const std::string &w, const std::string &h)
	:rect{x, y, w, h}
	{
	}

	std::string GetRect(const uint32_t i) const noexcept
	{
		return rect.at(static_cast<uint64_t>(i));
	}

	std::vector<std::string> GetRect(void) const noexcept
	{
		return rect;
	}

private:
	std::vector<std::string> rect;
};

struct VhalRectanglePropValues
{
	std::array<int32_t, 4> rect;
};

/* カメラ表示準備状態 */
enum class VhalCaptureFrameStatus : uint32_t {
	kInitial,			/* 初期値 */
	kReady,				/* 表示準備完了 */
	kChanged			/* SoC切替完了 */
};

/* シーン別電源状態 */
enum class ScenePowerState : uint32_t {
	VHAL_PWR_STS_IN_VEHICLE = 0,	/* 乗車中     */
	VHAL_PWR_STS_BG_BOOT,			/* 駐乗車起動 */
	VHAL_PWR_STS_NML_BOOT,			/* 通常起動   */
	VHAL_PWR_STS_MAX
};

/* 車両電源状態 */
enum class VehiclePowerState : uint32_t {
	VHAL_VPWR_STS_SUSPEND = 0,		/* サスペンド     */
	VHAL_VPWR_STS_BG_BOOT,			/* 駐乗車起動     */
	VHAL_VPWR_STS_VISUAL_OFF,		/* 乗車中(見た目オフ起動) */
	VHAL_VPWR_STS_VISUAL_ON,		/* 見た目オン起動 */
	VHAL_VPWR_STS_LIMP_MODE,		/* 縮退走行       */
	VHAL_VPWR_STS_MAX,				/* 有効値最大     */
	VHAL_VPWR_STS_NEXT				/* 次変換表参照   */
};

/*****************************************************************************
 クラス名称：CVhalPropertyControlHdmi
 処理概要  ：プロパティ処理のHDMI制御を行う。
*****************************************************************************/
class CVhalPropertyControlHdmi final {
public:
	using CtrlTarget = uint32_t;
	static constexpr uint32_t CTRL_TARGET_FRONT{0U};	/* 前席HDMI制御 */
	static constexpr uint32_t CTRL_TARGET_REAR{1U};		/* 後席HDMI制御 */
	static constexpr uint32_t CTRL_TARGET_MAX{2U};

	static constexpr int32_t kUnknown{-1};				/* 未判別値 */

	CVhalPropertyControlHdmi(void) noexcept;
	~CVhalPropertyControlHdmi(void);
  	CVhalPropertyControlHdmi(const CVhalPropertyControlHdmi& src) = delete;
	CVhalPropertyControlHdmi& operator=(const CVhalPropertyControlHdmi& src) & = delete;
	CVhalPropertyControlHdmi(CVhalPropertyControlHdmi&& src) = delete;
	CVhalPropertyControlHdmi& operator=(CVhalPropertyControlHdmi&& src) & = delete;

	int32_t Initialize(CVhalSysdbControl* const p_sysdbctrl, CVhalHdcpAuthManager* const p_hdcp_auth_mng) noexcept;
	void Finalize(void);
	void ReInit(void) noexcept;

	bool Available(int32_t& connect, int32_t& format, uint32_t& width, uint32_t& height) const;
	int32_t Play(const CtrlTarget target, CVhalLayoutManager* const p_layout_mng, CVhalCaptureControl* const p_capture_control, const uint32_t width=0U, const uint32_t height=0U);
	void Stop(void);
	void Stop(const CtrlTarget target);
	bool IsPlaying(const CtrlTarget target) const;

private:
	std::unique_ptr<CVhalCaptureHdmi>	p_capture_hdmi_;
	CVhalSysdbControl				*p_sysdbctrl_;
	CVhalHdcpAuthManager			*p_hdcp_auth_mng_;
	CVhalLayoutManager				*p_layout_mng_;
	std::bitset<CTRL_TARGET_MAX>	is_play_;
};

/*****************************************************************************
 クラス名称：CVhalPropertyControlCamera
 処理概要  ：プロパティ処理のカメラ制御を行う。
*****************************************************************************/
class CVhalPropertyControlCamera final {
public:
	CVhalPropertyControlCamera(void) noexcept;
	~CVhalPropertyControlCamera(void);
  	CVhalPropertyControlCamera(const CVhalPropertyControlCamera& src) = delete;
	CVhalPropertyControlCamera& operator=(const CVhalPropertyControlCamera& src) & = delete;
	CVhalPropertyControlCamera(CVhalPropertyControlCamera&& src) = delete;
	CVhalPropertyControlCamera& operator=(CVhalPropertyControlCamera&& src) & = delete;

	int32_t Initialize(CVhalSysdbControl* const p_sysdbctrl, CVhalLayoutManager* const p_layout_mng, CVhalGpioControl* const p_gpioctrl);
	int32_t InitializeCaptureTabReceiver(CVhalCaptureTabReceiver* const p_capture_tab_receiver);
	int32_t InitializeMute(CVhalMute* const p_mute);
	void InitializeFileObserver(CVhalFileObserverControl* const p_fileob) noexcept;
	void Finalize(void);
	void ReInit(void) noexcept;

	/* カメラキャプチャwait状態設定 */
	void SetWaitingCapture(const bool status) noexcept
	{
		camera_cap_waiting_ = status;
	}
	/* カメラキャプチャwait状態取得 */
	bool GetWaitingCapture(void) const noexcept
	{
		return camera_cap_waiting_;
	}

	/* カメラキャプチャ設定 */
	int32_t SettingtCameraCapture(CVhalCaptureControl* const p_capture_control, const int64_t cameraType, const bool error_output);
	/* カメラキャプチャ設定解除 */
	void ReleaseCameraCapture(void);
	/* カメラパス設定 */
	int32_t SetCameraPath(const std::string& path);

	/* カメラ同期処理 */
	void ActionCameraSync(const VhalTabCamSync sync) const;

	/* カメラ電源状態処理 */
	void ActionCameraPowerState(const VehiclePowerState power_state);

	/* カメラ映像の入力サイズ設定 */
	void SetCameraInputSize(const int32_t disp);

	/* 映像キャプチャ動作設定(カメラ映像入力同期変更) */
	void ActionCameraControl(const VhalTabCamSync sync) const;
	/* 映像キャプチャ動作設定(車両電源ステート) */
	void ActionCameraControl(const VehiclePowerState power_state) const;
	/* 映像キャプチャ動作設定(カメラキャプチャ動作通知) */
	void ActionCameraControl(void) const;
	/* 映像キャプチャ準備状態通知2 */
	void SetCameraCaptureStandby2(const bool status);
	/* カメラキャプチャ表示準備 */
	void SetCameraCaptureFrame(void);
	/* カメラ映像権設定処理 */
	void ActionCameraCaptureStandby2(const bool status);
	/* カメラキャプチャ設定有無 */
	bool IsCameraCapture(void) const noexcept;

private:
	static constexpr uint32_t	kCameraCapStby2WaitCnt{3U};

	CVhalSysdbControl			*p_sysdbctrl_{nullptr};
	CVhalGpioControl			*p_gpioctrl_{nullptr};
	CVhalLayoutManager			*p_layout_mng_{nullptr};
	CVhalMute					*p_mute_{nullptr};
	std::unique_ptr<CVhalCaptureCamera>		p_capture_camera_{nullptr};
	CVhalCaptureTabReceiver		*p_capture_tab_receiver_{nullptr};
	CVhalFileObserverControl	*p_fileobserver_control_{nullptr};

	std::mutex	mtx_standby2_{};
	bool		camera_cap_stby2_{false};
	bool		camera_cap_waiting_{false};		/* 動画再生開始中によるキャプチャ設定Wait状態[true:Wait中][false:Wait解除] */
	VhalCaptureFrameStatus		camera_cap_frame_{VhalCaptureFrameStatus::kInitial};				/* カメラ表示準備状態 */
	uint32_t	camera_cap_stby2_wait_{0U};
	VehiclePowerState		vpower_state_{VehiclePowerState::VHAL_VPWR_STS_MAX};					/* 車両電源ステート */

	/* 映像キャプチャ動作設定 */
	void ActionCameraControl(const VhalTabCamSync sync, const VehiclePowerState power_state) const;
	/* カメラ映像権設定可否取得 */
	bool IsSetCameraCaptureStandby2(std::string& path);
};


/*****************************************************************************
 クラス名称：CVhalPropertyControl
 処理概要  ：プロパティ処理の制御を行う。
*****************************************************************************/
class CVhalPropertyControl final {
	friend CVhalPropertyControlOutputListener;
public:

	CVhalPropertyControl(void);
	~CVhalPropertyControl(void);
  	CVhalPropertyControl(const CVhalPropertyControl& src) = delete;
	CVhalPropertyControl& operator=(const CVhalPropertyControl& src) & = delete;
	CVhalPropertyControl(CVhalPropertyControl&& src) = delete;
	CVhalPropertyControl& operator=(CVhalPropertyControl&& src) & = delete;

	int32_t Initialize(CVhalMainControl * const p_main);
	void Finalize(void);

	/* プロパティ値の取得（整数型） */
	int32_t GetValueNumber(const std::string &name, int64_t &value);
	/* プロパティ値の取得（bool型） */
	int32_t GetValueBool(const std::string &name, bool &value);
	/* プロパティ値の取得（文字列） */
	int32_t GetValueString(const std::string &name, std::string &value);

	/* プロパティ値の設定（整数型） */
	int32_t SetValueNumber(const std::string &name, const int64_t &value);
	/* プロパティ値の設定（bool型） */
	int32_t SetValueBool(const std::string &name, const bool &value);
	/* プロパティ値の設定（文字列） */
	int32_t SetValueString(const std::string &name, const std::string &value);

	/* プロパティ値の設定完了 */
	void SetValueComplete(void);

	/* HDMI接続状態の更新 */
	void UpdateFilePropertyHdmi(const int32_t state);
	void UpdateSettingConnectedHdmiDevice(const int32_t state);

	/* HDMIビデオフォーマット切替 */
	void ChangeHdmiVideoFormat(const int32_t state);
	/* HDMIオーディオフォーマット切替 */
	void ChangeHdmiAudioFormat(const int32_t state);

	/* 映像パス設定可否判定状態Update（コールバック） */
	int32_t UpdateVpathStatusAvailable(const std::vector<std::string> &videopath_names);
	/* ディスプレイデバイス状態Update（コールバック） */
	int32_t UpdateSettingStatusDisplay(const int32_t screen_id, const bool display, const int32_t width, const int32_t height);
	/* 映像パス設定切替結果Update（コールバック） */
	void UpdateVpathStatusCurrentHdmi(const std::vector<std::string> &videopath_names, const VideoOutputTarget output_target);
	/* 画面キャプチャ結果Update（コールバック） */
	void UpdateScreenShotStatus(const int32_t result);
	/* サーフェスキャプチャ結果Update（コールバック） */
	void UpdateSurfaceScreenShotStatus(const int32_t result);
	/* HDCP first認証結果Update（コールバック） */
	void UpdateHdcpAuthResultRse(const CVhalHdcpAuthRsltData &hdcp_result);
	/* C-Disp HDCP first認証結果Update（コールバック） */
	void UpdateHdcpAuthResultCdisp(const CVhalHdcpAuthRsltData &hdcp_cdisp);
	/* HDCP認証情報クリアUpdate（コールバック） */
	void UpdateHdcpAuthClear(void);
	/* カメラ映像パス結果Update（コールバック） */
	void UpdateCameraPathStatus(const int32_t result);
	/* Instrument Clusterディスプレイデバイス状態Update（コールバック） */
	void UpdateIClusterDisplay(const bool result);
	/* MOST RSE映像パス切替結果Update（コールバック） */
	void UpdateMostRsePathStatus(const uint32_t result, const enum MostUpdateType update_type);
	/* MOST 後席ディスプレイ全体のMUTE結果Update（コールバック） */
	void UpdateMostMuteRearDispStatus(const bool mute, const enum MostUpdateType update_type);
	/* サービス起動状態の設定 */
	int32_t SetServiceState(const int32_t state);
	/* 画質調整テーブルのクリア */
	int32_t ClearColorStep(void);
	/* HDCP認証キー情報のクリア */
	int32_t ClearHdcpKey(void);
	
	/* 汎用動画再生結果格納関数 */
	void UpdateMovieStartResult(const int32_t result);
	/* キャプチャ再初期化処理 */
	void ResetCapture(const captureInputType input_type);
	/* カメラ映像キャプチャ設定 */
	int32_t OpenCameraCapture(const bool movie_check, const bool timer_start, bool& is_capture);
	
	/* 電源状態遷移時処理 24MM互換用 */
	void ChangePowerState(const ScenePowerState power_state);
	/* 車両電源状態遷移時処理 */
	void ChangeVehiclePowerState(const VehiclePowerState power_state);
	/* 車両電源状態変換処理 */
	void ConvertVehiclePowerState(const int32_t special, const int32_t basic, const int32_t appearance, VehiclePowerState& power) const;

	/* 画面キャプチャ応答設定 */
	void NotifyScreenShotMiconResponse(const VhalScreenShotResult result) noexcept;

	/* STR受付処理(PFMスレッド：サスペンド) */
	int32_t StrStatePfmSuspend(void);
	/* STR受付処理(PFMスレッド：レジューム) */
	void StrStatePfmResume(void);
	/* STRイベント処理 */
	void StrStateEvent(void);

	/* shutdown処理 */
	void Shutdown(void) const noexcept;

protected:

private:
	/* 各プロパティのデフォルト値 */
	static constexpr bool 	  kDefaultClippingEnable{false};
	static constexpr bool 	  kDefaultVisibility{false};
	static constexpr uint32_t kDefaultOpacity{100U};
	static constexpr uint32_t kDefaultWideMode{VHAL_WIDE_MODE_NORMAL};

	/* 画質調整STEPタイプ */
	using ColorStepType = uint32_t;
	static constexpr ColorStepType COLOR_STEP_TYPE_CONTRAST{0x01U};			/* コントラスト */
	static constexpr ColorStepType COLOR_STEP_TYPE_BRIGHTNESS{0x02U};		/* 明るさ */
	static constexpr ColorStepType COLOR_STEP_TYPE_ALL{(COLOR_STEP_TYPE_CONTRAST | COLOR_STEP_TYPE_BRIGHTNESS)};

	/* カメラ映像モード通知 */
	static constexpr uint8_t  kCameraVisualModeOther{0x00U};				/* カメラ以外 */
	static constexpr uint8_t  kCameraVisualModeCamera{0x01U};				/* カメラ */

	static constexpr const char* const			kPathEmpty{""};
	static constexpr const char* const			kPathHdmi{"HDMI"};

	CVhalMainControl *p_main_;
	std::unique_ptr<CVhalEventRoute> p_event_route_;
	std::vector<std::string> publish_order_property_;						/* 要素順に通知処理を行うプロパティ配列 */

	/* 各プロパティ固有のAction処理を行うクラス */
	std::unique_ptr<CVhalSysdbControl>           p_sysdbctrl_;
	std::unique_ptr<CVhalGpioControl>            p_gpioctrl_;
	std::unique_ptr<CVhalLayoutManager>          p_layout_mng_;
	std::unique_ptr<CVhalMute>                   p_mute_;
	std::unique_ptr<CVhalCaptureControl>         p_capture_control_;
	CVhalPropertyControlHdmi     hdmi_control_;
	CVhalPropertyControlCamera   camera_control_;

	std::unique_ptr<wlrenderer::CWaylandRenderer>		p_renderer_;
	std::unique_ptr<CVhalMiconCommControl>				p_micon_comm_control_;
	std::unique_ptr<CVhalColorManager>					p_color_mng_;
	std::unique_ptr<CVhalPropertyControlOutputListener>	p_output_listener_;
	std::unique_ptr<CVhalPropertyControlEventListener>	p_propertyControl_listener_;
	std::unique_ptr<CVhalMiconMiscReceiver>				p_micon_misc_receiver_;
	std::unique_ptr<CVhalCaptureTabReceiver>			p_capture_tab_receiver_;
	std::unique_ptr<CVhalCameraModeSendItem>			p_camera_mode_send_item_;
	std::unique_ptr<CVhalMovieControl>					p_movie_control_;
	std::unique_ptr<CVhalFileObserverControl>			p_fileobserver_control_;
	std::unique_ptr<CVhalScreenShotMicon>				p_screen_shot_micon_;
	std::unique_ptr<CVhalScreenShot>					p_screen_shot_;
	std::unique_ptr<CVhalMiconMostVideoInfo>			p_micon_most_video_info_;
	std::unique_ptr<CVhalObserverControl>				p_observer_control_;
	std::unique_ptr<CVhalHdcpAuthManager>				p_hdcp_auth_mng_;
	std::unique_ptr<CVhalHdcpMiscReceiver>				p_hdcp_misc_receiver_;
	std::unique_ptr<CVhalColorAdjustMiscReceiver>		p_color_adjust_misc_receiver_;
	std::unique_ptr<CVhalEnvironmentManager>			p_environment_mng_;
	std::unique_ptr<CVhalScreenShotReceiver>			p_screenshot_receiver_;
	std::unique_ptr<CVhalHudScreenReceiver>				p_hud_screen_receiver_;
	std::unique_ptr<CVhalHudScreenController> 			p_hud_screen_controller_;

	std::unordered_map<std::string, struct VhalPropertyEntry> property_entries_;
	static struct VhalRectanglePropNames front_output_control_;
	static struct VhalRectanglePropNames front_output_status_;
	static struct VhalRectanglePropNames rear_output_control_;
	static struct VhalRectanglePropNames rear_output_status_;
	static struct VhalRectanglePropNames camera_output_control_;
	static struct VhalRectanglePropNames camera_output_status_;
	static struct VhalRectanglePropNames icluster_output_control_;
	static struct VhalRectanglePropNames icluster_output_status_;
	static struct VhalClippingPropNames front_clipping_control_;
	static struct VhalClippingPropNames front_clipping_status_;
	static struct VhalClippingPropNames icluster_clipping_control_;
	static struct VhalClippingPropNames icluster_clipping_status_;
	static struct VhalRectanglePropNames front_heacon_control_;
	static struct VhalRectanglePropNames front_heacon_status_;

	/* プロパティエントリーの取得 */
	struct VhalPropertyEntry *GetPropertyEntry(const std::string &name, const uint32_t type, const uint32_t attr);

	/* 前席映像パス切替機能Action登録関数 */
	int32_t ActionVpathFrontCurrent(const struct VhalPropertyEntry * const p_entry);
	/* 後席映像パス切替機能Action登録関数 */
	int32_t ActionVpathRearCurrent(const struct VhalPropertyEntry * const p_entry);
	/* カメラ映像パス切替機能Action登録関数 */
	int32_t ActionVpathCameraCurrent(const struct VhalPropertyEntry * const p_entry);
	/* Instrument Cluster パス切替機能Action登録関数 */
	int32_t ActionVpathIClusterCurrent(const struct VhalPropertyEntry * const p_entry);

	/* 前席映像パス設定可否判定Generate登録関数 */
	int32_t GenerateVpathFrontAvailable(struct VhalPropertyEntry * const p_entry);
	/* 後席映像パス設定可否判定Generate登録関数 */
	int32_t GenerateVpathRearAvailable(struct VhalPropertyEntry * const p_entry);
	/* カメラ映像パス設定可否判定Generate登録関数 */
	int32_t GenerateVpathCameraAvailable(struct VhalPropertyEntry * const p_entry);
	/* Instrument Cluster 映像パス設定可否判定Generate登録関数 */
	int32_t GenerateVpathIClusterAvailable(struct VhalPropertyEntry * const p_entry);

	/* 前席映像出力サイズ設定機能Action登録関数 */
	int32_t ActionVpathFrontOutput(struct VhalPropertyEntry * const p_entry);
	/* 後席映像出力サイズ設定機能Action登録関数 */
	int32_t ActionVpathRearOutput(struct VhalPropertyEntry * const p_entry);
	/* カメラ映像出力サイズ設定機能Action登録関数 */
	int32_t ActionVpathCameraOutput(struct VhalPropertyEntry * const p_entry);
	/* Instrument Cluster 出力サイズ設定機能Action登録関数 */
	int32_t ActionVpathIClusterOutput(struct VhalPropertyEntry * const p_entry);

	/* 出力サイズプロパティ共通処理 */
	int32_t UpdateVpathOutput(const struct VhalRectanglePropNames &prop_name, const struct VhalRectanglePropValues &current);
	/* 出力サイズプロパティ取得処理 */
	int32_t GetVpathOutputControl(const struct VhalRectanglePropNames &control, struct VhalRectanglePropValues &current, struct VhalRectanglePropValues &prev);

	/* 後席専用モード向け映像パス切替機能Action登録関数 */
	int32_t ActionVpathRearRseDisp(const struct VhalPropertyEntry * const p_entry);
	/* 後席専用映像反映通知Action登録関数 */
	int32_t ActionVpathRearRseNotify(struct VhalPropertyEntry * const p_entry);

	/* 前席映像のクリッピング機能Action登録関数 */
	int32_t ActionVpathFrontClipping(struct VhalPropertyEntry * const p_entry);
	/* InstrumentCluster映像のクリッピング機能Action登録関数 */
	int32_t ActionVpathIClusterClipping(struct VhalPropertyEntry * const p_entry);
	/* クリッピングプロパティ更新処理 */
	int32_t UpdateVpathClipping(const struct VhalClippingPropNames &prop_name, const struct VhalClippingPropValues &current);
	/* クリッピングプロパティ取得処理 */
	int32_t GetVpathClippingControl(const struct VhalClippingPropNames &control, struct VhalClippingPropValues &current, struct VhalClippingPropValues &prev);

	/* 前席映像のワイド設定機能Action登録関数 */
	int32_t ActionVpathFrontWide(const struct VhalPropertyEntry * const p_entry);
	/* 後席映像のワイド設定機能Action登録関数 */
	int32_t ActionVpathRearWide(const struct VhalPropertyEntry * const p_entry);

	/* 前席映像の可視状態設定機能Action登録関数 */
	int32_t ActionVpathFrontVisible(const struct VhalPropertyEntry * const p_entry);
	/* 前席映像の可視状態取得機能Generate登録関数 */	
	int32_t GenerateFrontVisible(struct VhalPropertyEntry * const p_entry);
	/* 後席映像の可視状態設定機能Action登録関数 */
	int32_t ActionVpathRearVisible(const struct VhalPropertyEntry * const p_entry);
	/* 後席映像の可視状態取得機能Generate登録関数 */	
	int32_t GenerateRearVisible(struct VhalPropertyEntry * const p_entry);
	/* Instrument Clusterの可視状態設定機能Action登録関数 */
	int32_t ActionVpathIClusterVisible(const struct VhalPropertyEntry * const p_entry);
	/* Instrument Clusterの可視状態取得機能Generate登録関数 */	
	int32_t GenerateIClusterVisible(struct VhalPropertyEntry * const p_entry);

	/* 前席映像の不透明度設定機能Action登録関数 */
	int32_t ActionVpathFrontOpacity(const struct VhalPropertyEntry * const p_entry);
	/* 後席映像の不透明度設定機能Action登録関数 */
	int32_t ActionVpathRearOpacity(const struct VhalPropertyEntry * const p_entry);
	/* Instrument Clusterの不透明度設定機能Action登録関数 */
	int32_t ActionVpathIClusterOpacity(const struct VhalPropertyEntry * const p_entry);

	/* 映像ソース設定機能Action登録関数 */
	int32_t ActionVsrcFrontId(const struct VhalPropertyEntry * const p_entry);

	/* 画質調整機能（明るさStep設定要求）Action登録関数 */
	int32_t ActionVsrcFrontBright(const struct VhalPropertyEntry * const p_entry);
	/* 画質調整機能（コントラストStep設定要求）Action登録関数 */
	int32_t ActionVsrcFrontContrast(const struct VhalPropertyEntry * const p_entry);
	/* 画質調整機能（別体ディスプレイへの画質調整値更新要求）Action登録関数 */
	static int32_t ActionVsrcImgAdjUpdate(struct VhalPropertyEntry * const p_entry);
	/* 画質調整機能（強制HMI画質適用設定）Action登録関数 */
	int32_t ActionVsrcForceHmiImgAdj(const struct VhalPropertyEntry * const p_entry);
	/* 画質調整機能（強制多感覚連携画質適用設定）Action登録関数 */
	int32_t ActionVsrcForceMultisensoryImgAdj(const struct VhalPropertyEntry * const p_entry);

	/* MUTE機能（前席ディスプレイ全体のMUTE要求）Action登録関数 */
	int32_t ActionMuteFrontDisp(const struct VhalPropertyEntry * const p_entry);
	/* MUTE機能（前席映像面のMUTE要求）Action登録関数 */
	int32_t ActionMuteFrontVideo(const struct VhalPropertyEntry * const p_entry);
	/* MUTE機能（前席バックライト設定変更要求）Action登録関数 */
	int32_t ActionMuteFrontBacklight(const struct VhalPropertyEntry * const p_entry);
	/* MUTE機能（後席ディスプレイ全体のMUTE要求）Action登録関数 */
	int32_t ActionMuteRearDisp(const struct VhalPropertyEntry * const p_entry);
	/* MUTE機能（後席映像面のMUTE要求）Action登録関数 */
	int32_t ActionMuteRearVideo(const struct VhalPropertyEntry * const p_entry);

	/* HDCP認証機能（HDCP(RSE) first認証開始要求）Action登録関数 */
	int32_t ActionHdcpAuthRse(struct VhalPropertyEntry * const p_entry);
	/* HDCP認証機能（HDCP(別体ディスプレイ) first認証開始要求）Action登録関数 */
	static int32_t ActionHdcpAuthSeparateDisp(struct VhalPropertyEntry * const p_entry);
	/* HDCP認証機能（HDCP(Instrument Cluster) first認証開始要求）Action登録関数 */
	static int32_t ActionHdcpAuthICluster(struct VhalPropertyEntry * const p_entry);

	/* iviレイアウト情報設定機能Action登録関数 */
	int32_t ActionIviLayoutConfFile(const struct VhalPropertyEntry * const p_entry);
	/* iviプロパティ取得機能（スクリーン）Generate登録関数 */
	int32_t GenerateIviScreenInfo(struct VhalPropertyEntry * const p_entry);
	/* iviプロパティ取得機能（レイヤー）Generate登録関数 */
	int32_t GenerateIviLayerInfo(struct VhalPropertyEntry * const p_entry);
	/* iviプロパティ取得機能（サーフェス）Generate登録関数 */
	int32_t GenerateIviSurfaceInfo(struct VhalPropertyEntry * const p_entry);

	/* iviレイヤー優先度変更機能Action登録関数 */
	int32_t ActionIviLayerOrder(struct VhalPropertyEntry * const p_entry);
	/* iviレイヤー優先度変更機能Generate登録関数 */
	int32_t GenerateIviLayerOrder(struct VhalPropertyEntry * const p_entry);

	/* クエリー文字列のパース */
	static int32_t ParseQueryString(const std::string &query, std::map<std::string, std::string> &params);

	/* 画面キャプチャ取得機能Action登録関数 */
	int32_t ActionScreenCapture(struct VhalPropertyEntry * const p_entry);
	/* サーフェスキャプチャ取得機能Action登録関数 */
	int32_t ActionSurfaceScreenCapture(struct VhalPropertyEntry * const p_entry);

	/* 汎用動画再生機能（動画再生パラメータ設定）Action登録関数 */
	int32_t ActionMovieFrontPrepare(struct VhalPropertyEntry * const p_entry);
	/* 汎用動画再生機能（設定した動画再生パラメータクリア）Action登録関数 */
	int32_t ActionMovieFrontClear(struct VhalPropertyEntry * const p_entry);
	/* 汎用動画再生機能（動画再生開始）Action登録関数 */
	int32_t ActionMovieFrontStart(struct VhalPropertyEntry * const p_entry);
	/* 汎用動画再生機能（動画再生中止）Action登録関数 */
	int32_t ActionMovieFrontCancel(struct VhalPropertyEntry * const p_entry);

	/* VideoHAL動作設定機能（車両ディスプレイ情報）Action登録関数 */
	int32_t ActionSettingDisplayType(const struct VhalPropertyEntry * const p_entry);
	/* VideoHAL動作設定機能（接続されているカメラ種別）Action登録関数 */
	int32_t ActionSettingConnectedCamera(const struct VhalPropertyEntry * const p_entry);
	/* VideoHAL動作設定機能（接続されているRSE種別）Action登録関数 */
	int32_t ActionSettingConnectedRSE(const struct VhalPropertyEntry * const p_entry);
	/* VideoHAL動作設定機能（接続されている別体ディスプレイ種別）Action登録関数 */
	int32_t ActionSettingConnectedSeparateDisp(const struct VhalPropertyEntry * const p_entry);
	/* VideoHAL動作設定機能（昼夜モード）Action登録関数 */
	int32_t ActionSettingDayNight(const struct VhalPropertyEntry * const p_entry);
	/* VideoHAL動作設定機能（テーマカラー）Action登録関数 */
	int32_t ActionSettingThemeColor(const struct VhalPropertyEntry * const p_entry);

	/* ヒーコンエリアサイズ機能（ヒーコンエリアサイズ設定）Action登録関数 */
	int32_t ActionHeaconAreaSize(struct VhalPropertyEntry * const p_entry);
	/* 短形設定プロパティ取得処理（共通） */
	int32_t GetRectangleControlCommon(const struct VhalRectanglePropNames &control_prop, const struct VhalRectanglePropNames &status_prop, struct VhalRectanglePropValues &current_values, struct VhalRectanglePropValues &prev_values);
	/* 短形状態プロパティ更新処理（共通） */
	int32_t UpdateRectangleStatusCommon(const struct VhalRectanglePropNames &status_prop, struct VhalRectanglePropValues &current_values);

	/* 画質調整STEP 状態プロパティ更新 */
	int32_t UpdateStatusColorStep(const ColorStepType type, const bool update_notify=true);
	/* ワイド設定 状態プロパティ更新 */
	int32_t UpdateStatusWideMode(const VideoOutputTarget output_target);
	/* 可視状態設定 プロパティ更新 */
	int32_t UpdateStatusVisible(const std::string &video_path, const VideoOutputTarget output_target, const bool update_notify);

	/* 映像パス設定可否チェック */
	bool IsValidVideoPathAvailable(const std::string &video_path, const VideoOutputTarget output_target);

	/* カメラ映像設定処理 */
	int32_t SettingCameraCapture(void);

	/* HDMI映像キャプチャ制御更新 */
	int32_t UpdateHdmiCaptureControl(const int64_t status, const uint32_t width, const uint32_t height);
	/* HDMI映像再生開始処理 */
	int32_t PlayHdmiCapture(const uint32_t width, const uint32_t height);

	/* ディスプレイ解像度更新 */
	void UpdateDisplayResolution(const std::string &name_width, const std::string &name_height, const int32_t value_width, const int32_t value_height);

	/* 出力サイズ設定値チェック */
	bool IsValidOutputSize(const int32_t screen_id, const std::vector<int32_t> &rect) const;

	/* HDMI接続状態更新処理 */
	void ChgConnInfoHdmi(const int32_t state, const bool initialized);

	/* HDCP認証キー書込み */
	int32_t SetHdcpAuthKey(const int32_t hdmi_state);

	/* STRサスペンド処理 */
	void StrSuspend(void);

	/* STRレジューム処理 */
	int32_t StrResume(void);

	/* STRサスペンド処理(カメラ映像パスクリア) */
	int32_t StrSuspendPathClearCamera(void);
	/* STRサスペンド処理(前席映像パスクリア) */
	int32_t StrSuspendPathClearFront(void);
	/* STRサスペンド処理(後席映像パスクリア) */
	int32_t StrSuspendPathClearRear(void);
	/* STRサスペンド処理(IC映像パスクリア) */
	int32_t StrSuspendPathClearICluster(void);
	/* 各リソース再初期化処理 */
	void ReInit(void) noexcept;

	/* プロパティ初期値設定 */
	void InitializeProperties(const std::unordered_map<std::string, int64_t>&     tbl_num, 
							  const std::unordered_map<std::string, bool>&        tbl_bool,
							  const std::unordered_map<std::string, std::string>& tbl_str, 
							  const std::vector<std::string>& status_quo);

	/* カメラ種別判別要求送信 */
	int32_t SendCameraType(void) const;

	/* カメラ映像モード 定期送信 */
	int32_t SendIntervalCameraMode(const bool create_flag);

};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_PROPERTY_CONTROL_H */
