/*******************************************************************************
    機能名称    ：  マイコン間通信処理モジュール(MISC)
    ファイル名称：  vhal_micon_misc_receiver.h
*******************************************************************************/
#ifndef	VHAL_MICON_MISC_RECEIVER_H
#define	VHAL_MICON_MISC_RECEIVER_H

#include <cstdint>
#include <vector>

#include "vhal_micon_receive_item.h"

namespace videohal
{

/*****************************************************************************
 クラス名称：CVhalMiscEventListenerBase
 処理概要  ：VhalMiconMiscReceiveイベントリスナベース。
*****************************************************************************/
class CVhalMiscEventListenerBase {
public:
	CVhalMiscEventListenerBase(void) noexcept = default;
	virtual ~CVhalMiscEventListenerBase(void) = default;
  	CVhalMiscEventListenerBase(const CVhalMiscEventListenerBase& src) = delete;
	CVhalMiscEventListenerBase(CVhalMiscEventListenerBase&& src) = delete;

	virtual void NotifyUpdateFilePropertyHdmi(const int32_t state) const noexcept = 0;
	virtual void NotifyChangeHdmiVideoFormat(const int32_t video_format) const noexcept = 0;
	virtual void NotifyChangeHdmiAudioFormat(const int32_t audio_format) const noexcept = 0;
private:
	CVhalMiscEventListenerBase& operator=(const CVhalMiscEventListenerBase& src) & = delete;
	CVhalMiscEventListenerBase& operator=(CVhalMiscEventListenerBase&& src) & = delete;
};

/*****************************************************************************
 クラス名称：CVhalMiconMiscReceiver
 処理概要  ：MISC通信イベントクラス
*****************************************************************************/
class CVhalMiconMiscReceiver : public CVhalMiconReceiveItem {
public:

	CVhalMiconMiscReceiver(void)
		:p_misc_listener_(nullptr)
	{
	}
	~CVhalMiconMiscReceiver(void) override = default;
	CVhalMiconMiscReceiver(const CVhalMiconMiscReceiver& src) = delete;
	CVhalMiconMiscReceiver& operator=(const CVhalMiconMiscReceiver& src) & = delete;
	CVhalMiconMiscReceiver(CVhalMiconMiscReceiver&& src) = delete;
	CVhalMiconMiscReceiver& operator=(CVhalMiconMiscReceiver&& src) & = delete;

	/* データ受信（VideoHALイベントスレッドからのコール） */
	void Receive(const std::vector<uint8_t>& data) override;

	/* データ受信事前通知（通信モジュールスレッドからのコール） */
	void ReceivePreNotify(const std::vector<uint8_t>& data) noexcept override;

	/* 受信アイテム種別取得 */
	ReceiveItemType GetItemType(void) const noexcept override;

	/* イベントリスナー登録 */
	int32_t RegisterEventListener(CVhalMiscEventListenerBase* const p_listener);
	/* イベントリスナー解除 */
	void ClearEventListener(void) noexcept;

	/* HDMI接続検知イベント処理 */
	void ActionHdmiConnInfo(const std::vector<uint8_t>& recv_data) const noexcept;

	/* HDMIビデオフォーマット切替イベント処理 */
	void ActionHdmiVideoFormat(const std::vector<uint8_t>& recv_data) const noexcept;

	/* HDMIオーディオフォーマット切替イベント処理 */
	void ActionHdmiAudioFormat(const std::vector<uint8_t>& recv_data) const noexcept;


private:
	static constexpr uint32_t	kMiscRecvDataSizeMin{2U};
	static constexpr int32_t	kMiscRecvNotFound{-1};

	CVhalMiscEventListenerBase		*p_misc_listener_;
};

} /* namespace videohal */


#endif	/* #ifndef	VHAL_MICON_MISC_RECEIVER_H */
