#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static uint32_t status_x;
static uint32_t status_x_expect;
static uint32_t status_y;
static uint32_t status_y_expect;
static uint32_t status_width;
static uint32_t status_width_expect;
static uint32_t status_height;
static uint32_t status_height_expect;
static bool status_enable;
static bool status_enable_expect;
static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;

static void CallbackX(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.clipping.x", &status_x, sizeof(status_x), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_x_expect, status_x);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackY(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.clipping.y", &status_y, sizeof(status_y), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_y_expect, status_y);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackWidth(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.clipping.width", &status_width, sizeof(status_width), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_width_expect, status_width);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackHeight(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.clipping.height", &status_height, sizeof(status_height), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_height_expect, status_height);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackEnable(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.clipping.enable", &status_enable, sizeof(status_enable), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(status_enable_expect, status_enable);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

struct VhalApiTestVpathFrontClippingParameter {
	int32_t surface_id;
	std::string vpath;
	bool enable;
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
};

struct VhalApiTestVpathFrontClippingRangeParameter {
	int32_t surface_id;
	std::string vpath;
	bool enable;
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
};

struct VhalApiTestVpathFrontClippingNotSetParameter {
	int32_t surface_id;
	std::string vpath;
};

class VhalApiTestVpathFrontClippingNormal : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathFrontClippingParameter> {};
class VhalApiTestVpathFrontClippingRange : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathFrontClippingRangeParameter> {};
class VhalApiTestVpathFrontClippingNotSet : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathFrontClippingNotSetParameter> {};
class VhalApiTestVpathFrontClippingNotSetHdmi : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathFrontClippingNotSetParameter> {};
class VhalApiTestVpathFrontClippingRepeat : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathFrontClippingParameter> {};
class VhalApiTestVpathFrontClippingAbnormalArg : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathFrontClippingParameter> {};
class VhalApiTestVpathFrontClippingAbnormalClear : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathFrontClippingParameter> {};

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathFrontClipping, VhalApiTestVpathFrontClippingNormal, ::testing::Values(
	VhalApiTestVpathFrontClippingParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, true, 400, 200, 100, 100}
));

std::vector<VhalApiTestVpathFrontClippingRangeParameter> GenerateVhalApiTestVpathFrontClippingRangeParameter(void)
{
	uint32_t video_width{};
	uint32_t video_height{};
    GetImageSize(SURFACE_ID_FRONT_DTV, video_width, video_height);

	std::vector<VhalApiTestVpathFrontClippingRangeParameter> v = {
	{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, true, 0, 100, 100, 100},
	{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, true, video_width - 1, 0, 1, 100},
	{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, true, 100, 0, 100, 100},
	{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, true, 100, video_height - 1, 100, 1},
	{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, true, 100, 100, 1, 100},
	{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, true, 0,   100, video_width, 100},
	{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, true, 100, 100, 100, 1},
	{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, true, 100, 0, 100, video_height}
	};

	return v;	
}

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathFrontClipping, VhalApiTestVpathFrontClippingRange, ::testing::ValuesIn(GenerateVhalApiTestVpathFrontClippingRangeParameter()));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathFrontClipping, VhalApiTestVpathFrontClippingNotSet, ::testing::Values(
	VhalApiTestVpathFrontClippingNotSetParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV},
	VhalApiTestVpathFrontClippingNotSetParameter{SURFACE_ID_MEDIA_PLAYER, VPATH_NAME_MEDIAPLAYER},
	VhalApiTestVpathFrontClippingNotSetParameter{SURFACE_ID_DRIVE_RECODER, VPATH_NAME_DRC},
	VhalApiTestVpathFrontClippingNotSetParameter{SURFACE_ID_CARPLAY, VPATH_NAME_CARPLAY},
	VhalApiTestVpathFrontClippingNotSetParameter{SURFACE_ID_ANDROIDAUTO, VPATH_NAME_ANDROIDAUTO},
	VhalApiTestVpathFrontClippingNotSetParameter{SURFACE_ID_MULTISENSORY, VPATH_NAME_MULTISENSORY},
	VhalApiTestVpathFrontClippingNotSetParameter{SURFACE_ID_FRAGRANCE, VPATH_NAME_FRAGRANCE}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathFrontClipping, VhalApiTestVpathFrontClippingNotSetHdmi, ::testing::Values(
	VhalApiTestVpathFrontClippingNotSetParameter{SURFACE_ID_FRONT_HDMI, VPATH_NAME_HDMI}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathFrontClipping, VhalApiTestVpathFrontClippingRepeat, ::testing::Values(
	VhalApiTestVpathFrontClippingParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, true, 10, 20, 800, 600}
));



std::vector<VhalApiTestVpathFrontClippingParameter> GenerateVhalApiTestVpathFrontClippingAbnormalArgParameter(void)
{
	uint32_t video_width{};
	uint32_t video_height{};
    GetImageSize(SURFACE_ID_IC_MAP, video_width, video_height);

	std::vector<VhalApiTestVpathFrontClippingParameter> v = {
	{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, true, video_width, 100, 100, 100},
	{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, true, 0, video_height, 100, 100},
	{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, true, 0, 100, video_width + 1, 100},
	{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, true, 0, 0, 100, video_height + 1}
	};

	return v;	
}

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathFrontClipping, VhalApiTestVpathFrontClippingAbnormalArg, ::testing::ValuesIn(GenerateVhalApiTestVpathFrontClippingAbnormalArgParameter()));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathFrontClipping, VhalApiTestVpathFrontClippingAbnormalClear, ::testing::Values(
	VhalApiTestVpathFrontClippingParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_CLEAR, true, 400, 200, 100, 100}
));

/**
 * 基本動作
 */
TEST_P(VhalApiTestVpathFrontClippingNormal, Normal)
{
	VhalApiTestVpathFrontClippingParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool on{};

	cockpit::hal::video_hal::Init(obj);

	std::string image{TEST_CLIPPING_IMAGE};
	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV, image));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, param.vpath));
	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontClipping(obj, 2, 2, 2, 2, DEFAULT_PROPERTY_CLIPPING_ENABLE, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.clipping.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.clipping.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.clipping.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.clipping.height", CallbackHeight));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.clipping.enable", CallbackEnable));

	/* 前席映像クリッピングサイズ設定 */
	status_x = UINT32_MAX;
	status_y = UINT32_MAX;
	status_width = UINT32_MAX;
	status_height = UINT32_MAX;
	status_enable = false;
	status_x_expect = param.x;
	status_y_expect = param.y;
	status_width_expect = param.width;
	status_height_expect = param.height;
	status_enable_expect = param.enable;
	on = true;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.x", &param.x, sizeof(param.x), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.y", &param.y, sizeof(param.y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.width", &param.width, sizeof(param.width), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.height", &param.height, sizeof(param.height), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.enable", &param.enable, sizeof(param.enable), VIDEO_HAL_DATA_TYPE_BOOL));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for clipping.");

	/* 設定したプロパティがすべて変更されるまで待つ */
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_x_expect == status_x) &&
																					          (status_y_expect == status_y) &&
																							  (status_width_expect == status_width) &&
																							  (status_height_expect == status_height) &&
																							  (status_enable_expect == status_enable); });
	EXPECT_EQ(true, intime);	

	uint32_t control_x{};
	uint32_t control_y{};
	uint32_t control_width{};
	uint32_t control_height{};
	bool control_on{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.control.clipping.x", &control_x, sizeof(control_x), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(param.x, control_x);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.control.clipping.y", &control_y, sizeof(control_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(param.y, control_y);
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.control.clipping.width", &control_width, sizeof(control_width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(param.width, control_width);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.control.clipping.height", &control_height, sizeof(control_height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(param.height, control_height);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.control.clipping.on", &control_on, sizeof(control_on), VIDEO_HAL_DATA_TYPE_BOOL));
    
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.clipping.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.clipping.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.clipping.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.clipping.height", CallbackHeight));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.clipping.enable", CallbackEnable));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontClipping(obj, DEFAULT_PROPERTY_CLIPPING_X, DEFAULT_PROPERTY_CLIPPING_Y, DEFAULT_PROPERTY_CLIPPING_WIDTH, DEFAULT_PROPERTY_CLIPPING_HEIGHT, DEFAULT_PROPERTY_CLIPPING_ENABLE, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ上限下限
 */
TEST_P(VhalApiTestVpathFrontClippingRange, Range)
{
	VhalApiTestVpathFrontClippingRangeParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool on{};

	std::cout << "param x=" << param.x << std::endl;
	std::cout << "param y=" << param.y << std::endl;
	std::cout << "param width=" << param.width << std::endl;
	std::cout << "param height=" << param.height << std::endl;

	cockpit::hal::video_hal::Init(obj);


	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, param.vpath));
	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontClipping(obj, 40, 40, 40, 40, true, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.clipping.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.clipping.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.clipping.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.clipping.height", CallbackHeight));

	/* 前席映像クリッピングサイズ設定 */
	status_x = UINT32_MAX;
	status_y = UINT32_MAX;
	status_width = UINT32_MAX;
	status_height = UINT32_MAX;
	status_enable = false;
	status_x_expect = param.x;
	status_y_expect = param.y;
	status_width_expect = param.width;
	status_height_expect = param.height;
	status_enable_expect = param.enable;
	on = true;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.x", &status_x_expect, sizeof(status_x_expect), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.y", &status_y_expect, sizeof(status_y_expect), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.width", &status_width_expect, sizeof(status_width_expect), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.height", &status_height_expect, sizeof(status_height_expect), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.enable", &param.enable, sizeof(param.enable), VIDEO_HAL_DATA_TYPE_BOOL));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for clipping.");

	/* 設定したプロパティがすべて変更されるまで待つ */
	EXPECT_EQ(true, cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_x_expect == status_x); }));
	EXPECT_EQ(true, cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_y_expect == status_y); }));
	EXPECT_EQ(true, cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_width_expect == status_width); }));
	EXPECT_EQ(true, cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_height_expect == status_height); }));
    
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.clipping.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.clipping.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.clipping.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.clipping.height", CallbackHeight));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontClipping(obj, DEFAULT_PROPERTY_CLIPPING_X, DEFAULT_PROPERTY_CLIPPING_Y, DEFAULT_PROPERTY_CLIPPING_WIDTH, DEFAULT_PROPERTY_CLIPPING_HEIGHT, DEFAULT_PROPERTY_CLIPPING_ENABLE, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 */
TEST_P(VhalApiTestVpathFrontClippingNotSet, NotSet)
{
	VhalApiTestVpathFrontClippingNotSetParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);


	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, param.vpath));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.clipping.x", &status_x, sizeof(status_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(0, status_x);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.clipping.y", &status_y, sizeof(status_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(0, status_y);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.clipping.width", &status_width, sizeof(status_width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(0, status_width);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.clipping.height", &status_height, sizeof(status_height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(0, status_height);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.clipping.enable", &status_enable, sizeof(status_enable), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, status_enable);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 */
TEST_P(VhalApiTestVpathFrontClippingNotSetHdmi, NotSet)
{
	VhalApiTestVpathFrontClippingNotSetParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, param.vpath));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.clipping.x", &status_x, sizeof(status_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(0, status_x);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.clipping.y", &status_y, sizeof(status_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(0, status_y);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.clipping.width", &status_width, sizeof(status_width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(0, status_width);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.clipping.height", &status_height, sizeof(status_height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(0, status_height);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.clipping.enable", &status_enable, sizeof(status_enable), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, status_enable);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 */
TEST_P(VhalApiTestVpathFrontClippingRepeat, Repeat)
{
	VhalApiTestVpathFrontClippingParameter param = GetParam();
	bool on{};

	cockpit::hal::video_hal::Init(obj);


	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, param.vpath));
	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontClipping(obj, DEFAULT_PROPERTY_CLIPPING_X, DEFAULT_PROPERTY_CLIPPING_Y, DEFAULT_PROPERTY_CLIPPING_WIDTH, DEFAULT_PROPERTY_CLIPPING_HEIGHT, DEFAULT_PROPERTY_CLIPPING_ENABLE, true));

	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		on = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.x", &param.x, sizeof(param.x), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.y", &param.y, sizeof(param.y), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.width", &param.width, sizeof(param.width), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.height", &param.height, sizeof(param.height), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.enable", &param.enable, sizeof(param.enable), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	}

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontClipping(obj, DEFAULT_PROPERTY_CLIPPING_X, DEFAULT_PROPERTY_CLIPPING_Y, DEFAULT_PROPERTY_CLIPPING_WIDTH, DEFAULT_PROPERTY_CLIPPING_HEIGHT, DEFAULT_PROPERTY_CLIPPING_ENABLE, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}



/**
 * パラメータ異常
 */
TEST_P(VhalApiTestVpathFrontClippingAbnormalArg, AbnormalArg)
{
	VhalApiTestVpathFrontClippingParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool on{};

	cockpit::hal::video_hal::Init(obj);
	std::string image{TEST_CLIPPING_IMAGE};
	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id, image));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, param.vpath));
	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	uint32_t video_width{};
	uint32_t video_height{};
    GetImageSize(SURFACE_ID_FRONT_DTV, video_width, video_height);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontClipping(obj, DEFAULT_PROPERTY_CLIPPING_X, DEFAULT_PROPERTY_CLIPPING_Y, video_width, video_height, DEFAULT_PROPERTY_CLIPPING_ENABLE, true));

	/* 前席映像クリッピングサイズ設定 */
	on = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.x", &param.x, sizeof(param.x), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.y", &param.y, sizeof(param.y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.width", &param.width, sizeof(param.width), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.height", &param.height, sizeof(param.height), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.enable", &param.enable, sizeof(param.enable), VIDEO_HAL_DATA_TYPE_BOOL));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontClipping(obj, DEFAULT_PROPERTY_CLIPPING_X, DEFAULT_PROPERTY_CLIPPING_Y, DEFAULT_PROPERTY_CLIPPING_WIDTH, DEFAULT_PROPERTY_CLIPPING_HEIGHT, DEFAULT_PROPERTY_CLIPPING_ENABLE, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 前提を満たさない条件でのコール
 * 映像パス＝クリアでクリッピングサイズを設定する
 */
TEST_P(VhalApiTestVpathFrontClippingAbnormalClear, AbnormalClear)
{
	VhalApiTestVpathFrontClippingParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool on{};

	cockpit::hal::video_hal::Init(obj);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, param.vpath));

	/* 前席映像クリッピングサイズ設定 */
	on = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.x", &param.x, sizeof(param.x), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.y", &param.y, sizeof(param.y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.width", &param.width, sizeof(param.width), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.height", &param.height, sizeof(param.height), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.enable", &param.enable, sizeof(param.enable), VIDEO_HAL_DATA_TYPE_BOOL));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	cockpit::hal::video_hal::Deinit(obj);
}