/*******************************************************************************
    機能名称    ：  MUTE機能モジュール
    ファイル名称：  vhal_mute.cpp
*******************************************************************************/
#include "vhal_mute.h"

#include <mutex>
#include <chrono>

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_round_cast.h"
#include "vhal_main_control.h"
#include "vhal_layout_mng.h"
#include "vhal_gpio_control.h"
#include "vhal_event_item_camera_changed.h"
#include "vhal_micon_comm_control.h"
#include "vhal_micon_most_video_info.h"
#include "vhal_str_mng.h"
#include "vhal_debug_system.h"
#if 1	// for BEVstep3
#include "drm_fourcc.h"
#endif

namespace videohal
{
namespace
{
/* MISRA C++-2008 Rule 16-2-1 #if 0	*/ /* デバッグ用 */
	/* constexpr int32_t kVhalBlinderColorDisplay{static_cast<int32_t>(0xffff8080U)}; */ /* 赤 */
	/* constexpr int32_t kVhalBlinderColorVideo{static_cast<int32_t>(0xff80ffffU)};   */ /* 水 */
	/* constexpr int32_t kVhalBlinderColorCamera{static_cast<int32_t>(0xffffff80U)};  */ /* 黄 */
	/* constexpr int32_t kVhalBlinderColorBg{static_cast<int32_t>(0xff0080FFU)};      */ /* 青 */
// MISRA C++-2008 Rule 16-2-1 #else
	constexpr int32_t kVhalBlinderColorDisplay{static_cast<int32_t>(0xff000000U)};  /* 黒 */
	constexpr int32_t kVhalBlinderColorVideo{static_cast<int32_t>(0xff000000U)};    /* 黒 */
	constexpr int32_t kVhalBlinderColorVideoSync{static_cast<int32_t>(0xff000000U)};    /* 黒 */
	constexpr int32_t kVhalBlinderColorCamera{static_cast<int32_t>(0xff000000U)};   /* 黒 */
	constexpr int32_t kVhalBlinderColorBg{static_cast<int32_t>(0xff000000U)};       /* 黒 */
// MISRA C++-2008 Rule 16-2-1 #endif
	/* 上位資料の指示の対応のため、前プロジェクトの値(244ms)から今プロジェクトの仕様値(239ms)へ変更する */
	constexpr uint32_t kVhalTimerCameraStart{239U};			/* タイマ周期(カメラ表示切替時のMUTE時間):239ms   */
	constexpr uint32_t kVhalTimerCameraEnd{50U};			/* タイマ周期(カメラ非表示切替時のMUTE時間):50ms  */

	/* BEVstep3 24/10/17 描画種別：実機=false / シミュレータ=true */
	constexpr bool kVhalRendererShm{RENDERER_SHM};

/*****************************************************************************
 処理概要：	タイマコールバック
 引数    ：	
 戻り値  ：	なし
*****************************************************************************/
extern "C" void CallbackDefaultTimer(void* const arg)
{
	VHAL_LOGE("muteTimer no setting.");
}

}


/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalMute::CVhalMute(void)
	:p_main_(nullptr)
	,p_event_route_(nullptr)
	,p_layout_mng_(nullptr)
	,p_gpioctrl_(nullptr)
	,p_mute_listener_(nullptr)
	,p_micon_most_video_info_(nullptr)
	,p_renderer_(nullptr)
	,p_blinder_display_(nullptr)
	,p_blinder_video_(nullptr)
	,p_blinder_video_sync_(nullptr)
	,p_blinder_camera_(nullptr)
	,p_blinder_background_(nullptr)
	,p_blinder_config_(nullptr)
	,initialized_(false)
	,camera_mute_sync_(false)		/* 初期値:同期なし[MUTE ON] */
	,camera_mute_power_(false)		/* 初期値:見た目オン起動以外[MUTE ON] */
	,camera_mute_path_(false)
	,camera_mute_setting_(false)
	,camera_changed_notify_(false)
	,camera_mute_timer_{}
	,mtx_camera_changed_notify_{}
{
	VHAL_LOGV("CVhalMute is created. this=%p", this);
	VHAL_LOGV("ColorDisplay=0x%x, ColorVideo=0x%x, ColorCamera=0x%x, ColorBg=0x%x", kVhalBlinderColorDisplay, kVhalBlinderColorVideo, kVhalBlinderColorCamera, kVhalBlinderColorBg);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalMute::~CVhalMute(void)
{
	VHAL_LOGV("CVhalMute is deleted. this=%p", this);
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ：	CVhalMainControl * const p_main_control						(i)メインコントロールインスタンスポインタ
           	CVhalLayoutManager * const p_layout_manager					(i)レイアウト制御インスタンスポインタ
           	CVhalGpioControl * const p_gpio_control						(i)GPIO制御インスタンスポインタ
           	wlrenderer::CWaylandRenderer * const p_wayland_renderer		(i)WaylandRendererインスタンスポインタ
           	CVhalMiconCommControl* const p_micon_common_control			(i)マイコン制御インスタンスポインタ
           	CVhalMiconMostVideoInfo * p_most_video_info					(i)MOST映像情報送信インスタンスポインタ
           	bool &mute_init												(i/o)true MUTE有効/false MUTE無効
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-N-163
*****************************************************************************/
int32_t CVhalMute::Initialize(CVhalMainControl * const p_main_control, CVhalLayoutManager * const p_layout_manager, CVhalGpioControl * const p_gpio_control, wlrenderer::CWaylandRenderer * const p_wayland_renderer, CVhalMiconMostVideoInfo * const p_most_video_info, const bool &mute_init)
{
	int32_t result{VHAL_SUCCESS};

	if ((nullptr == p_main_control) || (nullptr == p_layout_manager) || (nullptr == p_gpio_control) || (nullptr == p_wayland_renderer) || (nullptr == p_most_video_info))
	{
		VHAL_LOGE("parameter error. p_main_control=%p, p_layout_manager=%p, p_gpio_control=%p, p_wayland_renderer=%p, p_most_video_info=%p", p_main_control, p_layout_manager, p_gpio_control, p_wayland_renderer, p_most_video_info);
		result = VHAL_ERR_PARAM;
	}
	else
	{
		if (true == initialized_)
		{
			VHAL_LOGE("CVhalMute is already initialized");
			result = VHAL_ERR_PARAM;
		}
		else
		{
			p_main_       = p_main_control;
			p_layout_mng_ = p_layout_manager;
			p_gpioctrl_   = p_gpio_control;
			p_renderer_ = p_wayland_renderer;
			p_micon_most_video_info_ = p_most_video_info;

			p_event_route_ = std::make_unique<CVhalEventRoute>();
			if (nullptr == p_event_route_)
			{
				VHAL_LOGE("Failed to create CVhalEventRoute.");
				result = VHAL_ERR;
			}
			else
			{
				int32_t ret{p_event_route_->Initialize()};
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("EventRoute Initialize error. ret=%d", ret);
					result = ret;
				}
				else
				{
					ret = p_main_->RegisterEventSource(p_event_route_.get());
					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("RegisterEventSource error. ret=%d", ret);
						result = ret;
					}
					else
					{
						constexpr uint32_t kVhalTimerDefault{1000U};			/* タイマ周期(初期設定):1s                        */
						/* MUTE制御タイマ作成 */
						spf_timer_attr attr;
						attr.name		= nullptr;						/* 識別名指定なし       */
						attr.wake_cnt	= 1U;							/* サイクリックタイマ   */
						attr.cycle		= kVhalTimerDefault;			/* タイマ周期           */
						attr.callback	= &CallbackDefaultTimer;		/* コールバック関数     */
						attr.arg		= this;							/* コールバック引数     */

						ret = spf_timer_create(&camera_mute_timer_, &attr);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//						int32_t fail_ret{0};
//						bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-163",fail_ret)};
//						if(true == fail)
//						{
//							ret = fail_ret;
//						}
//#endif
						if (0 != ret)
						{
							VHAL_LOGE("spf_timer_create() error ret=%d", ret);
							result = VHAL_ERR_TIMER;
						}
						else
						{
							VHAL_LOGI("make blinder");

							constexpr uint32_t kVhalBlinderWidth{32U};
							constexpr uint32_t kVhalBlinderHeight{32U};
							/* BEVstep3 */
/*							constexpr int32_t buf_type{static_cast<int32_t>(wlrenderer::CWaylandRendererVideo::VIDEO_BUFFER_TYPE_DMA)}; */
							constexpr uint32_t buf_count{1U};
							constexpr uint32_t width{kVhalBlinderWidth};
							constexpr uint32_t height{kVhalBlinderHeight};
/*							constexpr int32_t format{static_cast<int32_t>(DRM_FORMAT_XRGB8888)}; */
							int32_t ivi_id_front{0};
							int32_t ivi_id_rear{0};
							int32_t ivi_id_ic{0};
							int32_t ivi_id_hud{0};
							/* BEVstep3 */
							int32_t buf_type{static_cast<int32_t>(wlrenderer::CWaylandRendererVideo::VIDEO_BUFFER_TYPE_DMA)};
							int32_t format{static_cast<int32_t>(DRM_FORMAT_XRGB8888)};
							if (true == kVhalRendererShm)
							{
								buf_type = static_cast<int32_t>(wlrenderer::CWaylandRendererVideo::VIDEO_BUFFER_TYPE_SHM);
								format = static_cast<int32_t>(WL_SHM_FORMAT_XRGB8888);
							}

							p_blinder_config_ = std::make_unique<wlrenderer::CWaylandRendererConfig>();

							p_blinder_config_->SetBufferType(buf_type);
							p_blinder_config_->SetBufferCount(buf_count);
							p_blinder_config_->SetWidth(width);
							p_blinder_config_->SetHeight(height);
							p_blinder_config_->SetFormat(format);
							
							p_layout_mng_->SetBlinderFrontDispMuteInit(mute_init);
							p_layout_mng_->SetBlinderHudDispMuteInit(mute_init);

							/* 前席ディスプレイ全体のMUTE用のWaylandRendererVideo作成 */
							ret = p_layout_mng_->GetBlinderID(VhalBlinderType::VHAL_BLINDER_TYPE_FRONT_DISPLAY, ivi_id_front);
							if (VHAL_SUCCESS != ret)
							{
								VHAL_LOGW("GetBlinderID(front display) error. ret=%d", ret);
							}
							else
							{
								p_blinder_config_->AddSurfaceId(ivi_id_front);
							}
							ret = p_layout_mng_->GetBlinderID(VhalBlinderType::VHAL_BLINDER_TYPE_REAR_DISPLAY, ivi_id_rear);
							if (VHAL_SUCCESS != ret)
							{
								VHAL_LOGW("GetBlinderID(rear display) error. ret=%d", ret);
							}
							else
							{
								p_blinder_config_->AddSurfaceId(ivi_id_rear);
							}
							ret = p_layout_mng_->GetBlinderID(VhalBlinderType::VHAL_BLINDER_TYPE_IC_DISPLAY, ivi_id_ic);
							if (VHAL_SUCCESS != ret)
							{
								VHAL_LOGW("GetBlinderID(ic display) error. ret=%d", ret);
							}
							else
							{
								p_blinder_config_->AddSurfaceId(ivi_id_ic);
							}

							if (0 != ivi_id_front)
							{
								p_blinder_display_ = p_renderer_->CreateRendererVideo(*p_blinder_config_);
								if (nullptr == p_blinder_display_)
								{
									VHAL_LOGW("CreateClientVideo(blinder_display) error.");
								}
								else
								{
									ret = InitBlinder(p_blinder_display_, kVhalBlinderColorDisplay, p_blinder_config_.get());
									if (0 > ret)
									{
										VHAL_LOGW("InitBlinder(blinder_display) error. ret=%d", ret);
									}
								}
							}
							p_blinder_config_->ClearSurfaceId();

							ivi_id_front = 0;
							ivi_id_rear  = 0;

							/* 映像面のMUTE用のWaylandRendererVideo作成 */
							ret = p_layout_mng_->GetBlinderID(VhalBlinderType::VHAL_BLINDER_TYPE_FRONT_VIDEO, ivi_id_front);
							if (VHAL_SUCCESS != ret)
							{
								VHAL_LOGW("GetBlinderID(front video) error. ret=%d", ret);
							}
							else
							{
								p_blinder_config_->AddSurfaceId(ivi_id_front);
							}
							ret = p_layout_mng_->GetBlinderID(VhalBlinderType::VHAL_BLINDER_TYPE_REAR_VIDEO, ivi_id_rear);
							if (VHAL_SUCCESS != ret)
							{
								VHAL_LOGW("GetBlinderID(rear video) error. ret=%d", ret);
							}
							else
							{
								p_blinder_config_->AddSurfaceId(ivi_id_rear);
							}

							if (0 != ivi_id_front)
							{
								p_blinder_video_ = p_renderer_->CreateRendererVideo(*p_blinder_config_);
								if (nullptr == p_blinder_video_)
								{
									VHAL_LOGW("CreateClientVideo(blinder_video) error.");
								}
								else
								{
									ret = InitBlinder(p_blinder_video_, kVhalBlinderColorVideo, p_blinder_config_.get());
									if (0 > ret)
									{
										VHAL_LOGW("InitBlinder(p_blinder_video_) error. ret=%d", ret);
									}
								}
							}
							p_blinder_config_->ClearSurfaceId();

							ivi_id_front = 0;

							/* 前席映像同期面のMUTE用のWaylandRendererVideo作成 */
							ret = p_layout_mng_->GetBlinderID(VhalBlinderType::VHAL_BLINDER_TYPE_FRONT_VIDEO_SYNC, ivi_id_front);
							if (VHAL_SUCCESS != ret)
							{
								VHAL_LOGW("GetBlinderID(front video sync) error. ret=%d", ret);
							}
							else
							{
								p_blinder_config_->AddSurfaceId(ivi_id_front);
							}

							if (0 != ivi_id_front)
							{
								p_blinder_video_sync_ = p_renderer_->CreateRendererVideo(*p_blinder_config_);
								if (nullptr == p_blinder_video_sync_)
								{
									VHAL_LOGW("CreateClientVideo(blinder_video_sync) error.");
								}
								else
								{
									ret = InitBlinder(p_blinder_video_sync_, kVhalBlinderColorVideoSync, p_blinder_config_.get());
									if (0 > ret)
									{
										VHAL_LOGW("InitBlinder(p_blinder_video_sync) error. ret=%d", ret);
									}
								}
							}
							p_blinder_config_->ClearSurfaceId();

							ivi_id_front = 0;

							/* カメラ面のMUTE用のWaylandRendererVideo作成 */
							ret = p_layout_mng_->GetBlinderID(VhalBlinderType::VHAL_BLINDER_TYPE_CAMERA, ivi_id_front);
							if (VHAL_SUCCESS != ret)
							{
								VHAL_LOGW("GetBlinderID(camera) error. ret=%d", ret);
							}
							else
							{
								p_blinder_config_->AddSurfaceId(ivi_id_front);
							}

							if (0 != ivi_id_front)
							{
								p_blinder_camera_ = p_renderer_->CreateRendererVideo(*p_blinder_config_);
								if (nullptr == p_blinder_camera_)
								{
									VHAL_LOGW("CreateClientVideo(blinder_camera) error.");
								}
								else
								{
									ret = InitBlinder(p_blinder_camera_, kVhalBlinderColorCamera, p_blinder_config_.get());
									if (0 > ret)
									{
										VHAL_LOGW("InitBlinder(p_blinder_camera_) error. ret=%d", ret);
									}
								}
							}
							p_blinder_config_->ClearSurfaceId();

							/* 映像面の背景黒画用のWaylandRendererVideo作成 */
							p_blinder_background_ = CreateBlinder(VhalBlinderType::VHAL_BLINDER_TYPE_FRONT_VIDEO_BG, VhalBlinderType::VHAL_BLINDER_TYPE_REAR_VIDEO_BG, kVhalBlinderColorBg, p_renderer_);
							if (nullptr == p_blinder_background_)
							{
								VHAL_LOGW("CreateBlinder(blinder_background) error.");
							}

							int32_t screen_id_hud{-1};
							ret = p_layout_mng_->GetScreenIdHud(screen_id_hud);
							if (VHAL_SUCCESS == ret)
							{
								const bool hud_available{p_layout_mng_->IsScreenAvailable(screen_id_hud)};
								if (true == hud_available)
								{
									/* HUDのMUTE用のWaylandRendererVideo作成 */
									p_layout_mng_->SetBlinderHudDispMuteInit(mute_init);
									ret = p_layout_mng_->GetBlinderID(VhalBlinderType::VHAL_BLINDER_TYPE_HUD_DISPLAY, ivi_id_hud);
									if (VHAL_SUCCESS != ret)
									{
										VHAL_LOGW("GetBlinderID(hud display) error. ret=%d", ret);
									}
									else
									{
										p_blinder_config_->AddSurfaceId(ivi_id_hud);
									}
									if (0 != ivi_id_hud)
									{
										p_blinder_video_ = p_renderer_->CreateRendererVideo(*p_blinder_config_);
										if (nullptr == p_blinder_video_)
										{
											VHAL_LOGW("CreateClientVideo(blinder_video) error.");
										}
										else
										{
											ret = InitBlinder(p_blinder_video_, kVhalBlinderColorVideo, p_blinder_config_.get());
											if (0 > ret)
											{
												VHAL_LOGW("InitBlinder(p_blinder_video_) error. ret=%d", ret);
											}
										}
									}
								}
								else
								{
									VHAL_LOGE("HUD screen is not available.");
								}
								p_blinder_config_->ClearSurfaceId();
							}
							else
							{
								VHAL_LOGI("HUD screen not found layout.");
							}
						}
					}
				}
			}
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	終了処理（内部リソースの解放）
 引数    ：	なし
 戻り値  ：	なし
 フェールセーフNo  ：	F-VHAL-N-167
*****************************************************************************/
void CVhalMute::Finalize(void)
{
	int32_t ret;

	if (nullptr != p_blinder_config_)
	{
		p_blinder_config_ = nullptr;
	}

	/* ブラインダー用のWaylandRendererVideo削除 */

	if (nullptr != p_blinder_display_)
	{
		p_renderer_->RemoveRendererVideo(p_blinder_display_);
		p_blinder_display_ = nullptr;
	}
	if (nullptr != p_blinder_video_)
	{
		p_renderer_->RemoveRendererVideo(p_blinder_video_);
		p_blinder_video_ = nullptr;
	}
	if (nullptr != p_blinder_video_sync_)
	{
		p_renderer_->RemoveRendererVideo(p_blinder_video_sync_);
		p_blinder_video_sync_ = nullptr;
	}
	if (nullptr != p_blinder_camera_)
	{
		p_renderer_->RemoveRendererVideo(p_blinder_camera_);
		p_blinder_camera_ = nullptr;
	}
	if (nullptr != p_blinder_background_)
	{
		p_renderer_->RemoveRendererVideo(p_blinder_background_);
		p_blinder_background_ = nullptr;
	}

	ret = spf_timer_delete(&camera_mute_timer_);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-167",fail_ret)};
//	if(true == fail)
//	{
//		ret = fail_ret;
//	}
//#endif
	if (0 != ret)
	{
		VHAL_LOGE("spf_timer_delete() error.(ret=%d)", ret);
	}

	if (nullptr != p_event_route_)
	{
		p_main_->ClearEventSource(p_event_route_.get());
		p_event_route_ = nullptr;
	}
}

/*****************************************************************************
 処理概要：	再初期化処理
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalMute::ReInit(void) noexcept
{
	/* MUTEタイマストップ */
	const int32_t ret{StopMuteTimer(&camera_mute_timer_)};
	if (VHAL_SUCCESS != ret)
	{
		/* 失敗時はCVhalMute::StopMuteTimer内でログ出力するため、ここでは検証用ログのみ */
		VHAL_LOGV("StopMuteTimer() error.(ret=%d)", ret);
	}

	/* 各メンバ変数の初期化 */
	camera_mute_sync_ = false;
	camera_mute_power_ = false;
	camera_mute_path_ = false;
	SetNotifyCameraChanged(false);
	SetMuteCamera();	/* カメラ面MUTE-OFF指示 */
}

/*****************************************************************************
 処理概要：	前席ディスプレイ全体のMUTE設定
 引数    ：	const bool mute	(i)true MUTE有効/false MUTE無効
 戻り値  ：	処理結果
           		VHAL_ERR			異常終了
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalMute::SetMuteFrontDisp(const bool mute)
{
	VHAL_LOGV_IN("SetMuteFrontDisp [%d]", mute);

	/* サーフェスMUTE設定/解除 */
	const int32_t result{p_layout_mng_->SetBlinderEnable(VhalBlinderType::VHAL_BLINDER_TYPE_FRONT_DISPLAY, mute)};
	if (VHAL_SUCCESS != result)
	{
		VHAL_LOGE("SetBlinderEnable NG result=%d mute=%d",result, mute);
	}

	VHAL_LOGV_OUT();

	return result;
}

/*****************************************************************************
 処理概要：	前席ディスプレイ全体のMUTE状態取得
 引数    ：	なし
 戻り値  ：	前席ディスプレイ全体のMUTE状態
				true	MUTE ON
				false	MUTE OFF
*****************************************************************************/
bool CVhalMute::GetMuteFrontDisp(void) const noexcept
{
	bool mute_status{false};
	
	return mute_status;
}

/*****************************************************************************
 処理概要：	前席映像面のMUTE設定
 引数    ：	bool mute	(i)true MUTE有効/false MUTE無効
 戻り値  ：	なし
*****************************************************************************/
void CVhalMute::SetMuteFrontVideo(const bool mute)
{
	VHAL_LOGV_IN("SetMuteFrontVideo [%d]", mute);

	(void)p_layout_mng_->SetBlinderEnable(VhalBlinderType::VHAL_BLINDER_TYPE_FRONT_VIDEO, mute);

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	後席ディスプレイ全体のMUTE設定
 引数    ：	bool mute	(i)true MUTE有効/false MUTE無効
 戻り値  ：	なし
*****************************************************************************/
void CVhalMute::SetMuteRearDisp(const bool mute)
{
	VHAL_LOGV_IN("SetMuteRearDisp [%d]", mute);

	(void)p_layout_mng_->SetBlinderEnable(VhalBlinderType::VHAL_BLINDER_TYPE_REAR_DISPLAY, mute);

	/* フルRSE時のみMM-RSE-MUTE */
	int32_t connected_rse;
	p_layout_mng_->GetConnectedRse(connected_rse);

	if (VHAL_CONNECTED_RSE_FULL == connected_rse)
	{
		SetMmRseMute(mute);
	}
	else if (VHAL_CONNECTED_RSE_DOP == connected_rse)
	{
		/* 用品RSE接続時に有効 */
		/* 後席ディスプレイ全体MUTE設定(MOST) */
		if (nullptr != p_micon_most_video_info_)
		{
			p_micon_most_video_info_->SetMuteRearDisp(mute);
		}
	}
	else
	{
		/* 処理なし */
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	後席映像面のMUTE設定
 引数    ：	bool mute	(i)true MUTE有効/false MUTE無効
 戻り値  ：	なし
*****************************************************************************/
void CVhalMute::SetMuteRearVideo(const bool mute)
{
	VHAL_LOGV_IN("SetMuteRearVideo [%d]", mute);

	(void)p_layout_mng_->SetBlinderEnable(VhalBlinderType::VHAL_BLINDER_TYPE_REAR_VIDEO, mute);

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	カメラ映像パス切り替えMUTE設定
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalMute::SetMuteCameraPathOn(void)
{
	VHAL_LOGV_IN();

	/* カメラMUTE-ON設定 */
	camera_mute_path_ = true;
	SetMuteCamera();

	VHAL_LOGV_OUT("[camera_mute_path_:%d]", camera_mute_path_);
}

/*****************************************************************************
 処理概要：	カメラ映像パス切り替えMUTE解除設定
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalMute::SetMuteCameraPathOff(void)
{
	VHAL_LOGV_IN();

	/* カメラMUTE-OFF設定 */
	camera_mute_path_ = false;
	SetMuteCamera();

	VHAL_LOGV_OUT("[camera_mute_path_:%d]", camera_mute_path_);
}

/*****************************************************************************
 処理概要：	カメラ映像パス切り替えMUTE状態取得
 引数    ：	なし
 戻り値  ：	MUTE状態
           		true	MUTE有効
           		false	MUTE無効
*****************************************************************************/
bool CVhalMute::GetMuteCameraPath(void) const noexcept
{
	return camera_mute_path_;
}

/*****************************************************************************
 処理概要：	カメラ同期ON状態設定
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalMute::SetMuteCameraSyncOn(void)
{
	VHAL_LOGV_IN();

	/* カメラMUTE-OFF設定 */
	camera_mute_sync_ = false;

	VHAL_LOGV_OUT("[camera_mute_sync_:%d]", camera_mute_sync_);
}

/*****************************************************************************
 処理概要：	カメラ同期OFF状態設定
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalMute::SetMuteCameraSyncOff(void)
{
	VHAL_LOGV_IN();

	/* カメラMUTE-ON設定 */
	camera_mute_sync_ = true;

	VHAL_LOGV_OUT("[camera_mute_sync_:%d]", camera_mute_sync_);
}

/*****************************************************************************
 処理概要：	電源状態：見た目オン起動状態設定
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalMute::SetMuteCameraVehiclePowerStateOn(void)
{
	VHAL_LOGV_IN();

	/* カメラMUTE-OFF設定 */
	camera_mute_power_ = false;

	VHAL_LOGV_OUT("[camera_mute_power_:%d]", camera_mute_power_);
}

/*****************************************************************************
 処理概要：	電源状態：見た目オン起動以外状態設定
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalMute::SetMuteCameraVehiclePowerStateOff(void)
{
	VHAL_LOGV_IN();

	/* カメラMUTE-ON設定 */
	camera_mute_power_ = true;

	VHAL_LOGV_OUT("[camera_mute_power_:%d]", camera_mute_power_);
}

/*****************************************************************************
 処理概要：	カメラ面のMUTE設定
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalMute::SetMuteCamera(void)
{
	VHAL_LOGV_IN();

	bool	setmute{true};

	/* カメラ同期あり/見た目オン起動状態/カメラ切替MUTE解除時のみMUTE解除 */
	if ((false == camera_mute_sync_) && (false == camera_mute_power_) && (false == camera_mute_path_))
	{
		setmute = false;
	}

	/* MUTE設定に変更がある時のみ */
	if (camera_mute_setting_ != setmute)
	{
		camera_mute_setting_ = setmute;
		VhalLogCameraMute(__builtin_FUNCTION(), camera_mute_setting_);
		/* SetBlinderEnableの戻り値を確認し、失敗時はログ出力する */
		int32_t ret = p_layout_mng_->SetBlinderEnable(VhalBlinderType::VHAL_BLINDER_TYPE_CAMERA, camera_mute_setting_);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetBlinderEnable(VHAL_BLINDER_TYPE_CAMERA, %d) failed. ret=%d", static_cast<int32_t>(camera_mute_setting_), ret);
		}
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	カメラ面のMUTE設定(パス切替用)
 引数    ：	const std::string&	path	(i)カメラpath文字列
 戻り値  ：	なし
*****************************************************************************/
void CVhalMute::SetMuteCameraPath(const std::string& path)
{
	int32_t ret;
	uint32_t time_ms{0U};
	MuteTimerCallback func;

	VHAL_LOGV_IN("SetMuteCameraPath [%s]", path.c_str());

	if (!path.empty())
	{
		time_ms = kVhalTimerCameraStart;
		func = &MuteReleaseMuteCameraOn;
	}
	else
	{
		time_ms = kVhalTimerCameraEnd;
		func = &MuteReleaseMuteCameraOff;
	}

	/* カメラ映像切替通知あり設定 */
	SetNotifyCameraChanged(true);

	/* カメラMUTE制御タイマストップ */
	ret = StopMuteTimer(&camera_mute_timer_);
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("StopMuteTimer() error.(ret=%d)", ret);
		func(this);
	}
	else
	{
		SetMuteCameraPathOn();

		/* カメラMUTE制御タイマスタート */
		ret = StartMuteTimer(&camera_mute_timer_, func, time_ms);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("StartMuteTimer() error.(ret=%d)", ret);
			func(this);
		}
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	カメラ面のMUTE解除設定(カメラ映像切り替え失敗)
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalMute::SetMuteCameraPathFailed(void)
{
	VHAL_LOGV_IN();

	/* カメラ映像切替通知なし設定 */
	SetNotifyCameraChanged(false);

	/* カメラMUTE OFF(強制) */
	SetMuteCameraOff();

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	カメラ面のMUTE有効設定(強制)
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalMute::SetMuteCameraOn(void)
{
	VHAL_LOGV_IN();

	camera_mute_path_ = true;
	SetMuteCamera();

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	カメラ面のMUTE無効設定(強制)
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalMute::SetMuteCameraOff(void)
{
	VHAL_LOGV_IN();

	camera_mute_path_ = false;

	/* MUTE設定に変更がある時のみ */
	if (camera_mute_setting_ != camera_mute_path_)
	{
		camera_mute_setting_ = camera_mute_path_;
		VhalLogCameraMute(__builtin_FUNCTION(), camera_mute_setting_);
		(void)p_layout_mng_->SetBlinderEnable(VhalBlinderType::VHAL_BLINDER_TYPE_CAMERA, camera_mute_setting_);
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	タイマコールバック(カメラ開始用MUTE OFF)
 引数    ：	void *arg			(i)レイアウト制御インスタンスポインタ
 戻り値  ：	なし
*****************************************************************************/
void CVhalMute::ReleaseMuteCameraOn(void* const arg)
{
	CVhalMute * const p_mute{static_cast<CVhalMute *>(arg)};

	VHAL_LOGV_IN();

	/* カメラMUTE OFF */
	p_mute->SetMuteCameraPathOff();

	int32_t	result{UI32ToI32(VHAL_VPATH_STS_SUCCESS)};
	/* カメラ同期なし、見た目オン起動以外によるMUTE設定のため切替失敗 */
	if ((true == p_mute->camera_mute_sync_) || (true == p_mute->camera_mute_power_))
	{
		result = UI32ToI32(VHAL_VPATH_STS_FAILED);
	}
	p_mute->NotifyCameraChanged(result);

	VHAL_LOGI("ReleaseMuteCamera. result=%d", result);

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	タイマコールバック(カメラ終了用MUTE OFF)
 引数    ：	void *arg			(i)レイアウト制御インスタンスポインタ
 戻り値  ：	なし
*****************************************************************************/
void CVhalMute::ReleaseMuteCameraOff(void* const arg)
{
	CVhalMute * const p_mute{static_cast<CVhalMute *>(arg)};

	VHAL_LOGV_IN();

	/* カメラMUTE OFF(強制) */
	p_mute->SetMuteCameraOff();

	p_mute->NotifyCameraChanged(UI32ToI32(VHAL_VPATH_STS_SUCCESS));

	VHAL_LOGI("ReleaseMuteCamera.");

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	イベントリスナー登録
 引数    ：	CVhalMuteListenerBase* p_listener		(i)リスナーポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_SUCCESS			正常終了
****************************************************************************/
int32_t CVhalMute::RegisterEventListener(CVhalMuteListenerBase* const p_listener)
{
	if (nullptr == p_listener)
	{
		VHAL_LOGE("p_listener is null");
		return VHAL_ERR_PARAM;
	}

	p_mute_listener_ = p_listener;

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	イベントリスナー解除
 引数    ：	なし
 戻り値  ：	なし
****************************************************************************/
void CVhalMute::ClearEventListener(void) noexcept
{
	p_mute_listener_ = nullptr;
}


/*****************************************************************************
 処理概要：	ブラインダーの初期化
 引数    ：	wlrenderer::CWaylandRendererVideo*	p_blinder			(i)ブラインダー用WaylandRendererVideo
           	int32_t								blinder_color		(i)ブラインダーカラー
           	wlrenderer::CWaylandRendererConfig*	p_blinder_config	(i)ブラインダー用WaylandRendererConfig
 戻り値  ：	処理結果
           		VHAL_ERR_WAYLAND	waylandプロトコルエラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalMute::InitBlinder(wlrenderer::CWaylandRendererVideo * const p_blinder, const int32_t blinder_color, const wlrenderer::CWaylandRendererConfig * const p_blinder_config)
{
	int32_t ret;
	wlrenderer::CWaylandRendererVideoBuffer *p_buffer;

	VHAL_LOGV_IN();

	p_buffer = p_blinder->GetBuffer();
	if (nullptr == p_buffer)
	{
		VHAL_LOGE("blinder GetBuf erorr.");
		return VHAL_ERR_WAYLAND_CLIENT;
	}

	p_buffer->Clear(blinder_color);

	std::vector<int32_t> surface_ids{};
	p_blinder_config->GetSurfaceIds(surface_ids);
	ret = p_blinder->SendBuffer(p_buffer, surface_ids);
	if (0 > ret)
	{
		VHAL_LOGE("blinder SndBuf erorr. ret=%d", ret);
		return ret;
	}

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	タイマスタート
 引数    ：	spf_timer_t*	mute_timer		(i)spf_timerポインタ
           	MuteTimerCallback	func		(i)コールバック関数
           	uint32_t		time_ms			(i)MUTE時間(ms)
 戻り値  ：	処理結果
           		VHAL_ERR_TIMER		Timer処理エラー
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-N-164
                     	F-VHAL-N-165
                     	F-VHAL-N-166
*****************************************************************************/
int32_t CVhalMute::StartMuteTimer(spf_timer_t * const mute_timer, MuteTimerCallback func, const uint32_t time_ms)
{
	int32_t ret;
	spf_timer_attr attr;

	VHAL_LOGV_IN();

	/* タイマの属性情報取得 */
	ret = spf_timer_get_attr(mute_timer, &attr);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-166",fail_ret)};
//	if(true == fail)
//	{
//		ret = fail_ret;
//	}
//#endif
	if (0 != ret)
	{
		VHAL_LOGE("spf_timer_get_attr() error.(ret=%d)", ret);
		return VHAL_ERR_TIMER;
	}

	/* タイマ属性情報変更及び設定 */
	attr.cycle = time_ms;
	attr.callback = func;
	ret = spf_timer_set_attr(mute_timer, &attr);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	fail = CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-165",fail_ret);
//	if(true == fail)
//	{
//		ret = fail_ret;
//	}
//#endif
	if (0 != ret)
	{
		VHAL_LOGE("spf_timer_set_attr() error.(ret=%d)", ret);
		return VHAL_ERR_TIMER;
	}

	/* タイマスタート */
	ret = spf_timer_start(mute_timer);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	fail = CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-164",fail_ret);
//	if(true == fail)
//	{
//		ret = fail_ret;
//	}
//#endif
	if (0 != ret)
	{
		VHAL_LOGE("spf_timer_start() error.(ret=%d)", ret);
		return VHAL_ERR_TIMER;
	}

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	タイマストップ
 引数    ：	spf_timer_t*	mute_timer		(i)spf_timerポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_TIMER		Timer処理エラー
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-N-168
*****************************************************************************/
int32_t CVhalMute::StopMuteTimer(spf_timer_t * const mute_timer)
{
	int32_t ret;

	VHAL_LOGV_IN();

	/* タイマストップ */
	ret = spf_timer_stop(mute_timer);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-168",fail_ret)};
//	if(true == fail)
//	{
//		ret = fail_ret;
//	}
//#endif
	if (0 != ret)
	{
		VHAL_LOGE("spf_timer_stop() error.(ret=%d)", ret);
		return VHAL_ERR_TIMER;
	}

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	後席MM-RSE-MUTE設定
 引数    ：	bool mute	(i)true MUTE有効/false MUTE無効
 戻り値  ：	なし
*****************************************************************************/
void CVhalMute::SetMmRseMute(const bool mute) const
{
	int32_t	ret{HAL_SUCCESS};

	VHAL_LOGV_IN("SetMmRseMute [%d]", mute);

	/* MM-RSE-MUTE処理 */
	/* 端子操作 */
	if (true == mute)
	{
		/* MUTEセット */
		ret = p_gpioctrl_->SetValueDirectionLow(HAL_GPIO_O_MM_RSE_MUTE, HAL_GPIO_HIGH);
		VHAL_LOGI("setValue() HAL_GPIO_HIGH (ret=%d)", ret);
	}
	else
	{
		/* MUTE解除 */
		ret = p_gpioctrl_->SetValueDirectionLow(HAL_GPIO_O_MM_RSE_MUTE, HAL_GPIO_LOW);
		VHAL_LOGI("setValue HAL_GPIO_LOW (ret=%d)", ret);
	}

	if (HAL_SUCCESS != ret)
	{
		VHAL_LOGE("setValue() error.(ret=%d)", ret);
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	CWaylandRendererVideo作成
 引数    ：	VhalBlinderType blinder_front				(i)前席ブラインダータイプ
            VhalBlinderType blinder_rear				(i)後席ブラインダータイプ
            int32_t blinder_color						(i)ブラインダーカラー
           	wlrenderer::CWaylandRenderer *p_renderer	(i)CWaylandRendererインスタンスポインタ
 戻り値  ：	CWaylandRendererVideoインスタンスポインタ
*****************************************************************************/
wlrenderer::CWaylandRendererVideo* CVhalMute::CreateBlinder(const VhalBlinderType blinder_front, const VhalBlinderType blinder_rear, const int32_t blinder_color, wlrenderer::CWaylandRenderer * const p_renderer)
{
	if (nullptr == p_renderer)
	{
		VHAL_LOGE("parameter error. p_renderer is null (front=%d, rear=%d)", blinder_front, blinder_rear);
		return nullptr;
	}
	/* 前席用は必ず必要 */
	if (VhalBlinderType::VHAL_BLINDER_TYPE_NONE == blinder_front)
	{
		VHAL_LOGE("parameter error. blinder_front is none. rear=%d", blinder_rear);
		return nullptr;
	}

	VHAL_LOGD("front=%d, rear=%d, color=0x%x", blinder_front, blinder_rear, blinder_color);

	/* 前席用 */
	int32_t ivi_id_front{0};	
	int32_t ret{p_layout_mng_->GetBlinderID(blinder_front, ivi_id_front)};
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGW("GetBlinderID(front=%d) error. ret=%d", blinder_front, ret);
	}
	else
	{
		p_blinder_config_->AddSurfaceId(ivi_id_front);
	}

	/* 後席用 */
	if (VhalBlinderType::VHAL_BLINDER_TYPE_NONE != blinder_rear)
	{
		int32_t ivi_id_rear{0};
		ret = p_layout_mng_->GetBlinderID(blinder_rear, ivi_id_rear);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGW("GetBlinderID(rear=%d) error. ret=%d", blinder_rear, ret);
		}
		else
		{
			p_blinder_config_->AddSurfaceId(ivi_id_rear);
		}
	}

	wlrenderer::CWaylandRendererVideo *p_blinder_video{nullptr};

	if (0 != ivi_id_front)
	{
		p_blinder_video = p_renderer->CreateRendererVideo(*p_blinder_config_);
		if (nullptr == p_blinder_video)
		{
			VHAL_LOGW("CreateRendererVideo(front=%d, rear=%d) error.", blinder_front, blinder_rear);
		}
		else
		{
			ret = InitBlinder(p_blinder_video, blinder_color, p_blinder_config_.get());
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGW("InitBlinder(front=%d, rear=%d, color=0x%x) error. ret=%d", blinder_front, blinder_rear, blinder_color, ret);
			}
		}
	}
	p_blinder_config_->ClearSurfaceId();

	return p_blinder_video;
}

/*****************************************************************************
 処理概要：	カメラ映像切替通知設定
 引数    ：	const bool	value	(i)カメラ映像切替通知 true:有効/false:無効
 戻り値  ：	なし
*****************************************************************************/
void CVhalMute::SetNotifyCameraChanged(const bool value)
{
	const std::lock_guard<std::mutex> lock_data{mtx_camera_changed_notify_};

	VHAL_LOGV_IN();

	camera_changed_notify_ = value;

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	カメラ映像切替通知
 引数    ：	const int32_t	result	(i)切替結果
 戻り値  ：	なし
*****************************************************************************/
void CVhalMute::NotifyCameraChanged(const int32_t result)
{
	const std::lock_guard<std::mutex> lock_data{mtx_camera_changed_notify_};

	VHAL_LOGV_IN();

	if (true == camera_changed_notify_)
	{
		std::unique_ptr<CVhalEventItemCameraChanged> p_event{std::make_unique<CVhalEventItemCameraChanged>()};
		if (nullptr == p_event)
		{
			VHAL_LOGE("Failed to create CVhalEventItemCameraChanged.");
		}
		else
		{
			p_event->SetName(std::string("Camera Changed Event"));
			p_event->SetData(p_mute_listener_, result);
			const int32_t	ret{p_event_route_->WriteEvent(p_event.get())};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("WriteEvent() error. ret=%d", ret);
				p_event = nullptr;
			}
			else
			{
				(void)p_event.release();
			}
		}
		/* 通知済のため通知無効設定 */
		camera_changed_notify_ = false;
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	タイマコールバック(カメラ開始用MUTE OFF)
 引数    ：	void *arg			(i)レイアウト制御インスタンスポインタ
 戻り値  ：	なし
*****************************************************************************/
extern "C" void MuteReleaseMuteCameraOn(void* const arg)
{
	/* サスペンド状態の場合は無処理 */
	if (videohal::CVhalStrManager::GetSuspend())
	{
		VHAL_LOGV("NOP callback.");
	}
	else
	{
		CVhalMute::ReleaseMuteCameraOn(arg);
	}
}

/*****************************************************************************
 処理概要：	タイマコールバック(カメラ終了用MUTE OFF)
 引数    ：	void *arg			(i)レイアウト制御インスタンスポインタ
 戻り値  ：	なし
*****************************************************************************/
extern "C" void MuteReleaseMuteCameraOff(void* const arg)
{
	/* サスペンド状態の場合は無処理 */
	if (videohal::CVhalStrManager::GetSuspend())
	{
		VHAL_LOGV("NOP callback.");
	}
	else
	{
		CVhalMute::ReleaseMuteCameraOff(arg);
	}
}


} /* namespace videohal */

