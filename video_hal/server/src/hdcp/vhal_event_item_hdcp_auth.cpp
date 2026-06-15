/*******************************************************************************
    機能名称    ：  認証結果イベントモジュール
    ファイル名称：  vhal_event_item_hdcp_auth.cpp
*******************************************************************************/
#include "vhal_event_item_hdcp_auth.h"
#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_round_cast.h"

namespace videohal
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalHdcpMiscReceiver::CVhalHdcpMiscReceiver(void)
	:p_hdcp_auth_rslt_listener_(nullptr)
{
	VHAL_LOGV("CVhalHdcpMiscReceiver is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalHdcpMiscReceiver::~CVhalHdcpMiscReceiver(void)
{
	VHAL_LOGV("CVhalHdcpMiscReceiver is deleted. this=%p", this);
	ClearEventListener();
}

/*****************************************************************************
 処理概要：	MISCコマンド通知受信処理（VideoHALイベントスレッドからのコール）
 引数    ：	const std::vector<uint8_t>&	data	(i)	コマンド情報ポインタ
 戻り値  ：	なし
*****************************************************************************/
void CVhalHdcpMiscReceiver::Receive(const std::vector<uint8_t>& data)
{
	/* データ要素数は2個以上であること(data_type+sub_type) */
	if (kHdcpMiscDataSizeMin > data.size())
	{
		VHAL_LOGE("parameter error. data.size is %lu", data.size());
	}
	else if (kDatatypeHdmi == data[data.size() - 1])
	{
		switch (data[kHdcpMiscDataOpc])
		{
			case SUB_TYPE_HDCP_AUTH_NTY_CDISP:
			case SUB_TYPE_HDCP_AUTH_NTY_RSE:	/* (追加予定) */
				MakeHdcpAuthKey(data);
				break;
			case SUB_TYPE_HDCP_AUTH_KEY_CLEAR:
				ClearHdcpAuthKey(data);
				break;
			case SUB_TYPE_HDMI_CONNECT:
			case SUB_TYPE_HDMI_VIDEO_FORMAT:
			case SUB_TYPE_HDMI_AUDIO_FORMAT:
				/* 無処理 */
				break;
			default:
				VHAL_LOGE("invalid sub_type. 0x%X", data[kHdcpMiscDataOpc]);
				break;
		}
	}
	else
	{
		/* 無処理 */
	}
}

/*****************************************************************************
 処理概要：	MISCコマンド通知受信事前通知処理 (OS間通信コールバックからのコール）
 引数    ：	const std::vector<uint8_t>&	data	(i)	コマンド情報ポインタ
 戻り値  ：	なし
*****************************************************************************/
void CVhalHdcpMiscReceiver::ReceivePreNotify(const std::vector<uint8_t>& data) noexcept
{
	/* 処理なし（Receiveで処理実施） */
}

/*****************************************************************************
 処理概要：	受信アイテム種別取得
 引数    ：	なし
 戻り値  ：	受信アイテム種別
           		RECEIVE_ITEM_TYPE_MISC
*****************************************************************************/
CVhalMiconReceiveItem::ReceiveItemType CVhalHdcpMiscReceiver::GetItemType(void) const noexcept
{
	return CVhalMiconReceiveItem::ReceiveItemType::RECEIVE_ITEM_TYPE_MISC;
}

/*****************************************************************************
 処理概要：	イベントリスナー登録
 引数    ：	CVhalHdcpAuthRsltListenerBase* p_listener		(i)リスナーポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_SUCCESS			正常終了
****************************************************************************/
int32_t CVhalHdcpMiscReceiver::RegisterEventListener(CVhalHdcpAuthRsltListenerBase* const p_listener)
{
	int32_t	result{VHAL_SUCCESS};

	if (nullptr == p_listener)
	{
		VHAL_LOGE("p_listener is null.");
		result = VHAL_ERR_PARAM;
	}
	else
	{
		p_hdcp_auth_rslt_listener_ = p_listener;
	}

	return result;
}

/*****************************************************************************
 処理概要：	イベントリスナー解除
 引数    ：	なし
 戻り値  ：	なし
****************************************************************************/
void CVhalHdcpMiscReceiver::ClearEventListener(void) noexcept
{
	p_hdcp_auth_rslt_listener_ = nullptr;
}

/*****************************************************************************
 処理概要：	HDCP認証情報通知
 引数    ：	const std::vector<uint8_t>& data	(i) 受信データ
 戻り値  ：	なし
*****************************************************************************/
void CVhalHdcpMiscReceiver::MakeHdcpAuthKey(const std::vector<uint8_t>& data) noexcept
{
	VHAL_LOGV_IN();
	constexpr	std::size_t		kDataHeaderLength{7U};	/* SubType, HDCP認証結果, MAX_DEVS, DEVICE_COUNT, MAX_CASCADE, DEPTH, data_type */
	constexpr	uint32_t		kDataIdxSubType{kHdcpMiscDataOpc};
	constexpr	uint32_t		kDataIdxResult{1U};
	constexpr	uint32_t		kDataIdxMaxDevs{2U};
	constexpr	uint32_t		kDataIdxDevCount{3U};
	constexpr	uint32_t		kDataIdxMaxCascade{4U};
	constexpr	uint32_t		kDataIdxDepth{5U};
	constexpr	uint32_t		kDataIdxReceiverIds{6U};
	constexpr	std::pair<uint8_t, int32_t>	kDataResultTable[2] {
		{ 0x00U , VHAL_HDCP_FIRST_AUTH_STS_SUCCESS	},	/* 認証成功 */
		{ 0x01U , VHAL_HDCP_FIRST_AUTH_STS_FAILED	},	/* 認証失敗 */
	};

	/* 最低でも 固定部分(6バイト)+data_typeの7バイト以上であること */
	if (kDataHeaderLength <= data.size())
	{
		CVhalHdcpAuthRsltData	hdcp_result;					/* HDCP認証キー(コンストラクタで初期値設定) */

		/* 固定部分の設定 */
		hdcp_result.SetSubType(data[kDataIdxSubType]);			/* SubType */
		hdcp_result.SetMaxDevs(data[kDataIdxMaxDevs]);			/* MAX_DEVS_EXCEED */
		hdcp_result.SetDevsCount(data[kDataIdxDevCount]);		/* DEVICE_COUNT */
		hdcp_result.SetMaxCascade(data[kDataIdxMaxCascade]);	/* MAX_CASCADE_EXCEEDED */
		hdcp_result.SetCascadeDepth(data[kDataIdxDepth]);		/* CASCADE_DEPTH */
		hdcp_result.SetResult(VHAL_HDCP_FIRST_AUTH_STS_NONE);	/* HDCP認証結果(init) */
		for (auto iter : kDataResultTable)
		{
			if (iter.first == data[kDataIdxResult])				/* found */
			{
				hdcp_result.SetResult(iter.second);				/* HDCP認証結果 */
				break;
			}
		}

		/* HDCP認証鍵の設定 */
		constexpr uint32_t	kReceiverIdSize{CVhalHdcpAuthRsltData::khdcpReceiverIdSize};
		uint32_t	left_size{SizeToUI32(data.size() - kDataHeaderLength)};
		uint32_t	pos{kDataIdxReceiverIds};
		for (uint8_t i{0U}; i < data[kDataIdxDevCount]; ++i)
		{
			/* 32以上 または データ不足の場合は中断 */
			if ((CVhalHdcpAuthRsltData::kMaxDeviceCount <= i) || (kReceiverIdSize > left_size))
			{
				hdcp_result.SetDevsCount(i);
				break;
			}
			/* 5バイトごとにレシーバーIDを設定 */
			std::vector<uint8_t> receiver_id{};
			for (uint32_t j{0U}; j < kReceiverIdSize; ++j)
			{
				receiver_id.push_back(data[pos]);
				++pos;
				--left_size;
			}
			hdcp_result.AddReceiverIds(receiver_id);
		}

		if (nullptr != p_hdcp_auth_rslt_listener_)
		{
			hdcpAuthType type{hdcpAuthType::HDCP_AUTH_TYPE_CDISP};
//			if (SUB_TYPE_HDCP_AUTH_NTY_RSE == data[kDataIdxSubType])	(予定)
//			{
//				type = hdcpAuthType::HDCP_AUTH_TYPE_RSE
//			}
			p_hdcp_auth_rslt_listener_->NotifyHdcpAuthResult(type, hdcp_result);
		}
	}
	else
	{
		VHAL_LOGE("data too short. size=%llu", data.size());
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	HDCP認証キークリア通知
 引数    ：	const std::vector<uint8_t>& data	(i) 受信データ
 戻り値  ：	なし
*****************************************************************************/
void CVhalHdcpMiscReceiver::ClearHdcpAuthKey(const std::vector<uint8_t>& data) noexcept
{
	VHAL_LOGV_IN();
	if (nullptr != p_hdcp_auth_rslt_listener_)
	{
		p_hdcp_auth_rslt_listener_->NotifyHdcpAuthClear();
	}
	VHAL_LOGV_OUT();
}

#if 0	// (予定) RSE_HDCP認証要求送信クラス
/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalHdcpAuthRseSendItem::CVhalHdcpAuthRseSendItem(void)
	:opc_(SUB_TYPE_HDCP_AUTH_REQ_RSE)
{
}

/*****************************************************************************
 処理概要：	MISC 送信データ構築
 引数    ：	std::vector<uint8_t> &send_data		(o)送信データ
 戻り値  ：	処理結果
           		VHAL_ERR				異常終了
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalHdcpAuthRseSendItem::Build(std::vector<uint8_t> &send_data) const
{
	int32_t result{VHAL_ERR};

	if (true == send_data.empty())
	{
		send_data.push_back(opc_);
		result = VHAL_SUCCESS;
	}
	return result;
}

/*****************************************************************************
 処理概要：	送信アイテム種別取得
 引数    ：	なし
 戻り値  ：	送信アイテム種別
*****************************************************************************/
CVhalMiconSendItem::SendItemType CVhalHdcpAuthRseSendItem::GetItemType(void) const noexcept
{
	return CVhalMiconSendItem::SendItemType::SEND_ITEM_TYPE_MISC;
}
#endif

} /* namespace videohal */

