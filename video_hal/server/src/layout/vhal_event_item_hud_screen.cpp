/*******************************************************************************
    機能名称    ：  画面HUD イベントモジュール
    ファイル名称：  vhal_event_item_hud_screen.cpp
*******************************************************************************/
#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_event_item_hud_screen_event.h"
#include "vhal_micon_misc_opc.h"
#include "vhal_debug_system.h"

#include <cstdint>

namespace videohal
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalHudScreenReceiver::CVhalHudScreenReceiver(void) noexcept
{
}

/*****************************************************************************
	処理概要：	HUD機能有無判定結果通知/HUD歪み補正パラメータ通知/HUD回転パラメータ通知受信
 引数    ：	const std::vector<uint8_t>&	data	(i)	コマンド情報ポインタ
 戻り値  ：	なし
*****************************************************************************/
void CVhalHudScreenReceiver::Receive(const std::vector<uint8_t>& data)
{
	VHAL_LOGV_IN();

	/* データ要素数は3個未満(sub_type + result + data_type) の場合はエラー */
	constexpr std::size_t kRecvDataSizeMinRequired{3U};

	if (data.size() < kRecvDataSizeMinRequired)
	{
		VHAL_LOGE("parameter error. data.size is %zu", data.size());
	}
	else
	{
		const uint8_t last_byte{static_cast<uint8_t>(data[data.size() - 1U])};
		if (static_cast<uint8_t>(kDatatypeDisplay) == last_byte)
		{
			constexpr std::size_t recv_data_opc{kRecvDataOpc_};
			switch (data[recv_data_opc])
			{
				/* HUD機能有無判定結果通知 */
				case SUB_TYPE_DISP_HUD_FUNC_STATUS:
					NotifyHudFunctionStatus(data);
					break;
				/* HUD歪み補正パラメータ通知 */
				case SUB_TYPE_DISP_HUD_DISTORTION_CORRECTION:
					NotifyHudDistortionCorrection(data);
					break;
				/* HUD回転パラメータ通知 */
				case SUB_TYPE_DISP_HUD_ROTATION:
					NotifyHudRotation(data);
					break;
				/* スクリーンショット応答は無処理 */
				/* 画質モード応答は無処理 */
				case SUB_TYPE_DISP_SCREEN_SHOT_RSP:
				case SUB_TYPE_DISP_MODE_RSP:
					break;
				default:
					VHAL_LOGW("unknown sub_type. sub_type=0x%02X", static_cast<uint32_t>(data[recv_data_opc]));
					break;
			}
		}
		else
		{
			/* 無処理 */
		}
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	HUD制御通知受信事前通知（通信モジュールスレッドからのコール）
 引数    ：	const std::vector<uint8_t>&	data	(i)	コマンド情報ポインタ
 戻り値  ：	なし
*****************************************************************************/
void CVhalHudScreenReceiver::ReceivePreNotify(const std::vector<uint8_t>& data) noexcept
{
	static_cast<void>(data);
	/* 処理なし（Receiveで処理実施） */
}

/*****************************************************************************
 処理概要：	受信アイテム種別取得(Misc)
 引数    ：	なし
 戻り値  ：	受信アイテム種別
*****************************************************************************/
CVhalMiconReceiveItem::ReceiveItemType CVhalHudScreenReceiver::GetItemType(void) const noexcept
{
	return CVhalMiconReceiveItem::ReceiveItemType::RECEIVE_ITEM_TYPE_MISC;
}

/*****************************************************************************
 処理概要：	HUD表示コントローラ登録
 引数    ：	CVhalHudScreenController* const p_ctrl	(i) HUD表示コントローラポインタ
 戻り値  ：	なし
****************************************************************************/
int32_t CVhalHudScreenReceiver::RegisterHudScreenController(CVhalHudScreenController* const p_ctrl) noexcept
{
	VHAL_LOGV_IN();

	int32_t ret{VHAL_SUCCESS};

	if (nullptr == p_ctrl)
	{
		VHAL_LOGE("p_ctrl is null.");
		ret = VHAL_ERR_PARAM;
	}
	else
	{
		p_hud_screen_controller_ = p_ctrl;
	}

	VHAL_LOGV_OUT();

	return ret;
}

/*****************************************************************************
 処理概要：	HUDスクリーンコントローラ解除 
 引数    ：	なし
 戻り値  ：	なし
****************************************************************************/
void CVhalHudScreenReceiver::ClearHudScreenController(void) noexcept
{
	VHAL_LOGV_IN();

	p_hud_screen_controller_ = nullptr;

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	HUD機能有無判定結果通知
 引数    ：	const std::vector<uint8_t>& data	(i) 受信データ
 戻り値  ：	なし
 フェールセーフNo  ：	F-VHAL-N-XXX, F-VHAL-N-XXX
*****************************************************************************/
void CVhalHudScreenReceiver::NotifyHudFunctionStatus(const std::vector<uint8_t>& data) noexcept
{
	VHAL_LOGV_IN();

	if (nullptr != p_hud_screen_controller_)
	{
		/* 受信データサイズの確認 */
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret{0};
//		size_t data_size{0};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-XXX",fail_ret)};
//		if(true == fail)
//		{
//			data_size = static_cast<size_t>(fail_ret);
//		}
//#else
		const size_t data_size{static_cast<size_t>(data.size())};
//#endif
		if ((hud_func_st_size_ + 1U) == data_size) 
		{
			enum class HudFunctionStatus : uint8_t {
				no_func = 0x00U,
				func = 0x01U,
			};
			/* HUD機能有無判定結果(0：機能無 1：機能有) */
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//			fail_ret = 0;
//			uint8_t hud_func_raw{0};
//			bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-XXX",fail_ret)};
//			if(true == fail)
//			{
//				hud_func_raw = static_cast<uint8_t>(fail_ret);
//			}
//#else
			const uint8_t hud_func_raw{static_cast<uint8_t>(data[1U])};
//#endif
			switch (hud_func_raw)
			{
				/* 機能有り */
				case static_cast<uint8_t>(HudFunctionStatus::func):
					p_hud_screen_controller_->ApplyHudFunctionStatus(true);
					break;
				/* 機能無し */
				case static_cast<uint8_t>(HudFunctionStatus::no_func):
					p_hud_screen_controller_->ApplyHudFunctionStatus(false);
					break;
				/* データ内容が不正の場合はHUD機能無とする */
				default:
					VHAL_LOGE("unknown HUD function status. value=0x%02X", static_cast<uint32_t>(hud_func_raw));
					p_hud_screen_controller_->ApplyHudFunctionStatus(false);
					break;
			}
		}
		else
		{
			/* データサイズエラーの場合はHUD機能無とする */
			VHAL_LOGE("invalid data size. %zu", data.size());
			p_hud_screen_controller_->ApplyHudFunctionStatus(false);
		}
	}
	else
	{
		/* p_hud_screen_controller_ がNULLの場合は通知を破棄 */
		VHAL_LOGE("HUD screen controller is not registered.");
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	HUD歪み補正パラメータ通知
 引数    ：	const std::vector<uint8_t>& data	(i) 受信データ
 戻り値  ：	なし
 フェールセーフNo  ：	F-VHAL-N-XXX, F-VHAL-N-XXX, F-VHAL-N-XXX
*****************************************************************************/
void CVhalHudScreenReceiver::NotifyHudDistortionCorrection(const std::vector<uint8_t>& data) noexcept
{
	VHAL_LOGV_IN();

	if (nullptr != p_hud_screen_controller_)
	{
		/* 受信データサイズの確認 */
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM	// HUD歪み補正パラメータ通知(36h-44h)の通知サイズ異常(137byte以外)
//		int32_t fail_ret{0};
//		size_t data_size{0};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-XXX",fail_ret)};
//		if(true == fail)
//		{
//			data_size = static_cast<size_t>(fail_ret);
//		}
//#else
		const size_t data_size{static_cast<size_t>(data.size())};
//#endif
		if ((hud_distortion_correct_size_ + 1U) == data_size)
		{
			enum class HudBlackRequest : uint8_t {
				no_black = 0x00U,
				black = 0x01U,
			};
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM		// hUD歪み補正パラメータ通知(36h-44h)の黒画表示要求フィールド値が機黒画表示要求なし/あり以外
//			fail_ret = 0;
//			uint8_t black_screen_req_raw{0};
//			bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-XXX",fail_ret)};
//			if(true == fail)
//			{
//				black_screen_req_raw = static_cast<uint8_t>(fail_ret);
//			}
//#else
			const uint8_t black_screen_req_raw{static_cast<uint8_t>(data[black_pos_])};
//#endif
			/* 黒画表示要求なしの場合 */
			if (static_cast<uint8_t>(HudBlackRequest::no_black) == black_screen_req_raw)
			{
				wlrenderer::HudDistortionCorrection corrections{wlrenderer::HudDistortionCorrection()};
				size_t parse_index{1U};	/* データは opcode の次（index 1）から開始 */
				/* HUDタイプ */
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM	// HUD歪み補正パラメータ通知(36h-44h)の黒画表示要求フィールドが"黒画表示要求なしで、フィールドにreserved値、未確定値、無効値が設定
//				fail_ret = 0;
//				uint8_t black_screen_req_raw{0};
//				bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-XXX",fail_ret)};
//				if(true == fail//				{
//					corrections.gv_sys_hud_type = 0b1111;
//				}
//#else
				corrections.gv_sys_hud_type = AssembleLe8(data, parse_index);
//#endif
				parse_index++;
				/* HUDサイズ */
				corrections.gv_sys_hud_size = AssembleLe8(data, parse_index);
				parse_index++;
				/* 描画方向(意匠向き) */
				corrections.gv_vipos_direction = AssembleLe8(data, parse_index);
				parse_index++;
				/* HUDTFT解像度 縦 */
				corrections.gv_vipos_resl_height = AssembleLe16(data, parse_index);
				parse_index += 2U;
				/* HUDTFT解像度 横 */
				corrections.gv_vipos_resl_width = AssembleLe16(data, parse_index);
				parse_index += 2U;
				/* HUDTFT有効エリア 基準点x */
				corrections.gv_vipos_base_x = AssembleLe16(data, parse_index);
				parse_index += 2U;
				/* HUDTFT有効エリア 基準点y */
				corrections.gv_vipos_base_y = AssembleLe16(data, parse_index);
				parse_index += 2U;
				/* HUDTFT有効エリア 縦 */
				corrections.gv_vipos_avail_height = AssembleLe16(data, parse_index);
				parse_index += 2U;
				/* HUDTFT有効エリア 横 */
				corrections.gv_vipos_avail_width = AssembleLe16(data, parse_index);
				parse_index += 2U;
				/* 画像標準値 x座標 point 1～15 */
				for (size_t i{0U}; i < wlrenderer::kHudCoordinates; ++i)
				{
					corrections.gv_vipos_basept_x[i] = AssembleLe16(data, i + parse_index + (i * 2U));
				}
				parse_index += (wlrenderer::kHudCoordinates * 2U);
				/* 画像標準値 y座標 point 1～15 */
				for (size_t i{0U}; i < wlrenderer::kHudCoordinates; ++i)
				{
					corrections.gv_vipos_basept_y[i] = AssembleLe16(data, i + parse_index + (i * 2U));
				}
				parse_index += (wlrenderer::kHudCoordinates * 2U);
				/* 画像補正値 x座標 point 1～15 */
				for (size_t i{0U}; i < wlrenderer::kHudCoordinates; ++i)
				{
					corrections.gv_vipos_adjpt_x[i] = AssembleLe16(data, i + parse_index + (i * 2U));
				}
				parse_index += (wlrenderer::kHudCoordinates * 2U);
				/* 画像補正値 y座標 point 1～15 */
				for (size_t i{0U}; i < wlrenderer::kHudCoordinates; ++i)
				{
					corrections.gv_vipos_adjpt_y[i] = AssembleLe16(data, i + parse_index + (i * 2U));
				}

				/* HUD歪み補正パラメータ設定 */
				p_hud_screen_controller_->ApplyHudDistortionCorrection(corrections, false);
			}
			/* 黒画表示要求ありの場合 */
			else if (static_cast<uint8_t>(HudBlackRequest::black) == black_screen_req_raw)
			{
				constexpr wlrenderer::HudDistortionCorrection corrections{wlrenderer::HudDistortionCorrection()};
				p_hud_screen_controller_->ApplyHudDistortionCorrection(corrections, true);
			}
			/* 無効値の場合はHUD黒画要求を有効にする */
			else
			{
				VHAL_LOGE("unknown black_screen_req. black_screen_req=0x%02X", static_cast<uint32_t>(black_screen_req_raw));
				constexpr wlrenderer::HudDistortionCorrection corrections{wlrenderer::HudDistortionCorrection()};
				p_hud_screen_controller_->ApplyHudDistortionCorrection(corrections, true);
			}
		}
		/* データサイズエラーの場合は通知を破棄 */
		else
		{
			VHAL_LOGE("invalid data size. %zu", data.size());
		}
	}
	/* p_hud_screen_controller_ がNULLの場合は通知を破棄 */
	else
	{
		VHAL_LOGE("HUD screen controller is not registered.");
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	HUD回転パラメータ通知
 引数    ：	const std::vector<uint8_t>& data	(i) 受信データ
 戻り値  ：	なし
 フェールセーフNo  ：	F-VHAL-N-XXX, F-VHAL-N-XXX
*****************************************************************************/
void CVhalHudScreenReceiver::NotifyHudRotation(const std::vector<uint8_t>& data) noexcept
{
	VHAL_LOGV_IN();

	if (nullptr != p_hud_screen_controller_)
	{
		/* 受信データサイズの確認 */
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM	// HUD回転パラメータ通知(36h-45h)の通知サイズ異常(3byte以外)
//		int32_t fail_ret{0};
//		size_t data_size{0};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-XXX",fail_ret)};
//		if(true == fail)
//		{
//			data_size = static_cast<size_t>(fail_ret);
//		}
//#else
		const size_t data_size{static_cast<size_t>(data.size())};
//#endif
		if ((hud_rotation_size_ + 1U) == data_size)
		{
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM	// HUD回転パラメータ通知(36h-45h)のHUD回転角度フィールドに-2550～2550、0以外の値が設定
//			fail_ret = 0;
//			uint16_t hud_rot_deg{0};
//			bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-XXX",fail_ret)};
//			if(true == fail)
//			{
//				hud_rot_deg = static_cast<int16_t>(fail_ret);
//			}
//#else
			const uint16_t hud_rot_deg{AssembleLe16(data, 1U)};	/* HUD回転角度(単位:Deg LSB:0.01) */
//#endif
			p_hud_screen_controller_->ApplyHudRotation(hud_rot_deg);
		}
		/* データサイズエラーの場合は通知を破棄 */
		else
		{
			VHAL_LOGE("invalid data size. %zu", data.size());
		}
	}
	/* p_hud_screen_controller_ がNULLの場合は通知を破棄 */
	else
	{
		VHAL_LOGE("HUD screen controller is not registered.");
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	16bitデータ取得
 引数    ：	const std::vector<uint8_t>& data	(i) 受信データ
			const size_t idx					(i) インデックス
 戻り値  ：	16bitデータ
*****************************************************************************/
uint16_t CVhalHudScreenReceiver::AssembleLe16(const std::vector<uint8_t>& data, const size_t idx) noexcept
{
	uint32_t assembled{0U};

	if ((false == data.empty()) &&
		((data.size() - 1U) > idx))
	{
		const uint8_t l_byte{static_cast<uint8_t>(data[idx])};
		const uint8_t h_byte{static_cast<uint8_t>(data[idx + 1U])};
		assembled = static_cast<uint32_t>(l_byte) | (static_cast<uint32_t>(h_byte) << 8U);
    }
	else
	{
		VHAL_LOGE("data size error %zu", data.size());
    	assembled = 0U;
	}

	return static_cast<uint16_t>(assembled);
}

/*****************************************************************************
 処理概要：	8bitデータ取得
 引数    ：	const std::vector<uint8_t>& data	(i) 受信データ
			const size_t idx					(i) インデックス
 戻り値  ：	8bitデータ
*****************************************************************************/
uint8_t CVhalHudScreenReceiver::AssembleLe8(const std::vector<uint8_t>& data, const size_t idx) noexcept
{
	uint8_t assembled{0U};

	if (data.size() > idx)
	{
		assembled = static_cast<uint8_t>(data[idx]);
    }
	else
	{
		VHAL_LOGE("data size error %zu", data.size());
    	assembled = 0U;
	}

	return assembled;
}

} /* namespace videohal */
