/*******************************************************************************
    機能名称    ：  周回スレッドモジュール
    ファイル名称：  vhal_cycle_thread.h
*******************************************************************************/
#ifndef	VHAL_CYCLE_THREAD_H
#define	VHAL_CYCLE_THREAD_H

#include <thread>
#include <mutex>
#include <condition_variable>

#include "vhal_define.h"
#include "vhal_log.h"

/*****************************************************************************
 クラス名称：CVhalCycleThread
 処理概要  ：周回スレッド処理の制御を行う。
*****************************************************************************/
namespace videohal
{

class CVhalCycleThread {
public:
	/*****************************************************************************
	 処理概要：	コンストラクタ
	 引数    ：	なし
	 戻り値  ：	なし
	*****************************************************************************/
	CVhalCycleThread(void)
		:p_cy_thread_(nullptr)
		,notify_exist_(false)
		,terminate_(false)
		,cycle_time_(0)
		,min_time_(0)
	{
	}

	/*****************************************************************************
	 処理概要：	デストラクタ
	 引数    ：	なし
	 戻り値  ：	なし
	*****************************************************************************/
	virtual ~CVhalCycleThread(void)
	{
		End();
	}

  	CVhalCycleThread(const CVhalCycleThread& src) = delete;
	CVhalCycleThread(CVhalCycleThread&& src) = delete;

	/*****************************************************************************
	 処理概要：	スレッド生成処理
	 引数    ：	const uint32_t			cycle_time			(i)周期時間
	           	const uint32_t			min_time			(i)最小周期時間
	 戻り値  ：	処理結果
					VHAL_ERR_****		エラー
					VHAL_SUCCESS		正常終了
	*****************************************************************************/
	int32_t Start(const uint32_t cycle_time, const uint32_t min_time) noexcept
	{
		int32_t	ret{VHAL_SUCCESS};

		cycle_time_ = std::chrono::milliseconds(cycle_time);
		min_time_   = std::chrono::milliseconds(min_time);

		/* スレッド生成 */
		try {
			p_cy_thread_ = std::make_unique<std::thread>(std::thread(&CVhalCycleThread::MainThread, this));
			if (nullptr == p_cy_thread_)
			{
				VHAL_LOGE("new std::thread null.");
				ret = VHAL_ERR_THREAD;
			}
		}
		catch (std::exception &ex)
		{
			VHAL_LOGE("exception received. [%s]", ex.what());
			p_cy_thread_ = nullptr;
			ret = VHAL_ERR_THREAD;
		}

		return ret;
	}
	/*****************************************************************************
	 処理概要：	スレッド終了処理
	 引数    ：	なし
	 戻り値  ：	なし
	*****************************************************************************/
	void End(void)
	{
		if ( nullptr != p_cy_thread_ )
		{
			/* スレッド終了要求 */
			{
				const std::lock_guard<std::mutex> lock_sync{mutex_sync_};
				terminate_ = true;
				notify_exist_ = true;
			}
			cond_.notify_one();

			/* スレッド終了待ち */
			if ( true == p_cy_thread_->joinable() )
			{
				p_cy_thread_->join();
			}
			p_cy_thread_ = nullptr;
		}
	}

private:
	CVhalCycleThread& operator=(const CVhalCycleThread& src) & = default;
	CVhalCycleThread& operator=(CVhalCycleThread&& src) & = delete;

	/*****************************************************************************
	 処理概要：	スレッドメイン処理
	 引数    ：	なし
	 戻り値  ：	なし
	*****************************************************************************/
	void MainThread(void)
	{
		std::chrono::milliseconds	delay_time{cycle_time_};

		bool done{false};
		while (!done)
		{
			std::unique_lock<std::mutex> lock_sync{mutex_sync_};		/* ここでロック */
			const std::cv_status result{cond_.wait_for(lock_sync, delay_time)};
			/* 1. lock_syncをアンロック */
			/* 2. 通知を受けるまでwait */
			/* 3. 通知を受けたらlock_syncをロック */
			/* 4. lock_syncのデストラクタでアンロック */

			const auto start = std::chrono::steady_clock::now();

			/* スレッド終了通知 */
			if ( true == terminate_ )
			{
				done = true;
			}
			else if ( std::cv_status::timeout == result )
			{
				const int32_t	ret{Execute()};
				if ( VHAL_SUCCESS != ret )
				{
					/* 実行処理からエラーが返ってきた場合はスレッド終了 */
					done = true;
				}
			}
			else
			{
			}

			const auto	end = std::chrono::steady_clock::now();
			const auto	dur = end - start;
			const std::chrono::milliseconds	exec_msec{std::chrono::duration_cast<std::chrono::milliseconds>(dur)};
			if ( exec_msec >= cycle_time_ / 2 )
			{
				VHAL_LOGW("Execute consumed too long time (%ld ms)", exec_msec.count());
			}
			delay_time = cycle_time_ - exec_msec;
			if ( delay_time < min_time_ )
			{
				VHAL_LOGW("set next timeout (%ld ms)", min_time_.count());
				delay_time = min_time_;
			}
		}
	}

	/*****************************************************************************
	 処理概要：	スレッド実行処理(virtual)
	 引数    ：	なし
	 戻り値  ：	処理結果
					VHAL_SUCCESS		正常終了
	*****************************************************************************/
	virtual int32_t Execute(void) noexcept = 0;

private:
	std::unique_ptr<std::thread>	p_cy_thread_;
	std::mutex 				mutex_sync_;
	std::condition_variable cond_;
	bool 					notify_exist_;
	bool 					terminate_;
	std::chrono::milliseconds	cycle_time_;
	std::chrono::milliseconds	min_time_;
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_CYCLE_THREAD_H */
