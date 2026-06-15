/*******************************************************************************
    機能名称    ：  MISC画質モード共通アイテムモジュール
    ファイル名称：  vhal_color_adjust_item_quality.h
*******************************************************************************/
#ifndef	VHAL_COLOR_ADJUST_ITEM_QUALITY_H
#define	VHAL_COLOR_ADJUST_ITEM_QUALITY_H

namespace videohal
{

/*****************************************************************************
 クラス名称：CVhalColorAdjustItemQuality
 処理概要  ：MISC画質モード共通アイテム
*****************************************************************************/
class CVhalColorAdjustItemQuality {
private:
	CVhalColorAdjustItemQuality(void) = default;
	~CVhalColorAdjustItemQuality(void) = default;
  	CVhalColorAdjustItemQuality(const CVhalColorAdjustItemQuality& src) = delete;
	CVhalColorAdjustItemQuality& operator=(const CVhalColorAdjustItemQuality& src) & = delete;
	CVhalColorAdjustItemQuality(CVhalColorAdjustItemQuality&& src) = delete;
	CVhalColorAdjustItemQuality& operator=(CVhalColorAdjustItemQuality&& src) & = delete;

public:
	/* 映像種別 */
	enum class VideoType : uint8_t {
		VTYPE_OTHER					= 0x00U,			/* 0h:その他 */
		VTYPE_CAM					= 0x01U,			/* 1h:カメラ */
		VTYPE_DTV					= 0x02U,			/* 2h:デジタルTV */
		VTYPE_HDMI					= 0x03U,			/* 3h:HDMI */
		VTYPE_MULTISENSORY			= 0x04U,			/* 4h:多感覚連携 */
	};

	/* 映像表示モード */
	enum class VideoDispMode : uint8_t {
		VDISP_NORMAL_DAY			= 0x00U,			/* 0h:昼画 */
		VDISP_NORMAL_NIGHT			= 0x01U,			/* 1h:夜画 */
		VDISP_NORMAL_FORCED_DAY		= 0x02U,			/* 2h:強制昼画ONでの昼画 */
		VDISP_COLOR_AUTO_LIGHT		= 0x03U,			/* 3h:昼画(Light color Auto) */
		VDISP_COLOR_AUTO_DARK		= 0x04U,			/* 4h:夜画(Dark color Auto) */
		VDISP_COLOR_SELECT_LIGHT	= 0x05U,			/* 5h:夜画(Light color) */
		VDISP_COLOR_SELECT_DARK		= 0x06U,			/* 6h:昼画(Dark color) */
	};
	
	/* 送受信データ バイト番号 */
	enum class AdjustItemNo : uint8_t {
		POS_OPC = 0U,											/* OPC(sub_type) */
		POS_VTYPE_MUTE,											/* 映像種別(bit7-4) + 0固定(bit3-2) + Mute(bit1)+ 0固定(bit0) */
		POS_CONTRAST,											/* コントラスト */
		POS_BRIGHTNESS,											/* 明るさ */
		POS_DUMMY4,
		POS_DUMMY5,
		POS_VDISP,												/* 映像表示モード */
		POS_DUMMY7,
		POS_WINDOW_W_HIGH,										/* 小窓サイズ(水平幅X - 上位) */
		POS_WINDOW_W_LOW,										/* 小窓サイズ(水平幅X - 下位) */
		POS_WINDOW_H_HIGH,										/* 小窓サイズ(垂直幅Y - 上位) */
		POS_WINDOW_H_LOW,										/* 小窓サイズ(垂直幅Y - 下位) */
		POS_WINDOW_X_HIGH,										/* 小窓開始位置(位置N - 上位) */
		POS_WINDOW_X_LOW,										/* 小窓開始位置(位置N - 下位) */
		POS_WINDOW_Y_HIGH,										/* 小窓開始位置(位置M - 上位) */
		POS_WINDOW_Y_LOW,										/* 小窓開始位置(位置M - 下位) */
		POS_WINDOW_VTYPE,										/* 映像種別(小窓) */
		POS_WINDOW_CONTRAST,									/* コントラスト(小窓) */
		POS_WINDOW_BRIGHTNESS,									/* 明るさ(小窓):BEVstep3では削除 */
		POS_DUMMY19,
		POS_DUMMY20,
		POS_DUMMY21,
		POS_DUMMY22,
		POS_DATA_TYPE,											/* data_type */
		POS_MAX,												/* 最大値 */
	};
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_COLOR_ADJUST_ITEM_QUALITY_H */
