/*******************************************************************************
    機能名称    ：  画面キャプチャ イベントモジュール
    ファイル名称：  vhal_event_item_screen_shot_micon.cpp
*******************************************************************************/
#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_event_item_screen_shot_event_micon.h"
#include "vhal_micon_misc_opc.h"
#include "vhal_screen_shot_micon.h"
#include "vhal_debug_system.h"

namespace videohal
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalScreenShotSendItem::CVhalScreenShotSendItem(void) noexcept
{
}

/*****************************************************************************
 処理概要：	スクリーンショット要求送信データ構築
 引数    ：	std::vector<uint8_t> &send_data		(o)送信データ
 戻り値  ：	処理結果
           		VHAL_ERR				異常終了
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalScreenShotSendItem::Build(std::vector<uint8_t> &send_data) const
{
	int32_t ret{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	if (true == send_data.empty())
	{
		send_data.push_back(SUB_TYPE_DISP_SCREEN_SHOT_REQ);	/* サブタイプ(30h：スクリーンショット要求) */
		send_data.push_back(screen_type_);					/* スクリーン種別(00h：CID 01h：RSE 02h：MET 03h：HUD) */
		send_data.push_back(kDatatypeDisplay);				/* 通信データタイプ(36h：Display) */
	}
	else
	{
		ret = VHAL_ERR;
	}

	VHAL_LOGV_OUT("ret=%d", ret);
	return ret;
}

/*****************************************************************************
 処理概要：	送信アイテム種別取得
 引数    ：	なし
 戻り値  ：	送信アイテム種別
*****************************************************************************/
CVhalMiconSendItem::SendItemType CVhalScreenShotSendItem::GetItemType(void) const noexcept
{
	return CVhalMiconSendItem::SendItemType::SEND_ITEM_TYPE_MISC;
}

/*****************************************************************************
 処理概要：	スクリーン種別設定
 引数    ：	const uint8_t type	(i)スクリーン種別
 戻り値  ：	なし
*****************************************************************************/
void CVhalScreenShotSendItem::SetScreenType(const uint8_t type) noexcept
{
	screen_type_ = type;
}

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalScreenShotReceiver::CVhalScreenShotReceiver(void) noexcept
{
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalScreenShotReceiver::~CVhalScreenShotReceiver(void) noexcept 
{
	ClearEventListener();
}

/*****************************************************************************
 処理概要：	スクリーンショット応答受信（VideoHALイベントスレッドからのコール）
 引数    ：	const std::vector<uint8_t>&	data	(i)	コマンド情報ポインタ
 戻り値  ：	なし
*****************************************************************************/
void CVhalScreenShotReceiver::Receive(const std::vector<uint8_t>& data)
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
			switch (data[kTabRecvDataOpc_])
			{
				case SUB_TYPE_DISP_SCREEN_SHOT_RSP:
					/* スクリーンショット応答 */
					ReceiveScreenShotResponse(data);
					break;
				case SUB_TYPE_DISP_MODE_RSP:					/* 画質モード応答は無処理 */
				case SUB_TYPE_DISP_HUD_FUNC_STATUS:				/* HUD機能有無判定結果通知は無処理 */
				case SUB_TYPE_DISP_HUD_DISTORTION_CORRECTION:	/* HUD歪み補正通知は無処理 */
				case SUB_TYPE_DISP_HUD_ROTATION:				/* HUD回転パラメータ通知は無処理 */
					break;
				default:
					VHAL_LOGE("sub_type error. value=0x%02X", data[kTabRecvDataOpc_]);
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
void CVhalScreenShotReceiver::ReceivePreNotify(const std::vector<uint8_t>& data) noexcept
{
	/* 処理なし（Receiveで処理実施） */
}

/*****************************************************************************
 処理概要：	送信アイテム種別取得
 引数    ：	なし
 戻り値  ：	送信アイテム種別
*****************************************************************************/
CVhalMiconReceiveItem::ReceiveItemType CVhalScreenShotReceiver::GetItemType(void) const noexcept
{
	return CVhalMiconReceiveItem::ReceiveItemType::RECEIVE_ITEM_TYPE_MISC;
}

/*****************************************************************************
 処理概要：	イベントリスナー登録
 引数    ：	CVhalScreenShotReceiveEventListenerBase* const p_listener		(i)リスナーポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_SUCCESS			正常終了
****************************************************************************/
int32_t CVhalScreenShotReceiver::RegisterEventListener(CVhalScreenShotReceiveEventListenerBase* const p_listener)
{
	int32_t	result{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	if (nullptr == p_listener)
	{
		VHAL_LOGE("p_listener is null.");
		result = VHAL_ERR_PARAM;
	}
	else
	{
		p_screenshot_result_listener_ = p_listener;
	}

	VHAL_LOGV_OUT("result=%d", result);
	return result;
}

/*****************************************************************************
 処理概要：	イベントリスナー解除
 引数    ：	なし
 戻り値  ：	なし
****************************************************************************/
void CVhalScreenShotReceiver::ClearEventListener(void) noexcept
{
	p_screenshot_result_listener_ = nullptr;
}

/*****************************************************************************
 処理概要：	スクリーンショット応答結果設定
 引数    ：	const std::vector<uint8_t>& data	(i) 受信データ
 戻り値  ：	なし
 フェールセーフNo  ：	F-VHAL-N-347, F-VHAL-N-348
*****************************************************************************/
void CVhalScreenShotReceiver::ReceiveScreenShotResponse(const std::vector<uint8_t>& data) noexcept
{
	VHAL_LOGV_IN();

	/* スクリーンショット取得結果をresultに格納 */
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	uint8_t result{static_cast<uint8_t>(data[kRecvDataScreenShotResult_])};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-347",fail_ret)};
//	if(true == fail)
//	{
//		result = static_cast<uint8_t>(fail_ret);
//	}
//	switch (result)
//#else
	const uint8_t result{static_cast<uint8_t>(data[kRecvDataScreenShotResult_])};
	switch (result)
//#endif
	{
		/* 00h：取得成功 */
		case static_cast<uint8_t>(VhalScreenShotResult::success):
		/* 01h：取得失敗 */
		case static_cast<uint8_t>(VhalScreenShotResult::failed):
			screenshot_result_ = static_cast<VhalScreenShotResult>(result);
			break;
		/* 不正な値 */
		default:
			screenshot_result_ = VhalScreenShotResult::failed;
			VHAL_LOGE("Invalid params. result=%u", result);
			break;
	}

	/* スクリーンショット応答受信 */
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	fail = CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-348",fail_ret);
//	if(true == fail)
//	{
//		if (0 == fail_ret)
//		{
//			p_screenshot_result_listener_->NotifyReceiveScreenShotMiconResponse(screenshot_result_);
//		}
//	}
//	else
//	{
//		p_screenshot_result_listener_->NotifyReceiveScreenShotMiconResponse(screenshot_result_);
//	}
//#else
	if (nullptr != p_screenshot_result_listener_)
	{
		p_screenshot_result_listener_->NotifyReceiveScreenShotMiconResponse(screenshot_result_);
	}
	else
	{
		VHAL_LOGE("p_screenshot_result_listener_ is null.");
	}
//#endif

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	CVhalScreenShotMicon* const p_screenshot	(i)スクリーンショット制御インスタンスポインタ
 戻り値  ：	なし
*****************************************************************************/
CVhalScreenShotTimer::CVhalScreenShotTimer(CVhalScreenShotMicon* const p_control) noexcept
	:p_screenshot_(p_control)
{
}

/*****************************************************************************
 処理概要：	タイマコールバックイベント
 引数    ：	const bool time_cycle_enable	(i)サイクリックタイマ有効状態
           		true	有効（タイマ回数制限なし or タイマ回数制限未到達）
           		false	無効（タイマ回数制限到達）
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalScreenShotTimer::OnTimerImpl(const bool time_cycle_enable) const
{
	VHAL_LOGV_IN();

	int32_t result{VHAL_SUCCESS};

	if (nullptr != p_screenshot_)
	{
		if (true == time_cycle_enable)
		{
			result = VHAL_ERR;
			VHAL_LOGD("time_cycle_enable=%d", time_cycle_enable);
		}
		else
		{
			/* 受信データを渡す（タイムアウト） */
			p_screenshot_->NotifyScreenShotMiconResponse(VhalScreenShotResult::failed);
		}
	}

	VHAL_LOGV_OUT("result=%d", result);
	return result;
}

} /* namespace videohal */

