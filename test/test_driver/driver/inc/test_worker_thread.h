/*******************************************************************************
    機能名称    ：  作業スレッドモジュール
    ファイル名称：  test_worker_thread.h
*******************************************************************************/
#ifndef	TEST_WORKER_THREAD_H
#define	TEST_WORKER_THREAD_H

#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>

/*****************************************************************************
 クラス名称：CTestWorkerThread
 処理概要  ：作業スレッド処理の制御を行う。
*****************************************************************************/

template <typename T>
class CTestWorkerThread {
public:
	/*****************************************************************************
	 処理概要：	コンストラクタ
	 引数    ：	なし
	 戻り値  ：	なし
	*****************************************************************************/
	CTestWorkerThread(void) noexcept
		:p_wk_thread_(nullptr)
		,notify_exist_(false)
		,terminate_(false)
	{
	}

	/*****************************************************************************
	 処理概要：	デストラクタ
	 引数    ：	なし
	 戻り値  ：	なし
	*****************************************************************************/
	virtual ~CTestWorkerThread(void)
	{
		End();
	}

	/*****************************************************************************
	 処理概要：	スレッド生成処理
	 引数    ：	なし
	 戻り値  ：	処理結果
					0以外	エラー
					0		正常終了
	*****************************************************************************/
	int32_t Start(void) noexcept
	{
		int32_t ret{0};
		/* スレッド生成 */
		try {
			p_wk_thread_ = new std::thread(&CTestWorkerThread::MainThread, this);
			if (nullptr == p_wk_thread_)
			{
//				VHAL_LOGE("new std::thread null.");
				ret = -1;
			}
		}
		catch (std::exception &ex)
		{
//			VHAL_LOGE("exception received. [%s]", ex.what());
			p_wk_thread_ = nullptr;
			ret = -1;
		}

		return ret;
	}

	/*****************************************************************************
	 処理概要：	スレッド終了処理
	 引数    ：	なし
	 戻り値  ：	なし
	*****************************************************************************/
	void End(void) noexcept
	{
		if ( nullptr != p_wk_thread_ )
		{
			/* スレッド終了要求 */
			{
				const std::lock_guard<std::mutex> lock_sync{mutex_sync_};
				terminate_ = true;
				notify_exist_ = true;
			}
			cond_.notify_one();

			/* スレッド終了待ち */
			if ( true == p_wk_thread_->joinable() )
			{
				p_wk_thread_->join();
			}
			delete p_wk_thread_;
			p_wk_thread_ = nullptr;
		}
	}

	/*****************************************************************************
	 処理概要：	スレッドへデータ送信
	 引数    ：	const T& 				data				(i)送信データ
	 戻り値  ：	処理結果
					0		正常終了
	*****************************************************************************/
	int32_t Notify(const T& data) noexcept
	{
		{
			const std::lock_guard<std::mutex> lock_sync{mutex_sync_};
			/* データを積む */
			que_.push(data);

			/* スレッドに通知 */
			notify_exist_ = true;
		}
		cond_.notify_one();

		return 0;
	}

  	CTestWorkerThread(const CTestWorkerThread& src) = delete;
	CTestWorkerThread& operator=(const CTestWorkerThread& src) & = delete;
	CTestWorkerThread(CTestWorkerThread&& src) = delete;
	CTestWorkerThread& operator=(CTestWorkerThread&& src) & = delete;

private:
	/*****************************************************************************
	 処理概要：	スレッドメイン処理
	 引数    ：	なし
	 戻り値  ：	なし
	*****************************************************************************/
	void MainThread(void) noexcept
	{
		bool running{true};

		while (running)
		{
			std::queue<T> que_local{};
			{
				std::unique_lock<std::mutex> lock_sync{mutex_sync_};		/* ここでロック */
				cond_.wait(lock_sync, [this](void) noexcept -> auto { return notify_exist_; });
				/* 1. lock_syncをアンロック */
				/* 2. 通知を受けるまでwait */
				/* 3. 通知を受けたらlock_syncをロック */
				/* 4. lock_syncのデストラクタでアンロック */

				notify_exist_ = false;

				/* スレッド終了通知 */
				if ( true == terminate_ )
				{
					running = false;
					break;
				}

				/* 通知されたデータを取り出す。 */
				que_local.swap(que_);
			}

			/* 取り出したデータを実行処理に渡す */
			while (!que_local.empty())
			{
				const T data{que_local.front()};
				que_local.pop();
				const int32_t ret{Execute(data)};
				if ( 0 != ret )
				{
					/* 実行処理からエラーが返ってきた場合はスレッド終了 */
					running = false;
					break;
				}
			}
		}
	}

	/*****************************************************************************
	 処理概要：	スレッド実行処理(virtual)
	 引数    ：	const T& 				data				(i)送信データ
	 戻り値  ：	処理結果
					0		正常終了
	*****************************************************************************/
	virtual int32_t Execute(const T& data) const noexcept
	{
		return 0;
	}

private:
	std::thread*			p_wk_thread_;
	std::mutex 				mutex_sync_;
	std::condition_variable cond_;
	std::queue<T>			que_;
	bool 					notify_exist_;
	bool 					terminate_;
};


#endif	/* #ifndef	TEST_WORKER_THREAD_H */
