/*******************************************************************************
    機能名称    ：  surface configureイベントモジュール
    ファイル名称：  vhal_event_item_surface_configure.cpp
*******************************************************************************/
#include "vhal_event_item_surface_configure.h"

#include <string>
#include <iostream>

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_round_cast.h"
#include "vhal_ivi_controller.h"

namespace videohal
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalEventItemSurfaceConfigure::CVhalEventItemSurfaceConfigure(void)
{
	VHAL_LOGV("CVhalEventItemSurfaceConfigure is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalEventItemSurfaceConfigure::~CVhalEventItemSurfaceConfigure(void)
{
	VHAL_LOGV("CVhalEventItemSurfaceConfigure is deleted. this=%p", this);
}

/*****************************************************************************
 処理概要：	イベント処理の実施
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalEventItemSurfaceConfigure::Exec(void) const
{
	/* surfaceのconfigureイベントを受信したときの処理 */
	VHAL_LOGD("CVhalEventItemSurfaceConfigure::Exec called. surface[%d], (%dx%d)", surface_id_, width_, height_);

	if ( nullptr != p_ivi_listener_ )
	{
		p_ivi_listener_->NotifySurfaceConfigure(UI32ToI32(surface_id_),  UI32ToI32(width_),  UI32ToI32(height_));
	}
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	データ設定
 引数    ：	CVhalIviEventListenerBase	*p_layout_mng	(i)IVIイベントリスナインスタンスポインタ
           	uint32_t					&surface_id		(i)サーフェスID
           	uint32_t					&width			(i)幅
           	uint32_t					&height			(i)高さ
 戻り値  ：	なし
*****************************************************************************/
void CVhalEventItemSurfaceConfigure::SetData(CVhalIviEventListenerBase* const pIviEventListener, const uint32_t surface_id, const uint32_t width, const uint32_t height) noexcept
{
	p_ivi_listener_ = pIviEventListener;
	surface_id_ = surface_id;
	width_ = width;
	height_ = height;
}

} /* namespace videohal */

