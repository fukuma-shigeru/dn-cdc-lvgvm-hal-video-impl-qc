/*******************************************************************************
    機能名称    ：  HUD画面 制御モジュール
    ファイル名称：  vhal_hud_screen_controller.cpp
*******************************************************************************/
#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_hud_screen_controller.h"

namespace videohal
{
/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalHudScreenController::CVhalHudScreenController(void) noexcept
{
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ：	CVhalLayoutManager * const p_layout_mng					(i)	レイアウトマネージャインスタンスポインタ
 			wlrenderer::CWaylandRenderer * const p_wayland_renderer	(i)	WaylandRendererインスタンスポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_****		エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalHudScreenController::Initialize(CVhalLayoutManager * const p_layout_mng, wlrenderer::CWaylandRenderer* const p_wayland_renderer) noexcept
{
	VHAL_LOGV_IN();

	int32_t ret{VHAL_SUCCESS};

	if ((nullptr != p_layout_mng) && (nullptr != p_wayland_renderer))
	{
		p_layout_ = p_layout_mng;
		p_renderer_ = p_wayland_renderer;
		/* HUDスクリーン有効判定結果を取得して保持 */
		hud_screen_available_ = IsHudScreenEnabled();
	}
	else
	{
		VHAL_LOGE("parameter error. p_layout_mng=%p p_wayland_renderer=%p",
			static_cast<const void*>(p_layout_mng), static_cast<const void*>(p_wayland_renderer));
		p_layout_ = nullptr;
		p_renderer_ = nullptr;
		ret = VHAL_ERR_PARAM;
	}

	VHAL_LOGV_OUT();

	return ret;
}

/*****************************************************************************
 処理概要：	終了処理（内部リソースの解放）
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalHudScreenController::Finalize(void) noexcept
{
	VHAL_LOGV_IN();

	p_layout_ = nullptr;
	p_renderer_ = nullptr;

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	HUD制御再初期化(STR(レジューム時))
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalHudScreenController::ReInit(void) noexcept
{
	VHAL_LOGV_IN();

	/* HUD機能有無判定結果(機能無にする) */
	hud_func_ = false;
	/* HUD黒画表示要求フラグ(黒画表示要求有にする) */
	black_screen_req_ = true;
	/* HUD歪み補正パラメータ */
	hud_corrections_.ClearCorrections();
	/* HUD回転パラメータ */
	hud_rotation_.ClearRotation();
	/* HUD歪み補正パラメータ設定エラーログは初回のみ出力するためのフラグ */
	set_distortion_log_once_ = false;
	/* HUD回転パラメータ設定エラーログは初回のみ出力するためのフラグ */
	set_rotation_log_once_ = false;


	/* HUDスクリーン有効判定結果(hud_screen_available_)、
		HUD MUTEサーフェス有効判定結果(hud_mute_surface_available_)は
		STR(サスペンド)になっても保持されているので初期化はしない */

	VHAL_LOGV_OUT();
}

/*****************************************************************************	
 処理概要：	HUD機能有無判定結果設定 
 引数    ：	const bool func	(i)	HUD機能有無判定結果 (有:true/無:false)
 戻り値  ：	なし
*****************************************************************************/
void CVhalHudScreenController::ApplyHudFunctionStatus(const bool func) noexcept
{
	VHAL_LOGV_IN();

	/* HUDスクリーン有効 */
	if (true == hud_screen_available_)
	{
		hud_func_ = func;		/* HUD機能有無判定結果設定を保持 */

		/* HUD機能有、かつ黒画表示要求無の場合 */
		if (true == IsHudStatusEnableAndBlackNoRequested())
		{
			/* 保持しているHUD歪み補正パラメータ、HUD回転パラメータ設定をWaylandプラグインに設定 */
			const int32_t ret{SetStoredHudParametersToWaylandPlugin()};
			if (WL_RENDERER_SUCCESS == ret)
			{
				/* HUD MUTEサーフェス設定(非表示) */
				SetHudMuteSurfaceVisible(false);
			}
			/* Waylandプラグイン設定失敗の場合は、HUD MUTEサーフェス設定(表示) */
			else
			{
				/* HUD MUTEサーフェス設定(表示) */
				SetHudMuteSurfaceVisible(true);		
			}
		}
		/* HUD機能無、又は黒画表示要求有の場合 */
		else
		{	/* HUD MUTEサーフェス設定(表示) */
			SetHudMuteSurfaceVisible(true);		
		}
	}
	/* HUDスクリーン無効の場合は、設定をスキップ */
	else
	{
		VHAL_LOGE("HUD screen is disabled. skip applying HUD function status.");
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************	
 処理概要：	HUD歪み補正パラメータ設定
 引数    ：	const wlrenderer::HudDistortionCorrection& corrections	(i)	HUD歪み補正パラメータ構造体
			const bool black										(i)	HUD黒画表示要求フラグ  (黒画表示要求有:true/黒画表示要求無:false)
 戻り値  ：	なし
*****************************************************************************/
void CVhalHudScreenController::ApplyHudDistortionCorrection(const wlrenderer::HudDistortionCorrection& corrections, const bool black) noexcept
{
	VHAL_LOGV_IN();

	/* HUDスクリーン有効 */
	if (true == hud_screen_available_)
	{
		if (nullptr != p_renderer_)
		{
			/* HUD黒画表示要求フラグ設定を保持 */
			black_screen_req_ = black;

			if (false == black_screen_req_)
			{
				/* HUD歪み補正パラメータを保持 */
				hud_corrections_.SetCorrections(corrections);
			}
			/* HUD機能有、かつ黒画表示要求無の場合 */
			if (true == IsHudStatusEnableAndBlackNoRequested())
			{
				/* WaylandプラグインにHUD歪み補正パラメータを設定 */
				const int32_t ret{SetStoredHudParametersToWaylandPlugin()};
				if (WL_RENDERER_SUCCESS == ret)
				{
					/* HUD MUTEサーフェス設定(非表示) */
					SetHudMuteSurfaceVisible(false);
				}
				/* Waylandプラグイン設定失敗の場合は、HUD MUTEサーフェス設定(表示) */
				else
				{
					/* HUD MUTEサーフェス設定(表示) */
					SetHudMuteSurfaceVisible(true);		
				}
			}
			/* HUD機能無、又は黒画表示要求有の場合 */
			else
			{
			   /* HUD MUTEサーフェス設定(表示) */
				SetHudMuteSurfaceVisible(true);

				/* HUD歪み補正パラメータは保持したままにして、
				   HUD機能有、かつ黒画表示要求無になった時にWayandプラグインに設定する */
			}
		}
		else
		{
			VHAL_LOGE("renderer is not initialized");
		}
	}
	/* HUDスクリーン無効の場合は、設定をスキップ */
	else
	{
		VHAL_LOGE("HUD screen is disabled. skip applying HUD distortion correction.");
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	HUD回転パラメータ設定
 引数    ：	const uint16_t rot_deg	(i)	HUD回転角度(単位:Deg LSB:0.01)	
 戻り値  ：	なし
*****************************************************************************/
void CVhalHudScreenController::ApplyHudRotation(const uint16_t rot_deg) noexcept
{
	VHAL_LOGV_IN();

	/* HUDスクリーン有効 */
	if (true == hud_screen_available_)
	{
		if (nullptr != p_renderer_)
		{
			/* HUD回転パラメータ設定を保持 */
			hud_rotation_.SetRotation(rot_deg);

			/* HUD機能有、かつ黒画表示要求無の場合 */
			if (true == IsHudStatusEnableAndBlackNoRequested())
			{
				/* WaylandプラグインにHUD回転パラメータを設定 */
				const int32_t ret{SetStoredHudParametersToWaylandPlugin()};
				/* Waylandプラグイン設定成功の場合は、HUD MUTEサーフェス設定(非表示) */
				if (WL_RENDERER_SUCCESS == ret)
				{
					/* HUD MUTEサーフェス設定(非表示) */
					SetHudMuteSurfaceVisible(false);
				}
				/* Waylandプラグイン設定失敗の場合は、HUD MUTEサーフェス設定(表示) */
				else
				{
					/* HUD MUTEサーフェス設定(表示) */
					SetHudMuteSurfaceVisible(true);
				}
			}
			/* HUD機能無、又は黒画表示要求有の場合は、HUD回転パラメータを保持したままにして、
			   HUD機能有、かつ黒画表示要求無になった時にWayandプラグインに設定する */
		}
		else
		{
			VHAL_LOGE("renderer is not initialized");
		}
	}
	/* HUDスクリーン無効の場合は、設定をスキップ */
	else
	{
		VHAL_LOGE("HUD screen is disabled. skip applying HUD rotation.");
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	HUDスクリーン有効判定
 引数    ：	なし
 戻り値  ：	HUDスクリーン判定結果(有効:true/無効:false)
*****************************************************************************/
bool CVhalHudScreenController::IsHudScreenEnabled(void) const noexcept
{
	VHAL_LOGV_IN();

	bool available{false};

	if (nullptr != p_layout_)
	{
		int32_t screen_id_hud{-1};
		/* HUDスクリーンID取得 */
		const int32_t ret{p_layout_->GetScreenIdHud(screen_id_hud)};
		if (VHAL_SUCCESS == ret)
		{
			/* スクリーン有効判定 */
			available = p_layout_->IsScreenAvailable(screen_id_hud);
		}
		else
		{
			VHAL_LOGI("GetScreenIdHud nothing ret=%d", ret);
		}
	}
	else
	{
		VHAL_LOGE("layout manager is not initialized");
	}

	VHAL_LOGV_OUT();

	return available;
}

/*****************************************************************************	
 処理概要：	HUD MUTEディスプレイサーフェス有効判定 
 引数    ：	なし
 戻り値  ：	有効な場合:true / 無効な場合:false
*****************************************************************************/
bool CVhalHudScreenController::IsHudMuteDisplaySurfaceEnabled(void) noexcept
{
	VHAL_LOGV_IN();

	bool available{false};

	if (nullptr != p_layout_)
	{
		/* 前回までHUD MUTEディスプレイサーフェスが無効だった場合のみ確認 */
		if (false == hud_mute_surface_available_)
		{
			/* HUD MUTEディスプレイサーフェスが生成されたかチェック */
			int32_t hud_surface_id{-1};
			const int32_t ret{p_layout_->GetBlinderID(VhalBlinderType::VHAL_BLINDER_TYPE_HUD_DISPLAY, hud_surface_id)};
			if (VHAL_SUCCESS == ret)
			{
				available = p_layout_->IsValidSurfaceIdAvailable(hud_surface_id);
				/* HUD MUTEディスプレイサーフェス有効判定結果を更新して保持 */
				hud_mute_surface_available_ = available;
			}
			else
			{
				/* HUD MUTEディスプレイサーフェス有効判定結果を更新して保持 */
				hud_mute_surface_available_ = false;
				VHAL_LOGI("GetBlinderID nothing ret=%d", ret);
			}
		}
		else
		{
			/* HUD MUTEディスプレイサーフェス生成後は削除されることが無い */
			/* 処理コストを抑える為にチェックしない */
			available = true;
		}
	}
	else
	{
		VHAL_LOGE("layout manager is not initialized");
	}

	VHAL_LOGV_OUT();

	return available;
}

/*****************************************************************************
 処理概要：	HUD黒画表示要否確認
 引数    ：	なし
 戻り値  ：	HUD黒画表示要否 (要:true/不要:false)
*****************************************************************************/
bool CVhalHudScreenController::IsHudStatusEnableAndBlackNoRequested(void) const noexcept
{
	VHAL_LOGV_IN();

	/* HUD機能有、かつ黒画表示要求なしの場合、trueを返す */
	const bool ret{(true == hud_func_) && (false == black_screen_req_)};

	VHAL_LOGV_OUT();

	return ret;
}

/*****************************************************************************
 処理概要：	HUD MUTEサーフェス設定
 引数    ：	const bool enabled	(i)	HUD MUTEサーフェス有効フラグ
 戻り値  ：	なし
*****************************************************************************/
void CVhalHudScreenController::SetHudMuteSurfaceVisible(const bool enabled) noexcept
{
	VHAL_LOGV_IN();

	if (nullptr != p_layout_)
	{
		/* 	HUD MUTEサーフェス生成時の表示/非表示を設定する */
 		/*	表示/非表示設定はサーフェス状態変化通知でおこなっている為、 */
		/*	HUD MUTEサーフェス生成済で、サーフェス状態変化通知が来ていない可能性がある */
		/*	上記のタイミングを考慮して、HUD MUTEサーフェス生成/未生成に関わらず常に設定する */
		p_layout_->SetBlinderHudDispMuteInit(enabled);

		/* HUD MUTEディスプレイサーフェス有効判定 */
		if (true == IsHudMuteDisplaySurfaceEnabled())
		{
			/* HUD MUTEディスプレイサーフェスの表示/非表示設定 */
			const int32_t result{p_layout_->SetBlinderEnable(VhalBlinderType::VHAL_BLINDER_TYPE_HUD_DISPLAY, enabled)};
			if (VHAL_SUCCESS != result)
			{
				VHAL_LOGE("SetBlinderEnable NG result=%d enabled=%s", result, (enabled ? "1" : "0"));
			}
		}
	}
	else
	{
		VHAL_LOGE("layout manager is not initialized");
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	保持しているHUD歪み補正パラメータ、HUD回転パラメータ設定をWaylandプラグインに設定 
 引数    ：	なし
 戻り値  ：	処理結果
            	WL_RENDERER_SUCCESS		正常終了
            	WL_RENDERER_SUCCESS以外	異常終了
*****************************************************************************/
int32_t CVhalHudScreenController::SetStoredHudParametersToWaylandPlugin(void) noexcept
{
	VHAL_LOGV_IN();

	int32_t ret{WL_RENDERER_SUCCESS};

	if (nullptr != p_renderer_)
	{
		/* 保持済みのHUD歪み補正パラメータがあれば、Waylandプラグインに設定 */
		if (true == hud_corrections_.HasCorrections())
		{
			const wlrenderer::HudDistortionCorrection corrections{hud_corrections_.GetCorrections()};
			ret = p_renderer_->SetHudDistortionCorrection(corrections);
			if (WL_RENDERER_SUCCESS != ret)
			{
				/* エラーは初回の１回のみログ出力 */
				if (false == set_distortion_log_once_)
				{
					VHAL_LOGE("NG ret=%d", ret);
					set_distortion_log_once_ = true;
				}
			}
			/* 成功、失敗に関わらず保持しているHUD歪み補正パラメータをクリア */
			hud_corrections_.ClearCorrections();
		}

		if (WL_RENDERER_SUCCESS == ret)
		{
			/* 保持済みのHUD回転パラメータがあれば、Waylandプラグインに設定 */
			if (true == hud_rotation_.HasRotation())
			{
				const uint16_t rotation{hud_rotation_.GetRotation()};
				ret	= p_renderer_->SetHudRotation(rotation);
				if (WL_RENDERER_SUCCESS != ret)
				{
					/* エラーは初回の１回のみログ出力 */
					if (false == set_rotation_log_once_)
					{
						VHAL_LOGE("NG ret=%d", ret);
						set_rotation_log_once_ = true;
					}
				}
				/* 成功、失敗に関わらず保持しているHUD回転パラメータをクリア */
				hud_rotation_.ClearRotation();
			}
		}
	}
	else
	{
		ret = WL_RENDERER_ERR;
		VHAL_LOGE("renderer is not initialized");
	}

	VHAL_LOGV_OUT();

	return ret;
}

} /* namespace videohal */
