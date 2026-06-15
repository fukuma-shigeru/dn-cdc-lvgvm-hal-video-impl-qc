/*******************************************************************************
    機能名称    ：  画面キャプチャ イベントモジュール
    ファイル名称：  vhal_event_item_screen_shot_event.cpp
*******************************************************************************/
#include "vhal_event_item_screen_shot_event.h"

#include <iostream>

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_layout_mng.h"

namespace videohal
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalEventItemScreenShotEvent::CVhalEventItemScreenShotEvent(void)
{
	VHAL_LOGV("CVhalEventItemScreenShotEvent is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalEventItemScreenShotEvent::~CVhalEventItemScreenShotEvent(void)
{
	VHAL_LOGV("CVhalEventItemScreenShotEvent is deleted. this=%p", this);
}

/*****************************************************************************
 処理概要：	イベント処理の実施
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalEventItemScreenShotEvent::Exec(void) const
{
	/* 画面キャプチャ イベントを受信したときの処理 */
	VHAL_LOGD("CVhalEventItemScreenShotEvent::Exec called. type=%d", static_cast<int32_t>(type_));
	if ( nullptr != p_layout_mng_ )
	{
		p_layout_mng_->NotifyScreenShotResult(result_, type_);
	}
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	データ設定
 引数    ：	CVhalLayoutManager* p_layout_mng	(i)レイアウト制御インスタンスポインタ
           	const int32_t result				(i)処理結果
           	const ScreenShotType type			(i)キャプチャタイプ
 戻り値  ：	なし
*****************************************************************************/
void CVhalEventItemScreenShotEvent::SetData(CVhalLayoutManager* const p_layout_mng, const int32_t result, const ScreenShotType type) noexcept
{
	p_layout_mng_ = p_layout_mng;
	result_ = result;
	type_ = type;
}

} /* namespace videohal */

