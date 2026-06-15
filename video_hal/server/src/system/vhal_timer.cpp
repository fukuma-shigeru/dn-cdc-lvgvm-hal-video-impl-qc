/*******************************************************************************
    機能名称    ：  タイマーモジュール
    ファイル名称：  vhal_timer.cpp
*******************************************************************************/
#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_main_control.h"
#include "vhal_event_route.h"
#include "vhal_debug_system.h"

#include "vhal_timer.h"

extern "C"
{
#include "com_stddef.h"
#include "ccm_public.h"
#include "StdGType.h"
}


namespace videohal
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalTimerSendEventItem::CVhalTimerSendEventItem(void) noexcept
	:CVhalEventItemBase()
{
}

/*****************************************************************************
 処理概要：	イベント処理の実施
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
 注意    ： Execは、VideoHALメインイベントシステムからコールされる
*****************************************************************************/
int32_t CVhalTimerSendEventItem::Exec(void) const
{
	if ( nullptr != p_timer_ )
	{
		p_timer_->OnTimer();
	}
	return VHAL_SUCCESS;
}
/*****************************************************************************
 処理概要：	データ設定
 引数    ：	CVhalTimer* const p_tm	(i)タイマ処理モジュールインスタンスポインタ
 戻り値  ：	なし
*****************************************************************************/
void CVhalTimerSendEventItem::SetData(CVhalTimer* const p_tm) noexcept
{
	p_timer_ = p_tm;
}


/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalTimer::CVhalTimer(void) noexcept
	:p_event_route_(nullptr)
	,p_event_item_(nullptr)
	,event_timer_()
	,time_cycle_max_(0U)
	,time_cycle_count_(0U)
{
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalTimer::~CVhalTimer(void)
{
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化
 引数    ：	CVhalEventRoute* const p_route	(i)内部イベント送信ルートインスタンスポインタ
           	const uint32_t interval			(i)タイマ周期
           	const uint32_t cycle_max		(i)繰り返し回数
 戻り値  ：	処理結果
           		VHAL_ERR_****			エラー
           		VHAL_SUCCESS			正常終了
 フェールセーフNo  ：	F-VHAL-N-163
*****************************************************************************/
int32_t CVhalTimer::Initialize(CVhalEventRoute* const p_route, const uint32_t interval, const uint32_t cycle_max)
{
	int32_t result{VHAL_SUCCESS};

	if ( nullptr != p_route )
	{
		/* タイマ処理開始 */
		spf_timer_attr	attr{};
		attr.name		= nullptr;						/* 識別名指定なし       */
		if (0U == cycle_max)
		{
			attr.wake_cnt	= 0U;						/* サイクリックタイマ   */
		}
		else
		{
			attr.wake_cnt	= 1U;						/* ワンショットタイマ   */
		}
		attr.cycle		= interval;						/* タイマ周期           */
		attr.callback	= &VhalTimerCallbackTimer;		/* コールバック関数     */
		attr.arg		= this;							/* コールバック引数     */

		/* 定期送信用タイマ作成 */
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret{0};
//		int32_t	ret{spf_timer_create(&event_timer_, &attr)};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-163",fail_ret)};
//		if(true == fail)
//		{
//			ret = fail_ret;
//		}
//#else
		const int32_t	ret{spf_timer_create(&event_timer_, &attr)};
//#endif
		if (0 != ret)
		{
			VHAL_LOGE("spf_timer_create() error ret=%d", ret);
			result = VHAL_ERR_TIMER;
		}
		else
		{
			p_event_route_    = p_route;
			time_cycle_count_ = 0U;
			time_cycle_max_   = cycle_max;
		}
	}
	else
	{
		VHAL_LOGE("p_route is null");
		result = VHAL_ERR_PARAM;
	}

	return result;
}

/*****************************************************************************
 処理概要：	終了処理
 引数    ：	なし
 戻り値  ：	なし
 フェールセーフNo  ：	F-VHAL-N-167
*****************************************************************************/
void CVhalTimer::Finalize(void)
{
	/* spf_timer多重削除防止 */
	if ( nullptr != p_event_route_ )
	{
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret{0};
//		int32_t ret{spf_timer_delete(&event_timer_)};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-167",fail_ret)};
//		if(true == fail)
//		{
//			ret = fail_ret;
//		}
//#else
		const int32_t	ret{spf_timer_delete(&event_timer_)};
//#endif
		if (0 != ret)
		{
			VHAL_LOGE("spf_timer_delete() error ret=%d", ret);
		}
		p_event_route_    = nullptr;
		time_cycle_count_ = 0U;
		time_cycle_max_   = 0U;
	}
}

/*****************************************************************************
 処理概要：	定期送信用タイマ開始
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_****			エラー
           		VHAL_SUCCESS			正常終了
 フェールセーフNo  ：	F-VHAL-N-164
*****************************************************************************/
int32_t CVhalTimer::StartTimer(void)
{
	VHAL_LOGV_IN();
	int32_t result{VHAL_SUCCESS};

	/* タイマスタート */
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	int32_t ret{spf_timer_start(&event_timer_)};
//	bool fail = CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-164",fail_ret);
//	if(true == fail)
//	{
//		ret = fail_ret;
//	}
//#else
	const int32_t	ret{spf_timer_start(&event_timer_)};
//#endif
	if (0 != ret)
	{
		VHAL_LOGE("spf_timer_start() error ret=%d", ret);
		result = VHAL_ERR_TIMER;
	}
	else
	{
		if ((0U < time_cycle_max_) && (time_cycle_count_ < time_cycle_max_))
		{
			++time_cycle_count_;
		}
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	定期送信用タイマ再開
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_****			エラー
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalTimer::RestartTimer(void)
{
	VHAL_LOGV_IN();
	int32_t	result{VHAL_SUCCESS};

	/* タイマ停止 */
	EndTimer();

	time_cycle_count_ = 0U;

	/* タイマスタート */
	result = StartTimer();
	if (VHAL_SUCCESS != result)
	{
		VHAL_LOGE("StartTimer() error. ret=%d", result);
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	定期送信用タイマ終了
 引数    ：	なし
 戻り値  ：	なし
 フェールセーフNo  ：	F-VHAL-N-168
*****************************************************************************/
void CVhalTimer::EndTimer(void)
{
	VHAL_LOGV_IN();

	/* タイマ停止 */
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	int32_t ret{spf_timer_stop(&event_timer_)};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-168",fail_ret)};
//	if(true == fail)
//	{
//		ret = fail_ret;
//	}
//#else
	const int32_t	ret{spf_timer_stop(&event_timer_)};
//#endif
	if (0 != ret)
	{
		VHAL_LOGE("spf_timer_stop() error ret=%d", ret);
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	タイマコールバック
 引数    ：	void*	arg	(i)定期送信用タイマインスタンスポインタ
 戻り値  ：	なし
*****************************************************************************/
extern "C" void CVhalTimer::CallbackTimer(void* const arg)
{
	VHAL_LOGV_IN();

	CVhalTimer* const p_timer{static_cast<CVhalTimer*>(arg)};
	if (nullptr != p_timer)
	{
		/* タイマ停止 */
		p_timer->EndTimer();

		/* イベントアイテム書き込み（イベントシステム経由でイベント処理(Exec)実施） */
		if (nullptr != p_timer->p_event_route_)
		{
			std::unique_ptr<CVhalTimerSendEventItem> p_item{std::make_unique<CVhalTimerSendEventItem>()};
			if (nullptr != p_item)
			{
				p_item->SetName("event_timer_send");
				p_item->SetData(p_timer);
				const int32_t ret{p_timer->p_event_route_->WriteEvent(p_item.get())};
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGEW("p_timer->p_event_route_->WriteEvent ret=%d", ret);
				}
				else
				{
					(void)p_item.release();
				}
			}
		}

	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	タイマー満了
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalTimer::OnTimer(void)
{
	bool	time_cycle_enable{false};

	/* 回数制限なし or 回数制限未到達 */
	if ( (0U == time_cycle_max_) ||
		 ((0U < time_cycle_max_) && (time_cycle_count_ < time_cycle_max_)))
	{
		time_cycle_enable = true;
	}

	int32_t ret{OnTimerImpl(time_cycle_enable)};

	/* OnTimerImpl戻り値が異常値でなければタイマー継続 */
	if ( VHAL_SUCCESS == ret )
	{
		/* 回数制限なし or 回数制限未到達 */
		if (true == time_cycle_enable)
		{
			/* 次のサイクルタイマー開始 */
			ret = StartTimer();
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("StartTimer() error. ret=%d", ret);
			}
		}
	}

}

/*****************************************************************************
 処理概要：	タイマコールバック
 引数    ：	void*	arg	(i)定期送信用タイマインスタンスポインタ
 戻り値  ：	なし
*****************************************************************************/
extern "C" void VhalTimerCallbackTimer(void* const arg)
{
	CVhalTimer::CallbackTimer(arg);
}

} /* namespace videohal */
