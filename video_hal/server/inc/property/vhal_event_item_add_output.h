/*******************************************************************************
    機能名称    ：  出力デバイス追加イベントモジュール
    ファイル名称：  vhal_event_item_add_output.h
*******************************************************************************/
#ifndef	VHAL_EVENT_ITEM_ADD_OUTPUT_H
#define	VHAL_EVENT_ITEM_ADD_OUTPUT_H

#include "vhal_event_item_base.h"

namespace videohal
{

class CVhalLayoutManager;

/*****************************************************************************
 クラス名称：CVhalEventItemAddOutput
 処理概要  ：出力デバイス追加イベントクラス
*****************************************************************************/
class CVhalEventItemAddOutput : public CVhalEventItemBase {
public:

	CVhalEventItemAddOutput(void);
	~CVhalEventItemAddOutput(void) override;
  	CVhalEventItemAddOutput(const CVhalEventItemAddOutput& src) = delete;
	CVhalEventItemAddOutput& operator=(const CVhalEventItemAddOutput& src) & = delete;
	CVhalEventItemAddOutput(CVhalEventItemAddOutput&& src) = delete;
	CVhalEventItemAddOutput& operator=(CVhalEventItemAddOutput&& src) & = delete;

	/* イベント処理の実施 */
	int32_t Exec(void) const override;

	/* データ設定 */
	void SetData(CVhalLayoutManager* const p_layout_mng, const int32_t width, const int32_t height, const std::string& model, const std::string& make);

private:
	CVhalLayoutManager* p_layout_mng_;
	int32_t width_;
	int32_t height_;
	std::string model_;
	std::string make_;
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_EVENT_ITEM_ADD_OUTPUT_H */
