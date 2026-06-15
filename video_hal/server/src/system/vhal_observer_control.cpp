/*******************************************************************************
    機能名称    ：  状態監視制御モジュール
    ファイル名称：  vhal_observer_control.cpp
*******************************************************************************/
#include "vhal_define.h"
#include "vhal_event_route.h"
#include "vhal_log.h"
#include "vhal_observer_client.h"
#include "vhal_observer_control.h"
#include "vhal_str_mng.h"

namespace videohal
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalObserverControl::CVhalObserverControl(void)
{
	VHAL_LOGV("CVhalObserverControl is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalObserverControl::~CVhalObserverControl(void)
{
	VHAL_LOGV("CVhalObserverControl is deleted. this=%p", this);
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化処理（監視開始）
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_****		異常終了
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalObserverControl::Initialize(void)
{
	const int32_t	ret{StartObserver()};
	return ret;
}

/*****************************************************************************
 処理概要：	終了処理（監視停止）
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalObserverControl::Finalize(void)
{
	StopObserver();
}

/*****************************************************************************
 処理概要：	監視者登録
 引数    ：	CVhalObserverClient*	p_client	(i)ObserverClientポインタ
           	const int64_t			interval	(i)interval時間(ms)
 戻り値  ：	処理結果
           		VHAL_ERR_****			異常終了
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalObserverControl::RegistryObserver(CVhalObserverClient * const p_client, const int64_t interval)
{
	int32_t	ret{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	const std::lock_guard<std::mutex> lock_data{mtx_item_};

	do
	{
		if ((nullptr == p_client) || (0 >= interval))
		{
			VHAL_LOGE("Invalid params. p_client=%p interval=%ld", p_client, interval);
			ret = VHAL_ERR_PARAM;
			break;
		}

		const auto result = std::find_if(observe_list_.begin(), observe_list_.end(),
			[p_client] (const std::unique_ptr<ObserveEntry> &p_entry) noexcept { return p_entry->p_client_ == p_client; });
		if (result == observe_list_.end())
		{
			std::unique_ptr<struct ObserveEntry> p_observe_entry{std::make_unique<ObserveEntry>()};
			if (nullptr == p_observe_entry)
			{
				VHAL_LOGE("Failed to create ObserveEntry.");
				ret = VHAL_ERR;
				break;
			}
			p_observe_entry->interval_ = interval;
			p_observe_entry->elapse_ = 0;
			p_observe_entry->p_client_ = p_client;
			observe_list_.push_back(std::move(p_observe_entry));
		}
		else
		{
			VHAL_LOGW("p_client(%p) is setting.", p_client);
		}
	} while (false);

	VHAL_LOGV_OUT();
	return ret;
}

/*****************************************************************************
 処理概要：	監視者削除
 引数    ：	const CVhalObserverClient*	p_client	(i)ObserverClientポインタ
 戻り値  ：	なし
*****************************************************************************/
void CVhalObserverControl::ClearObserver(const CVhalObserverClient * const p_client)
{
	VHAL_LOGV_IN();

	const std::lock_guard<std::mutex> lock_data{mtx_item_};

	if (nullptr != p_client)
	{
		const auto result = std::find_if(observe_list_.begin(), observe_list_.end(),
			[p_client](const std::unique_ptr<ObserveEntry> &p_observe_entry) noexcept { return p_observe_entry->p_client_ == p_client; });
		if (result == observe_list_.end())
		{
			VHAL_LOGW("p_client(%p) is not found.", p_client);
		}
		else
		{
			VHAL_LOGD("p_observeEntry(%p). ", result->get());
			(void)observe_list_.erase(result);
		}
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	監視動作中判定
 引数    ：	const CVhalObserverClient*	p_client	(i)ObserverClientポインタ
 戻り値  ：	false:非監視 true:監視動作中
*****************************************************************************/
bool CVhalObserverControl::IsRunningObserver(const CVhalObserverClient * const p_client) const
{
	bool is_running{false};

	VHAL_LOGV_IN();

	const std::lock_guard<std::mutex> lock_data{mtx_item_};

	if (nullptr != p_client)
	{
		const auto result = std::find_if(observe_list_.begin(), observe_list_.end(),
			[p_client](const std::unique_ptr<ObserveEntry> &p_observe_entry) noexcept -> bool { return p_observe_entry->p_client_ == p_client; });
		if (result != observe_list_.end())
		{
			is_running = true;
		}
	}

	VHAL_LOGV_OUT();

	return is_running;
}

/*****************************************************************************
 処理概要：	監視開始
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_SOCKET			socket処理エラー
           		VHAL_ERR_THREAD			スレッド処理エラー
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalObserverControl::StartObserver(void)
{
	VHAL_LOGV_IN();

	/* スレッド生成 */
	const int32_t	ret{CVhalCycleThread::Start(kObserveCycle, kObserveCycleMin)};
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("CVhalCycleThread Start error. ret=%d", ret);
	}

	VHAL_LOGV_OUT();
	return ret;
}

/*****************************************************************************
 処理概要：	監視停止
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalObserverControl::StopObserver(void)
{
	VHAL_LOGV_IN();

	/* スレッド破棄 */
	CVhalCycleThread::End();

	/* 監視者リストのクリア */
	ObserveEntryListClear();

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	監視者リストのクリア
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalObserverControl::ObserveEntryListClear(void)
{
	VHAL_LOGV_IN();

	const std::lock_guard<std::mutex> lock_data{mtx_item_};

	observe_list_.clear();

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	実行処理
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalObserverControl::Execute(void) noexcept
{
	/* サスペンド状態の場合は無処理とする */
	if (videohal::CVhalStrManager::GetSuspend())
	{
		return VHAL_SUCCESS;
	}

	const std::lock_guard<std::mutex> lock_data{mtx_item_};

	auto itr = observe_list_.begin();
	while (itr != observe_list_.end())
	{
		struct ObserveEntry * const p_observe_entry{itr->get()};
		bool is_erased{false};
		if (nullptr != p_observe_entry)
		{
			if (nullptr != p_observe_entry->p_client_)
			{
				p_observe_entry->elapse_ += static_cast<int64_t>(kObserveCycle);
				if (p_observe_entry->interval_ <= p_observe_entry->elapse_)
				{
					const int32_t	ret{p_observe_entry->p_client_->Notify()};
					p_observe_entry->elapse_ = 0;

					/* コールバックからマイナス値が返された場合エントリーを削除 */
					if (0 > ret)
					{
						VHAL_LOGD("callback returned value(%d). Delete entry(%p).", ret, p_observe_entry);
						itr = observe_list_.erase(itr);
						is_erased = true;
					}
				}
			}
		}

		if (false == is_erased)
		{
			++itr;
		}
	}

	return VHAL_SUCCESS;
}


} /* namespace videohal */

