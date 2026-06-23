/*******************************************************************************
    機能名称    ：  MUTE処理モジュール
    ファイル名称：  vhal_mute.h
*******************************************************************************/
#ifndef	VHAL_MUTE_H
#define	VHAL_MUTE_H

#include <condition_variable>

#include "wl_renderer_public.h"
#include "vhal_layout_mng.h"
#include "vhal_event_route.h"
#include "vhal_micon_most_video_info.h"

extern "C"
{
#if 1	// for BEVstep3
#include "com_stddef.h"
#endif
#include "spf_timer_public.h"
}

extern "C" using MuteTimerCallback = void (*)(void* p);

namespace videohal
{
class CVhalMainControl;
class CVhalLayoutManager;
class CVhalGpioControl;
class CVhalMiconCommControl;


/*****************************************************************************
 クラス名称：CVhalMuteListenerBase
 処理概要  ：Muteイベントリスナベース。
*****************************************************************************/
class CVhalMuteListenerBase {
public:
	CVhalMuteListenerBase(void) noexcept = default;
	virtual ~CVhalMuteListenerBase(void) = default;
	CVhalMuteListenerBase(const CVhalMuteListenerBase& src) = delete;
	CVhalMuteListenerBase(CVhalMuteListenerBase&& src) = delete;

	virtual void NotifyCameraChanged(const int32_t result) const noexcept = 0;
private:
	CVhalMuteListenerBase& operator=(const CVhalMuteListenerBase& src) & = delete;
	CVhalMuteListenerBase& operator=(CVhalMuteListenerBase&& src) & = delete;
};


/*****************************************************************************
 クラス名称：CVhalMute
 処理概要  ：
*****************************************************************************/
class CVhalMute {
public:
	CVhalMute(void);
	~CVhalMute(void);
  	CVhalMute(const CVhalMute& src) = delete;
	CVhalMute& operator=(const CVhalMute& src) & = delete;
	CVhalMute(CVhalMute&& src) = delete;
	CVhalMute& operator=(CVhalMute&& src) & = delete;

	int32_t Initialize(CVhalMainControl * const p_main_control, CVhalLayoutManager * const p_layout_manager, CVhalGpioControl * const p_gpio_control, wlrenderer::CWaylandRenderer * const p_wayland_renderer, CVhalMiconMostVideoInfo * const p_most_video_info, const bool &mute_init);
	void Finalize(void);
	void ReInit(void) noexcept;

	/* 前席ディスプレイ全体のMUTE設定 */
	int32_t SetMuteFrontDisp(const bool mute);
	/* 前席ディスプレイ全体のMUTE状態取得 */
	bool GetMuteFrontDisp(void) const noexcept;
	/* 前席映像面のMUTE設定 */
	void SetMuteFrontVideo(const bool mute);
	/* 後席ディスプレイ全体のMUTE設定 */
	void SetMuteRearDisp(const bool mute);
	/* 後席映像面のMUTE設定 */
	void SetMuteRearVideo(const bool mute);

	/* カメラ映像パス切り替えMUTE設定 */
	void SetMuteCameraPathOn(void);
	/* カメラ映像パス切り替えMUTE解除設定 */
	void SetMuteCameraPathOff(void);
	/* カメラ映像パス切り替えMUTE状態取得 */
	bool GetMuteCameraPath(void) const noexcept;
	/* カメラ同期ON状態設定 */
	void SetMuteCameraSyncOn(void);
	/* カメラ同期OFF状態設定 */
	void SetMuteCameraSyncOff(void);
	/* 電源状態：見た目オン起動状態設定 */
	void SetMuteCameraVehiclePowerStateOn(void);
	/* 電源状態：見た目オン起動以外状態設定 */
	void SetMuteCameraVehiclePowerStateOff(void);
	/* カメラ面のMUTE設定(同期処理/IG処理用) */
	void SetMuteCamera(void);

	/* カメラ面のMUTE設定(パス切替用) */
	void SetMuteCameraPath(const std::string& path);
	/* カメラ面のMUTE解除設定(カメラ映像パス切り替え失敗) */
	void SetMuteCameraPathFailed(void);
	/* カメラ面のMUTE無効設定(強制) */
	void SetMuteCameraOff(void);
	/* カメラ面のMUTE有効設定(強制) */
	void SetMuteCameraOn(void);

	/* タイマコールバック(カメラ開始用MUTE OFF) */
	static void ReleaseMuteCameraOn(void* const arg);
	/* タイマコールバック(カメラ終了用MUTE OFF) */
	static void ReleaseMuteCameraOff(void* const arg);

	/* イベントリスナー登録 */
	int32_t RegisterEventListener(CVhalMuteListenerBase* const p_listener);
	/* イベントリスナー解除 */
	void ClearEventListener(void) noexcept;

protected:

private:
	CVhalMainControl*		p_main_;
	std::unique_ptr<CVhalEventRoute>	p_event_route_;
	CVhalLayoutManager*		p_layout_mng_;
	CVhalGpioControl*		p_gpioctrl_;
	CVhalMuteListenerBase*	p_mute_listener_;
	CVhalMiconMostVideoInfo* p_micon_most_video_info_;

	wlrenderer::CWaylandRenderer*		p_renderer_;
	wlrenderer::CWaylandRendererVideo*	p_blinder_display_;
	wlrenderer::CWaylandRendererVideo*	p_blinder_video_;
	wlrenderer::CWaylandRendererVideo*	p_blinder_video_sync_;
	wlrenderer::CWaylandRendererVideo*	p_blinder_camera_;
	wlrenderer::CWaylandRendererVideo*	p_blinder_background_;
	wlrenderer::CWaylandRendererVideo*	p_blinder_display_hud_;
	std::unique_ptr<wlrenderer::CWaylandRendererConfig>	p_blinder_config_;
	bool		initialized_;
	bool		camera_mute_sync_;		/* カメラ同期状態によるMUTE設定	true:MUTE ON/false:MUTE OFF */
	bool		camera_mute_power_;		/* 電源状態によるMUTE設定		true:MUTE ON/false:MUTE OFF */
	bool		camera_mute_path_;		/* カメラ切り替えMUTE設定		true:MUTE ON/false:MUTE OFF */
	bool		camera_mute_setting_;	/* カメラMUTE状態				true:MUTE ON/false:MUTE OFF */
	bool		camera_changed_notify_;	/* カメラ映像切替通知			true:有効/false:無効 */

	spf_timer_t	camera_mute_timer_;

	mutable std::mutex	mtx_camera_changed_notify_;	/* カメラ映像切替通知用 */

	/* ブラインダーの初期化 */
	static int32_t InitBlinder(wlrenderer::CWaylandRendererVideo * const p_blinder, const int32_t blinder_color, const wlrenderer::CWaylandRendererConfig * const p_blinder_config);
	/* タイマスタート */
	static int32_t StartMuteTimer(spf_timer_t * const mute_timer, MuteTimerCallback func, const uint32_t time_ms);
	/* タイマストップ */
	static int32_t StopMuteTimer(spf_timer_t * const mute_timer);

	/* 後席MM-RSE-MUTE設定 */
	void SetMmRseMute(const bool mute) const;
	/* WaylandRendererVideo作成 */
	wlrenderer::CWaylandRendererVideo* CreateBlinder(const VhalBlinderType blinder_front, const VhalBlinderType blinder_rear, const int32_t blinder_color, wlrenderer::CWaylandRenderer * const p_renderer);
	/* カメラ映像切替通知設定 */
	void SetNotifyCameraChanged(const bool value);
	/* カメラ映像切替通知 */
	void NotifyCameraChanged(const int32_t result);

	/* HUDディスプレイ全体のMUTE用のWaylandRendererVideo作成 */
	wlrenderer::CWaylandRendererVideo* CreateHudBlinder(const VhalBlinderType blinder_hud, const int32_t blinder_color, const bool mute_init, wlrenderer::CWaylandRenderer * const p_rend) noexcept;
};

extern "C" void MuteReleaseMuteCameraOn(void* const arg);
extern "C" void MuteReleaseMuteCameraOff(void* const arg);

} /* namespace videohal */

#endif	/* #ifndef VHAL_MUTE_H */
