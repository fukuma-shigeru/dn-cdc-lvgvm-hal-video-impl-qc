/*******************************************************************************
    機能名称    ：  カメラ映像切替イベントモジュール
    ファイル名称：  vhal_event_item_camera_changed.h
*******************************************************************************/
#ifndef	VHAL_EVENT_ITEM_CAMERA_CHANGED_H
#define	VHAL_EVENT_ITEM_CAMERA_CHANGED_H

#include "vhal_event_item_base.h"

namespace videohal
{

class CVhalMuteListenerBase;

/*****************************************************************************
 クラス名称：CVhalEventItemCameraChanged
 処理概要  ：カメラ映像切替イベントクラス
*****************************************************************************/
class CVhalEventItemCameraChanged : public CVhalEventItemBase {
public:

	CVhalEventItemCameraChanged(void);
	~CVhalEventItemCameraChanged(void) override;
  	CVhalEventItemCameraChanged(const CVhalEventItemCameraChanged& src) = delete;
	CVhalEventItemCameraChanged& operator=(const CVhalEventItemCameraChanged& src) & = delete;
	CVhalEventItemCameraChanged(CVhalEventItemCameraChanged&& src) = delete;
	CVhalEventItemCameraChanged& operator=(CVhalEventItemCameraChanged&& src) & = delete;

	/* イベント処理の実施 */
	int32_t Exec(void) const override;

	/* データ設定 */
	void SetData(CVhalMuteListenerBase* const p_listener, const int32_t result) noexcept;

private:
	CVhalMuteListenerBase*	p_listener_{nullptr};
	int32_t					result_{0};
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_EVENT_ITEM_CAMERA_CHANGED_H */
