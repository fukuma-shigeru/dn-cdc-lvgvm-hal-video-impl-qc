/*******************************************************************************
    機能名称    ：  画面キャプチャ イベントモジュール
    ファイル名称：  vhal_event_item_screen_shot_event.h
*******************************************************************************/
#ifndef	VHAL_EVENT_ITEM_SCREEN_SHOT_EVENT_H
#define	VHAL_EVENT_ITEM_SCREEN_SHOT_EVENT_H

#include "vhal_define.h"
#include "vhal_event_item_base.h"

namespace videohal
{
/*****************************************************************************
 キャプチャタイプ列挙型
*****************************************************************************/
enum class ScreenShotType : int32_t
{
	SCREEN = 0,		/* スクリーンキャプチャ */
	SURFACE = 1		/* サーフェスキャプチャ */
};

class CVhalLayoutManager;

/*****************************************************************************
 クラス名称：CVhalEventItemScreenShotEvent
 処理概要  ：画面キャプチャ イベントクラス
*****************************************************************************/
class CVhalEventItemScreenShotEvent : public CVhalEventItemBase {
public:

	CVhalEventItemScreenShotEvent(void);
	~CVhalEventItemScreenShotEvent(void) override;
  	CVhalEventItemScreenShotEvent(const CVhalEventItemScreenShotEvent& src) = delete;
	CVhalEventItemScreenShotEvent& operator=(const CVhalEventItemScreenShotEvent& src) & = delete;
	CVhalEventItemScreenShotEvent(CVhalEventItemScreenShotEvent&& src) = delete;
	CVhalEventItemScreenShotEvent& operator=(CVhalEventItemScreenShotEvent&& src) & = delete;

	/* イベント処理の実施 */
	int32_t Exec(void) const override;

	/* データ設定 */
	void SetData(CVhalLayoutManager* const p_layout_mng, const int32_t result, const ScreenShotType type) noexcept;

private:
	CVhalLayoutManager* p_layout_mng_{nullptr};
	int32_t result_{VHAL_CAPTURE_STS_FAILED};
	ScreenShotType type_{ScreenShotType::SCREEN};
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_EVENT_ITEM_SCREEN_SHOT_EVENT_H */
