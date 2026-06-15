/*******************************************************************************
    機能名称    ：  画質調整データベースモジュール
    ファイル名称：  vhal_color_db.cpp
*******************************************************************************/
#include "vhal_color_db.h"

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_round_cast.h"

#include "vhal_color_record_daynight.h"
#include "vhal_color_record_theme.h"
#include "vhal_backup_control.h"

namespace videohal
{

namespace
{
/* 画質調整バックアップ対象のデータ識別文字列 */
const std::string		VHAL_COLOR_ADJUST_BACKUP_NAME				{"/tier1_video_hal/color_adjust_param"};
/* 画質調整バックアップサイズ */
constexpr std::size_t	VHAL_COLOR_BACKUP_SIZE						{164U};
/* 画質調整バックアップ 初期化上情報 */
constexpr uint8_t		VHAL_COLOR_BACKUP_UNINITIALIZED				{0U};		/* 未初期化 */
constexpr uint8_t		VHAL_COLOR_BACKUP_INITIALIZED				{1U};		/* 初期化済み */

}

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalColorDatabase::CVhalColorDatabase(void)
	:init_state_(VHAL_COLOR_BACKUP_UNINITIALIZED)
	,p_backup_control_(nullptr)
	,theme_color_init_{0, 0}
	,daynight_color_init_{0, 0}
{
	VHAL_LOGV("CVhalColorDatabase is created. this=%p", this);
	record_list_.clear();
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalColorDatabase::~CVhalColorDatabase(void)
{
	VHAL_LOGV("CVhalColorDatabase is deleted. this=%p", this);
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_MEMORY		メモリ確保エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalColorDatabase::Initialize(void)
{
	VHAL_LOGV("CVhalColorDatabase");

	int32_t	ret{VHAL_SUCCESS};
	p_backup_control_ = std::make_unique<CVhalBackupControl>();
	if (nullptr != p_backup_control_)
	{
		ret = VHAL_SUCCESS;
	}
	else
	{
		VHAL_LOGE("new CVhalBackupControl error.");
		ret = VHAL_ERR_MEMORY;
	}

	return ret;
}

/*****************************************************************************
 処理概要：	終了処理（内部リソースの解放）
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorDatabase::Finalize(void)
{
	VHAL_LOGV("CVhalColorDatabase");

	if (false == record_list_.empty())
	{
		record_list_.clear();
	}

	if (nullptr != p_backup_control_)
	{
		p_backup_control_ = nullptr;
	}
}

/*****************************************************************************
 処理概要：	画質調整テーブルの追加
 引数    ：	const uint32_t color_id			(i)ユニークID
           	const ColorType color_type		(i)DayNight/Theme
           	const CVhalColorRecord::ColorImgAdjStep& color_init		(i)画質調整初期値データ
			const bool backup				(i)バックアップ要不要
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_ERR_MEMORY		メモリ確保エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalColorDatabase::CreateColorRecord(const uint32_t color_id, const CVhalColorRecord::ColorType color_type, const CVhalColorRecord::ColorImgAdjStep& color_init, const bool backup)
{
	const std::lock_guard<std::mutex> lock_step{mtx_record_};

	VHAL_LOGD("color_id=%d, color_type=%d", color_id, color_type);

	const auto iter = record_list_.find(color_id);
	if (iter != record_list_.end())
	{
		VHAL_LOGE("parameter error. already color_id=%d", color_id);
		return VHAL_ERR_PARAM;
	}

	if (CVhalColorRecord::ColorType::COLOR_TYPE_DAYNIGHT == color_type)		/* DayNight */
	{
		std::unique_ptr<CVhalColorRecordDaynight> dayNight{std::make_unique<CVhalColorRecordDaynight>()};
		if (nullptr == dayNight)
		{
			VHAL_LOGE("new CVhalColorRecordDaynight error.");
			return VHAL_ERR_MEMORY;
		}
		const int32_t ret{dayNight->Initialize(color_init, backup)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGW("dayNight->Initialize() error. ret=%d", ret);
		}
		(void)record_list_.insert(std::make_pair(color_id, std::move(dayNight)));
		daynight_color_init_ = color_init;
	}
	else if (CVhalColorRecord::ColorType::COLOR_TYPE_THEME == color_type)	/* Theme */
	{
		std::unique_ptr<CVhalColorRecordTheme> theme{std::make_unique<CVhalColorRecordTheme>()};
		if (nullptr == theme)
		{
			VHAL_LOGE("new CVhalColorRecordTheme error.");
			return VHAL_ERR_MEMORY;
		}
		const int32_t ret{theme->Initialize(color_init, backup)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGW("theme->Initialize() error. ret=%d", ret);
		}
		(void)record_list_.insert(std::make_pair(color_id, std::move(theme)));
		theme_color_init_ = color_init;
	}
	else
	{
		VHAL_LOGE("parameter error. color_type=%d", color_type);
		return VHAL_ERR_PARAM;
	}

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	画質調整テーブルの削除
 引数    ：	const uint32_t color_id		(i)ユニークID
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorDatabase::DeleteColorRecord(const uint32_t color_id)
{
	const std::lock_guard<std::mutex> lock_step{mtx_record_};

	const auto iter = record_list_.find(color_id);
	if (iter != record_list_.end())
	{
		VHAL_LOGV("color_id=%d", color_id);

		(void)record_list_.erase(color_id);
	}

}

/*****************************************************************************
 処理概要：	画質調整テーブルのリストア（データ無し時はデフォルトデータをバックアップ）
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR			異常終了
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalColorDatabase::LoadColorRecord(void)
{
	VHAL_LOGV("record_list_.size=%lu", record_list_.size());

	int32_t	ret{VHAL_SUCCESS};

	/* データリストア */
	std::vector<uint8_t> backup_data(VHAL_COLOR_BACKUP_SIZE);
	ret = p_backup_control_->Load(VHAL_COLOR_ADJUST_BACKUP_NAME, backup_data);
	if (VHAL_SUCCESS == ret)
	{
		/* 初期化情報取得 */
		init_state_ = backup_data[0];
	}
	if ((VHAL_SUCCESS != ret) || (VHAL_COLOR_BACKUP_UNINITIALIZED == init_state_))
	{
		/* データリストア失敗 又は、データ無し */
		/* レコードリスト(record_list_)から、バックアップデータへ設定（デフォルトデータ） */
		ret = SerializeBackupData(backup_data);
		if (VHAL_SUCCESS == ret)
		{
			/* データバックアップ */
			ret = p_backup_control_->Save(VHAL_COLOR_ADJUST_BACKUP_NAME, backup_data, false);
			if (VHAL_SUCCESS == ret)
			{
				VHAL_LOGI("Backup default color data=%s", VHAL_COLOR_ADJUST_BACKUP_NAME.c_str());
				init_state_ = VHAL_COLOR_BACKUP_INITIALIZED;
				ret = VHAL_SUCCESS;
			}
			else
			{
				VHAL_LOGE("p_backup_control_->Save() error. ret=%d", ret);
				ret = VHAL_ERR;
			}
		}
		else
		{
			VHAL_LOGE("SerializeBackupData() error. ret=%d", ret);
			ret = VHAL_ERR;
		}
	}
	else
	{
		/* データリストア成功 */
		/* バックアップデータから、レコードリスト(record_list_)へ設定 */
		ret = DeserializeBackupData(backup_data);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("DeserializeBackupData() error. ret=%d", ret);
			ret = VHAL_ERR;
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	画質調整テーブルのバックアップ
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR			異常終了
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalColorDatabase::SaveColorRecord(void)
{
	VHAL_LOGV("record_list_.size=%lu", record_list_.size());

	int32_t	ret{VHAL_SUCCESS};

	std::vector<uint8_t> backup_data(VHAL_COLOR_BACKUP_SIZE);
	/* レコードリスト(record_list_)から、バックアップデータへ設定 */
	ret = SerializeBackupData(backup_data);
	if (VHAL_SUCCESS == ret)
	{
		/* データバックアップ（データ属性チェックあり） */
		ret = p_backup_control_->Save(VHAL_COLOR_ADJUST_BACKUP_NAME, backup_data, true);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("p_backup_control_->Save() error. ret=%d", ret);
			ret = VHAL_ERR;
		}
	}
	else
	{
		VHAL_LOGE("SerializeBackupData() error. ret=%d", ret);
		ret = VHAL_ERR;
	}

	return ret;
}

/*****************************************************************************
 処理概要：	画質調整テーブルのクリア（画質調整デフォルト設定）
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR			異常終了
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalColorDatabase::ClearColorRecord(void)
{
	VHAL_LOGV("record_list_.size=%lu", record_list_.size());

	int32_t	result{VHAL_SUCCESS};
	for (const auto &itr_record : record_list_)
	{
		int32_t	ret{VHAL_SUCCESS};
		const uint32_t color_id{itr_record.first};
		const CVhalColorRecord::ColorType color_type{itr_record.second->GetColorType()};
		VHAL_LOGD("color_id=%d, color_type=%d", color_id, color_type);
		if (CVhalColorRecord::ColorType::COLOR_TYPE_THEME == color_type)
		{
			/* テーマカラー */
			CVhalColorRecordTheme	colorRecord{};
			for (uint32_t color_mode{0U}; static_cast<uint8_t>(CVhalColorRecordTheme::ColorTypeTheme::COLOR_TYPE_THEME_MODE_MAX) > color_mode; (void)color_mode++)
			{
				/* 画質調整データの設定 */
				ret = colorRecord.SetStep(color_mode, theme_color_init_);
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("colorRecord.SetStep error. ret=%d, color_mode=%d", ret, color_mode);
					result = VHAL_ERR;
					break;
				}
			}
			/* 画質調整テーブルの書込み */
			if (VHAL_SUCCESS == ret)
			{
				ret = Write(color_id, colorRecord);				
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("Write error. ret=%d, color_id=%d", ret, color_id);
					result = VHAL_ERR;
				}
			}
		}
		else
		{
			/* 昼夜モード */
			CVhalColorRecordDaynight	colorRecord{};
			for (int32_t color_mode{0}; static_cast<int32_t>(CVhalColorRecordDaynight::ColorTypeDayNight::COLOR_TYPE_DAYNIGHT_MODE_MAX) > color_mode; (void)color_mode++)
			{
				/* 画質調整データの設定 */
				ret = colorRecord.SetStep(static_cast<uint32_t>(color_mode), daynight_color_init_);
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("colorRecord.SetStep error. ret=%d, color_mode=%d", ret, color_mode);
					result = VHAL_ERR;
					break;
				}
			}
			/* 画質調整テーブルの書込み */
			if (VHAL_SUCCESS == ret)
			{
				ret = Write(color_id, colorRecord);				
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("Write error. ret=%d, color_id=%d", ret, color_id);
					result = VHAL_ERR;
				}
			}
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	画質調整テーブルの書込み
 引数    ：	const uint32_t color_id						(i)ユニークID
           	const CVhalColorRecord& color_record		(i)画質調整データ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalColorDatabase::Write(const uint32_t color_id, const CVhalColorRecord& color_record)
{
	const std::lock_guard<std::mutex> lock_step{mtx_record_};

	VHAL_LOGV("color_id=%d", color_id);

	const auto iter = record_list_.find(color_id);
	if (iter == record_list_.end())
	{
		VHAL_LOGE("parameter error. unknown color_id=%d", color_id);
		return VHAL_ERR_PARAM;
	}

	/* 画質調整データのコピー */
	CVhalColorRecord::Copy(*iter->second, color_record);
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	画質調整テーブルの読込み
 引数    ：	const uint32_t color_id						(i)ユニークID
           	const CVhalColorRecord& color_record		(o)画質調整データ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalColorDatabase::Read(const uint32_t color_id, CVhalColorRecord& color_record) const
{
	const std::lock_guard<std::mutex> lock_step{mtx_record_};

	VHAL_LOGV("color_id=%d", color_id);

	const auto iter = record_list_.find(color_id);
	if (iter == record_list_.end())
	{
		VHAL_LOGE("parameter error. unknown color_id=%d", color_id);
		return VHAL_ERR_PARAM;
	}

	/* 画質調整データのコピー */
	CVhalColorRecord::Copy(color_record, *iter->second);
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	レコードリスト(record_list_)から、バックアップデータへ設定
 引数    ：	std::vector<uint8_t> &backup_data		(o)バックアップデータ           	
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalColorDatabase::SerializeBackupData(std::vector<uint8_t> &backup_data)
{
	VHAL_LOGV("record_list_.size=%lu", record_list_.size());

	backup_data.clear();
	backup_data.assign(VHAL_COLOR_BACKUP_SIZE, 0U);

	int32_t	ret{VHAL_SUCCESS};

	if (!(record_list_.empty()))
	{
		std::size_t pos{0U};
		constexpr std::size_t pos_offset{static_cast<std::size_t>(sizeof(int32_t))};

		/* 初期化情報 */
		backup_data.at(pos) = VHAL_COLOR_BACKUP_INITIALIZED;
		pos += pos_offset;

		/* 画質調整テーブルを先頭から取得 */
		for (auto itr_record = record_list_.begin(); itr_record != record_list_.end(); (void)++itr_record)
		{
			const uint32_t color_id{itr_record->first};
			const CVhalColorRecord::ColorType color_type{itr_record->second->GetColorType()};
			CVhalColorRecord::ColorImgAdjStep colorStep{};
			/* バックアップ不要のレコードリストはスキップ */
			if (false == itr_record->second->IsBackupTarget())
			{
				/* 何もしない */
			}
			else if (CVhalColorRecord::ColorType::COLOR_TYPE_THEME == color_type)
			{
				/* テーマカラー */
				for (uint32_t color_mode{0U}; static_cast<uint32_t>(CVhalColorRecordTheme::ColorTypeTheme::COLOR_TYPE_THEME_MODE_MAX) > color_mode; (void)color_mode++)
				{
					if (UINT64_MAX < pos + pos_offset * 2U)
					{
						break;
					}
					/* 画質調整データの取得 */
					colorStep = itr_record->second->GetStep(color_mode);
					VHAL_LOGV("color_id=%d, color_mode=%d, con=%02d, bri=%02d", color_id, color_mode, colorStep.contrast_, colorStep.brightness_);
					/* コントラスト */
					backup_data.at(pos) = I32ToUI8(colorStep.contrast_);
					pos += pos_offset;
					/* 明るさ */
					backup_data.at(pos) = I32ToUI8(colorStep.brightness_);
					pos += pos_offset;
				}
			}
			else
			{
				/* 昼夜モード */
				for (uint32_t color_mode{0U}; static_cast<uint32_t>(CVhalColorRecordTheme::ColorTypeTheme::COLOR_TYPE_THEME_MODE_MAX) > color_mode; (void)color_mode++)
				{
					if (UINT64_MAX < pos + pos_offset * 2U)
					{
						break;
					}

					if (static_cast<uint32_t>(CVhalColorRecordDaynight::ColorTypeDayNight::COLOR_TYPE_DAYNIGHT_MODE_MAX) > color_mode)
					{
						/* 画質調整データの取得 */
						colorStep = itr_record->second->GetStep(color_mode);
						VHAL_LOGV("color_id=%d, color_mode=%d, con=%02d, bri=%02d", color_id, color_mode, colorStep.contrast_, colorStep.brightness_);
						/* コントラスト */
						backup_data.at(pos) = I32ToUI8(colorStep.contrast_);
						pos += pos_offset;
						/* 明るさ */
						backup_data.at(pos) = I32ToUI8(colorStep.brightness_);
						pos += pos_offset;
					}
					else
					{
						/* 未使用データ（昼夜モード数より大きいテーマカラー分のサイズで確保しているため） */
						VHAL_LOGV("color_id=%d, color_mode=%d, con=%02d, bri=%02d (dummy)", color_id, color_mode, 0, 0);
						/* コントラスト */
						pos += pos_offset;
						/* 明るさ */
						pos += pos_offset;
					}
				}
			}
		}
	}
	else
	{
		VHAL_LOGE("parameter error. record_list_ empty");
		ret = VHAL_ERR_PARAM;
	}

	return ret;
}

/*****************************************************************************
 処理概要：	バックアップデータから、レコードリスト(record_list_)へ設定
 引数    ：	std::vector<uint8_t> &backup_data		(i)バックアップデータ
 戻り値  ：	処理結果
           		VHAL_ERR			異常終了
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalColorDatabase::DeserializeBackupData(std::vector<uint8_t> &backup_data)
{
	VHAL_LOGV("backup_data.size=%lu", backup_data.size());

	int32_t	result{VHAL_SUCCESS};

	if (!(backup_data.empty()))
	{
		std::size_t pos{0U};
		constexpr std::size_t pos_offset{static_cast<std::size_t>(sizeof(int32_t))};

		/* 初期化情報 */
		init_state_ = backup_data.at(pos);
		pos += pos_offset;

		/* 画質調整テーブルを先頭から取得 */
		for (auto itr_record = record_list_.begin(); itr_record != record_list_.end(); (void)++itr_record)
		{
			const uint32_t color_id{itr_record->first};
			const CVhalColorRecord::ColorType color_type{itr_record->second->GetColorType()};
			/* バックアップ不要のレコードリストはスキップ */
			if (false == itr_record->second->IsBackupTarget())
			{
				/* 何もしない */
			}
			else if (CVhalColorRecord::ColorType::COLOR_TYPE_THEME == color_type)
			{
				/* テーマカラー */
				CVhalColorRecordTheme	colorRecord{};
				int32_t	mode_ret{VHAL_SUCCESS};
				for (uint32_t color_mode{0U}; static_cast<uint32_t>(CVhalColorRecordTheme::ColorTypeTheme::COLOR_TYPE_THEME_MODE_MAX) > color_mode; (void)color_mode++)
				{
					if (UINT64_MAX < pos + pos_offset * 2U)
					{
						break;
					}
					/* コントラスト */
					const int32_t contrast{static_cast<int32_t>(backup_data.at(pos))};
					pos += pos_offset;
					/* 明るさ */
					const int32_t brightness{static_cast<int32_t>(backup_data.at(pos))};
					pos += pos_offset;
					VHAL_LOGD("color_id=%d, color_mode=%d, con=%02d, bri=%02d", color_id, color_mode, contrast, brightness);

					const CVhalColorRecord::ColorImgAdjStep	colorStep{contrast, brightness};
					/* 画質調整データの設定 */
					const int32_t ret{colorRecord.SetStep(color_mode, colorStep)};
					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("colorRecord.SetStep error. ret=%d, color_mode=%d", ret, color_mode);
						result = VHAL_ERR;
						mode_ret = VHAL_ERR;
					}
				}
				/* 画質調整テーブルの書込み */
				if (VHAL_SUCCESS == mode_ret)
				{
					const int32_t ret{Write(color_id, colorRecord)};				
					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("Write error. ret=%d, color_id:%d", ret, color_id);
						result = VHAL_ERR;
					}
				}
			}
			else
			{
				/* 昼夜モード */
				CVhalColorRecordDaynight	colorRecord{};
				int32_t	mode_ret{VHAL_SUCCESS};
				for (uint32_t color_mode{0U}; static_cast<uint32_t>(CVhalColorRecordTheme::ColorTypeTheme::COLOR_TYPE_THEME_MODE_MAX) > color_mode; (void)color_mode++)
				{
					if (UINT64_MAX < pos + pos_offset * 2U)
					{
						break;
					}

					if (static_cast<uint32_t>(CVhalColorRecordDaynight::ColorTypeDayNight::COLOR_TYPE_DAYNIGHT_MODE_MAX) > color_mode)
					{
						/* コントラスト */
						const int32_t contrast{static_cast<int32_t>(backup_data.at(pos))};
						pos += pos_offset;
						/* 明るさ */
						const int32_t brightness{static_cast<int32_t>(backup_data.at(pos))};
						pos += pos_offset;
						VHAL_LOGD("color_id=%d, color_mode=%d, con=%02d, bri=%02d", color_id, color_mode, contrast, brightness);

						const CVhalColorRecord::ColorImgAdjStep	colorStep{contrast, brightness};
						/* 画質調整データの設定 */
						const int32_t ret{colorRecord.SetStep(color_mode, colorStep)};
						if (VHAL_SUCCESS != ret)
						{
							VHAL_LOGE("colorRecord.SetStep error. ret=%d, color_mode=%d", ret, color_mode);
							result = VHAL_ERR;
							mode_ret = VHAL_ERR;
						}
					}
					else
					{
						/* 未使用データ（昼夜モード数より大きいテーマカラー分のサイズで確保しているため） */
						VHAL_LOGV("color_id=%d, color_mode=%d, con=%02d, bri=%02d (dummy)", color_id, color_mode, 0, 0);
						/* コントラスト */
						pos += pos_offset;
						/* 明るさ */
						pos += pos_offset;
					}
				}
				/* 画質調整テーブルの書込み */
				if (VHAL_SUCCESS == mode_ret)
				{
					const int32_t ret{Write(color_id, colorRecord)};				
					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("Write error. ret=%d, color_id:%d", ret, color_id);
						result = VHAL_ERR;
					}
				}
			}
		}
	}
	else
	{
		VHAL_LOGE("parameter error. backup_data empty");
		result = VHAL_ERR_PARAM;
	}

	return result;
}

} /* namespace videohal */
