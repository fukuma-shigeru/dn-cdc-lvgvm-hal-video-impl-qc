/*******************************************************************************
    機能名称    ：  シーン別電源状態イベントモジュール
    ファイル名称：  vhal_event_item_power_state.cpp
*******************************************************************************/
#include "vhal_event_item_power_state.h"

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_wait_event.h"
#include "vhal_property_control.h"

namespace videohal
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalEventItemPowerState::CVhalEventItemPowerState(void)
{
	VHAL_LOGV("CVhalEventItemPowerState is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalEventItemPowerState::~CVhalEventItemPowerState(void)
{
	VHAL_LOGV("CVhalEventItemPowerState is deleted. this=%p", this);
}

/*****************************************************************************
 処理概要：	イベント処理の実施
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalEventItemPowerState::Exec(void) const
{
	/* イベントを受信したときの処理 */
	VHAL_LOGD("CVhalEventItemPowerState::Exec called.");

	if (nullptr != p_prop_ctl_)
	{
		p_prop_ctl_->ChangePowerState(dest_power_state_);
		p_wait_notify_->NotifyOne();
	}
	
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	データ設定
 引数    ：	CVhalPropertyControl*	p_prop		(i)プロパティ制御インスタンスポインタ
         ：	CVhalWaitEvent* p_wait_event		(i)シーン別電源イベントクラスインスタンスポインタ
         ：	const ScenePowerState power_state	(i)電源状態
 戻り値  ：	なし
*****************************************************************************/
void CVhalEventItemPowerState::SetData(CVhalPropertyControl* const p_prop,  CVhalWaitEvent* const p_wait_event, const ScenePowerState power_state) noexcept
{
	p_prop_ctl_       = p_prop;
	p_wait_notify_    = p_wait_event;
	dest_power_state_ = power_state;
}


/* ここからBEV */
/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalEventItemVehiclePowerState::CVhalEventItemVehiclePowerState(void)
{
	VHAL_LOGV("CVhalEventItemVehiclePowerState is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalEventItemVehiclePowerState::~CVhalEventItemVehiclePowerState(void)
{
	VHAL_LOGV("CVhalEventItemVehiclePowerState is deleted. this=%p", this);
}

/*****************************************************************************
 処理概要：	イベント処理の実施
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalEventItemVehiclePowerState::Exec(void) const
{
	/* イベントを受信したときの処理 */
	VHAL_LOGD("CVhalEventItemVehiclePowerState::Exec called.");

	if (nullptr != p_prop_ctl_)
	{
		p_prop_ctl_->ChangeVehiclePowerState(dest_power_state_);
		p_wait_notify_->NotifyOne();
	}
	
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	データ設定
 引数    ：	CVhalPropertyControl*	p_prop		(i)プロパティ制御インスタンスポインタ
         ：	CVhalWaitEvent* p_wait_event		(i)シーン別電源イベントクラスインスタンスポインタ
         ：	const VehiclePowerState power_state	(i)車両電源状態
 戻り値  ：	なし
*****************************************************************************/
void CVhalEventItemVehiclePowerState::SetData(CVhalPropertyControl* const p_prop,  CVhalWaitEvent* const p_wait_event, const VehiclePowerState power_state) noexcept
{
	p_prop_ctl_       = p_prop;
	p_wait_notify_    = p_wait_event;
	dest_power_state_ = power_state;
}

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalEventItemStrState::CVhalEventItemStrState(void)
{
	VHAL_LOGV("CVhalEventItemStrState is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalEventItemStrState::~CVhalEventItemStrState(void)
{
	VHAL_LOGV("CVhalEventItemStrState is deleted. this=%p", this);
}

/*****************************************************************************
 処理概要：	イベント処理の実施
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_****		エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalEventItemStrState::Exec(void) const
{
	/* イベントを受信したときの処理 */
	VHAL_LOGD("CVhalEventItemStrState::Exec called.");
	if (nullptr != p_prop_ctl_)
	{
		p_prop_ctl_->StrStateEvent();
		return VHAL_SUCCESS;
	}
	return VHAL_ERR;
}

/*****************************************************************************
 処理概要：	データ設定
 引数    ：	CVhalPropertyControl*	p_prop		(i)プロパティ制御インスタンスポインタ
 戻り値  ：	なし
*****************************************************************************/
void CVhalEventItemStrState::SetData(CVhalPropertyControl* const p_prop) noexcept
{
	p_prop_ctl_    = p_prop;
}

} /* namespace videohal */

