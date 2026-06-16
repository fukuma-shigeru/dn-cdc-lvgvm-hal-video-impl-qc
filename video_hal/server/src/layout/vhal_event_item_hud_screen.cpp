/*******************************************************************************
    機能名称    ：  画面HUD イベントモジュール
    ファイル名称：  vhal_event_item_screen_hud.cpp
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
 処理概要：	HUD機能有無判定結果通通知/HUD歪み補正パラメータ通知/HUD回転パラメータ通知受信
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
 処理概要：	スクリーンショット受信事前通知（通信モジュールスレッドからのコール）
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
*****************************************************************************/
void CVhalHudScreenReceiver::NotifyHudFunctionStatus(const std::vector<uint8_t>& data) noexcept
{
	VHAL_LOGV_IN();

	/* この関数内のみで使用するため可視範囲を最小化 */
	enum class HudFunctionStatus : uint8_t {
		no_func = 0x00U,
		func = 0x01U,
	};

	if (nullptr != p_hud_screen_controller_)
	{
		/* 受信データサイズの確認 */
		if ((hud_func_st_size_ + 1U) == data.size() && data.size() >= 2U) 
		{
			/* HUD機能有無判定結果(0：機能無 1：機能有) */
			const uint8_t hud_func_raw{data[1U]};
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
*****************************************************************************/
void CVhalHudScreenReceiver::NotifyHudDistortionCorrection(const std::vector<uint8_t>& data) noexcept
{
	VHAL_LOGV_IN();

	/* この関数内のみで使用するため可視範囲を最小化 */
	enum class HudBlackRequest : uint8_t {
		no_black = 0x00U,
		black = 0x01U,
	};

	if (nullptr != p_hud_screen_controller_)
	{
		/* 受信データサイズの確認 */
		if ((hud_distortion_correct_size_ + 1U) == data.size())
		{
			const uint8_t black_screen_req_raw{data[black_pos_]};
			/* 黒画表示要求なしの場合 */
			if (static_cast<uint8_t>(HudBlackRequest::no_black) == black_screen_req_raw)
			{
				wlrenderer::HudDistortionCorrection corrections{};
				size_t parse_index{1U};	/* データは opcode の次（index 1）から開始 */
				constexpr size_t payload_end{black_pos_};

				/* 8bitデータの読み込み(境界チェックあり)ラムダ式 */
				const auto read_u8{[&data, &parse_index, payload_end](uint8_t& out) noexcept -> bool
				{
					bool ret{true};
					if (parse_index >= payload_end)
					{
						ret = false;
					}
					else
					{
						out = data[parse_index];
						++parse_index;
					}
					return ret;
				}};
				/* 16bitデータの読み込み(境界チェックあり)ラムダ式 */
				const auto read_le16{[&data, &parse_index, payload_end](uint16_t& out) noexcept -> bool
				{
					bool ret{true};
					if ((parse_index + 1U) >= payload_end)
					{
						ret = false;
					}
					else
					{
						out = static_cast<uint16_t>(static_cast<uint16_t>(data[parse_index]) |
							(static_cast<uint16_t>(data[parse_index + 1U]) << 8U));
						parse_index += 2U;
					}
					return ret;
				}};

				if ((false == read_u8(corrections.gv_sys_hud_type)) ||			/* HUDタイプ */
					(false == read_u8(corrections.gv_sys_hud_size)) ||			/* HUDサイズ */
					(false == read_u8(corrections.gv_vipos_direction)) ||		/* 描画方向(意匠向き) */
					(false == read_le16(corrections.gv_vipos_resl_height)) ||	/* HUDTFT解像度 縦 */
					(false == read_le16(corrections.gv_vipos_resl_width)) ||	/* HUDTFT解像度 横 */
					(false == read_le16(corrections.gv_vipos_base_x)) ||		/* HUDTFT有効エリア 基準点x */
					(false == read_le16(corrections.gv_vipos_base_y)) ||		/* HUDTFT有効エリア 基準点y */
					(false == read_le16(corrections.gv_vipos_avail_height)) ||	/* HUDTFT有効エリア 縦 */
					(false == read_le16(corrections.gv_vipos_avail_width)))		/* HUDTFT有効エリア 横*/
				{
					/* 境界チェックエラーの場合は通知を破棄 */
					VHAL_LOGE("invalid data layout. size=%zu", data.size());
				}
				else
				{
					/* 16bitデータのLOOP読み込み(境界チェックあり)ラムダ式 */
					const auto read_coord_array{[&read_le16](auto& dest, const char* const label) noexcept -> bool
					{
						bool ret{true};
						for (size_t i{0U}; i < wlrenderer::kHudCoordinates; ++i)
						{
							if (false == read_le16(dest[i]))
							{
								VHAL_LOGE("invalid %s data. idx=%zu", label, i);
								ret = false;
							}
						}
						return ret;
					}};

					if (read_coord_array(corrections.gv_vipos_basept_x, "basept_x") &&		/* 画像標準値 x座標 point 1～15 */
						read_coord_array(corrections.gv_vipos_basept_y, "basept_y") &&		/* 画像標準値 y座標 point 1～15 */
						read_coord_array(corrections.gv_vipos_adjpt_x, "adjpt_x") &&		/* 画像補正値 x座標 point 1～15 */
						read_coord_array(corrections.gv_vipos_adjpt_y, "adjpt_y") &&		/* 画像補正値 y座標 point 1～15 */
						(parse_index == payload_end))
					{
						/* HUD歪み補正パラメータ設定 */
						p_hud_screen_controller_->ApplyHudDistortionCorrection(corrections, false);
					}
					else
					{
						/* 境界チェックエラーの場合は通知を破棄 */
						VHAL_LOGE("invalid data layout. remaining bytes=%zu", (payload_end - parse_index));
					}
				}
			}
			/* 黒画表示要求ありの場合 */
			else if (static_cast<uint8_t>(HudBlackRequest::black) == black_screen_req_raw)
			{
				wlrenderer::HudDistortionCorrection corrections{};
				p_hud_screen_controller_->ApplyHudDistortionCorrection(corrections, true);
			}
			/* 無効値の場合はHUD黒画要求を有効にする */
			else
			{
				VHAL_LOGE("unknown black_screen_req. black_screen_req=0x%02X", static_cast<u_int32_t>(black_screen_req_raw));
				wlrenderer::HudDistortionCorrection corrections{};
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
*****************************************************************************/
void CVhalHudScreenReceiver::NotifyHudRotation(const std::vector<uint8_t>& data) noexcept
{
	VHAL_LOGV_IN();

	if (nullptr != p_hud_screen_controller_)
	{
		/* 受信データサイズの確認 */
		if ((hud_rotation_size_ + 1U) == data.size())
		{
			const uint16_t hud_rot_deg{static_cast<uint16_t>(
				static_cast<uint16_t>(data[1]) |
				static_cast<uint16_t>(static_cast<uint16_t>(data[2]) << 8U))};	/* HUD回転角度(単位:Deg LSB:0.01) */
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

} /* namespace videohal */
