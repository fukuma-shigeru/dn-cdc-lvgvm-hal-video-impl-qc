/*******************************************************************************
    機能名称    ：  キャプチャ制御モジュール
    ファイル名称：  vhal_capture_control.h
*******************************************************************************/
#ifndef	VHAL_CAPTURE_CONTROL_H
#define	VHAL_CAPTURE_CONTROL_H

#include <string>
#include <thread>
#include <sys/epoll.h>

#include <algorithm>
#include <vector>
#include <unordered_map>

#include "qcarcam.h"
#include "wl_renderer_public.h"
#include "vhal_hdcp_auth_control.h"
#include "vhal_observer_client.h"
#include "vhal_observer_control.h"
#include "vhal_event_route.h"
#include "vhal_timer.h"

//#define VHAL_SUPPORT_FRAME_CNT

namespace videohal
{
class CVhalMainControl;
class CVhalLayoutManager;
class CVhalCaptureDraw;
class CVhalPropertyControl;
class CVhalCaptureControl;

/* Capture Input Type */
enum class captureInputType : uint8_t {
  VHAL_CAPTURE_INPUT_CAMERA,
  VHAL_CAPTURE_INPUT_HDMI,
  VHAL_CAPTURE_INPUT_CAMERA_DUMMY,
  VHAL_CAPTURE_INPUT_MAX
};

/* Capture Event Type */
enum class captureEventType : uint8_t {
  VHAL_CAPTURE_EVENT_FRAME_CAPTURE,
  VHAL_CAPTURE_EVENT_FRAME_RELEASE,
  VHAL_CAPTURE_EVENT_MAX
};

/* イベントパラメータ構造体 */
struct event_param_t {
	captureInputType	input_type;
	captureEventType	event_type;
	int32_t				get_buf_indx;
	uint32_t			release_buf_indx;
};

/*****************************************************************************
 クラス名称：CVhalCaptureInterruptObserver
 処理概要  ：キャプチャ割り込み監視クラス
*****************************************************************************/
class CVhalCaptureInterruptObserver : public CVhalObserverClient {
public:
	CVhalCaptureInterruptObserver(void) = default;
	~CVhalCaptureInterruptObserver(void) override;
	CVhalCaptureInterruptObserver(const CVhalCaptureInterruptObserver& src) = delete;
	CVhalCaptureInterruptObserver& operator=(const CVhalCaptureInterruptObserver& src) & = delete;
	CVhalCaptureInterruptObserver(CVhalCaptureInterruptObserver&& src) = delete;
	CVhalCaptureInterruptObserver& operator=(CVhalCaptureInterruptObserver&& src) & = delete;

	void Initialize(CVhalCaptureControl* const p_cap_control, const captureInputType cap_input_type) noexcept;
	void Finalize(void) noexcept;

	/* キャプチャ割り込み監視周期コールバック */
	int32_t Notify(void) override;
private:
	CVhalCaptureControl*	p_capctrl_{nullptr};
	captureInputType 		input_type_{captureInputType::VHAL_CAPTURE_INPUT_MAX};
};


/* バッファ管理構造体 */
struct capturecontrol_bufinfo_t {
	captureInputType							input_type;
	uint32_t									frame_buf_indx;
	wlrenderer::CWaylandRendererVideoBuffer*	p_renderer_buffer;
};

/* キャプチャコントロール管理構造体 */
struct capturecontrol_info_t {
public:
	capturecontrol_info_t(const captureInputType input_type) noexcept
	:p_renderer_config_(nullptr)
	,p_renderer_video_(nullptr)
	,p_black_buffer_(nullptr)
	,input_type_(input_type)
	,p_capture_draw_(nullptr)
	,hndl_(QCARCAM_HNDL_INVALID)
	,buf_cnt_(0U)
	,p_buf_info_(nullptr)
	,qcarcam_start_(false)
	,mtx_qcarcam_{}
	,capture_send_(false)
	,capture_chk_(false)
	,inter_observer_(nullptr)
	,inter_heartbeat_(false)
	,mtx_inter_heartbeat_{}
	,reset_event_send_(false)
	,submit_request_id_(0U)
	{
	}

	void SetRendererVideo(wlrenderer::CWaylandRendererVideo* const p_renderer_video) noexcept
	{
		p_renderer_video_ = p_renderer_video;
	}

	void SetBlackBuffer(wlrenderer::CWaylandRendererVideoBuffer* const p_black_buffer) noexcept
	{
		p_black_buffer_ = p_black_buffer;
	}

	void SetHndl(const QCarCamHndl_t hndl) noexcept
	{
		hndl_ = hndl;
	}

	void SetBufCnt(const uint32_t buf_cnt)  noexcept
	{
		buf_cnt_ = buf_cnt;
	}

	void SetBufInfo(capturecontrol_bufinfo_t* const p_buf_info) noexcept
	{
		p_buf_info_ = p_buf_info;
	}

	void SetQcarcamStart(const bool qcarcam_start) noexcept
	{
		qcarcam_start_ = qcarcam_start;
	}

	void SetCaptureSend(const bool capture_send) noexcept
	{
		capture_send_ = capture_send;
	}

	void SetCaptureChk(const bool capture_chk) noexcept
	{
		capture_chk_ = capture_chk;
	}

	void SetHeartBeat(const bool inter_heartbeat) noexcept
	{
		inter_heartbeat_ = inter_heartbeat;
	}

	void SetCaptureResetEvent(const bool reset_event_send) noexcept
	{
		reset_event_send_ = reset_event_send;
	}

	wlrenderer::CWaylandRendererConfig* GetRendererConfig(void) const noexcept
	{
		return p_renderer_config_.get();
	}

	wlrenderer::CWaylandRendererVideo* GetRendererVideo(void) const noexcept
	{
		return p_renderer_video_;
	}

	wlrenderer::CWaylandRendererVideoBuffer* GetBlackBuffer(void) const noexcept
	{
		return p_black_buffer_;
	}

	captureInputType GetInputType(void) const noexcept
	{
		return input_type_;
	}

	CVhalCaptureDraw* GetCaptureDraw(void) const noexcept
	{
		return p_capture_draw_.get();
	}

	QCarCamHndl_t GetHndl(void) const noexcept
	{
		return hndl_;
	}

	uint32_t GetBufCnt(void) const noexcept
	{
		return buf_cnt_;
	}

	capturecontrol_bufinfo_t* GetBufInfo(void) const noexcept
	{
		return p_buf_info_;
	}

	bool GetQcarcamStart(void) const noexcept
	{
		return qcarcam_start_;
	}

	bool GetCaptureSend(void) const noexcept
	{
		return capture_send_;
	}

	bool GetCaptureChk(void) const noexcept
	{
		return capture_chk_;
	}

	bool GetHeartBeat(void) const noexcept
	{
		return inter_heartbeat_;
	}

	bool GetCaptureResetEvent(void) const noexcept
	{
		return reset_event_send_;
	}

	uint32_t GetSubmitRequestId(void) noexcept
	{
		/* オーバーフロー許可: 0xFFFFFFFF → 0 */
		return ++submit_request_id_;
	}

	void AllocateCaptureDraw(void)
	{
		p_capture_draw_ = std::make_unique<CVhalCaptureDraw>();
	}

	void FreeCaptureDraw(void) noexcept
	{
		p_capture_draw_ = nullptr;
	}

	void AllocateRendererConfig(void)
	{
		p_renderer_config_ = std::make_unique<wlrenderer::CWaylandRendererConfig>();
	}

	void FreeRenderConfig(void) noexcept
	{
		p_renderer_config_ = nullptr;
	}

	void LockMtxQcarcam(void)
	{
		mtx_qcarcam_.lock();
	}

	void UnLockMtxQcarcam(void)
	{
		mtx_qcarcam_.unlock();
	}

	void AllocateObserver(void)
	{
		inter_observer_ = std::make_unique<CVhalCaptureInterruptObserver>();
	}

	CVhalCaptureInterruptObserver* GetObserver(void) const noexcept
	{
		return inter_observer_.get();
	}

	void FreeObserver(void) noexcept
	{
		inter_observer_ = nullptr;
	}

	void LockMtxHeartBeat(void)
	{
		mtx_inter_heartbeat_.lock();
	}

	void UnLockMtxHeartBeat(void)
	{
		mtx_inter_heartbeat_.unlock();
	}

private:
	std::unique_ptr<wlrenderer::CWaylandRendererConfig>	p_renderer_config_;
	wlrenderer::CWaylandRendererVideo*	p_renderer_video_;
	wlrenderer::CWaylandRendererVideoBuffer*	p_black_buffer_;		/* 黒画用 */
	captureInputType			input_type_;
	std::unique_ptr<CVhalCaptureDraw>	p_capture_draw_;
	QCarCamHndl_t				hndl_;
	uint32_t					buf_cnt_;
	capturecontrol_bufinfo_t*	p_buf_info_;
	bool						qcarcam_start_;
	mutable std::mutex			mtx_qcarcam_;
	bool						capture_send_;
	bool						capture_chk_;
	std::unique_ptr<CVhalCaptureInterruptObserver>	inter_observer_;
	bool						inter_heartbeat_;			/* false:キャプチャ割り込み生存確認なし true:キャプチャ割り込み生存確認あり */
 	mutable std::mutex 			mtx_inter_heartbeat_;		/* inter_heartbeatの排他mutex */
	bool						reset_event_send_;			/* false:再起動エラーイベント発行なし true:再起動エラーイベント発行あり */
 	uint32_t					submit_request_id_;			/* SubmitRequest実施回数 */
};

/* デバイス管理構造体 */
struct VhalCaptureDeviceEntry{
	/* デバイスごとの固定パラメータは以下に追加する */
	const char*				inputName;
	int32_t					format;
	int32_t					buf_count;
	int32_t					color_space;
};

class CVhalCaptureDraw final {
public:
	CVhalCaptureDraw(void);
	~CVhalCaptureDraw(void);
	CVhalCaptureDraw(const CVhalCaptureDraw& src) = delete;
	CVhalCaptureDraw& operator=(const CVhalCaptureDraw& src) & = delete;
	CVhalCaptureDraw(CVhalCaptureDraw&& src) = delete;
	CVhalCaptureDraw& operator=(CVhalCaptureDraw&& src) & = delete;

	int32_t Initialize(void);
	void Finalize(void);

	/* イベントアイテムの書き込み */
	int32_t WriteEvent(const event_param_t event_param) const;

private:
	static constexpr int32_t	kSocketPairRead{0};
	static constexpr int32_t	kSocketPairWrite{1};

	static constexpr uint32_t	kSourceEventReadable{0x01U};
	static constexpr uint32_t	kSourceEventWritable{0x02U};
	static constexpr uint32_t	kSourceEventHungup{0x04U};
	static constexpr uint32_t	kSourceEventError{0x08U};

	bool			running_;
	int32_t			epoll_fd_;
	int32_t			pair_fd_[2];	/* socketpairで生成したfdふたつ */
	std::unique_ptr<std::thread>	p_draw_thread_;

	int32_t ExecEvent(const uint32_t source_event);
	void DrawThreadMain(void);
};

/*****************************************************************************
 クラス名称：CVhalCaptureReceiveEventListenerBase
 処理概要  ：VhalCaptureReceiveイベントリスナベース。
*****************************************************************************/
class CVhalCaptureReceiveEventListenerBase {
public:
	CVhalCaptureReceiveEventListenerBase(void) noexcept = default;
	virtual ~CVhalCaptureReceiveEventListenerBase(void) = default;
  	CVhalCaptureReceiveEventListenerBase(const CVhalCaptureReceiveEventListenerBase& src) = delete;
	CVhalCaptureReceiveEventListenerBase(CVhalCaptureReceiveEventListenerBase&& src) = delete;


	virtual void NotifyReceiveCameraControl(void) const noexcept = 0;
	virtual int32_t NotifyReceiveOpenCameraCapture(const bool movie_check, bool& is_capture) const noexcept = 0;
	virtual void NotifyReceiveCaptureStandby2(const captureInputType input_type, const bool status) const noexcept = 0;

	virtual void NotifyReceiveCaptureFrame(void) const noexcept = 0;
private:
	CVhalCaptureReceiveEventListenerBase& operator=(const CVhalCaptureReceiveEventListenerBase& src) & = delete;
	CVhalCaptureReceiveEventListenerBase& operator=(CVhalCaptureReceiveEventListenerBase&& src) & = delete;
};

/*****************************************************************************
 クラス名称：CVhalOpenCaptureTimer
 処理概要  ：キャプチャ設定指示送信用タイマ処理
*****************************************************************************/
class CVhalOpenCaptureTimer : public CVhalTimer {
public:

	CVhalOpenCaptureTimer(CVhalCaptureReceiveEventListenerBase* const p_captureRecv_listener, const captureInputType input_type) noexcept;
	~CVhalOpenCaptureTimer(void) override = default;
  	CVhalOpenCaptureTimer(const CVhalOpenCaptureTimer& src) = delete;
	CVhalOpenCaptureTimer& operator=(const CVhalOpenCaptureTimer& src) & = delete;
	CVhalOpenCaptureTimer(CVhalOpenCaptureTimer&& src) = delete;
	CVhalOpenCaptureTimer& operator=(CVhalOpenCaptureTimer&& src) & = delete;

private:
	/* タイマー満了実装 */
	int32_t OnTimerImpl(const bool time_cycle_enable) const override;

	CVhalCaptureReceiveEventListenerBase* p_captureRecv_listener_;
	captureInputType input_type_;
};

/*****************************************************************************
 クラス名称：CVhalCaptureControl
 処理概要  ：キャプチャの制御を行う。
*****************************************************************************/
class CVhalCaptureControl {
public:
	CVhalCaptureControl(void);
	virtual ~CVhalCaptureControl(void);
  	CVhalCaptureControl(const CVhalCaptureControl& src) = delete;
	CVhalCaptureControl& operator=(const CVhalCaptureControl& src) & = delete;
	CVhalCaptureControl(CVhalCaptureControl&& src) = delete;
	CVhalCaptureControl& operator=(CVhalCaptureControl&& src) & = delete;

	int32_t Initialize(wlrenderer::CWaylandRenderer * const p_wayland_renderer, CVhalMainControl * const p_main_ctl, CVhalPropertyControl * const p_prop_ctl, CVhalObserverControl * const p_observer_ctl);
	void Finalize(void);

	/* QCarCam終了処理 */
	void FinalizeQcarcam(void);

	/* キャプチャ設定 */
	int32_t OpenCapture(CVhalLayoutManager * const p_layout_mng, const captureInputType input_type, const uint32_t res_width=0U, const uint32_t res_height=0U);
	/* キャプチャ設定解除 */
	void CloseCapture(const captureInputType input_type);
	/* キャプチャ開始 */
	int32_t StartCapture(const captureInputType input_type) noexcept;
	/* キャプチャ停止 */
	int32_t StopCapture(const captureInputType input_type, const bool setting_black_buf=true) noexcept;
	/* キャプチャ動作取得 */
	bool IsCaptureStatus(const captureInputType input_type) const noexcept;

	/* 映像更新開始 */
	int32_t Start(const captureInputType input_type);
	/* 映像更新停止 */
	int32_t Stop(const captureInputType input_type);

	/* キャプチャフレーム送信 */
	int32_t SendFrame(const captureInputType input_type, const int32_t get_buf_indx);

	/* キャプチャイベント通知 */
	void NotifyCaptureEvent(const QCarCamHndl_t hndl, const int32_t get_buf_indx);

	/* HDCP認証の後段機器情報更新 BEVstep3では不要 */
//	void SetHdcpAuthResultRse(const CVhalHdcpAuthRsltData &hdcp_result);

	/* イベントリスナー登録 */
	int32_t RegisterEventListener(CVhalCaptureReceiveEventListenerBase* const p_listener);
	
	/* バッファ取得時異常判定 BEVstep3では不要 */
//	void CheckGetBufferError(const QCarCamHndl_t hndl, const QCarCamRet_e qret, const uint32_t event_id);
	/* バッファ解放時異常判定 BEVstep3では不要 */
//	void CheckReleaseBufferError(const QCarCamHndl_t hndl, const QCarCamRet_e qret);
	/* QCarCamフレームリクエスト送信準備 */
	void SettingSubmitRequestPrev(const QCarCamHndl_t hndl, const uint32_t buf_index, QCarCamRet_e& qret);
	/* HDMI-Rxデバイス異常判定 */
	void CheckHdmiReceiverError(const QCarCamHndl_t hndl);
	/* キャプチャ割り込み監視開始 */
	void StartInterruptObserver(capturecontrol_info_t* const p_capture_ctrl_info);
	/* キャプチャ割り込み監視停止 */
	void StopInterruptObserver(capturecontrol_info_t* const p_capture_ctrl_info);
	/* キャプチャ割り込み発生通知 */
	void NotifyInterruptHeartbeat(const QCarCamHndl_t hndl);
	/* キャプチャ割り込み受信有無確認 */
	bool CheckInterruptHeartbeat(const captureInputType input_type);
	/* キャプチャ設定(遅延処理) */
	int32_t OpenCaptureDelayProcess(const captureInputType input_type, const uint32_t interval, const uint32_t cycle_count);
	/* QCarCamパラメータ処理(HDCP認証キー書込み) */
	int32_t SetHdcpAuthKey(const CVhalHdcpAuthRsltData& hdcp_cdisp, const CVhalHdcpAuthRsltData& hdcp_rse);
	/* HDCP認証キー書込み済フラグ取得 */
	bool GetHdcpAuthKeyWriteStatus(void) const noexcept;
	/* HDCP認証キー書込み済フラグクリア */
	void ClearHdcpAuthKeyWriteStatus(void) noexcept;

private:

	/* サーフェスID管理構造体 */
	struct VhalCameraIviId {
		int32_t front;
		int32_t rear;
	};
	/* キャプチャリセット要因 */
	enum class CaptureResetFactor : int32_t {
		kInterruptNothing = 2,
		kHdmiReceiverError
	};

	static std::unordered_map<captureInputType, struct VhalCaptureDeviceEntry> device_entries_;
	std::vector<std::unique_ptr<capturecontrol_info_t>>	p_capture_ctrl_infos_{};

	CVhalMainControl*						p_main_{nullptr};
	wlrenderer::CWaylandRenderer*			p_renderer_{nullptr};
	CVhalCaptureReceiveEventListenerBase*	p_captureRecv_listener_{nullptr};
	bool									initialized_{false};
	bool									initialized_qcarcam_{false};
	bool									sethdcpauthkey_{false};
	CVhalPropertyControl*					p_prop_{nullptr};
	CVhalObserverControl*					p_observer_control_{nullptr};
	std::unique_ptr<CVhalEventRoute>		p_route_;


	/* <デバイス種別, キャプチャ設定用タイマ> */
	using COpenCaptureTimerList = std::map<const captureInputType, std::unique_ptr<CVhalOpenCaptureTimer>>;
	COpenCaptureTimerList					open_capture_timer_list_;

	/* キャプチャ設定解除(リソース解放) */
	void ReleaseProcess(capturecontrol_info_t * const p_capture_ctrl_info);
	/* QCarCam初期化処理 */
	int32_t InitializeQcarcam(void);
	/* QCarCam入力情報処理 */
	int32_t SettingQueryInput(const char* inputName, QCarCamInput_t * const p_query_input, QCarCamInputSrc_t& inputSource, uint32_t& iModeIndex, const uint32_t res_width, const uint32_t res_height);
	bool GetInputSource(const QCarCamMode_t* inputModes, const uint32_t uiModes, const uint32_t width, const uint32_t height, QCarCamInputSrc_t& inputSource, uint32_t& iModeIndex);
	/* Wayland描画設定処理 */
	static int32_t SettingRendererConfig(VhalCaptureDeviceEntry * const p_entry, capturecontrol_info_t * const p_capture_ctrl_info, const uint32_t width, const uint32_t height);
	/* サーフェスID設定処理 */
	static int32_t SettingIviId(CVhalLayoutManager * const p_layout_mng, capturecontrol_info_t * const p_capture_ctrl_info, VhalCaptureDeviceEntry * const p_entry);
	/* 描画バッファ設定処理 */
	int32_t SettingBuffer(capturecontrol_info_t * const p_capture_ctrl_info, const QCarCamColorFmt_e color_fmt, const int32_t buf_count, const uint32_t width, const uint32_t height);
	/* QCarCamパラメータ処理(コールバック設定) */
	static int32_t SettingParamEventCallBack(const QCarCamHndl_t hndl);
	/* QCarCamフレームリクエスト送信 */
	void SettingSubmitRequest(const QCarCamHndl_t hndl, const uint32_t request_id, const uint32_t buf_index, QCarCamRet_e& qret) const noexcept;
	/* HDCP認証の後段機器情報設定 BEVstep3では不要 */
//	void SettingParamHdcpAuthResultRse(const captureInputType input_type);
	/* 黒画設定処理 */
	int32_t SettingBlackBuffer(capturecontrol_info_t * const p_capture_ctrl_info, const uint32_t height) const;
	/* 黒画描画処理 */
	int32_t SendFrameBlackBuffer(const capturecontrol_info_t * const p_capture_ctrl_info) const;
	/* キャプチャリセットイベント通知 */
	void NotifyCaptureResetEvent(const captureInputType input_type, const CaptureResetFactor factor);

};

} /* namespace videohal */


#endif	/* #ifndef	VHAL_CAPTURE_CONTROL_H */
