/*******************************************************************************
    機能名称    ：  画質調整レコード(テーマ)モジュール
    ファイル名称：  vhal_color_record_theme.cpp
*******************************************************************************/
#include "vhal_color_record_theme.h"

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_round_cast.h"

#include <map>

namespace videohal
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalColorRecordTheme::CVhalColorRecordTheme(void)
{
	VHAL_LOGV("CVhalColorRecordTheme is created. this=%p", this);

	for (uint32_t i{0U}; static_cast<uint8_t>(ColorTypeTheme::COLOR_TYPE_THEME_MODE_MAX) > i; i++)
	{
		step_[i].contrast_ = 0;
		step_[i].brightness_ = 0;
	}
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalColorRecordTheme::~CVhalColorRecordTheme(void)
{
	VHAL_LOGV("CVhalColorRecordTheme is deleted. this=%p", this);
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ：	const struct CVhalColorRecord::ColorImgAdjStep& step		(i)画質調整初期値データ
			const bool backup											(i)バックアップ要不要
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalColorRecordTheme::Initialize(const struct CVhalColorRecord::ColorImgAdjStep& step, const bool backup)
{
	VHAL_LOGV("CVhalColorRecordTheme");
	for (uint32_t i{0U}; static_cast<uint8_t>(ColorTypeTheme::COLOR_TYPE_THEME_MODE_MAX) > i; i++)
	{
		step_[i] = step;
	}
	backup_target_ = backup;
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	終了処理（内部リソースの解放）
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorRecordTheme::Finalize(void) const
{
	VHAL_LOGV("CVhalColorRecordTheme");
}

/*****************************************************************************
 処理概要：	画質調整データの取得
 引数    ：	uint32_t color_mode			(i)テーマモード(ColorTypeTheme)
 戻り値  ：	画質調整データ
*****************************************************************************/
const struct CVhalColorRecord::ColorImgAdjStep& CVhalColorRecordTheme::GetStep(uint32_t color_mode) const
{
	if (static_cast<uint8_t>(ColorTypeTheme::COLOR_TYPE_THEME_MODE_MAX) <= color_mode)
	{
		VHAL_LOGW("MODE_MAX over. color_mode=%d -> %d", color_mode, VHAL_THEME_COLOR_AUTO_DARK);
		color_mode = I32ToUI32(VHAL_THEME_COLOR_AUTO_DARK);
	}

	VHAL_LOGV("color_mode=%d, contrast=%d, brightness=%d", color_mode, step_[color_mode].contrast_, step_[color_mode].brightness_);
	return step_[color_mode];
}

/*****************************************************************************
 処理概要：	バックアップ対象か否かを照会
 引数    ：	なし
 戻り値  ：	バックアップ対象か否か
*****************************************************************************/
bool CVhalColorRecordTheme::IsBackupTarget(void) const
{
	return backup_target_;
}

/*****************************************************************************
 処理概要：	画質調整データの設定
 引数    ：	const uint32_t color_mode				(i)テーマモード(ColorTypeTheme)
           	const struct ColorImgAdjStep& step		(i)画質調整データ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalColorRecordTheme::SetStep(const uint32_t color_mode, const struct CVhalColorRecord::ColorImgAdjStep& step)
{
	if (static_cast<uint8_t>(ColorTypeTheme::COLOR_TYPE_THEME_MODE_MAX) <= color_mode)
	{
		VHAL_LOGE("MODE_MAX over. color_mode=%d", color_mode);
		return VHAL_ERR_PARAM;
	}

	VHAL_LOGV("color_mode=%d, contrast=%d, brightness=%d", color_mode, step.contrast_, step.brightness_);
	step_[color_mode] = step;

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	画質調整データのコピー
 引数    ：	const CVhalColorRecord& src		(i)画質調整レコードモジュール
 戻り値  ：	画質調整レコードモジュール
*****************************************************************************/
const CVhalColorRecord& CVhalColorRecordTheme::operator=(const CVhalColorRecord& src)
{
	for (uint32_t i{0U}; static_cast<uint8_t>(ColorTypeTheme::COLOR_TYPE_THEME_MODE_MAX) > i; i++)
	{
		const int32_t ret{SetStep(i, src.GetStep(i))};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGW("SetStep error. ret=%d, i=%d", ret, i);
		}
	}

	return *this;
}

/*****************************************************************************
 処理概要：	カラータイプ（テーマカラー）の取得
 引数    ：	なし
 戻り値  ：	カラータイプ（テーマカラー）
*****************************************************************************/
CVhalColorRecord::ColorType CVhalColorRecordTheme::GetColorType(void) const noexcept
{
	return ColorType::COLOR_TYPE_THEME;
}

} /* namespace videohal */
