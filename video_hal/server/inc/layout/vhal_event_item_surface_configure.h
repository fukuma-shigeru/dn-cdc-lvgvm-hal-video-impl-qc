/*******************************************************************************
    機能名称    ：  surface configureイベントモジュール
    ファイル名称：  vhal_event_item_surface_configure.h
*******************************************************************************/
#ifndef	VHAL_EVENT_ITEM_SURFACE_CONFIGURE_H
#define	VHAL_EVENT_ITEM_SURFACE_CONFIGURE_H

#include "vhal_event_item_base.h"

namespace videohal
{

class CVhalIviEventListenerBase;

/*****************************************************************************
 クラス名称：CVhalEventItemSurfaceConfigure
 処理概要  ：surface configureイベントクラス
*****************************************************************************/
class CVhalEventItemSurfaceConfigure : public CVhalEventItemBase {
public:

	CVhalEventItemSurfaceConfigure (void);
	~CVhalEventItemSurfaceConfigure (void) override;
  	CVhalEventItemSurfaceConfigure(const CVhalEventItemSurfaceConfigure& src) = delete;
	CVhalEventItemSurfaceConfigure& operator=(const CVhalEventItemSurfaceConfigure& src) & = delete;
	CVhalEventItemSurfaceConfigure(CVhalEventItemSurfaceConfigure&& src) = delete;
	CVhalEventItemSurfaceConfigure& operator=(CVhalEventItemSurfaceConfigure&& src) & = delete;

	/* イベント処理の実施 */
	int32_t Exec(void) const override;

	/* データ設定 */
	void SetData(CVhalIviEventListenerBase* const pIviEventListener, const uint32_t surface_id, const uint32_t width, const uint32_t height) noexcept;

protected:

private:
	CVhalIviEventListenerBase* p_ivi_listener_{nullptr};
	uint32_t surface_id_{0U};	/* サーフェスID */
	uint32_t width_{0U};		/* 幅 */
	uint32_t height_{0U};		/* 高さ */
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_EVENT_ITEM_SURFACE_CONFIGURE_H */
