/*******************************************************************************
    機能名称    ：  レイアウト制御映像出力サイズモジュール
    ファイル名称：  vhal_layout_video_setting.cpp
*******************************************************************************/
#include "vhal_layout_video_setting.h"
#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_round_cast.h"

namespace videohal
{

namespace
{

/* 映像パス識別子（映像パス名のみだと前後席同じ名前があるため、識別子を付加） */
const std::string kVhalPathNameAppendFront{"_front"};		/* 前席 */
const std::string kVhalPathNameAppendCamera{"_camera"};		/* カメラ */
const std::string kVhalPathNameAppendRear{"_rear"};			/* 後席 */
const std::string kVhalPathNameAppendIc{"_ic"};				/* InstrumentCluster */

}	// namespace


/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalLayoutVideoSetting::CVhalLayoutVideoSetting(void)
{
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalLayoutVideoSetting::~CVhalLayoutVideoSetting(void)
{
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化処理
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutVideoSetting::Initialize(void) noexcept
{
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	終了処理
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalLayoutVideoSetting::Finalize(void) noexcept
{
	video_surface_list_.clear();
}

/*****************************************************************************
 処理概要：	管理対象の映像を登録
 引数    ：	const std::string& path		(i)映像パス名
         ：	uint32_t out_target			(i)出力先（or指定可）
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutVideoSetting::RegisterVideoPath(const std::string& path, const uint32_t out_target)
{
	int32_t result{VHAL_SUCCESS};
	struct VideoSurfaceRecord record_wideon{0,0,1280,720, VHAL_WIDE_MODE_NORMAL};
	struct VideoSurfaceRecord record_wideoff{0,0,1280,720, VHAL_WIDE_MODE_INVALID};

	if ( VIDEO_OUTPUT_TARGET_NONE == out_target )
	{
		result = VHAL_ERR_PARAM;
	}
	else
	{
		if ( VIDEO_OUTPUT_TARGET_FRONT == (out_target & VIDEO_OUTPUT_TARGET_FRONT) )
		{
			std::string identifier{};
			const int32_t ret{GetVideoPathIdentifier(path, VIDEO_OUTPUT_TARGET_FRONT, identifier)};
			if ( VHAL_SUCCESS == ret )
			{
				(void)video_surface_list_.insert(std::make_pair(identifier, record_wideon));
			}
		}
		if ( VIDEO_OUTPUT_TARGET_CAMERA == (out_target & VIDEO_OUTPUT_TARGET_CAMERA) )
		{
			std::string identifier{};
			const int32_t ret{GetVideoPathIdentifier(path, VIDEO_OUTPUT_TARGET_CAMERA, identifier)};
			if ( VHAL_SUCCESS == ret )
			{
				(void)video_surface_list_.insert(std::make_pair(identifier, record_wideon));
			}
		}
		if ( VIDEO_OUTPUT_TARGET_REAR == (out_target & VIDEO_OUTPUT_TARGET_REAR) )
		{
			std::string identifier{};
			const int32_t ret{GetVideoPathIdentifier(path, VIDEO_OUTPUT_TARGET_REAR, identifier)};
			if ( VHAL_SUCCESS == ret )
			{
				(void)video_surface_list_.insert(std::make_pair(identifier, record_wideon));
			}
		}
		if ( VIDEO_OUTPUT_TARGET_IC == (out_target & VIDEO_OUTPUT_TARGET_IC) )
		{
			std::string identifier{};
			const int32_t ret{GetVideoPathIdentifier(path, VIDEO_OUTPUT_TARGET_IC, identifier)};
			if ( VHAL_SUCCESS == ret )
			{
				(void)video_surface_list_.insert(std::make_pair(identifier, record_wideoff));
			}
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	映像出力サイズ設定
 引数    ：	const std::string& path					(i)映像パス名
         ：	VideoOutputTarget out_target			(i)映像出力先（or指定可）
         ：	const struct VideoRectangleData& output	(i)出力サイズ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutVideoSetting::SetVideoOutputSize(const std::string& path, const VideoOutputTarget out_target, const struct VideoRectangleData& output)
{
	int32_t ret{VHAL_SUCCESS};
	std::string identifier{};

	ret = GetVideoPathIdentifier(path, out_target, identifier);
	if ( VHAL_SUCCESS == ret )
	{
		const auto it_record = video_surface_list_.find(identifier);
		if ( it_record != video_surface_list_.end() )
		{
			it_record->second.SetOutputX(output.x_);
			it_record->second.SetOutputY(output.y_);
			it_record->second.SetOutputW(output.width_);
			it_record->second.SetOutputH(output.height_);
			it_record->second.SetOutputSet(true);			/* 一度以上矩形設定されたらtrue */
		}
		else
		{
			/* not found */
			ret = VHAL_ERR_PARAM;
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	映像出力サイズ取得
 引数    ：	const std::string& path				(i)映像パス名
         ：	VideoOutputTarget out_target		(i)映像出力先
         ：	struct VideoRectangleData& output	(o)出力サイズ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutVideoSetting::GetVideoOutputSize(const std::string& path, const VideoOutputTarget out_target, struct VideoRectangleData& output) const
{
	int32_t ret{VHAL_SUCCESS};
	std::string identifier{};

	ret = GetVideoPathIdentifier(path, out_target, identifier);
	if ( VHAL_SUCCESS == ret )
	{
		const auto it_record = video_surface_list_.find(identifier);
		if ( it_record != video_surface_list_.end() )
		{
			output.x_      = it_record->second.GetOutputX();
			output.y_      = it_record->second.GetOutputY();
			output.width_  = it_record->second.GetOutputW();
			output.height_ = it_record->second.GetOutputH();
			output.set_    = it_record->second.GetOutputSet();
		}
		else
		{
			/* not found */
			ret = VHAL_ERR_PARAM;
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	映像入力オリジナルサイズ設定
 引数    ：	const std::string& path					(i)映像パス名
         ：	VideoOutputTarget out_target			(i)映像出力先
         ：	const struct VideoRectangleData& input	(i)入力サイズ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutVideoSetting::SetVideoInputOriginalSize(const std::string& path, const VideoOutputTarget out_target, const struct VideoRectangleData& input)
{
	int32_t ret{VHAL_SUCCESS};
	std::string identifier{};

	ret = GetVideoPathIdentifier(path, out_target, identifier);
	if ( VHAL_SUCCESS == ret )
	{
		const auto it_record = video_surface_list_.find(identifier);
		if ( it_record != video_surface_list_.end() )
		{
			/* オリジナルサイズ設定（初回更新のみ有効） */
			if (it_record->second.GetInputOriginalSet())
			{
				/* 既に設定済みの場合はスキップ */
				VHAL_LOGI("already set input(%d,%d,%d,%d), original(%d,%d,%d,%d)", input.x_, input.y_, input.width_, input.height_
					, it_record->second.GetInputOriginalX(), it_record->second.GetInputOriginalY(), it_record->second.GetInputOriginalW(), it_record->second.GetInputOriginalH());
			}
			else
			{
				/* オリジナルサイズの有効値チェック */
				if ((0 == input.x_) && (0 == input.y_) && (0 < input.width_) && (0 < input.height_))
				{
					it_record->second.SetInputOriginalX(input.x_);
					it_record->second.SetInputOriginalY(input.y_);
					it_record->second.SetInputOriginalW(input.width_);
					it_record->second.SetInputOriginalH(input.height_);
					it_record->second.SetInputOriginalSet(true);			/* 一度以上矩形設定されたらtrue */
				}
				else
				{
					VHAL_LOGE("parameter error. input(%d,%d,%d,%d)", input.x_, input.y_, input.width_, input.height_);
					ret = VHAL_ERR_PARAM;
				}
			}
		}
		else
		{
			/* not found */
			ret = VHAL_ERR_PARAM;
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	映像入力オリジナルサイズクリア
 引数    ：	const std::string& path					(i)映像パス名
         ：	const VideoOutputTarget out_target		(i)映像出力先
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutVideoSetting::ClearVideoInputOriginalSize(const std::string& path, const VideoOutputTarget out_target)
{
	int32_t ret{VHAL_SUCCESS};
	std::string identifier{};

	ret = GetVideoPathIdentifier(path, out_target, identifier);
	if ( VHAL_SUCCESS == ret )
	{
		const auto it_record = video_surface_list_.find(identifier);
		if ( it_record != video_surface_list_.end() )
		{
			it_record->second.SetInputOriginalX(0);
			it_record->second.SetInputOriginalY(0);
			it_record->second.SetInputOriginalW(0);
			it_record->second.SetInputOriginalH(0);
			it_record->second.SetInputOriginalSet(false);
		}
		else
		{
			VHAL_LOGE("parameter error. input(0,0,0,0)");
			ret = VHAL_ERR_PARAM;
		}
	}
	return ret;
}

/*****************************************************************************
 処理概要：	映像入力パラメータ設定
 引数    ：	const std::string& path					(i)映像パス名
         ：	VideoOutputTarget out_target			(i)映像出力先
         ：	bool clipping_enable					(i)クリッピング有効フラグ
         ：	const struct VideoRectangleData& input	(i)入力サイズ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutVideoSetting::SetVideoInputParam(const std::string& path, const VideoOutputTarget out_target, const bool clipping_enable, const struct VideoRectangleData& input)
{
	int32_t ret{VHAL_SUCCESS};
	std::string identifier{};

	ret = GetVideoPathIdentifier(path, out_target, identifier);
	if ( VHAL_SUCCESS == ret )
	{
		const auto it_record = video_surface_list_.find(identifier);
		if ( it_record != video_surface_list_.end() )
		{
			if (clipping_enable)
			{
				if ( it_record->second.GetInputOriginalSet() )
				{
					/* オリジナルサイズ内に収まるか有効値チェック */
					if ( ((0 <= input.x_) && (input.x_ < it_record->second.GetInputOriginalW()))
					  && ((0 <= input.y_) && (input.y_ < it_record->second.GetInputOriginalH())) 
					  && ((0 < input.width_) && (input.width_ <= (it_record->second.GetInputOriginalW() - input.x_))) 
					  && ((0 < input.height_) && (input.height_ <= (it_record->second.GetInputOriginalH() - input.y_))) )
					{
					}
					else
					{
						VHAL_LOGE("parameter error. clipping(%d,%d,%d,%d), original(%d,%d,%d,%d)", input.x_, input.y_, input.width_, input.height_
							, it_record->second.GetInputOriginalX(), it_record->second.GetInputOriginalY(), it_record->second.GetInputOriginalW(), it_record->second.GetInputOriginalH());
						ret = VHAL_ERR_PARAM;
					}
				}
				else
				{
					/* If the original size is not available, check only the minimum value */
					if ((0 <= input.x_) && (0 <= input.y_) && (0 < input.width_) && (0 < input.height_))
					{
					}
					else
					{
						VHAL_LOGE("parameter error. clipping(%d,%d,%d,%d)", input.x_, input.y_, input.width_, input.height_);
						ret = VHAL_ERR_PARAM;
					}
				}

				if ( VHAL_SUCCESS == ret )
				{
					/* クリッピング有効フラグ設定 */
					it_record->second.SetClippingEnable(clipping_enable);
					/* クリッピングサイズ設定 */
					it_record->second.SetInputClippingX(input.x_);
					it_record->second.SetInputClippingY(input.y_);
					it_record->second.SetInputClippingW(input.width_);
					it_record->second.SetInputClippingH(input.height_);
					it_record->second.SetInputClippingSet(true);			/* 一度以上矩形設定されたらtrue */
				}
			}
			else
			{
				/* クリッピング有効フラグ設定 */
				it_record->second.SetClippingEnable(clipping_enable);
				/* クリッピングサイズ設定（オリジナルサイズを適用） */
				it_record->second.SetInputClipping(it_record->second.GetInputOriginal());
			}
		}
		else
		{
			/* not found */
			ret = VHAL_ERR_PARAM;
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	映像入力パラメータ取得
 引数    ：	const std::string& path				(i)映像パス名
         ：	VideoOutputTarget out_target		(i)映像出力先
         ：	bool& clipping_enable				(o)クリッピング有効フラグ
         ：	struct VideoRectangleData& input	(o)入力サイズ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutVideoSetting::GetVideoInputParam(const std::string& path, const VideoOutputTarget out_target, bool& clipping_enable, struct VideoRectangleData& input) const
{
	int32_t ret{VHAL_SUCCESS};
	std::string identifier{};

	ret = GetVideoPathIdentifier(path, out_target, identifier);
	if ( VHAL_SUCCESS == ret )
	{
		const auto it_record = video_surface_list_.find(identifier);
		if ( it_record != video_surface_list_.end() )
		{
			/* クリッピング有効フラグ */
			clipping_enable = it_record->second.GetClippingEnable();
			if (clipping_enable)
			{
				/* クリッピングサイズ */
				input.x_      = it_record->second.GetInputClippingX();
				input.y_      = it_record->second.GetInputClippingY();
				input.width_  = it_record->second.GetInputClippingW();
				input.height_ = it_record->second.GetInputClippingH();
				input.set_    = it_record->second.GetInputClippingSet();
			}
			else
			{
				/* オリジナルサイズ */
				input.x_      = it_record->second.GetInputOriginalX();
				input.y_      = it_record->second.GetInputOriginalY();
				input.width_  = it_record->second.GetInputOriginalW();
				input.height_ = it_record->second.GetInputOriginalH();
				input.set_    = it_record->second.GetInputOriginalSet();
			}
		}
		else
		{
			/* not found */
			ret = VHAL_ERR_PARAM;
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	映像ワイドモードの設定
 引数    ：	const std::string&	path		(i)映像パス名
            VideoOutputTarget	out_target	(i)映像出力先
            uint32_t			wide_mode	(i)ワイド設定(カメラ以外:0-2, カメラ:0-3)
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutVideoSetting::SetVideoWideMode(const std::string& path, const VideoOutputTarget out_target, const uint32_t wide_mode)
{
	int32_t ret{VHAL_SUCCESS};
	std::string identifier{};

	ret = GetVideoPathIdentifier(path, out_target, identifier);
	if ( VHAL_SUCCESS == ret )
	{
		const auto it_record = video_surface_list_.find(identifier);
		if ( it_record != video_surface_list_.end() )
		{
			/* ワイド設定値チェック */
			if ( out_target == VIDEO_OUTPUT_TARGET_CAMERA )
			{
				if ( VHAL_WIDE_MODE_INVALID < wide_mode )
				{
					VHAL_LOGE("wide_mode over. wide_mode=%d, path=%s, output target=0x%x", wide_mode, path.c_str(), out_target);
					ret = VHAL_ERR_PARAM;
				}
			}
			else
			{
				if ( VHAL_WIDE_MODE_ZOOMED < wide_mode )
				{
					VHAL_LOGE("wide_mode over. wide_mode=%d, path=%s, output target=0x%x", wide_mode, path.c_str(), out_target);
					ret = VHAL_ERR_PARAM;
				}
			}

			if ( VHAL_SUCCESS == ret )
			{
				VHAL_LOGD("path=%s, output target=0x%x, wide_mode=%d", path.c_str(), out_target, wide_mode);
				it_record->second.SetWideMode(wide_mode);
			}
		}
		else
		{
			/* not found(not support) */
			VHAL_LOGE("not support. path=%s, output target=0x%x, wide_mode=%d", path.c_str(), out_target, wide_mode);
			ret = VHAL_ERR_PARAM;
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	映像ワイドモードの取得
 引数    ：	const std::string&	path		(i)映像パス名
            VideoOutputTarget	out_target	(i)映像出力先
            uint32_t			wide_mode	(o)ワイド設定(カメラ以外:0-2, カメラ:0-3)
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutVideoSetting::GetVideoWideMode(const std::string& path, const VideoOutputTarget out_target, uint32_t &wide_mode) const
{
	int32_t ret{VHAL_SUCCESS};

	if ( path.empty() )
	{
		wide_mode = VHAL_WIDE_MODE_NORMAL;
	}
	else
	{
		std::string identifier{};
		ret = GetVideoPathIdentifier(path, out_target, identifier);
		if ( VHAL_SUCCESS == ret )
		{
			const auto it_record = video_surface_list_.find(identifier);
			if ( it_record != video_surface_list_.end() )
			{
				if ( out_target == VIDEO_OUTPUT_TARGET_CAMERA )
				{
					if ( it_record->second.GetWideMode() <= VHAL_WIDE_MODE_INVALID )
					{
						wide_mode = it_record->second.GetWideMode();
					}
					else
					{
						/* wide not support */
						VHAL_LOGE("not support. path=%s, output target=0x%x", path.c_str(), out_target);
						ret = VHAL_ERR_PARAM;
					}
				}
				else
				{
					if ( it_record->second.GetWideMode() < VHAL_WIDE_MODE_INVALID )
					{
						wide_mode = it_record->second.GetWideMode();
					}
					else
					{
						/* wide not support */
						VHAL_LOGE("not support. path=%s, output target=0x%x", path.c_str(), out_target);
						ret = VHAL_ERR_PARAM;
					}
				}
			}
			else
			{
				/* not found(not support) */
				VHAL_LOGE("not support. path=%s, output target=0x%x", path.c_str(), out_target);
				ret = VHAL_ERR_PARAM;
			}
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	ワイド設定の短形取得
           	[POST21MM-53784]カメラのみ初回出力サイズ設定までWIDE設定をNORMALとし、センタリングを実施しない。
           	                出力サイズ設定後はWIDE設定をINVALIDとするため、指定座標へ変更可能となります。
 引数    ：	const std::string&									path		(i)映像パス名
            VideoOutputTarget									out_target	(i)映像出力先
           	struct CVhalLayoutVideoSetting::VideoRectangleData 	&in_src		(i)ワイド設定前のソース短形
           	struct CVhalLayoutVideoSetting::VideoRectangleData 	&in_dest	(i)ワイド設定前のデスティネーション短形
           	struct CVhalLayoutVideoSetting::VideoRectangleData 	&out_src	(o)ワイド設定後のソース短形
           	struct CVhalLayoutVideoSetting::VideoRectangleData 	&out_dest	(o)ワイド設定後のデスティネーション短形
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutVideoSetting::GetVideoWideRectangle(const std::string& path, const VideoOutputTarget out_target, const struct CVhalLayoutVideoSetting::VideoRectangleData &in_src, const struct CVhalLayoutVideoSetting::VideoRectangleData &in_dest, struct CVhalLayoutVideoSetting::VideoRectangleData &out_src, struct CVhalLayoutVideoSetting::VideoRectangleData &out_dest) const
{
	int32_t ret{VHAL_SUCCESS};
	std::string identifier{};

	ret = GetVideoPathIdentifier(path, out_target, identifier);
	if ( VHAL_SUCCESS == ret )
	{
		/* Check if input src/dest is valid */
		if ((0 == in_src.width_) || (0 == in_src.height_))
		{
			VHAL_LOGD("in_src(%d,%d)", in_src.width_,in_src.height_);

			/* ワイド設定前のソース短形の幅または高さが0の場合はワイド設定不可の為、入力サイズをそのまま出力サイズとする。 */
			out_src = in_src;
			out_dest = in_dest;
		}
		else if ((0 == in_dest.width_) || (0 == in_dest.height_))
		{
			VHAL_LOGD("in_dest(%d,%d)", in_dest.width_, in_dest.height_);

			/* ワイド設定前のデスティネーション短形の幅または高さが0の場合はワイド設定不可の為、入力サイズをそのまま出力サイズとする。 */
			out_src = in_src;
			out_dest = in_dest;
		}
		else
		{
			uint32_t wide_mode{VHAL_WIDE_MODE_INVALID};
			const auto it_record = video_surface_list_.find(identifier);
			if ( it_record != video_surface_list_.end() )
			{
				wide_mode = it_record->second.GetWideMode();
			}

			/* ワイド設定のスケーリング処理 */
			switch( wide_mode )
			{
				case VHAL_WIDE_MODE_NORMAL:
					VHAL_LOGD("output path=%s, target=0x%x, wide_mode=%d", path.c_str(), out_target, wide_mode);
					ret = ScalingWideNormal(in_src, in_dest, out_src, out_dest);
					/* [POST21MM-53784]カメラのみセンタリングを実施しない */
					if (VIDEO_OUTPUT_TARGET_CAMERA == out_target)
					{
						out_dest.x_ = 0;
						out_dest.y_ = 0;
					}
					break;

				case VHAL_WIDE_MODE_STRETCHED:
					VHAL_LOGD("output path=%s, target=0x%x, wide_mode=%d", path.c_str(), out_target, wide_mode);
					ret = ScalingWideStretched(in_src, in_dest, out_src, out_dest);
					break;

				case VHAL_WIDE_MODE_ZOOMED:
					VHAL_LOGD("output path=%s, target=0x%x, wide_mode=%d", path.c_str(), out_target, wide_mode);
					ret = ScalingWideZoomed(in_src, in_dest, out_src, out_dest);
					break;

				default:
					/* ワイド設定未サポートの場合は入力サイズをそのまま出力サイズとする。 */
					VHAL_LOGD("not support. path=%s, target=0x%x wide_mode=%d", path.c_str(), out_target, wide_mode);
					out_src = in_src;
					out_dest = in_dest;
					break;
			}
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	映像可視状態の設定
 引数    ：	const std::string&	path		(i)映像パス名
            VideoOutputTarget	out_target	(i)映像出力先
            bool				visibility	(i)可視状態
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutVideoSetting::SetVideoVisibility(const std::string& path, const VideoOutputTarget out_target, const bool visibility)
{
	int32_t ret{VHAL_SUCCESS};
	std::string identifier{};

	ret = GetVideoPathIdentifier(path, out_target, identifier);
	if ( VHAL_SUCCESS == ret )
	{
		const auto it_record = video_surface_list_.find(identifier);
		if ( it_record != video_surface_list_.end() )
		{
			VHAL_LOGD("path=%s, output target=0x%x, visibility=%d", path.c_str(), out_target, visibility);
			it_record->second.SetVisibility(visibility);
		}
		else
		{
			/* not found(not support) */
			VHAL_LOGE("not support. path=%s, output target=0x%x, visibility=%d", path.c_str(), out_target, visibility);
			ret = VHAL_ERR_PARAM;
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	映像可視状態の取得
 引数    ：	const std::string&	path		(i)映像パス名
            VideoOutputTarget	out_target	(i)映像出力先
            bool&				visibility	(o)可視状態
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutVideoSetting::GetVideoVisibility(const std::string& path, const VideoOutputTarget out_target, bool& visibility) const
{
	int32_t ret{VHAL_SUCCESS};
	std::string identifier{};

	ret = GetVideoPathIdentifier(path, out_target, identifier);
	if ( VHAL_SUCCESS == ret )
	{
		const auto it_record = video_surface_list_.find(identifier);
		if ( it_record != video_surface_list_.end() )
		{
			visibility = it_record->second.GetVisibility();
		}
		else
		{
			/* not found(not support) */
			VHAL_LOGE("not support. path=%s, output target=0x%x", path.c_str(), out_target);
			ret = VHAL_ERR_PARAM;
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	映像不透明度の設定
 引数    ：	const std::string&	path		(i)映像パス名
            VideoOutputTarget	out_target	(i)映像出力先
            uint32_t			opacity		(i)不透明度(0-100)
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutVideoSetting::SetVideoOpacity(const std::string& path, const VideoOutputTarget out_target, const uint32_t opacity)
{
	std::string identifier{};

	/* 不透明度チェック */
	if( VHAL_OPACITY_VALUE_MAX < opacity )
	{
		VHAL_LOGE("opacity over. opacity=%d, path=%s, output target=0x%x", opacity, path.c_str(), out_target);
		return VHAL_ERR_PARAM;
	}

	int32_t ret{GetVideoPathIdentifier(path, out_target, identifier)};
	if ( VHAL_SUCCESS == ret )
	{
		const auto it_record = video_surface_list_.find(identifier);
		if ( it_record != video_surface_list_.end() )
		{
			VHAL_LOGD("path=%s, output target=0x%x, opacity=%d", path.c_str(), out_target, opacity);
			it_record->second.SetOpacity(opacity);
		}
		else
		{
			/* not found(not support) */
			VHAL_LOGE("not support. path=%s, output target=0x%x, opacity=%d", path.c_str(), out_target, opacity);
			ret = VHAL_ERR_PARAM;
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	映像不透明度の取得
 引数    ：	const std::string&	path		(i)映像パス名
            VideoOutputTarget	out_target	(i)映像出力先
            uint32_t&			opacity		(o)不透明度(0-100)
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutVideoSetting::GetVideoOpacity(const std::string& path, const VideoOutputTarget out_target, uint32_t& opacity) const
{
	int32_t ret{VHAL_SUCCESS};
	std::string identifier{};

	ret = GetVideoPathIdentifier(path, out_target, identifier);
	if ( VHAL_SUCCESS == ret )
	{
		const auto it_record = video_surface_list_.find(identifier);
		if ( it_record != video_surface_list_.end() )
		{
			opacity = it_record->second.GetOpacity();
		}
		else
		{
			/* not found(not support) */
			VHAL_LOGE("not support. path=%s, output target=0x%x", path.c_str(), out_target);
			ret = VHAL_ERR_PARAM;
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	映像入力クリッピングサイズクリア
 引数    ：	const std::string& path					(i)映像パス名
         ：	const VideoOutputTarget out_target		(i)映像出力先
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutVideoSetting::ClearVideoInputClippingSize(const std::string& path, const VideoOutputTarget out_target)
{
	int32_t ret{VHAL_SUCCESS};
	std::string identifier{};

	ret = GetVideoPathIdentifier(path, out_target, identifier);
	if ( VHAL_SUCCESS == ret )
	{
		const auto it_record = video_surface_list_.find(identifier);
		if ( it_record != video_surface_list_.end() )
		{
			it_record->second.SetInputClippingX(0);
			it_record->second.SetInputClippingY(0);
			it_record->second.SetInputClippingW(0);
			it_record->second.SetInputClippingH(0);
			it_record->second.SetInputClippingSet(false);
		}
		else
		{
			VHAL_LOGE("parameter error. input(0,0,0,0)");
			ret = VHAL_ERR_PARAM;
		}
	}
	return ret;
}

/*****************************************************************************
 処理概要：	ワイド設定のスケーリング処理(NORMAL)
 引数    ：	struct CVhalLayoutVideoSetting::VideoRectangleData 	&in_src		(i)ワイド設定前のソース短形
           	struct CVhalLayoutVideoSetting::VideoRectangleData 	&in_dest	(i)ワイド設定前のデスティネーション短形
           	struct CVhalLayoutVideoSetting::VideoRectangleData 	&out_src	(o)ワイド設定後のソース短形
           	struct CVhalLayoutVideoSetting::VideoRectangleData 	&out_dest	(o)ワイド設定後のデスティネーション短形
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutVideoSetting::ScalingWideNormal(const struct CVhalLayoutVideoSetting::VideoRectangleData &in_src, const struct CVhalLayoutVideoSetting::VideoRectangleData &in_dest, struct CVhalLayoutVideoSetting::VideoRectangleData &out_src, struct CVhalLayoutVideoSetting::VideoRectangleData &out_dest) const
{
	constexpr int32_t ret{VHAL_SUCCESS};

	/* スケーリング処理(NORMAL) */
	/* ・画像サイズのアスペクト比を維持し、出力先に収まる最大サイズに拡縮する */
	/* ・出力画像はセンタリング表示 */
	/* ・出力先に余白が出た場合は、黒画で埋める */

	/* 元画像サイズはそのまま流用 */
	out_src.x_		= in_src.x_;
	out_src.y_		= in_src.y_;
	out_src.width_	= in_src.width_;
	out_src.height_	= in_src.height_;

	const int32_t ratio_w{( MulI32(in_src.width_, 100) ) / in_dest.width_};
	const int32_t ratio_h{( MulI32(in_src.height_, 100) ) / in_dest.height_};

	/* ( 元画像幅 / 出力幅 ) > ( 元画像高さ / 出力高さ ) の場合 */
	if ( ratio_w > ratio_h )
	{
		/* 上下に余白がでる */
		out_dest.width_ = in_dest.width_;
		out_dest.height_ = DivI32(( MulI32(in_dest.width_, in_src.height_) ), in_src.width_);
	}
	/* ( 元画像幅 / 出力幅 ) <= ( 元画像高さ / 出力高さ ) の場合 */
	else
	{
		/* 左右に余白がでる */
		out_dest.width_ = DivI32(( MulI32(in_dest.height_, in_src.width_) ), in_src.height_);
		out_dest.height_ = in_dest.height_;
	}
	out_dest.x_ = AddI32(in_dest.x_, DivI32(( SubI32(in_dest.width_, out_dest.width_) ), 2));
	out_dest.y_ = AddI32(in_dest.y_, DivI32(( SubI32(in_dest.height_, out_dest.height_) ), 2));

	VHAL_LOGD("in_src(%d,%d,%d,%d) in_dest(%d,%d,%d,%d) out_src(%d,%d,%d,%d) out_dest(%d,%d,%d,%d) ratio_w=%d ratio_h=%d",
		in_src.x_, in_src.y_, in_src.width_, in_src.height_,
		in_dest.x_, in_dest.y_, in_dest.width_, in_dest.height_,
		out_src.x_, out_src.y_, out_src.width_, out_src.height_,
		out_dest.x_, out_dest.y_, out_dest.width_, out_dest.height_, ratio_w, ratio_h);

	return ret;
}

/*****************************************************************************
 処理概要：	ワイド設定のスケーリング処理(STRETCHED)
 引数    ：	struct CVhalLayoutVideoSetting::VideoRectangleData 	&in_src		(i)ワイド設定前のソース短形
        	struct CVhalLayoutVideoSetting::VideoRectangleData 	&in_dest	(i)ワイド設定前のデスティネーション短形
           	struct CVhalLayoutVideoSetting::VideoRectangleData 	&out_src	(o)ワイド設定後のソース短形
           	struct CVhalLayoutVideoSetting::VideoRectangleData 	&out_dest	(o)ワイド設定後のデスティネーション短形
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutVideoSetting::ScalingWideStretched(const struct CVhalLayoutVideoSetting::VideoRectangleData &in_src, const struct CVhalLayoutVideoSetting::VideoRectangleData &in_dest, struct CVhalLayoutVideoSetting::VideoRectangleData &out_src, struct CVhalLayoutVideoSetting::VideoRectangleData &out_dest) const
{
	constexpr int32_t ret{VHAL_SUCCESS};

	/* スケーリング処理(STRETCHED) */
	/* ・画像サイズを縦横共に出力先サイズにフィットさせる（＝出力先に余白は出ない）*/
	/* ・画像サイズによってはアスペクト比は維持されない */

	/* 元画像サイズはそのまま流用 */
	out_src.x_		= in_src.x_;
	out_src.y_		= in_src.y_;
	out_src.width_	= in_src.width_;
	out_src.height_	= in_src.height_;

	/* 出力サイズはそのまま使用 */
	out_dest.x_			= in_dest.x_;
	out_dest.y_			= in_dest.y_;
	out_dest.width_		= in_dest.width_;
	out_dest.height_	= in_dest.height_;
	VHAL_LOGD("in_src(%d,%d,%d,%d) in_dest(%d,%d,%d,%d) out_src(%d,%d,%d,%d) out_dest(%d,%d,%d,%d)",
		in_src.x_, in_src.y_, in_src.width_, in_src.height_,
		in_dest.x_, in_dest.y_, in_dest.width_, in_dest.height_,
		out_src.x_, out_src.y_, out_src.width_, out_src.height_,
		out_dest.x_, out_dest.y_, out_dest.width_, out_dest.height_);

	return ret;
}

/*****************************************************************************
 処理概要：	ワイド設定のスケーリング処理(ZOOMED)
 引数    ：	struct CVhalLayoutVideoSetting::VideoRectangleData 	&in_src		(i)ワイド設定前のソース短形
           	struct CVhalLayoutVideoSetting::VideoRectangleData 	&in_dest	(i)ワイド設定前のデスティネーション短形
           	struct CVhalLayoutVideoSetting::VideoRectangleData 	&out_src	(o)ワイド設定後のソース短形
           	struct CVhalLayoutVideoSetting::VideoRectangleData 	&out_dest	(o)ワイド設定後のデスティネーション短形
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutVideoSetting::ScalingWideZoomed(const struct CVhalLayoutVideoSetting::VideoRectangleData &in_src, const struct CVhalLayoutVideoSetting::VideoRectangleData &in_dest, struct CVhalLayoutVideoSetting::VideoRectangleData &out_src, struct CVhalLayoutVideoSetting::VideoRectangleData &out_dest) const
{
	constexpr int32_t ret{VHAL_SUCCESS};

	/* スケーリング処理(ZOOMED) */
	/* ・画像サイズのアスペクト比を維持し、出力先を埋めるように拡縮する（＝出力先に余白は出ない） */
	/* ・出力先サイズから はみ出す画像部分は表示されない（画像の片側を削らないよう両端を均等に削る） */

	const int32_t ratio_w{( MulI32(in_src.width_, 100) ) / in_dest.width_};
	const int32_t ratio_h{( MulI32(in_src.height_, 100) ) / in_dest.height_};

	/* ( 元画像幅 / 出力幅 ) > ( 元画像高さ / 出力高さ ) の場合 */
	if ( ratio_w > ratio_h )
	{
		/* 左右をカットする */
		out_src.width_	= DivI32(( MulI32(in_dest.width_, in_src.height_) ), in_dest.height_);
		out_src.height_	= in_src.height_;
	}
	/* ( 元画像幅 / 出力幅 ) <= ( 元画像高さ / 出力高さ ) の場合 */
	else
	{
		/* 上下をカットする */
		out_src.width_	= in_src.width_;
		out_src.height_	= DivI32(( MulI32(in_dest.height_, in_src.width_) ), in_dest.width_);
	}
	out_src.x_ = AddI32(in_src.x_, DivI32(( SubI32(in_src.width_, out_src.width_) ), 2));
	out_src.y_ = AddI32(in_src.y_, DivI32(( SubI32(in_src.height_, out_src.height_) ), 2));

	/* 出力サイズはそのまま使用 */
	out_dest.x_			= in_dest.x_;
	out_dest.y_			= in_dest.y_;
	out_dest.width_		= in_dest.width_;
	out_dest.height_	= in_dest.height_;
	VHAL_LOGD("in_src(%d,%d,%d,%d) in_dest(%d,%d,%d,%d) out_src(%d,%d,%d,%d) out_dest(%d,%d,%d,%d) ratio_w=%d ratio_h=%d",
		in_src.x_, in_src.y_, in_src.width_, in_src.height_,
		in_dest.x_, in_dest.y_, in_dest.width_, in_dest.height_,
		out_src.x_, out_src.y_, out_src.width_, out_src.height_,
		out_dest.x_, out_dest.y_, out_dest.width_, out_dest.height_,ratio_w, ratio_h);

	return ret;
}

/*****************************************************************************
 処理概要：	映像パス名、映像出力先を元に映像識別名を取得
 引数    ：	const std::string& path			(i)映像パス名
         ：	VideoOutputTarget out_target	(i)映像出力先
         ：	std::string& identifier			(o)出力サイズ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutVideoSetting::GetVideoPathIdentifier(const std::string& path, const VideoOutputTarget out_target, std::string& identifier) const
{
	int32_t ret{VHAL_SUCCESS};

	switch( out_target )
	{
	case VIDEO_OUTPUT_TARGET_FRONT:
		identifier = path + kVhalPathNameAppendFront;
		break;
	case VIDEO_OUTPUT_TARGET_CAMERA:
		identifier = path + kVhalPathNameAppendCamera;
		break;
	case VIDEO_OUTPUT_TARGET_REAR:
		identifier = path + kVhalPathNameAppendRear;
		break;
	case VIDEO_OUTPUT_TARGET_IC:
		identifier = path + kVhalPathNameAppendIc;
		break;
	default:
		ret = VHAL_ERR_PARAM;
		break;
	}

	return ret;
}


} /* namespace videohal */

