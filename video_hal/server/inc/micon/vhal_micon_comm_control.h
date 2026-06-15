/*******************************************************************************
    機能名称    ：  マイコン間通信制御モジュール
    ファイル名称：  vhal_micon_comm_control.h
*******************************************************************************/
#ifndef	VHAL_MICON_COMM_CONTROL_H
#define	VHAL_MICON_COMM_CONTROL_H

#include <map>
#include <mutex>
#include <memory>

#include "vhal_event_item_base.h"
#include "vhal_micon_misc_control.h"
#include "vhal_event_route.h"
#include "vhal_micon_most_control.h"
#include "vhal_timer.h"
#include "vhal_layout_mng.h"

extern "C"
{
#include "sif_util.h"
}

namespace videohal
{
class CVhalMiconCommControl;
class CVhalMiconSendItem;
class CVhalMiconReceiveItem;
class CVhalMainControl;
class CVhalMiconCommMiscControl;

/*****************************************************************************
 クラス名称：CVhalMiconCommSendTimer
 処理概要  ：定期送信用タイマ処理
*****************************************************************************/
class CVhalMiconCommSendTimer : public CVhalTimer {
public:

	CVhalMiconCommSendTimer(CVhalMiconCommControl* const p_micon_ctrl, CVhalMiconSendItem* const p_send) noexcept;
	~CVhalMiconCommSendTimer(void) override = default;
  	CVhalMiconCommSendTimer(const CVhalMiconCommSendTimer& src) = delete;
	CVhalMiconCommSendTimer& operator=(const CVhalMiconCommSendTimer& src) & = default;
	CVhalMiconCommSendTimer(CVhalMiconCommSendTimer&& src) = delete;
	CVhalMiconCommSendTimer& operator=(CVhalMiconCommSendTimer&& src) & = delete;


private:
	/* タイマー満了実装 */
	int32_t OnTimerImpl(const bool time_cycle_enable) const override;

	CVhalMiconCommControl* p_micon_control_;
	CVhalMiconSendItem* p_send_item_;
};


/*****************************************************************************
 クラス名称：CVhalMiconCommControl
 処理概要  ：マイコン間通信制御を行う。
*****************************************************************************/
class CVhalMiconCommControl {
public:

	CVhalMiconCommControl(void);
	~CVhalMiconCommControl(void);
  	CVhalMiconCommControl(const CVhalMiconCommControl& src) = delete;
	CVhalMiconCommControl& operator=(const CVhalMiconCommControl& src) & = default;
	CVhalMiconCommControl(CVhalMiconCommControl&& src) = delete;
	CVhalMiconCommControl& operator=(CVhalMiconCommControl&& src) & = delete;

	int32_t Initialize(CVhalMainControl* const p_vhal_main, CVhalLayoutManager * const p_layout_mng, CVhalSysdbControl* const p_sysdbctrl);
	void Finalize(void);

	int32_t InitializeResume(const bool str_resume);
	void FinalizeSuspend(void);

	int32_t InitializeMost(void);

	/* データ送信 */
	int32_t Send(const CVhalMiconSendItem &send_item);

	/* データ送信タイムアウト */
	int32_t SendTimeout(const CVhalMiconSendItem &send_item);

	/* 定期送信タイマーの停止及び再開が必要な場合は */
	/* タイマー破棄を行わない 再開/停止用関数を使用すること */
	/* 定期送信データ作成（再開/停止用） */
	int32_t CreateIntervalTimer(CVhalMiconSendItem* p_send_item, const uint32_t interval, const uint32_t cycle_count=0U);
	/* 定期データ送信再開（再開/停止用） */
	int32_t SendIntervalRestart(const CVhalMiconSendItem* const p_send_item);
	/* 定期データ送信停止（再開/停止用） */
	int32_t SendIntervalStop(const CVhalMiconSendItem* const p_send_item);

	/* 定期データ送信開始 */
	int32_t SendIntervalStart(CVhalMiconSendItem* p_send_item, const uint32_t interval, const uint32_t cycle_count=0U);

	/* 定期データ送信終了 */
	int32_t SendIntervalEnd(const CVhalMiconSendItem* const p_send_item);

	/* データ受信CB登録 */
	int32_t RegistryReceiver(CVhalMiconReceiveItem* const p_rcv_item);

	/* データ受信CB削除 */
	int32_t ClearReceiver(CVhalMiconReceiveItem* const p_rcv_item);

	/* 現在のRSE種別の取得 */
	void GetConnectedRse(int32_t& rse) const noexcept;
	/* 現在のRSE種別の設定 */
	void SetConnectedRse(const int32_t rse) noexcept;
	/* Most初期化済み判定 */
	bool IsInitializedMost(void) const noexcept;
	/* HDCP認証情報削除指示 */
	int32_t ClearHdcpKey(void) noexcept;

	//Availability関数も必要？

private:
	mutable std::mutex mtx_item_;

	// <マイコン間通信送信アイテム, 定期送信用タイマ>
	using CSendItemTimerList = std::map<const CVhalMiconSendItem*, std::unique_ptr<CVhalMiconCommSendTimer>>;

	CVhalMainControl*		p_vhal_main_;
	CVhalLayoutManager*		p_layout_mng_;
	CVhalSysdbControl*		p_sysdbctrl_;
	std::unique_ptr<CVhalEventRoute>	p_event_route_;

	CVhalMiconCommMiscControl	misc_control_;
	CVhalMiconCommMostControl	most_control_;
	CSendItemTimerList			send_item_timer_list_;

	void*	p_ccm_obj_;
	bool	initialized_;
	bool	initialized_most_;
	bool	initialized_resume_;
	int32_t	connected_rse_;

};


} /* namespace videohal */

#endif	/* #ifndef	VHAL_MICON_COMM_CONTROL_H */
