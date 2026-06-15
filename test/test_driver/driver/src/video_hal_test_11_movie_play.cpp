/*
 * @copyright Copyright (c) 2022 Woven Alpha, Inc
 */

/*** inclue ***/
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <gtest/gtest.h>

#include "ilm/ilm_common.h"
#include "ilm/ilm_control.h"

#include "video_hal_ct_common.h"
#include "video_hal.h"
#include "vhal_log.h"

/*** define ***/

/*** prototype ***/

/*** variables ***/
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static bool result_updated;
static int32_t movie_result;
static cockpit::hal::video_hal::CtlObj Obj;

class VideoHAL_MoviePlay: public::testing::Test
{
protected:
	static void Callback_MovieStatus();
	virtual void SetUp();
	virtual void TearDown();
protected:
	int32_t screen_width;
	int32_t screen_height;
};

/*** Callback ***/
void VideoHAL_MoviePlay::Callback_MovieStatus()
{
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);

		/* Check front vpath switching result */
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "movie.front.status.result", &movie_result, sizeof(movie_result), VIDEO_HAL_DATA_TYPE_NUM));
		VHAL_LOGI("/*** call result=%d", movie_result);
		result_updated = true;
	}

	cond_sync.notify_one();
}

void VideoHAL_MoviePlay::SetUp()
{
	struct ilmScreenProperties screenProp;

	/* Get the screen properties */
	memset(&screenProp, 0x00, sizeof(struct ilmScreenProperties));
	get_screen_properties(VHAL_CT_FRONT_ID, &screenProp);

	/* Set width and height of front screen */
	screen_width  = screenProp.screenWidth;
	screen_height = screenProp.screenHeight;
}

void VideoHAL_MoviePlay::TearDown()
{
}

//
// Googletest TestCase
//
TEST_F(VideoHAL_MoviePlay, Lv1Normal011)
{
	std::string file_path_base = "/opt/dc-ivi-pf/tests/dn-cdc-lvgvm-hal-video-impl-qc/certification-program/Startup_Toyota_Global_1280x720.mp4";
	std::string file_path;
	int32_t x = 0;
	int32_t y = 0;
	int32_t w = 0;
	int32_t h = 0;
	bool flag = false;
	bool intime;
	bool set_flag = false;
	std::string vpath_str = "";

	/* Halcp Indication */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Test No.11 Start\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Overview : Movie Play Test\n"));
/*	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] Please copy a mp4 movie file to /run/arene/share/sample_movie.mp4.\n")); */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] Please copy a mp4 movie file to /opt/dc-ivi-pf/tests/dn-cdc-lvgvm-hal-video-impl-qc/certification-program/Startup_Toyota_Global_1280x720.mp4\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTANYKEY("[CertificationTest][Info] Please Input Any Key For Continue Test:"));

	/* Check screen size */
	EXPECT_GT(screen_width, 0);
	EXPECT_GT(screen_height, 0);

	/* VideoHAL Initialize */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(Obj));
	VHAL_LOGI("/*** Test No.11 Start");

	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	/* ========== Test Start ========== */

	/* VideoHAL Register callback function */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(Obj, "movie.front.status.result", Callback_MovieStatus));

	/* prepare playing a movie */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] Play a movie and wait until it is finished.\n"));
//	file_path = "/run/arene/share/sample_movie.mp4";
	file_path = file_path_base;
	x = 0;
	y = 0;
	w = screen_width;
	h = screen_height;
	flag = true;
	result_updated = false;
	VHAL_LOGI("/*** phase01 set movie.front.control.prepare=%d", flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "movie.front.control.file.path",     &file_path, (uint32_t)file_path.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "movie.front.control.output.x",      &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "movie.front.control.output.y",      &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "movie.front.control.output.width",  &w, sizeof(w), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "movie.front.control.output.height", &h, sizeof(h), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "movie.front.control.prepare",       &flag, sizeof(flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "movie.front.control.prepare",       &flag, sizeof(flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, flag);

	/* wait callback */
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(VHAL_CT_TIMEOUT_SECONDS), [] { return result_updated; });
	EXPECT_EQ(true, intime);
	EXPECT_EQ(VIDEO_HAL_MOVIE_STS_READY, movie_result);

	/* start playing the movie */
	flag = true;
	result_updated = false;
	VHAL_LOGI("/*** phase02 set movie.front.control.start=%d", flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "movie.front.control.start", &flag, sizeof(flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "movie.front.control.start", &flag, sizeof(flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, flag);

	/* wait callback */
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(VHAL_CT_TIMEOUT_SECONDS), [] { return result_updated; });
	EXPECT_EQ(true, intime);
	EXPECT_EQ(VIDEO_HAL_MOVIE_STS_PLAYING, movie_result);

	/* wait until playing the movie is finished. */
	result_updated = false;
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTANYKEY("[CertificationTest][Info] Please input any key after playing the movie is finished."));
	wait_msec(2000);

	VHAL_LOGI("/*** phase03 playing the movie is finished");

	/* wait callback */
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(VHAL_CT_TIMEOUT_SECONDS), [] { return result_updated; });
	EXPECT_EQ(true, intime);
	EXPECT_EQ(VIDEO_HAL_MOVIE_STS_FINISHED, movie_result);

	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the movie played successfully ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* clear all parameters */
	flag = true;
	result_updated = false;
	VHAL_LOGI("/*** phase04 set movie.front.control.clear=%d", flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "movie.front.control.clear", &flag, sizeof(flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "movie.front.control.clear", &flag, sizeof(flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, flag);

	/* wait callback */
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(VHAL_CT_TIMEOUT_SECONDS), [] { return result_updated; });
	EXPECT_EQ(true, intime);
	EXPECT_EQ(VIDEO_HAL_MOVIE_STS_NONE, movie_result);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "movie.front.control.file.path",     &file_path, (uint32_t)file_path.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "movie.front.control.output.x",      &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "movie.front.control.output.y",      &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "movie.front.control.output.width",  &w, sizeof(w), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "movie.front.control.output.height", &h, sizeof(h), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ("", file_path);
	EXPECT_EQ(0, x);
	EXPECT_EQ(0, y);
	EXPECT_EQ(0, w);
	EXPECT_EQ(0, h);

	/* prepare playing the movie again */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] Play the movie and cancel it before it is finished.\n"));
//	file_path = "/run/arene/share/sample_movie.mp4";
	file_path = file_path_base;
	x = 0;
	y = 0;
	w = screen_width;
	h = screen_height;
	flag = true;
	result_updated = false;
	VHAL_LOGI("/*** phase05 set movie.front.control.prepare=%d", flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "movie.front.control.file.path",     &file_path, (uint32_t)file_path.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "movie.front.control.output.x",      &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "movie.front.control.output.y",      &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "movie.front.control.output.width",  &w, sizeof(w), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "movie.front.control.output.height", &h, sizeof(h), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "movie.front.control.prepare",       &flag, sizeof(flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "movie.front.control.prepare",       &flag, sizeof(flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, flag);

	/* wait callback */
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(VHAL_CT_TIMEOUT_SECONDS), [] { return result_updated; });
	EXPECT_EQ(true, intime);
	EXPECT_EQ(VIDEO_HAL_MOVIE_STS_READY, movie_result);

	/* start playing a movie */
	flag = true;
	result_updated = false;
	VHAL_LOGI("/*** phase06 set movie.front.control.start=%d", flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "movie.front.control.start", &flag, sizeof(flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "movie.front.control.start", &flag, sizeof(flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, flag);

	/* wait callback */
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(VHAL_CT_TIMEOUT_SECONDS), [] { return result_updated; });
	EXPECT_EQ(true, intime);
	EXPECT_EQ(VIDEO_HAL_MOVIE_STS_PLAYING, movie_result);

	/* Cancel it before it is finished. */
	result_updated = false;
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTANYKEY("[CertificationTest][Info] Please input any key just after playing the movie is started."));
	wait_msec(2000);

	/* stop playing a movie */
	flag = true;
	result_updated = false;
	VHAL_LOGI("/*** phase07 playing the movie is started -> cancel=%d", flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "movie.front.control.cancel", &flag, sizeof(flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "movie.front.control.cancel", &flag, sizeof(flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, flag);

	/* wait callback */
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(VHAL_CT_TIMEOUT_SECONDS), [] { return result_updated; });
	EXPECT_EQ(true, intime);
	EXPECT_EQ(VIDEO_HAL_MOVIE_STS_CANCELED, movie_result);

	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the movie cancelled successfully ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* clear all parameters */
	flag = true;
	result_updated = false;
	VHAL_LOGI("/*** phase08 set movie.front.control.clear=%d", flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "movie.front.control.clear", &flag, sizeof(flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "movie.front.control.clear", &flag, sizeof(flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, flag);

	/* wait callback */
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(VHAL_CT_TIMEOUT_SECONDS), [] { return result_updated; });
	EXPECT_EQ(true, intime);
	EXPECT_EQ(VIDEO_HAL_MOVIE_STS_NONE, movie_result);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "movie.front.control.file.path",     &file_path, (uint32_t)file_path.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "movie.front.control.output.x",      &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "movie.front.control.output.y",      &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "movie.front.control.output.width",  &w, sizeof(w), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "movie.front.control.output.height", &h, sizeof(h), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ("", file_path);
	EXPECT_EQ(0, x);
	EXPECT_EQ(0, y);
	EXPECT_EQ(0, w);
	EXPECT_EQ(0, h);

	/* ClearCallback() */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(Obj, "movie.front.status.result", Callback_MovieStatus));

	/* ========== Test End ========== */

	/* Deinit() */
	VHAL_LOGI("/*** Test No.11 Exit");
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(Obj));
}
