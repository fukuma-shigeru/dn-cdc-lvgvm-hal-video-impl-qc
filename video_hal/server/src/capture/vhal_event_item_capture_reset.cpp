/*******************************************************************************
    機能名称    ：  キャプチャ再初期化イベントモジュール
    ファイル名称：  vhal_event_item_capture_reset.cpp
*******************************************************************************/
#include "vhal_event_item_capture_reset.h"

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_property_control.h"

namespace videohal
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalEventItemCaptureReset::CVhalEventItemCaptureReset(void)
{
	VHAL_LOGV("CVhalEventItemCaptureReset is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalEventItemCaptureReset::~CVhalEventItemCaptureReset(void)
{
	VHAL_LOGV("CVhalEventItemCaptureReset is deleted. this=%p", this);
}

/*****************************************************************************
 処理概要：	イベント処理の実施
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalEventItemCaptureReset::Exec(void) const
{
	VHAL_LOGD("CVhalEventItemCaptureReset::Exec called.");

	if ( nullptr != p_prop_ )
	{
		p_prop_->ResetCapture(input_type_);
	}
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	データ設定
 引数    ：	CVhalPropertyControl* p_prop_ctl			(i) プロパティ制御インスタンスポインタ
           	captureInputType      input_capture_type	(i) デバイス種別
 戻り値  ：	なし
*****************************************************************************/
void CVhalEventItemCaptureReset::SetData(CVhalPropertyControl* const p_prop_ctl, const captureInputType input_capture_type) noexcept
{
	p_prop_ = p_prop_ctl;
	input_type_ = input_capture_type;
}

} /* namespace videohal */

