/*******************************************************************************
    機能名称    ：  認証結果イベントモジュール
    ファイル名称：  vhal_event_item_hdcp_auth.h
*******************************************************************************/
#ifndef	VHAL_EVENT_ITEM_HDCP_AUTH_H
#define	VHAL_EVENT_ITEM_HDCP_AUTH_H

#include "vhal_event_item_base.h"
#include "vhal_hdcp_auth_control.h"
#include "vhal_micon_receive_item.h"
#include "vhal_micon_send_item.h"

#include <vector>

namespace videohal
{

class CVhalHdcpAuthRsltListenerBase;

/*****************************************************************************
 クラス名称：CVhalHdcpMiscReceiver
 処理概要  ：HDCP認証通信レシーバークラス
*****************************************************************************/
class CVhalHdcpMiscReceiver : public CVhalMiconReceiveItem {
public:

	CVhalHdcpMiscReceiver(void);
	~CVhalHdcpMiscReceiver(void) override;
	CVhalHdcpMiscReceiver(const CVhalHdcpMiscReceiver& src) = delete;
	CVhalHdcpMiscReceiver& operator=(const CVhalHdcpMiscReceiver& src) & = delete;
	CVhalHdcpMiscReceiver(CVhalHdcpMiscReceiver&& src) = delete;
	CVhalHdcpMiscReceiver& operator=(CVhalHdcpMiscReceiver&& src) & = delete;

	/* データ受信（VideoHALイベントスレッドからのコール） */
	void Receive(const std::vector<uint8_t>& data) override;

	/* データ受信事前通知（通信モジュールスレッドからのコール） */
	void ReceivePreNotify(const std::vector<uint8_t>& data) noexcept override;

	/* 受信アイテム種別取得 */
	ReceiveItemType GetItemType(void) const noexcept override;

	/* イベントリスナー登録 */
	int32_t RegisterEventListener(CVhalHdcpAuthRsltListenerBase* const p_listener);

	/* イベントリスナー解除 */
	void ClearEventListener(void) noexcept;

private:
	static constexpr uint32_t	kHdcpMiscDataSizeMin{2U};	/* data_type+sub_type */
	static constexpr uint32_t	kHdcpMiscDataOpc{0U};

	CVhalHdcpAuthRsltListenerBase	*p_hdcp_auth_rslt_listener_;

	/* HDCP認証情報通知 */
	void MakeHdcpAuthKey(const std::vector<uint8_t>& data) noexcept;

	/* HDCP認証キークリア通知 */
	void ClearHdcpAuthKey(const std::vector<uint8_t>& data) noexcept;
};

#if 0	// (予定)
/*****************************************************************************
 クラス名称：CVhalHdcpAuthRseSendItem
 処理概要  ：RSE_HDCP認証要求送信クラス
*****************************************************************************/
class CVhalHdcpAuthRseSendItem : public CVhalMiconSendItem {
public:

	CVhalHdcpAuthRseSendItem(void);
	~CVhalHdcpAuthRseSendItem(void) override = default;
  	CVhalHdcpAuthRseSendItem(const CVhalHdcpAuthRseSendItem& src) = delete;
	CVhalHdcpAuthRseSendItem& operator=(const CVhalHdcpAuthRseSendItem& src) & = delete;
	CVhalHdcpAuthRseSendItem(CVhalHdcpAuthRseSendItem&& src) = delete;
	CVhalHdcpAuthRseSendItem& operator=(CVhalHdcpAuthRseSendItem&& src) & = delete;

	/* 送信データ構築 */
	int32_t Build(std::vector<uint8_t> &send_data) const override;

	/* 送信アイテム種別取得 */
	SendItemType GetItemType(void) const noexcept override;

private:
	uint8_t					opc_;
};
#endif



} /* namespace videohal */

#endif	/* #ifndef	VHAL_EVENT_ITEM_HDCP_AUTH_H */
