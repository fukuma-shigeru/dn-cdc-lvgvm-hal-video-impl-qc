/*******************************************************************************
    機能名称    ：  作業スレッドモジュール
    ファイル名称：  vhal_worker_thread.h
*******************************************************************************/
#ifndef	VHAL_WORKER_THREAD_H
#define	VHAL_WORKER_THREAD_H

#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>

#include "vhal_define.h"
#include "vhal_log.h"

/*****************************************************************************
 クラス名称：CVhalWorkerThread
 処理概要  ：作業スレッド処理の制御を行う。
*****************************************************************************/
namespace videohal
{

template <typename T>
class CVhalWorkerThread {
public:
	/*****************************************************************************
	 処理概要：	コンストラクタ
	 引数    ：	なし
	 戻り値  ：	なし
	*****************************************************************************/
	CVhalWorkerThread(void) noexcept
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
	virtual ~CVhalWorkerThread(void)
	{
		End();
	}

	/*****************************************************************************
	 処理概要：	スレッド生成処理
	 引数    ：	なし
	 戻り値  ：	処理結果
					VHAL_ERR_****		エラー
					VHAL_SUCCESS		正常終了
	*****************************************************************************/
	int32_t Start(void) noexcept
	{
		int32_t ret{VHAL_SUCCESS};
		/* スレッド生成 */
		try {
			p_wk_thread_ = std::make_unique<std::thread>(std::thread(&CVhalWorkerThread::MainThread, this));
			if (nullptr == p_wk_thread_)
			{
				VHAL_LOGE("new std::thread null.");
				ret = VHAL_ERR_THREAD;
			}
		}
		catch (std::exception &ex)
		{
			VHAL_LOGE("exception received. [%s]", ex.what());
			p_wk_thread_ = nullptr;
			ret = VHAL_ERR_THREAD;
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
			p_wk_thread_ = nullptr;
		}
	}

	/*****************************************************************************
	 処理概要：	スレッドへデータ送信
	 引数    ：	const T& 				data				(i)送信データ
	 戻り値  ：	処理結果
					VHAL_SUCCESS		正常終了
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

		return VHAL_SUCCESS;
	}

  	CVhalWorkerThread(const CVhalWorkerThread& src) = delete;
	CVhalWorkerThread(CVhalWorkerThread&& src) = delete;

private:
	CVhalWorkerThread& operator=(const CVhalWorkerThread& src) & = delete;
	CVhalWorkerThread& operator=(CVhalWorkerThread&& src) & = delete;

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
				const int32_t ret{Execute(data, terminate_)};
				/* 	VHAL_ERR_SUSPENDは非致命的エラーのため継続 */
				if (( VHAL_SUCCESS != ret ) && ( VHAL_ERR_SUSPEND != ret ))
				{
					/* サスペンドエラー以外のエラーが返ってきた場合はスレッド終了 */
					running = false;
					break;
				}
			}
		}
	}

	/*****************************************************************************
	 処理概要：	スレッド実行処理(virtual)
	 引数    ：	const T& 				data				(i)送信データ
	            const bool 				terminate			(i)処理強制終了(false:継続 true:終了)
	 戻り値  ：	処理結果
					VHAL_SUCCESS		正常終了
	*****************************************************************************/
	virtual int32_t Execute(const T& data, const bool terminate) const noexcept = 0;


private:
	std::unique_ptr<std::thread>	p_wk_thread_;
	std::mutex 				mutex_sync_;
	std::condition_variable cond_;
	std::queue<T>			que_;
	bool 					notify_exist_;
	bool 					terminate_;
protected:
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_WORKER_THREAD_H */
