/*******************************************************************************
    機能名称    ：  マイコン間MOST通信制御モジュール
    ファイル名称：  vhal_micon_most_control.cpp
*******************************************************************************/
#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_main_control.h"
#include "vhal_event_route.h"
#include <functional>

#include "vhal_micon_comm_control.h"
#include "vhal_micon_receive_item.h"
#include "vhal_debug_system.h"

extern "C"
{
#include "sif_util.h"
}

namespace videohal
{

namespace 
{

CVhalMiconCommMostControl *p_micon_most_control{nullptr};
constexpr uint32_t kRetryCntLs3CtlApi{15U};			/* リトライカウント */
constexpr uint32_t kRetryWaitLs3CtlApi{100U};		/* リトライ待ち時間(ms) */
}

/*****************************************************************************
 処理概要：	イベント処理の実施
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
 注意    ： Execは、VideoHALメインイベントシステムからコールされる
*****************************************************************************/
int32_t CVhalMiconMostRecvEventItem::Exec(void) const
{
	/* イベントを受信したときの処理 */
	VHAL_LOGD("CVhalMiconMostRecvEventItem::Exec called.");
	if (nullptr != p_most_control_)
	{
		/* 受信データを渡す */
		p_most_control_->ExecReceiveEventLs3Data(MostRecvEvent::kLs3Data, ls3_data_);
	}

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	データ設定
 引数    ：	Ls3CtlApiDat &recv_data		(i)受信データ
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconMostRecvEventItem::SetData(const Ls3CtlApiDat &recv_data) noexcept
{
	ls3_data_ = recv_data;
}

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	CVhalMiconCommMostControl* const p_most_ctrl	(i)マイコン間MOST通信制御インスタンスポインタ
 戻り値  ：	なし
*****************************************************************************/
CVhalMostSendTimer::CVhalMostSendTimer(CVhalMiconCommMostControl* const p_most_ctrl) noexcept
	:CVhalTimer()
	,p_most_control_(p_most_ctrl)
{
}

/*****************************************************************************
 処理概要：	タイマコールバックイベント
 引数    ：	const bool time_cycle_enable	(i)サイクリックタイマ有効状態
           		true	有効（タイマ回数制限なし or タイマ回数制限未到達）
           		false	無効（タイマ回数制限到達）
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
 フェールセーフNo  ：	F-VHAL-R-117
*****************************************************************************/
int32_t CVhalMostSendTimer::OnTimerImpl(const bool time_cycle_enable) const
{
	int32_t result{VHAL_SUCCESS};
	if (nullptr != p_most_control_)
	{
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		bool ret_most{false};
//		int32_t fail_ret{0};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-117",fail_ret)};
//		if(true == fail)
//		{
//			ret_most = static_cast<bool>(fail_ret);
//		}
//		else
//		{
//			ret_most = time_cycle_enable;
//		}
//		if (true == ret_most)
//#else
		if (true == time_cycle_enable)
//#endif
		{
			result = VHAL_ERR;
			VHAL_LOGD("time_cycle_enable:%d", time_cycle_enable);
		}
		else
		{
			/* 受信データを渡す（タイムアウト） */
			p_most_control_->ExecReceiveEvent(MostRecvEvent::kResponsTimeout);
		}
	}
	return result;
}

/*****************************************************************************
 処理概要：	コンストラクタ (MOST)
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalMiconCommMostControl::CVhalMiconCommMostControl(void) noexcept
	:p_route_(nullptr)
	,p_layout_mng_(nullptr)
	,p_most_send_timer_(nullptr)
	,ls3ctl_obj_{}
	,send_data_{}
	,ls3ctl_initialized_(false)
{
}

/*****************************************************************************
 処理概要：	初期化処理 (MOST)
 引数    ：	CVhalEventRoute*	p_event_route	(i)	内部イベント送信ルートインスタンスポインタ
           	CVhalLayoutManager*	p_layout_mng	(i)	レイアウト制御インスタンスポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_****			エラー
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconCommMostControl::Initialize(CVhalEventRoute* const p_event_route, CVhalLayoutManager * const p_layout_mng)
{
	int32_t result{VHAL_SUCCESS};

	p_route_ = p_event_route;
	p_layout_mng_ = p_layout_mng;
	p_micon_most_control = this;

	/* Result応答待ち(10sec)タイマ初期化 */
	p_most_send_timer_ = std::make_unique<CVhalMostSendTimer>(this);
	if (nullptr != p_most_send_timer_)
	{
		constexpr uint32_t kMostResultTime{10000U};		/* Result応答待ちタイマ値(ms) 10sec */
		constexpr uint32_t kMostResultRetryCount{1U};	/* Result応答待ち繰り返し回数 1回 */
		result = p_most_send_timer_->Initialize(p_event_route, kMostResultTime, kMostResultRetryCount);
		if (VHAL_SUCCESS != result)
		{
			VHAL_LOGE("p_most_send_timer_->Initialize error. result=%d", result);
		}
	}
	else
	{
		VHAL_LOGE("Failed to create CVhalMostSendTimer.");
		result = VHAL_ERR_TIMER;
	}

	return result;
}

/*****************************************************************************
 処理概要：	終了処理 (MOST)
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconCommMostControl::Finalize(void)
{
	/* データ初期化 */
	ReInit();

	/* タイマ終了 */
	if (nullptr != p_most_send_timer_)
	{
		p_most_send_timer_->Finalize();
		p_most_send_timer_ = nullptr;
	}

	/* MOSTコマンド受信リストの削除 */
	if (false == receive_list_.empty())
	{
		receive_list_.clear();
	}
}

/*****************************************************************************
 処理概要：	再初期化処理
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconCommMostControl::ReInit(void) noexcept
{
	/* Ls3Ctl終了処理 */
	FinalizeLs3Ctl();

	/* 送信照合用データを初期化 */
	send_data_ = Ls3CtlApiDat{};

	/* タイマ停止 */
	if (nullptr != p_most_send_timer_)
	{
		p_most_send_timer_->EndTimer();
	}
}

/*****************************************************************************
 処理概要：	API初期処理 (MOST)
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_****			エラー
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconCommMostControl::InitializeLs3Ctl(void)
{
	int32_t result{VHAL_SUCCESS};

	if (false == ls3ctl_initialized_)
	{
		/* LS3CTL API初期化 */
		result = Open();
		if (VHAL_SUCCESS == result)
		{
			/* LS3CTLコールバック登録 */
			result = RegistryCallBack();
			if (VHAL_SUCCESS == result)
			{
				VHAL_LOGI("RegistryCallBack() success");
				ls3ctl_initialized_ = true;
			}
			else
			{
				VHAL_LOGE("RegistryCallBack() result=%d", result);
			}
		}
		else
		{
			VHAL_LOGE("Open() result=%d", result);
		}
	}
	else
	{
		VHAL_LOGW("already initialized");
	}

	return result;
}

/*****************************************************************************
 処理概要：	API終了処理 (MOST)
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconCommMostControl::FinalizeLs3Ctl(void)
{
	VHAL_LOGD("ls3ctl_initialized_:%d", ls3ctl_initialized_);
	if (false != ls3ctl_initialized_)
	{
		/* LS3CTLコールバック削除 */
		ClearCallBack();

		/* LS3CTL API終了 */
		Close();

		ls3ctl_initialized_ = false;
	}
}

/*****************************************************************************
 処理概要：	データ送信 (MOST)
 引数    ：	std::vector<uint8_t> &data	(i)	送信コマンド
 戻り値  ：	処理結果
           		VHAL_ERR_****			エラー
           		VHAL_SUCCESS			正常終了
 フェールセーフNo  ：	F-VHAL-N-115
                    	F-VHAL-R-116
                    	F-VHAL-N-121
                    	F-VHAL-R-122
                    	F-VHAL-N-187
                    	F-VHAL-N-188
*****************************************************************************/
int32_t CVhalMiconCommMostControl::Send(std::vector<uint8_t> &data)
{
	int32_t result{VHAL_SUCCESS};

	do
	{
		if (false != ls3ctl_initialized_)
		{
			/* LS3CTLコマンド送信 */
			if (data.size() != sizeof(send_data_))
			{
				VHAL_LOGE("parameter error. data.size=%ld", data.size());
				result = VHAL_ERR_PARAM;
				break;
			}
			(void)memcpy(static_cast<void*>(&send_data_), static_cast<void*>(data.data()), sizeof(send_data_));
			uint32_t count{0U};

			while (count < kRetryCntLs3CtlApi)
			{
				bool loop_end{false};
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//				int32_t ret_most{Ls3CtlApiSndCmd(&ls3ctl_obj_, &send_data_)};
//				int32_t fail_ret{0};
//				bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-116",fail_ret)};
//				if(true == fail)
//				{
//					ret_most = fail_ret;
//				}
//#else
				const int32_t ret_most{Ls3CtlApiSndCmd(&ls3ctl_obj_, &send_data_)};
//#endif
				if (LS3_CTL_RET_SUCCESS == ret_most)
				{
					VHAL_LOGD("Ls3CtlApiSndCmd() success");

					/* Result応答待ち(10sec)タイマ開始 */
					result = p_most_send_timer_->RestartTimer();
					if (VHAL_SUCCESS != result)
					{
						VHAL_LOGE("p_most_send_timer_->StartTimer() error. result=%d", result);
					}
					loop_end = true;
				}
// BECstep3 では、LS3_CTL_RET_ERR_ACC_OFFが存在しない
//				else if (LS3_CTL_RET_ERR_ACC_OFF == ret_most)
//				{
//					/* SystemNotOK状態でACC OFF検知 */
//					/* 正常終了扱いのためリトライなし */
//					VHAL_LOGI("Ls3CtlApiSndCmd() ret=%d (acc_off)", ret_most);
//					result = VHAL_SUCCESS;
//					loop_end = true;
//				}
				else if (LS3_CTL_RET_ERR_PARAM == ret_most)
				{
					/* パラメータエラー */
					VHAL_LOGE("Ls3CtlApiSndCmd() ret=%d", ret_most);
					result = VHAL_ERR;
					loop_end = true;
				}
				else
				{
					/* その他エラー */
					sif_mdelay(kRetryWaitLs3CtlApi);
					count++;
					if (kRetryCntLs3CtlApi <= count)
					{
						VHAL_LOGE("Ls3CtlApiSndCmd() count=%d/%d ret=%d", count, kRetryCntLs3CtlApi, ret_most);
						result = VHAL_ERR;
						loop_end = true;
					}
				}
		
				if (true == loop_end)
				{
					break;
				}
			}
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//			int32_t fail_ret{0};
//			bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-122",fail_ret)};
//			if(true == fail)
//			{
//				result = fail_ret;
//				send_data_.FktID = 0xC93U;
//			}
//#endif
			if (VHAL_SUCCESS != result)
			{
				if (static_cast<uint16_t>(MostFktID::kVideoMute) == send_data_.FktID)
				{
					(void)p_layout_mng_->SetBlinderEnable(VhalBlinderType::VHAL_BLINDER_TYPE_REAR_DISPLAY, true);
				}
			}
		}
		else
		{
			/* 送信失敗 */
			VHAL_LOGD("Ls3Ctl not initialized");
			result = VHAL_ERR;
		}
	} while (false);

	return result;
}

/*****************************************************************************
 処理概要：	LS3CTLコマンド通知Callback
 引数    ：	INT32 event				(i)	イベントID
           	UINT8 FBlockID			(i)	送信先FblockID
           	UINT8 InstID			(i)	送信先InstID
           	Ls3CtlApiDat* data		(i)	受信コマンドデータポインタ
 戻り値  ：	なし
*****************************************************************************/
extern "C" void MiconCommMostControlLs3CtlCallback(const int32_t event, const uint8_t FBlockID , const uint8_t InstID, const Ls3CtlApiDat* const data)
{
	CVhalMiconCommMostControl::Ls3CtlCallback(event, FBlockID, InstID, *data);
}

/*****************************************************************************
 処理概要：	LS3CTLコマンド通知Callback
 引数    ：	INT32 event				(i)	イベントID
           	UINT8 fBlockID			(i)	送信先FblockID
           	UINT8 instID			(i)	送信先InstID
           	Ls3CtlApiDat* data		(i)	受信コマンドデータポインタ
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconCommMostControl::Ls3CtlCallback(const int32_t event, const uint8_t fBlockID, const uint8_t instID, const Ls3CtlApiDat& data)
{
	VHAL_LOGV("event=%d, FBlockID=0x%x, InstID=0x%x", event, fBlockID, instID);
	/* コマンド受信通知 */
	if (LS3_CTL_EVENT_CMD_RCV == event)
	{
		VHAL_LOGI("FBlockID=0x%x, InstID=0x%x, FktID=0x%x, OPType=0x%x", data.FBlockID, data.InstID, data.FktID, data.OPType);
		/* 内部イベント通知 */
		Ls3CtlApiDat	recv_data{};
		recv_data.DeviceID	= data.DeviceID;
		recv_data.FBlockID	= data.FBlockID;
		recv_data.InstID	= data.InstID;
		recv_data.FktID		= data.FktID;
		recv_data.OPType	= data.OPType;
		recv_data.DataType	= data.DataType;
		recv_data.Length	= data.Length;
		if ((LS3_CTL_TRANSFER_MEMORY == recv_data.DataType) && (LS3_CTL_MAX_CMD_DATA_LENGTH >= recv_data.Length))
		{
			/* メモリ転送 */
			(void)memcpy(static_cast<void*>(recv_data.Data), static_cast<const void*>(data.Data), static_cast<size_t>(recv_data.Length));
			/* 内部イベント通知 */
			p_micon_most_control->NotifyReceiveEvent(recv_data);
		}
		else
		{
			VHAL_LOGW("DataType=%d, Length=%d", recv_data.DataType, recv_data.Length);
		}
	}

	return;
}

/*****************************************************************************
 処理概要：	コマンド送信繰り返し終了
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconCommMostControl::NotifySendRetryCountEnd(void)
{
	/* Result応答待ち(10sec)タイマ停止 */
	p_most_send_timer_->EndTimer();

	ExecReceiveEvent(MostRecvEvent::kRetryCountEnd);
}

/*****************************************************************************
 処理概要：	内部イベントデータ受信CB登録
 引数    ：	CVhalMiconReceiveItem*	p_rcv_item	(i)	受信アイテムモジュールポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconCommMostControl::RegistryReceiver(CVhalMiconReceiveItem* const p_rcv_item)
{
	const std::lock_guard<std::mutex> lock_data{mtx_recv_item_};

	/* リストにp_rcv_itemを登録 */
	receive_list_.push_back(p_rcv_item);

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	内部イベントデータ受信CB削除
 引数    ：	CVhalMiconReceiveItem*	p_rcv_item	(i)	受信アイテムモジュールポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconCommMostControl::ClearReceiver(CVhalMiconReceiveItem* const p_rcv_item)
{
	const std::lock_guard<std::mutex> lock_data{mtx_recv_item_};

	/* リストからp_rcv_itemを削除 */
	receive_list_.remove(p_rcv_item);

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	内部イベント通知
 引数    ：	Ls3CtlApiDat &recv_data		(i)受信データ
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconCommMostControl::NotifyReceiveEvent(const Ls3CtlApiDat &recv_data)
{
	std::unique_ptr<CVhalMiconMostRecvEventItem> p_recv_event{std::make_unique<CVhalMiconMostRecvEventItem>(this)};
	if (nullptr != p_recv_event)
	{
		p_recv_event->SetName(std::string("most_ctrl event"));
		p_recv_event->SetData(recv_data);
		const int32_t ret{p_route_->WriteEvent(p_recv_event.get())};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGEW("p_route_->WriteEvent ret=%d", ret);
		}
		else
		{
			(void)p_recv_event.release();
		}
	}
	else
	{
		VHAL_LOGE("Failed to create CVhalMiconMostRecvEventItem.");
	}
}

/*****************************************************************************
 処理概要：	内部イベント処理
 引数    ：	const uint8_t recv_event		(i)受信イベント
           	const Ls3CtlApiDat &recv_data	(i)受信データ
 戻り値  ：	なし
 フェールセーフNo  ：	F-VHAL-N-118
                     	F-VHAL-N-119
                     	F-VHAL-N-120
*****************************************************************************/
void CVhalMiconCommMostControl::ExecReceiveEventLs3Data(const uint8_t recv_event, const Ls3CtlApiDat &recv_data)
{
	const std::lock_guard<std::mutex> lock_data{mtx_recv_item_};
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-120",fail_ret)};
//	if(true == fail)
//	{
//		send_data_.FBlockID = 0x00U;
//	}
//#endif
	/* 送信時の「FblockID」「InstID」「FktID」と一致することを受信時にチェックする */
	/* コールバックAPI仕様により、FblockID/InstIDを持つコマンドがコールバック通知されるため、 */
	/* 自身が送信したコマンド以外（FblockID/InstIDは同じ、FktIDが異なる）のコールバックも通知されてしまう。 */
	/* そのため、FblockID/InstIDだけでなく「FktID」も受信時にチェックする必要がある。 */
	if ((send_data_.FBlockID == recv_data.FBlockID) && (send_data_.InstID == recv_data.InstID) && (send_data_.FktID == recv_data.FktID))
	{
		switch (recv_data.OPType)
		{
			case static_cast<uint8_t>(MostOPType::kResult):
			case static_cast<uint8_t>(MostOPType::kError):
			case static_cast<uint8_t>(MostOPType::kProcessing):
				{
					/* Result応答待ち(10sec)タイマ停止 */
					if (static_cast<uint8_t>(MostOPType::kProcessing) == recv_data.OPType)
					{
						VHAL_LOGI("DataType=Processing. (FBlockID=0x%x, InstID=0x%x)", recv_data.FBlockID, recv_data.InstID);
						/* Result応答待ち(10sec)タイマ継続中 */
					}
					else
					{
						p_most_send_timer_->EndTimer();
					}

					/* データ格納 */
					constexpr std::size_t event_size{sizeof(uint8_t)};
					constexpr std::size_t data_size{sizeof(recv_data)};
					std::vector<uint8_t> data{};
					data.resize(event_size + data_size);
					data[0] = recv_event;
					(void)memcpy(static_cast<void*>(&data[event_size]), static_cast<const void*>(&recv_data), data_size);

					/* リスト(receive_list_)に登録されているReceive(recv_data)の実行 */
					for (const auto p_rcv_item : receive_list_)
					{
						p_rcv_item->Receive(data);
					}
				}
				break;

			default:
				/* 処理なし */
				break;
		}
	}
}

/*****************************************************************************
 処理概要：	内部イベント処理
 引数    ：	const uint8_t recv_event		(i)受信イベント
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconCommMostControl::ExecReceiveEvent(const uint8_t recv_event)
{
	const std::lock_guard<std::mutex> lock_data{mtx_recv_item_};

	/* データ格納 */
	constexpr std::size_t event_size{sizeof(uint8_t)};
	std::vector<uint8_t> data{};
	data.resize(event_size);
	data[0] = recv_event;

	/* リスト(receive_list_)に登録されているReceive(recv_data)の実行 */
	for (const auto p_rcv_item : receive_list_)
	{
		p_rcv_item->Receive(data);
	}
}

/*****************************************************************************
 処理概要：	LS3CTL API初期化
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_****			エラー
           		VHAL_SUCCESS			正常終了
 フェールセーフNo  ：	F-VHAL-N-106
                    	F-VHAL-R-107
                     	F-VHAL-R-108
*****************************************************************************/
int32_t CVhalMiconCommMostControl::Open(void)
{
	int32_t result{VHAL_SUCCESS};
	int32_t ret_most{LS3_CTL_RET_SUCCESS};
	uint32_t count{0U};

	constexpr uint32_t kRetryMaxOpen{15U};		/* リトライ回数 */
	while (count < kRetryMaxOpen)
	{
		bool loop_end{false};
		(void)++count;
		/* LS3CTL API初期化 */
		ret_most = Ls3CtlApiOpen(&ls3ctl_obj_);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret{0};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-107",fail_ret)};
//		if(true == fail)
//		{
//			ret_most = fail_ret;
//		}
//#endif
		if (LS3_CTL_RET_SUCCESS == ret_most)
		{
			/* 正常終了 */
			result = VHAL_SUCCESS;
			loop_end = true;
		}
		else if ((LS3_CTL_RET_ERR_NOSVC == ret_most) || (LS3_CTL_RET_ERR_ERR == ret_most))
		{
			/* サービス未オープンエラー、その他エラー */
			if ( kRetryMaxOpen <= count)
			{
				result = VHAL_ERR;
				loop_end = true;
			}
			else
			{
				constexpr uint32_t kRetryWaitOpen{100U};		/* リトライ待ち時間(ms) */
				sif_mdelay(kRetryWaitOpen);
			}
		}
		else
		{
			/* 異常終了 */
			result = VHAL_ERR;
			loop_end = true;
		}

		if (true == loop_end)
		{
			break;
		}
	}

	if (VHAL_SUCCESS == result)
	{
		VHAL_LOGI("Ls3CtlApiOpen() count=%d/%d ret_most=%d", count, kRetryMaxOpen, ret_most);
	}
	else
	{
		VHAL_LOGE("Ls3CtlApiOpen() count=%d/%d ret_most=%d", count, kRetryMaxOpen, ret_most);
	}

	return result;
}

/*****************************************************************************
 処理概要：	LS3CTL API終了
 引数    ：	なし
 戻り値  ：	なし
 フェールセーフNo  ：	F-VHAL-N-109
                    	F-VHAL-C-110
*****************************************************************************/
void CVhalMiconCommMostControl::Close(void)
{
	/* LS3CTL API終了 */
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t ret_most{Ls3CtlApiClose(&ls3ctl_obj_)};
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-109",fail_ret)};
//	if(true == fail)
//	{
//		ret_most = fail_ret;
//	}
//#else
	const int32_t ret_most{Ls3CtlApiClose(&ls3ctl_obj_)};
//#endif
	if (LS3_CTL_RET_SUCCESS != ret_most)
	{
		/* 異常終了 */
		VHAL_LOGE("Ls3CtlApiClose() ret_most=%d", ret_most);
	}

	return;
}

/*****************************************************************************
 処理概要：	LS3CTLコールバック登録
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_****			エラー
           		VHAL_SUCCESS			正常終了
 フェールセーフNo  ：	F-VHAL-N-111
                    	F-VHAL-R-112
*****************************************************************************/
int32_t CVhalMiconCommMostControl::RegistryCallBack(void)
{
	int32_t result{VHAL_SUCCESS};

	/* SourceActivity結果通知用 */
	/* 後席系映像パス情報 */
	for (const auto& tbl : most_rear_path_table_)
	{
		bool done{false};
		for(uint32_t i{0U}; (!done) && (i<kRetryCntLs3CtlApi); ++i)
		{
			/* LS3CTLコマンド受信通知 */
			const int32_t ret_most{Ls3CtlApiCmdAddCallback(&ls3ctl_obj_, static_cast<uint8_t>(tbl.second.fblockid_), static_cast<uint8_t>(tbl.second.instid_), reinterpret_cast<Ls3CtlApiCmdClbk>(&MiconCommMostControlLs3CtlCallback))};
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//			int32_t fret_most{ret_most};
//			CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-R-112_1",fret_most);
//			int32_t* p_ret_most{const_cast<int32_t*>(&ret_most)};
//			*p_ret_most = fret_most;
//#endif
			if (LS3_CTL_RET_SUCCESS == ret_most)
			{
				/* 正常終了 */
				done = true;
			}
			else if (LS3_CTL_RET_ERR_PARAM == ret_most)
			{
				/* パラメータエラー */
				result = VHAL_ERR;
				VHAL_LOGE("Ls3CtlApiCmdAddCallback() FblockID=0x%x, InstID=0x%x, ret_most=%d", tbl.second.fblockid_, tbl.second.instid_, ret_most);
				done = true;
			}
			else
			{
				/* その他エラー */
				if (kRetryCntLs3CtlApi <= (i+1U))
				{
					result = VHAL_ERR;
					VHAL_LOGE("Ls3CtlApiCmdAddCallback() FblockID=0x%x, InstID=0x%x, count=%d/%d, ret_most=%d", tbl.second.fblockid_, tbl.second.instid_, (i+1U), kRetryCntLs3CtlApi, ret_most);
					done = true;
				}
				sif_mdelay(kRetryWaitLs3CtlApi);
			}
		}
	}

	if (VHAL_SUCCESS == result)
	{
		bool done{false};
		for(uint32_t i{0U}; (!done) && (i<kRetryCntLs3CtlApi); ++i)
		{
			/* VideoMode結果通知用 */
			const auto iter = cmd_head_table_.find(MostCmdType::kRearDisplayVideoMode);
			if ((iter != cmd_head_table_.end()))
			{
				/* LS3CTLコマンド受信通知 */
				const int32_t ret_most{Ls3CtlApiCmdAddCallback(&ls3ctl_obj_, static_cast<uint8_t>(iter->second.fblockid_), static_cast<uint8_t>(iter->second.instid_), reinterpret_cast<Ls3CtlApiCmdClbk>(&MiconCommMostControlLs3CtlCallback))};
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//				int32_t fret_most{ret_most};
//				CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-R-112_2",fret_most);
//				int32_t* p_ret_most{const_cast<int32_t*>(&ret_most)};
//				*p_ret_most = fret_most;
//#endif
				if (LS3_CTL_RET_SUCCESS == ret_most)
				{
					/* 正常終了 */
					done = true;
				}
				else if (LS3_CTL_RET_ERR_PARAM == ret_most)
				{
					/* パラメータエラー */
					result = VHAL_ERR;
					VHAL_LOGE("Ls3CtlApiCmdAddCallback() FblockID=0x%x, InstID=0x%x, ret_most=%d", iter->second.fblockid_, iter->second.instid_, ret_most);
					done = true;
				}
				else
				{
					/* その他エラー */
					if (kRetryCntLs3CtlApi <= (i+1U))
					{
						result = VHAL_ERR;
						VHAL_LOGE("Ls3CtlApiCmdAddCallback() FblockID=0x%x, InstID=0x%x, count=%d/%d, ret_most=%d", iter->second.fblockid_, iter->second.instid_, (i+1U), kRetryCntLs3CtlApi, ret_most);
						done = true;
					}
				}
			}
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	LS3CTLコールバック削除
 引数    ：	なし
 戻り値  ：	なし
 フェールセーフNo  ：	F-VHAL-N-113
                    	F-VHAL-C-114
*****************************************************************************/
void CVhalMiconCommMostControl::ClearCallBack(void)
{
	/* SourceActivity結果通知用 */
	/* 後席系映像パス情報 */
	for (const auto& tbl : most_rear_path_table_)
	{
		/* LS3CTLコマンド受信通知削除 */
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t ret_most{Ls3CtlApiCmdDelCallback(&ls3ctl_obj_, static_cast<uint8_t>(tbl.second.fblockid_), static_cast<uint8_t>(tbl.second.instid_))};
//		int32_t fail_ret{0};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-113_1",fail_ret)};
//		if(true == fail)
//		{
//			ret_most = fail_ret;
//		}
//#else
		const int32_t ret_most{Ls3CtlApiCmdDelCallback(&ls3ctl_obj_, static_cast<uint8_t>(tbl.second.fblockid_), static_cast<uint8_t>(tbl.second.instid_))};
//#endif
		if (LS3_CTL_RET_SUCCESS != ret_most)
		{
			/* 異常終了 */
			/* 失敗しても削除処理継続 */
			VHAL_LOGE("Ls3CtlApiCmdDelCallback() FblockID=0x%x, InstID=0x%x, ret_most=%d", tbl.second.fblockid_, tbl.second.instid_, ret_most);
		}
	}

	/* VideoMode結果通知用 */
	const auto iter = cmd_head_table_.find(MostCmdType::kRearDisplayVideoMode);
	if ((iter != cmd_head_table_.end()))
	{
		/* LS3CTLコマンド受信通知削除 */
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t ret_most{Ls3CtlApiCmdDelCallback(&ls3ctl_obj_, static_cast<uint8_t>(iter->second.fblockid_), static_cast<uint8_t>(iter->second.instid_))};
//		int32_t fail_ret{0};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-113_2",fail_ret)};
//		if(true == fail)
//		{
//			ret_most = fail_ret;
//		}
//#else
		const int32_t ret_most{Ls3CtlApiCmdDelCallback(&ls3ctl_obj_, static_cast<uint8_t>(iter->second.fblockid_), static_cast<uint8_t>(iter->second.instid_))};
//#endif
		if (LS3_CTL_RET_SUCCESS != ret_most)
		{
			/* 異常終了 */
			VHAL_LOGE("Ls3CtlApiCmdDelCallback() FblockID=0x%x, InstID=0x%x, ret_most=%d", iter->second.fblockid_, iter->second.instid_, ret_most);
		}
	}

	return;
}

} /* namespace videohal */

