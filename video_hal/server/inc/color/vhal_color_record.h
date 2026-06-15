/*******************************************************************************
    機能名称    ：  画質調整レコードモジュール
    ファイル名称：  vhal_color_record.h
*******************************************************************************/
#ifndef	VHAL_COLOR_RECORD_H
#define	VHAL_COLOR_RECORD_H

#include <cstdint>

namespace videohal
{


/*****************************************************************************
 クラス名称：CVhalColorRecord
 処理概要  ：画質調整のデータ管理を行う。
*****************************************************************************/
class CVhalColorRecord {
public:
	enum class ColorType : uint8_t {
		COLOR_TYPE_DAYNIGHT = 0,			/* DayNight */
		COLOR_TYPE_THEME					/* Theme */
	};

	struct ColorImgAdjStep {
		int32_t contrast_;			/* コントラスト調整STEP	*/
		int32_t brightness_;		/* 明るさ調整STEP		*/
	};


	CVhalColorRecord(void);
	virtual ~CVhalColorRecord(void);
	CVhalColorRecord(const CVhalColorRecord& src) = delete;
	CVhalColorRecord(CVhalColorRecord&& src) = delete;

	virtual int32_t Initialize(const struct ColorImgAdjStep& step, const bool backup) = 0;

	virtual const struct ColorImgAdjStep& GetStep(uint32_t color_mode) const = 0;
	virtual int32_t SetStep(const uint32_t color_mode, const struct ColorImgAdjStep& step) = 0;
	virtual ColorType GetColorType(void) const = 0;
	virtual bool IsBackupTarget(void) const = 0;
	static void Copy(CVhalColorRecord &dest, const CVhalColorRecord &src);

protected:
	virtual const CVhalColorRecord& operator=(const CVhalColorRecord& src) = 0;
	CVhalColorRecord& operator=(CVhalColorRecord&& src) & = delete;

};


} /* namespace videohal */


#endif	/* #ifndef	VHAL_COLOR_RECORD_H */
