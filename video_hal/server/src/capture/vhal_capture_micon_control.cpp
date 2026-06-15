/*******************************************************************************
    機能名称    ：  マイコン間通信処理モジュール(キャプチャ)
    ファイル名称：  vhal_capture_micon_control.cpp
*******************************************************************************/
#include "vhal_capture_micon_control.h"
#include "vhal_micon_misc_opc.h"

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_round_cast.h"

extern "C"
{
#include "StdGType.h"
#include "TabPower.h"
}

/*****************************************************************************
*	define																	 *
*****************************************************************************/

/*****************************************************************************
*	globals																	 *
*****************************************************************************/

/*****************************************************************************
*	statics																	 *
*****************************************************************************/

namespace videohal
{

/* 表示用カメラ映像サイズ管理テーブル */
const std::unordered_map<CVhalCaptureTabReceiver::CameraSizeMode, CVhalCaptureTabReceiver::VhalCameraSizeData> CVhalCaptureTabReceiver::cam_size_entries_
{
	{CameraSizeMode::CAM_SIZE_1920_1080,	{1920,	1080}},
	{CameraSizeMode::CAM_SIZE_1920_954,		{1920,	954	}},
	{CameraSizeMode::CAM_SIZE_1920_900,		{1920,	900	}},
	{CameraSizeMode::CAM_SIZE_1920_720,		{1920,	720	}},
	{CameraSizeMode::CAM_SIZE_1280_846,		{1280,	846	}},
	{CameraSizeMode::CAM_SIZE_1280_720,		{1280,	720	}},
	{CameraSizeMode::CAM_SIZE_1280_621,		{1280,	621	}},
	{CameraSizeMode::CAM_SIZE_1696_954,		{1696,	954	}},
	{CameraSizeMode::CAM_SIZE_1104_621,		{1104,	621	}},
	{CameraSizeMode::CAM_SIZE_1672_720,		{1672,	720	}},
};

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalCaptureTabReceiver::CVhalCaptureTabReceiver(void)
	:camera_path_sync_status_(VhalTabCamSync::kUnknown)
	,camera_sync_status_(VhalTabCamSync::kUnknown)
	,camera_vic_status_(kTabRecvDataVicStatusInit)
	,center_display_status_(kTabCamReqUnknown)
	,camera_size_{static_cast<uint8_t>(CameraSizeMode::CAM_SIZE_UNKNOWN)}
	,prev_camera_type_()
{
	VHAL_LOGV("CVhalCaptureTabReceiver is created. this=%p", this);
	p_capture_tab_receiver_= this;
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalCaptureTabReceiver::~CVhalCaptureTabReceiver(void)
{
	VHAL_LOGV("CVhalCaptureTabReceiver is deleted. this=%p", this);
	ClearEventListener();
}

/*****************************************************************************
 処理概要：	再初期化処理
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalCaptureTabReceiver::ReInit(void) noexcept
{
	/* 各メンバ変数の初期化 */
	camera_path_sync_status_ = VhalTabCamSync::kUnknown;
	camera_sync_status_ = VhalTabCamSync::kUnknown;
	camera_vic_status_ = kTabRecvDataVicStatusInit;
	center_display_status_ = kTabCamReqUnknown;
	camera_size_[VHAL_CAMERA_DISPLAY_CENTER] = static_cast<uint8_t>(CameraSizeMode::CAM_SIZE_UNKNOWN);
	prev_camera_type_.clear();
}

/*****************************************************************************
 処理概要：	TABコマンド通知受信処理（VideoHALメインスレッドからのコール）
 引数    ：	const std::vector<uint8_t>&	data	(i)	コマンド情報ポインタ
 戻り値  ：	なし
*****************************************************************************/
void CVhalCaptureTabReceiver::Receive(const std::vector<uint8_t>& data)
{
	/* データ要素数は3個未満(data_type+sub_type+α) */
	if (kTabRecvDataSizeMin > data.size())
	{
		VHAL_LOGE("parameter error. data.size is %lu", data.size());
	}
	else if (kDatatypeCamera == data[data.size() - 1])
	{
		switch (data[kTabRecvDataOpc])
		{
			case SUB_TYPE_CAMERA_SYNC:
				/* 同期検知情報通知 */
				p_capture_tab_receiver_->SetCameraSync(data);
				break;
			case SUB_TYPE_CAMERA_TYPE:
				/* カメラ種別判別通知 */
				p_capture_tab_receiver_->SetCameraType(data);
				break;
			default:
				VHAL_LOGE("sub_type error. value=0x%02X", data[kTabRecvDataOpc]);
				break;
		}
	}
	else
	{
		/* 無処理 */
	}
}

/*****************************************************************************
 処理概要：	TABコマンド通知受信事前通知処理（通信モジュールスレッドからのコール）
 引数    ：	const std::vector<uint8_t>&	data	(i)	コマンド情報ポインタ
 戻り値  ：	なし
*****************************************************************************/
void CVhalCaptureTabReceiver::ReceivePreNotify(const std::vector<uint8_t>& data) noexcept
{
	/* 処理なし（Receiveで処理実施） */
}

/*****************************************************************************
 処理概要：	受信アイテム種別取得
 引数    ：	なし
 戻り値  ：	受信アイテム種別
           		RECEIVE_ITEM_TYPE_MISC
*****************************************************************************/
CVhalMiconReceiveItem::ReceiveItemType CVhalCaptureTabReceiver::GetItemType(void) const noexcept
{
	return CVhalMiconReceiveItem::ReceiveItemType::RECEIVE_ITEM_TYPE_MISC;
}

/*****************************************************************************
 処理概要：	イベントリスナー登録
 引数    ：	CVhalCaptureTabReceiveEventListenerBase* p_listener		(i)リスナーポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_SUCCESS			正常終了
****************************************************************************/
int32_t CVhalCaptureTabReceiver::RegisterEventListener(CVhalCaptureTabReceiveEventListenerBase* const p_listener)
{
	int32_t	result{VHAL_SUCCESS};

	if (nullptr == p_listener)
	{
		VHAL_LOGE("p_listener is null.");
		result = VHAL_ERR_PARAM;
	}
	else
	{
		p_capture_tab_recv_listener_ = p_listener;
	}

	return result;
}

/*****************************************************************************
 処理概要：	イベントリスナー解除
 引数    ：	なし
 戻り値  ：	なし
****************************************************************************/
void CVhalCaptureTabReceiver::ClearEventListener(void) noexcept
{
	p_capture_tab_recv_listener_ = nullptr;
}

/*****************************************************************************
 処理概要：	カメラ映像パス設定時の同期状態更新
 引数    ：	const VhalTabCamSync	state	(i)カメラ同期状態
 戻り値  ：	true	更新あり
           	false	更新なし
*****************************************************************************/
bool CVhalCaptureTabReceiver::UpdateCameraPathSync(const VhalTabCamSync state)
{
	bool ret{false};

	VHAL_LOGV_IN();

	if (camera_path_sync_status_ != state)
	{
		ret = true;
		camera_path_sync_status_ = state;
	}

	VHAL_LOGV_OUT();
	return ret;
}

/*****************************************************************************
 処理概要：	カメラ同期の取得
 引数    ：	なし
 戻り値  ：	カメラ映像入力同期(00h:同期無し/01h：同期有り/FFh：未確定・無効)
*****************************************************************************/
VhalTabCamSync CVhalCaptureTabReceiver::GetCameraSync(void) const noexcept
{
	return camera_sync_status_;
}

/*****************************************************************************
 処理概要：	表示用カメラ映像サイズの取得
 引数    ：	int32_t		disp	(i)表示カメラ種別
           	int32_t		&w		(o)表示用カメラ映像サイズ幅
           	int32_t		&h		(o)表示用カメラ映像サイズ高さ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータエラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalCaptureTabReceiver::GetCameraInputSize(const int32_t disp, int32_t &w, int32_t &h) const
{
	int32_t	ret{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	if (disp < 0)
	{
		VHAL_LOGE("disp is out of camera_size_. disp=%d", disp);
	}
	else
	{
		const std::size_t index{static_cast<std::size_t>(disp)};
		if (camera_size_.size() > index)
		{
			const auto itr_map = cam_size_entries_.find(static_cast<CameraSizeMode>(camera_size_[index]));
			if (itr_map == cam_size_entries_.end())
			{
				VHAL_LOGE("camera size [%d] is not supported.", camera_size_[index]);
				ret = VHAL_ERR_PARAM;
			}
			else
			{
				struct VhalCameraSizeData input{};
				input = itr_map->second;
				w = input.width;
				h = input.height;
			}
		}
		else
		{
			VHAL_LOGE("disp is out of camera_size_. camera_size_.size()=%lu disp=%d", camera_size_.size(), disp);
		}
	}

	VHAL_LOGV_OUT();
	return ret;
}

/*****************************************************************************
 処理概要：	カメラ映像表示要求状況(センターディスプレイ)の取得
 引数    ：	なし
 戻り値  ：	処理結果
           		true	[ON検知]
           		false	[ON検知]以外
*****************************************************************************/
bool CVhalCaptureTabReceiver::GetCameraCenterDisplayState(void) const
{
	bool ret{false};

	VHAL_LOGV_IN();

	if (kTabCamReqOn == center_display_status_)
	{
		ret = true;
	}

	VHAL_LOGV_OUT();
	return ret;
}

/*****************************************************************************
 処理概要：	カメラ同期通知
 引数    ：	const std::vector<uint8_t>& data	(i) 受信データ
 戻り値  ：	なし
*****************************************************************************/
void CVhalCaptureTabReceiver::SetCameraSync(const std::vector<uint8_t>& data) noexcept
{
	/* 同期状態更新 */
	uint8_t sync{data[kTabRecvDataCamSync]};
	switch (sync)
	{
		case static_cast<uint8_t>(VhalTabCamSync::kSyncNo):
		case static_cast<uint8_t>(VhalTabCamSync::kSync):
		case static_cast<uint8_t>(VhalTabCamSync::kUnknown):
			camera_sync_status_ = static_cast<VhalTabCamSync>(sync);
			break;
		default:
			VHAL_LOGE("Invalid params. sync=%u", sync);
			break;
	}

	/* V-IC-STATUS2信号のログ出力 */
	uint32_t data_length{SizeToUI32(data.size() - 1U)};		/* data_typeの分を除去 */
	if (kTabRecvDataVicStatus2 < data_length)
	{
		uint8_t vic_status{data[kTabRecvDataVicStatus2]};
		if (camera_vic_status_ != vic_status)
		{
			VHAL_LOGI("V-IC-STATUS2 changed. status=%u", vic_status);
			camera_vic_status_ = vic_status;
		}
	}

	/* 同期検知情報通知 */
	p_capture_tab_recv_listener_->NotifyTabReceiveCameraSync(camera_sync_status_);
}

/*****************************************************************************
 処理概要：	カメラ種別判別通知
 引数    ：	const std::vector<uint8_t>&	data	(i) 受信データ
 戻り値  ：	なし
*****************************************************************************/
void CVhalCaptureTabReceiver::SetCameraType(const std::vector<uint8_t>& data)
{
	VHAL_LOGV_IN();

	/* データ要素数は6個(OPC+α) */
	if (kTabRecvDataSizeCamType > data.size())
	{
		VHAL_LOGE("parameter error. data.size is %lu", data.size());
	}
	else
	{
		/* Suppress log output */
		if (prev_camera_type_ != data) {
			VHAL_LOGI("data.size is %lu opc=0x%x camType=0x%x cent_camSize=0x%x cent_dispReq=0x%x", data.size()-1,
				data[kTabRecvDataOpc], data[1], data[kTabRecvDataCenterCamSize],data[kTabRecvDataCenterCamReq]);

			/* Locally save received data */
			prev_camera_type_.assign(data.begin(), data.end());
		}

		if (camera_size_[VHAL_CAMERA_DISPLAY_CENTER] != data[kTabRecvDataCenterCamSize])
		{
			camera_size_[VHAL_CAMERA_DISPLAY_CENTER] = data[kTabRecvDataCenterCamSize];
			p_capture_tab_recv_listener_->NotifyTabSetCameraInputSize(VHAL_CAMERA_DISPLAY_CENTER);
		}

		center_display_status_ = data[kTabRecvDataCenterCamReq];
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalCaptureTabSendItem::CVhalCaptureTabSendItem(void)
	:opc_(0U)
{

}

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalCameraModeSendItem::CVhalCameraModeSendItem(void)
	:opc_(0U)
{

}

/*****************************************************************************
 処理概要：	TAB2送信データ構築
 引数    ：	std::vector<uint8_t> &send_data		(o)送信データ
 戻り値  ：	処理結果
           		VHAL_ERR				異常終了
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalCaptureTabSendItem::Build(std::vector<uint8_t> &send_data) const
{
	int32_t ret{VHAL_SUCCESS};
	
	if (false == data_.empty())
	{
		send_data.push_back(opc_);
		(void)send_data.insert(send_data.cend(), data_.begin(), data_.end());
	}
	else
	{
		ret = VHAL_ERR;
	}

	return ret;
}

/*****************************************************************************
 処理概要：	MISC 送信データ構築
 引数    ：	std::vector<uint8_t> &send_data		(o)送信データ
 戻り値  ：	処理結果
           		VHAL_ERR				異常終了
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalCameraModeSendItem::Build(std::vector<uint8_t> &send_data) const
{
	int32_t ret{VHAL_SUCCESS};
	
	if (false == data_.empty())
	{
		send_data.push_back(opc_);
		(void)send_data.insert(send_data.cend(), data_.begin(), data_.end());
	}
	else
	{
		ret = VHAL_ERR;
	}

	return ret;
}

/*****************************************************************************
 処理概要：	送信アイテム種別取得
 引数    ：	なし
 戻り値  ：	送信アイテム種別
*****************************************************************************/
CVhalMiconSendItem::SendItemType CVhalCaptureTabSendItem::GetItemType(void) const noexcept
{
	return CVhalMiconSendItem::SendItemType::SEND_ITEM_TYPE_MISC;
}

/*****************************************************************************
 処理概要：	送信アイテム種別取得
 引数    ：	なし
 戻り値  ：	送信アイテム種別
*****************************************************************************/
CVhalMiconSendItem::SendItemType CVhalCameraModeSendItem::GetItemType(void) const noexcept
{
	return CVhalMiconSendItem::SendItemType::SEND_ITEM_TYPE_MISC;
}

/*****************************************************************************
 処理概要：	カメラ種別判別通知要求
 引数    ：	なし
 戻り値  ：	送信アイテム種別
*****************************************************************************/
void CVhalCaptureTabSendItem::SetTabCmdCameraTypeNotify(void)
{
	/* OPC:カメラ種別判別要求 */
	SetTabCmdCode(static_cast<uint8_t>(SUB_TYPE_CAMERA_REQUEST));

	/* 接続通知の要求(data_typeを設定) */
	std::vector<uint8_t>	data{kDatatypeCamera};
	VHAL_LOGD("OPC:%02x DATA:%02x", opc_, data[0]);
	SetTabData(data);
}

/*****************************************************************************
 処理概要：	カメラ映像モード通知 データ設定
 引数    ：	const uint8_t mode  (i) カメラ映像モード
 戻り値  ：	なし
*****************************************************************************/
void CVhalCameraModeSendItem::SendCameraVisualMode(const uint8_t mode)
{
	SetMiscCmdCode(static_cast<uint8_t>(SUB_TYPE_CAMERA_MODE));
	std::vector<uint8_t> data{mode};
	data.push_back(kDatatypeCamera);
	VHAL_LOGD("OPC:%02x DATA:%02x MODE:%02x", opc_, data[0], data[1]);
	SetMiscData(data);
}

/*****************************************************************************
 処理概要：	OPC設定
 引数    ：	uint8_t	opc	(i)OPC
 戻り値  ：	なし
*****************************************************************************/
void CVhalCaptureTabSendItem::SetTabCmdCode(const uint8_t opc) noexcept
{
	opc_ = opc;
}

/*****************************************************************************
 処理概要：	OPC設定
 引数    ：	uint8_t	opc	(i)OPC
 戻り値  ：	なし
*****************************************************************************/
void CVhalCameraModeSendItem::SetMiscCmdCode(const uint8_t opc) noexcept
{
	opc_ = opc;
}

/*****************************************************************************
 処理概要：	送信データ設定
 引数    ：	std::vector<uint8_t>&	data	(i)送信データ
 戻り値  ：	なし
*****************************************************************************/
void CVhalCaptureTabSendItem::SetTabData(std::vector<uint8_t> &data)
{
	if (false == data.empty())
	{
		data_.assign(data.begin(), data.end());
	}

}

/*****************************************************************************
 処理概要：	送信データ設定
 引数    ：	std::vector<uint8_t>&	data	(i)送信データ
 戻り値  ：	なし
*****************************************************************************/
void CVhalCameraModeSendItem::SetMiscData(std::vector<uint8_t> &data)
{
	if (false == data.empty())
	{
		data_.assign(data.begin(), data.end());
	}
}

} /* namespace videohal */

