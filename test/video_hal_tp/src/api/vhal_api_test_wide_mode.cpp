#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static uint32_t status_widemode;
static uint32_t status_widemode_expect;
static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;

#define ABNORMAL_ARG_WIDEMODE (VIDEO_HAL_WIDE_MODE_ZOOMED + 1)

static void CallbackWideMode(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.widemode", &status_widemode, sizeof(status_widemode), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_widemode_expect, status_widemode);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackWideModeRear(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.rear.status.widemode", &status_widemode, sizeof(status_widemode), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_widemode_expect, status_widemode);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

struct VhalApiTestWideModeParameter {
    uint32_t widemode;
};

class VhalApiTestWideModeNormal : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestWideModeParameter> {};
class VhalApiTestWideModeRearNormal : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestWideModeParameter> {};
class VhalApiTestWideModeRepeat : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestWideModeParameter> {};
class VhalApiTestWideModeRearRepeat : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestWideModeParameter> {};
class VhalApiTestWideModeAbnormalArg : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestWideModeParameter> {};
class VhalApiTestWideModeAbnormalClear : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestWideModeParameter> {};
class VhalApiTestWideModeAbnormalNotSet : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestWideModeParameter> {};
class VhalApiTestWideModeRearAbnormalArg : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestWideModeParameter> {};
class VhalApiTestWideModeRearAbnormalClear : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestWideModeParameter> {};
class VhalApiTestWideModeRearAbnormalNotSet : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestWideModeParameter> {};

INSTANTIATE_TEST_CASE_P(VhalApiTestWideMode, VhalApiTestWideModeNormal, ::testing::Values(
	VhalApiTestWideModeParameter{VIDEO_HAL_WIDE_MODE_NORMAL},
	VhalApiTestWideModeParameter{VIDEO_HAL_WIDE_MODE_STRETCHED},
	VhalApiTestWideModeParameter{VIDEO_HAL_WIDE_MODE_ZOOMED}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestWideMode, VhalApiTestWideModeRearNormal, ::testing::Values(
	VhalApiTestWideModeParameter{VIDEO_HAL_WIDE_MODE_NORMAL},
	VhalApiTestWideModeParameter{VIDEO_HAL_WIDE_MODE_STRETCHED},
	VhalApiTestWideModeParameter{VIDEO_HAL_WIDE_MODE_ZOOMED}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestWideMode, VhalApiTestWideModeRepeat, ::testing::Values(
	VhalApiTestWideModeParameter{VIDEO_HAL_WIDE_MODE_NORMAL}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestWideMode, VhalApiTestWideModeRearRepeat, ::testing::Values(
	VhalApiTestWideModeParameter{VIDEO_HAL_WIDE_MODE_NORMAL}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestWideMode, VhalApiTestWideModeAbnormalArg, ::testing::Values(
	VhalApiTestWideModeParameter{ABNORMAL_ARG_WIDEMODE}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestWideMode, VhalApiTestWideModeAbnormalClear, ::testing::Values(
	VhalApiTestWideModeParameter{VIDEO_HAL_WIDE_MODE_STRETCHED}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestWideMode, VhalApiTestWideModeAbnormalNotSet, ::testing::Values(
	VhalApiTestWideModeParameter{VIDEO_HAL_WIDE_MODE_NORMAL}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestWideMode, VhalApiTestWideModeRearAbnormalArg, ::testing::Values(
	VhalApiTestWideModeParameter{ABNORMAL_ARG_WIDEMODE}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestWideMode, VhalApiTestWideModeRearAbnormalClear, ::testing::Values(
	VhalApiTestWideModeParameter{VIDEO_HAL_WIDE_MODE_STRETCHED}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestWideMode, VhalApiTestWideModeRearAbnormalNotSet, ::testing::Values(
	VhalApiTestWideModeParameter{VIDEO_HAL_WIDE_MODE_NORMAL}
));

/**
 * 基本動作
 * 前席映像ワイド設定
 */
TEST_P(VhalApiTestWideModeNormal, Normal)
{
	VhalApiTestWideModeParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};

	cockpit::hal::video_hal::Init(obj);

	std::string image{TEST_WIDE_IMAGE};
	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV, image));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetWideModeFront(obj, OTHER_NUM_VALUE(param.widemode, VIDEO_HAL_WIDE_MODE_ZOOMED)));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.widemode", CallbackWideMode));

	/* 前席映像ワイド設定 */
	status_widemode = VIDEO_HAL_WIDE_MODE_ZOOMED + 1;
	status_widemode_expect = param.widemode;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.widemode", &param.widemode, sizeof(param.widemode), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for widemode.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_widemode_expect == status_widemode); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.control.widemode", &status_widemode, sizeof(status_widemode), VIDEO_HAL_DATA_TYPE_NUM));
    
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.widemode", CallbackWideMode));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetWideModeFront(obj, DEFAULT_PROPERTY_WIDEMODE));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * 後席映像ワイド設定
 */
TEST_P(VhalApiTestWideModeRearNormal, Normal)
{
	VhalApiTestWideModeParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};

	cockpit::hal::video_hal::Init(obj);

	std::string image{TEST_WIDE_IMAGE};
	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV, image));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_DTV));
	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_REAR, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetRearOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetWideModeRear(obj, OTHER_NUM_VALUE(param.widemode, VIDEO_HAL_WIDE_MODE_ZOOMED)));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.widemode", CallbackWideModeRear));

	/* 後席映像ワイド設定 */
	status_widemode = VIDEO_HAL_WIDE_MODE_ZOOMED + 1;
	status_widemode_expect = param.widemode;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.widemode", &param.widemode, sizeof(param.widemode), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for widemode.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_widemode_expect == status_widemode); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.rear.control.widemode", &status_widemode, sizeof(status_widemode), VIDEO_HAL_DATA_TYPE_NUM));
    
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleRear(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.widemode", CallbackWideModeRear));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetWideModeRear(obj, DEFAULT_PROPERTY_WIDEMODE));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_CLEAR));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 * 前席
 */
TEST(VhalApiTestWideModeFrontNotSet, FrontNotSet)
{
	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.widemode", &status_widemode, sizeof(status_widemode), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_WIDE_MODE_NORMAL, status_widemode);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 * 後席
 */
TEST(VhalApiTestWideModeRearNotSet, RearNotSet)
{
	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.rear.status.widemode", &status_widemode, sizeof(status_widemode), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_WIDE_MODE_NORMAL, status_widemode);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 * 前席
 */
TEST_P(VhalApiTestWideModeRepeat, Repeat)
{
	VhalApiTestWideModeParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetWideModeFront(obj, OTHER_NUM_VALUE(param.widemode, VIDEO_HAL_WIDE_MODE_ZOOMED)));

	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.widemode", &param.widemode, sizeof(param.widemode), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	}

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetWideModeFront(obj, DEFAULT_PROPERTY_WIDEMODE));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 * 後席
 */
TEST_P(VhalApiTestWideModeRearRepeat, Repeat)
{
	VhalApiTestWideModeParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_DTV));
		struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_REAR, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetRearOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetWideModeRear(obj, OTHER_NUM_VALUE(param.widemode, VIDEO_HAL_WIDE_MODE_ZOOMED)));

	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.widemode", &param.widemode, sizeof(param.widemode), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	}

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetWideModeRear(obj, DEFAULT_PROPERTY_WIDEMODE));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ異常 
 * 前席
 */
TEST_P(VhalApiTestWideModeAbnormalArg, AbnormalArg)
{
	VhalApiTestWideModeParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};

	cockpit::hal::video_hal::Init(obj);

	std::string image{TEST_WIDE_IMAGE};
	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV, image));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetWideModeFront(obj, VIDEO_HAL_WIDE_MODE_NORMAL));

	/* 前席映像ワイド設定 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.widemode", &param.widemode, sizeof(param.widemode), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetWideModeFront(obj, DEFAULT_PROPERTY_WIDEMODE));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ異常 
 * 後席
 */
TEST_P(VhalApiTestWideModeRearAbnormalArg, AbnormalArg)
{
	VhalApiTestWideModeParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};

	cockpit::hal::video_hal::Init(obj);

	std::string image{TEST_WIDE_IMAGE};
	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV, image));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_DTV));
	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_REAR, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetRearOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetWideModeRear(obj, VIDEO_HAL_WIDE_MODE_NORMAL));

	/* 後席映像ワイド設定 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.widemode", &param.widemode, sizeof(param.widemode), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetWideModeRear(obj, DEFAULT_PROPERTY_WIDEMODE));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 前提を満たさない条件でのコール
 * 映像パス＝クリアでワイドモード設定
 * 前席
 */
TEST_P(VhalApiTestWideModeAbnormalClear, AbnormalClear)
{
	VhalApiTestWideModeParameter param = GetParam();
	bool intime{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.widemode", &param.widemode, sizeof(param.widemode), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 前提を満たさない条件でのコール
 * 映像パス＝クリアでワイドモード設定
 * 後席
 */
TEST_P(VhalApiTestWideModeRearAbnormalClear, AbnormalClear)
{
	VhalApiTestWideModeParameter param = GetParam();
	bool intime{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.widemode", &param.widemode, sizeof(param.widemode), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 前提を満たさない条件でのコール
 * 出力サイズ未設定でワイドモード設定
 * 前席
 */
TEST_P(VhalApiTestWideModeAbnormalNotSet, AbnormalNotSet)
{
	VhalApiTestWideModeParameter param = GetParam();
	bool intime{};

	cockpit::hal::video_hal::Init(obj);

	std::string image{TEST_WIDE_IMAGE};
	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV, image));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetWideModeFront(obj, OTHER_NUM_VALUE(param.widemode, VIDEO_HAL_WIDE_MODE_ZOOMED)));

	/* 前席映像ワイド設定 */
	status_widemode = VIDEO_HAL_WIDE_MODE_ZOOMED + 1;
	status_widemode_expect = param.widemode;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.widemode", &param.widemode, sizeof(param.widemode), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetWideModeFront(obj, DEFAULT_PROPERTY_WIDEMODE));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 前提を満たさない条件でのコール
 * 出力サイズ未設定でワイドモード設定
 * 後席
 */
TEST_P(VhalApiTestWideModeRearAbnormalNotSet, AbnormalNotSet)
{
	VhalApiTestWideModeParameter param = GetParam();
	bool intime{};

	cockpit::hal::video_hal::Init(obj);

	std::string image{TEST_WIDE_IMAGE};
	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV, image));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetWideModeRear(obj, OTHER_NUM_VALUE(param.widemode, VIDEO_HAL_WIDE_MODE_ZOOMED)));

	/* 前席映像ワイド設定 */
	status_widemode = VIDEO_HAL_WIDE_MODE_ZOOMED + 1;
	status_widemode_expect = param.widemode;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.widemode", &param.widemode, sizeof(param.widemode), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetWideModeRear(obj, DEFAULT_PROPERTY_WIDEMODE));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}