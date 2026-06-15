/*******************************************************************************
    機能名称    ：  MISC画質モード応答受信アイテムモジュール
    ファイル名称：  vhal_color_recv_item_quality.cpp
*******************************************************************************/
#include "vhal_define.h"
#include "vhal_log.h"

#include "vhal_color_recv_item_quality.h"
#include "vhal_color_adjust_item_quality.h"
#include "vhal_color_mng.h"
#include "vhal_debug_system.h"

#include <vector>

namespace videohal
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalColorAdjustMiscReceiver::CVhalColorAdjustMiscReceiver(void)
	:current_back_light_(true)
	,current_mute_front_disp_(true)
{
	VHAL_LOGV("CVhalColorAdjustMiscReceiver is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalColorAdjustMiscReceiver::~CVhalColorAdjustMiscReceiver(void)
{
	VHAL_LOGV("CVhalColorAdjustMiscReceiver is deleted. this=%p", this);
}

/*****************************************************************************
 処理概要：	画質モード応答受信処理（VideoHALイベントスレッドからのコール）
 引数    ：	const std::vector<uint8_t>&	data	(i)	コマンド情報ポインタ
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorAdjustMiscReceiver::Receive(const std::vector<uint8_t>& data)
{
	/* データ要素数は3個未満(data_type+sub_type+α) */
	if (kColorRcvMiscDataSizeMin > data.size())
	{
		VHAL_LOGE("parameter error. data.size is %lu", data.size());
	}
	else if (kDatatypeDisplay == data[data.size() - 1])
	{
		switch (data[kColorRcvDataOpc])
		{
			case SUB_TYPE_DISP_MODE_RSP:
				/* 画質モード応答 */
				ReceiveColorAdjustStatus(data);
				break;
			case SUB_TYPE_DISP_SCREEN_SHOT_RSP:				/* スクリーンショット応答は無処理 */
			case SUB_TYPE_DISP_HUD_FUNC_STATUS:				/* HUD機能有無判定結果通知は無処理 */
			case SUB_TYPE_DISP_HUD_DISTORTION_CORRECTION:	/* HUD歪み補正通知は無処理 */
			case SUB_TYPE_DISP_HUD_ROTATION:				/* HUD回転パラメータ通知は無処理 */
				break;
			default:
				VHAL_LOGE("sub_type error. value=0x%02X", data[kColorRcvDataOpc]);
				break;
		}
	}
	else
	{
		/* 無処理 */
	}
}

/*****************************************************************************
 処理概要：	MISCコマンド通知受信事前通知処理 (OS間通信コールバックからのコール）
 引数    ：	const std::vector<uint8_t>&	data	(i)	コマンド情報ポインタ
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorAdjustMiscReceiver::ReceivePreNotify(const std::vector<uint8_t>& data) noexcept
{
	/* 処理なし（Receiveで処理実施） */
}


/*****************************************************************************
 処理概要：	送信アイテム種別取得
 引数    ：	なし
 戻り値  ：	送信アイテム種別
*****************************************************************************/
CVhalMiconReceiveItem::ReceiveItemType CVhalColorAdjustMiscReceiver::GetItemType(void) const noexcept
{
	return CVhalMiconReceiveItem::ReceiveItemType::RECEIVE_ITEM_TYPE_MISC;
}

/*****************************************************************************
 処理概要：	画質モード応答受信データ処理
 引数    ：	const std::vector<uint8_t>& recv_data	画質モード応答データ
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorAdjustMiscReceiver::ReceiveColorAdjustStatus(const std::vector<uint8_t>& recv_data)
{
	/* データ要素数は送信データ最大値と同じ */
	if ( static_cast<size_t>(CVhalColorAdjustItemQuality::AdjustItemNo::POS_MAX) > recv_data.size())
	{
		VHAL_LOGE("parameter error. data.size is %lu", recv_data.size());
	}
	else
	{
		static std::vector<uint8_t>	prev_micon_recv_data_{};
		uint8_t recv_vtype      {recv_data[static_cast<uint8_t>(CVhalColorAdjustItemQuality::AdjustItemNo::POS_VTYPE_MUTE)]};
		uint8_t recv_contrast   {recv_data[static_cast<uint8_t>(CVhalColorAdjustItemQuality::AdjustItemNo::POS_CONTRAST)]};
		uint8_t recv_brightness {recv_data[static_cast<uint8_t>(CVhalColorAdjustItemQuality::AdjustItemNo::POS_BRIGHTNESS)]};
		uint8_t recv_vdisp      {recv_data[static_cast<uint8_t>(CVhalColorAdjustItemQuality::AdjustItemNo::POS_VDISP)]};

		/* 映像種別(4bit)、ミュート状態(1bit) の取得、バックライト(3bit) の取得 */
		uint8_t video_type      {GetByteData(recv_vtype, 4U, 4U)};
		uint8_t mute_value      {GetByteData(recv_vtype, 1U, 1U)};
		uint8_t backlight_value {GetByteData(recv_vtype, 1U, 3U)};
	//#ifdef VHAL_SUPPORT_FAIL_SYSTEM	// F-VHAL-R-345
	//			int32_t value{0};
	//		CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-R-345",value);
	//			static uint8_t count = 0;
	//			if (count == 0)
	//			{
	//				switch (value)
	//				{
	//				case 0:
	//						count += 1;
	//						break;
	//				case 1:
	//					video_type = 8U; /* 映像種別応答データ範囲外 */
	//					count += 1;
	//					VHAL_LOGW("fail F-VHAL-R-345");
	//					break;
	//				case 2:
	//					backlight_value = 1U; /* 画面消し応答データ範囲外 */
	//					count += 1;
	//					VHAL_LOGW("fail F-VHAL-R-345");
	//					break;
	//				case 3:
	//					recv_brightness = 64U; /* 明るさ応答データ範囲外 */
	//					count += 1;
	//					VHAL_LOGW("fail F-VHAL-R-345");
	//					break;
	//				case 4:
	//					recv_contrast = 64U; /* コントラスト応答データ範囲外 */
	//					count += 1;
	//					VHAL_LOGW("fail F-VHAL-R-345");
	//					break;
	//				case 5:
	//					recv_vdisp = 8U; /* 映像表示モード応答データ範囲外 */
	//					count += 1;
	//					VHAL_LOGW("fail F-VHAL-R-345");
	//					break;
	//				default:
	//					break;
	//				}
	//			}
	//#endif
		bool recv_error{false};
		/* 映像種別範囲チェック */
		if ((static_cast<uint8_t>(CVhalColorAdjustItemQuality::VideoType::VTYPE_OTHER) > video_type) || ((static_cast<uint8_t>(CVhalColorAdjustItemQuality::VideoType::VTYPE_MULTISENSORY) < video_type)))
		{
			VHAL_LOGE("Receive parameter error. vtype=0x%x", video_type);
			recv_error = true;
		}
		/* 画面消し値チェック 0固定想定 */
		if (0U != backlight_value)
		{
			VHAL_LOGE("Receive parameter error. backlight=%u", backlight_value);
			recv_error = true;
		}
		/* 明るさ範囲チェック */
		if ((static_cast<uint8_t>(CVhalColorManager::ColorStepNum::COLOR_STEP_NUM_MIN) > recv_brightness) || ((static_cast<uint8_t>(CVhalColorManager::ColorStepNum::COLOR_STEP_NUM_MAX) < recv_brightness)))
		{
			VHAL_LOGE("Receive parameter error. bright=%u", recv_brightness); 
			recv_error = true;
		}
		/* コントラスト範囲チェック */
		if ((static_cast<uint8_t>(CVhalColorManager::ColorStepNum::COLOR_STEP_NUM_MIN) > recv_contrast) || (static_cast<uint8_t>(CVhalColorManager::ColorStepNum::COLOR_STEP_NUM_MAX) < recv_contrast))
		{
			VHAL_LOGE("Receive parameter error. contrast=%u", recv_contrast);
			recv_error = true;
		}
		/* 映像表示モード範囲チェック */
		if ((static_cast<uint8_t>(CVhalColorAdjustItemQuality::VideoDispMode::VDISP_NORMAL_DAY) > recv_vdisp) || (static_cast<uint8_t>(CVhalColorAdjustItemQuality::VideoDispMode::VDISP_COLOR_SELECT_DARK) < recv_vdisp))
		{
			VHAL_LOGE("Receive parameter error. vdisp=0x%x", recv_vdisp);
			recv_error = true;
		}

		if (false == recv_error)
		{
			/* 応答データが前回と異なる場合logを出力する */
			if (prev_micon_recv_data_ != recv_data) 
			{
				/* (受信 + 内部)データのログ出力 ※[recv：画質モード応答データ] [current：VideoHAL現在値] */
				VHAL_LOGI("[recv size=%lu opc=0x%x vtype=0x%x(mute=%u backlight=%u) contrast=%u bright=%u vdisp=0x%x] [current mute=%d backlight=%d]", 
					recv_data.size()-1,
					recv_data[static_cast<uint8_t>(CVhalColorAdjustItemQuality::AdjustItemNo::POS_OPC)], 
					recv_vtype,
					mute_value,
					backlight_value,
					recv_contrast,
					recv_brightness,
					recv_vdisp,
					current_mute_front_disp_,
					current_back_light_);
				/* 内部値の更新 */
				prev_micon_recv_data_.assign(recv_data.begin(), recv_data.end());
			}
		}
	} 
}

/*****************************************************************************
 処理概要：	現在のバックライト設定（propertyによる設定値）
 引数    ：	bool light		(i)バックライト点灯/消灯
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorAdjustMiscReceiver::SetCurrentBackLight(const bool light)
{
	current_back_light_ = light;
}

/*****************************************************************************
 処理概要：	現在の前席ディスプレイMUTE設定（propertyによる設定値）
 引数    ：	bool mute		(i)MuteON/解除
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorAdjustMiscReceiver::SetCurrentMuteFrontDisp(const bool mute)
{
	current_mute_front_disp_ = mute;
}

/*****************************************************************************
 処理概要：	１バイトデータの取得
 引数    ：	const uint8_t in_data			(i)入力データ
           	const uint8_t enable_bit		(i)入力データの有効ビット数
           	const uint8_t rightShift_bit		(i)入力データの右シフトビット数
 戻り値  ：	取得した１バイトデータ
*****************************************************************************/
uint8_t CVhalColorAdjustMiscReceiver::GetByteData(const uint8_t in_data, const uint8_t enable_bit, const uint8_t rightShift_bit) const
{
	uint8_t out_data{0U};
	VHAL_LOGV("in_data=0x%02X, enable_bit=%u, rightShift_bit=%u", in_data, enable_bit, rightShift_bit);

	/* シフト数チェック */
	if (static_cast<uint8_t>(UINT8_WIDTH) < (enable_bit + rightShift_bit))
	{
		VHAL_LOGE("Over shift. enable_bit=%u, rightShift_bit=%u", enable_bit, rightShift_bit);
	}
	else
	{
		/* 右シフト後のマスク適用 */
		out_data = (in_data >> rightShift_bit) & ((1U << enable_bit) - 1U);
		VHAL_LOGV("out_data=0x%02X", out_data);
	}
	return out_data;
}

} /* namespace videohal */
