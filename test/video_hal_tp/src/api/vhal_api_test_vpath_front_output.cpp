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
static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;

static void CallbackX(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.front.status.output.x", &status_x, sizeof(status_x), VIDEO_HAL_DATA_TYPE_NUM));
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
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.front.status.output.y", &status_y, sizeof(status_y), VIDEO_HAL_DATA_TYPE_NUM));
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
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.front.status.output.width", &status_width, sizeof(status_width), VIDEO_HAL_DATA_TYPE_NUM));
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
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.output.height", &status_height, sizeof(status_height), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_height_expect, status_height);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

struct VhalApiTestFrontOutputParameter {
	int32_t surface_id;
	std::string vpath;
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
};

struct VhalApiTestFrontOutputNotSetParameter {
	std::string vpath;
	int32_t surface_id;
};

class VhalApiTestVpathFrontOutputNormal : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestFrontOutputParameter> {};
class VhalApiTestVpathFrontOutputRange : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestFrontOutputParameter> {};
class VhalApiTestVpathFrontOutputNotSet : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestFrontOutputNotSetParameter> {};
class VhalApiTestVpathFrontOutputNotSetHdmi : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestFrontOutputNotSetParameter> {};
class VhalApiTestVpathFrontOutputRepeat : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestFrontOutputParameter> {};
class VhalApiTestVpathFrontOutputAbnormalArg : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestFrontOutputParameter> {};
class VhalApiTestVpathFrontOutputAbnormalClear : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestFrontOutputParameter> {};

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathFrontOutput, VhalApiTestVpathFrontOutputNormal, ::testing::Values(
	VhalApiTestFrontOutputParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 10, 10, 800, 600}
));

std::vector<VhalApiTestFrontOutputParameter> GenerateVhalApiTestFrontOutputParameter(void)
{
    struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);

	std::vector<VhalApiTestFrontOutputParameter> v = {
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 0, 100, 100, 100},
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, prop.screenWidth - 1, 100, 1, 100},
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 100, 0, 100, 100},
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 100, prop.screenHeight - 1, 100, 1},
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 100, 100, 1, 100},
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 0, 100, prop.screenWidth, 100},
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 100, 100, 100, 1},
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 100, 0, 100, prop.screenHeight}
	};

	return v;
}

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathFrontOutput, VhalApiTestVpathFrontOutputRange, ::testing::ValuesIn(GenerateVhalApiTestFrontOutputParameter()));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathFrontOutput, VhalApiTestVpathFrontOutputNotSet, ::testing::Values(
	VhalApiTestFrontOutputNotSetParameter{VPATH_NAME_DTV, SURFACE_ID_FRONT_DTV},
	VhalApiTestFrontOutputNotSetParameter{VPATH_NAME_MEDIAPLAYER, SURFACE_ID_MEDIA_PLAYER},
	VhalApiTestFrontOutputNotSetParameter{VPATH_NAME_DRC, SURFACE_ID_DRIVE_RECODER},
	VhalApiTestFrontOutputNotSetParameter{VPATH_NAME_CARPLAY, SURFACE_ID_CARPLAY},
	VhalApiTestFrontOutputNotSetParameter{VPATH_NAME_ANDROIDAUTO, SURFACE_ID_ANDROIDAUTO},
	VhalApiTestFrontOutputNotSetParameter{VPATH_NAME_MULTISENSORY, SURFACE_ID_MULTISENSORY},
	VhalApiTestFrontOutputNotSetParameter{VPATH_NAME_FRAGRANCE, SURFACE_ID_FRAGRANCE}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathFrontOutput, VhalApiTestVpathFrontOutputNotSetHdmi, ::testing::Values(
	VhalApiTestFrontOutputNotSetParameter{VPATH_NAME_HDMI}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathFrontOutput, VhalApiTestVpathFrontOutputRepeat, ::testing::Values(
	VhalApiTestFrontOutputParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 10, 10, 800, 600}
));

std::vector<VhalApiTestFrontOutputParameter> GenerateVhalApiTestFrontOutputAbnormalArgParameter(void)
{
    struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);

	std::vector<VhalApiTestFrontOutputParameter> v = {
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, prop.screenWidth, 100, 100, 100},
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 100, prop.screenHeight, 100, 100},
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 0, 100, prop.screenWidth + 1, 100},
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 100, 0, 100, prop.screenHeight + 1}
	};

	return v;
}

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathFrontOutput, VhalApiTestVpathFrontOutputAbnormalArg, ::testing::ValuesIn(GenerateVhalApiTestFrontOutputAbnormalArgParameter()));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathFrontOutput, VhalApiTestVpathFrontOutputAbnormalClear, ::testing::Values(
	VhalApiTestFrontOutputParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 10, 10, 800, 600}
));

/**
 * 基本動作
 */
TEST_P(VhalApiTestVpathFrontOutputNormal, Normal)
{
	VhalApiTestFrontOutputParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	uint32_t x{};
	uint32_t y{};
	uint32_t width{};
	uint32_t height{};
	bool on{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, param.vpath));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, 10, 10, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.output.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.output.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.output.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.output.height", CallbackHeight));

	/* 映像出力サイズ設定 */
	status_x = 0;
	status_y = 0;
	status_width = 0;
	status_height = 0;
	status_x_expect = param.x;
	status_y_expect = param.y;
	status_width_expect = param.width;
	status_height_expect = param.height;
	x = status_x_expect;
	y = status_y_expect;
	width = status_width_expect;
	height = status_height_expect;
	on = true;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.x", &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.y", &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for output.");

	/* 設定したプロパティがすべて変更されるまで待つ */
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_x_expect == status_x) &&
																					          (status_y_expect == status_y) &&
																							  (status_width_expect == status_width) &&
																							  (status_height_expect == status_height); });
	EXPECT_EQ(true, intime);

	uint32_t control_x{};
	uint32_t control_y{};
	uint32_t control_width{};
	uint32_t control_height{};
	bool control_on{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.control.output.x", &control_x, sizeof(control_x), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(x, control_x);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.control.output.y", &control_y, sizeof(control_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(y, control_y);
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.control.output.width", &control_width, sizeof(control_width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(width, control_width);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.control.output.height", &control_height, sizeof(control_height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(height, control_height);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.control.output.on", &control_on, sizeof(control_on), VIDEO_HAL_DATA_TYPE_BOOL));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.output.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.output.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.output.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.output.height", CallbackHeight));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ上下限
 */
TEST_P(VhalApiTestVpathFrontOutputRange, Range)
{
	VhalApiTestFrontOutputParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	uint32_t x{};
	uint32_t y{};
	uint32_t width{};
	uint32_t height{};
	bool on{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, param.vpath));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, 10, 10, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.output.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.output.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.output.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.output.height", CallbackHeight));

	/* 映像出力サイズ設定 */
	status_x = 0;
	status_y = 0;
	status_width = 0;
	status_height = 0;
	status_x_expect = param.x;
	status_y_expect = param.y;
	status_width_expect = param.width;
	status_height_expect = param.height;
	x = status_x_expect;
	y = status_y_expect;
	width = status_width_expect;
	height = status_height_expect;
	on = true;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.x", &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.y", &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for output.");

	/* 設定したプロパティがすべて変更されるまで待つ */
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_x_expect == status_x) &&
																					          (status_y_expect == status_y) &&
																							  (status_width_expect == status_width) &&
																							  (status_height_expect == status_height); });
	EXPECT_EQ(true, intime);

	uint32_t control_x{};
	uint32_t control_y{};
	uint32_t control_width{};
	uint32_t control_height{};
	bool control_on{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.control.output.x", &control_x, sizeof(control_x), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(x, control_x);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.control.output.y", &control_y, sizeof(control_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(y, control_y);
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.control.output.width", &control_width, sizeof(control_width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(width, control_width);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.control.output.height", &control_height, sizeof(control_height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(height, control_height);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.control.output.on", &control_on, sizeof(control_on), VIDEO_HAL_DATA_TYPE_BOOL));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.output.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.output.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.output.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.output.height", CallbackHeight));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 */
TEST_P(VhalApiTestVpathFrontOutputNotSet, NotSet)
{
	VhalApiTestFrontOutputNotSetParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));

    struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, param.vpath));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.front.status.output.x", &status_x, sizeof(status_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(0, status_x);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.front.status.output.y", &status_y, sizeof(status_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(0, status_y);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.front.status.output.width", &status_width, sizeof(status_width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(prop.screenWidth, status_width);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.front.status.output.height", &status_height, sizeof(status_height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(prop.screenHeight, status_height);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

TEST_P(VhalApiTestVpathFrontOutputNotSetHdmi, NotSet)
{
	VhalApiTestFrontOutputNotSetParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

    struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, param.vpath));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.front.status.output.x", &status_x, sizeof(status_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(0, status_x);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.front.status.output.y", &status_y, sizeof(status_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(0, status_y);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.front.status.output.width", &status_width, sizeof(status_width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(prop.screenWidth, status_width);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.front.status.output.height", &status_height, sizeof(status_height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(prop.screenHeight, status_height);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 */
TEST_P(VhalApiTestVpathFrontOutputRepeat, Repeat)
{
	VhalApiTestFrontOutputParameter param = GetParam();
	bool on{};

	cockpit::hal::video_hal::Init(obj);


	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, param.vpath));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, 10, 10, true));

	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		on = true;		
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.x", &param.x, sizeof(param.x), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.y", &param.y, sizeof(param.y), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.width", &param.width, sizeof(param.width), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.height", &param.height, sizeof(param.height), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	}

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ異常
 */
TEST_P(VhalApiTestVpathFrontOutputAbnormalArg, AbnormalArg)
{
	VhalApiTestFrontOutputParameter param = GetParam();
	bool on{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, param.vpath));
	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));

	/* 映像出力サイズ設定 */
	on = true;	
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.x", &param.x, sizeof(param.x), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.y", &param.y, sizeof(param.y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.width", &param.width, sizeof(param.width), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.height", &param.height, sizeof(param.height), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 前提を満たさない条件でのコール
 * 映像パス＝クリアで出力サイズを設定する
 */
TEST_P(VhalApiTestVpathFrontOutputAbnormalClear, AbnormalClear)
{
	VhalApiTestFrontOutputParameter param = GetParam();
	bool on{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	/* 映像出力サイズ設定 */
	on = true;	
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.x", &param.x, sizeof(param.x), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.y", &param.y, sizeof(param.y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.width", &param.width, sizeof(param.width), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.height", &param.height, sizeof(param.height), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	cockpit::hal::video_hal::Deinit(obj);
}