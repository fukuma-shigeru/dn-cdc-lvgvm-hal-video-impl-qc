/*******************************************************************************
    機能名称    ：  動画制御モジュール
    ファイル名称：  vhal_movie_control.h
*******************************************************************************/
#ifndef	VHAL_MOVIE_CONTROL_H
#define	VHAL_MOVIE_CONTROL_H

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <bitset>
#include <algorithm>
#include <csignal>
#include <unistd.h>
#include <sys/wait.h>

#include "wl_renderer_public.h"
#include "vhal_layout_video_setting.h"
#include "vhal_event_route.h"
extern "C"
{
#include "vhal_gst_player.h"
}
namespace videohal
{

class CVhalMainControl;
class CVhalPropertyControl;
class CVhalEventItemMovieWait;
class CVhalLayoutManager;

/*****************************************************************************
 クラス名称：CVhalMovieControl
 処理概要  ：動画再生処理の制御を行う。
*****************************************************************************/
class CVhalMovieControl {
	friend CVhalEventItemMovieWait;
public:

	struct MovieParameter {
	public:
		/* コンストラクタ */
		MovieParameter(void)
		:file_path_("")
		,x_(0U)
		,y_(0U)
		,width_(0U)
		,height_(0U){}

		uint32_t GetX(void) const noexcept
		{
			return x_;
		}

		uint32_t GetY(void) const noexcept
		{
			return y_;
		}

		uint32_t GetWidth(void) const noexcept
		{
			return width_;
		}

		uint32_t GetHeight(void) const noexcept
		{
			return height_;
		}

		std::string GetFilePath(void) const noexcept
		{
			return file_path_;
		}

		void SetFilePath(const std::string &file_path)
		{
			file_path_ = file_path;
		}

		void SetX(const uint32_t x) noexcept
		{
			x_ = x;
		}

		void SetY(const uint32_t y) noexcept
		{
			y_ = y;
		}

		void SetWidth(const uint32_t width) noexcept
		{
			width_ = width;
		}

		void SetHeight(const uint32_t height) noexcept
		{
			height_ = height;
		}

	private:
		std::string		file_path_;
		uint32_t		x_;
		uint32_t		y_;
		uint32_t		width_;
		uint32_t		height_;
	};
	
	CVhalMovieControl(void);
	~CVhalMovieControl(void);
  	CVhalMovieControl(const CVhalMovieControl& src) = delete;
	CVhalMovieControl& operator=(const CVhalMovieControl& src) & = delete;
	CVhalMovieControl(CVhalMovieControl&& src) = delete;
	CVhalMovieControl& operator=(CVhalMovieControl&& src) & = delete;

	int32_t Initialize(CVhalMainControl * const p_main, CVhalPropertyControl * const p_prop, CVhalLayoutManager * const p_layout_mng);
	void Finalize(void);

	/* 汎用動画再生機能 動画再生パラメータ設定 */
	int32_t PrepareFrontMovie(const uint32_t screen_width, const uint32_t screen_height, const struct MovieParameter& param);
	/* 汎用動画再生機能 動画再生パラメータクリア */
	int32_t ClearFrontMovie(void);
	/* 汎用動画再生機能 動画再生開始 */
	int32_t StartFrontMovie(void);
	/* 汎用動画再生機能 動画再生中止 */
	int32_t CancelFrontMovie(void);
	/* 汎用動画再生状態取得 */
	int32_t GetFrontMovieStatus(void) const noexcept;
	void HandleGstMovieEvent(const int32_t result);

private:
	enum class MovieRequestType : uint8_t {kPrepare, kClear, kStart, kCancel};

	void NotifyMovieFinished(const int32_t result);

	bool IsValidPlayStatus(const int32_t play_status, const MovieRequestType request) const;

	CVhalMainControl *p_main_;
	std::unique_ptr<CVhalEventRoute> p_event_route_;

	/* 各プロパティ固有のAction処理を行うクラス */
	CVhalLayoutManager           *p_layout_mng_;
	CVhalPropertyControl         *p_prop_;
	
	struct MovieParameter	movie_parameter_;
	int32_t	play_status_;
	bool is_playing_;

	struct vhal_player		*gst_movie_ctx_;
	struct vhal_player_cb	gst_movie_callback_;
};

} /* namespace videohal */

#endif	/* #ifndef VHAL_MOVIE_CONTROL_H */
