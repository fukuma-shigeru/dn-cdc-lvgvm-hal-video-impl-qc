/*******************************************************************************
    機能名称    ：  HUD画面 制御モジュール
    ファイル名称：  vhal_hud_screen_controller.h
*******************************************************************************/
#ifndef VHAL_HUD_SCREEN_CONTROLLER_H
#define VHAL_HUD_SCREEN_CONTROLLER_H

#include <cstdint>
#include <atomic>

#include "vhal_log.h"
#include "wl_renderer_public.h"
#include "vhal_layout_mng.h"

namespace videohal
{

/* HUD機能有無判定結果 */
enum class sys_hud_func : uint8_t {
	no_func	= 0x00U,	/* 機能無 */
	func	= 0x01U,	/* 機能有 */
};
enum class sys_hud_black_req : uint8_t {
	no_black	= 0x00U,	/* 黒画表示要求なし（HUD通常表示） */
	black		= 0x01U,	/* 黒画表示要求あり（HUD黒画表示） */
};

class CVhalHudScreenController {
public:
	CVhalHudScreenController(void) noexcept;
	~CVhalHudScreenController(void) noexcept;
	CVhalHudScreenController(const CVhalHudScreenController& src) = delete;
	CVhalHudScreenController& operator=(const CVhalHudScreenController& src) & = delete;
	CVhalHudScreenController(CVhalHudScreenController&& src) = delete;
	CVhalHudScreenController& operator=(CVhalHudScreenController&& src) & = delete;

	/* 初期化処理（内部リソースの確保） */
	int32_t Initialize(CVhalLayoutManager* const p_layout_mng, wlrenderer::CWaylandRenderer* const p_wayland_renderer) noexcept;
	/* 終了処理（内部リソースの解放）*/
	void Finalize(void) noexcept;

	/* HUD制御再初期化(STR(レジューム時)) */
	void ReInit(void) noexcept;
	/* HUDスクリーン有効判定 (false:無効 / true:有効) */
	bool IsHudScreenEnabled(void) const noexcept;

	/* QNXからのHUD制御関連設定 */
	/* HUD機能有無判定結果設定 */
	void ApplyHudFunctionStatus(const bool hud_func) noexcept;
	/* HUD歪み補正パラメータ設定 */
	void ApplyHudDistortionCorrection(const wlrenderer::HudDistortionCorrection& corrections, const bool black_screen_req) noexcept;
	/* HUD回転パラメータ設定 */
	void ApplyHudRotation(const uint16_t hud_rot_deg) noexcept;

private:
	CVhalLayoutManager* p_layout_mng_;
	wlrenderer::CWaylandRenderer* p_renderer_;

	/* HUD機能有無判定結果 */
	bool hud_func_;
	/* HUD黒画入力要求フラグ (false:HUD黒画要求無し / true:HUD黒画要求有り) */
	bool black_screen_req_;
	/* HUDスクリーン有効判定結果 */
	bool hud_screen_available_;
	/* HUD MUTEサーフェス有効判定結果 */
	bool hud_mute_surface_available_;

	/* HUD歪み補正パラメータ保持用構造体 */
	struct HudCorrectionsState {
		wlrenderer::HudDistortionCorrection value{};
		bool valid{false};
		void Clear(void) noexcept
		{
			value = wlrenderer::HudDistortionCorrection{};
			valid = false;
		}
		void Set(const wlrenderer::HudDistortionCorrection& corrections) noexcept
		{
			value = corrections;
			valid = true;
		}
		const wlrenderer::HudDistortionCorrection& Get(void) const noexcept
		{
			return value;
		}
	};
	HudCorrectionsState hud_corrections_;

	/* HUD回転パラメータ保持用構造体 */
	struct HudRotationState {
		uint16_t value{};
		bool valid{false};
		void Clear(void) noexcept
		{
			value = 0;
			valid = false;
		}
		void Set(const uint16_t hud_rot_deg) noexcept
		{
			value = hud_rot_deg;
			valid = true;
		}
		const uint16_t& Get(void) const noexcept
		{
			return value;
		}
	};
	HudRotationState hud_rotation_;

	/* HUDパラメータ設定エラーログは初回のみ出力する */
	bool set_distortion_log_once_;
	bool set_rotation_log_once_;

	/* HUD MUTEディスプレイサーフェス有効判定 */
	bool IsHudMuteDisplaySurfaceEnabled(void) noexcept;
	/* HUD黒画表示要否確認 (要:true/不要:false) */
	bool IsHudStatusEnableAndBlackNoRequested(void) const noexcept;
	/* HUD MUTEサーフェス設定 */
	void SetHudMuteSurfaceVisible(const bool enabled) noexcept;
	/* 保持しているHUD歪み補正パラメータ、HUD回転パラメータ設定をWaylandプラグインに設定 */
	int32_t SetStoredHudParametersToWaylandPlugin(void) noexcept;
};

} /* namespace videohal */

#endif /* VHAL_HUD_SCREEN_CONTROLLER_H */
