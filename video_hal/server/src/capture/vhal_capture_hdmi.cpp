/*******************************************************************************
    機能名称    ：  HDMI制御モジュール
    ファイル名称：  vhal_capture_hdmi.cpp
*******************************************************************************/
#include "vhal_capture_hdmi.h"

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
CVhalCaptureHdmi::CVhalCaptureHdmi(void)
{
	VHAL_LOGV("CVhalCaptureHdmi is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalCaptureHdmi::~CVhalCaptureHdmi(void)
{
	VHAL_LOGV("CVhalCaptureHdmi is deleted. this=%p", this);
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ：	CVhalLayoutManager*		p_layout_mng		(i)レイアウト制御インスタンスポインタ
           	CVhalCaptureControl*	p_capture_control	(i)キャプチャコントロールインスタンスポインタ
           	uint32_t 				width				(i)解像度幅　（指定なしであれば'0'）
           	uint32_t 				height				(i)解像度高さ（指定なしであれば'0'）
 戻り値  ：	処理結果
           		VHAL_ERR_****		エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalCaptureHdmi::Initialize(CVhalLayoutManager * const p_layout_mng, CVhalCaptureControl * const p_capture_control, const uint32_t width, const uint32_t height)
{
	if ((nullptr == p_layout_mng) || (nullptr == p_capture_control))
	{
		VHAL_LOGE("parameter error. p_layout_mng=%p, p_capture_control=%p", p_layout_mng, p_capture_control);
		return VHAL_ERR_PARAM;
	}

	int32_t ret{VHAL_SUCCESS};

	if (!running_)
	{
		p_capture_control_ = p_capture_control;

		/* キャプチャ設定 */
		ret = p_capture_control_->OpenCapture(p_layout_mng, captureInputType::VHAL_CAPTURE_INPUT_HDMI, width, height);
		if (VHAL_SUCCESS != ret)
		{
			return ret;
		}

		/* キャプチャ開始 */
		(void)p_capture_control_->Start(captureInputType::VHAL_CAPTURE_INPUT_HDMI);
		running_ = true;	/* 開始 */
	}

	return ret;
}

/*****************************************************************************
 処理概要：	終了処理（内部リソースの解放）
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalCaptureHdmi::Finalize(void)
{
	if (nullptr != p_capture_control_)
	{
		/* 停止以外 */
		if (running_)
		{
			const int32_t ret{p_capture_control_->Stop(captureInputType::VHAL_CAPTURE_INPUT_HDMI)};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("stop error. ret=%d", ret);
			}
			running_ = false;	/* 停止 */
		}
		/* キャプチャ設定解除 */
		p_capture_control_->CloseCapture(captureInputType::VHAL_CAPTURE_INPUT_HDMI);
	}
}

} /* namespace videohal */

