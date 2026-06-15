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
static std::string cb_available_param;
static bool status_available;
static bool status_available_expect;
static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;

static void CallbackX(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.status.clipping.x", &status_x, sizeof(status_x), VIDEO_HAL_DATA_TYPE_NUM));
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
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.status.clipping.y", &status_y, sizeof(status_y), VIDEO_HAL_DATA_TYPE_NUM));
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
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.status.clipping.width", &status_width, sizeof(status_width), VIDEO_HAL_DATA_TYPE_NUM));
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
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.status.clipping.height", &status_height, sizeof(status_height), VIDEO_HAL_DATA_TYPE_NUM));
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
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.status.clipping.enable", &status_enable, sizeof(status_enable), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(status_enable_expect, status_enable);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

struct VhalApiTestVpathIcClippingParameter {
	int32_t surface_id;
	std::string vpath;
	bool enable;
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
};

struct VhalApiTestVpathIcClippingRangeParameter {
	int32_t surface_id;
	std::string vpath;
	bool enable;
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
};

struct VhalApiTestVpathIcClippingNotSetParameter {
	std::string vpath;
	uint32_t surface_id;
};

class VhalApiTestVpathIcClippingNormal : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathIcClippingParameter> {};
class VhalApiTestVpathIcClippingRange : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathIcClippingRangeParameter> {};
class VhalApiTestVpathIcClippingNotSet : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathIcClippingNotSetParameter> {};
class VhalApiTestVpathIcClippingRepeat : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathIcClippingParameter> {};
class VhalApiTestVpathIcClippingAbnormalArg : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathIcClippingParameter> {};
class VhalApiTestVpathIcClippingAbnormalClear : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathIcClippingParameter> {};

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathIcClipping, VhalApiTestVpathIcClippingNormal, ::testing::Values(
	VhalApiTestVpathIcClippingParameter{SURFACE_ID_IC_MAP, VPATH_NAME_MAP, true, 400, 200, 100, 100}
));

std::vector<VhalApiTestVpathIcClippingRangeParameter> GenerateVhalApiTestVpathIcClippingRangeParameter(void)
{
	uint32_t video_width{};
	uint32_t video_height{};
    GetImageSize(SURFACE_ID_IC_MAP, video_width, video_height);

	std::vector<VhalApiTestVpathIcClippingRangeParameter> v = {
	{SURFACE_ID_IC_MAP, VPATH_NAME_MAP, true, 0, 100, 100, 100},
	{SURFACE_ID_IC_MAP, VPATH_NAME_MAP, true, video_width - 1, 0, 1, 100},
	{SURFACE_ID_IC_MAP, VPATH_NAME_MAP, true, 100, 0, 100, 100},
	{SURFACE_ID_IC_MAP, VPATH_NAME_MAP, true, 100, video_height - 1, 100, 1},
	{SURFACE_ID_IC_MAP, VPATH_NAME_MAP, true, 100, 100, 1, 100},
	{SURFACE_ID_IC_MAP, VPATH_NAME_MAP, true, 0,   100, video_width, 100},
	{SURFACE_ID_IC_MAP, VPATH_NAME_MAP, true, 100, 100, 100, 1},
	{SURFACE_ID_IC_MAP, VPATH_NAME_MAP, true, 100, 0, 100, video_height}
	};

	return v;	
}

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathIcClipping, VhalApiTestVpathIcClippingRange, ::testing::ValuesIn(GenerateVhalApiTestVpathIcClippingRangeParameter()));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathIcClipping, VhalApiTestVpathIcClippingNotSet, ::testing::Values(
	VhalApiTestVpathIcClippingNotSetParameter{VPATH_NAME_MAP, SURFACE_ID_IC_MAP},
	VhalApiTestVpathIcClippingNotSetParameter{VPATH_NAME_CARPLAY, SURFACE_ID_IC_CARPLAY},
	VhalApiTestVpathIcClippingNotSetParameter{VPATH_NAME_ANDROIDAUTO, SURFACE_ID_IC_ANDROIDAUTO}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathIcClipping, VhalApiTestVpathIcClippingRepeat, ::testing::Values(
	VhalApiTestVpathIcClippingParameter{SURFACE_ID_IC_MAP, VPATH_NAME_MAP, true, 10, 20, 800, 600}
));


std::vector<VhalApiTestVpathIcClippingParameter> GenerateVhalApiTestVpathIcClippingAbnormalArgParameter(void)
{
	uint32_t video_width{};
	uint32_t video_height{};
    GetImageSize(SURFACE_ID_IC_MAP, video_width, video_height);

	std::vector<VhalApiTestVpathIcClippingParameter> v = {
	{SURFACE_ID_IC_MAP, VPATH_NAME_MAP, true, video_width, 100, 100, 100},
	{SURFACE_ID_IC_MAP, VPATH_NAME_MAP, true, 0, video_height, 100, 100},
	{SURFACE_ID_IC_MAP, VPATH_NAME_MAP, true, 0, 0, video_width + 1, 100},
	{SURFACE_ID_IC_MAP, VPATH_NAME_MAP, true, 0, 0, 100, video_height + 1}
	};

	return v;	
}

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathIcClipping, VhalApiTestVpathIcClippingAbnormalArg, ::testing::ValuesIn(GenerateVhalApiTestVpathIcClippingAbnormalArgParameter()));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathIcClipping, VhalApiTestVpathIcClippingAbnormalClear, ::testing::Values(
	VhalApiTestVpathIcClippingParameter{SURFACE_ID_IC_MAP, VPATH_NAME_CLEAR, true, 400, 200, 100, 100}
));

/**
 * 基本動作
 */
TEST_P(VhalApiTestVpathIcClippingNormal, Normal)
{
	VhalApiTestVpathIcClippingParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool on{};

	cockpit::hal::video_hal::Init(obj);
	std::string image{TEST_CLIPPING_IMAGE};
	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id, image));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, param.vpath));
	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_IC, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetIcOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetIcClipping(obj, DEFAULT_PROPERTY_CLIPPING_X, DEFAULT_PROPERTY_CLIPPING_Y, DEFAULT_PROPERTY_CLIPPING_WIDTH, DEFAULT_PROPERTY_CLIPPING_HEIGHT, DEFAULT_PROPERTY_CLIPPING_ENABLE, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.clipping.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.clipping.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.clipping.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.clipping.height", CallbackHeight));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.clipping.enable", CallbackEnable));

	/* IC映像クリッピングサイズ設定 */
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

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.x", &param.x, sizeof(param.x), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.y", &param.y, sizeof(param.y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.width", &param.width, sizeof(param.width), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.height", &param.height, sizeof(param.height), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.enable", &param.enable, sizeof(param.enable), VIDEO_HAL_DATA_TYPE_BOOL));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
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
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.control.clipping.x", &control_x, sizeof(control_x), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(param.x, control_x);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.control.clipping.y", &control_y, sizeof(control_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(param.y, control_y);
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.control.clipping.width", &control_width, sizeof(control_width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(param.width, control_width);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.control.clipping.height", &control_height, sizeof(control_height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(param.height, control_height);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.control.clipping.on", &control_on, sizeof(control_on), VIDEO_HAL_DATA_TYPE_BOOL));

    
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleIc(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.clipping.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.clipping.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.clipping.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.clipping.height", CallbackHeight));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.clipping.enable", CallbackEnable));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetIcClipping(obj, DEFAULT_PROPERTY_CLIPPING_X, DEFAULT_PROPERTY_CLIPPING_Y, DEFAULT_PROPERTY_CLIPPING_WIDTH, DEFAULT_PROPERTY_CLIPPING_HEIGHT, DEFAULT_PROPERTY_CLIPPING_ENABLE, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ上限下限
 */
TEST_P(VhalApiTestVpathIcClippingRange, Range)
{
	VhalApiTestVpathIcClippingRangeParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool on{};

	std::cout << "param x=" << param.x << std::endl;
	std::cout << "param y=" << param.y << std::endl;
	std::cout << "param width=" << param.width << std::endl;
	std::cout << "param height=" << param.height << std::endl;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, param.vpath));
	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_IC, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetIcOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetIcClipping(obj, 40, 40, 40, 40, true, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.clipping.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.clipping.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.clipping.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.clipping.height", CallbackHeight));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.clipping.enable", CallbackEnable));

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

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.x", &status_x_expect, sizeof(status_x_expect), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.y", &status_y_expect, sizeof(status_y_expect), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.width", &status_width_expect, sizeof(status_width_expect), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.height", &status_height_expect, sizeof(status_height_expect), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.enable", &param.enable, sizeof(param.enable), VIDEO_HAL_DATA_TYPE_BOOL));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for clipping.");

	/* 設定したプロパティがすべて変更されるまで待つ */
	EXPECT_EQ(true, cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_x_expect == status_x) &&
																					        	 	 (status_y_expect == status_y) &&
																									 (status_width_expect == status_width) &&
																									 (status_height_expect == status_height); }));

	uint32_t control_x{};
	uint32_t control_y{};
	uint32_t control_width{};
	uint32_t control_height{};
	bool control_on{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.control.clipping.x", &control_x, sizeof(control_x), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(status_x_expect, control_x);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.control.clipping.y", &control_y, sizeof(control_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(status_y_expect, control_y);
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.control.clipping.width", &control_width, sizeof(control_width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(status_width_expect, control_width);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.control.clipping.height", &control_height, sizeof(control_height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(status_height_expect, control_height);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.control.clipping.on", &control_on, sizeof(control_on), VIDEO_HAL_DATA_TYPE_BOOL));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleIc(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.clipping.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.clipping.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.clipping.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.clipping.height", CallbackHeight));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.clipping.enable", CallbackEnable));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetIcClipping(obj, DEFAULT_PROPERTY_CLIPPING_X, DEFAULT_PROPERTY_CLIPPING_Y, DEFAULT_PROPERTY_CLIPPING_WIDTH, DEFAULT_PROPERTY_CLIPPING_HEIGHT, DEFAULT_PROPERTY_CLIPPING_ENABLE, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 */
TEST_P(VhalApiTestVpathIcClippingNotSet, NotSet)
{
	VhalApiTestVpathIcClippingNotSetParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	ExecuteRenderImage(obj, param.surface_id);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, param.vpath));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.status.clipping.x", &status_x, sizeof(status_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(0, status_x);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.status.clipping.y", &status_y, sizeof(status_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(0, status_y);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.status.clipping.width", &status_width, sizeof(status_width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(0, status_width);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.status.clipping.height", &status_height, sizeof(status_height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(0, status_height);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.status.clipping.enable", &status_enable, sizeof(status_enable), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, status_enable);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 */
TEST_P(VhalApiTestVpathIcClippingRepeat, Repeat)
{
	VhalApiTestVpathIcClippingParameter param = GetParam();
	bool on{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, param.vpath));
	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_IC, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetIcOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetIcClipping(obj, DEFAULT_PROPERTY_CLIPPING_X, DEFAULT_PROPERTY_CLIPPING_Y, DEFAULT_PROPERTY_CLIPPING_WIDTH, DEFAULT_PROPERTY_CLIPPING_HEIGHT, DEFAULT_PROPERTY_CLIPPING_ENABLE, true));

	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		on = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.x", &param.x, sizeof(param.x), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.y", &param.y, sizeof(param.y), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.width", &param.width, sizeof(param.width), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.height", &param.height, sizeof(param.height), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.enable", &param.enable, sizeof(param.enable), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	}

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetIcClipping(obj, DEFAULT_PROPERTY_CLIPPING_X, DEFAULT_PROPERTY_CLIPPING_Y, DEFAULT_PROPERTY_CLIPPING_WIDTH, DEFAULT_PROPERTY_CLIPPING_HEIGHT, DEFAULT_PROPERTY_CLIPPING_ENABLE, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ異常
 */
TEST_P(VhalApiTestVpathIcClippingAbnormalArg, AbnormalArg)
{
	VhalApiTestVpathIcClippingParameter param = GetParam();
	bool on{};

	cockpit::hal::video_hal::Init(obj);
	std::string image{TEST_CLIPPING_IMAGE};
	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id, image));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, param.vpath));
	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_IC, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetIcOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	uint32_t video_width{};
	uint32_t video_height{};
    GetImageSize(SURFACE_ID_IC_MAP, video_width, video_height);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetIcClipping(obj, DEFAULT_PROPERTY_CLIPPING_X, DEFAULT_PROPERTY_CLIPPING_Y, video_width, video_height, DEFAULT_PROPERTY_CLIPPING_ENABLE, true));

	/* 前席映像クリッピングサイズ設定 */
	on = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.x", &param.x, sizeof(param.x), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.y", &param.y, sizeof(param.y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.width", &param.width, sizeof(param.width), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.height", &param.height, sizeof(param.height), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.enable", &param.enable, sizeof(param.enable), VIDEO_HAL_DATA_TYPE_BOOL));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetIcClipping(obj, DEFAULT_PROPERTY_CLIPPING_X, DEFAULT_PROPERTY_CLIPPING_Y, DEFAULT_PROPERTY_CLIPPING_WIDTH, DEFAULT_PROPERTY_CLIPPING_HEIGHT, DEFAULT_PROPERTY_CLIPPING_ENABLE, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 前提を満たさない条件でのコール
 * 映像パス＝クリアでクリッピングサイズを設定する
 */
TEST_P(VhalApiTestVpathIcClippingAbnormalClear, AbnormalClear)
{
	VhalApiTestVpathIcClippingParameter param = GetParam();
	bool on{};

	cockpit::hal::video_hal::Init(obj);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, param.vpath));

	/* IC映像クリッピングサイズ設定 */
	on = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.x", &param.x, sizeof(param.x), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.y", &param.y, sizeof(param.y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.width", &param.width, sizeof(param.width), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.height", &param.height, sizeof(param.height), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.enable", &param.enable, sizeof(param.enable), VIDEO_HAL_DATA_TYPE_BOOL));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	cockpit::hal::video_hal::Deinit(obj);
}