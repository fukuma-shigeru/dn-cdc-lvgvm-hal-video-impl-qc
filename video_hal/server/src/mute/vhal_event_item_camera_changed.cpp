/*******************************************************************************
    機能名称    ：  カメラ映像切替イベントモジュール
    ファイル名称：  vhal_event_item_camera_changed.cpp
*******************************************************************************/
#include "vhal_event_item_camera_changed.h"

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_mute.h"

namespace videohal
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalEventItemCameraChanged::CVhalEventItemCameraChanged(void)
{
	VHAL_LOGV("CVhalEventItemCameraChanged is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalEventItemCameraChanged::~CVhalEventItemCameraChanged(void)
{
	VHAL_LOGV("CVhalEventItemCameraChanged is deleted. this=%p", this);
}

/*****************************************************************************
 処理概要：	イベント処理の実施
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalEventItemCameraChanged::Exec(void) const
{
	/* イベントを受信したときの処理 */
	VHAL_LOGD("CVhalEventItemCameraChanged::Exec called.");

	if (nullptr != p_listener_)
	{
		p_listener_->NotifyCameraChanged(result_);
	}
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	データ設定
 引数    ：	CVhalMuteListenerBase*	p_listener		(i)リスナーポインタ
           	const int32_t			result			(i)切替完了結果
 戻り値  ：	なし
*****************************************************************************/
void CVhalEventItemCameraChanged::SetData(CVhalMuteListenerBase* const p_listener, const int32_t result) noexcept
{
	p_listener_ = p_listener;
	result_ = result;
}

} /* namespace videohal */

