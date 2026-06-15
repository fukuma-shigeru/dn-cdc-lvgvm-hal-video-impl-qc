/*******************************************************************************
    機能名称    ：  VideoHAL接続クライアントモジュール
    ファイル名称：  vhal_property_client.cpp
*******************************************************************************/
#include "vhal_property_client.h"

#include <iostream>
#include <set>
#include <cstring>
#include <unistd.h>

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_property_control.h"
#include "vhal_main_control.h"
#include "vhal_str_mng.h"
#include "vhal_debug_system.h"

namespace videohal
{
namespace
{
/* Hal通信ライブラリリトライ回数 */
constexpr int32_t kHalComRetryCount{15};
/* Hal通信ライブラリリトライ待ち時間(ms) */
constexpr int32_t kHalComRetryWait{100};
}

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalPropertyClient::CVhalPropertyClient(void)
	:p_main_(nullptr)
	,p_prop_(nullptr)
	,halcomm_obj_()
	,reqMsg_()
	,ackMsg_()
{
	VHAL_LOGV("CVhalPropertyClient is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalPropertyClient::~CVhalPropertyClient(void)
{
	VHAL_LOGV("CVhalPropertyClient is deleted. this=%p", this);
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalPropertyClient::Initialize(void) noexcept
{
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ：	CVhalMainControl	*p_main		(i)メインコントロールインスタンスポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM				パラメータ不正
           		VHAL_ERR_NOT_INITIALIZED	未初期化エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalPropertyClient::Initialize(CVhalMainControl * const p_main)
{
	if (nullptr == p_main)
	{
		return VHAL_ERR_PARAM;
	}

	p_main_ = p_main;
	p_prop_ = GetPropertyControl(p_main_);
	if (nullptr == p_prop_)
	{
		VHAL_LOGE("GetPropertyControl return nullptr");
		return VHAL_ERR_NOT_INITIALIZED;
	}

	const int32_t ret{p_main_->RegisterEventSource(this)};
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("p_main_->RegisterEventSource ret=%d", ret);
	}
	return Initialize();
}

/*****************************************************************************
 処理概要：	終了処理（内部リソースの解放）
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalPropertyClient::Finalize(void)
{
	if (nullptr != p_main_)
	{
		p_main_->ClearEventSource(this);
	}

	const int32_t ret{HalCommClose(&halcomm_obj_)};
	if (HALCOMM_RET_OK != ret)
	{
		VHAL_LOGW("HalCommClose error. ret=%d", ret);
	}
}

/*****************************************************************************
 処理概要：	イベント受信用fdの取得
 引数    ：	なし
 戻り値  ：	イベント受信用fd
*****************************************************************************/
int32_t CVhalPropertyClient::GetSourceFd(void) const
{
	if (0 == halcomm_obj_.fd)
	{
		VHAL_LOGW("fd is 0. HalComm client is not accepted successfully.");
	}

	return halcomm_obj_.fd;
}

/*****************************************************************************
 処理概要：	イベントの読み込みと実行
 引数    ：	uint32_t	source_event	(i)イベントフラグ
 戻り値  ：	処理結果
           		VHAL_ERR_CLIENT_DISCONNECTED	クライアント切断
           		VHAL_ERR_HALCOMM				Halcommエラー
           		VHAL_SUCCESS					正常終了
 フェールセーフNo  ：	F-VHAL-R-003
                    	F-VHAL-N-004
                    	F-VHAL-N-005
                    	F-VHAL-R-006
*****************************************************************************/
int32_t CVhalPropertyClient::ExecEvent(const uint32_t source_event)
{
	VHAL_LOGV_IN();

	if (0U != (source_event & (kSourceEventHungup | kSourceEventError)))
	{
		/* クライアントリクエスト待ちソケットでエラー発生。 */
		/* クライアント切断とみなして処理継続 */
		VHAL_LOGI("CVhalPropertyClient: received fd error event. source_event=%d. Assume this client disconnected.", source_event);
		return VHAL_ERR_CLIENT_DISCONNECTED;
	}

	if (0U != (source_event & kSourceEventReadable))
	{
		int32_t ret;

		/* サイズが大きすぎるためメンバー変数で確保。毎回0クリアする */
		(void)memset(&reqMsg_, 0, sizeof(reqMsg_));
		(void)memset(&ackMsg_, 0, sizeof(ackMsg_));

		ret = halcomm::HalCommRecv(&halcomm_obj_, &reqMsg_, kHalComRetryWait, kHalComRetryCount);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret{0};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-003",fail_ret)};
//		if(true == fail)
//		{
//			ret = fail_ret;
//			memset(&reqMsg_, 0, sizeof(reqMsg_));
//		}
//#endif
		if (HALCOMM_RET_CONTINUE == ret)
		{
			/* いったん処理中断してイベント待ちからやり直し */
			VHAL_LOGD("continue event read");
			return VHAL_SUCCESS;
		}
		else if (HALCOMM_RET_ERR_DISCON == ret)
		{
			/* クライアントからの再接続を待つ */
			VHAL_LOGD("wait connected");
			return VHAL_ERR_CLIENT_DISCONNECTED;
		}
		else if (HALCOMM_RET_ERR_ARG == ret)
		{
			/* 未定義のデータ */
			VHAL_LOGE("HalCommRecv(server) error. ret=%d", ret);
		}
		else if( HALCOMM_RET_OK != ret )
		{
			/* リトライ失敗 */
			VHAL_LOGE("HalCommRecv(server) invalid data. ret=%d", ret);
		}
		else
		{
			/* 処理なし */
		}

		if (HALCOMM_RET_OK != ret)
		{
			VHAL_LOGE("HalCommRecv(server) error. ret=%d", ret);
			return VHAL_ERR_HALCOMM;
		}

		bool set_complete_needed{false};
		ackMsg_.dataType = halcomm::HALCOMM_DATATYPE_JSON;
		/* 一部プロパティでエラーになっても処理は継続して、エラーログ出力して成功終了とする。 */
		for (uint32_t i{0U}; i < HALCOMM_DATA_MAX; ++i)
		{
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//			fail = CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-004",fail_ret);
//			if(true == fail)
//			{
//				reqMsg_.data.json[i].methodType = static_cast<halcomm::HalCommMethodType_t>(fail_ret);
//			}
//#endif
			ackMsg_.data.json[i].id = reqMsg_.data.json[i].id;
			switch (reqMsg_.data.json[i].methodType)
			{
				case halcomm::HALCOMM_METHOD_SET:
					ackMsg_.data.json[i].methodType = halcomm::HALCOMM_METHOD_SETACK;
					ackMsg_.data.json[i].data.setAckData.result = VHAL_API_ERR;
					/* SetSuspend()の呼び出し元はStrStateEvent()のみであり、*/
					/* ExecEvent()と同じVideoHALメインスレッドで実行されるため */
					/* GetSuspend()との間にTOCTOU競合は発生しない */
					if (!videohal::CVhalStrManager::GetSuspend())
					{
						ret = SetValue(&reqMsg_.data.json[i].data.setReqData, &ackMsg_.data.json[i].data.setAckData);
						if (VHAL_SUCCESS != ret)
						{
							VHAL_LOGE("SetValue ret=%d", ret);
						}
						set_complete_needed = true;
					}
					break;

				case halcomm::HALCOMM_METHOD_GET:
					ackMsg_.data.json[i].methodType = halcomm::HALCOMM_METHOD_GETACK;
					ackMsg_.data.json[i].data.getAckData.result = VHAL_API_ERR;
					if (!videohal::CVhalStrManager::GetSuspend())
					{
						ret = GetValue(&reqMsg_.data.json[i].data.getReqData, &ackMsg_.data.json[i].data.getAckData);
						if (VHAL_SUCCESS != ret)
						{
							VHAL_LOGE("GetValue ret=%d", ret);
						}
					}
					break;

				case halcomm::HALCOMM_METHOD_SUB:
					ackMsg_.data.json[i].methodType = halcomm::HALCOMM_METHOD_SUBACK;
					ackMsg_.data.json[i].data.subAckData.result = VHAL_API_ERR;
					if (!videohal::CVhalStrManager::GetSuspend())
					{
						ret = Subscribe(&reqMsg_.data.json[i].data.subReqData, &ackMsg_.data.json[i].data.subAckData);
						if (VHAL_SUCCESS != ret)
						{
							VHAL_LOGE("Subscribe ret=%d", ret);
						}
					}
					break;

				case halcomm::HALCOMM_METHOD_UNSUB:
					ackMsg_.data.json[i].methodType = halcomm::HALCOMM_METHOD_UNSUBACK;
					ackMsg_.data.json[i].data.unSubAckData.result = VHAL_API_ERR;
					if (!videohal::CVhalStrManager::GetSuspend())
					{
						ret = UnSubscribe(&reqMsg_.data.json[i].data.unSubReqData, &ackMsg_.data.json[i].data.unSubAckData);
						if (VHAL_SUCCESS != ret)
						{
							VHAL_LOGE("UnSubscribe ret=%d", ret);
						}
					}
					break;

				/* データなし */
				case halcomm::HALCOMM_METHOD_INVALID:
					ackMsg_.data.json[i].methodType = halcomm::HALCOMM_METHOD_INVALID;
					break;

				/* その他は、サーバー側では無効 */
				default:
					VHAL_LOGW("methodType is invalid. methodType=%d", reqMsg_.data.json[i].methodType);
					ackMsg_.data.json[i].methodType = halcomm::HALCOMM_METHOD_INVALID;
					break;
			}
		}
		
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		fail = CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-006",fail_ret);
//		if(true == fail)
//		{
//			memset(&ackMsg_, 0, sizeof(ackMsg_));
//		}
//#endif
		ret = halcomm::HalCommSend(&halcomm_obj_, &ackMsg_, kHalComRetryWait, kHalComRetryCount);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		fail = CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-006",fail_ret);
//		if(true == fail)
//		{
//			ret = fail_ret;
//		}
//#endif
		if (HALCOMM_RET_OK != ret)
		{
			VHAL_LOGEW("HalCommSend error. ret=%d", ret);
			return VHAL_ERR_HALCOMM;
		}

		/* SetValueがあった場合Ackを返してから更新通知を行う。 */
		if (true == set_complete_needed)
		{
			p_prop_->SetValueComplete();
		}
	}

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	HalCommonクライアントオブジェクトの取得
 引数    ：	なし
 戻り値  ：	HalCommonクライアントオブジェクトポインタ
*****************************************************************************/
halcomm::HalCommObj_t *GetHalCommClient(CVhalPropertyClient * const p_client) noexcept
{
	return &p_client->halcomm_obj_;
}

/*****************************************************************************
 処理概要：	SetValue処理の実行
 引数    ：	halcomm::HalCommSetReq_t*	p_req	(i)受信データ
           	halcomm::HalCommSetAck_t*	p_ack	(i)返信データ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalPropertyClient::SetValue(const halcomm::HalCommSetReq_t* const p_req, halcomm::HalCommSetAck_t* const p_ack)
{
	int32_t ret{};
	int32_t total_result{VHAL_API_SUCCESS};
	bool no_more_data{false};

	VHAL_LOGV("Request received.");

	for (uint32_t i{0U}; i < HALCOMM_UNIT_MAX; ++i)
	{
		ret = 0;
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret{0};
//		halcomm::HalCommUnitType_t type{};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-005",fail_ret)};
//		if(true == fail)
//		{
//			type = static_cast<halcomm::HalCommUnitType_t>(fail_ret);
//		}
//		else
//		{
//			type = p_req->params[i].unitType;
//		}
//		switch (type)
//#else
		switch (p_req->params[i].unitType)
//#endif
		{
			case halcomm::HALCOMM_UNITTYPE_INVALID:
				/* もうデータがないとみなしてループ終了 */
				no_more_data = true;
				break;

			case halcomm::HALCOMM_UNITTYPE_INTEGER:
				ret = p_prop_->SetValueNumber(p_req->params[i].name, p_req->params[i].unit.number);
				break;

			case halcomm::HALCOMM_UNITTYPE_STRING:
				ret = p_prop_->SetValueString(p_req->params[i].name, p_req->params[i].unit.string);
				break;

			case halcomm::HALCOMM_UNITTYPE_BOOL:
				ret = p_prop_->SetValueBool(p_req->params[i].name, p_req->params[i].unit.flg);
				break;

			default: /* その他のデータは非対応 */
				ret = VHAL_ERR_PARAM;
				break;
		}

		if (0 > ret)
		{
			if((VHAL_ERR_PARAM == ret) || (VHAL_ERR_PROPERTY_ENTRY == ret))
			{
				total_result = VHAL_API_ERR_PARAM;
			}
			else
			{
				total_result = VHAL_API_ERR;
			}
		}

		if (true == no_more_data)
		{
			break;
		}
	}

	p_ack->result = total_result;
	VHAL_LOGI("method:SETACK result:%d", total_result);

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	GetValue処理の実行
 引数    ：	halcomm::HalCommGetReq_t*	p_req	(i)受信データ
           	halcomm::HalCommGetAck_t*	p_ack	(i)返信データ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalPropertyClient::GetValue(const halcomm::HalCommGetReq_t* const p_req, halcomm::HalCommGetAck_t* const p_ack)
{
	int32_t ret{};
	int32_t total_result{VHAL_API_SUCCESS};
	bool no_more_data{false};

	VHAL_LOGV("Request received.");

	for (uint32_t i{0U}; i < HALCOMM_UNIT_MAX; ++i)
	{
		ret = 0;

		/* プロパティ名やタイプなどをまとめてコピーする。 */
		(void)memcpy(&p_ack->params[i], &p_req->params[i], sizeof(p_ack->params[i]));
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret{0};
//		halcomm::HalCommUnitType_t type{};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-005",fail_ret)};
//		if(true == fail)
//		{
//			type = static_cast<halcomm::HalCommUnitType_t>(fail_ret);
//		}
//		else
//		{
//			type = p_req->params[i].unitType;
//		}
//		switch (type)
//#else
		switch (p_req->params[i].unitType)
//#endif
		{
			case halcomm::HALCOMM_UNITTYPE_INVALID:
				/* もうデータがないとみなしてループ終了 */
				no_more_data = true;
				break;

			case halcomm::HALCOMM_UNITTYPE_INTEGER:
				ret = p_prop_->GetValueNumber(p_req->params[i].name, p_ack->params[i].unit.number);
				break;

			case halcomm::HALCOMM_UNITTYPE_STRING:
			{
				std::string value{};
				ret = p_prop_->GetValueString(p_req->params[i].name, value);
				if (VHAL_SUCCESS == ret)
				{
					(void)strncpy(p_ack->params[i].unit.string, value.c_str(), sizeof(p_ack->params[i].unit.string) - 1U);
				}
				break;
			}

			case halcomm::HALCOMM_UNITTYPE_BOOL:
				ret = p_prop_->GetValueBool(p_req->params[i].name, p_ack->params[i].unit.flg);
				break;

			default:
				/* その他のデータは非対応 */
				ret = VHAL_ERR_PARAM;
				break;
		}

		if (0 > ret)
		{
			if((VHAL_ERR_PARAM == ret) || (VHAL_ERR_PROPERTY_ENTRY == ret))
			{
				total_result = VHAL_API_ERR_PARAM;
			}
			else
			{
				total_result = VHAL_API_ERR;
			}
		}

		if (true == no_more_data)
		{
			break;
		}
	}

	p_ack->result = total_result;
	VHAL_LOGI("method:GETACK result:%d", total_result);

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	Subscribe処理の実行
 引数    ：	halcomm::HalCommSubReq_t*	p_req	(i)受信データ
           	halcomm::HalCommSubAck_t*	p_ack	(i)返信データ
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalPropertyClient::Subscribe(halcomm::HalCommSubReq_t* const p_req, halcomm::HalCommSubAck_t* const p_ack)
{
	VHAL_LOGV("Request received.");

	for (uint32_t i{0U}; i < HALCOMM_UNIT_MAX; ++i)
	{
		if ('\0' == p_req->params[i].name[0])
		{
			/* もうデータがないとみなしてループ終了 */
			break;
		}

		(void)subscriptions_.insert(p_req->params[i].name);
		VHAL_LOGI("method:SUBSCRIBE name:%s", p_req->params[i].name);
	}

	VHAL_LOGV("subscription size=%ld", subscriptions_.size());
	p_ack->result = VHAL_API_SUCCESS;
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-005",fail_ret)};
//	if(true == fail)
//	{
//		p_ack->result = VHAL_API_ERR_PARAM;
//	}
//#endif
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	UnSubscribe処理の実行
 引数    ：	halcomm::HalCommUnSubReq_t*	p_req	(i)受信データ
           	halcomm::HalCommUnSubAck_t*	p_ack	(i)返信データ
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalPropertyClient::UnSubscribe(halcomm::HalCommUnSubReq_t* const p_req, halcomm::HalCommUnSubAck_t* const p_ack)
{
	VHAL_LOGV("Request received.");

	for (uint32_t i{0U}; i < HALCOMM_UNIT_MAX; ++i)
	{
		if ('\0' == p_req->params[i].name[0])
		{
			/* もうデータがないとみなしてループ終了 */
			break;
		}

		(void)subscriptions_.erase(p_req->params[i].name);
		VHAL_LOGI("method:UNSUBSCRIBE name:%s", p_req->params[i].name);
	}

	VHAL_LOGV("subscription size=%ld", subscriptions_.size());
	p_ack->result = VHAL_API_SUCCESS;
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-005",fail_ret)};
//	if(true == fail)
//	{
//		p_ack->result = VHAL_API_ERR_PARAM;
//	}
//#endif
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	プロパティ更新通知
 引数    ：	std::vector<std::string>	&property_names	(i)更新プロパティリスト
 戻り値  ：	処理結果
           		VHAL_ERR_HALCOMM	Halcommエラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalPropertyClient::Publish(std::vector<std::string> &property_names)
{
	/* サスペンド実施中は更新通知をしない */
	if (videohal::CVhalStrManager::GetSuspend())
	{
		return VHAL_SUCCESS;
	}

	std::vector<std::string> publish_names{};
	VHAL_LOGV_IN();
	int32_t result{VHAL_SUCCESS};

	for (auto itr_prop = property_names.begin(); itr_prop != property_names.end(); ++itr_prop)
	{

		const auto itr_name = std::find(publish_names.begin(), publish_names.end(), *itr_prop);
		if (itr_name == publish_names.end())
		{
			for (auto itr_subsc = subscriptions_.begin(); itr_subsc != subscriptions_.end(); ++itr_subsc)
			{
				const std::string& name{*itr_prop};
				const std::string& prefix{*itr_subsc};
			
				if ((name.size() >= prefix.size()) &&
					std::equal(std::begin(prefix), std::end(prefix), std::begin(name)))
				{
					/* 通知が必要（subscribeされた文字列が前方一致した）プロパティ名を集める。重複は無効 */
					(void)publish_names.push_back(prefix);
				}
			}
		}
	}

	if (0U != publish_names.size())
	{
		uint32_t i{0U};
		halcomm::HalCommPublish_t* p_publish{nullptr};

		(void)memset(&ackMsg_, 0, sizeof(ackMsg_));

		ackMsg_.dataType = halcomm::HALCOMM_DATATYPE_JSON;
		ackMsg_.data.json[0].methodType = halcomm::HALCOMM_METHOD_PUBLISH;
		p_publish = &ackMsg_.data.json[0].data.publishData;

		/* HalCommonを使ってpublish送信 */
		for (const auto& itr_name: publish_names)
		{
			(void)strncpy(p_publish->params[i].name, itr_name.c_str(), sizeof(p_publish->params[i].name)-1U);
			/* 0のまま（HALCOMM_UNITTYPE_INVALID）ではHalCommSendが送信してくれないため固定で指定する。 */
			p_publish->params[i].unitType = halcomm::HALCOMM_UNITTYPE_STRING;
			i++;
			VHAL_LOGI("method:PUBLISH name:%s", itr_name.c_str());
			if (HALCOMM_UNIT_MAX == i)
			{
				VHAL_LOGW("publish HALCOMM_UNIT_MAX over. The extra info is discarded.");
				break;
			}
		}

		const int32_t ret{halcomm::HalCommSend(&halcomm_obj_, &ackMsg_, kHalComRetryWait, kHalComRetryCount)};
		if (HALCOMM_RET_OK != ret)
		{
			VHAL_LOGE("HalCommSend error. ret=%d", ret);
			result = VHAL_ERR_HALCOMM;
		}
	}

	VHAL_LOGV_OUT();
	return result;
}

} /* namespace videohal */
