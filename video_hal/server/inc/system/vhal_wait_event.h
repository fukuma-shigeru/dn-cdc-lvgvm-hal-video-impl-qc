/*******************************************************************************
    機能名称    ：  イベント待ちモジュール
    ファイル名称：  vhal_wait_event.h
*******************************************************************************/
#ifndef	VHAL_WAIT_EVENT_H
#define	VHAL_WAIT_EVENT_H

#include <mutex>
#include <chrono>
#include <condition_variable>

#include "vhal_define.h"
#include "vhal_log.h"

/*****************************************************************************
 クラス名称：CVhalWaitEvent
 処理概要  ：イベント待ち制御を行う。
*****************************************************************************/
namespace videohal
{

class CVhalWaitEvent {
public:
	/*****************************************************************************
	 処理概要：	コンストラクタ
	 引数    ：	なし
	 戻り値  ：	なし
	*****************************************************************************/
	CVhalWaitEvent(void)
		:notify_exist_(false)
	{
 		VHAL_LOGV("CVhalWaitEvent is created this=%p", this);
	}

	/*****************************************************************************
	 処理概要：	デストラクタ
	 引数    ：	なし
	 戻り値  ：	なし
	*****************************************************************************/
	virtual ~CVhalWaitEvent(void)
	{
 		VHAL_LOGV("CVhalWaitEvent is deleted this=%p", this);
 	}

  	CVhalWaitEvent(const CVhalWaitEvent& src) = delete;
	CVhalWaitEvent& operator=(const CVhalWaitEvent& src) & = delete;
	CVhalWaitEvent(CVhalWaitEvent&& src) = delete;
	CVhalWaitEvent& operator=(CVhalWaitEvent&& src) & = delete;
	
	/*****************************************************************************
	 処理概要：	イベント待ちを解除通知（Wait関数とは別スレッドからコール）
	 引数    ：	なし
	 戻り値  ：	なし
	*****************************************************************************/
	void NotifyOne(void) noexcept
	{
		{
			const std::lock_guard<std::mutex> lock_sync{mutex_sync_};

			/* イベント待ち側に通知 */
			notify_exist_ = true;
		}
		cond_.notify_one();
	}

	/*****************************************************************************
	 処理概要：	イベント待ち
	 引数    ：	const uint32_t timeout		(i)タイムアウト値
	 戻り値  ：	処理結果
					true		正常終了
					false		タイムアウト
	*****************************************************************************/
	bool Wait(const uint32_t timeout) noexcept
	{
		{
			std::unique_lock<std::mutex> lock_sync{mutex_sync_};		/* ここでロック */
			notify_exist_ = false;
			
			(void)cond_.wait_for(lock_sync, std::chrono::milliseconds(timeout), [this] (void) noexcept -> bool{ return notify_exist_; });
			/* 1. lock_syncをアンロック */
			/* 2. 通知を受けるまでwait */
			/* 3. 通知を受けたらlock_syncをロック */
			/* 4. lock_syncのデストラクタでアンロック */

		}
		return notify_exist_;
	}

private:
	std::mutex 				mutex_sync_;
	std::condition_variable cond_;
	bool 					notify_exist_;
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_WAIT_EVENT_H */
