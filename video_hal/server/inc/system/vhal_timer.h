/*******************************************************************************
    機能名称    ：  タイマーモジュール
    ファイル名称：  vhal_timer.h
*******************************************************************************/
#ifndef	VHAL_TIMER_H
#define	VHAL_TIMER_H

#include <map>
#include <mutex>
#include <functional>
#include <memory>

#include "vhal_event_item_base.h"

extern "C"
{
#include "sif_util.h"
#include "spf_timer_public.h"
}

namespace videohal
{
class CVhalTimer;
class CVhalEventRoute;

/*****************************************************************************
 クラス名称：CVhalTimerSendEventItem
 処理概要  ：タイマ－満了時の内部イベントアイテム
*****************************************************************************/
class CVhalTimerSendEventItem : public CVhalEventItemBase {
public:

	CVhalTimerSendEventItem(void) noexcept;
	~CVhalTimerSendEventItem(void) override = default;
  	CVhalTimerSendEventItem(const CVhalTimerSendEventItem& src) = delete;
	CVhalTimerSendEventItem& operator=(const CVhalTimerSendEventItem& src) & = default;
	CVhalTimerSendEventItem(CVhalTimerSendEventItem&& src) = delete;
	CVhalTimerSendEventItem& operator=(CVhalTimerSendEventItem&& src) & = delete;

	/* イベント処理の実施 */
	int32_t Exec(void) const override;

	/* データ設定 */
	void SetData(CVhalTimer* const p_tm) noexcept;

private:

	CVhalTimer* p_timer_{nullptr};

};

/*****************************************************************************
 クラス名称：CVhalTimer
 処理概要  ：タイマ処理
           StartTimer/RestartTimerの違いについて。
           サイクリックタイマ時のカウンター状態に違いがある。
            ①定期送信用タイマの終了(EndTimer)＋開始(StartTimer)：
              カウンター０クリアなし。カウンターは継続する。
            ②定期送信用タイマ再開(RestartTimer)：
              カウンター０クリアあり。
              カウントし直したい場合に使用する。
*****************************************************************************/
class CVhalTimer {
public:

	CVhalTimer(void) noexcept;
	virtual ~CVhalTimer(void);
  	CVhalTimer(const CVhalTimer& src) = delete;
	CVhalTimer(CVhalTimer&& src) = delete;

	/* 初期化 */
	int32_t Initialize(CVhalEventRoute* const p_route, const uint32_t interval, const uint32_t cycle_max=0U);

	/* 終了処理 */
	void Finalize(void);

	/* タイマ開始 */
	int32_t StartTimer(void);

	/* タイマ再開 */
	int32_t RestartTimer(void);

	/* タイマ終了 */
	void EndTimer(void);

	/* タイマー満了 */
	void OnTimer(void);

	/* タイマコールバック */
	static void CallbackTimer(void* const arg);

protected:
	CVhalTimer& operator=(const CVhalTimer& src) & = delete;
	CVhalTimer& operator=(CVhalTimer&& src) & = delete;

private:

	/* タイマー満了実装 */
	virtual int32_t OnTimerImpl(const bool time_cycle_enable) const = 0;


	CVhalEventRoute*			p_event_route_;
	CVhalTimerSendEventItem* 	p_event_item_;

	spf_timer_t					event_timer_;
	uint32_t					time_cycle_max_;
	uint32_t					time_cycle_count_;

};

extern "C" void VhalTimerCallbackTimer(void* const arg);

} /* namespace videohal */

#endif	/* #ifndef	VHAL_TIMER_H */
