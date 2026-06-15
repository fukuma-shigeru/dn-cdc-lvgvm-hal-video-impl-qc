/*******************************************************************************
    機能名称    ：  画質調整制御モジュール
    ファイル名称：  vhal_color_mng.cpp
*******************************************************************************/
#include "vhal_define.h"
#include "vhal_log.h"

#include "vhal_color_mng.h"
#include "vhal_color_db.h"
#include "vhal_color_record.h"
#include "vhal_color_record_daynight.h"
#include "vhal_color_record_theme.h"
#include "vhal_color_send_item_quality.h"
#include "vhal_color_recv_item_quality.h"
#include "vhal_color_adjust_item_quality.h"
#include "vhal_micon_comm_control.h"

#include <algorithm>

namespace videohal
{

namespace 
{

	/* DB用 */
	/* 映像ソースIDとCVhalColorDatabaseのID変換用構造体 */
	struct VsrcColorTable {
		uint32_t							color_id_;					/* ユニークID */
		CVhalColorRecord::ColorType			color_type_;				/* カラータイプ(DayNight/Theme) */
		bool								backup_target_;				/* バックアップ有無 */
	};

	using CVsrcColor_map = std::map<CVhalColorManager::VideoSourceId, struct VsrcColorTable>;
	const CVsrcColor_map gVsrcColor_table
	{
		/* 映像ソースID,                          ユニークID,															カラータイプ,										バックアップ有無*/
		{VHAL_VSRC_ID_DTV,				{static_cast<uint32_t>(CVhalColorManager::StepType::STEP_TYPE_DTV),				CVhalColorRecord::ColorType::COLOR_TYPE_DAYNIGHT,	true}	},	/* RGBDTVチューナ */
		{VHAL_VSRC_ID_OTHER,			{static_cast<uint32_t>(CVhalColorManager::StepType::STEP_TYPE_OTHER),			CVhalColorRecord::ColorType::COLOR_TYPE_THEME,		true}	},	/* 操作画面 */
		{VHAL_VSRC_ID_IMAGE_ADJUST_CAM,	{static_cast<uint32_t>(CVhalColorManager::StepType::STEP_TYPE_CAM),				CVhalColorRecord::ColorType::COLOR_TYPE_DAYNIGHT,	true}	},	/* カメラ画質描画 */
		{VHAL_VSRC_ID_HDMI,				{static_cast<uint32_t>(CVhalColorManager::StepType::STEP_TYPE_HDMI),			CVhalColorRecord::ColorType::COLOR_TYPE_DAYNIGHT,	true}	},	/* HDMI映像 */
		{VHAL_VSRC_ID_MULTISENSORY,		{static_cast<uint32_t>(CVhalColorManager::StepType::STEP_TYPE_MULTISENSORY),	CVhalColorRecord::ColorType::COLOR_TYPE_THEME,		false}	},	/* 多感覚連携 */
	};

	/* 連動設定 */
	/* <ユニークID(連動元), ユニークID(連動先)> */
	using CColorStepLink = std::map<uint32_t, uint32_t>;
	const CColorStepLink colorid_link_table
	{
		/* ユニークID(連動元),                   ユニークID(連動先),   */
		{static_cast<uint32_t>(CVhalColorManager::StepType::STEP_TYPE_DTV),			static_cast<uint32_t>(CVhalColorManager::StepType::STEP_TYPE_HDMI)},			/* DTV */
		{static_cast<uint32_t>(CVhalColorManager::StepType::STEP_TYPE_HDMI),		static_cast<uint32_t>(CVhalColorManager::StepType::STEP_TYPE_DTV)},				/* HDMI */
	};
}


/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalColorManager::CVhalColorManager(void)
	:video_source_id_(VHAL_VSRC_ID_OTHER)
	,videopath_source_id_(VHAL_VSRC_ID_OTHER)
	,day_night_(UI32ToI32(VHAL_SETTING_NIGHT))				/* 眩しいのを抑えるためフェールで夜設定 */
	,theme_color_(VHAL_THEME_COLOR_AUTO_DARK)				/* 眩しいのを抑えるためフェールで夜設定 */
	,forced_hmi_img_adj_(false)
	,forced_multisensory_img_adj_(false)
	,video_front_path_current_("")
	,camera_path_current_("")
	,p_color_database_(nullptr)
	,p_micon_comm_control_(nullptr)
	,p_color_send_item_quality_(nullptr)
	,p_color_adjust_misc_receiver_(nullptr)
{
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalColorManager::~CVhalColorManager(void)
{
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化
            CVhalMiconCommControl* p_micon_comm_control		(i)マイコン間通信制御モジュール
            CVhalColorAdjustMiscReceiver* p_color_adjust_misc_receiver	(i)画質調整MISC受信データ管理モジュール
 戻り値  ：	処理結果
           		VHAL_ERR				異常終了
           		VHAL_ERR_MEMORY			メモリ確保エラー
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalColorManager::Initialize(CVhalMiconCommControl* const p_micon_comm_control, CVhalColorAdjustMiscReceiver* const p_color_adjust_misc_receiver)
{
	int32_t	result{VHAL_SUCCESS};

	p_micon_comm_control_ = p_micon_comm_control;
	p_color_adjust_misc_receiver_ = p_color_adjust_misc_receiver;	/* p_color_adjust_misc_receiver_はNULLの場合がありえる */

	p_color_send_item_quality_ = std::make_unique<CVhalColorSendItemQuality>();
	if (nullptr == p_color_send_item_quality_)
	{
		VHAL_LOGE("new CVhalColorSendItemQuality error.");
		result = VHAL_ERR_MEMORY;
	}
	else
	{
		p_color_database_ = std::make_unique<CVhalColorDatabase>();
		if (nullptr == p_color_database_)
		{
			VHAL_LOGE("new CVhalColorDatabase error.");
			result = VHAL_ERR_MEMORY;
		}
		else
		{
			int32_t ret{p_color_database_->Initialize()};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("p_color_database_->Initialize error. ret=%d", ret);
				result = VHAL_ERR;
			}
			else
			{
				/* 画質調整テーブルの作成 */
				ret = CreateColorStep();
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("CreateColorStep error. ret=%d", ret);
					result = VHAL_ERR;
				}
				else
				{
					/* 画質調整設定読込み */
					ret = LoadColorStep();
					if (VHAL_SUCCESS != ret)
					{
						/* 画質調整設定読込み（データロード）に失敗しているが、 */
						/* 他VideoHAL機能は動作可能なため、NGリターンしない（プロセス終了させない）。 */
						VHAL_LOGE("LoadColorStep error. ret=%d", ret);
					}

					/* MISC通信用 */
					/* 画質モード通知MISCコマンドインターバル時間(ms) */
					constexpr uint32_t kMiscCtrlApiCmdInterval_colorMode{3000U};

					/* 画質モード通知定期送信データ作成 */
					ret = p_micon_comm_control_->CreateIntervalTimer(p_color_send_item_quality_.get(), kMiscCtrlApiCmdInterval_colorMode);
					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("CreateIntervalTimer error. ret=%d", ret);
						result = VHAL_ERR;
					}

					/* BEVS3CDC-20629対応 */
					/* 画質モード通知定期送信データ送信（初期値） */
					/* 画質調整未設定区間（MUTE解除～初回送信トリガーまで）を無くすため */
					/* 引数=false : データ更新チェックなし */
					ret = ApplyAdjustment(false);
					if (VHAL_SUCCESS != ret)
					{
						/* 画質モード通知送信に失敗しているが、 */
						/* 他VideoHAL機能は動作可能なため、NGリターンしない（プロセス終了させない）。 */
						VHAL_LOGE("ApplyAdjustment error. ret=%d", ret);
					}
				}
			}
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	終了処理
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorManager::Finalize(void)
{
	VHAL_LOGD("CVhalColorManager.");

	/* 画質モード通知定期送信終了 */
	if (nullptr != p_micon_comm_control_)
	{
		const int32_t ret{p_micon_comm_control_->SendIntervalEnd(p_color_send_item_quality_.get())};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SendIntervalEnd error. ret=%d", ret);
		}

		p_micon_comm_control_ = nullptr;
	}

	if (nullptr != p_color_database_)
	{
		/* 画質調整テーブルの削除 */
		for (const auto& tbl : gVsrcColor_table)
		{
			p_color_database_->DeleteColorRecord(tbl.second.color_id_);
		}

		p_color_database_->Finalize();
		p_color_database_ = nullptr;
	}

	if (nullptr != p_color_send_item_quality_)
	{
		p_color_send_item_quality_ = nullptr;
	}
}

/*****************************************************************************
 処理概要：	再初期化処理
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorManager::ReInit(void) noexcept
{
	/* color_db関連データの初期化はしない。継続して使用する */

	/* ReInitではDB参照を行わず、コンストラクタ相当の既定値を直接反映する */
	video_source_id_ = VHAL_VSRC_ID_OTHER;
	videopath_source_id_ = VHAL_VSRC_ID_OTHER;
	day_night_ = ToI32(VHAL_SETTING_NIGHT);
	theme_color_ = VHAL_THEME_COLOR_AUTO_DARK;
	forced_hmi_img_adj_ = false;
	forced_multisensory_img_adj_ = false;
	video_front_path_current_.clear();
	camera_path_current_.clear();

	if (nullptr != p_color_send_item_quality_)
	{
		p_color_send_item_quality_->ReInit();
		p_color_send_item_quality_->SetDaynightType(day_night_);
		p_color_send_item_quality_->SetThemeColorType(theme_color_);
		p_color_send_item_quality_->SetStepContrast(kColorImgAdjStepInitContrast, VHAL_VSRC_ID_OTHER);
		p_color_send_item_quality_->SetStepBrightness(kColorImgAdjStepInitBrightness, VHAL_VSRC_ID_OTHER);
		p_color_send_item_quality_->SetColorType(GetColorType(VHAL_VSRC_ID_OTHER), VHAL_VSRC_ID_OTHER);
		p_color_send_item_quality_->SetCameraPath("");
		p_color_send_item_quality_->SetFrontPath("");
		p_color_send_item_quality_->SetForceMultisensoryEnable(false);
		p_color_send_item_quality_->SetForceHmiEnable(false);
		p_color_send_item_quality_->SetBackLight(true);
		p_color_send_item_quality_->SetMuteFrontDisp(true);
	}
	if (nullptr != p_color_adjust_misc_receiver_)
	{
		p_color_adjust_misc_receiver_->SetCurrentBackLight(true);
		p_color_adjust_misc_receiver_->SetCurrentMuteFrontDisp(true);
	}
}

/*****************************************************************************
 処理概要：	画質調整テーブルの作成
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR				異常終了
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalColorManager::CreateColorStep(void)
{
	int32_t	ret{VHAL_SUCCESS};
	int32_t	result{VHAL_SUCCESS};

	/* 画質調整テーブルの追加用に重複したユニークIDを除く */
	std::vector<struct VsrcColorTable> color_table{};
	for (const auto& tbl : gVsrcColor_table)
	{
		color_table.push_back(tbl.second);
	}
	std::sort(color_table.begin(), color_table.end(), [](const VsrcColorTable& a, const VsrcColorTable& b) noexcept
	{
		return a.color_id_ < b.color_id_;
	});
	(void)color_table.erase(std::unique(color_table.begin(), color_table.end(), [](const VsrcColorTable& a, const VsrcColorTable& b) noexcept
	{
		return a.color_id_ == b.color_id_;
	}), color_table.end());
	/* 画質調整テーブルの追加 */
	constexpr CVhalColorRecord::ColorImgAdjStep color_init{kColorImgAdjStepInitContrast, kColorImgAdjStepInitBrightness};
	for (const auto& tbl : color_table)
	{
		ret = p_color_database_->CreateColorRecord(tbl.color_id_, tbl.color_type_, color_init, tbl.backup_target_);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("p_color_database_->CreateColorRecord error. ret=%d, color_id=%d, color_type=%d", ret, tbl.color_id_, tbl.color_type_);
			break;
		}
	}
	/* 画質調整テーブルの追加処理でエラーが発生した場合は初期化処理を終了する */
	if (VHAL_SUCCESS != ret)
	{
		result = VHAL_ERR;
	}

	return result;
}

/*****************************************************************************
 処理概要：	画質調整設定読込み
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR				異常終了
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalColorManager::LoadColorStep(void)
{
	int32_t	ret{VHAL_SUCCESS};

	/* 	画質調整テーブルのリストア */
	ret = p_color_database_->LoadColorRecord();
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("p_color_database_->LoadColorRecord error. ret=%d", ret);
	}

	/* HMI用送信データの初期化 */

	/* カラーDBからStep値を取得 */
	/* 現在の映像ソースIDのSTEP設定を取得 */
	CVhalColorRecord::ColorImgAdjStep	colorStep{};
	ret = GetCurrentColorStep(colorStep, videopath_source_id_);
	if (VHAL_SUCCESS == ret)
	{
		/* 送信データ更新 */
		p_color_send_item_quality_->SetStepContrast(colorStep.contrast_, videopath_source_id_);
		p_color_send_item_quality_->SetStepBrightness(colorStep.brightness_, videopath_source_id_);
		/* カラータイプ(DayNight/Theme)を取得 */
		const CVhalColorRecord::ColorType color_type{GetColorType(video_source_id_)};
		p_color_send_item_quality_->SetColorType(color_type, videopath_source_id_);
	}
	else
	{
		VHAL_LOGE("GetCurrentColorStep error. ret=%d", ret);
		ret = VHAL_ERR;
	}

	return ret;
}

/*****************************************************************************
 処理概要：	映像ソースID設定
 引数    ：	VideoSourceId vsrcid			(i)映像ソースID
 戻り値  ：	処理結果
           		VHAL_ERR				異常終了
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalColorManager::SetVideoSource(const VideoSourceId vsrcid)
{
	int32_t result{VHAL_SUCCESS};
	do{
		VHAL_LOGV("vsrcid=0x%x", vsrcid);

		/* 引数チェック */
		const auto iter = gVsrcColor_table.find(vsrcid);
		if (iter == gVsrcColor_table.end())
		{
			VHAL_LOGE("parameter error. vsrcid=0x%x", vsrcid);
			result = VHAL_ERR_PARAM;
			break;
		}

		video_source_id_ = vsrcid;

		/* カラーDBからStep値を取得 */
		/* 現在の映像ソースIDのSTEP設定を取得 */
		CVhalColorRecord::ColorImgAdjStep	colorStep{};
		const int32_t ret{GetCurrentColorStep(colorStep, vsrcid)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetCurrentColorStep error. ret=%d vsrcid=0x%x", ret, vsrcid);
			result = VHAL_ERR;
			break;
		}

		/* 送信データ更新 */
		p_color_send_item_quality_->SetStepContrast(colorStep.contrast_, vsrcid);
		p_color_send_item_quality_->SetStepBrightness(colorStep.brightness_, vsrcid);
		/* カラータイプ(DayNight/Theme)を取得 */
		const CVhalColorRecord::ColorType color_type{GetColorType(vsrcid)};
		p_color_send_item_quality_->SetColorType(color_type, vsrcid);
	} while(false);

	return result;
}

/*****************************************************************************
 処理概要：	昼夜モード種別設定
 引数    ：	DayNightType day_night			(i)昼夜モード種別
 戻り値  ：	処理結果
           		VHAL_ERR				異常終了
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalColorManager::SetDayNight(const DayNightType day_night)
{
	int32_t result{VHAL_SUCCESS};
	do{
		VHAL_LOGV("day_night=%d", day_night);

		/* 引数チェック */
		if (UI32ToI32(VHAL_SETTING_FORCED_DAY) < day_night)
		{
			VHAL_LOGE("parameter error. day_night=%d", day_night);
			result = VHAL_ERR_PARAM;
			break;
		}

		day_night_ = day_night;

		/* カラーDBからStep値を取得 */
		/* 映像パスに紐づく映像ソースIDを使用（即時反映のため） */
		CVhalColorRecord::ColorImgAdjStep colorStep{};
		const int32_t ret{GetCurrentColorStep(colorStep, videopath_source_id_)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetCurrentColorStep error. ret=%d day_night=%d", ret, day_night);
			result = VHAL_ERR;
			break;
		}

		/* 送信データ更新 */
		p_color_send_item_quality_->SetDaynightType(day_night);
		p_color_send_item_quality_->SetStepContrast(colorStep.contrast_, videopath_source_id_);
		p_color_send_item_quality_->SetStepBrightness(colorStep.brightness_, videopath_source_id_);
	} while(false);

	return result;
}

/*****************************************************************************
 処理概要：	テーマカラー種別設定
 引数    ：	ThemeColorType theme_color			(i)テーマカラー種別
 戻り値  ：	処理結果
           		VHAL_ERR				異常終了
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalColorManager::SetThemeColor(const ThemeColorType theme_color)
{
	int32_t result{VHAL_SUCCESS};

	VHAL_LOGV("theme_color=%d", theme_color);

	/* 引数チェック */
	if (VHAL_THEME_COLOR_FORCED_LIGHT < theme_color)
	{
		VHAL_LOGE("parameter error. theme_color=%d", theme_color);
		result = VHAL_ERR_PARAM;
	}
	else
	{
		theme_color_ = theme_color;
		p_color_send_item_quality_->SetThemeColorType(theme_color);

		/* 不具合対応による24MMとのコード乖離箇所 */
		/* 詳細はPD01_ソフトウェア詳細設計書_Video-HAL_～.xlsx
		   画質調整(テーマカラー切替)：シートを参照*/

		/* テーマカラー用映像ソースIDを収集 */
		std::vector<VideoSourceId> theme_vsrcids{};
		for (const auto& tbl : gVsrcColor_table)
		{
			if (CVhalColorRecord::ColorType::COLOR_TYPE_THEME == tbl.second.color_type_)
			{
				VHAL_LOGV("set theme_vsrcids[%zu] = %u", theme_vsrcids.size(), tbl.first);
				theme_vsrcids.push_back(tbl.first);
			}
		}

		/* 収集したテーマカラー用映像ソースID数分ループ */
		for (const auto& vsrcid : theme_vsrcids)
		{
			CVhalColorRecord::ColorImgAdjStep colorStep{};
			const int32_t ret{GetCurrentColorStep(colorStep, vsrcid)};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("GetCurrentColorStep error. ret=%d theme_color=%d", ret, theme_color);
				result = VHAL_ERR;
			}
			else
			{
				VHAL_LOGV("vsrcid=%u colorStep.contrast=%d colorStep.brightness=%d", vsrcid, colorStep.contrast_, colorStep.brightness_);
				/* 送信データ更新 */
				p_color_send_item_quality_->SetStepContrast(colorStep.contrast_, vsrcid);
				p_color_send_item_quality_->SetStepBrightness(colorStep.brightness_, vsrcid);
			}
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	バックライト点灯/消灯設定
 引数    ：	bool light			(i)バックライト点灯状態
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalColorManager::SetBackLight(const bool light)
{
	VHAL_LOGV("light=%d", light);

	if (nullptr != p_color_send_item_quality_)
	{
		/* 送信データ更新 */
		p_color_send_item_quality_->SetBackLight(light);
	}
	if (nullptr != p_color_adjust_misc_receiver_)
	{
		/* 応答時内部データ参照用 */
		p_color_adjust_misc_receiver_->SetCurrentBackLight(light);
	}

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	MuteON/解除設定
 引数    ：	bool mute			(i)Mute状態
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalColorManager::SetMuteFrontDisp(const bool mute)
{
	VHAL_LOGV("mute=%d", mute);

	if (nullptr != p_color_send_item_quality_)
	{
		/* 送信データ更新 */
		p_color_send_item_quality_->SetMuteFrontDisp(mute);
	}
	if (nullptr != p_color_adjust_misc_receiver_)
	{
		/* 応答時内部データ参照用 */
		p_color_adjust_misc_receiver_->SetCurrentMuteFrontDisp(mute);
	}

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	現在の映像ソースIDに対してコントラストを設定
 引数    ：	int32_t step			(i)STEP値
 戻り値  ：	処理結果
           		VHAL_ERR				異常終了
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalColorManager::SetCurrentColorStepContrast(const int32_t step)
{
	int32_t result{VHAL_SUCCESS};
	do{
		VHAL_LOGV("step=%d", step);

		/* 引数チェック */
		if ((static_cast<int32_t>(ColorStepNum::COLOR_STEP_NUM_MIN) > step) || (static_cast<int32_t>(ColorStepNum::COLOR_STEP_NUM_MAX) < step))
		{
			VHAL_LOGE("parameter error. step=%d", step);
			result = VHAL_ERR_PARAM;
			break;
		}

		/* カラーDB更新 */
		/* 現在の映像ソースIDのSTEP設定を取得 */
		CVhalColorRecord::ColorImgAdjStep	colorStep{};
		int32_t ret{GetCurrentColorStep(colorStep, video_source_id_)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetCurrentColorStep error. ret=%d step=%d", ret, step);
			result = VHAL_ERR;
			break;
		}

		/* 設定値を更新 */
		VHAL_LOGV("val=%d -> %d", colorStep.contrast_, step);
		colorStep.contrast_ = step;

		/* 最新の映像ソースIDのSTEP設定を設定 */
		ret = SetCurrentColorStep(colorStep, video_source_id_);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetCurrentColorStep error. ret=%d step=%d", ret, step);
			result = VHAL_ERR;
			break;
		}

		/* 画質調整テーブルのバックアップ */
		if (VHAL_VSRC_ID_MULTISENSORY != video_source_id_)
		{
			ret = p_color_database_->SaveColorRecord();
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("SaveColorRecord error. ret=%d step=%d", ret, step);
				result = VHAL_ERR;
				break;
			}
		}

		/* 送信データ更新 */
		p_color_send_item_quality_->SetStepContrast(colorStep.contrast_, video_source_id_);
	} while(false);

	return result;
}

/*****************************************************************************
 処理概要：	現在の映像ソースIDに対して明るさを設定
 引数    ：	int32_t step			(i)STEP値
 戻り値  ：	処理結果
           		VHAL_ERR			異常終了
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalColorManager::SetCurrentColorStepBrightness(const int32_t step)
{
	int32_t result{VHAL_SUCCESS};
	do{
		VHAL_LOGV("step=%d", step);

		/* 引数チェック */
		if ((static_cast<int32_t>(ColorStepNum::COLOR_STEP_NUM_MIN) > step) || (static_cast<int32_t>(ColorStepNum::COLOR_STEP_NUM_MAX) < step))
		{
			VHAL_LOGE("parameter error. step=%d", step);
			result = VHAL_ERR_PARAM;
			break;
		}

		/* カラーDB更新 */
		/* 現在の映像ソースIDのSTEP設定を取得 */
		CVhalColorRecord::ColorImgAdjStep	colorStep{};
		int32_t ret{GetCurrentColorStep(colorStep, video_source_id_)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetCurrentColorStep error. ret=%d step=%d", ret, step);
			result = VHAL_ERR;
			break;
		}

		/* 設定値を更新 */
		VHAL_LOGV("val=%d -> %d", colorStep.brightness_, step);
		colorStep.brightness_ = step;

		/* 最新の映像ソースIDのSTEP設定を設定 */
		ret = SetCurrentColorStep(colorStep, video_source_id_);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetCurrentColorStep error. ret=%d step=%d", ret, step);
			result = VHAL_ERR;
			break;
		}

		/* 画質調整テーブルのバックアップ */
		if (VHAL_VSRC_ID_MULTISENSORY != video_source_id_)
		{
			ret = p_color_database_->SaveColorRecord();
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("SaveColorRecord error. ret=%d step=%d", ret, step);
				result = VHAL_ERR;
				break;
			}
		}

		/* 送信データ更新 */
		p_color_send_item_quality_->SetStepBrightness(colorStep.brightness_, video_source_id_);
	} while(false);

	return result;
}

/*****************************************************************************
 処理概要：	前席映像パスの設定
 引数    ：	const std::string&	path	(i)vpath文字列
 戻り値  ：	処理結果
           		false		設定不可（カメラ映像パスを優先）
           		true		設定可
*****************************************************************************/
bool CVhalColorManager::SetFrontPath(const std::string& path)
{
	bool set_path{false};
	video_front_path_current_ = path;

	/* カメラ映像パスを優先 */
	if (camera_path_current_.empty())
	{
		/* 映像パスに紐づく映像ソースIDの取得 */
		videopath_source_id_ = p_color_send_item_quality_->GetVideoPathSourceId(path);

		/* カラーDBからStep値を取得 */
		/* 現在の映像ソースIDのSTEP設定を取得 */
		CVhalColorRecord::ColorImgAdjStep	colorStep{};
		const int32_t ret{GetCurrentColorStep(colorStep, videopath_source_id_)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetCurrentColorStep error. ret=%d", ret);
		}
		else
		{
			/* カラータイプ(DayNight/Theme)を取得 */
			const CVhalColorRecord::ColorType color_type{GetColorType(videopath_source_id_)};

			/* 送信データ更新 */
			p_color_send_item_quality_->SetFrontPath(path);
			p_color_send_item_quality_->SetStepContrast(colorStep.contrast_, videopath_source_id_);
			p_color_send_item_quality_->SetStepBrightness(colorStep.brightness_, videopath_source_id_);
			p_color_send_item_quality_->SetColorType(color_type, videopath_source_id_);
			set_path = true;
		}
	}

	return set_path;
}

/*****************************************************************************
 処理概要：	カメラ映像パスの設定
 引数    ：	const std::string&	path	(i)vpath文字列
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorManager::SetCameraPath(const std::string& path)
{
	camera_path_current_ = path;

	if (camera_path_current_.empty() && (!video_front_path_current_.empty()))
	{
		/* 前席映像パスを再設定（カメラ映像パス優先の終了） */
		/* 送信データ更新 */
		p_color_send_item_quality_->SetCameraPath(path);
		/* 前席映像パスの設定 */
		(void)SetFrontPath(video_front_path_current_);
	}
	else
	{
		/* 映像パスに紐づく映像ソースIDの取得 */
		videopath_source_id_ = p_color_send_item_quality_->GetVideoPathSourceId(path);

		/* カラーDBからStep値を取得 */
		/* 現在の映像ソースIDのSTEP設定を取得 */
		CVhalColorRecord::ColorImgAdjStep	colorStep{};
		const int32_t ret{GetCurrentColorStep(colorStep, videopath_source_id_)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetCurrentColorStep error. ret=%d", ret);
		}
		else
		{
			/* カラータイプ(DayNight/Theme)を取得 */
			const CVhalColorRecord::ColorType color_type{GetColorType(videopath_source_id_)};

			/* 送信データ更新 */
			p_color_send_item_quality_->SetCameraPath(path);
			p_color_send_item_quality_->SetStepContrast(colorStep.contrast_, videopath_source_id_);
			p_color_send_item_quality_->SetStepBrightness(colorStep.brightness_, videopath_source_id_);
			p_color_send_item_quality_->SetColorType(color_type, videopath_source_id_);
		}
	}
}

/*****************************************************************************
 処理概要：	強制HMI画質適用設定
 引数    ：	const bool			force_hmi_enable	(i)強制HMI有効フラグ
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorManager::SetForceHmiEnable(const bool force_hmi_enable) noexcept
{
	forced_hmi_img_adj_ = force_hmi_enable;
	/* 送信データ更新 */
	p_color_send_item_quality_->SetForceHmiEnable(force_hmi_enable);
}

/*****************************************************************************
 処理概要：	強制多感覚連携画質適用設定
 引数    ：	const bool			force_multisensory_enable	(i)強制多感覚連携有効フラグ
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorManager::SetForceMultisensoryEnable(const bool force_multisensory_enable) noexcept
{
	forced_multisensory_img_adj_ = force_multisensory_enable;
	/* 送信データ更新 */
	p_color_send_item_quality_->SetForceMultisensoryEnable(force_multisensory_enable);
}

/*****************************************************************************
 処理概要：	ディスプレイ出力矩形を設定
 引数    ：	const int32_t		x					(i)x座標
           	const int32_t		y					(i)y座標
           	const int32_t		w					(i)幅
           	const int32_t		h					(i)高さ
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorManager::SetDisplayRectangle(const int32_t x, const int32_t y, const int32_t w, const int32_t h) noexcept
{
	/* 送信データ更新 */
	p_color_send_item_quality_->SetDisplayRectangle(x, y, w, h);
}

/*****************************************************************************
 処理概要：	ヒーコン矩形を設定
 引数    ：	const int32_t		x					(i)x座標
           	const int32_t		y					(i)y座標
           	const int32_t		w					(i)幅
           	const int32_t		h					(i)高さ
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorManager::SetHeaconRectangle(const int32_t x, const int32_t y, const int32_t w, const int32_t h) noexcept
{
	/* 送信データ更新 */
	p_color_send_item_quality_->SetHeaconRectangle(x, y, w, h);
}

/*****************************************************************************
 処理概要：	画質調整テーブルのクリア（デフォルトデータ）
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR				異常終了
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalColorManager::ClearColorStep(void)
{
	int32_t	ret{VHAL_SUCCESS};

	/* 画質調整テーブルのクリア */
	ret = p_color_database_->ClearColorRecord();
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("ClearColorRecord error. ret=%d", ret);
		ret = VHAL_ERR;
	}
	else
	{
		/* 画質調整テーブルのバックアップ */
		ret = p_color_database_->SaveColorRecord();
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SaveColorRecord error. ret=%d", ret);
			ret = VHAL_ERR;
		}
		else
		{
			/* 現在の映像ソースIDのSTEP設定を取得 */
			CVhalColorRecord::ColorImgAdjStep	colorStep{};
			ret = GetCurrentColorStep(colorStep, videopath_source_id_);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("GetCurrentColorStep error. ret=%d", ret);
				ret = VHAL_ERR;
			}
			else
			{
				/* 送信データ更新 */
				p_color_send_item_quality_->SetStepContrast(colorStep.contrast_, videopath_source_id_);
				p_color_send_item_quality_->SetStepBrightness(colorStep.brightness_, videopath_source_id_);

				ret = ApplyAdjustment(false);
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("ApplyAdjustment error. ret=%d", ret);
					ret = VHAL_ERR;
				}
			}
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	CVhalColorDatabase用のユニークIDを取得
 引数    ：	const VideoSourceId vsrcid			(i)映像ソースID
 戻り値  ：	CVhalColorDatabase用のユニークID
*****************************************************************************/
uint32_t CVhalColorManager::GetColorId(const VideoSourceId vsrcid) const
{
	uint32_t color_id{static_cast<uint32_t>(CVhalColorManager::StepType::STEP_TYPE_OTHER)};

	/* 映像ソースIDからユニークIDを取得 */
	const auto iter = gVsrcColor_table.find(vsrcid);
	if (iter != gVsrcColor_table.end())
	{
		color_id = iter->second.color_id_;
	}

	VHAL_LOGV("vsrcid=0x%x, color_id=%d", vsrcid, color_id);
	return color_id;
}

/*****************************************************************************
 処理概要：	CVhalColorDatabase用のカラータイプ(DayNight/Theme)を取得
 引数    ：	const VideoSourceId vsrcid			(i)映像ソースID
 戻り値  ：	CVhalColorDatabase用のカラータイプ(DayNight/Theme)
*****************************************************************************/
CVhalColorRecord::ColorType CVhalColorManager::GetColorType(const VideoSourceId vsrcid) const
{
	CVhalColorRecord::ColorType color_type{CVhalColorRecord::ColorType::COLOR_TYPE_DAYNIGHT};

	/* 映像ソースIDからカラータイプを取得 */
	const auto iter = gVsrcColor_table.find(vsrcid);
	if (iter != gVsrcColor_table.end())
	{
		color_type = iter->second.color_type_;
	}

	VHAL_LOGV("vsrcid=0x%x, color_type=%d", vsrcid, color_type);
	return color_type;
}

/*****************************************************************************
 処理概要：	現在の映像ソースIDのSTEP設定を取得
 引数    ：	CVhalColorRecord::ColorImgAdjStep& colorStep		(o)画質調整データ
			const VideoSourceId vsrcid							(i)映像ソースID
 戻り値  ：	処理結果
           		VHAL_ERR			異常終了
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalColorManager::GetCurrentColorStep(CVhalColorRecord::ColorImgAdjStep& colorStep, const VideoSourceId vsrcid) const
{
	int32_t result{VHAL_SUCCESS};
	do{
		/* CVhalColorRecord用のカラータイプ(DayNight/Theme)を取得 */
		const CVhalColorRecord::ColorType color_type{GetColorType(vsrcid)};

		if (CVhalColorRecord::ColorType::COLOR_TYPE_DAYNIGHT == color_type)
		{
			/* DayNight */
			CVhalColorRecordDaynight	ColorRecordDaynight{};
			colorStep = ReadCurrentColorStep(ColorRecordDaynight, I32ToUI32(day_night_), vsrcid);
		}
		else if (CVhalColorRecord::ColorType::COLOR_TYPE_THEME == color_type)
		{
			/* Theme */
			CVhalColorRecordTheme	ColorRecordTheme{};
			colorStep = ReadCurrentColorStep(ColorRecordTheme, I32ToUI32(theme_color_), vsrcid);
		}
		else
		{
			VHAL_LOGE("unknown color_type=%d", color_type);
			result = VHAL_ERR;
			break;
		}
	} while(false);

	return result;
}

/*****************************************************************************
 処理概要：	最新の映像ソースIDのSTEP設定を設定
 引数    ：	const CVhalColorRecord::ColorImgAdjStep& colorStep		(i)画質調整データ
			const VideoSourceId vsrcid								(i)映像ソースID
 戻り値  ：	処理結果
           		VHAL_ERR			異常終了
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalColorManager::SetCurrentColorStep(const CVhalColorRecord::ColorImgAdjStep& colorStep, const VideoSourceId vsrcid)
{
	int32_t result{VHAL_SUCCESS};
	do{
		VHAL_LOGV("contrast=%d, brightness=%d, vsrcid=0x%x", colorStep.contrast_, colorStep.brightness_, vsrcid);

		/* CVhalColorDatabase用のカラータイプ(DayNight/Theme)を取得 */
		const CVhalColorRecord::ColorType color_type{GetColorType(vsrcid)};

		if (CVhalColorRecord::ColorType::COLOR_TYPE_DAYNIGHT == color_type)
		{
			/* DayNight */
			CVhalColorRecordDaynight	ColorRecordDaynight{};
			(void)ReadCurrentColorStep(ColorRecordDaynight, I32ToUI32(day_night_), vsrcid);
			const int32_t ret{WriteCurrentColorStep(ColorRecordDaynight, I32ToUI32(day_night_), colorStep, vsrcid)};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("Write error. ret=%d", ret);
				result = VHAL_ERR;
				break;
			}
		}
		else if (CVhalColorRecord::ColorType::COLOR_TYPE_THEME == color_type)
		{
			/* Theme */
			CVhalColorRecordTheme	ColorRecordTheme{};
			(void)ReadCurrentColorStep(ColorRecordTheme, I32ToUI32(theme_color_), vsrcid);
			const int32_t ret{WriteCurrentColorStep(ColorRecordTheme, I32ToUI32(theme_color_), colorStep, vsrcid)};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("Write error. ret=%d", ret);
				result = VHAL_ERR;
				break;
			}
		}
		else
		{
			VHAL_LOGE("unknown color_type=%d", color_type);
			result = VHAL_ERR;
			break;
		}
	} while(false);

	return result;
}

/*****************************************************************************
 処理概要：	現在の映像ソースIDのSTEP設定を読込み
 引数    ：	CVhalColorRecord &ColorRecord		(o)画質調整レコードモジュール
           	const uint32_t color_mode			(i)カラーモード
			const VideoSourceId vsrcid			(i)映像ソースID
 戻り値  ：	画質調整データ
*****************************************************************************/
CVhalColorRecord::ColorImgAdjStep CVhalColorManager::ReadCurrentColorStep(CVhalColorRecord &ColorRecord, const uint32_t color_mode, const VideoSourceId vsrcid) const
{
	CVhalColorRecord::ColorImgAdjStep colorStep{};
	do{
		VHAL_LOGV("color_mode=%d, vsrcid=0x%x", color_mode, vsrcid);

		/* CVhalColorDatabase用のユニークIDを取得 */
		const uint32_t color_id{GetColorId(vsrcid)};

		/* 画質調整テーブルの読込み */
		const int32_t ret{p_color_database_->Read(color_id, ColorRecord)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("Read error. ret=%d, color_id:%d", ret, color_id);
			break;
		}

		/* 画質調整データの取得 */
		colorStep = ColorRecord.GetStep(color_mode);
	} while(false);

	return colorStep;
}

/*****************************************************************************
 処理概要：	現在の映像ソースIDのSTEP設定を書込み
 引数    ：	CVhalColorRecord &ColorRecord							(i)画質調整レコードモジュール
           	const uint32_t color_mode								(i)カラーモード
           	const CVhalColorRecord::ColorImgAdjStep& colorStep		(i)画質調整データ
			const VideoSourceId vsrcid								(i)映像ソースID
 戻り値  ：	処理結果
           		VHAL_ERR			異常終了
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalColorManager::WriteCurrentColorStep(CVhalColorRecord &ColorRecord, const uint32_t color_mode, const CVhalColorRecord::ColorImgAdjStep& colorStep, const VideoSourceId vsrcid)
{
	int32_t result{VHAL_SUCCESS};
	do{
		VHAL_LOGV("color_mode=%d, vsrcid=0x%x", color_mode, vsrcid);

		/* 画質調整データの設定 */
		int32_t ret{ColorRecord.SetStep(color_mode, colorStep)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("ColorRecord.SetStep error. ret=%d", ret);
			result = VHAL_ERR;
			break;
		}

		/* CVhalColorDatabase用のユニークIDを取得 */
		const uint32_t color_id{GetColorId(vsrcid)};
		/* 画質調整テーブルの書込み */
		ret = p_color_database_->Write(color_id, ColorRecord);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("Write error. ret=%d, color_id:%d", ret, color_id);
			result = VHAL_ERR;
			break;
		}

		/* 連動設定 */
		const auto iter = colorid_link_table.find(color_id);
		if (iter != colorid_link_table.end())
		{
			/* 画質調整テーブルの書込み */
			VHAL_LOGV("Link color_id:%d -> %d, contrast=%d, brightness=%d", color_id, iter->second, colorStep.contrast_, colorStep.brightness_);
			ret = p_color_database_->Write(iter->second, ColorRecord);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("Link Write error. ret=%d, color_id:%d", ret, iter->second);
				result = VHAL_ERR;
				break;
			}
		}
	} while(false);

	return result;
}

/*****************************************************************************
 処理概要：	画質モード通知データを更新して、通知する。
 引数    ：	const bool check_update			(i)データ更新チェック
 戻り値  ：	処理結果
           		VHAL_ERR			異常終了
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalColorManager::ApplyAdjustment(const bool check_update)
{
	int32_t result{VHAL_SUCCESS};
	
	do{
		/* データ更新チェック */
		if (true == check_update)
		{
			/* データ更新条件 */
			/*  画質調整設定（更新対象映像）と、表示中映像（TAB2小窓用送信対象）が一致 */
			/*  画質調整設定（更新対象映像）がHMI(OTHER)の場合 */
			const uint8_t vtype{p_color_send_item_quality_->GetVideoType(video_source_id_)};
			const uint8_t vpath_vtype{p_color_send_item_quality_->GetVideoType(videopath_source_id_)};
			if ((vtype != vpath_vtype) && (static_cast<uint8_t>(CVhalColorAdjustItemQuality::VideoType::VTYPE_OTHER) != vtype))
			{
				/* データ更新不要（エラーではない） */
				/* 画質調整設定（更新対象映像）がHMI(OTHER)でなく、画質調整設定（更新対象映像）と、表示中映像（TAB2小窓用送信対象）が不一致 */
				VHAL_LOGD("vtype=%d(id=%d) != vpath_vtype=%d(id=%d)", vtype, video_source_id_, vpath_vtype, videopath_source_id_);
				break;
			}
		}

		/* TAB2コマンド送信 */
		/* 画質モード通知定期送信停止 */
		int32_t ret{p_micon_comm_control_->SendIntervalStop(p_color_send_item_quality_.get())};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGEW("SendIntervalStop error. ret=%d", ret);
			result = VHAL_ERR;
		}
		else
		{
			/* 画質モード通知データ更新 */
			p_color_send_item_quality_->ApplyAdjustment();
		
			/* 画質モード通知定期送信再開 */
			ret = p_micon_comm_control_->SendIntervalRestart(p_color_send_item_quality_.get());
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGEW("SendIntervalRestart error. ret=%d", ret);
				result = VHAL_ERR;
			}
		}
	} while(false);
	
	return result;
}

/*****************************************************************************
 処理概要：	現在の映像ソースIDの取得
 引数    ：	なし
 戻り値  ：	映像ソースID
*****************************************************************************/
CVhalColorManager::VideoSourceId CVhalColorManager::GetVideoSourceId(void) const noexcept
{
	return video_source_id_;
}

/*****************************************************************************
 処理概要：	映像パスに紐づく映像ソースIDのカラータイプ(DayNight/Theme)を取得
 引数    ：	なし
 戻り値  ：	カラータイプ(DayNight/Theme)
*****************************************************************************/
CVhalColorRecord::ColorType CVhalColorManager::GetVideoPathColorType(void) const noexcept
{
	return GetColorType(videopath_source_id_);
}

/*****************************************************************************
 処理概要：	画質モード通知送信状態取得
 引数    ：	なし
 戻り値  ：	処理結果
           		true			未送信
           		false			送信開始済み
*****************************************************************************/
bool CVhalColorManager::IsFirstSend(void) const noexcept
{
	return p_color_send_item_quality_->IsSendDataEmpty();
}

} /* namespace videohal */

