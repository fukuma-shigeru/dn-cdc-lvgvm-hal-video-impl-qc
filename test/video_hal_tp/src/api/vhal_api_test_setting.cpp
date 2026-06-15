#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static int32_t status_connected_camera;
static int32_t status_connected_camera_expect;
static int32_t status_day_night;
static int32_t status_day_night_expect;
static int32_t status_theme_color;
static int32_t status_theme_color_expect;
static int32_t status_connected_rse;
static int32_t status_connected_rse_expect;
static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;

#define ABNORMAL_ARG_CONNECTED_CAMERA (VIDEO_HAL_CONNECTED_CAMERA_MTM_METER + 1)
#define ABNORMAL_ARG_CONNECTED_RSE (VIDEO_HAL_CONNECTED_RSE_FULL + 1)
#define ABNORMAL_ARG_DAY_NIGHT (VIDEO_HAL_SETTING_FORCED_DAY + 1)
#define ABNORMAL_ARG_THEME_COLOR (VIDEO_HAL_THEME_COLOR_FORCED_LIGHT + 1)

static void CallbackConnectedCamera(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.connected.camera", &status_connected_camera, sizeof(status_connected_camera), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_connected_camera_expect, status_connected_camera);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackConnectedRse(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.connected.rse", &status_connected_rse, sizeof(status_connected_rse), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_connected_rse_expect, status_connected_rse);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackDayNight(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.day_night", &status_day_night, sizeof(status_day_night), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_day_night_expect, status_day_night);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackThemeColor(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.theme_color", &status_theme_color, sizeof(status_theme_color), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_theme_color_expect, status_theme_color);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

/* 接続されているカメラ種別 */
struct VhalApiTestConnectedCameraParameter {
    int32_t connected_camera;
};

/* 接続されているRSE種別 */
struct VhalApiTestConnectedRseParameter {
    int32_t connected_rse;
};

/* 昼夜モード */
struct VhalApiTestDayNightParameter {
    int32_t day_night;
};

/* テーマカラー */
struct VhalApiTestThemeColorParameter {
    int32_t theme_color;
};

/* HDMI接続状態 */
struct VhalApiTestConnectedHdmiParameter {
    int32_t connected_hdmi;
};

/* 前席ディスプレイデバイス状態・解像度 */
struct VhalApiTestDisplayFrontParameter {
    bool display;
    uint32_t width;
    uint32_t height;
};

/* 後席ディスプレイデバイス状態・解像度 */
struct VhalApiTestDisplayRearParameter {
    bool display;
    uint32_t width;
    uint32_t height;
};

/* ICディスプレイデバイス状態・解像度 */
struct VhalApiTestDisplayIcParameter {
    bool display;
    uint32_t width;
    uint32_t height;
};

class VhalApiTestSettingNormalConnectedCamera : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestConnectedCameraParameter> {};
class VhalApiTestSettingNormalConnectedRse : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestConnectedRseParameter> {};
class VhalApiTestSettingNormalDayNight : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestDayNightParameter> {};
class VhalApiTestSettingNormalThemeColor : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestThemeColorParameter> {};
class VhalApiTestSettingNormalConnectedHdmi : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestConnectedHdmiParameter> {};
class VhalApiTestSettingNormalNotConnectedH : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestConnectedHdmiParameter> {};
class VhalApiTestSettingNormalDisplayFront : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestDisplayFrontParameter> {};
class VhalApiTestSettingNormalDisplayRear : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestDisplayRearParameter> {};
class VhalApiTestSettingNormalDisplayIc : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestDisplayIcParameter> {};
class VhalApiTestSettingRepeatConnectedCamera : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestConnectedCameraParameter> {};
class VhalApiTestSettingRepeatConnectedRse : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestConnectedRseParameter> {};
class VhalApiTestSettingRepeatDayNight : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestDayNightParameter> {};
class VhalApiTestSettingRepeatThemeColor : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestThemeColorParameter> {};
class VhalApiTestSettingAbnormalArgConnectedCamera : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestConnectedCameraParameter> {};
class VhalApiTestSettingAbnormalArgConnectedRse : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestConnectedRseParameter> {};
class VhalApiTestSettingAbnormalArgDayNight : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestDayNightParameter> {};
class VhalApiTestSettingAbnormalArgThemeColor : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestThemeColorParameter> {};

INSTANTIATE_TEST_CASE_P(VhalApiTestSetting, VhalApiTestSettingNormalConnectedCamera, ::testing::Values(
    VhalApiTestConnectedCameraParameter{VIDEO_HAL_CONNECTED_CAMERA_INVALID},
    VhalApiTestConnectedCameraParameter{VIDEO_HAL_CONNECTED_CAMERA_NONE},
    VhalApiTestConnectedCameraParameter{VIDEO_HAL_CONNECTED_CAMERA_ABGM},
    VhalApiTestConnectedCameraParameter{VIDEO_HAL_CONNECTED_CAMERA_DBGM},
    VhalApiTestConnectedCameraParameter{VIDEO_HAL_CONNECTED_CAMERA_DPVM_NORMAL},
    VhalApiTestConnectedCameraParameter{VIDEO_HAL_CONNECTED_CAMERA_DPVM_WIDE},
    VhalApiTestConnectedCameraParameter{VIDEO_HAL_CONNECTED_CAMERA_DPVM_FULLHD},
    VhalApiTestConnectedCameraParameter{VIDEO_HAL_CONNECTED_CAMERA_SBGM_CAA},
    VhalApiTestConnectedCameraParameter{VIDEO_HAL_CONNECTED_CAMERA_BGM_ADU},
    VhalApiTestConnectedCameraParameter{VIDEO_HAL_CONNECTED_CAMERA_SIM},
    VhalApiTestConnectedCameraParameter{VIDEO_HAL_CONNECTED_CAMERA_PVM},
    VhalApiTestConnectedCameraParameter{VIDEO_HAL_CONNECTED_CAMERA_MTM},
    VhalApiTestConnectedCameraParameter{VIDEO_HAL_CONNECTED_CAMERA_PVM_METER},
    VhalApiTestConnectedCameraParameter{VIDEO_HAL_CONNECTED_CAMERA_MTM_METER}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestSetting, VhalApiTestSettingNormalConnectedRse, ::testing::Values(
    VhalApiTestConnectedRseParameter{VIDEO_HAL_CONNECTED_RSE_INVALID},
    VhalApiTestConnectedRseParameter{VIDEO_HAL_CONNECTED_RSE_NONE},
    VhalApiTestConnectedRseParameter{VIDEO_HAL_CONNECTED_RSE_DOP},
    VhalApiTestConnectedRseParameter{VIDEO_HAL_CONNECTED_RSE_FULL}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestSetting, VhalApiTestSettingNormalDayNight, ::testing::Values(
    VhalApiTestDayNightParameter{VIDEO_HAL_SETTING_DAY},
    VhalApiTestDayNightParameter{VIDEO_HAL_SETTING_NIGHT},
    VhalApiTestDayNightParameter{VIDEO_HAL_SETTING_FORCED_DAY}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestSetting, VhalApiTestSettingNormalThemeColor, ::testing::Values(
    VhalApiTestThemeColorParameter{VIDEO_HAL_THEME_COLOR_AUTO_LIGHT},
    VhalApiTestThemeColorParameter{VIDEO_HAL_THEME_COLOR_AUTO_DARK},
    VhalApiTestThemeColorParameter{VIDEO_HAL_THEME_COLOR_SELECT_LIGHT},
    VhalApiTestThemeColorParameter{VIDEO_HAL_THEME_COLOR_SELECT_DARK},
    VhalApiTestThemeColorParameter{VIDEO_HAL_THEME_COLOR_FORCED_LIGHT}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestSetting, VhalApiTestSettingNormalConnectedHdmi, ::testing::Values(
    VhalApiTestConnectedHdmiParameter{VIDEO_HAL_CONNECTED_HDMI_READY}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestSetting, VhalApiTestSettingNormalNotConnectedH, ::testing::Values(
    VhalApiTestConnectedHdmiParameter{VIDEO_HAL_CONNECTED_HDMI_NONE}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestSetting, VhalApiTestSettingNormalDisplayFront, ::testing::Values(
    VhalApiTestDisplayFrontParameter{true, 0, 0}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestSetting, VhalApiTestSettingNormalDisplayRear, ::testing::Values(
    VhalApiTestDisplayRearParameter{true, 0, 0}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestSetting, VhalApiTestSettingNormalDisplayIc, ::testing::Values(
    VhalApiTestDisplayIcParameter{true, 0, 0}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestSetting, VhalApiTestSettingRepeatConnectedCamera, ::testing::Values(
    VhalApiTestConnectedCameraParameter{VIDEO_HAL_CONNECTED_CAMERA_DPVM_NORMAL}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestSetting, VhalApiTestSettingRepeatConnectedRse, ::testing::Values(
    VhalApiTestConnectedRseParameter{VIDEO_HAL_CONNECTED_RSE_DOP}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestSetting, VhalApiTestSettingRepeatDayNight, ::testing::Values(
    VhalApiTestDayNightParameter{VIDEO_HAL_SETTING_DAY}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestSetting, VhalApiTestSettingRepeatThemeColor, ::testing::Values(
    VhalApiTestThemeColorParameter{VIDEO_HAL_THEME_COLOR_AUTO_LIGHT}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestSetting, VhalApiTestSettingAbnormalArgConnectedCamera, ::testing::Values(
    VhalApiTestConnectedCameraParameter{ABNORMAL_ARG_CONNECTED_CAMERA}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestSetting, VhalApiTestSettingAbnormalArgConnectedRse, ::testing::Values(
    VhalApiTestConnectedRseParameter{ABNORMAL_ARG_CONNECTED_RSE}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestSetting, VhalApiTestSettingAbnormalArgDayNight, ::testing::Values(
    VhalApiTestDayNightParameter{ABNORMAL_ARG_DAY_NIGHT}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestSetting, VhalApiTestSettingAbnormalArgThemeColor, ::testing::Values(
    VhalApiTestThemeColorParameter{ABNORMAL_ARG_THEME_COLOR}
));

/**
 * 基本動作
 * 接続されているカメラ種別
 */
TEST_P(VhalApiTestSettingNormalConnectedCamera, ConnectedCameraNormal)
{
	VhalApiTestConnectedCameraParameter param = GetParam();
    std::unique_lock<std::mutex> lock_sync(mtx_sync);
    bool intime{};

    cockpit::hal::video_hal::Init(obj);

    EXPECT_EQ(VHAL_TEST_SUCCESS, SetConnectedCamera(obj, OTHER_NUM_VALUE(param.connected_camera, VIDEO_HAL_CONNECTED_CAMERA_MTM_METER)));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.connected.camera", CallbackConnectedCamera));
#if 0 /* NOP_CONNECTED_CAMERA */
    status_connected_camera = OTHER_NUM_VALUE(param.connected_camera, VIDEO_HAL_CONNECTED_CAMERA_MTM_METER);
    status_connected_camera_expect = param.connected_camera;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.connected.camera", &param.connected_camera, sizeof(param.connected_camera), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_connected_camera_expect == status_connected_camera); });
	EXPECT_EQ(true, intime);

    int32_t control_connected_camera{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.control.connected.camera", &control_connected_camera, sizeof(control_connected_camera), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(param.connected_camera, control_connected_camera);
#endif /* NOP_CONNECTED_CAMERA */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.connected.camera", CallbackConnectedCamera));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * 接続されているRSE種別
 */
TEST_P(VhalApiTestSettingNormalConnectedRse, ConnectedRseNormal)
{
	VhalApiTestConnectedRseParameter param = GetParam();
    std::unique_lock<std::mutex> lock_sync(mtx_sync);
    bool intime{};

    cockpit::hal::video_hal::Init(obj);

    EXPECT_EQ(VHAL_TEST_SUCCESS, SetConnectedRse(obj, OTHER_NUM_VALUE(param.connected_rse, VIDEO_HAL_CONNECTED_RSE_FULL)));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.connected.rse", CallbackConnectedRse));

    status_connected_rse = OTHER_NUM_VALUE(param.connected_rse, VIDEO_HAL_CONNECTED_RSE_FULL);
    status_connected_rse_expect = param.connected_rse;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.connected.rse", &param.connected_rse, sizeof(param.connected_rse), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_connected_rse_expect == status_connected_rse); });
	EXPECT_EQ(true, intime);

    int32_t control_connected_rse{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.control.connected.rse", &control_connected_rse, sizeof(control_connected_rse), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(param.connected_rse, control_connected_rse);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.connected.rse", CallbackConnectedRse));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * 昼夜モード
 */
TEST_P(VhalApiTestSettingNormalDayNight, DayNightNormal)
{
	VhalApiTestDayNightParameter param = GetParam();
    std::unique_lock<std::mutex> lock_sync(mtx_sync);
    bool intime{};

    cockpit::hal::video_hal::Init(obj);

    SetDayNight(obj, OTHER_NUM_VALUE(param.day_night, VIDEO_HAL_SETTING_FORCED_DAY));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.day_night", CallbackDayNight));

    status_day_night = OTHER_NUM_VALUE(param.day_night, VIDEO_HAL_SETTING_FORCED_DAY);
    status_day_night_expect = param.day_night;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.day_night", &param.day_night, sizeof(param.day_night), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_day_night_expect == status_day_night); });
	EXPECT_EQ(true, intime);

    uint32_t control_day_night{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.control.day_night", &control_day_night, sizeof(control_day_night), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(param.day_night, control_day_night);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.day_night", CallbackDayNight));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * テーマカラー
 */
TEST_P(VhalApiTestSettingNormalThemeColor, ThemeColorNormal)
{
	VhalApiTestThemeColorParameter param = GetParam();
    std::unique_lock<std::mutex> lock_sync(mtx_sync);
    bool intime{};

    cockpit::hal::video_hal::Init(obj);

    SetThemeColor(obj, OTHER_NUM_VALUE(param.theme_color, VIDEO_HAL_THEME_COLOR_FORCED_LIGHT));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.theme_color", CallbackThemeColor));

    status_theme_color = OTHER_NUM_VALUE(param.theme_color, VIDEO_HAL_THEME_COLOR_FORCED_LIGHT);
    status_theme_color_expect = param.theme_color;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.theme_color", &param.theme_color, sizeof(param.theme_color), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_theme_color_expect == status_theme_color); });
	EXPECT_EQ(true, intime);

    uint32_t control_theme_color{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.control.theme_color", &control_theme_color, sizeof(control_theme_color), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(param.theme_color, control_theme_color);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.theme_color", CallbackThemeColor));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * HDMI接続状態
 */
TEST_P(VhalApiTestSettingNormalConnectedHdmi, ConnectedHdmiNormal)
{
	VhalApiTestConnectedHdmiParameter param = GetParam();
    int32_t connected_hdmi{};

    cockpit::hal::video_hal::Init(obj);

    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.connected.hdmi", &connected_hdmi, sizeof(connected_hdmi), VIDEO_HAL_DATA_TYPE_NUM));
//    EXPECT_EQ(param.connected_hdmi, connected_hdmi);

	cockpit::hal::video_hal::Deinit(obj);
}


/**
 * 基本動作
 * HDMI接続状態なし
 */
TEST_P(VhalApiTestSettingNormalNotConnectedH, NotConnectedHNormal)
{
	VhalApiTestConnectedHdmiParameter param = GetParam();
    int32_t connected_hdmi{};

    cockpit::hal::video_hal::Init(obj);

    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.connected.hdmi", &connected_hdmi, sizeof(connected_hdmi), VIDEO_HAL_DATA_TYPE_NUM));
//    EXPECT_EQ(param.connected_hdmi, connected_hdmi);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * 前席ディスプレイ状態・解像度
 */
TEST_P(VhalApiTestSettingNormalDisplayFront, DisplayFrontNormal)
{
	VhalApiTestDisplayFrontParameter param = GetParam();
    bool display{};
    uint32_t width{};
    uint32_t height{};

    struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);

    cockpit::hal::video_hal::Init(obj);

    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.display.front", &display, sizeof(display), VIDEO_HAL_DATA_TYPE_BOOL));
    EXPECT_EQ(true, display);
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.display.front.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(prop.screenWidth, width);
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.display.front.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(prop.screenHeight, height);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * 後席ディスプレイ状態・解像度
 */
TEST_P(VhalApiTestSettingNormalDisplayRear, DisplayRearNormal)
{
	VhalApiTestDisplayRearParameter param = GetParam();
    bool display{};
    uint32_t width{};
    uint32_t height{};

    struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_REAR, &prop);

    cockpit::hal::video_hal::Init(obj);

    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.display.rear", &display, sizeof(display), VIDEO_HAL_DATA_TYPE_BOOL));
    EXPECT_EQ(true, display);
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.display.rear.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(prop.screenWidth, width);
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.display.rear.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(prop.screenHeight, height);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * 後席ディスプレイ状態・解像度
 * 後席ディスプレイ未接続
 */
TEST(VhalApiTestSettingNormalNotConnectedR, Normal)
{
    bool display{};
    uint32_t width{};
    uint32_t height{};

    cockpit::hal::video_hal::Init(obj);

    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.display.rear", &display, sizeof(display), VIDEO_HAL_DATA_TYPE_BOOL));
    EXPECT_EQ(false, display);
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.display.rear.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(0, width);
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.display.rear.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(0, height);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * ICディスプレイ状態・解像度
 */
TEST_P(VhalApiTestSettingNormalDisplayIc, DisplayIcNormal)
{
	VhalApiTestDisplayIcParameter param = GetParam();
    bool display{};
    uint32_t width{};
    uint32_t height{};

    struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_IC, &prop);

    cockpit::hal::video_hal::Init(obj);

    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.display.instrumentcluster", &display, sizeof(display), VIDEO_HAL_DATA_TYPE_BOOL));
    EXPECT_EQ(true, display);
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.display.instrumentcluster.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(prop.screenWidth, width);
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.display.instrumentcluster.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(prop.screenHeight, height);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * ICディスプレイ状態・解像度
 * 後席ディスプレイ未接続
 */
TEST(VhalApiTestSettingNormalNotConnectedI, Normal)
{
    bool display{};
    uint32_t width{};
    uint32_t height{};

    cockpit::hal::video_hal::Init(obj);

    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.display.instrumentcluster", &display, sizeof(display), VIDEO_HAL_DATA_TYPE_BOOL));
    EXPECT_EQ(false, display);
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.display.instrumentcluster.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(0, width);
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.display.instrumentcluster.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(0, height);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 以下は起動時に上位から設定されているため、API仕様書VideoHALプロパティ一覧記載のdefault値と
 * 取得結果が異なる。評価対象からはずすためコメントアウト。
 * ・現在設定されているカメラ種別(vhal.setting.status.connected.camera)
 * ・現在設定されているテーマカラー(vhal.setting.status.theme_color)
 * ・現在設定されている昼夜モード(vhal.setting.status.vhal.setting.status.day_night)
 */

/**
 * 設定前の参照
 * 接続されているカメラ種別
 */
TEST(VhalApiTestSettingCameraNotSet, NotSet)
{
    cockpit::hal::video_hal::Init(obj);

#if 0 /* NOP */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.connected.camera", &status_connected_camera, sizeof(status_connected_camera), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_CONNECTED_CAMERA_INVALID, status_connected_camera);
#endif /* NOP */

	cockpit::hal::video_hal::Deinit(obj);
}


/**
 * 設定前の参照
 * 接続されているRSE種別
 */
TEST(VhalApiTestSettingRseNotSet, NotSet)
{
    cockpit::hal::video_hal::Init(obj);

    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.connected.rse", &status_connected_rse, sizeof(status_connected_rse), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_CONNECTED_RSE_INVALID, status_connected_rse);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 * 現在設定されている昼夜モード
 */
TEST(VhalApiTestSettingDayNightNotSet, NotSet)
{
    cockpit::hal::video_hal::Init(obj);

#if 0 /* NOP */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.day_night", &status_day_night, sizeof(status_day_night), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_SETTING_NIGHT, status_day_night);
#endif /* NOP */

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 * 現在設定されているテーマカラー
 */
TEST(VhalApiTestSettingThemeColorNotSet, NotSet)
{
    cockpit::hal::video_hal::Init(obj);

#if 0 /* NOP */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.theme_color", &status_theme_color, sizeof(status_theme_color), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_THEME_COLOR_AUTO_DARK, status_theme_color);
#endif /* NOP */

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 * 接続されているカメラ種別
 */
TEST_P(VhalApiTestSettingRepeatConnectedCamera, Repeat)
{
	VhalApiTestConnectedCameraParameter param = GetParam();

    cockpit::hal::video_hal::Init(obj);
#if 0 /* NOP_CONNECTED_CAMERA */
    EXPECT_EQ(VHAL_TEST_SUCCESS, SetConnectedCamera(obj, OTHER_NUM_VALUE(param.connected_camera, VIDEO_HAL_CONNECTED_CAMERA_MTM_METER)));

    for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
    {
        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.connected.camera", &param.connected_camera, sizeof(param.connected_camera), VIDEO_HAL_DATA_TYPE_NUM));
        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
    }
#endif /* NOP_CONNECTED_CAMERA */
	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 * 接続されているRSE種別
 */
TEST_P(VhalApiTestSettingRepeatConnectedRse, Repeat)
{
	VhalApiTestConnectedRseParameter param = GetParam();

    cockpit::hal::video_hal::Init(obj);

    EXPECT_EQ(VHAL_TEST_SUCCESS, SetConnectedRse(obj, OTHER_NUM_VALUE(param.connected_rse, VIDEO_HAL_CONNECTED_RSE_FULL)));

    for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
    {
        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.connected.rse", &param.connected_rse, sizeof(param.connected_rse), VIDEO_HAL_DATA_TYPE_NUM));
        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
    }

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 * 昼夜モード
 */
TEST_P(VhalApiTestSettingRepeatDayNight, Repeat)
{
	VhalApiTestDayNightParameter param = GetParam();

    cockpit::hal::video_hal::Init(obj);

    SetDayNight(obj, OTHER_NUM_VALUE(param.day_night, VIDEO_HAL_SETTING_FORCED_DAY));

    for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
    {
        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.day_night", &param.day_night, sizeof(param.day_night), VIDEO_HAL_DATA_TYPE_NUM));
        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
    }

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 * テーマカラー
 */
TEST_P(VhalApiTestSettingRepeatThemeColor, Repeat)
{
	VhalApiTestThemeColorParameter param = GetParam();
    bool intime{};

    cockpit::hal::video_hal::Init(obj);

    SetThemeColor(obj, OTHER_NUM_VALUE(param.theme_color, VIDEO_HAL_THEME_COLOR_FORCED_LIGHT));

    for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
    {
        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.theme_color", &param.theme_color, sizeof(param.theme_color), VIDEO_HAL_DATA_TYPE_NUM));
        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
    }

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ異常 
 * 接続されているカメラ種別
 */
TEST_P(VhalApiTestSettingAbnormalArgConnectedCamera, AbnormalArg)
{
	VhalApiTestConnectedCameraParameter param = GetParam();

    cockpit::hal::video_hal::Init(obj);
#if 0 /* NOP_CONNECTED_CAMERA */
    EXPECT_EQ(VHAL_TEST_SUCCESS, SetConnectedCamera(obj, OTHER_NUM_VALUE(param.connected_camera, VIDEO_HAL_CONNECTED_CAMERA_MTM_METER)));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.connected.camera", &param.connected_camera, sizeof(param.connected_camera), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PROP_UPDATE, cockpit::hal::video_hal::RequestUpdate(obj));
#endif /* NOP_CONNECTED_CAMERA */
	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ異常 
 * 接続されているRSE種別
 */
TEST_P(VhalApiTestSettingAbnormalArgConnectedRse, AbnormalArg)
{
	VhalApiTestConnectedRseParameter param = GetParam();

    cockpit::hal::video_hal::Init(obj);

    EXPECT_EQ(VHAL_TEST_SUCCESS, SetConnectedRse(obj, OTHER_NUM_VALUE(param.connected_rse, VIDEO_HAL_CONNECTED_RSE_FULL)));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.connected.rse", &param.connected_rse, sizeof(param.connected_rse), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ異常
 * 昼夜モード
 */
TEST_P(VhalApiTestSettingAbnormalArgDayNight, AbnormalArg)
{
	VhalApiTestDayNightParameter param = GetParam();

    cockpit::hal::video_hal::Init(obj);

    SetDayNight(obj, OTHER_NUM_VALUE(param.day_night, VIDEO_HAL_SETTING_FORCED_DAY));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.day_night", &param.day_night, sizeof(param.day_night), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ異常
 * テーマカラー
 */
TEST_P(VhalApiTestSettingAbnormalArgThemeColor, AbnormalArg)
{
	VhalApiTestThemeColorParameter param = GetParam();

    cockpit::hal::video_hal::Init(obj);

    SetThemeColor(obj, OTHER_NUM_VALUE(param.theme_color, VIDEO_HAL_THEME_COLOR_FORCED_LIGHT));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.theme_color", &param.theme_color, sizeof(param.theme_color), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	cockpit::hal::video_hal::Deinit(obj);
}
