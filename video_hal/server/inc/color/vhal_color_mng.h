/*******************************************************************************
    機能名称    ：  画質調整制御モジュール
    ファイル名称：  vhal_color_mng.h
*******************************************************************************/
#ifndef	VHAL_COLOR_MNG_H
#define	VHAL_COLOR_MNG_H

#include "vhal_color_record.h"
#include "vhal_color_db.h"

namespace videohal
{

class CVhalColorDatabase;
class CVhalMiconCommControl;
class CVhalColorSendItemQuality;
class CVhalColorAdjustMiscReceiver;

/*****************************************************************************
 クラス名称：CVhalColorManager
 処理概要  ：画質調整の管理を行う。
*****************************************************************************/
class CVhalColorManager final {
public:
	// [TODO]VideoHAL共通定義の方が良いかも？
	using VideoSourceId = uint32_t;		/* 映像ソースID型 */
	using DayNightType = int32_t;		/* 昼夜モード種別 */
	using ThemeColorType = int32_t;		/* テーマカラー種別 */

	enum class StepType : uint32_t {
		STEP_TYPE_OTHER = 0,					/* 操作画面 */
		STEP_TYPE_CAM,							/* カメラ */
		STEP_TYPE_DTV,							/* DTV */
		STEP_TYPE_HDMI,							/* HDMI */
		STEP_TYPE_MULTISENSORY,					/* 多感覚連携 */
		STEP_TYPE_MAX
	};

	/* STEP値 */
	enum class ColorStepNum : int32_t {
		COLOR_STEP_NUM_INVALID	= 0,			/*  0:無効値 */
		COLOR_STEP_NUM_MIN		= 1,			/*  1:最小値 */
		COLOR_STEP_NUM_MAX		= 63,			/* 63:最大値 */
	};

	/* 画質調整ステップ値の初期値 */
	static constexpr int32_t kColorImgAdjStepInitContrast{32};
	static constexpr int32_t kColorImgAdjStepInitBrightness{32};

	CVhalColorManager(void);
	~CVhalColorManager(void);
  	CVhalColorManager(const CVhalColorManager& src) = delete;
	CVhalColorManager& operator=(const CVhalColorManager& src) & = delete;
	CVhalColorManager(CVhalColorManager&& src) = delete;
	CVhalColorManager& operator=(CVhalColorManager&& src) & = delete;

	int32_t Initialize(CVhalMiconCommControl* const p_micon_comm_control, CVhalColorAdjustMiscReceiver* const p_color_adjust_misc_receiver);
	void Finalize(void);
	void ReInit(void) noexcept;

	/* 映像ソースID設定 */
	int32_t SetVideoSource(const VideoSourceId vsrcid);
	/* 昼夜モード種別設定 */
	int32_t SetDayNight(const DayNightType day_night);
	/* テーマカラー種別設定 */
	int32_t SetThemeColor(const ThemeColorType theme_color);
	/* バックライト点灯/消灯設定 */
	int32_t SetBackLight(const bool light);
	/* MuteON/解除設定 */
	int32_t SetMuteFrontDisp(const bool mute);

	/* 現在の映像ソースIDに対してコントラストを設定 */
	int32_t SetCurrentColorStepContrast(const int32_t step);
	/* 現在の映像ソースIDに対して明るさを設定 */
	int32_t SetCurrentColorStepBrightness(const int32_t step);

	/* 前席映像パスの設定 */
	bool SetFrontPath(const std::string& path);
	/* カメラ映像パスの設定 */
	void SetCameraPath(const std::string& path);
	/* 強制HMI画質適用設定 */
	void SetForceHmiEnable(const bool force_hmi_enable) noexcept;
	/* 強制多感覚連携画質適用設定 */
	void SetForceMultisensoryEnable(const bool force_multisensory_enable) noexcept;
	/* ディスプレイ出力矩形を設定 */
	void SetDisplayRectangle(const int32_t x, const int32_t y, const int32_t w, const int32_t h) noexcept;
	/* ヒーコン矩形を設定 */
	void SetHeaconRectangle(const int32_t x, const int32_t y, const int32_t w, const int32_t h) noexcept;

	/* 現在の映像ソースIDの設定を取得 */
	int32_t GetCurrentColorStep(CVhalColorRecord::ColorImgAdjStep& colorStep, const VideoSourceId vsrcid) const;

	/* 画質モード通知更新 */
	int32_t ApplyAdjustment(const bool check_update);

	/* 画質調整テーブルのクリア */
	int32_t ClearColorStep(void);
	/* 現在の映像ソースIDの取得 */
	VideoSourceId GetVideoSourceId(void) const noexcept;
	/* 映像パスに紐づく映像ソースIDのカラータイプ(DayNight/Theme)を取得 */
	CVhalColorRecord::ColorType GetVideoPathColorType(void) const noexcept;
	/* 画質モード通知送信状態取得 */
	bool IsFirstSend(void) const noexcept;

private:
	VideoSourceId	video_source_id_;				/* 前席映像ソース設定された映像ソースID（画質調整ステップ値の変更対象） */
	VideoSourceId	videopath_source_id_;			/* 映像パスに紐づく映像ソースID（TAB2小窓用送信対象） */
	DayNightType	day_night_;						/* 昼夜モード種別 */
	ThemeColorType	theme_color_;					/* テーマカラー種別 */
	bool			forced_hmi_img_adj_;			/* 強制HMI有効フラグ */
	bool			forced_multisensory_img_adj_;	/* 強制多感覚連携有効フラグ */

	std::string		video_front_path_current_;		/* 前席映像パス */
	std::string		camera_path_current_;			/* カメラ映像パス */

	std::unique_ptr<CVhalColorDatabase>		p_color_database_;
	CVhalMiconCommControl*	p_micon_comm_control_;
	std::unique_ptr<CVhalColorSendItemQuality> p_color_send_item_quality_;
	CVhalColorAdjustMiscReceiver*		p_color_adjust_misc_receiver_;
	
	/* CVhalColorDatabase用のユニークIDを取得 */
	uint32_t GetColorId(const VideoSourceId vsrcid) const;
	/* CVhalColorDatabase用のカラータイプ(DayNight/Theme)を取得 */
	CVhalColorRecord::ColorType GetColorType(const VideoSourceId vsrcid) const;
	/* 最新の映像ソースIDのSTEP設定を設定 */
	int32_t SetCurrentColorStep(const CVhalColorRecord::ColorImgAdjStep& colorStep, const VideoSourceId vsrcid);
	/* 現在の映像ソースIDのSTEP設定を読込み */
	CVhalColorRecord::ColorImgAdjStep ReadCurrentColorStep(CVhalColorRecord &ColorRecord, const uint32_t color_mode, const VideoSourceId vsrcid) const;
	/* 現在の映像ソースIDのSTEP設定を書込み */
	int32_t WriteCurrentColorStep(CVhalColorRecord &ColorRecord, const uint32_t color_mode, const CVhalColorRecord::ColorImgAdjStep& colorStep, const VideoSourceId vsrcid);
	/* 画質調整テーブルの作成 */
	int32_t CreateColorStep(void);
	/* 画質調整設定読込み */
	int32_t LoadColorStep(void);
};


} /* namespace videohal */


#endif	/* #ifndef	VHAL_COLOR_MNG_H */
