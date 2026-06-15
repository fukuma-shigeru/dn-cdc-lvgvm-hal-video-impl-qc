/*******************************************************************************
    機能名称    ：  HDCP認証制御モジュール
    ファイル名称：  vhal_hdcp_auth_control.cpp
*******************************************************************************/
#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_hdcp_auth_control.h"
#include "vhal_main_control.h"
#include "vhal_debug_system.h"
#include "vhal_round_cast.h"
#include "vhal_backup_control.h"
#include <unordered_map>

namespace videohal
{

namespace
{
/* HDCP認証キー対象のデータ識別文字列 */
const std::string		VHAL_HDCP_CDISP_BACKUP_NAME				{"/tier1_video_hal/hdcp_auth_key_cdisp"};
const std::string		VHAL_HDCP_RSE_BACKUP_NAME				{"/tier1_video_hal/hdcp_auth_key_rse"};
/* HDCP認証キーバックアップサイズ */
constexpr std::size_t	VHAL_HDCP_BACKUP_SIZE					{sizeof(CVhalHdcpAuthSerialize)};
/* HDCP認証キー対象テーブル */
const std::unordered_map<const hdcpAuthType, const std::string&>	kHdcpKeyTable{
		{ hdcpAuthType::HDCP_AUTH_TYPE_CDISP	, VHAL_HDCP_CDISP_BACKUP_NAME	},
		{ hdcpAuthType::HDCP_AUTH_TYPE_RSE		, VHAL_HDCP_RSE_BACKUP_NAME		},
	};
}

/*****************************************************************************
 処理概要：	HDCP認証鍵を設定
 引数    ：	std::vector<uint8_t>& receiver_id			バイト化されたHDCPレシーバーID(5バイト)
 戻り値  ：	なし
*****************************************************************************/
void CVhalHdcpAuthRsltData::AddReceiverIds(const std::vector<uint8_t>& receiver_id) noexcept
{
	VHAL_LOGV_IN();
	if (khdcpReceiverIdSize <= receiver_id.size())
	{
		uint64_t	o_receiver_id{0ULL};
		for (uint32_t i{0U}; i < khdcpReceiverIdSize; ++i)
		{
			o_receiver_id += (static_cast<uint64_t>(receiver_id[i]) << (khdcpByteToBit * i));
		}
		receiver_ids_.push_back(o_receiver_id);
	}
	else
	{
		VHAL_LOGE("parameter-size error. receiver_id.size=%llu", receiver_id.size());
	}
	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	HDCP認証鍵を変換
 引数    ：	const uint64_t	i_receiver_id				(i)HDCPレシーバーID 0~31
           	std::vector<uint8_t>&	o_receiver_id		(o)HDCPレシーバーID(5バイト分)
 戻り値  ：	なし
*****************************************************************************/
void CVhalHdcpAuthRsltData::ConvReceiverIds(const uint64_t i_receiver_id, std::vector<uint8_t>& o_receiver_id) const noexcept
{
	VHAL_LOGV_IN();
	constexpr uint64_t	byte_mask{0xFFU};
	o_receiver_id.resize(khdcpReceiverIdSize);
	for (uint32_t i{0U}; i < khdcpReceiverIdSize; ++i)
	{
		o_receiver_id[i] = ToUI8((i_receiver_id >> (khdcpByteToBit * i)) & byte_mask);
	}
	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	シリアライズ
 引数    ：	std::vector<uint8_t>& serial_data	(o)シリアライズされたHDCP認証キー
 戻り値  ：	なし
*****************************************************************************/
void CVhalHdcpAuthRsltData::Serialize(std::vector<uint8_t>& serial_data) const noexcept
{
	VHAL_LOGV_IN();

	/* いったん全削除してから領域再確保する(ゴミデータが残らないように) */
	serial_data.clear();
	serial_data.resize(sizeof(CVhalHdcpAuthSerialize));

	/* 各値設定 */
	CVhalHdcpAuthSerialize* p_auth_key{reinterpret_cast<CVhalHdcpAuthSerialize*>(serial_data.data())};
	if (nullptr != p_auth_key)
	{
		/* receiver_ids_の格納上限は32とする */
		uint8_t device_count_max{GetDevsCount()};
		if (device_count_max > kMaxDeviceCount)
		{
			device_count_max = kMaxDeviceCount;	/* 最大32件 */
		}
		if (device_count_max > GetReceiverIds().size())
		{
			device_count_max = ToUI8(GetReceiverIds().size());
		}
		for (uint8_t i{0U}; i < device_count_max; ++i)
		{
			std::vector<uint8_t> receiver_id{};
			ConvReceiverIds(GetReceiverIds()[i], receiver_id);
			p_auth_key->SetReceiverId(i, receiver_id);	
		}
		p_auth_key->SetSubType(GetSubType());
		p_auth_key->SetMaxDevs(GetMaxDevs());
		p_auth_key->SetDevsCount(device_count_max);
		p_auth_key->SetMaxCascade(GetMaxCascade());
		p_auth_key->SetCascadeDepth(GetCascadeDepth());
	}
	else
	{
		VHAL_LOGE("memory resource error. p_auth_key=%p", p_auth_key);
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	デシリアライズ
 引数    ：	std::vector<uint8_t>& serial_data	(i)シリアライズされたHDCP認証キー
 戻り値  ：	なし
*****************************************************************************/
void CVhalHdcpAuthRsltData::Deserialize(const std::vector<uint8_t>& serial_data) noexcept
{
	VHAL_LOGV_IN();

	/* 全メンバクリア */
	Clear();

	/* 各値設定 */
	const CVhalHdcpAuthSerialize* p_auth_key{reinterpret_cast<const CVhalHdcpAuthSerialize*>(serial_data.data())};
	const std::size_t length{serial_data.size()};
	if ((nullptr != p_auth_key) && (sizeof(CVhalHdcpAuthSerialize) <= length))
	{
		if (p_auth_key->GetSubType() == p_auth_key->GetSubTypeEnd())
		{
			SetSubType(p_auth_key->GetSubType());
		}
		SetMaxDevs(p_auth_key->GetMaxDevs());
		SetMaxCascade(p_auth_key->GetMaxCascade());
		SetCascadeDepth(p_auth_key->GetCascadeDepth());

		/* receiver_ids_の格納上限は32とする */
		uint8_t device_count_max{p_auth_key->GetDevsCount()};
		if (device_count_max > kMaxDeviceCount)
		{
			device_count_max = kMaxDeviceCount;	/* 最大32件 */
		}
		SetDevsCount(device_count_max);
		for (uint8_t i{0U}; i < device_count_max; ++i)
		{
			std::vector<uint8_t> receiver_id{};
			p_auth_key->GetReceiverId(i, receiver_id);
			AddReceiverIds(receiver_id);
		}
	}
	else
	{
		VHAL_LOGE("memory resource error. p_auth_key=%p length=%llu", p_auth_key, length);
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	HDCP認証キー情報クリア
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalHdcpAuthRsltData::Clear(void) noexcept
{
	VHAL_LOGV_IN();

	SetResult(VHAL_HDCP_FIRST_AUTH_STS_NONE);
	SetSubType(0U);
	SetMaxDevs(0U);
	SetDevsCount(0U);
	SetMaxCascade(0U);
	SetCascadeDepth(0U);
	receiver_ids_.clear();

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	HDCP認証キー比較
 引数    ：	CVhalHdcpAuthRsltData& hdcp_key 比較対象HDCP認証キー
 戻り値  ：	判定結果
				true		同一
				false		同一でない
*****************************************************************************/
bool CVhalHdcpAuthRsltData::Equal(const CVhalHdcpAuthRsltData& hdcp_key) const noexcept
{
	VHAL_LOGV_IN();
	bool	result{true};

	/* 変更があるかチェック */
	if ((GetResult()		!= hdcp_key.GetResult())		||
		(GetSubType()		!= hdcp_key.GetSubType())		||
		(GetMaxDevs()		!= hdcp_key.GetMaxDevs())		||
		(GetDevsCount()		!= hdcp_key.GetDevsCount())		||
		(GetMaxCascade()	!= hdcp_key.GetMaxCascade())	||
		(GetCascadeDepth()	!= hdcp_key.GetCascadeDepth())	||
		(GetReceiverIds()	!= hdcp_key.GetReceiverIds()))
	{
		result = false;
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalHdcpAuthControl::CVhalHdcpAuthControl(void)
{
	VHAL_LOGV("CVhalHdcpAuthControl is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalHdcpAuthControl::~CVhalHdcpAuthControl(void)
{
	VHAL_LOGV("CVhalHdcpAuthControl is deleted. this=%p", this);
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ： hdcpAuthType			auth_type	HDCP認証対象種別
 戻り値  ：	処理結果
           		VHAL_ERR_****		異常終了
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalHdcpAuthControl::Initialize(const hdcpAuthType auth_type)
{
	int32_t	result{VHAL_SUCCESS};

	auth_type_          = auth_type;
	dev_type_           = VHAL_CONNECTED_RSE_INVALID;	/* 接続機器未設定状態 */

	p_backup_control_ = std::make_unique<CVhalBackupControl>();
	if (nullptr == p_backup_control_)
	{
		VHAL_LOGE("Failed to create CVhalBackupControl.");
		result = VHAL_ERR_MEMORY;
	}
	else
	{
		/* 不揮発からHDCP認証キーの取得 */
		LoadHdcpAuthKey(hdcp_result_);
	}

	return result;
}

/*****************************************************************************
 処理概要：	終了処理（内部リソースの解放）
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalHdcpAuthControl::Finalize(void)
{
	p_backup_control_ = nullptr;
}

/*****************************************************************************
 処理概要：	HDCP認証開始
 引数    ：	const int32_t	type	(i)接続機器種別
 戻り値  ：	処理結果
           		VHAL_ERR_****			エラー
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalHdcpAuthControl::StartAuth(const int32_t type)
{
	int32_t	result{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	/* 接続機器種別更新 */
	dev_type_ = type;

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	HDCP認証停止
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalHdcpAuthControl::StopAuth(void)
{
	VHAL_LOGV_IN();

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	接続機器種別設定
 引数    ：	const int32_t	type	(i)接続機器種別
 戻り値  ：	なし
*****************************************************************************/
void CVhalHdcpAuthControl::SetConnectedDevice(const int32_t type) noexcept
{
	dev_type_ = type;
}

/*****************************************************************************
 処理概要：	接続機器状態判定
 引数    ：	なし
 戻り値  ：	処理結果
           		true			機器接続状態
           		false			機器接続無し/未設定状態
*****************************************************************************/
bool CVhalHdcpAuthControl::CheckConnectedDevice(void) const
{
	bool state{false};

	VHAL_LOGV_IN();

	/* 接続機器がRSE */
	if ((VHAL_CONNECTED_RSE_DOP == dev_type_) || (VHAL_CONNECTED_RSE_FULL == dev_type_))
	{
		state = true;
	}

	VHAL_LOGV_OUT();
	return state;
}

/*****************************************************************************
 処理概要：	HDCP認証キー正当性チェック
 引数    ： CVhalHdcpAuthRsltData&	hdcp_key	HDCP認証キー
 戻り値  ：	処理結果
           		VHAL_ERR				異常終了
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalHdcpAuthControl::CheckHdcpAuthKey(const CVhalHdcpAuthRsltData& hdcp_key) const noexcept
{
	int32_t		result{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	/* sub_type */
	const uint8_t		sub_type = hdcp_key.GetSubType();
	if (((hdcpAuthType::HDCP_AUTH_TYPE_CDISP == auth_type_) && (SUB_TYPE_HDCP_AUTH_NTY_CDISP == sub_type))
	 || ((hdcpAuthType::HDCP_AUTH_TYPE_RSE == auth_type_) && (SUB_TYPE_HDCP_AUTH_NTY_RSE == sub_type)))
	{
		/* 問題なしの組み合わせ */
	}
	else
	{
		VHAL_LOGW("sub_type failed... auth_type=%d sub_type=%u", auth_type_, sub_type);
		result = VHAL_ERR;
	}

	/* HDCP認証結果 */
	const int32_t		key_result{hdcp_key.GetResult()};
	if ((VHAL_SUCCESS == result) &&
		(VHAL_HDCP_FIRST_AUTH_STS_SUCCESS != key_result) &&
		(VHAL_HDCP_FIRST_AUTH_STS_FAILED  != key_result))
	{
		VHAL_LOGE("key_result failed... key_result=%d", key_result);
		result = VHAL_ERR;
	}

	/* HDCPデバイスオーバー 0~1 */
	if ((VHAL_SUCCESS == result) && (hdcp_key.kMaxDevsExceed < hdcp_key.GetMaxDevs()))
	{
		VHAL_LOGE("GetMaxDevs failed... max_devs_exceeded=%u", hdcp_key.GetMaxDevs());
		result = VHAL_ERR;
	}

	/* HDCPデバイス数 0~32 */
	if ((VHAL_SUCCESS == result) && (hdcp_key.kMaxDeviceCount < hdcp_key.GetDevsCount()))
	{
		VHAL_LOGE("GetDevsCount failed... device_count=%u", hdcp_key.GetDevsCount());
		result = VHAL_ERR;
	}

	/* HDCP接続段数オーバー 0~1 */
	if ((VHAL_SUCCESS == result) && (hdcp_key.kMaxCascadeExceed < hdcp_key.GetMaxCascade()))
	{
		VHAL_LOGE("GetMaxCascade failed... max_cascade_exceeded=%u", hdcp_key.GetMaxCascade());
		result = VHAL_ERR;
	}

	/* HDCP接続段数 0~4 */
	if ((VHAL_SUCCESS == result) && (hdcp_key.kMaxDepth < hdcp_key.GetCascadeDepth()))
	{
		VHAL_LOGE("GetCascadeDepth failed... depth=%u", hdcp_key.GetCascadeDepth());
		result = VHAL_ERR;
	}

	/* HDCPレシーバーID 0~32 (HDCPデバイス数と等しいこと) */
	if ((VHAL_SUCCESS == result) && (hdcp_key.GetDevsCount() != hdcp_key.SizeReceiverIds()))
	{
		VHAL_LOGE("GetDevsCount and SizeReceiverIds not equal... device_count=%u SizeReceiverIds=%d", hdcp_key.GetDevsCount(), hdcp_key.SizeReceiverIds());
		result = VHAL_ERR;
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	HDCP認証キー読込
 引数    ： CVhalHdcpAuthRsltData&	hdcp_key	HDCP認証キー
 戻り値  ：	なし
 フェールセーフNo  ：	F-VHAL-N-328
                    	F-VHAL-R-329
*****************************************************************************/
void CVhalHdcpAuthControl::LoadHdcpAuthKey(CVhalHdcpAuthRsltData& hdcp_key) const noexcept
{
	VHAL_LOGV_IN();
	int32_t		result{VHAL_SUCCESS};
	std::vector<uint8_t>	serial_data(VHAL_HDCP_BACKUP_SIZE);

	auto iter = kHdcpKeyTable.find(auth_type_);
	if (iter != kHdcpKeyTable.end())
	{
		if (nullptr != p_backup_control_)
		{
			result = p_backup_control_->Load(iter->second, serial_data);
			if (VHAL_SUCCESS != result)
			{
				VHAL_LOGE("Load() failed. result=%d.", result);
			}
		}

//#ifdef VHAL_SUPPORT_FAIL_SYSTEM	// F-VHAL-N-328, F-VHAL-N-329
//		constexpr int32_t	notsetting{-1};
//		int32_t value[2]{notsetting,notsetting};
//		CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-328",value[0]);
//		if (notsetting != value[0])
//		{
//			serial_data[0] = 0U;		/* 認証結果失敗 */
//			hdcp_key.SetResult(VHAL_HDCP_FIRST_AUTH_STS_FAILED);
//			VHAL_LOGW("fail F-VHAL-N-328");
//		}
//		CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-329",value[1]);
//		if (notsetting != value[1])
//		{
//			serial_data[3] = 2U;		/* 認証キー異常 */
//			VHAL_LOGW("fail F-VHAL-N-329");
//		}
//#endif

		if (VHAL_SUCCESS == result)
		{
			/* HDCP認証キークラスに復元 */
			hdcp_key.Deserialize(serial_data);

			uint8_t		sub_type{hdcp_key.GetSubType()};
			if (((hdcpAuthType::HDCP_AUTH_TYPE_CDISP == auth_type_) && (SUB_TYPE_HDCP_AUTH_NTY_CDISP == sub_type)) ||
				((hdcpAuthType::HDCP_AUTH_TYPE_RSE == auth_type_) && (SUB_TYPE_HDCP_AUTH_NTY_RSE == sub_type)))
			{
				/* 問題なし */
				hdcp_key.SetResult(VHAL_HDCP_FIRST_AUTH_STS_SUCCESS);
			}

			/* HDCP認証キー正当性チェック */
			result = CheckHdcpAuthKey(hdcp_key);
			if (VHAL_SUCCESS != result)
			{
				VHAL_LOGI("Invalid HDCPAuthKey... result=%d.", result);
			}
		}
	}
	else
	{
		/* パラメータエラー */
		VHAL_LOGE("Param_error auth_type=%d.", static_cast<int32_t>(auth_type_));
		result = VHAL_ERR;
	}

	/* 失敗時はHDCP認証キーをクリア */
	if (VHAL_SUCCESS != result)
	{
		VHAL_LOGI("HDCP key validation failed. Clearing key.");
		hdcp_key.Clear();
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	HDCP認証キー保存
 引数    ： CVhalHdcpAuthRsltData&	hdcp_key	HDCP認証キー
 戻り値  ：	なし
*****************************************************************************/
void CVhalHdcpAuthControl::SaveHdcpAuthKey(const CVhalHdcpAuthRsltData& hdcp_key) const noexcept
{
	VHAL_LOGV_IN();
	int32_t		result{VHAL_SUCCESS};
	std::vector<uint8_t>	serial_data{};

	auto iter = kHdcpKeyTable.find(auth_type_);
	if (iter != kHdcpKeyTable.end())
	{
		/* HDCP認証キークラスをシリアル化 */
		hdcp_key.Serialize(serial_data);

		if (nullptr != p_backup_control_)
		{
			result = p_backup_control_->Save(iter->second, serial_data, false);
			if (VHAL_SUCCESS != result)
			{
				VHAL_LOGE("Save() failed. result=%d.", result);
			}
		}
	}
	else
	{
		/* パラメータエラー */
		VHAL_LOGE("Param_error auth_type=%d.", static_cast<int32_t>(auth_type_));
	}

	VHAL_LOGV_OUT();
}


/*****************************************************************************
 処理概要：	HDCP認証キー取得 
 引数    ： CVhalHdcpAuthRsltData&	hdcp_key	(o)HDCP認証キー
 戻り値  ： なし
*****************************************************************************/
void CVhalHdcpAuthControl::GetHdcpAuthKey(CVhalHdcpAuthRsltData& hdcp_key) const noexcept
{
	hdcp_key = hdcp_result_;
}

/*****************************************************************************
 処理概要：	HDCP認証キー更新
 引数    ： CVhalHdcpAuthRsltData&	hdcp_key	HDCP認証キー
 戻り値  ：	処理結果
           		VHAL_ERR		C-Disp_HDCP認証キーに変化なし または 認証キー異常の場合
           		VHAL_SUCCESS	C-Disp_HDCP認証キーに変化あり または C-Disp_HDCP認証失敗時の場合
*****************************************************************************/
int32_t CVhalHdcpAuthControl::UpdateHdcpAuthResult(const CVhalHdcpAuthRsltData& hdcp_key) noexcept
{
	int32_t		result{VHAL_ERR};

	VHAL_LOGV_IN();

	/* HDCP認証キー正当性チェック */
	int32_t		ret{CheckHdcpAuthKey(hdcp_key)};
	if (VHAL_SUCCESS == ret)
	{
		/* HDCP認証キー変更チェック */
		bool is_equal{hdcp_result_.Equal(hdcp_key)};

		/* 認証成功以外(=認証失敗)はHDCP認証キーをクリアする */
		if (VHAL_HDCP_FIRST_AUTH_STS_SUCCESS != hdcp_key.GetResult())
		{
			VHAL_LOGI("hdcp_key is Auth-Failed.");

			/* HDCP認証キークリア */
			ClearHdcpAuthKey();

			/* 戻り値設定 */
			result = VHAL_SUCCESS;
		}
		/* HDCP認証キーに変更あり */
		else if (!is_equal)
		{
			/* HDCP認証キーの上書き */
			hdcp_result_ = hdcp_key;

			/* HDCP認証キー保存 */
			SaveHdcpAuthKey(hdcp_key);

			/* 戻り値設定 */
			result = VHAL_SUCCESS;
		}
		/* HDCP認証キーに変更なし */
		else
		{
			VHAL_LOGD("Same HDCPAuthKey.");
		}
	}
	else
	{
		/* 異常な場合、CheckHdcpAuthKey()にてログ出力するためここでは何もしない */
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	HDCP認証キークリア
 引数    ： なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalHdcpAuthControl::ClearHdcpAuthKey(void) noexcept
{
	VHAL_LOGV_IN();
	
	/* 各HDCP認証キーをクリア */
	hdcp_result_.Clear();

	/* クリアしたHDCP認証キーを不揮発に保存 */
	SaveHdcpAuthKey(hdcp_result_);

	VHAL_LOGV_OUT();
}


} /* namespace videohal */

