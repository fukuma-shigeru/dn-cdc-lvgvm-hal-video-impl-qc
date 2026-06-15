/*******************************************************************************
    機能名称    ：  カメラ制御モジュール
    ファイル名称：  vhal_capture_camera.cpp
*******************************************************************************/
#include "vhal_capture_camera.h"

#include "vhal_define.h"
#include "vhal_log.h"

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

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalCaptureCamera::CVhalCaptureCamera(void)
{
 	VHAL_LOGV("CVhalCaptureCamera is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalCaptureCamera::~CVhalCaptureCamera(void)
{
	VHAL_LOGV("CVhalCaptureCamera is deleted. this=%p", this);
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ：	CVhalLayoutManager*		p_layout_mng		(i)レイアウト制御インスタンスポインタ
           	CVhalCaptureControl*	p_capture_control	(i)キャプチャコントロールインスタンスポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_****		エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalCaptureCamera::Initialize(CVhalLayoutManager * const p_layout_mng, CVhalCaptureControl * const p_capture_control)
{
	int32_t	ret{VHAL_SUCCESS};

	do
	{
		if ((nullptr == p_layout_mng) || (nullptr == p_capture_control))
		{
			VHAL_LOGE("parameter error. p_layout_mng=%p, p_capture_control=%p", p_layout_mng, p_capture_control);
			ret = VHAL_ERR_PARAM;
			break;
		}

		p_capture_control_ = p_capture_control;
		input_type_ = captureInputType::VHAL_CAPTURE_INPUT_CAMERA;

		/* SUPPORT_DUMMY_CAMERA */
		const char*	p_debug{nullptr};
		p_debug = secure_getenv("DUMMY_CAMERA");
		if (nullptr != p_debug)
		{
			const std::string s_debug{p_debug};
			const std::string::size_type pos{s_debug.find("1")};
			if (std::string::npos != pos)
			{
				input_type_ = captureInputType::VHAL_CAPTURE_INPUT_CAMERA_DUMMY;
			}
		}

		/* キャプチャ設定 */
		ret = p_capture_control_->OpenCapture(p_layout_mng, input_type_);
	} while (false);

	return ret;
}

/*****************************************************************************
 処理概要：	終了処理（内部リソースの解放）
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalCaptureCamera::Finalize(void)
{
	if (nullptr != p_capture_control_)
	{
		/* 停止以外 */
		if (running_)
		{
			int32_t	ret{VHAL_SUCCESS};
			ret = p_capture_control_->Stop(input_type_);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("stop error. ret=%d", ret);
			}
			running_ = false;	/* 停止 */
		}
		/* キャプチャ設定解除 */
		p_capture_control_->CloseCapture(input_type_);
	}
}

/*****************************************************************************
 処理概要：	カメラpathの設定
 引数    ：	const std::string&	path	(i)カメラpath文字列
 戻り値  ：	処理結果
           		VHAL_ERR_****		エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalCaptureCamera::SetPath(const std::string& path)
{
	int32_t	ret{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	do
	{
		VHAL_LOGI("camera path [%s]", path.c_str());

		if (nullptr == p_capture_control_)
		{
			VHAL_LOGE("not SettingConnectedCamera.");
			ret = VHAL_ERR_NOT_INITIALIZED;
			break;
		}

		/* キャプチャ停止 */
		if (running_)
		{
			(void)p_capture_control_->Stop(input_type_);
			running_ = false;	/* 停止 */
		}

		if (!path.empty())
		{
			/* キャプチャ開始 */
			if (!running_)
			{
				/* キャプチャ開始(カメラ出力先指定) */
				ret = p_capture_control_->Start(input_type_);
				if (VHAL_SUCCESS == ret)
				{
					running_ = true;	/* 開始 */
				}
			}
		}
	} while (false);

	VHAL_LOGV_OUT();
	return ret;
}

/*****************************************************************************
 処理概要：	カメラキャプチャ開始
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_****		エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalCaptureCamera::Start(void) const
{
	VHAL_LOGV_IN();

	int32_t	ret{VHAL_SUCCESS};

	if (nullptr == p_capture_control_)
	{
		VHAL_LOGE("not SettingConnectedCamera.");
		ret = VHAL_ERR_NOT_INITIALIZED;
	}
	else
	{
		ret = p_capture_control_->StartCapture(input_type_);
	}

	VHAL_LOGV_OUT();
	return ret;
}

/*****************************************************************************
 処理概要：	カメラキャプチャ停止
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_****		エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalCaptureCamera::Stop(void) const
{
	VHAL_LOGV_IN();

	int32_t	ret{VHAL_SUCCESS};

	if (nullptr == p_capture_control_)
	{
		VHAL_LOGE("not SettingConnectedCamera.");
		ret = VHAL_ERR_NOT_INITIALIZED;
	}
	else
	{
		ret = p_capture_control_->StopCapture(input_type_);
	}

	VHAL_LOGV_OUT();
	return ret;
}

/*****************************************************************************
 処理概要：	カメラキャプチャ動作取得
 引数    ：	なし
 戻り値  ：	キャプチャ状態
           		true		キャプチャ動作中
           		false		キャプチャ停止中
*****************************************************************************/
bool CVhalCaptureCamera::IsCapture(void) const
{
	VHAL_LOGV_IN();

	bool	ret{false};

	if (nullptr == p_capture_control_)
	{
		VHAL_LOGE("not SettingConnectedCamera.");
	}
	else
	{
		ret = p_capture_control_->IsCaptureStatus(input_type_);
	}

	VHAL_LOGV_OUT();
	return ret;
}

} /* namespace videohal */

