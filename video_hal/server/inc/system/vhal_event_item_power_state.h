/*******************************************************************************
    機能名称    ：  シーン別電源状態イベントモジュール
    ファイル名称：  vhal_event_item_power_state.h
*******************************************************************************/
#ifndef	VHAL_EVENT_ITEM_POWER_STATE_H
#define	VHAL_EVENT_ITEM_POWER_STATE_H

#include "vhal_event_item_base.h"
#include "vhal_property_control.h"

namespace videohal
{
class CVhalWaitEvent;

/*****************************************************************************
 クラス名称：CVhalEventItemPowerState
 処理概要  ：シーン別電源状態イベントクラス
*****************************************************************************/
class CVhalEventItemPowerState : public CVhalEventItemBase {
public:

	CVhalEventItemPowerState(void);
	~CVhalEventItemPowerState(void) override;
	CVhalEventItemPowerState(const CVhalEventItemPowerState& src) = delete;
	CVhalEventItemPowerState& operator=(const CVhalEventItemPowerState& src) & = delete;
	CVhalEventItemPowerState(CVhalEventItemPowerState&& src) = delete;
	CVhalEventItemPowerState& operator=(CVhalEventItemPowerState&& src) & = delete;

	/* イベント処理の実施 */
	int32_t Exec(void) const override;

	/* データ設定 */
	void SetData(CVhalPropertyControl* const p_prop, CVhalWaitEvent* const p_wait_event, const ScenePowerState power_state) noexcept;

private:
	CVhalPropertyControl* p_prop_ctl_{nullptr};
	CVhalWaitEvent* p_wait_notify_{nullptr};
	ScenePowerState dest_power_state_{ScenePowerState::VHAL_PWR_STS_MAX};
};

/*****************************************************************************
 クラス名称：CVhalEventItemVehiclePowerState
 処理概要  ：車両電源ステートイベントクラス
*****************************************************************************/
class CVhalEventItemVehiclePowerState : public CVhalEventItemBase {
public:

	CVhalEventItemVehiclePowerState(void);
	~CVhalEventItemVehiclePowerState(void) override;
	CVhalEventItemVehiclePowerState(const CVhalEventItemVehiclePowerState& src) = delete;
	CVhalEventItemVehiclePowerState& operator=(const CVhalEventItemVehiclePowerState& src) & = delete;
	CVhalEventItemVehiclePowerState(CVhalEventItemVehiclePowerState&& src) = delete;
	CVhalEventItemVehiclePowerState& operator=(CVhalEventItemVehiclePowerState&& src) & = delete;

	/* イベント処理の実施 */
	int32_t Exec(void) const override;

	/* データ設定 */
	void SetData(CVhalPropertyControl* const p_prop, CVhalWaitEvent* const p_wait_event, const VehiclePowerState power_state) noexcept;

private:
	CVhalPropertyControl* p_prop_ctl_{nullptr};
	CVhalWaitEvent* p_wait_notify_{nullptr};
	VehiclePowerState dest_power_state_{VehiclePowerState::VHAL_VPWR_STS_MAX};
};

/*****************************************************************************
 クラス名称：CVhalEventItemStrState
 処理概要  ：STRステートイベントクラス
*****************************************************************************/
class CVhalEventItemStrState : public CVhalEventItemBase {
public:

	CVhalEventItemStrState(void);
	~CVhalEventItemStrState(void) override;
	CVhalEventItemStrState(const CVhalEventItemStrState& src) = delete;
	CVhalEventItemStrState& operator=(const CVhalEventItemStrState& src) & = delete;
	CVhalEventItemStrState(CVhalEventItemStrState&& src) = delete;
	CVhalEventItemStrState& operator=(CVhalEventItemStrState&& src) & = delete;

	/* イベント処理の実施 */
	int32_t Exec(void) const override;

	/* データ設定 */
	void SetData(CVhalPropertyControl* const p_prop) noexcept;

private:
	CVhalPropertyControl* p_prop_ctl_{nullptr};
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_EVENT_ITEM_POWER_STATE_H */
