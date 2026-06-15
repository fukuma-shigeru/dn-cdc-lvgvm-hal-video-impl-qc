/*******************************************************************************
    機能名称    ：  MISC画質モード通知送信アイテムモジュール
    ファイル名称：  vhal_color_send_item_quality.cpp
*******************************************************************************/
#include "vhal_define.h"
#include "vhal_log.h"

#include "vhal_color_send_item_quality.h"
#include "vhal_color_adjust_item_quality.h"
#include "vhal_color_mng.h"

#include <vector>

namespace videohal
{

namespace 
{
	/* 映像ソースIDからMISC映像種別への変換用 */
	using CVsrcColorSendItem_map = std::map<CVhalColorManager::VideoSourceId, CVhalColorAdjustItemQuality::VideoType>;
	const CVsrcColorSendItem_map gVsrcColorSendItem_table
	{
		/* 映像ソースID,                         映像種別 */
		{VHAL_VSRC_ID_DTV,						CVhalColorAdjustItemQuality::VideoType::VTYPE_DTV	},				/* RGBDTVチューナ */
		{VHAL_VSRC_ID_OTHER,					CVhalColorAdjustItemQuality::VideoType::VTYPE_OTHER	},				/* 操作画面 */
		{VHAL_VSRC_ID_IMAGE_ADJUST_CAM,			CVhalColorAdjustItemQuality::VideoType::VTYPE_CAM	},				/* カメラ画質描画 */
		{VHAL_VSRC_ID_HDMI,						CVhalColorAdjustItemQuality::VideoType::VTYPE_HDMI	},				/* HDMI映像 */
		{VHAL_VSRC_ID_MULTISENSORY,				CVhalColorAdjustItemQuality::VideoType::VTYPE_OTHER	},				/* 多感覚連携：無効時は映像種別 [その他] を適用 */
	};

	/* 映像パス名から映像ソースIDへの変換用 */
	using videopath_type_map = std::map<std::string, CVhalColorManager::VideoSourceId>;
	/* カメラ映像パス変換用 */
	const videopath_type_map camera_path_type_table_
	{
		/* 映像パス名,                           映像ソースID */
		{"CAMERA",								VHAL_VSRC_ID_IMAGE_ADJUST_CAM			},				/* カメラ */
	};
	/* 前席映像パス変換用 */
	const videopath_type_map front_path_type_table_
	{
		/* 映像パス名,                           映像ソースID */
		{"DTV",									VHAL_VSRC_ID_DTV						},				/* DTV */
		{"HDMI",								VHAL_VSRC_ID_HDMI						},				/* HDMI */
		{"DRC",									VHAL_VSRC_ID_IMAGE_ADJUST_CAM			},				/* ドラレコ */
		{"CAMERA-IMG-ADJ",						VHAL_VSRC_ID_IMAGE_ADJUST_CAM			},				/* カメラ画質調整 */
		{"CARPLAY",								VHAL_VSRC_ID_OTHER						},				/* CarPlay */
		{"ANDROIDAUTO",							VHAL_VSRC_ID_OTHER						},				/* AndroidAuto */
		{"MEDIAPLAYER",							VHAL_VSRC_ID_OTHER						},				/* メディアプレイヤー */
		{"MULTISENSORY",						VHAL_VSRC_ID_OTHER						},				/* 多感覚連携 */
		{"FRAGRANCE",							VHAL_VSRC_ID_OTHER						},				/* フレグランス */
	};

	/* モード種別から映像表示モードへの変換用 */
	using CVideoDispModeSendItem_map = std::map<uint32_t, CVhalColorAdjustItemQuality::VideoDispMode>;
	/* 映像表示モード（昼夜モード種別） */
	const CVideoDispModeSendItem_map gDaynightSendItem_table
	{
		/* モード種別,                           映像表示モード */
		{VHAL_SETTING_DAY,						CVhalColorAdjustItemQuality::VideoDispMode::VDISP_NORMAL_DAY			},	/* 昼設定 */
		{VHAL_SETTING_NIGHT,					CVhalColorAdjustItemQuality::VideoDispMode::VDISP_NORMAL_NIGHT			},	/* 夜設定 */
		{VHAL_SETTING_FORCED_DAY,				CVhalColorAdjustItemQuality::VideoDispMode::VDISP_NORMAL_FORCED_DAY		}	/* 強制昼設定 */
	};
	/* 映像表示モード（テーマカラー種別） */
	const CVideoDispModeSendItem_map gThemeSendItem_table
	{
		/* モード種別,                           映像表示モード */
		{VHAL_THEME_COLOR_AUTO_LIGHT,			CVhalColorAdjustItemQuality::VideoDispMode::VDISP_COLOR_AUTO_LIGHT		},	/* 自動設定(Light color) */
		{VHAL_THEME_COLOR_AUTO_DARK,			CVhalColorAdjustItemQuality::VideoDispMode::VDISP_COLOR_AUTO_DARK		},	/* 自動設定(Dark color) */
		{VHAL_THEME_COLOR_SELECT_LIGHT,			CVhalColorAdjustItemQuality::VideoDispMode::VDISP_COLOR_SELECT_LIGHT	},	/* 手動設定(Light color) */
		{VHAL_THEME_COLOR_SELECT_DARK,			CVhalColorAdjustItemQuality::VideoDispMode::VDISP_COLOR_SELECT_DARK		},	/* 手動設定(Dark color) */
		{VHAL_THEME_COLOR_FORCED_LIGHT,			CVhalColorAdjustItemQuality::VideoDispMode::VDISP_NORMAL_FORCED_DAY		}	/* 強制昼設定 */
	};

}

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalColorSendItemQuality::CVhalColorSendItemQuality(void)
{
	prev_micon_send_data_.clear();
}

/*****************************************************************************
 処理概要：	再初期化処理
 引数    ：	なし
 戻り値  ：	なし
 注意    ：	weston由来のdisplay_rectは保持対象
*****************************************************************************/
void CVhalColorSendItemQuality::ReInit(void) noexcept
{
	/* メンバ変数のクリア */
	rectangleItem	rect{reserve_data_.GetDisplayRect()};	/* weston由来のdisplay_rectは保持対象 */
	reserve_data_ = ColorSendItem{};
	reserve_data_.SetDisplayRect(rect.GetX(), rect.GetY(), rect.GetW(), rect.GetH());	/* 再設定 */
	current_data_ = ColorSendItem{};
	prev_micon_send_data_.clear();
}

/*****************************************************************************
 処理概要：	画質モード通知送信データ構築
 引数    ：	std::vector<uint8_t> &send_data		(o)送信データ
 戻り値  ：	処理結果
           		VHAL_ERR				異常終了
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalColorSendItemQuality::Build(std::vector<uint8_t> &send_data) const
{
	constexpr int32_t result{VHAL_SUCCESS};
	bool is_mute_status{false};

	/* Video HAL内部のMuteとバックライトの両プロパティがfalseの意味を示す場合のみ(0：解除)で送信する */
	if ((false == current_data_.GetMuteFrontDisp()) && (true == current_data_.GetBackLight()))
	{
		is_mute_status = false;
	}
	else
	{
		is_mute_status = true;
	}

	uint8_t type       {SetByteData(GetVideoType(current_data_.GetVideoSourceId()), 4U, 4U)};
	uint8_t back_light {SetByteData(0U, 1U, 3U)}; /* 0固定 */
	uint8_t mute       {SetByteData(BToUI8(is_mute_status), 1U, 1U)};
	uint8_t contrast   {I32ToUI8(CVhalColorManager::kColorImgAdjStepInitContrast)};
	uint8_t brightness {I32ToUI8(CVhalColorManager::kColorImgAdjStepInitBrightness)};
	uint8_t mode       {SetByteData(GetVideoDispMode(current_data_.GetColorType()), 4U, 0U)};

	/* 優先度1：カメラ映像パス設定時 */
	if ("CAMERA" == current_data_.GetCameraPathCurrent())
	{
		type      += SetByteData(0U, 1U, 1U);	/* Mute解除固定 */
		contrast   = I32ToUI8(current_data_.GetStepContrast());
		brightness = I32ToUI8(current_data_.GetStepBrightness());
	}
	/* 優先度2：強制多感覚連携画質 適用有効 */
	else if ((true == current_data_.GetForceMultisensoryEnable()) && (VHAL_PATH_MULTISENSORY == current_data_.GetFrontPathCurrent()))
	{
		type       = SetByteData(static_cast<uint8_t>(CVhalColorAdjustItemQuality::VideoType::VTYPE_MULTISENSORY), 4U, 4U) + mute + back_light;
		contrast   = I32ToUI8(current_data_.GetForceMultisensoryStepContrast());
		brightness = I32ToUI8(current_data_.GetForceMultisensoryStepBrightness());
		mode       = SetByteData(GetVideoDispMode(current_data_.GetHmiColorType()), 4U, 0U);
	}
	/* 優先度3：強制HMI画質 適用有効 */
	else if (true == current_data_.GetForceHmiEnable())
	{
		type       = SetByteData(static_cast<uint8_t>(CVhalColorAdjustItemQuality::VideoType::VTYPE_OTHER), 4U, 4U) + mute + back_light;
		contrast   = I32ToUI8(current_data_.GetHmiStepContrast());
		brightness = I32ToUI8(current_data_.GetHmiStepBrightness());
		mode       = SetByteData(GetVideoDispMode(current_data_.GetHmiColorType()), 4U, 0U);
	}
	/* 優先度4：上記以外 */
	else
	{
		type += mute + back_light;

		/* HMI関連 */
		/* 映像ソースID多感覚連携は強制多感覚が無効の場合HMIとして扱う */
		if (VHAL_VSRC_ID_OTHER == current_data_.GetVideoSourceId() || VHAL_VSRC_ID_MULTISENSORY == current_data_.GetVideoSourceId())
		{
			contrast   = I32ToUI8(current_data_.GetHmiStepContrast());
			brightness = I32ToUI8(current_data_.GetHmiStepBrightness());
			mode       = SetByteData(GetVideoDispMode(current_data_.GetHmiColorType()), 4U, 0U);
		}
		/* HMI以外 */
		else
		{
			contrast   = I32ToUI8(current_data_.GetStepContrast());
			brightness = I32ToUI8(current_data_.GetStepBrightness());
		}
	}

	/* 送信データ構築 */
	/* 旧要件：HMI 以外は小窓に対して設定 */
	/* 新要件：小窓は使用しない 画面全体に画質調整を適用 */
	send_data.assign(static_cast<uint8_t>(CVhalColorAdjustItemQuality::AdjustItemNo::POS_MAX), 0U);				/* 送信データ全体を0クリア */
	send_data[static_cast<uint8_t>(CVhalColorAdjustItemQuality::AdjustItemNo::POS_OPC)]         = static_cast<uint8_t>(CVhalColorSendItemQuality::OpcType::OPC_COLOR_MODE);		/* OPC：画質モード通知(01h) */
	send_data[static_cast<uint8_t>(CVhalColorAdjustItemQuality::AdjustItemNo::POS_DATA_TYPE)]   = static_cast<uint8_t>(CVhalColorSendItemQuality::DataType::DTYPE_COLOR_MODE);	/* data_type(Display(=36h) */
	send_data[static_cast<uint8_t>(CVhalColorAdjustItemQuality::AdjustItemNo::POS_VTYPE_MUTE)]  = type;			/* 映像種別(bit7-4) + 0固定(bit3-2) + Mute(bit1)+ 0固定(bit0) */
	send_data[static_cast<uint8_t>(CVhalColorAdjustItemQuality::AdjustItemNo::POS_CONTRAST)]    = contrast;		/* コントラスト */
	send_data[static_cast<uint8_t>(CVhalColorAdjustItemQuality::AdjustItemNo::POS_BRIGHTNESS)]  = brightness;	/* 明るさ */
	send_data[static_cast<uint8_t>(CVhalColorAdjustItemQuality::AdjustItemNo::POS_VDISP)]       = mode;			/* 映像表示モード */

	/* Suppress log output */
	if (prev_micon_send_data_ != send_data)
	{
		/* 送信データログ */
		VHAL_LOGI("send_data size=%lu opc=0x%x vtype=0x%x contrast=%u bright=%u vdisp=0x%x", 
			send_data.size(),
			send_data[static_cast<uint8_t>(CVhalColorAdjustItemQuality::AdjustItemNo::POS_OPC)], 
			send_data[static_cast<uint8_t>(CVhalColorAdjustItemQuality::AdjustItemNo::POS_VTYPE_MUTE)], 
			send_data[static_cast<uint8_t>(CVhalColorAdjustItemQuality::AdjustItemNo::POS_CONTRAST)], 
			send_data[static_cast<uint8_t>(CVhalColorAdjustItemQuality::AdjustItemNo::POS_BRIGHTNESS)], 
			send_data[static_cast<uint8_t>(CVhalColorAdjustItemQuality::AdjustItemNo::POS_VDISP)]);
		/* Locally save send data */
		prev_micon_send_data_.assign(send_data.begin(), send_data.end());
	}

	return result;
}

/*****************************************************************************
 処理概要：	送信アイテム種別取得
 引数    ：	なし
 戻り値  ：	送信アイテム種別
*****************************************************************************/
CVhalMiconSendItem::SendItemType CVhalColorSendItemQuality::GetItemType(void) const noexcept
{
	return CVhalMiconSendItem::SendItemType::SEND_ITEM_TYPE_MISC;
}

/*****************************************************************************
 処理概要：	バックライト点灯/消灯設定
 引数    ：	bool light		(i)バックライト点灯/消灯
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorSendItemQuality::SetBackLight(const bool light)
{
	if (("CAMERA" == reserve_data_.GetCameraPathCurrent()) && (false == light))	/* 画面消要求時かつカメラ映像パス設定中 */
	{
		VHAL_LOGW("Backlight ON while camera_path=%s", reserve_data_.GetCameraPathCurrent().c_str());
	}
	reserve_data_.SetBackLight(light);
}

/*****************************************************************************
 処理概要：	MuteON/解除設定
 引数    ：	bool mute		(i)MuteON/解除
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorSendItemQuality::SetMuteFrontDisp(const bool mute)
{
	if (("CAMERA" == reserve_data_.GetCameraPathCurrent()) && (true == mute))	/* MuteOn要求時かつカメラ映像パス設定中 */
	{
		VHAL_LOGW("Mute ON while camera_path=%s", reserve_data_.GetCameraPathCurrent().c_str());
	}
	reserve_data_.SetMuteFrontDisp(mute);
}

/*****************************************************************************
 処理概要：	コントラスト値設定
 引数    ：	const int32_t step									(i)コントラスト値
           	const CVhalColorManager::VideoSourceId vsrcid		(i)映像ソースID
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorSendItemQuality::SetStepContrast(const int32_t step, const CVhalColorManager::VideoSourceId vsrcid) noexcept
{
	if (VHAL_VSRC_ID_OTHER == vsrcid)
	{
		/* コントラスト値設定：HMI 画質用にセット */
		reserve_data_.SetHmiStepContrast(step);
	}
	else if (VHAL_VSRC_ID_MULTISENSORY == vsrcid)
	{
		/* コントラスト値設定：強制多感覚連携 画質用にセット */
		reserve_data_.SetForceMultisensoryStepContrast(step);
	}
	else
	{
		/* 映像パスに紐づく映像ソースIDと一致する場合は更新する */
		/* （映像パス切替時に最新値で再設定される） */
		if (vsrcid == reserve_data_.GetVideoSourceId())
		{
			reserve_data_.SetStepContrast(step);
		}
	}
}

/*****************************************************************************
 処理概要：	明るさ値設定
 引数    ：	const int32_t step									(i)明るさ値
           	const CVhalColorManager::VideoSourceId vsrcid		(i)映像ソースID
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorSendItemQuality::SetStepBrightness(const int32_t step, const CVhalColorManager::VideoSourceId vsrcid) noexcept
{
	if (VHAL_VSRC_ID_OTHER == vsrcid)
	{
		/* 明るさ値設定：HMI 画質用にセット */
		reserve_data_.SetHmiStepBrightness(step);
	}
	else if (VHAL_VSRC_ID_MULTISENSORY == vsrcid)
	{
		/* 明るさ値設定：強制多感覚連携 画質用にセット */
		reserve_data_.SetForceMultisensoryStepBrightness(step);
	}
	else
	{
		/* 映像パスに紐づく映像ソースIDと一致する場合は更新する */
		/* （映像パス切替時に最新値で再設定される） */
		if (vsrcid == reserve_data_.GetVideoSourceId())
		{
			reserve_data_.SetStepBrightness(step);
		}
	}
}

/*****************************************************************************
 処理概要：	カラータイプ(DayNight/Theme)設定
 引数    ：	const CVhalColorRecord::ColorType type				(i)カラータイプ(DayNight/Theme)
           	const CVhalColorManager::VideoSourceId vsrcid		(i)映像ソースID
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorSendItemQuality::SetColorType(const CVhalColorRecord::ColorType type, const CVhalColorManager::VideoSourceId vsrcid) noexcept
{
	if (VHAL_VSRC_ID_OTHER == vsrcid)
	{
		/* カラータイプ(Theme)：HMI */
		reserve_data_.SetHmiColorType(type);
	}
	else if (VHAL_VSRC_ID_MULTISENSORY == vsrcid)
	{
		/* カラータイプ(Theme)：強制多感覚連携 */
		/* ※Theme設定値はHMIと同じ扱いとし、共有する */
		reserve_data_.SetHmiColorType(type);
	}
	else
	{
		reserve_data_.SetColorType(type);
	}
}

/*****************************************************************************
 処理概要：	昼夜モード種別設定
 引数    ：	CVhalColorManager::DayNightType type		(i)昼夜モード種別
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorSendItemQuality::SetDaynightType(const CVhalColorManager::DayNightType type) noexcept
{
	reserve_data_.SetDayNight(type);
}

/*****************************************************************************
 処理概要：	テーマカラー種別設定
 引数    ：	CVhalColorManager::ThemeColorType type		(i)テーマカラー種別
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorSendItemQuality::SetThemeColorType(const CVhalColorManager::ThemeColorType type) noexcept
{
	reserve_data_.SetThemeColor(type);
}

/*****************************************************************************
 処理概要：	強制HMI画質適用設定
 引数    ：	const bool			force_hmi_enable		(i)強制HMI有効フラグ
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorSendItemQuality::SetForceHmiEnable(const bool force_hmi_enable) noexcept
{
	reserve_data_.SetForceHmiEnable(force_hmi_enable);
}

/*****************************************************************************
 処理概要：	強制多感覚連携画質適用設定
 引数    ：	const bool	force_multisensory_enable	(i)強制多感覚連携有効フラグ
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorSendItemQuality::SetForceMultisensoryEnable(const bool force_multisensory_enable) noexcept
{
	reserve_data_.SetForceMultisensoryEnable(force_multisensory_enable);
}

/*****************************************************************************
 処理概要：	ディスプレイ出力矩形設定
 引数    ：	const int32_t	 	x					(i)x座標
           	const int32_t		y					(i)y座標
           	const int32_t		w					(i)幅
           	const int32_t		h					(i)高さ
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorSendItemQuality::SetDisplayRectangle(const int32_t x, const int32_t y, const int32_t w, const int32_t h) noexcept
{
	reserve_data_.SetDisplayRect(x, y, w, h);
}

/*****************************************************************************
 処理概要：	ヒーコン矩形設定
 引数    ：	const int32_t		x					(i)x座標
           	const int32_t		y					(i)y座標
           	const int32_t		w					(i)幅
           	const int32_t		h					(i)高さ
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorSendItemQuality::SetHeaconRectangle(const int32_t x, const int32_t y, const int32_t w, const int32_t h) noexcept
{
	reserve_data_.SetHeaconRect(x, y, w, h);
}

/*****************************************************************************
 処理概要：	前席映像パス設定
 引数    ：	const std::string& path			 		(i)前席映像パス
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorSendItemQuality::SetFrontPath(const std::string& path)
{
	reserve_data_.SetVideoFrontPathCurrent(path);
	reserve_data_.SetVideoSourceId(GetVideoPathSourceId(path));
}

/*****************************************************************************
 処理概要：	カメラ映像パス設定
 引数    ：	const std::string& path			 		(i)カメラ映像パス
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorSendItemQuality::SetCameraPath(const std::string& path)
{
	/* カメラ映像パス設定：画面消中 or ミュート中 */
	if (("CAMERA" == path) && 
		(( false == reserve_data_.GetBackLight() ) || ( true == reserve_data_.GetMuteFrontDisp() )))
	{
		VHAL_LOGW("Backlight=%d, mute=%d while camera_path=%s", reserve_data_.GetBackLight(), reserve_data_.GetMuteFrontDisp(), path.c_str());
	}
	reserve_data_.SetCameraPathCurrent(path);
	reserve_data_.SetVideoSourceId(GetVideoPathSourceId(path));
}

/*****************************************************************************
 処理概要：	画質モード通知送信データ格納
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorSendItemQuality::ApplyAdjustment(void)
{
	current_data_ = reserve_data_;
}

/*****************************************************************************
 処理概要：	映像ソースIDから映像種別を取得
 引数    ：	const CVhalColorManager::VideoSourceId vsrcid		(i)映像ソースID
 戻り値  ：	映像種別
*****************************************************************************/
uint8_t CVhalColorSendItemQuality::GetVideoType(const CVhalColorManager::VideoSourceId vsrcid) const
{
	CVhalColorAdjustItemQuality::VideoType vtype{CVhalColorAdjustItemQuality::VideoType::VTYPE_OTHER};

	const auto iter = gVsrcColorSendItem_table.find(vsrcid);
	if (iter != gVsrcColorSendItem_table.end())
	{
		vtype = iter->second;
	}

	return static_cast<uint8_t>(vtype);
}

/*****************************************************************************
 処理概要：	映像パス名から映像ソースIDを取得
 引数    ：	const std::string& path			 		(i)映像パス
 戻り値  ：	映像ソースID
*****************************************************************************/
CVhalColorManager::VideoSourceId CVhalColorSendItemQuality::GetVideoPathSourceId(const std::string& path) const
{
	CVhalColorManager::VideoSourceId vsrcid{VHAL_VSRC_ID_OTHER};

	/* カメラ映像パスを優先 */
	auto iter = camera_path_type_table_.find(path);
	if (iter != camera_path_type_table_.end())
	{
		vsrcid = iter->second;
	}
	else
	{
		iter = front_path_type_table_.find(path);
		if (iter != front_path_type_table_.end())
		{
			/* 前席映像パス 映像用 */
			vsrcid = iter->second;
		}
	}

	return vsrcid;
}

/*****************************************************************************
 処理概要：	画質モード通知送信データ格納状態取得
 引数    ：	なし
 戻り値  ：	処理結果
           		true			画質モード通知送信データなし(未送信)
           		false			画質モード通知送信データあり(送信開始済み)
*****************************************************************************/
bool CVhalColorSendItemQuality::IsSendDataEmpty(void) const noexcept
{
	return prev_micon_send_data_.empty();
}

/*****************************************************************************
 処理概要：	１バイトデータの成型
 引数    ：	const uint8_t in_data			(i)入力データ
           	const uint8_t enable_bit		(i)入力データの有効ビット数
           	const uint8_t leftShift_bit		(i)入力データの左シフトビット数
 戻り値  ：	設定した１バイトデータ
*****************************************************************************/
uint8_t CVhalColorSendItemQuality::SetByteData(const uint8_t in_data, const uint8_t enable_bit, const uint8_t leftShift_bit) const
{
	uint8_t out_data{0U};
	do{
		VHAL_LOGV("in_data=%d, enable_bit=%d, leftShift_bit=%d", in_data, enable_bit, leftShift_bit);

		/* シフト数チェック */
		if (static_cast<uint8_t>(UINT8_WIDTH) < (enable_bit + leftShift_bit))
		{
			VHAL_LOGE("Over shift.  enable_bit=%d, leftShift_bit=%d", enable_bit, leftShift_bit);
			break;
		}

		/* 有効ビット内に収まるデータかチェック */
		if (((1U << enable_bit) - 1U) < in_data)
		{
			VHAL_LOGE("Over data.  in_data=%d, enable_bit=%d", in_data, enable_bit);
			break;
		}

		out_data = in_data << leftShift_bit;
		VHAL_LOGV("out_data=0x%x", out_data);
	} while(false);
	return out_data;
}

/*****************************************************************************
 処理概要：	映像表示モードの取得
 引数    ：	const CVhalColorRecord::ColorType color_type	(i)カラータイプ(DayNight/Theme)
 戻り値  ：	映像表示モード
*****************************************************************************/
uint8_t CVhalColorSendItemQuality::GetVideoDispMode(const CVhalColorRecord::ColorType color_type) const
{
	uint8_t videoDispMode{static_cast<uint8_t>(CVhalColorAdjustItemQuality::VideoDispMode::VDISP_NORMAL_NIGHT)};

	VHAL_LOGV("color_type=%d, theme_color_=%d, day_night_=%d", color_type, current_data_.GetThemeColor(), current_data_.GetDayNight());

	do{
		if (CVhalColorRecord::ColorType::COLOR_TYPE_THEME == color_type)
		{
			/* テーマカラー種別 */
			const auto iter = gThemeSendItem_table.find(I32ToUI32(current_data_.GetThemeColor()));
			if (iter == gThemeSendItem_table.end())
			{
				VHAL_LOGE("theme_color_=0x%x", current_data_.GetThemeColor());
				videoDispMode =  static_cast<uint8_t>(CVhalColorAdjustItemQuality::VideoDispMode::VDISP_COLOR_SELECT_DARK);
				break;
			}
			videoDispMode = static_cast<uint8_t>(iter->second);
		}
		else
		{
			/* 昼夜モード種別 */
			const auto iter = gDaynightSendItem_table.find(I32ToUI32(current_data_.GetDayNight()));
			if (iter == gDaynightSendItem_table.end())
			{
				VHAL_LOGE("day_night_=0x%x", current_data_.GetDayNight());
				break;
			}
			videoDispMode = static_cast<uint8_t>(iter->second);
		}

		VHAL_LOGV("videoDispMode=%d", videoDispMode);
	} while(false);
	return videoDispMode;
}

/*****************************************************************************
 処理概要：	入力データの上位バイト取得
 引数    ：	const int32_t in_data			(i)入力データ
 戻り値  ：	１バイトデータ
*****************************************************************************/
uint8_t CVhalColorSendItemQuality::GetHighData(const int32_t in_data) const
{
	const uint8_t out_data{static_cast<uint8_t>((I32ToUI32(in_data) & 0xFF00U) >> static_cast<uint8_t>(UINT8_WIDTH))};
	VHAL_LOGV("in_data=0x%x, out_data=0x%x", in_data, out_data);
	return out_data;
}

/*****************************************************************************
 処理概要：	入力データの下位バイト取得
 引数    ：	const int32_t in_data			(i)入力データ
 戻り値  ：	１バイトデータ
*****************************************************************************/
uint8_t CVhalColorSendItemQuality::GetLowData(const int32_t in_data) const
{
	const uint8_t out_data{static_cast<uint8_t>(I32ToUI32(in_data) & 0x00FFU)};
	VHAL_LOGV("in_data=0x%x, out_data=0x%x", in_data, out_data);
	return out_data;
}

} /* namespace videohal */
