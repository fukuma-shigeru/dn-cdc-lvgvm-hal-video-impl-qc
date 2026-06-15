/*******************************************************************************
    機能名称    ：  画質調整レコード(テーマ)モジュール
    ファイル名称：  vhal_color_record_theme.h
*******************************************************************************/
#ifndef	VHAL_COLOR_RECORD_THEME_H
#define	VHAL_COLOR_RECORD_THEME_H

#include "vhal_color_record.h"

namespace videohal
{

/*****************************************************************************
 クラス名称：CVhalColorRecordTheme
 処理概要  ：画質調整のデータ管理を行う。
*****************************************************************************/
class CVhalColorRecordTheme : public CVhalColorRecord {
public:
	/* テーマモード */
	enum class ColorTypeTheme : uint8_t {
		COLOR_TYPE_THEME_MODE_DARK = 0,			/* 手動設定(Dark color) */
		COLOR_TYPE_THEME_MODE_DARKAUTO,			/* 自動設定(Dark color) */
		COLOR_TYPE_THEME_MODE_LIGHT,			/* 手動設定(Light color) */
		COLOR_TYPE_THEME_MODE_LIGHTAUTO,		/* 自動設定(Light color) */
		COLOR_TYPE_THEME_MODE_FORCED_LIGHT,		/* 強制昼画 */
		COLOR_TYPE_THEME_MODE_MAX
	};

	CVhalColorRecordTheme(void);
	~CVhalColorRecordTheme(void) override;
  	CVhalColorRecordTheme(const CVhalColorRecordTheme& src) = delete;
	CVhalColorRecordTheme& operator=(const CVhalColorRecordTheme& src) & = delete;
	CVhalColorRecordTheme(CVhalColorRecordTheme&& src) = delete;
	CVhalColorRecordTheme& operator=(CVhalColorRecordTheme&& src) & = delete;

	int32_t Initialize(const struct CVhalColorRecord::ColorImgAdjStep& step, const bool backup) override;
	void Finalize(void) const;

	const struct CVhalColorRecord::ColorImgAdjStep& GetStep(uint32_t color_mode) const override;
	int32_t SetStep(const uint32_t color_mode, const struct CVhalColorRecord::ColorImgAdjStep& step) override;
	const CVhalColorRecord& operator=(const CVhalColorRecord& src) override;
	CVhalColorRecord::ColorType GetColorType(void) const noexcept override;
	bool IsBackupTarget(void) const override;

private:
	struct CVhalColorRecord::ColorImgAdjStep		step_[static_cast<uint8_t>(ColorTypeTheme::COLOR_TYPE_THEME_MODE_MAX)];
	bool											backup_target_;
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_COLOR_RECORD_THEME_H */
