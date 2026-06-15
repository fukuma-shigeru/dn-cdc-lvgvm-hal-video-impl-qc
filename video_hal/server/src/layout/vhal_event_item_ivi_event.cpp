/*******************************************************************************
    機能名称    ：  ivi イベントモジュール
    ファイル名称：  vhal_event_item_ivi_event.cpp
*******************************************************************************/
#include "vhal_event_item_ivi_event.h"

#include <iostream>

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_ivi_controller.h"

namespace videohal
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalEventItemIviEvent::CVhalEventItemIviEvent(void)
{
	VHAL_LOGV("CVhalEventItemIviEvent is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalEventItemIviEvent::~CVhalEventItemIviEvent(void)
{
	VHAL_LOGV("CVhalEventItemSurfaceConfigure is deleted. this=%p", this);
}

/*****************************************************************************
 処理概要：	イベント処理の実施
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalEventItemIviEvent::Exec(void) const
{
	/* iviイベントを受信したときの処理 */
	VHAL_LOGD("CVhalEventItemIviEvent::Exec called.");

	if ( nullptr != p_ivi_listener_ )
	{
		CVhalIviEventListenerBase::IviObjectType obj_type{CVhalIviEventListenerBase::IviObjectType::kIviObjectSurface};
		if ( true == object_ )
		{
			obj_type = CVhalIviEventListenerBase::IviObjectType::kIviObjectLayer;
		}
		p_ivi_listener_->NotifyEvent(obj_type, id_, created_);
	}
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	データ取得
 引数    ：	CVhalIviEventListenerBase	*p_layout_mng	(i)IVIイベントリスナインスタンスポインタ
           	bool						object			(i)true:layer false:surface
           	int32_t						id				(i)IVI-ID
           	bool						created			(i)true:create false:remove
 戻り値  ：	なし
*****************************************************************************/
void CVhalEventItemIviEvent::SetData(CVhalIviEventListenerBase* const pIviEventListener, const bool object, const int32_t id, const bool created) noexcept
{
	p_ivi_listener_ = pIviEventListener;
	object_  = object;
	id_      = id;
	created_ = created;
}

} /* namespace videohal */

