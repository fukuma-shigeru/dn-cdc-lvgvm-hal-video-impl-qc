/*******************************************************************************
    機能名称    ：  キャプチャ再初期化イベントモジュール
    ファイル名称：  vhal_event_item_capture_reset.h
*******************************************************************************/
#ifndef	VHAL_EVENT_ITEM_CAPTURE_RESET_H
#define	VHAL_EVENT_ITEM_CAPTURE_RESET_H

#include "vhal_event_item_base.h"
#include "vhal_capture_control.h"

namespace videohal
{
class CVhalPropertyControl;

/*****************************************************************************
 クラス名称：CVhalEventItemCaptureReset
 処理概要  ：キャプチャ再初期化イベントクラス
*****************************************************************************/
class CVhalEventItemCaptureReset : public CVhalEventItemBase {
public:

	CVhalEventItemCaptureReset(void);
	~CVhalEventItemCaptureReset(void) override;
  	CVhalEventItemCaptureReset(const CVhalEventItemCaptureReset& src) = delete;
	CVhalEventItemCaptureReset& operator=(const CVhalEventItemCaptureReset& src) & = default;
	CVhalEventItemCaptureReset(CVhalEventItemCaptureReset&& src) = delete;
	CVhalEventItemCaptureReset& operator=(CVhalEventItemCaptureReset&& src) & = delete;

	/* イベント処理の実施 */
	int32_t Exec(void) const override;

	/* データ設定 */
	void SetData(CVhalPropertyControl* const p_prop_ctl, const captureInputType input_capture_type) noexcept;

private:
	CVhalPropertyControl*	p_prop_{nullptr};
	captureInputType 		input_type_{captureInputType::VHAL_CAPTURE_INPUT_MAX};
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_EVENT_ITEM_CAPTURE_RESET_H */
