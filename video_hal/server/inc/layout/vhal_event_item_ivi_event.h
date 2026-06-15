/*******************************************************************************
    機能名称    ：  ivi イベントモジュール
    ファイル名称：  vhal_event_item_ivi_event.h
*******************************************************************************/
#ifndef	VHAL_EVENT_ITEM_IVI_EVENT_H
#define	VHAL_EVENT_ITEM_IVI_EVENT_H

#include "vhal_event_item_base.h"

namespace videohal
{

class CVhalIviEventListenerBase;

/*****************************************************************************
 クラス名称：CVhalEventItemIviEvent
 処理概要  ：ivi イベントクラス
*****************************************************************************/
class CVhalEventItemIviEvent : public CVhalEventItemBase {
public:

	CVhalEventItemIviEvent(void);
	~CVhalEventItemIviEvent(void) override;
  	CVhalEventItemIviEvent(const CVhalEventItemIviEvent& src) = delete;
	CVhalEventItemIviEvent& operator=(const CVhalEventItemIviEvent& src) & = delete;
	CVhalEventItemIviEvent(CVhalEventItemIviEvent&& src) = delete;
	CVhalEventItemIviEvent& operator=(CVhalEventItemIviEvent&& src) & = delete;

	/* イベント処理の実施 */
	int32_t Exec(void) const override;

	/* データ設定 */
	void SetData(CVhalIviEventListenerBase* const pIviEventListener, const bool object, const int32_t id, const bool created) noexcept;

private:
	CVhalIviEventListenerBase* p_ivi_listener_{nullptr};
	bool object_{false};		/* true:layer false:surface */
	int32_t id_{0};
	bool created_{false};
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_EVENT_ITEM_IVI_EVENT_H */
