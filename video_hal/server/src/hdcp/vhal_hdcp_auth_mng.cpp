/*******************************************************************************
    機能名称    ：  HDCP認証モジュール
    ファイル名称：  vhal_hdcp_auth_mng.cpp
*******************************************************************************/
#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_hdcp_auth_mng.h"

namespace videohal
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalHdcpAuthManager::CVhalHdcpAuthManager(void)
{
	VHAL_LOGV("CVhalHdcpAuthManager is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalHdcpAuthManager::~CVhalHdcpAuthManager(void)
{
	VHAL_LOGV("CVhalHdcpAuthManager is deleted. this=%p", this);
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_****		異常終了
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalHdcpAuthManager::Initialize(void)
{
	int32_t	result{VHAL_SUCCESS};

	do
	{
		/* 認証初期化 */
		p_hdcp_auth_rse_ = std::make_unique<CVhalHdcpAuthControl>();
		if (nullptr == p_hdcp_auth_rse_)
		{
			VHAL_LOGE("Failed to create CVhalHdcpAuthControl.(RSE)");
			result = VHAL_ERR_MEMORY;
			break;
		}

		p_hdcp_auth_cdisp_ = std::make_unique<CVhalHdcpAuthControl>();
		if (nullptr == p_hdcp_auth_cdisp_)
		{
			VHAL_LOGE("Failed to create CVhalHdcpAuthControl.(CDISP)");
			result = VHAL_ERR_MEMORY;
			break;
		}

		result = p_hdcp_auth_rse_->Initialize(hdcpAuthType::HDCP_AUTH_TYPE_RSE);
		if (VHAL_SUCCESS != result)
		{
			VHAL_LOGE("CVhalHdcpAuthControl::Initialize error.(RSE) ret=%d", result);
			break;
		}

		result = p_hdcp_auth_cdisp_->Initialize(hdcpAuthType::HDCP_AUTH_TYPE_CDISP);
		if (VHAL_SUCCESS != result)
		{
			VHAL_LOGE("CVhalHdcpAuthControl::Initialize error.(CDISP) ret=%d", result);
			break;
		}

	} while (false);

	return result;
}

/*****************************************************************************
 処理概要：	終了処理（内部リソースの解放）
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalHdcpAuthManager::Finalize(void)
{
	StopAuthRse();
	p_hdcp_auth_cdisp_  = nullptr;
	p_hdcp_auth_rse_    = nullptr;
}

/*****************************************************************************
 処理概要：	HDCP認証開始(RSE)
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_****			異常終了
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalHdcpAuthManager::StartAuthRse(void)
{
	int32_t	result{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	if (nullptr == p_hdcp_auth_rse_)
	{
		VHAL_LOGE("Not started. p_hdcp_auth_rse_=%p", p_hdcp_auth_rse_.get());
		result = VHAL_ERR;
	}
	else
	{
		/* RSE認証開始 */
		result = p_hdcp_auth_rse_->StartAuth(rse_type_);
		if (VHAL_SUCCESS != result)
		{
			VHAL_LOGE("CVhalHdcpAuthControl::StartAuth(RSE) error. ret=%d", result);
		}
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	HDCP認証停止(RSE)
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalHdcpAuthManager::StopAuthRse(void)
{
	VHAL_LOGV_IN();

	if (nullptr == p_hdcp_auth_rse_)
	{
		VHAL_LOGW("Not started. p_hdcp_auth_rse_=%p", p_hdcp_auth_rse_.get());
	}
	else
	{
		/* 認証停止処理 */
		const int32_t ret{p_hdcp_auth_rse_->StopAuth()};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("CVhalHdcpAuthControl::StopAuth(RSE) error. ret=%d", ret);
		}

	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	RSE種別設定
 引数    ：	const int32_t	type	(i)RSE種別
 戻り値  ：	なし
*****************************************************************************/
void CVhalHdcpAuthManager::SetConnectedRse(const int32_t type) noexcept
{
	rse_type_ = type;
	if (nullptr != p_hdcp_auth_rse_)
	{
		p_hdcp_auth_rse_->SetConnectedDevice(type);
	}
}

/*****************************************************************************
 処理概要：	HDCP認証状態初期値取得
 引数    ：	なし
 戻り値  ：	HDCP認証状態初期値
****************************************************************************/
int32_t CVhalHdcpAuthManager::GetHdcpInitStatus(void) const noexcept
{
	return hdcp_init_status_;
}

/*****************************************************************************
 処理概要：	HDCP状態監視クラス取得
 引数    ： hdcpAuthType			auth_type	HDCP認証対象種別
 戻り値  ：	CVhalHdcpAuthControlクラス
*****************************************************************************/
CVhalHdcpAuthControl* CVhalHdcpAuthManager::GetHdcpAuthControl(const hdcpAuthType auth_type) const noexcept
{
	CVhalHdcpAuthControl*	p_hdcp_auth{nullptr};
	if ((nullptr != p_hdcp_auth_rse_) && (hdcpAuthType::HDCP_AUTH_TYPE_RSE == auth_type))
	{
		p_hdcp_auth = p_hdcp_auth_rse_.get();
	}
	else if ((nullptr != p_hdcp_auth_cdisp_) && (hdcpAuthType::HDCP_AUTH_TYPE_CDISP == auth_type))
	{
		p_hdcp_auth = p_hdcp_auth_cdisp_.get();
	}
	else
	{
		/* 実装ミス以外ここに来ることはないはず */
		VHAL_LOGE("param-error. auth_type=%d p_hdcp_auth_rse_=%p p_hdcp_auth_cdisp_=%p", auth_type, p_hdcp_auth_rse_.get(), p_hdcp_auth_cdisp_.get());
	}
	return p_hdcp_auth;
}

/*****************************************************************************
 処理概要：	HDCP認証キー取得 
 引数    ： hdcpAuthType			auth_type	HDCP認証対象種別
			CVhalHdcpAuthRsltData&	hdcp_key	(o)HDCP認証キー
 戻り値  ：	処理結果
           		VHAL_ERR				異常終了
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalHdcpAuthManager::GetHdcpAuthKey(const hdcpAuthType auth_type, CVhalHdcpAuthRsltData& hdcp_key) const noexcept
{
	int32_t		result{VHAL_ERR};
	CVhalHdcpAuthControl* p_hdcp_auth{GetHdcpAuthControl(auth_type)};
	if (nullptr != p_hdcp_auth)
	{
		p_hdcp_auth->GetHdcpAuthKey(hdcp_key);
		result = VHAL_SUCCESS;
	}
	return result;
}

/*****************************************************************************
 処理概要：	HDCP認証キー更新
 引数    ： hdcpAuthType			auth_type	HDCP認証対象種別
			CVhalHdcpAuthRsltData&	hdcp_key	HDCP認証キー
 戻り値  ：	処理結果
           		VHAL_ERR		C-Disp_HDCP認証キーに変化なし または 認証キー異常の場合
           		VHAL_SUCCESS	C-Disp_HDCP認証キーに変化あり または C-Disp_HDCP認証失敗時の場合
*****************************************************************************/
int32_t CVhalHdcpAuthManager::UpdateHdcpAuthResult(const hdcpAuthType auth_type, const CVhalHdcpAuthRsltData& hdcp_key) noexcept
{
	int32_t		result{VHAL_ERR};
	CVhalHdcpAuthControl* p_hdcp_auth{GetHdcpAuthControl(auth_type)};
	if (nullptr != p_hdcp_auth)
	{
		result = p_hdcp_auth->UpdateHdcpAuthResult(hdcp_key);
	}
	return result;
}

/*****************************************************************************
 処理概要：	HDCP認証キークリア
 引数    ： なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalHdcpAuthManager::ClearHdcpAuthKey(void) noexcept
{
	if (nullptr != p_hdcp_auth_rse_)
	{
		p_hdcp_auth_rse_->ClearHdcpAuthKey();
	}
	if (nullptr != p_hdcp_auth_cdisp_)
	{
		p_hdcp_auth_cdisp_->ClearHdcpAuthKey();
	}
	VHAL_LOGI("HdcpAuthKey was cleared.");
}

} /* namespace videohal */

