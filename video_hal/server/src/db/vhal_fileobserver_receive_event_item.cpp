/*******************************************************************************
    機能名称    ：  FileObserverイベントモジュール
    ファイル名称：  vhal_fileobserver_receive_event_item.cpp
*******************************************************************************/
#include "vhal_fileobserver_receive_event_item.h"

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
CVhalFileObserverReceiveEventItem::CVhalFileObserverReceiveEventItem(void)
	:p_fileob_(nullptr)
	,value_{}
{
	VHAL_LOGV("CVhalFileObserverReceiveEventItem is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalFileObserverReceiveEventItem::~CVhalFileObserverReceiveEventItem(void)
{
	VHAL_LOGV("CVhalFileObserverReceiveEventItem is deleted. this=%p", this);
}

/*****************************************************************************
 処理概要：	イベント処理の実施
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalFileObserverReceiveEventItem::Exec(void) const
{
	/* イベントを受信したときの処理 */
	VHAL_LOGD("CVhalFileObserverReceiveEventItem::Exec called.");
	if ( nullptr != p_fileob_ )
	{
		p_fileob_->ExecFileObserverEvent(monitor_path_, value_);
	}
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	データ設定
 引数    ：	CVhalFileObserverControl*	p_fileObserverctrl		(i)FileObserver制御インスタンスポインタ
          	std::string&				property_monitor_path	(i)監視対象ファイルパス
          	uint32_t					value					(i)読み込み値
 戻り値  ：	なし
*****************************************************************************/
void CVhalFileObserverReceiveEventItem::SetData(CVhalFileObserverControl* const p_fileObserverctrl, const std::string& property_monitor_path, const uint32_t value) noexcept
{
	p_fileob_		= p_fileObserverctrl;
	monitor_path_	= property_monitor_path;
	value_			= value;
}

} /* namespace videohal */
