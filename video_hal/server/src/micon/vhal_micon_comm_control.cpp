/*******************************************************************************
    機能名称    ：  マイコン間通信制御モジュール
    ファイル名称：  vhal_micon_comm_control.cpp
*******************************************************************************/
#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_main_control.h"
#include "vhal_event_route.h"

#include "vhal_micon_comm_control.h"
#include "vhal_micon_send_item.h"
#include "vhal_micon_receive_item.h"
#include "vhal_round_cast.h"
#include "vhal_debug_system.h"

extern "C"
{
#include "com_stddef.h"
#include "ccm_public.h"
#include "spf_timer_public.h"
#include "StdGType.h"
}


namespace videohal
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	CVhalMiconCommControl* const p_micon_ctrl	(i)マイコン間通信制御インスタンスポインタ
           	CVhalMiconSendItem* const p_send			(i)送信データインスタンスポインタ
 戻り値  ：	なし
*****************************************************************************/
CVhalMiconCommSendTimer::CVhalMiconCommSendTimer(CVhalMiconCommControl* const p_micon_ctrl, CVhalMiconSendItem* const p_send) noexcept
	:CVhalTimer()
	,p_micon_control_(p_micon_ctrl)
	,p_send_item_(p_send)
{
}

/*****************************************************************************
 処理概要：	タイマコールバックイベント
 引数    ：	const bool time_cycle_enable	(i)サイクリックタイマ有効状態
           		true	有効（タイマ回数制限なし or タイマ回数制限未到達）
           		false	無効（タイマ回数制限到達）
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconCommSendTimer::OnTimerImpl(const bool time_cycle_enable) const
{
	if (nullptr != p_micon_control_)
	{
		if (true == time_cycle_enable)
		{
			/* アイテム送信 */
			(void)p_micon_control_->Send(*p_send_item_);
		}
		else
		{
			/* 送信処理タイムアウト */
			(void)p_micon_control_->SendTimeout(*p_send_item_);
		}

	}
	return VHAL_SUCCESS;
}


/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalMiconCommControl::CVhalMiconCommControl(void)
	:p_vhal_main_(nullptr)
	,p_layout_mng_(nullptr)
	,p_sysdbctrl_(nullptr)
	,p_event_route_(nullptr)
	,p_ccm_obj_(nullptr)
	,initialized_(false)
	,initialized_most_(false)
	,initialized_resume_(false)
	,connected_rse_(VHAL_CONNECTED_RSE_INVALID)
{
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalMiconCommControl::~CVhalMiconCommControl(void)
{
	if (initialized_ != false)
	{
		Finalize();
	}
}

/*****************************************************************************
 処理概要：	初期化
 引数    ：	CVhalMainControl	*p_vhal_main	(i)メインコントロールインスタンスポインタ
           	CVhalLayoutManager  *p_layout_mng	(i)レイアウト制御インスタンスポインタ
           	CVhalSysdbControl*	p_sysdbctrl		(i)SysDB制御インスタンスポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_****			エラー
           		VHAL_SUCCESS			正常終了
 フェールセーフNo：	F-VHAL-R-056A
                   	F-VHAL-R-056B
*****************************************************************************/
int32_t CVhalMiconCommControl::Initialize(CVhalMainControl* const p_vhal_main, CVhalLayoutManager * const p_layout_mng, CVhalSysdbControl* const p_sysdbctrl)
{
	int32_t result{VHAL_SUCCESS};

	if (initialized_ == true)
	{
		VHAL_LOGW("CVhalMiconCommControl is already initialized");
	}
	else
	{
		initialized_ = true;

		p_vhal_main_ = p_vhal_main;
		p_layout_mng_ = p_layout_mng;
		p_sysdbctrl_ = p_sysdbctrl;

		p_event_route_ = std::make_unique<CVhalEventRoute>();
		if (nullptr == p_event_route_)
		{
			VHAL_LOGE("p_event_route is nullptr.");
			result = VHAL_ERR;
		}
		else
		{
			int32_t ret;

			ret = p_event_route_->Initialize();
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("p_event_route_->Initialize ret=%d", ret);
			}

			ret = p_vhal_main_->RegisterEventSource(p_event_route_.get());
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("p_vhal_main_->RegisterEventSource ret=%d", ret);
			}


			/* CCM初期化 */
			/* [TODO]すぐにOpen出来ないようなら別スレッド化が必要 */
			int32_t ret_ccm{CCM_RET_SUCCESS};
			constexpr uint32_t kCcmOpenApiRetryMax{600U};	/* CCMAPIオープンリトライ回数 */
			bool done{false};
			for (uint32_t i{0U}; (!done) && (i<kCcmOpenApiRetryMax); ++i)
			{
				ret_ccm = CcmApiOpen(&p_ccm_obj_);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//				int32_t fail_ret{0};
//				bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-056",fail_ret)};
//				if(true == fail)
//				{
//					ret_ccm = fail_ret;
//				}
//#endif
				if (CCM_RET_SUCCESS == ret_ccm)
				{
					/* 正常終了 */
					VHAL_LOGI("CcmApiOpen success");
					done = true;
				}
				else if ((CCM_RET_ERR_PARAM == ret_ccm) || (CCM_RET_ERR_SYSTEM == ret_ccm) || (CCM_RET_ERR_COMM == ret_ccm) || (CCM_RET_ERR_OTHER == ret_ccm))
				{
					/* 失敗（リトライ不要） */
					VHAL_LOGE("CcmApiOpen error. ret_ccm=%d", ret_ccm);
					done = true;
				}
				else
				{
					/* リトライあり */
					if( kCcmOpenApiRetryMax <= (i+1U))
					{
						VHAL_LOGE("CcmApiOpen error retry over ret_ccm=%d count=%d", ret_ccm, (i+1U));
						break;
					}
					constexpr uint32_t kCcmOpenApiCycleTime{100U};	/* CCMAPIオープンリトライタイマ値(ms) */
					sif_mdelay(kCcmOpenApiCycleTime);
				}
			}

			if (CCM_RET_SUCCESS != ret_ccm)
			{
				result = VHAL_ERR;
			}

			ret = InitializeResume(false);
			if (VHAL_SUCCESS != ret)
			{
				result = VHAL_ERR;
			}
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	終了処理
 引数    ：	なし
 戻り値  ：	なし
 フェールセーフNo：	F-VHAL-C-057
*****************************************************************************/
void CVhalMiconCommControl::Finalize(void)
{
	if (initialized_ != false)
	{
		FinalizeSuspend();

		{
			const std::lock_guard<std::mutex> lock_data{mtx_item_};

			/* 残っている定期送信イベントがあれば削除 */
			for (auto& elem : send_item_timer_list_)
			{
				CVhalMiconCommSendTimer* const	p_timer{elem.second.get()};
				if (nullptr != p_timer)
				{
					/* p_timer->EndTimer(); EndTimerはFinalizeSuspend()にて実施済 */
					p_timer->Finalize();
				}
			}
			send_item_timer_list_.clear();
		}

		/* CCM終了 */
		if (nullptr != p_ccm_obj_)
		{

//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//			int32_t	ret_ccm{CcmApiClose(p_ccm_obj_)};
//			int32_t fail_ret{0};
//			bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-C-057",fail_ret)};
//			if(true == fail)
//			{
//				ret_ccm = fail_ret;
//			}
//#else
			const int32_t	ret_ccm{CcmApiClose(p_ccm_obj_)};
//#endif
			if (CCM_RET_SUCCESS != ret_ccm)
			{
				VHAL_LOGE("CcmApiClose error ret_ccm=%d", ret_ccm);
			}
			p_ccm_obj_ = nullptr;
		}

		if (nullptr != p_event_route_)
		{
			p_vhal_main_->ClearEventSource(p_event_route_.get());

			p_event_route_->Finalize();
			p_event_route_ = nullptr;
		}

		p_vhal_main_ = nullptr;

		initialized_ = false;
	}
}

/*****************************************************************************
 処理概要：	初期化(レジューム)
 引数    ：	bool				str_resume		(i)	起動種別(true:STRレジューム時 false:通常起動時)
 戻り値  ：	処理結果
           		VHAL_ERR				エラー
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconCommControl::InitializeResume(const bool str_resume)
{
	int32_t result{VHAL_SUCCESS};
	VHAL_LOGV_IN();
	
	if ((nullptr == p_event_route_) || (nullptr == p_layout_mng_))
	{
		VHAL_LOGE("parameter error. route=[%p] layout=[%p]", p_event_route_.get(), p_layout_mng_);
		result = VHAL_ERR;
	}
	else
	{
		/* 既に成功済みの場合は二重で実行しない */
		if (!initialized_resume_)
		{
			int32_t ret{VHAL_SUCCESS};
			/* p_ccm_obj_がnullでもMiscCtrl初期化実施(MiscCtrlにてチェックしてエラーを返す) */
			ret = misc_control_.Initialize(p_event_route_.get(), p_sysdbctrl_, p_ccm_obj_, str_resume);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("MiscControl Initialize error. ret=%d", ret);
				result = VHAL_ERR;
			}
			/* Most初期化実施 */
			ret = most_control_.Initialize(p_event_route_.get(), p_layout_mng_);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("MostControl Initialize error. ret=%d", ret);
				result = VHAL_ERR;
			}
		}
	}

	if (VHAL_SUCCESS == result)
	{
		initialized_resume_ = true;
	}
	else
	{
		FinalizeSuspend();
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	終了処理(サスペンド)
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconCommControl::FinalizeSuspend(void)
{
	VHAL_LOGV_IN();
	const std::lock_guard<std::mutex> lock_data{mtx_item_};

	/* 定期送信イベントがあれば停止(削除はしない) */
	for (auto& elem : send_item_timer_list_)
	{
		CVhalMiconCommSendTimer* const	p_timer{elem.second.get()};
		if (nullptr != p_timer)
		{
			p_timer->EndTimer();
		}
	}

	most_control_.ReInit();
	misc_control_.ReInit(p_ccm_obj_);

	initialized_resume_ = false;
	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	MOST初期化
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_****			エラー
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconCommControl::InitializeMost(void)
{
	int32_t result{VHAL_SUCCESS};

	do
	{	
		if (false == initialized_resume_)
		{
			result = VHAL_ERR;
			break;
		}

		/* API初期処理 */
		const int32_t ret{most_control_.InitializeLs3Ctl()};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("InitializeLs3Ctl() error. ret=%d", ret);
			result = VHAL_ERR;
		}
		else
		{
			/* MOST初期化済み */
			initialized_most_ = true;
		}
	} while (false);

	return result;
}

/*****************************************************************************
 処理概要：	送信処理
 引数    ：	const CVhalMiconSendItem*	send_item	(i)送信データ
 戻り値  ：	処理結果
           		VHAL_ERR_****			エラー
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconCommControl::Send(const CVhalMiconSendItem &send_item)
{
	int32_t result{VHAL_SUCCESS};

	if (false == initialized_resume_)
	{
		result = VHAL_ERR;
	}
	else
	{
		if (nullptr == p_ccm_obj_)
		{
			VHAL_LOGE("CcmApiOpen not successful");
			result = VHAL_ERR;
		}
		else
		{
			/* 送信データ構築 */
			std::vector<uint8_t> send_data{};
			const int32_t ret{send_item.Build(send_data)};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("send_item.Build ret=%d", ret);
			}

			/* 送信データなしチェック */
			const bool no_send{send_data.empty()};
			if (true == no_send)
			{
				result = VHAL_ERR;
			}
			else
			{
				const CVhalMiconSendItem::SendItemType item_type{send_item.GetItemType()};
				if ( CVhalMiconSendItem::SendItemType::SEND_ITEM_TYPE_MISC == item_type )
				{
					/* MISCコマンド送信 */
					result = misc_control_.Send(p_ccm_obj_, send_data);
				}
				else if ( CVhalMiconSendItem::SendItemType::SEND_ITEM_TYPE_MOST == item_type )
				{
					/* 接続されているRSE種別取得(MOST) */
					int32_t connected_rse_type{VHAL_CONNECTED_RSE_INVALID};
					GetConnectedRse(connected_rse_type);
					if ((VHAL_CONNECTED_RSE_FULL == connected_rse_type) || (VHAL_CONNECTED_RSE_DOP == connected_rse_type))
					{
						/* MOSTコマンド送信 */
						result = most_control_.Send(send_data);
					}
				}
				else
				{
					VHAL_LOGE("Unmatch SendItem is %d.", item_type);
					result = VHAL_ERR_PARAM;
				}
			}
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	送信処理タイムアウト
 引数    ：	const CVhalMiconSendItem*	send_item	(i)送信データ
 戻り値  ：	処理結果
           		VHAL_ERR_****			エラー
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconCommControl::SendTimeout(const CVhalMiconSendItem &send_item)
{
	int32_t result{VHAL_SUCCESS};

	if (false == initialized_resume_)
	{
		result = VHAL_ERR;
	}
	else
	{
		const CVhalMiconSendItem::SendItemType item_type{send_item.GetItemType()};
		if ( CVhalMiconSendItem::SendItemType::SEND_ITEM_TYPE_MOST == item_type )
		{
			/* コマンド送信繰り返し終了 */
			most_control_.NotifySendRetryCountEnd();
		}
		else
		{
			VHAL_LOGE("Unmatch SendItem is %d.", item_type);
			result = VHAL_ERR_PARAM;
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	定期送信データ作成（再開/停止用）
 引数    ：	CVhalMiconSendItem*	p_send_item	(i)送信データインスタンスポインタ
           	const uint32_t		interval	(i)タイマ周期
           	const uint32_t		cycle_count	(i)繰り返し回数
 戻り値  ：	処理結果
           		VHAL_ERR_****			エラー
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconCommControl::CreateIntervalTimer(CVhalMiconSendItem* p_send_item, const uint32_t interval, const uint32_t cycle_count)
{
	const std::lock_guard<std::mutex> lock_data{mtx_item_};
	int32_t result{VHAL_SUCCESS};

	VHAL_LOGV_IN("p_send_item=%p, interval=%d, cycle_count=%d", p_send_item, interval, cycle_count);

	if (false == initialized_resume_)
	{
		result = VHAL_ERR;
	}
	else
	{
		/* 既にsend_item_timer_list_に入っていないかチェック */
		const CSendItemTimerList::const_iterator	it{send_item_timer_list_.find(p_send_item)};
		if (it != send_item_timer_list_.end())
		{
			VHAL_LOGE("Running SendTimer(%p).", p_send_item);
			result = VHAL_ERR;
		}
		else
		{
			/* 定期送信設定 */
			std::unique_ptr<CVhalMiconCommSendTimer>	p_timer{std::make_unique<CVhalMiconCommSendTimer>(this, p_send_item)};
			if (nullptr == p_timer)
			{
				VHAL_LOGE("error. p_timer=%p", p_timer.get());
				result = VHAL_ERR_TIMER;
			}
			else
			{
				const int32_t ret{p_timer->Initialize(p_event_route_.get(), interval, cycle_count)};
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("SendTimer Initialize error. ret=%d", ret);
					result = ret;
				}
				else
				{
					(void)send_item_timer_list_.insert(std::make_pair(p_send_item, std::move(p_timer)));
				}
			}
		}
	}

	VHAL_LOGV_OUT("result=%d", result);
	return result;
}

/*****************************************************************************
 処理概要：	定期データ送信開始
 引数    ：	CVhalMiconSendItem*	p_send_item	(i)送信データインスタンスポインタ
           	const uint32_t		interval	(i)タイマ周期
           	const uint32_t		cycle_count	(i)繰り返し回数
 戻り値  ：	処理結果
           		VHAL_ERR_****			エラー
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconCommControl::SendIntervalStart(CVhalMiconSendItem* p_send_item, const uint32_t interval, const uint32_t cycle_count)
{
	const std::lock_guard<std::mutex> lock_data{mtx_item_};
	int32_t result{VHAL_SUCCESS};

	if (false == initialized_resume_)
	{
		result = VHAL_ERR;
	}
	else
	{
		/* 既にsend_item_timer_list_に入っていないかチェック */
		const CSendItemTimerList::const_iterator	it{send_item_timer_list_.find(p_send_item)};
		if (it != send_item_timer_list_.end())
		{
			VHAL_LOGE("Running SendIntervalStart(%p).", p_send_item);
			result = VHAL_ERR;
		}
		else
		{
			/* 送信処理 */
			int32_t	ret{Send(*p_send_item)};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("Send() error. ret=%d", ret);
				result = ret;
			}
			else
			{
				/* 定期送信設定 */
				std::unique_ptr<CVhalMiconCommSendTimer>	p_timer{std::make_unique<CVhalMiconCommSendTimer>(this, p_send_item)};
				if (nullptr == p_timer)
				{
					VHAL_LOGE("error. p_timer=%p", p_timer.get());
					result = VHAL_ERR_TIMER;
				}
				else
				{
					ret = p_timer->Initialize(p_event_route_.get(), interval, cycle_count);
					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("SendTimer Initialize error. ret=%d", ret);
						result = ret;
					}
					else
					{
						ret = p_timer->StartTimer();
						if (VHAL_SUCCESS != ret)
						{
							VHAL_LOGE("SendTimer Start error. ret=%d", ret);
							result = ret;
						}
						else
						{
							(void)send_item_timer_list_.insert(std::make_pair(p_send_item, std::move(p_timer)));
						}
					}
				}
			}
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	定期データ送信再開（再開/停止用）
 引数    ：	const CVhalMiconSendItem*	p_send_item	(i)送信データインスタンスポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_****			エラー
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconCommControl::SendIntervalRestart(const CVhalMiconSendItem* const p_send_item)
{
	const std::lock_guard<std::mutex> lock_data{mtx_item_};

	int32_t	result{VHAL_SUCCESS};

	VHAL_LOGV_IN("p_send_item=%p", p_send_item);

	if (false == initialized_resume_)
	{
		result = VHAL_ERR;
	}
	else
	{
		/* 定期送信再開 */
		const CSendItemTimerList::const_iterator it{send_item_timer_list_.find(p_send_item)};
		if (it != send_item_timer_list_.end())
		{
			/* 送信処理 */
			int32_t	ret{Send(*p_send_item)};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("Send() error. ret=%d", ret);
				result = ret;
			}
			else
			{
				CVhalMiconCommSendTimer* const p_timer{it->second.get()};
				ret = p_timer->RestartTimer();
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("RestartTimer error. ret=%d", ret);
					result = ret;
				}
			}
		}
		else
		{
			VHAL_LOGE("Not Running SendTimer(%p).", p_send_item);
			result = VHAL_ERR;
		}
	}

	VHAL_LOGV_OUT("result=%d", result);
	return result;
}

/*****************************************************************************
 処理概要：	定期データ送信停止（再開/停止用）
 引数    ：	const CVhalMiconSendItem*	p_send_item	(i)送信データインスタンスポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_****			エラー
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconCommControl::SendIntervalStop(const CVhalMiconSendItem* const p_send_item)
{
	const std::lock_guard<std::mutex> lock_data{mtx_item_};
	int32_t result{VHAL_SUCCESS};

	VHAL_LOGV_IN("p_send_item=%p", p_send_item);

	if (false == initialized_resume_)
	{
		result = VHAL_ERR;
	}
	else
	{
		const CSendItemTimerList::const_iterator it{send_item_timer_list_.find(p_send_item)};
		if (it != send_item_timer_list_.end())
		{
			CVhalMiconCommSendTimer* const	p_timer{it->second.get()};
			p_timer->EndTimer();
		}
	}

	VHAL_LOGV_OUT("result=%d", result);
	return result;
}

/*****************************************************************************
 処理概要：	定期データ送信終了
 引数    ：	const CVhalMiconSendItem*	p_send_item	(i)送信データインスタンスポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_****			エラー
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconCommControl::SendIntervalEnd(const CVhalMiconSendItem* const p_send_item)
{
	const std::lock_guard<std::mutex> lock_data{mtx_item_};

	if (false == initialized_resume_)
	{
		return VHAL_ERR;
	}

	const CSendItemTimerList::const_iterator it{send_item_timer_list_.find(p_send_item)};
	if (it != send_item_timer_list_.end())
	{
		CVhalMiconCommSendTimer* const	p_timer{it->second.get()};
		p_timer->EndTimer();
		p_timer->Finalize();
		(void)send_item_timer_list_.erase(it);
	}

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	データ受信CB登録
 引数    ：	CVhalMiconReceiveItem*	p_rcv_item	(i)受信アイテムモジュールポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_****			エラー
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconCommControl::RegistryReceiver(CVhalMiconReceiveItem* const p_rcv_item)
{
	CVhalMiconReceiveItem::ReceiveItemType item_type{};
	int32_t result{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	item_type = p_rcv_item->GetItemType();

	if (CVhalMiconReceiveItem::ReceiveItemType::RECEIVE_ITEM_TYPE_MISC == item_type)
	{
		const int32_t ret{misc_control_.RegistryReceiver(p_rcv_item)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("misc_control_.RegistryReceiver ret=%d", ret);
		}
	}
	else if (CVhalMiconReceiveItem::ReceiveItemType::RECEIVE_ITEM_TYPE_MOST == item_type)
	{
		const int32_t ret{most_control_.RegistryReceiver(p_rcv_item)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("most_control_.RegistryReceiver ret=%d", ret);
		}
	}
	else
	{
		VHAL_LOGE("Unmatch ReceiveItem is %d.", item_type);
		result = VHAL_ERR;
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	データ受信CB削除
 引数    ：	CVhalMiconReceiveItem*	p_rcv_item	(i)受信アイテムモジュールポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_****			エラー
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconCommControl::ClearReceiver(CVhalMiconReceiveItem* const p_rcv_item)
{
	CVhalMiconReceiveItem::ReceiveItemType item_type{};
	int32_t result{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	item_type = p_rcv_item->GetItemType();

	if (CVhalMiconReceiveItem::ReceiveItemType::RECEIVE_ITEM_TYPE_MISC == item_type)
	{
		const int32_t ret{misc_control_.ClearReceiver(p_rcv_item)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("misc_control_.ClearReceiver ret=%d", ret);
		}
	}
	else if (CVhalMiconReceiveItem::ReceiveItemType::RECEIVE_ITEM_TYPE_MOST == item_type)
	{
		const int32_t ret{most_control_.ClearReceiver(p_rcv_item)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("most_control_.ClearReceiver ret=%d", ret);
		}
	}
	else
	{
		VHAL_LOGE("Unmatch ReceiveItem is %d.", item_type);
		result = VHAL_ERR;
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	現在のRSE種別の取得
 引数    ：	int32_t& rse		(o) RSE種別
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconCommControl::GetConnectedRse(int32_t& rse) const noexcept
{
	rse = connected_rse_;
}

/*****************************************************************************
 処理概要：	現在のRSE種別の設定
 引数    ：	int32_t rse			(i) RSE種別
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconCommControl::SetConnectedRse(const int32_t rse) noexcept
{
	connected_rse_ = rse;
}

/*****************************************************************************
 処理概要：	Most初期化済み状態取得
 引数    ：	なし
 戻り値  ：	Most初期化済み状態
           		true					Most初期化済み
           		false					Most未初期化
*****************************************************************************/
bool CVhalMiconCommControl::IsInitializedMost(void) const noexcept
{
	return initialized_most_;
}

/*****************************************************************************
 処理概要：	HDCP認証情報削除指示
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconCommControl::ClearHdcpKey(void) noexcept
{
	constexpr uint8_t	data_type{kDatatypeHdmi};
	constexpr uint8_t	sub_type{SUB_TYPE_HDCP_AUTH_KEY_CLEAR};
	misc_control_.NotifyReceiveEvent(SizeToUI32(sizeof(sub_type)), &sub_type, data_type);
	return VHAL_SUCCESS;
}

} /* namespace videohal */

