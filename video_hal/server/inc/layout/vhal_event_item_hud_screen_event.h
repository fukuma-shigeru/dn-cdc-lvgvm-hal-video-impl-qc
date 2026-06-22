/*******************************************************************************
    機能名称    ：  HUD画面 イベントモジュール
    ファイル名称：  vhal_event_item_hud_screen_event.h
*******************************************************************************/
#ifndef	VHAL_EVENT_ITEM_HUD_SCREEN_EVENT_H
#define	VHAL_EVENT_ITEM_HUD_SCREEN_EVENT_H

#include <cstddef>
#include <cstdint>
#include <vector>

#include "vhal_micon_receive_item.h"
#include "vhal_hud_screen_controller.h"

namespace videohal
{

/*****************************************************************************
 クラス名称：CVhalHudScreenReceiver
 処理概要  ：HUD制御（機能有無/歪み補正/回転）通知受信イベントクラス
*****************************************************************************/
class CVhalHudScreenReceiver : public CVhalMiconReceiveItem {
public:
	CVhalHudScreenReceiver(void) noexcept;
	~CVhalHudScreenReceiver(void) noexcept override = default;
	CVhalHudScreenReceiver(const CVhalHudScreenReceiver& src) = delete;
	CVhalHudScreenReceiver& operator=(const CVhalHudScreenReceiver& src) & = delete;
	CVhalHudScreenReceiver(CVhalHudScreenReceiver&& src) = delete;
	CVhalHudScreenReceiver& operator=(CVhalHudScreenReceiver&& src) & = delete;

	/* HUDスクリーンコントローラ登録 */
	int32_t RegisterHudScreenController(CVhalHudScreenController* const p_ctrl) noexcept;
	/* HUDスクリーンコントローラ解除 */
	void ClearHudScreenController(void) noexcept;
	/* HUD機能有無判定結果通知/HUD歪み補正パラメータ通知/HUD回転パラメータ通知受信 */
	void Receive(const std::vector<uint8_t>& data) override;
	/* 受信事前通知(無処理) */
	void ReceivePreNotify(const std::vector<uint8_t>& data) noexcept override;
	/* 受信アイテム種別取得(Misc) */
	ReceiveItemType GetItemType(void) const noexcept override;

private:
	static constexpr size_t 	kRecvDataOpc_{0U};					/* 通知データのサブタイプ位置 */
	static constexpr size_t 	black_pos_{136U};					/* 黒画表示要求のデータ位置 */
	static constexpr size_t		hud_func_st_size_{2U};				/* HUD機能有無判定結果通知のデータサイズ */
	static constexpr size_t		hud_distortion_correct_size_{137U};	/* HUD歪み補正パラメータ通知のデータサイズ */
	static constexpr size_t		hud_rotation_size_{3U};				/* HUD回転パラメータ通知のデータサイズ */

	CVhalHudScreenController*    p_hud_screen_controller_{nullptr};  /* HUDスクリーンコントローラ */

	/* HUD機能有無判定結果通知 */
	void NotifyHudFunctionStatus(const std::vector<uint8_t>& data) noexcept;
	/* HUD歪み補正パラメータ通知 */
	void NotifyHudDistortionCorrection(const std::vector<uint8_t>& data) noexcept;
	/* HUD回転パラメータ通知 */
	void NotifyHudRotation(const std::vector<uint8_t>& data) noexcept;

	/* 16bitデータ取得 */
	static uint16_t AssembleLe16(const std::vector<uint8_t>& data, const size_t idx) noexcept;
	/* 8bitデータ取得 */
	static uint8_t  AssembleLe8(const std::vector<uint8_t>& data, const size_t idx) noexcept;
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_EVENT_ITEM_HUD_SCREEN_EVENT_H */
