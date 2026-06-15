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

struct VhalApiTestMuteParameter {
    bool mute;
};

class VhalApiTestMuteNormal : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestMuteParameter> {};
class VhalApiTestMuteRearNormal : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestMuteParameter> {};
class VhalApiTestMuteRepeat : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestMuteParameter> {};
class VhalApiTestMuteRearRepeat : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestMuteParameter> {};
class VhalApiTestMuteRearAbnormalNotConnectedR : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestMuteParameter> {};

INSTANTIATE_TEST_CASE_P(VhalApiTestMute, VhalApiTestMuteNormal, ::testing::Values(
	VhalApiTestMuteParameter{false},
	VhalApiTestMuteParameter{true}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestMute, VhalApiTestMuteRearNormal, ::testing::Values(
	VhalApiTestMuteParameter{false},
	VhalApiTestMuteParameter{true}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestMute, VhalApiTestMuteRepeat, ::testing::Values(
	VhalApiTestMuteParameter{true}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestMute, VhalApiTestMuteRearRepeat, ::testing::Values(
	VhalApiTestMuteParameter{true}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestMute, VhalApiTestMuteRearAbnormalNotConnectedR, ::testing::Values(
	VhalApiTestMuteParameter{false}
));

/**
 * 基本動作
 * 前席ディスプレイ全体MUTE
 */
TEST_P(VhalApiTestMuteNormal, MuteDisplay)
{
	VhalApiTestMuteParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);


	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayFront(obj, OTHER_BOOL_VALUE(param.mute)));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "mute.front.status.display", CallbackMuteDisplay));

	/* 前席ディスプレイ全体MUTE */
	status_mute_display = OTHER_BOOL_VALUE(param.mute);
	status_mute_display_expect = param.mute;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.display", &param.mute, sizeof(param.mute), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for mute_display.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_display_expect == status_mute_display); });
	EXPECT_EQ(true, intime);	

    bool control_mute_display{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "mute.front.control.display", &control_mute_display, sizeof(control_mute_display), VIDEO_HAL_DATA_TYPE_BOOL));
    EXPECT_EQ(param.mute, control_mute_display);

	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "mute.front.status.display", CallbackMuteDisplay));

	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayFront(obj, false));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * 前席映像面MUTE
 */
TEST_P(VhalApiTestMuteNormal, MuteVideo)
{
	VhalApiTestMuteParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);


	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteVideoFront(obj, OTHER_BOOL_VALUE(param.mute)));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "mute.front.status.video", CallbackMuteVideo));

	/* 前席映像面MUTE */
	status_mute_video = OTHER_BOOL_VALUE(param.mute);
	status_mute_video_expect = param.mute;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.video", &param.mute, sizeof(param.mute), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for mute_video.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_video_expect == status_mute_video); });
	EXPECT_EQ(true, intime);	

    bool control_mute_video{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "mute.front.control.video", &control_mute_video, sizeof(control_mute_video), VIDEO_HAL_DATA_TYPE_BOOL));
    EXPECT_EQ(param.mute, control_mute_video);

	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "mute.front.status.video", CallbackMuteVideo));

	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteVideoFront(obj, false));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作 
 * 前席バックライト 
 */
TEST_P(VhalApiTestMuteNormal, MuteBacklight)
{
	VhalApiTestMuteParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteBacklight(obj, OTHER_BOOL_VALUE(param.mute)));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "mute.front.status.backlight", CallbackMuteBacklight));

	/* 前席バックライトOFF */
	status_mute_backlight = OTHER_BOOL_VALUE(param.mute);
	status_mute_backlight_expect = param.mute;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.backlight", &param.mute, sizeof(param.mute), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for mute_backlight.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_backlight_expect == status_mute_backlight); });
	EXPECT_EQ(true, intime);

    bool control_mute_backlight{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "mute.front.control.backlight", &control_mute_backlight, sizeof(control_mute_backlight), VIDEO_HAL_DATA_TYPE_BOOL));
    EXPECT_EQ(param.mute, control_mute_backlight);

	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "mute.front.status.backlight", CallbackMuteBacklight));

	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteBacklight(obj, true));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * 後席ディスプレイ全体MUTE
 */
TEST_P(VhalApiTestMuteRearNormal, MuteDisplay)
{
	VhalApiTestMuteParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayRear(obj, OTHER_BOOL_VALUE(param.mute)));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "mute.rear.status.display", CallbackMuteDisplayRear));

	/* 前席ディスプレイ全体MUTE */
	status_mute_display = OTHER_BOOL_VALUE(param.mute);
	status_mute_display_expect = param.mute;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.rear.control.display", &param.mute, sizeof(param.mute), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for mute_display.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_display_expect == status_mute_display); });
	EXPECT_EQ(true, intime);	

    bool control_mute_display{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "mute.rear.control.display", &control_mute_display, sizeof(control_mute_display), VIDEO_HAL_DATA_TYPE_BOOL));
    EXPECT_EQ(param.mute, control_mute_display);

	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_REAR, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetRearOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleRear(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "mute.rear.status.display", CallbackMuteDisplayRear));

	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayRear(obj, false));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * 後席映像面MUTE
 */
TEST_P(VhalApiTestMuteRearNormal, MuteVideo)
{
	VhalApiTestMuteParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);


	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteVideoRear(obj, OTHER_BOOL_VALUE(param.mute)));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "mute.rear.status.video", CallbackMuteVideoRear));

	/* 前席映像面MUTE */
	status_mute_video = OTHER_BOOL_VALUE(param.mute);
	status_mute_video_expect = param.mute;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.rear.control.video", &param.mute, sizeof(param.mute), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for mute_video.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_mute_video_expect == status_mute_video); });
	EXPECT_EQ(true, intime);	

    bool control_mute_video{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "mute.rear.control.video", &control_mute_video, sizeof(control_mute_video), VIDEO_HAL_DATA_TYPE_BOOL));
    EXPECT_EQ(param.mute, control_mute_video);

	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_REAR, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetRearOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleRear(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "mute.rear.status.video", CallbackMuteVideoRear));

	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteVideoRear(obj, false));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 * 前席
 */
TEST(VhalApiTestMuteNotSet, NotSet)
{
	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "mute.front.status.display", &status_mute_display, sizeof(status_mute_display), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(true, status_mute_display);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "mute.front.status.video", &status_mute_video, sizeof(status_mute_video), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, status_mute_video);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "mute.front.status.backlight", &status_mute_backlight, sizeof(status_mute_backlight), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, status_mute_backlight);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 * 後席
 */
TEST(VhalApiTestMuteRearNotSet, NotSet)
{
	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "mute.rear.status.display", &status_mute_display, sizeof(status_mute_display), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, status_mute_display);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "mute.rear.status.video", &status_mute_video, sizeof(status_mute_video), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, status_mute_video);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 * 前席ディスプレイ
 */
TEST_P(VhalApiTestMuteRepeat, MuteDisplay)
{
	VhalApiTestMuteParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayFront(obj, OTHER_BOOL_VALUE(param.mute)));

	/* 前席ディスプレイ全体MUTE */
	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.display", &param.mute, sizeof(param.mute), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	}

	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayFront(obj, false));
	EXPECT_EQ(VHAL_TEST_SUCCESS, KillAllRenderImage(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 * 前席映像面
 */
TEST_P(VhalApiTestMuteRepeat, MuteVideo)
{
	VhalApiTestMuteParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteVideoFront(obj, OTHER_BOOL_VALUE(param.mute)));

	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.video", &param.mute, sizeof(param.mute), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	}

	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteVideoFront(obj, false));
	EXPECT_EQ(VHAL_TEST_SUCCESS, KillAllRenderImage(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 * 前席バックライト 
 */
TEST_P(VhalApiTestMuteRepeat, MuteBacklight)
{
	VhalApiTestMuteParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteBacklight(obj, OTHER_BOOL_VALUE(param.mute)));

	/* 前席バックライトOFF */
	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.front.control.backlight", &param.mute, sizeof(param.mute), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	}

	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteBacklight(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, KillAllRenderImage(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 * 後席ディスプレイ
 */
TEST_P(VhalApiTestMuteRearRepeat, MuteDisplay)
{
	VhalApiTestMuteParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayRear(obj, OTHER_BOOL_VALUE(param.mute)));

	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.rear.control.display", &param.mute, sizeof(param.mute), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	}

	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayRear(obj, false));
	EXPECT_EQ(VHAL_TEST_SUCCESS, KillAllRenderImage(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 * 後席映像面
 */
TEST_P(VhalApiTestMuteRearRepeat, MuteVideo)
{
	VhalApiTestMuteParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteVideoRear(obj, OTHER_BOOL_VALUE(param.mute)));

	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.rear.control.video", &param.mute, sizeof(param.mute), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	}

	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteVideoRear(obj, false));
	EXPECT_EQ(VHAL_TEST_SUCCESS, KillAllRenderImage(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 前提を満たさない条件でのコール
 * RSE未接続で後席ディスプレイ全体のMUTE
 * 前提条件:RSE未接続
 */
TEST_P(VhalApiTestMuteRearAbnormalNotConnectedR, MuteDisplay)
{
	VhalApiTestMuteParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

    bool control_mute_display{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "mute.rear.control.display", &control_mute_display, sizeof(control_mute_display), VIDEO_HAL_DATA_TYPE_BOOL));
	control_mute_display = !control_mute_display;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "mute.rear.status.display", CallbackMuteDisplayRear));

	/* 前席ディスプレイ全体MUTE */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.rear.control.display", &control_mute_display, sizeof(control_mute_display), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PROP_UPDATE, cockpit::hal::video_hal::RequestUpdate(obj));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "mute.rear.status.display", CallbackMuteDisplayRear));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 前提を満たさない条件でのコール
 * RSE未接続で後席映像面のMUTE
 * 前提条件:RSE未接続
 */

TEST_P(VhalApiTestMuteRearAbnormalNotConnectedR, MuteVideo)
{
	VhalApiTestMuteParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

    bool control_mute_video{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "mute.rear.control.video", &control_mute_video, sizeof(control_mute_video), VIDEO_HAL_DATA_TYPE_BOOL));
	control_mute_video = !control_mute_video;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "mute.rear.status.video", CallbackMuteVideoRear));

	/* 後席映像面MUTE */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "mute.rear.control.video", &control_mute_video, sizeof(control_mute_video), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PROP_UPDATE, cockpit::hal::video_hal::RequestUpdate(obj));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "mute.rear.status.video", CallbackMuteVideoRear));

	cockpit::hal::video_hal::Deinit(obj);
}
