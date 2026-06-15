/*******************************************************************************
    機能名称    ：  状態監視制御モジュール
    ファイル名称：  vhal_observer_control.h
*******************************************************************************/
#ifndef	VHAL_OBSERVER_CONTROL_H
#define	VHAL_OBSERVER_CONTROL_H

#include <algorithm>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include "vhal_cycle_thread.h"

namespace videohal
{
class CVhalObserverClient;

/*****************************************************************************
 クラス名称：CVhalObserverControl
 処理概要  ：状態監視の制御を行う。
*****************************************************************************/
class CVhalObserverControl : public CVhalCycleThread {
public:

	CVhalObserverControl(void);
	~CVhalObserverControl(void) override;
  	CVhalObserverControl(const CVhalObserverControl& src) = delete;
	CVhalObserverControl& operator=(const CVhalObserverControl& src) & = default;
	CVhalObserverControl(CVhalObserverControl&& src) = delete;
	CVhalObserverControl& operator=(CVhalObserverControl&& src) & = delete;

	int32_t Initialize(void);
	void Finalize(void);

	int32_t RegistryObserver(CVhalObserverClient * const p_client, const int64_t interval);
	void ClearObserver(const CVhalObserverClient * const p_client);
	bool IsRunningObserver(const CVhalObserverClient * const p_client) const;

private:

	static constexpr uint32_t	kObserveCycle{30U};		/* スレッドの実行周期(ms) */
	static constexpr uint32_t	kObserveCycleMin{5U};	/* 次回タイムアウトの最小時間(ms) */

	/* 監視者エントリ */
	struct ObserveEntry {
		int64_t					interval_;	/* 監視周期(ms)                     */
		int64_t					elapse_;	/* 経過時間(ms)                     */
		CVhalObserverClient*	p_client_;	/* 状態監視制御クライアントポインタ */
	};

	/* 監視者リスト */
	std::vector<std::unique_ptr<struct ObserveEntry>>	observe_list_{};

	mutable std::mutex	mtx_item_{};

	int32_t StartObserver(void);		/* 監視開始             */
	void StopObserver(void);			/* 監視停止             */
	void ObserveEntryListClear(void);	/* 監視者リストのクリア */
	int32_t Execute(void) noexcept override;		/* 実行処理             */
};

} /* namespace videohal */


#endif	/* #ifndef	VHAL_OBSERVER_CONTROL_TIMER_H */
