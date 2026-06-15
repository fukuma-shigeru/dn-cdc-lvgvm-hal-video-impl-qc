/*******************************************************************************
    機能名称    ：  画質調整レコード(昼夜)モジュール
    ファイル名称：  vhal_color_record_daynight.h
*******************************************************************************/
#ifndef	VHAL_COLOR_RECORD_DAYNIGHT_H
#define	VHAL_COLOR_RECORD_DAYNIGHT_H

#include "vhal_color_record.h"

namespace videohal
{

/*****************************************************************************
 クラス名称：CVhalColorRecordDaynight
 処理概要  ：画質調整のデータ管理を行う。
*****************************************************************************/
class CVhalColorRecordDaynight : public CVhalColorRecord {
public:
	/* 昼夜モード */
	enum class ColorTypeDayNight : uint8_t {
		COLOR_TYPE_DAYNIGHT_MODE_DAY = 0,			/* 昼 */
		COLOR_TYPE_DAYNIGHT_MODE_NIGHT,				/* 夜 */
		COLOR_TYPE_DAYNIGHT_MODE_FORCED_DAY,		/* 強制昼 */
		COLOR_TYPE_DAYNIGHT_MODE_MAX
	};

	CVhalColorRecordDaynight(void);
	~CVhalColorRecordDaynight(void) override;
	CVhalColorRecordDaynight(const CVhalColorRecordDaynight& src) = delete;
	CVhalColorRecordDaynight& operator=(const CVhalColorRecordDaynight& src) & = delete;
	CVhalColorRecordDaynight(CVhalColorRecordDaynight&& src) = delete;
	CVhalColorRecordDaynight& operator=(CVhalColorRecordDaynight&& src) & = delete;

	int32_t Initialize(const struct CVhalColorRecord::ColorImgAdjStep& step, const bool backup) override;
	void Finalize(void) const;

	const struct CVhalColorRecord::ColorImgAdjStep& GetStep(uint32_t color_mode) const override;
	int32_t SetStep(const uint32_t color_mode, const struct CVhalColorRecord::ColorImgAdjStep& step) override;
	const CVhalColorRecord& operator=(const CVhalColorRecord& src) override;
	CVhalColorRecord::ColorType GetColorType(void) const noexcept override;
	bool IsBackupTarget(void) const override;

private:
	struct CVhalColorRecord::ColorImgAdjStep		step_[static_cast<uint8_t>(ColorTypeDayNight::COLOR_TYPE_DAYNIGHT_MODE_MAX)];
	bool											backup_target_;
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_COLOR_RECORD_DAYNIGHT_H */
