/*******************************************************************************
    機能名称    ：  レイアウト制御映像出力サイズモジュール
    ファイル名称：  vhal_layout_video_setting.h
*******************************************************************************/
#ifndef	VHAL_LAYOUT_VIDEO_SETTING_H
#define	VHAL_LAYOUT_VIDEO_SETTING_H

#include <string>
#include <map>
#include "vhal_define.h"

namespace videohal
{

/* 映像出力先 */
using VideoOutputTarget = uint32_t;
static constexpr VideoOutputTarget VIDEO_OUTPUT_TARGET_NONE{0x00U};		/* NONE */
static constexpr VideoOutputTarget VIDEO_OUTPUT_TARGET_FRONT{0x01U};	/* 前席レイヤ */
static constexpr VideoOutputTarget VIDEO_OUTPUT_TARGET_REAR{0x02U};		/* 後席レイヤ */
static constexpr VideoOutputTarget VIDEO_OUTPUT_TARGET_CAMERA{0x04U};	/* カメラレイヤ（前席） */
static constexpr VideoOutputTarget VIDEO_OUTPUT_TARGET_IC{0x08U};		/* メータレイヤ */


/*****************************************************************************
 クラス名称：CVhalLayoutVideoSetting
 処理概要  ：画面レイアウト映像出力サイズの管理を行う。
*****************************************************************************/
class CVhalLayoutVideoSetting final {
public:
	struct VideoRectangleData {
		int32_t 	x_;
		int32_t 	y_;
		int32_t 	width_;
		int32_t 	height_;
		bool		set_;
	};

	CVhalLayoutVideoSetting(void);
	~CVhalLayoutVideoSetting(void);
  	CVhalLayoutVideoSetting(const CVhalLayoutVideoSetting& src) = delete;
	CVhalLayoutVideoSetting& operator=(const CVhalLayoutVideoSetting& src) & = delete;
	CVhalLayoutVideoSetting(CVhalLayoutVideoSetting&& src) = delete;
	CVhalLayoutVideoSetting& operator=(CVhalLayoutVideoSetting&& src) & = delete;

	/* 初期化処理 */
	static int32_t Initialize(void) noexcept;
	/* 終了処理 */
	void Finalize(void) noexcept;

	/* 管理対象の映像を登録 */
	int32_t RegisterVideoPath(const std::string& path, const uint32_t out_target);
	/* 映像出力サイズ設定 */
	int32_t SetVideoOutputSize(const std::string& path, const VideoOutputTarget out_target, const struct VideoRectangleData& output);
	/* 映像出力サイズ取得 */
	int32_t GetVideoOutputSize(const std::string& path, const VideoOutputTarget out_target, struct VideoRectangleData& output) const;

	/* 映像入力オリジナルサイズ設定 */
	int32_t SetVideoInputOriginalSize(const std::string& path, const VideoOutputTarget out_target, const struct VideoRectangleData& input);
	/* 映像入力オリジナルサイズクリア */	
	int32_t ClearVideoInputOriginalSize(const std::string& path, const VideoOutputTarget out_target);
	/* 映像入力パラメータ設定 */
	int32_t SetVideoInputParam(const std::string& path, const VideoOutputTarget out_target, const bool clipping_enable, const struct VideoRectangleData& input);
	/* 映像入力パラメータ取得 */
	int32_t GetVideoInputParam(const std::string& path, const VideoOutputTarget out_target, bool& clipping_enable, struct VideoRectangleData& input) const;

	/* 映像ワイドモードの設定 */
	int32_t SetVideoWideMode(const std::string& path, const VideoOutputTarget out_target, const uint32_t wide_mode);
	/* 映像ワイドモードの取得 */
	int32_t GetVideoWideMode(const std::string& path, const VideoOutputTarget out_target, uint32_t &wide_mode) const;
	/* ワイド設定の短形取得 */
	int32_t GetVideoWideRectangle(const std::string& path, const VideoOutputTarget out_target, const struct CVhalLayoutVideoSetting::VideoRectangleData &in_src, const struct CVhalLayoutVideoSetting::VideoRectangleData &in_dest, struct CVhalLayoutVideoSetting::VideoRectangleData &out_src, struct CVhalLayoutVideoSetting::VideoRectangleData &out_dest) const;

	/* 映像可視状態の設定 */
	int32_t SetVideoVisibility(const std::string& path, const VideoOutputTarget out_target, const bool visibility);
	/* 映像可視状態の取得 */
	int32_t GetVideoVisibility(const std::string& path, const VideoOutputTarget out_target, bool& visibility) const;

	/* 映像不透明度の設定 */
	int32_t SetVideoOpacity(const std::string& path, const VideoOutputTarget out_target, const uint32_t opacity);
	/* 映像不透明度の取得 */
	int32_t GetVideoOpacity(const std::string& path, const VideoOutputTarget out_target, uint32_t& opacity) const;

	/* 映像入力クリッピングサイズクリア */
	int32_t ClearVideoInputClippingSize(const std::string& path, const VideoOutputTarget out_target);

private:
	/* ワイド設定のスケーリング処理(NORMAL) */
	int32_t ScalingWideNormal(const struct CVhalLayoutVideoSetting::VideoRectangleData &in_src, const struct CVhalLayoutVideoSetting::VideoRectangleData &in_dest, struct CVhalLayoutVideoSetting::VideoRectangleData &out_src, struct CVhalLayoutVideoSetting::VideoRectangleData &out_dest) const;
	/* ワイド設定のスケーリング処理(STRETCHED) */
	int32_t ScalingWideStretched(const struct CVhalLayoutVideoSetting::VideoRectangleData &in_src, const struct CVhalLayoutVideoSetting::VideoRectangleData &in_dest, struct CVhalLayoutVideoSetting::VideoRectangleData &out_src, struct CVhalLayoutVideoSetting::VideoRectangleData &out_dest) const;
	/* ワイド設定のスケーリング処理(ZOOMED) */
	int32_t ScalingWideZoomed(const struct CVhalLayoutVideoSetting::VideoRectangleData &in_src, const struct CVhalLayoutVideoSetting::VideoRectangleData &in_dest, struct CVhalLayoutVideoSetting::VideoRectangleData &out_src, struct CVhalLayoutVideoSetting::VideoRectangleData &out_dest) const;

	/* 映像パス名、映像出力先を元に映像識別名を取得 */
	int32_t GetVideoPathIdentifier(const std::string& path, const VideoOutputTarget out_target, std::string& identifier) const;

	/* 出力サイズ構造体（内部管理用） */
	struct VideoRectangleRecord {
		int32_t 	x_;
		int32_t 	y_;
		int32_t 	w_;
		int32_t 	h_;
		bool		set_;		/* 一度以上矩形設定されたらtrue */
	};
	/* サーフェスレコード構造体（内部管理用） */
	struct VideoSurfaceRecord {
	public:
		VideoSurfaceRecord(const int32_t x, const int32_t y, const int32_t w, const int32_t h, const uint32_t wide) noexcept
			:input_original_{x, y, w, h, false}
			,input_clipping_{x, y, w, h, false}
			,output_{x, y, w, h, false}
			,wide_mode_(wide)
			,visibility_(false)
			,opacity_(VHAL_OPACITY_VALUE_MAX)
			,clipping_enable_(false)
		{}

		void SetInputOriginalX(const int32_t x) noexcept
		{
			input_original_.x_ = x;
		}

		void SetInputOriginalY(const int32_t y) noexcept
		{
			input_original_.y_ = y;
		}

		void SetInputOriginalW(const int32_t w) noexcept
		{
			input_original_.w_ = w;
		}

		void SetInputOriginalH(const int32_t h) noexcept
		{
			input_original_.h_ = h;
		}

		void SetInputOriginalSet(const bool set) noexcept
		{
			input_original_.set_ = set;
		}

		void SetInputClipping(const VideoRectangleRecord input_clipping) noexcept
		{
			input_clipping_ = input_clipping;
		}

		void SetInputClippingX(const int32_t x) noexcept
		{
			input_clipping_.x_ = x;
		}

		void SetInputClippingY(const int32_t y) noexcept
		{
			input_clipping_.y_ = y;
		}

		void SetInputClippingW(const int32_t w) noexcept
		{
			input_clipping_.w_ = w;
		}

		void SetInputClippingH(const int32_t h) noexcept
		{
			input_clipping_.h_ = h;
		}

		void SetInputClippingSet(const bool set) noexcept
		{
			input_clipping_.set_ = set;
		}

		void SetOutputX(const int32_t x) noexcept
		{
			output_.x_ = x;
		}

		void SetOutputY(const int32_t y) noexcept
		{
			output_.y_ = y;
		}

		void SetOutputW(const int32_t w) noexcept
		{
			output_.w_ = w;
		}

		void SetOutputH(const int32_t h) noexcept
		{
			output_.h_ = h;
		}

		void SetOutputSet(const bool set) noexcept
		{
			output_.set_ = set;
		}

		void SetWideMode(const uint32_t wide_mode) noexcept
		{
			wide_mode_ = wide_mode;
		}

		void SetVisibility(const bool visibility) noexcept
		{
			visibility_ = visibility;
		}

		void SetOpacity(const uint32_t opacity) noexcept
		{
			opacity_ = opacity;
		}

		void SetClippingEnable(const bool clipping_enable) noexcept
		{
			clipping_enable_ = clipping_enable;
		}

		VideoRectangleRecord GetInputOriginal(void) const noexcept
		{
			return input_original_;
		}

		int32_t GetInputOriginalX(void) const noexcept
		{
			return input_original_.x_;
		}

		int32_t GetInputOriginalY(void) const noexcept
		{
			return input_original_.y_;
		}

		int32_t GetInputOriginalW(void) const noexcept
		{
			return input_original_.w_;
		}

		int32_t GetInputOriginalH(void) const noexcept
		{
			return input_original_.h_;
		}

		bool GetInputOriginalSet(void) const noexcept
		{
			return input_original_.set_;
		}

		int32_t GetInputClippingX(void) const noexcept
		{
			return input_clipping_.x_;
		}

		int32_t GetInputClippingY(void) const noexcept
		{
			return input_clipping_.y_;
		}

		int32_t GetInputClippingW(void) const noexcept
		{
			return input_clipping_.w_;
		}

		int32_t GetInputClippingH(void) const noexcept
		{
			return input_clipping_.h_;
		}

		bool GetInputClippingSet(void) const noexcept
		{
			return input_clipping_.set_;
		}

		int32_t GetOutputX(void) const noexcept
		{
			return output_.x_;
		}

		int32_t GetOutputY(void) const noexcept
		{
			return output_.y_;
		}

		int32_t GetOutputW(void) const noexcept
		{
			return output_.w_;
		}

		int32_t GetOutputH(void) const noexcept
		{
			return output_.h_;
		}

		bool GetOutputSet(void) const noexcept
		{
			return output_.set_;
		}

		uint32_t GetWideMode(void) const noexcept
		{
			return wide_mode_;
		}

		bool GetVisibility(void) const noexcept
		{
			return visibility_;
		}

		uint32_t GetOpacity(void) const noexcept
		{
			return opacity_;
		}

		bool GetClippingEnable(void) const noexcept
		{
			return clipping_enable_;
		}

	private:
		struct VideoRectangleRecord input_original_;		/* オリジナルサイズ */
		struct VideoRectangleRecord input_clipping_;		/* クリッピングサイズ */
		struct VideoRectangleRecord output_;
		uint32_t wide_mode_;
		bool visibility_;
		uint32_t opacity_;
		bool clipping_enable_;								/* クリッピング有効フラグ */
	};
	/* <映像識別名（映像パス+"_出力先識別子"）, サーフェスレコード> */
	using CVideoSurfaceList = std::map<std::string, struct VideoSurfaceRecord>;
	CVideoSurfaceList	video_surface_list_;
};


} /* namespace videohal */


#endif	/* #ifndef	VHAL_LAYOUT_VIDEO_SETTING_H */
