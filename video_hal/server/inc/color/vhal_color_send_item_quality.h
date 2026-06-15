/*******************************************************************************
    機能名称    ：  MISC画質モード通知送信アイテムモジュール
    ファイル名称：  vhal_color_send_item_quality.h
*******************************************************************************/
#ifndef	VHAL_COLOR_SEND_ITEM_QUALITY_H
#define	VHAL_COLOR_SEND_ITEM_QUALITY_H

#include "vhal_micon_send_item.h"
#include "vhal_color_mng.h"
#include "vhal_color_db.h"
#include "vhal_round_cast.h"
#include "vhal_micon_misc_control.h"

namespace videohal
{

class CVhalColorDatabase;
class CVhalColorManager;

/*****************************************************************************
 クラス名称：CVhalColorSendItemQuality
 処理概要  ：MISC画質モード通知送信アイテム
*****************************************************************************/
class CVhalColorSendItemQuality : public CVhalMiconSendItem {
public:

	/* data_type */
	enum class DataType : uint8_t {
		DTYPE_COLOR_MODE			= kDatatypeDisplay,	/* 36h:Display */
	};

	/* OPC(sub_type) */
	enum class OpcType : uint8_t {
		OPC_COLOR_MODE				= SUB_TYPE_DISP_MODE_REQ,	/* 01h:画質モード通知 */
	};

	CVhalColorSendItemQuality(void);
	~CVhalColorSendItemQuality(void) override = default;
  	CVhalColorSendItemQuality(const CVhalColorSendItemQuality& src) = delete;
	CVhalColorSendItemQuality& operator=(const CVhalColorSendItemQuality& src) & = delete;
	CVhalColorSendItemQuality(CVhalColorSendItemQuality&& src) = delete;
	CVhalColorSendItemQuality& operator=(CVhalColorSendItemQuality&& src) & = delete;

	/* 再初期化処理 */
	void ReInit(void) noexcept;

	/* 送信データ構築 */
	int32_t Build(std::vector<uint8_t> &send_data) const override;

	/* 送信アイテム種別取得 */
	SendItemType GetItemType(void) const noexcept override;

	/* バックライト点灯/消灯設定 */
	void SetBackLight(const bool light);
	/* MuteON/解除設定 */
	void SetMuteFrontDisp(const bool mute);
	/* コントラスト値設定 */
	void SetStepContrast(const int32_t step, const CVhalColorManager::VideoSourceId vsrcid) noexcept;
	/* 明るさ値設定 */
	void SetStepBrightness(const int32_t step, const CVhalColorManager::VideoSourceId vsrcid) noexcept;
	/* カラータイプ(DayNight/Theme)設定 */
	void SetColorType(const CVhalColorRecord::ColorType type, const CVhalColorManager::VideoSourceId vsrcid) noexcept;
	/* 昼夜モード種別設定 */
	void SetDaynightType(const CVhalColorManager::DayNightType type) noexcept;
	/* テーマカラー種別設定 */
	void SetThemeColorType(const CVhalColorManager::ThemeColorType type) noexcept;
	/* 強制HMI画質適用設定 */
	void SetForceHmiEnable(const bool force_hmi_enable) noexcept;
	/* 強制多感覚連携画質適用設定 */
	void SetForceMultisensoryEnable(const bool force_multisensory_enable) noexcept;
	/* ディスプレイ出力矩形設定 */
	void SetDisplayRectangle(const int32_t x, const int32_t y, const int32_t w, const int32_t h) noexcept;
	/* ヒーコン矩形設定 */
	void SetHeaconRectangle(const int32_t x, const int32_t y, const int32_t w, const int32_t h) noexcept;
	/* 前席映像パス設定 */
	void SetFrontPath(const std::string& path);
	/* カメラ映像パス設定 */
	void SetCameraPath(const std::string& path);
	/* 画質モード通知データ設定 */
	void ApplyAdjustment(void);
	/* 映像ソースIDから映像種別を取得 */
	uint8_t GetVideoType(const CVhalColorManager::VideoSourceId vsrcid) const;
	/* 映像パス名から映像ソースIDを取得 */
	CVhalColorManager::VideoSourceId GetVideoPathSourceId(const std::string& path) const;
	/* 画質モード通知送信データ格納状態取得 */
	bool IsSendDataEmpty(void) const noexcept;
private:

	/* 短形情報 */
	struct rectangleItem {
	public:
		rectangleItem(const int32_t rect_x, const int32_t rect_y, const int32_t rect_w, const int32_t rect_h) noexcept
			:x_(rect_x)
			,y_(rect_y)
			,w_(rect_w)
			,h_(rect_h)
		{}
		rectangleItem(void) noexcept
			:rectangleItem(0, 0, 0, 0)
		{}
		void SetRect(const int32_t rect_x, const int32_t rect_y, const int32_t rect_w, const int32_t rect_h) noexcept
		{
			x_ = rect_x;
			y_ = rect_y;
			w_ = rect_w;
			h_ = rect_h;
		}
		int32_t GetX(void) const noexcept
		{
			return x_;
		}
		int32_t GetY(void) const noexcept
		{
			return y_;
		}
		int32_t GetW(void) const noexcept
		{
			return w_;
		}
		int32_t GetH(void) const noexcept
		{
			return h_;
		}
	private:
		int32_t 	x_;												/* 矩形X */
		int32_t 	y_;												/* 矩形Y */
		int32_t 	w_;												/* 矩形W */
		int32_t 	h_;												/* 矩形H */
	};

	/* ヒーコン情報 */
	struct rectangleEnableItem {
	public:
		rectangleEnableItem(void) noexcept
			:rect_(0, 0, 0, 0)
		{}
		void SetRectItem(const int32_t rect_x, const int32_t rect_y, const int32_t rect_w, const int32_t rect_h) noexcept
		{
			rect_.SetRect(rect_x, rect_y, rect_w, rect_h);
		}
		int32_t GetW(void) const noexcept
		{
			return rect_.GetW();
		}
		int32_t GetH(void) const noexcept
		{
			return rect_.GetH();
		}
	private:
		struct rectangleItem	rect_;								/* 短形情報 */
	};

	struct ColorSendItem {
	public:
		/* コンストラクタ */
		ColorSendItem(void)
		:video_source_id_(VHAL_VSRC_ID_OTHER)
		,step_contrast_(static_cast<int32_t>(CVhalColorManager::kColorImgAdjStepInitContrast))							/* 画質モード通知で0が送信されないよう、フェールセーフとして32を設定 */
		,step_brightness_(static_cast<int32_t>(CVhalColorManager::kColorImgAdjStepInitBrightness))						/* 画質モード通知で0が送信されないよう、フェールセーフとして32を設定 */
		,back_light_(true)
		,mute_frontdisp_(true)
		,color_type_(CVhalColorRecord::ColorType::COLOR_TYPE_DAYNIGHT)
		,day_night_(UI32ToI32(VHAL_SETTING_NIGHT))					/* 眩しいのを抑えるためフェールで夜設定 */
		,theme_color_(VHAL_THEME_COLOR_AUTO_DARK)					/* 眩しいのを抑えるためフェールで夜設定 */
		,hmi_step_contrast_(static_cast<int32_t>(CVhalColorManager::kColorImgAdjStepInitContrast))						/* 画質モード通知で0が送信されないよう、フェールセーフとして32を設定 */
		,hmi_step_brightness_(static_cast<int32_t>(CVhalColorManager::kColorImgAdjStepInitBrightness))					/* 画質モード通知で0が送信されないよう、フェールセーフとして32を設定 */
		,force_multisensory_step_contrast_(static_cast<int32_t>(CVhalColorManager::kColorImgAdjStepInitContrast))		/* 画質モード通知で0が送信されないよう、フェールセーフとして32を設定 */
		,force_multisensory_step_brightness_(static_cast<int32_t>(CVhalColorManager::kColorImgAdjStepInitBrightness))	/* 画質モード通知で0が送信されないよう、フェールセーフとして32を設定 */
		,hmi_color_type_(CVhalColorRecord::ColorType::COLOR_TYPE_THEME)
		,force_hmi_enable_(false)
		,force_multisensory_enable_(false)
		,display_rect_()
		,heacon_item_()
		,video_front_path_current_("")
		,camera_path_current_(""){}

		void SetVideoSourceId(const CVhalColorManager::VideoSourceId video_source_id) noexcept
		{
			video_source_id_ = video_source_id;
		}

		void SetStepContrast(const int32_t step_contrast) noexcept
		{
			step_contrast_ = step_contrast;
		}

		void SetStepBrightness(const int32_t step_brightness) noexcept
		{
			step_brightness_ = step_brightness;
		}

		void SetBackLight(const bool back_light) noexcept
		{
			back_light_ = back_light;
		}

		void SetMuteFrontDisp(const bool mute_frontdisp) noexcept
		{
			mute_frontdisp_ = mute_frontdisp;
		}

		void SetColorType(const CVhalColorRecord::ColorType color_type) noexcept
		{
			color_type_ = color_type;
		}

		void SetDayNight(const CVhalColorManager::DayNightType day_night) noexcept
		{
			day_night_ = day_night;
		}

		void SetThemeColor(const CVhalColorManager::ThemeColorType theme_color) noexcept
		{
			theme_color_ = theme_color;
		}

		void SetHmiStepContrast(const int32_t hmi_step_contrast) noexcept
		{
			hmi_step_contrast_ = hmi_step_contrast;
		}

		void SetHmiStepBrightness(const int32_t hmi_step_brightness) noexcept
		{
			hmi_step_brightness_ = hmi_step_brightness;
		}

		void SetForceMultisensoryStepContrast(const int32_t force_multisensory_step_contrast) noexcept
		{
			force_multisensory_step_contrast_ = force_multisensory_step_contrast;
		}

		void SetForceMultisensoryStepBrightness(const int32_t force_multisensory_step_brightness) noexcept
		{
			force_multisensory_step_brightness_ = force_multisensory_step_brightness;
		}

		void SetHmiColorType(const CVhalColorRecord::ColorType hmi_color_type) noexcept
		{
			hmi_color_type_ = hmi_color_type;
		}

		void SetForceHmiEnable(const bool force_hmi_enable) noexcept
		{
			force_hmi_enable_ = force_hmi_enable;
		}

		void SetForceMultisensoryEnable(const bool force_multisensory_enable) noexcept
		{
			force_multisensory_enable_ = force_multisensory_enable;
		}

		void SetVideoFrontPathCurrent(const std::string &video_front_path_current)
		{
			video_front_path_current_ = video_front_path_current;
		}

		void SetCameraPathCurrent(const std::string &camera_path_current)
		{
			camera_path_current_ = camera_path_current;
		}

		CVhalColorManager::VideoSourceId GetVideoSourceId(void) const noexcept
		{
			return video_source_id_;
		}

		int32_t GetStepContrast(void) const noexcept
		{
			return step_contrast_;
		}

		int32_t GetStepBrightness(void) const noexcept
		{
			return step_brightness_;
		}

		bool GetBackLight(void) const noexcept
		{
			return back_light_;
		}

		bool GetMuteFrontDisp(void) const noexcept
		{
			return mute_frontdisp_;
		}

		CVhalColorRecord::ColorType GetColorType(void) const noexcept
		{
			return color_type_;	
		}

		CVhalColorManager::DayNightType GetDayNight(void) const noexcept
		{
			return day_night_;
		}

		CVhalColorManager::ThemeColorType GetThemeColor(void) const noexcept
		{
			return theme_color_;
		}

		int32_t GetHmiStepContrast(void) const noexcept
		{
			return hmi_step_contrast_;
		}

		int32_t GetHmiStepBrightness(void) const noexcept
		{
			return hmi_step_brightness_;
		}

		int32_t GetForceMultisensoryStepContrast(void) const noexcept
		{
			return force_multisensory_step_contrast_;
		}

		int32_t GetForceMultisensoryStepBrightness(void) const noexcept
		{
			return force_multisensory_step_brightness_;
		}

		CVhalColorRecord::ColorType GetHmiColorType(void) const noexcept
		{
			return hmi_color_type_;
		}

		bool GetForceHmiEnable(void) const noexcept
		{
			return force_hmi_enable_;
		}

		bool GetForceMultisensoryEnable(void) const noexcept
		{
			return force_multisensory_enable_;
		}

		rectangleItem GetDisplayRect(void) const noexcept
		{
			return display_rect_;
		}

		void SetDisplayRect(const int32_t rect_x, const int32_t rect_y, const int32_t rect_w, const int32_t rect_h) noexcept
		{
			display_rect_.SetRect(rect_x, rect_y, rect_w, rect_h);
		}

		rectangleEnableItem GetHeaconItem(void) const noexcept
		{
			return heacon_item_;
		}

		void SetHeaconRect(const int32_t rect_x, const int32_t rect_y, const int32_t rect_w, const int32_t rect_h) noexcept
		{
			heacon_item_.SetRectItem(rect_x, rect_y, rect_w, rect_h);
		}

		std::string GetCameraPathCurrent(void) const noexcept
		{
			return camera_path_current_;
		}

		std::string GetFrontPathCurrent(void) const noexcept
		{
			return video_front_path_current_;
		}

	private:
		CVhalColorManager::VideoSourceId 	video_source_id_;						/* 映像ソースID(マスタ) */
		int32_t								step_contrast_;							/* コントラスト */
		int32_t								step_brightness_;						/* 明るさ */
		bool 								back_light_;							/* バックライト */
		bool								mute_frontdisp_;						/* 前席ディスプレイMute */
		CVhalColorRecord::ColorType			color_type_;							/* カラータイプ(DayNight/Theme) */
		CVhalColorManager::DayNightType		day_night_;								/* 昼夜モード種別 */
		CVhalColorManager::ThemeColorType	theme_color_;							/* テーマカラー種別 */
		int32_t								hmi_step_contrast_;						/* コントラスト(HMI) */
		int32_t								hmi_step_brightness_;					/* 明るさ(HMI) */
		int32_t								force_multisensory_step_contrast_;		/* コントラスト(多感覚連携) */
		int32_t								force_multisensory_step_brightness_;	/* 明るさ(多感覚連携) */
		CVhalColorRecord::ColorType			hmi_color_type_;						/* カラータイプ(DayNight/Theme)(HMI) */
		bool								force_hmi_enable_;						/* 強制HMI画質適用 */
		bool								force_multisensory_enable_;				/* 強制多感覚連携画質適用 */
		struct rectangleItem				display_rect_;							/* ディスプレイ出力矩形 */
		struct rectangleEnableItem			heacon_item_;							/* ヒーコン情報 */
		std::string							video_front_path_current_;				/* 前席映像パス */
		std::string							camera_path_current_;					/* カメラ映像パス */
	};
	
	uint8_t SetByteData(const uint8_t in_data, const uint8_t enable_bit, const uint8_t leftShift_bit) const;
	uint8_t GetVideoDispMode(const CVhalColorRecord::ColorType color_type) const;
	uint8_t GetHighData(const int32_t in_data) const;
	uint8_t GetLowData(const int32_t in_data) const;

	ColorSendItem reserve_data_;			/* 画質モード通知送信データの更新予約 */
	ColorSendItem current_data_;			/* 画質モード通知送信データ */

	mutable std::vector<uint8_t>	prev_micon_send_data_;	/* Previous send data to the micon */
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_COLOR_SEND_ITEM_QUALITY_H */
