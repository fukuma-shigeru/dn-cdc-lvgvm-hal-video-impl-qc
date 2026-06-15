/*******************************************************************************
    機能名称    ：  SysDBイベントモジュール
    ファイル名称：  vhal_sysdb_event_item.cpp
*******************************************************************************/
#include "vhal_sysdb_event_item.h"

#include <iostream>

#include "vhal_define.h"
#include "vhal_log.h"

namespace videohal
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalSysdbEventItem::CVhalSysdbEventItem(void)
{
	VHAL_LOGV("CVhalSysdbEventItem is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalSysdbEventItem::~CVhalSysdbEventItem(void)
{
	VHAL_LOGV("CVhalSysdbEventItem is deleted. this=%p", this);
}

/*****************************************************************************
 処理概要：	イベント処理の実施
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalSysdbEventItem::Exec(void) const
{
	/* iviイベントを受信したときの処理 */
	VHAL_LOGD("CVhalSysdbEventItem::Exec called.");
	if ( nullptr != p_sysdbctrl_ )
	{
		p_sysdbctrl_->ExecSysdbEvent(evtcode_);
	}
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	データ取得
 引数    ：	CVhalSysdbControl	*p_sysdbctrl	(i)SysDB制御インスタンスポインタ
           	sysdbEventType		evtcode			(i)SysDB用イベントコード
 戻り値  ：	なし
*****************************************************************************/
void CVhalSysdbEventItem::SetData(CVhalSysdbControl * const p_sysdbctrl, const sysdbEventType evtcode) noexcept
{
	p_sysdbctrl_ = p_sysdbctrl;
	evtcode_     = evtcode;
}

} /* namespace videohal */

