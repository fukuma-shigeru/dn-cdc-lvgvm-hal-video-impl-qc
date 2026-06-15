#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static bool status_mute_display;
static bool status_mute_display_expect;
static bool status_mute_video;
static bool status_mute_video_expect;
static bool status_mute_backlight;
static bool status_mute_backlight_expect;
static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;

static void CallbackMuteDisplay(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "mute.front.status.display", &status_mute_display, sizeof(status_mute_display), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(status_mute_display_expect, status_mute_display);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackMuteVideo(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "mute.front.status.video", &status_mute_video, sizeof(status_mute_video), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(status_mute_video_expect, status_mute_video);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackMuteBacklight(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "mute.front.status.backlight", &status_mute_backlight, sizeof(status_mute_backlight), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(status_mute_backlight_expect, status_mute_backlight);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackMuteDisplayRear(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "mute.rear.status.display", &status_mute_display, sizeof(status_mute_display), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(status_mute_display_expect, status_mute_display);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackMuteVideoRear(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "mute.rear.status.video", &status_mute_video, sizeof(status_mute_video), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(status_mute_video_expect, status_mute_video);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

struct VhalSeqTestMuteParameter {
	int32_t dummy;
};

class VhalSeqTestMuteApiSpecification : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestMuteParameter> {};
class VhalSeqTestMuteRearApiSpecification : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestMuteParameter> {};

INSTANTIATE_TEST_CASE_P(VhalSeqTestMute, VhalSeqTestMuteApiSpecification, ::testing::Values(
	VhalSeqTestMuteParameter{0}
));

INSTANTIATE_TEST_CASE_P(VhalSeqTestMute, VhalSeqTestMuteRearApiSpecification, ::testing::Values(
	VhalSeqTestMuteParameter{0}
));

/**
 * API仕様書記載のシーケンス
 * 前席ディスプレイ全体MUTE 
 */
TEST_P(VhalSeqTestMuteApiSpecification, MuteDisplay)
{
	VhalSeqTestMuteParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayFront(obj, false));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteBacklight(obj, true));

	/**
	 * 前席ディスプレイ全体MUTE
	 * バックライトプロパティがTrue
	 * ①Muteプロパティ false -> true (MUTE ON)
	 * ②Muteプロパティ true -> false (MUTE 解除)
	 * ③Muteプロパティ false -> false 
	*/

	/* ① */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "mute.front.status.display", CallbackMuteDisplay));
	status_mute_display = false;
	status_mute_display_expect = true;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.display", &status_mute_display_expect, sizeof(status_mute_display_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for mute_display.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_display_expect == status_mute_display); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "mute.front.status.display", CallbackMuteDisplay));

	/* ② */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "mute.front.status.display", CallbackMuteDisplay));
	status_mute_display = true;
	status_mute_display_expect = false;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.display", &status_mute_display_expect, sizeof(status_mute_display_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for unmute_display.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_display_expect == status_mute_display); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "mute.front.status.display", CallbackMuteDisplay));

	/* ③ */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "mute.front.status.display", CallbackMuteDisplay));
	status_mute_display = false;
	status_mute_display_expect = false;
	PrintMsg("wait_for donot_change_display.");
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.display", &status_mute_display_expect, sizeof(status_mute_display_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_display_expect == status_mute_display); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "mute.front.status.display", CallbackMuteDisplay));

	/** 
	 * 前席ディスプレイ全体MUTE 
	 * バックライトプロパティがFalse
	 * ④Muteプロパティ true -> false (MUTE ON)
	 * ⑤Muteプロパティ false -> true (MUTE ON)
	 * ⑥Muteプロパティ true -> true 
	*/
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteBacklight(obj, false));

	/* ④ */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "mute.front.status.display", CallbackMuteDisplay));
	status_mute_display = true;
	status_mute_display_expect = false;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.display", &status_mute_display_expect, sizeof(status_mute_display_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for mute_display.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_display_expect == status_mute_display); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "mute.front.status.display", CallbackMuteDisplay));

	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayFront(obj, false));
	
	/* ⑤ */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "mute.front.status.display", CallbackMuteDisplay));
	status_mute_display = false;
	status_mute_display_expect = true;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.display", &status_mute_display_expect, sizeof(status_mute_display_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for mute_display.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_display_expect == status_mute_display); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "mute.front.status.display", CallbackMuteDisplay));

	/* ⑥ */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "mute.front.status.display", CallbackMuteDisplay));
	status_mute_display = true;
	status_mute_display_expect = true;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.display", &status_mute_display_expect, sizeof(status_mute_display_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for donot_change_display.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_display_expect == status_mute_display); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "mute.front.status.display", CallbackMuteDisplay));

	/* 元に戻す */
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteBacklight(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayFront(obj, false));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * API仕様書記載のシーケンス
 * 前席映像面MUTE 
 */
TEST_P(VhalSeqTestMuteApiSpecification, MuteVideo)
{
	VhalSeqTestMuteParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteVideoFront(obj, false));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "mute.front.status.video", CallbackMuteVideo));

	/* 前席映像面MUTE */
	status_mute_video = false;
	status_mute_video_expect = true;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.video", &status_mute_video_expect, sizeof(status_mute_video_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for mute_video.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_video_expect == status_mute_video); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "mute.front.status.video", CallbackMuteVideo));

	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteVideoFront(obj, false));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * API仕様書記載のシーケンス
 * 前席バックライト変更 
 */
TEST_P(VhalSeqTestMuteApiSpecification, MuteBacklight)
{
	VhalSeqTestMuteParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteBacklight(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayFront(obj, true));

	/**
	 * 前席バックライト変更
	 * MuteプロパティがTrue
	 * ①バックライトプロパティ true -> false (MUTE ON)
	 * ②バックライトプロパティ false -> true (MUTE ON)
	 * ③バックライトプロパティ true -> true
	 */

	/* ① */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "mute.front.status.backlight", CallbackMuteBacklight));
	status_mute_backlight = true;
	status_mute_backlight_expect = false;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.backlight", &status_mute_backlight_expect, sizeof(status_mute_backlight_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for mute_backlight(display).");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_backlight_expect == status_mute_backlight); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "mute.front.status.backlight", CallbackMuteBacklight));

	/* ② */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "mute.front.status.backlight", CallbackMuteBacklight));
	status_mute_backlight = false;
	status_mute_backlight_expect = true;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.backlight", &status_mute_backlight_expect, sizeof(status_mute_backlight_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for mute_backlight(display).");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_backlight_expect == status_mute_backlight); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "mute.front.status.backlight", CallbackMuteBacklight));

	/* ⑤ */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "mute.front.status.backlight", CallbackMuteBacklight));
	status_mute_backlight = true;
	status_mute_backlight_expect = true;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.backlight", &status_mute_backlight_expect, sizeof(status_mute_backlight_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for donot_change_display.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_backlight_expect == status_mute_backlight); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "mute.front.status.backlight", CallbackMuteBacklight));

	/** 
	 * 前席バックライト変更
	 * MuteプロパティがFalse
	 * ④バックライトプロパティ false -> true (MUTE 解除)
	 * ⑤バックライトプロパティ true -> false (MUTE ON)
	 * ⑥バックライトプロパティ false -> false
	 */

	/* ④ */
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayFront(obj, false));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteBacklight(obj, false));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "mute.front.status.backlight", CallbackMuteBacklight));
	status_mute_backlight = false;
	status_mute_backlight_expect = true;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.backlight", &status_mute_backlight_expect, sizeof(status_mute_backlight_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for unmute_backlight(display).");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_backlight_expect == status_mute_backlight); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "mute.front.status.backlight", CallbackMuteBacklight));

	/* ⑤ */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "mute.front.status.backlight", CallbackMuteBacklight));
	status_mute_backlight = true;
	status_mute_backlight_expect = false;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.backlight", &status_mute_backlight_expect, sizeof(status_mute_backlight_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for mute_backlight(display).");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_backlight_expect == status_mute_backlight); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "mute.front.status.backlight", CallbackMuteBacklight));

	/* ⑥ */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "mute.front.status.backlight", CallbackMuteBacklight));
	status_mute_backlight = false;
	status_mute_backlight_expect = false;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.backlight", &status_mute_backlight_expect, sizeof(status_mute_backlight_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for donot_change_display.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_backlight_expect == status_mute_backlight); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "mute.front.status.backlight", CallbackMuteBacklight));

	/* 元に戻す */

	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteBacklight(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * API仕様書記載のシーケンス
 * 後席ディスプレイ全体MUTE
 */
TEST_P(VhalSeqTestMuteRearApiSpecification, MuteDisplay)
{
	VhalSeqTestMuteParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayRear(obj, false));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "mute.rear.status.display", CallbackMuteDisplayRear));

	/* ディスプレイ全体MUTE */
	status_mute_display = false;
	status_mute_display_expect = true;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.rear.control.display", &status_mute_display_expect, sizeof(status_mute_display_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for mute_display.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_display_expect == status_mute_display); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleRear(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "mute.rear.status.display", CallbackMuteDisplayRear));

	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayRear(obj, false));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * API仕様書記載のシーケンス
 * 後席映像面MUTE
 */
TEST_P(VhalSeqTestMuteRearApiSpecification, MuteVideo)
{
	VhalSeqTestMuteParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteVideoRear(obj, false));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "mute.rear.status.video", CallbackMuteVideoRear));

	/* 前席映像面MUTE */
	status_mute_video = false;
	status_mute_video_expect = true;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.rear.control.video", &status_mute_video_expect, sizeof(status_mute_video_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for mute_video.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_video_expect == status_mute_video); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "mute.rear.status.video", CallbackMuteVideoRear));

	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteVideoRear(obj, false));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * API仕様書記載のシーケンス
 * MUTE状態遷移表の網羅
 * 手動実施しない場合はコメントにしておく
 */
// TEST_P(VhalSeqTestMuteApiSpecification, MuteStateTransition)
// {
// 	VhalSeqTestMuteParameter param = GetParam();
// 	std::unique_lock<std::mutex> lock_sync(mtx_sync);
// 	int32_t result{VHAL_TEST_SUCCESS};
// 	cockpit::hal::video_hal::Init(obj);

// 	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
// 	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

// 	/**共通イベント
// 	* [E1]mute.front.control.display：mute on
// 	* [E2]mute.front.control.display：mute off
// 	* [E3]mute.front.control.backlight：backlight on
// 	* [E4]mute.front.control.backlight：backlight off
// 	*/
	
// 	/**MUTEの状態 [S1]
// 	 * MUTE-ON
// 	 * mute.front.control.display：mute on
// 	 * mute.front.control.backlight：backlight off
// 	*/
// 	for (size_t i = 0; i <= 3; i++)
// 	{
// 		EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayFront(obj, true));
// 		EXPECT_EQ(VHAL_TEST_SUCCESS, MuteBacklight(obj, false));

// 		if(i == 0)
// 		{		
// 			result = MuteDisplayFront(obj, true);
// 			PrintMsg("[S1][E1]Check No change in display MUTE state.");
// 		}
// 		else if(i == 1)
// 		{
// 			result = MuteDisplayFront(obj, false);
// 			PrintMsg("[S1][E2]Check No change in display MUTE state.");
// 		}
// 		else if(i == 2)
// 		{
// 			result = MuteBacklight(obj, true);
// 			PrintMsg("[S1][E3]Check No change in display MUTE state.");
// 		}
// 		else if(i == 3)
// 		{
// 			result = MuteBacklight(obj, false);
// 			PrintMsg("[S1][E4]Check No change in display MUTE state.");
// 		}
// 		else
// 		{
// 			PrintMsg("Invalid iteration.");
// 		}

// 		if(VHAL_TEST_SUCCESS == result)
// 		{
// 			PrintMsg("press Enter .");
// 			std::cin.get();
// 			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
// 			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
// 		}
// 		else
// 		{
// 			PrintMsg("Mute State Transition Error");
// 			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
// 		}
// 	}

// 	/**MUTEの状態 [S2]
// 	 * MUTE-ON
// 	 * mute.front.control.display：mute on
// 	 * mute.front.control.backlight：backlight on
// 	*/
// 	for (size_t i = 0; i <= 3; i++)
// 	{
// 		EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayFront(obj, true));
// 		EXPECT_EQ(VHAL_TEST_SUCCESS, MuteBacklight(obj, true));

// 		if(i == 0)
// 		{
// 			result = MuteDisplayFront(obj, true);
// 			PrintMsg("[S2][E1]Check No change in display MUTE state.");
// 		}
// 		else if(i == 1)
// 		{
// 			result = MuteDisplayFront(obj, false);
// 			PrintMsg("[S2][E2]Check display MUTE state is unmuted.");
// 		}
// 		else if(i == 2)
// 		{
// 			result = MuteBacklight(obj, true);	
// 			PrintMsg("[S2][E3]Check No change in display MUTE state.");
// 		}
// 		else if(i == 3)
// 		{
// 			result = MuteBacklight(obj, false);
// 			PrintMsg("[S2][E4]Check No change in display MUTE state.");
// 		}
// 		else
// 		{
// 			PrintMsg("Invalid iteration.");
// 		}

// 		if(VHAL_TEST_SUCCESS == result)
// 		{
// 			PrintMsg("press Enter .");
// 			std::cin.get();
// 			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
// 			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
// 		}
// 		else
// 		{
// 			PrintMsg("Mute State Transition Error");
// 			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
// 		}
// 	}

// 	/**MUTEの状態 [S3]
// 	 * MUTE-ON
// 	 * mute.front.control.display：mute off
// 	 * mute.front.control.backlight：backlight off
// 	*/
// 	for (size_t i = 0; i <= 3; i++)
// 	{
// 		EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayFront(obj, false));
// 		EXPECT_EQ(VHAL_TEST_SUCCESS, MuteBacklight(obj, false));

// 		if(i == 0)
// 		{
// 			result = MuteDisplayFront(obj, true);
// 			PrintMsg("[S3][E1]Check No change in display MUTE state");
// 		}
// 		else if(i == 1)
// 		{
// 			result = MuteDisplayFront(obj, false);
// 			PrintMsg("[S3][E2]Check No change in display MUTE state");
// 		}
// 		else if(i == 2)
// 		{
// 			result = MuteBacklight(obj, true);
// 			PrintMsg("[S3][E3]Check display MUTE state is unmuted.");
// 		}
// 		else if(i == 3)
// 		{
// 			result = MuteBacklight(obj, false);
// 			PrintMsg("[S3][E4]Check No change in display MUTE state.");
// 		}
// 		else
// 		{
// 			PrintMsg("Invalid iteration.");
// 		}

// 		if(VHAL_TEST_SUCCESS == result)
// 		{
// 			PrintMsg("press Enter .");
// 			std::cin.get();
// 			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
// 			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
// 		}
// 		else
// 		{
// 			PrintMsg("Mute State Transition Error");
// 			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
// 		}
// 	}

// 	/**MUTEの状態 [S4]
// 	 * MUTE-OFF
// 	 * mute.front.control.display：mute off
// 	 * mute.front.control.backlight：backlight on
// 	*/
// 	for (size_t i = 0; i <= 3; i++)
// 	{
// 		EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayFront(obj, false));
// 		EXPECT_EQ(VHAL_TEST_SUCCESS, MuteBacklight(obj, true));

// 		if(i == 0)
// 		{
// 			result = MuteDisplayFront(obj, true);
// 			PrintMsg("[S4][E1]Check display MUTE state is muted.");
// 		}
// 		else if(i == 1)
// 		{
// 			result = MuteDisplayFront(obj, false);
// 			PrintMsg("[S4][E2]Check No change in display MUTE state.");
// 		}
// 		else if(i == 2)
// 		{
// 			result = MuteBacklight(obj, true);
// 			PrintMsg("[S4][E3]Check No change in display MUTE state.");
// 		}
// 		else if(i == 3)
// 		{
// 			result = MuteBacklight(obj, false);
// 			PrintMsg("[S4][E4]Check display MUTE state is muted.");
// 		}
// 		else
// 		{
// 			PrintMsg("Invalid iteration.");
// 		}

// 		if(VHAL_TEST_SUCCESS == result)
// 		{
// 			PrintMsg("press Enter .");
// 			std::cin.get();
// 			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
// 			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
// 		}
// 		else
// 		{
// 			PrintMsg("Mute State Transition Error");
// 			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
// 		}
// 	}

// 	/* 元に戻す */
// 	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteBacklight(obj, true));
// 	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayFront(obj, true));
// 	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
// 	KillAllRenderImage(obj);

// 	cockpit::hal::video_hal::Deinit(obj);
// }

/** 
 * 連続動作
 * 前席ディスプレイ全体MUTE 
 */
TEST(VhalSeqTestMuteRepeat, MuteDisplay)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayFront(obj, false));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "mute.front.status.display", CallbackMuteDisplay));

	/*バックライトプロパティがTrue*/
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteBacklight(obj, true));
	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		status_mute_display = false;
		status_mute_display_expect = true;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.display", &status_mute_display_expect, sizeof(status_mute_display_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for mute_display.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_display_expect == status_mute_display); });
		EXPECT_EQ(true, intime);

		/* もとに戻す */
		status_mute_display = true;
		status_mute_display_expect = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.display", &status_mute_display_expect, sizeof(status_mute_display_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for mute_display.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_display_expect == status_mute_display); });
		EXPECT_EQ(true, intime);
	}

	/*バックライトプロパティがfalse*/
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteBacklight(obj, false));
	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		status_mute_display = false;
		status_mute_display_expect = true;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.display", &status_mute_display_expect, sizeof(status_mute_display_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for mute_display.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_display_expect == status_mute_display); });
		EXPECT_EQ(true, intime);

		/* もとに戻す */
		status_mute_display = true;
		status_mute_display_expect = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.display", &status_mute_display_expect, sizeof(status_mute_display_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for mute_display.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_display_expect == status_mute_display); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "mute.front.status.display", CallbackMuteDisplay));

	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayFront(obj, false));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteBacklight(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 * 前席映像面MUTE 
 */
TEST(VhalSeqTestMuteRepeat, MuteVideo)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool mute_video{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteVideoFront(obj, false));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "mute.front.status.video", CallbackMuteVideo));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		status_mute_video = false;
		status_mute_video_expect = true;
		mute_video = status_mute_video_expect;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.video", &status_mute_video_expect, sizeof(status_mute_video_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for mute_video.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_video_expect == status_mute_video); });
		EXPECT_EQ(true, intime);

		/* もとに戻す */
		status_mute_video = true;
		status_mute_video_expect = false;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.video", &status_mute_video_expect, sizeof(status_mute_video_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for mute_video.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_video_expect == status_mute_video); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "mute.front.status.video", CallbackMuteVideo));

	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteVideoFront(obj, false));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 * 前席バックライトOFF 
 */
TEST(VhalSeqTestMuteRepeat, MuteBacklight)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteBacklight(obj, false));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "mute.front.status.backlight", CallbackMuteBacklight));

	/* Muteプロパティがtrue*/
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayFront(obj, true));
	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		status_mute_backlight = false;
		status_mute_backlight_expect = true;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.backlight", &status_mute_backlight_expect, sizeof(status_mute_backlight_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for mute_backlight.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_backlight_expect == status_mute_backlight); });
		EXPECT_EQ(true, intime);

		/* もとに戻す */
		status_mute_backlight = true;
		status_mute_backlight_expect = false;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.backlight", &status_mute_backlight_expect, sizeof(status_mute_backlight_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for mute_backlight.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_backlight_expect == status_mute_backlight); });
		EXPECT_EQ(true, intime);
	}

	/* Muteプロパティがfalse*/
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayFront(obj, false));
	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		status_mute_backlight = false;
		status_mute_backlight_expect = true;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.backlight", &status_mute_backlight_expect, sizeof(status_mute_backlight_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for mute_backlight.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_backlight_expect == status_mute_backlight); });
		EXPECT_EQ(true, intime);

		/* もとに戻す */
		status_mute_backlight = true;
		status_mute_backlight_expect = false;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.backlight", &status_mute_backlight_expect, sizeof(status_mute_backlight_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for mute_backlight.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_backlight_expect == status_mute_backlight); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "mute.front.status.backlight", CallbackMuteBacklight));

	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteBacklight(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 * 後席ディスプレイ全体MUTE
 */
TEST(VhalSeqTestMuteRearRepeat, MuteDisplay)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayRear(obj, false));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "mute.rear.status.display", CallbackMuteDisplayRear));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		status_mute_display = false;
		status_mute_display_expect = true;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.rear.control.display", &status_mute_display_expect, sizeof(status_mute_display_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for mute_display.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_display_expect == status_mute_display); });
		EXPECT_EQ(true, intime);

		/* もとに戻す */
		status_mute_display = true;
		status_mute_display_expect = false;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.rear.control.display", &status_mute_display_expect, sizeof(status_mute_display_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for mute_display.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_display_expect == status_mute_display); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "mute.rear.status.display", CallbackMuteDisplayRear));

	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayRear(obj, false));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 * 後席映像面MUTE
 */
TEST(VhalSeqTestMuteRearRepeat, MuteVideo)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteVideoRear(obj, false));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "mute.rear.status.video", CallbackMuteVideoRear));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		status_mute_video = false;
		status_mute_video_expect = true;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.rear.control.video", &status_mute_video_expect, sizeof(status_mute_video_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for mute_video.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_video_expect == status_mute_video); });
		EXPECT_EQ(true, intime);

		/* もとに戻す */
		status_mute_video = true;
		status_mute_video_expect = false;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.rear.control.video", &status_mute_video_expect, sizeof(status_mute_video_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for mute_video.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_video_expect == status_mute_video); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "mute.rear.status.video", CallbackMuteVideoRear));

	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteVideoRear(obj, false));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}
