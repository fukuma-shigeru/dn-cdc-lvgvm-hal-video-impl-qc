/*******************************************************************************
    機能名称    ：  メインコントロールモジュール
    ファイル名称：  vhal_main_control.cpp
*******************************************************************************/
#include "vhal_main_control.h"

#include <algorithm>
#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include "json/json.h"

#include "vhal_debug_system.h"
#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_property_control.h"
#include "vhal_debug_system.h"

namespace videohal
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalMainControl::CVhalMainControl(void)
	:p_route_(nullptr)
	,p_prop_(nullptr)
	,p_server_(nullptr)
	,running_()
	,epoll_fd_()
{
	VHAL_LOGV("CVhalMainControl is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalMainControl::~CVhalMainControl(void)
{
	VHAL_LOGV("CVhalMainControl is deleted. this=%p", this);
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_EPOLL		epoll処理エラー
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-C-008
*****************************************************************************/
int32_t CVhalMainControl::Initialize(void)
{
	int32_t ret;

	/* イベントループ用pollの初期化 */
	epoll_fd_ = epoll_create1(0);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-C-008",fail_ret)};
//	if(true == fail)
//	{
//		epoll_fd_ = fail_ret;
//	}
//#endif
	if (-1 == epoll_fd_)
	{
		VHAL_LOGE("epoll_create1 error.");
		return VHAL_ERR_EPOLL;
	}

	/* メインループの待ち解除用イベントルートを登録。（イベント受信用fdの登録） */
	p_route_ = std::make_unique<CVhalEventRoute>();
	ret = p_route_->Initialize();
	if (0 > ret)
	{
		VHAL_LOGE("CVhalEventRoute::Initialize error. ret=%d", ret);
		return ret;
	}

	ret = RegisterEventSource(p_route_.get());
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("RegisterEventSource ret=%d", ret);
	}

	/* プロパティ制御初期化 */
	p_prop_ = std::make_unique<CVhalPropertyControl>();
	ret = p_prop_->Initialize(this);
	if (0 > ret)
	{
		VHAL_LOGE("CVhalPropertyControl::Initialize error. ret=%d", ret);
		return ret;
	}

	/* VhalServer初期化 */
	/* VideoHAL初期化の最後に行い、HalCommonによる待ち受けを開始する。 */
	/* 先に待ち受けを開始すると、VideoHAL初期化完了前にプロパティ通知が来るため最後に行う。 */
	p_server_ = std::make_unique<CVhalServer>();
	ret = p_server_->Initialize(this);
	if (0 > ret)
	{
		VHAL_LOGE("CVhalServer::Initialize error. ret=%d", ret);
		return ret;
	}

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	終了処理（内部リソースの解放）
 引数    ：	なし
 戻り値  ：	なし
 フェールセーフNo  ：	F-VHAL-C-013
*****************************************************************************/
void CVhalMainControl::Finalize(void)
{
	if (nullptr != p_server_)
	{
		p_server_ = nullptr;
	}

	if (nullptr != p_prop_)
	{
		p_prop_ = nullptr;
	}

	if (nullptr != p_route_)
	{
		p_route_ = nullptr;
	}

	if (0 != epoll_fd_)
	{
		errno = 0;
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret{0};
//		int32_t ret{close(epoll_fd_)};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-C-013",fail_ret)};
//		if(true == fail)
//		{
//			ret = fail_ret;
//		}
//#else
		const int32_t ret{close(epoll_fd_)};
//#endif
		if(0 > ret)
		{
			VHAL_LOGE("close error errno=%d", errno);
		}
		epoll_fd_ = 0;
	}
}

/*****************************************************************************
 処理概要：	イベントソースの登録
 引数    ：	CVhalEventSource	*p_source		(i)イベントソースオブジェクト
 戻り値  ：	処理結果
           		VHAL_ERR_EPOLL		epoll処理エラー
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-C-009
*****************************************************************************/
int32_t CVhalMainControl::RegisterEventSource(CVhalEventSource * const p_source)
{
	/* epoll待ちfdに追加 */
	struct epoll_event event;
	event.events = static_cast<uint32_t>(EPOLLIN);
	event.data.ptr = static_cast<void *>(p_source);

	VHAL_LOGV_IN();

	const int32_t fd{p_source->GetSourceFd()};
	VHAL_LOGD("RegisterEventSource. fd=%d", fd);

//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t ret{epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event)};
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-C-009",fail_ret)};
//	if(true == fail)
//	{
//		ret = fail_ret;
//	}
//#else
	const int32_t ret{epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event)};
//#endif
	if (-1 == ret)
	{
		VHAL_LOGE("epoll_ctl(EPOLL_CTL_ADD) error.");
		return VHAL_ERR_EPOLL;
	}

	event_sources_.push_back(p_source);

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	イベントソースの解除
 引数    ：	CVhalEventSource	*p_source		(i)イベントソースオブジェクト
 戻り値  ：	なし
 フェールセーフNo  ：	F-VHAL-C-012
*****************************************************************************/
void CVhalMainControl::ClearEventSource(CVhalEventSource * const p_source)
{
	VHAL_LOGV_IN();

	const auto itr_source = std::find(event_sources_.cbegin(), event_sources_.cend(), p_source);
	if (itr_source == event_sources_.cend())
	{
		VHAL_LOGW("event source is not registered.");
	}
	else
	{
		(void)event_sources_.erase(itr_source);

		/* epoll待ちfdから削除 */
		const int32_t fd{p_source->GetSourceFd()};

//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t ret{epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, NULL)};
//		int32_t fail_ret{0};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-C-012",fail_ret)};
//		if(true == fail)
//		{
//			ret = fail_ret;
//		}
//#else
		const int32_t ret{epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, NULL)};
//#endif
		if (-1 == ret)
		{
			VHAL_LOGW("epoll_ctl(EPOLL_CTL_DEL) error.");
		}
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	イベントソースの解除(仮想関数未使用)
 引数    ：	CVhalEventSource	*p_source		(i)イベントソースオブジェクト
            int32_t 			fd				(i)イベントfd	
 戻り値  ：	なし
*****************************************************************************/
void CVhalMainControl::ClearEventSource(CVhalEventSource * const p_source, const int32_t fd)
{
	VHAL_LOGV_IN();

	const auto itr_source = std::find(event_sources_.cbegin(), event_sources_.cend(), p_source);
	if (itr_source == event_sources_.cend())
	{
		VHAL_LOGW("event source is not registered.");
	}
	else
	{
		(void)event_sources_.erase(itr_source);

		/* epoll待ちfdから削除 */
		const int32_t ret{epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr)};
		if (-1 == ret)
		{
			VHAL_LOGW("epoll_ctl(EPOLL_CTL_DEL) error.");
		}
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	メインループ処理開始（ループ中はreturnしない）
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_EPOLL		epoll処理エラー
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-N-010
                    	F-VHAL-N-011
*****************************************************************************/
int32_t CVhalMainControl::RunEventLoop(void)
{
	int32_t ret{VHAL_SUCCESS};

	running_ = true;
	VHAL_LOGI("start event loop in MainControl");

	while (running_)
	{
		struct epoll_event events[VHAL_EPOLL_SIZE_MAX];
		errno = 0;
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t count{epoll_wait(epoll_fd_, &events[0], VHAL_EPOLL_SIZE_MAX, -1)};
//		int32_t fail_ret{0};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-010",fail_ret)};
//		if(true == fail)
//		{
//			count = -1;
//			errno = fail_ret;
//		}
//#else
		const int32_t count{epoll_wait(epoll_fd_, &events[0], VHAL_EPOLL_SIZE_MAX, -1)};
//#endif
		if (-1 == count)
		{
			VHAL_LOGE("epoll_wait error(%d).", errno);
		}
		else
		{
			for (int32_t i{0}; i < count; ++i)
			{
				CVhalEventSource * const p_source{static_cast<CVhalEventSource *>(events[i].data.ptr)};
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//				int32_t fail_ret{0};
//				bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-011",fail_ret)};
//				if(true == fail)
//				{
//					events[i].events = static_cast<uint32_t>(fail_ret);
//				}
//#endif
				uint32_t source_event{0U};

				if (0U != (events[i].events & static_cast<uint32_t>(EPOLLIN)))
				{
					source_event |= kSourceEventReadable;
				}
				if (0U != (events[i].events & static_cast<uint32_t>(EPOLLOUT)))
				{
					source_event |= kSourceEventWritable;
				}
				if (0U != (events[i].events & static_cast<uint32_t>(EPOLLHUP)))
				{
					VHAL_LOGD("received EPOLLHUP");
					source_event |= kSourceEventHungup;
				}
				if (0U != (events[i].events & static_cast<uint32_t>(EPOLLERR)))
				{
					VHAL_LOGD("received EPOLLERR");
					source_event |= kSourceEventError;
				}

				ret = p_source->ExecEvent(source_event);
				if (VHAL_ERR_CLIENT_DISCONNECTED == ret)
				{
					/* クライアント切断処理 */
					p_server_->ClientDisconnected(p_source);
				}
				else if (0 > ret)
				{
					/* pfm_shutdown契機によりループを抜ける */
					if ( kSourceEventHungup == (source_event & kSourceEventHungup) )
					{
						running_ = false;

						if ( kSourceEventError == (source_event & kSourceEventError) )
						{
							/* エラーイベントも発生 */
							VHAL_LOGW("ExecEvent error in MainControl. ret=%d", ret);
						}
						else
						{
							/* エラーイベントが発生していない場合は戻り値を正常とする */
							ret = VHAL_SUCCESS;
						}
						break;
					}

					/* イベント処理でエラー発生 */
					VHAL_LOGW("ExecEvent error in MainControl. ret=%d", ret);
				}
				else
				{
					/* 処理なし */
				}
			}
		}
	}

	VHAL_LOGI("exited event loop in MainControl. exit code=%d", ret);
	return ret;
}

/*****************************************************************************
 処理概要：	メインループ強制終了
           	メインループ外から止める。メインループ内ならrunning_フラグ操作のみでOK。
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalMainControl::ExitEventLoop(void) const
{
	const int32_t fd{p_route_->GetSourceFd()};

	VHAL_LOGI("requested to exit event loop.");

	/* イベントループの待ち解除（EPOLLHUPを発生させる） */
	(void)shutdown(fd, SHUT_RDWR);
}

/*****************************************************************************
 処理概要：	更新されたプロパティ名の通知
 引数    ：	std::vector<std::string>	&property_names		(i)更新されたプロパティ名リスト
 戻り値  ：	なし
*****************************************************************************/
void CVhalMainControl::UpdatedProperties(std::vector<std::string> &property_names)
{
	if (nullptr != p_server_)
	{
		(void)p_server_->Publish(property_names);
	}
	else
	{
		/* クライアントと、まだ疎通していないため、通知不要 */
	}
}

/*****************************************************************************
 処理概要：	プロパティ制御インスタンスの取得
 引数    ：	なし
 戻り値  ：	プロパティ制御インスタンスポインタ
*****************************************************************************/
CVhalPropertyControl *GetPropertyControl(const CVhalMainControl * const p_main) noexcept
{
	return p_main->p_prop_.get();
}

/*****************************************************************************
 処理概要：	内部イベント送信処理
 引数    ：	CVhalEventItemBase	*p_item		(i)イベントアイテムインスタンスポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_**		処理エラー
           		VHAL_SUCCESS	正常終了
*****************************************************************************/
int32_t CVhalMainControl::WriteEvent(CVhalEventItemBase * const p_item) const noexcept
{
	const int32_t ret{p_route_->WriteEvent(p_item)};
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGEW("WriteEvent error. ret=%d", ret);
	}
	return ret;
}

/*****************************************************************************
 処理概要：	内部イベント破棄処理
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalMainControl::DropEvents(void)
{
	/* epoll_wait件数が無くなるまで繰り返し */
	while (true)
	{
		struct epoll_event events[VHAL_EPOLL_SIZE_MAX]{};
		/* 待ち時間なしでイベント取得 */
		const int32_t count{static_cast<int32_t>(epoll_wait(epoll_fd_, &events[0], VHAL_EPOLL_SIZE_MAX, 0))};
		/* イベントが無くなればループから抜ける */
		if (0 >= count)
		{
			if ((-1 == count) && (EINTR == errno))
			{
				continue;	/* 割り込みによる中断はやり直し(MISRA C++2023/AUTOSAR C++14) */
			}
			break;
		}

		for (int32_t i{0}; i < count; ++i)
		{
			CVhalEventSource* const p_source{static_cast<CVhalEventSource*>(events[i].data.ptr)};
			if (nullptr != p_source)
			{
				uint32_t source_event{0U};
				auto set_bit_if = [&source_event](uint32_t mask, uint32_t value) -> void {
				    if (0U != mask) { source_event |= value; }
				};
				set_bit_if(events[i].events & static_cast<uint32_t>(EPOLLIN), kSourceEventReadable);
				set_bit_if(events[i].events & static_cast<uint32_t>(EPOLLOUT), kSourceEventWritable);
				set_bit_if(events[i].events & static_cast<uint32_t>(EPOLLHUP), kSourceEventHungup);
				set_bit_if(events[i].events & static_cast<uint32_t>(EPOLLERR), kSourceEventError);

				const int32_t ret{p_source->ExecEvent(source_event)};
				if (VHAL_ERR_CLIENT_DISCONNECTED == ret)
				{
					/* クライアント切断処理 */
					p_server_->ClientDisconnected(p_source);
				}
			}
		}
	}
}

} /* namespace videohal */

