/*******************************************************************************
    機能名称    ：  HUD画面 制御モジュール
    ファイル名称：  vhal_hud_screen_controller.h
*******************************************************************************/
#ifndef VHAL_HUD_SCREEN_CONTROLLER_H
#define VHAL_HUD_SCREEN_CONTROLLER_H

#include <cstdint>

#include "wl_renderer_public.h"
#include "vhal_layout_mng.h"

namespace videohal
{

class CVhalHudScreenController {
public:
	CVhalHudScreenController(void) noexcept;
	~CVhalHudScreenController(void) noexcept = default;
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

	/* QNXからのHUD制御関連設定 */
	/* HUD機能有無判定結果設定 */
	void ApplyHudFunctionStatus(const bool func) noexcept;
	/* HUD歪み補正パラメータ設定 */
	void ApplyHudDistortionCorrection(const wlrenderer::HudDistortionCorrection& corrections, const bool black) noexcept;
	/* HUD回転パラメータ設定 */
	void ApplyHudRotation(const uint16_t rot_deg) noexcept;

private:
	CVhalLayoutManager* p_layout_{nullptr};
	wlrenderer::CWaylandRenderer* p_renderer_{nullptr};

	/* HUD機能有無判定結果 */
	bool hud_func_{false};
	/* HUD黒画入力要求フラグ (false:HUD黒画要求無し / true:HUD黒画要求有り) */
	bool black_screen_req_{true};
	/* HUDスクリーン有効判定結果 */
	bool hud_screen_available_{false};
	/* HUD MUTEサーフェス有効判定結果 */
	bool hud_mute_surface_available_{false};

	/* HUD歪み補正パラメータ保持用構造体 */
	class HudCorrectionsState {
	private:
		wlrenderer::HudDistortionCorrection value{};
		bool valid{false};

	public:
		void ClearCorrections(void) noexcept
		{
			value = wlrenderer::HudDistortionCorrection{};
			valid = false;
		}
		void SetCorrections(const wlrenderer::HudDistortionCorrection& corrections) noexcept
		{
			value = corrections;
			valid = true;
		}
		wlrenderer::HudDistortionCorrection GetCorrections(void) const noexcept
		{
			return value;
		}
		bool HasCorrections(void) const noexcept
		{
			return valid;
		}
	};
	HudCorrectionsState hud_corrections_;

	/* HUD回転パラメータ保持用構造体 */
	class HudRotationState {
	private:
		uint16_t value{};
		bool valid{false};

	public:
		void ClearRotation(void) noexcept
		{
			value = 0U;
			valid = false;
		}
		void SetRotation(const uint16_t hud_rot_deg) noexcept
		{
			value = hud_rot_deg;
			valid = true;
		}
		uint16_t GetRotation(void) const noexcept
		{
			return value;
		}
		bool HasRotation(void) const noexcept
		{
			return valid;
		}
	};
	HudRotationState hud_rotation_;

	/* HUDパラメータ設定エラーログは初回のみ出力する */
	bool set_distortion_log_once_{false};
	bool set_rotation_log_once_{false};

	/* HUD MUTEディスプレイサーフェス有効判定 */
	bool IsHudMuteDisplaySurfaceEnabled(void) noexcept;
	/* HUD黒画表示要否確認 (要:true/不要:false) */
	bool IsHudStatusEnableAndBlackNoRequested(void) const noexcept;
	/* HUD MUTEサーフェス設定 */
	void SetHudMuteSurfaceVisible(const bool enabled) noexcept;
	/* 通知パラメータ設定 共通処理 */
	void SetHudParametersCommon(void) noexcept;
	/* 保持しているHUD歪み補正パラメータ、HUD回転パラメータ設定をWaylandプラグインに設定 */
	int32_t SetStoredHudParametersToWaylandPlugin(void) noexcept;
};

} /* namespace videohal */

#endif /* VHAL_HUD_SCREEN_CONTROLLER_H */
