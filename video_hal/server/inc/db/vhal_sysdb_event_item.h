/*******************************************************************************
    機能名称    ：  SysDB イベントモジュール
    ファイル名称：  vhal_sysdb_event_item.h
*******************************************************************************/
#ifndef	VHAL_SYSDB_EVENT_ITEM_H
#define	VHAL_SYSDB_EVENT_ITEM_H

#include "vhal_event_item_base.h"
#include "vhal_sysdb_control.h"

namespace videohal
{
class CVhalSysdbControl;

/*****************************************************************************
 クラス名称：CVhalSysdbEventItem
 処理概要  ：ivi イベントクラス
*****************************************************************************/
class CVhalSysdbEventItem : public CVhalEventItemBase {
public:

	CVhalSysdbEventItem(void);
	~CVhalSysdbEventItem(void) override;
	CVhalSysdbEventItem(const CVhalSysdbEventItem& src) = delete;
	CVhalSysdbEventItem& operator=(const CVhalSysdbEventItem& src) & = delete;
	CVhalSysdbEventItem(CVhalSysdbEventItem&& src) = delete;
	CVhalSysdbEventItem& operator=(CVhalSysdbEventItem&& src) & = delete;

	/* イベント処理の実施 */
	int32_t Exec(void) const override;

	/* データ設定 */
	void SetData(CVhalSysdbControl * const p_sysdbctrl, const sysdbEventType evtcode) noexcept;

private:
	CVhalSysdbControl	*p_sysdbctrl_{nullptr};
	sysdbEventType 		evtcode_{};
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_SYSDB_EVENT_ITEM_H */
