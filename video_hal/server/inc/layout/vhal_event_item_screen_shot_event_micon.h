/*******************************************************************************
    機能名称    ：  画面キャプチャ イベントモジュール
    ファイル名称：  vhal_event_item_screen_shot_event_micon.h
*******************************************************************************/
#ifndef	VHAL_EVENT_ITEM_SCREEN_SHOT_EVENT_MICON_H
#define	VHAL_EVENT_ITEM_SCREEN_SHOT_EVENT_MICON_H

#include "vhal_micon_receive_item.h"
#include "vhal_micon_send_item.h"
#include "vhal_timer.h"

namespace videohal
{
class CVhalScreenShotMicon;

/* スクリーンショット取得結果 */
enum class VhalScreenShotResult : uint8_t {
	success  = 0x00U,	/* 取得成功 */
	failed   = 0x01U,	/* 取得失敗 */
};

/* スクリーンショット要求時のスクリーン種別 */
enum class VhalScreenShotType : uint8_t {
	cid  = 0x00U,	/* cid */
	rse,			/* RSE */
	met,			/* MET */
	hud				/* HUD */
};

/*****************************************************************************
 クラス名称：CVhalScreenShotReceiveEventListenerBase
 処理概要  ：CVhalScreenShotReceiverイベントリスナベース。
*****************************************************************************/
class CVhalScreenShotReceiveEventListenerBase {
public:
	CVhalScreenShotReceiveEventListenerBase(void) noexcept = default;
	virtual ~CVhalScreenShotReceiveEventListenerBase(void) = default;
  	CVhalScreenShotReceiveEventListenerBase(const CVhalScreenShotReceiveEventListenerBase& src) = delete;
	CVhalScreenShotReceiveEventListenerBase(CVhalScreenShotReceiveEventListenerBase&& src) = delete;

	virtual void NotifyReceiveScreenShotMiconResponse(const VhalScreenShotResult result) const noexcept = 0;
	virtual void NotifyScreenShotMiconResult(const int32_t result) const noexcept = 0;

private:
	CVhalScreenShotReceiveEventListenerBase& operator=(const CVhalScreenShotReceiveEventListenerBase& src) & = delete;
	CVhalScreenShotReceiveEventListenerBase& operator=(CVhalScreenShotReceiveEventListenerBase&& src) & = delete;
};

/*****************************************************************************
 クラス名称：CVhalScreenShotSendItem
 処理概要  ：スクリーンショット要求送信クラス
*****************************************************************************/
class CVhalScreenShotSendItem : public CVhalMiconSendItem {
public:
	CVhalScreenShotSendItem(void) noexcept;
	~CVhalScreenShotSendItem(void) override = default;
	CVhalScreenShotSendItem(const CVhalScreenShotSendItem& src) = delete;
	CVhalScreenShotSendItem& operator=(const CVhalScreenShotSendItem& src) & = delete;
	CVhalScreenShotSendItem(CVhalScreenShotSendItem&& src) = delete;
	CVhalScreenShotSendItem& operator=(CVhalScreenShotSendItem&& src) & = delete;

	/* スクリーンショット要求送信データ構築 */
	int32_t Build(std::vector<uint8_t> &send_data) const override;

	/* 送信アイテム種別取得(MISC) */
	SendItemType GetItemType(void) const noexcept override;

	/* 	スクリーン種別設定 */
	void SetScreenType(const uint8_t type) noexcept;

private:
	uint8_t	screen_type_{0U};	/* スクリーン種別 */
};

/*****************************************************************************
 クラス名称：CVhalScreenShotReceiver
 処理概要  ：スクリーンショット応答通信イベントクラス
*****************************************************************************/
class CVhalScreenShotReceiver : public CVhalMiconReceiveItem {
public:
	CVhalScreenShotReceiver(void) noexcept;
	~CVhalScreenShotReceiver(void) noexcept override;
	CVhalScreenShotReceiver(const CVhalScreenShotReceiver& src) = delete;
	CVhalScreenShotReceiver& operator=(const CVhalScreenShotReceiver& src) & = delete;
	CVhalScreenShotReceiver(CVhalScreenShotReceiver&& src) = delete;
	CVhalScreenShotReceiver& operator=(CVhalScreenShotReceiver&& src) & = delete;

	/* イベントリスナー登録 */
	int32_t RegisterEventListener(CVhalScreenShotReceiveEventListenerBase* const p_listener);

	/* イベントリスナー解除 */
	void ClearEventListener(void) noexcept;

	/* スクリーンショット応答受信（VideoHALイベントスレッドからのコール） */
	void Receive(const std::vector<uint8_t>& data) override;

	/* スクリーンショット受信事前通知（通信モジュールスレッドからのコール） */
	void ReceivePreNotify(const std::vector<uint8_t>& data) noexcept override;

	/* 受信アイテム種別取得 */
	ReceiveItemType GetItemType(void) const noexcept override;

private:
	static constexpr uint32_t				kTabRecvDataOpc_{0U};

	/* スクリーンショット応答結果ポジション */
	static constexpr uint32_t				kRecvDataScreenShotResult_{1U};

	/*スクリーンショット応答結果 */
	VhalScreenShotResult					screenshot_result_{VhalScreenShotResult::success};

	CVhalScreenShotReceiveEventListenerBase	*p_screenshot_result_listener_{nullptr};

	/* スクリーンショット応答受信 */
	void ReceiveScreenShotResponse(const std::vector<uint8_t>& data) noexcept;
};

/*****************************************************************************
 クラス名称：CVhalScreenShotTimer
 処理概要  ：タイマ処理
*****************************************************************************/
class CVhalScreenShotTimer : public CVhalTimer {
public:
	CVhalScreenShotTimer(CVhalScreenShotMicon* const p_control) noexcept;
	~CVhalScreenShotTimer(void) override = default;
	CVhalScreenShotTimer(const CVhalScreenShotTimer& src) = delete;
	CVhalScreenShotTimer& operator=(const CVhalScreenShotTimer& src) & = delete;
	CVhalScreenShotTimer(CVhalScreenShotTimer&& src) = delete;
	CVhalScreenShotTimer& operator=(CVhalScreenShotTimer&& src) & = delete;

private:
	/* タイマー満了 */
	int32_t OnTimerImpl(const bool time_cycle_enable) const override;

	CVhalScreenShotMicon* p_screenshot_;
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_EVENT_ITEM_SCREEN_SHOT_EVENT_MICON_H */
