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
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.camera.status.output.x", &status_x, sizeof(status_x), VIDEO_HAL_DATA_TYPE_NUM));
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
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.camera.status.output.y", &status_y, sizeof(status_y), VIDEO_HAL_DATA_TYPE_NUM));
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
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.camera.status.output.width", &status_width, sizeof(status_width), VIDEO_HAL_DATA_TYPE_NUM));
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
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.camera.status.output.height", &status_height, sizeof(status_height), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_height_expect, status_height);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

struct VhalApiTestCameraOutputParameter {
	std::string vpath;
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
};

struct VhalApiTestCameraOutputNotSetParameter {
	std::string vpath;
};

class VhalApiTestVpathCameraOutputNormal : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestCameraOutputParameter> {};
class VhalApiTestVpathCameraOutputRange : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestCameraOutputParameter> {};
class VhalApiTestVpathCameraOutputNotSet : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestCameraOutputNotSetParameter> {};
class VhalApiTestVpathCameraOutputRepeat : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestCameraOutputParameter> {};
class VhalApiTestVpathCameraOutputAbnormalArg : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestCameraOutputParameter> {};
class VhalApiTestVpathCameraOutputAbnormalClear : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestCameraOutputParameter> {};

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathCameraOutput, VhalApiTestVpathCameraOutputNormal, ::testing::Values(
	VhalApiTestCameraOutputParameter{VPATH_NAME_CAMERA, 10, 10, 800, 600}
));

std::vector<VhalApiTestCameraOutputParameter> GenerateVhalApiTestCameraOutputParameter(void)
{
    struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);

	std::vector<VhalApiTestCameraOutputParameter> v = {
		{VPATH_NAME_CAMERA, 0, 100, 100, 100},
		{VPATH_NAME_CAMERA, prop.screenWidth - 1, 100, 1, 100},
		{VPATH_NAME_CAMERA, 100, 0, 100, 100},
		{VPATH_NAME_CAMERA, 100, prop.screenHeight - 1, 100, 1},
		{VPATH_NAME_CAMERA, 100, 100, 1, 100},
		{VPATH_NAME_CAMERA, 0, 100, prop.screenWidth, 100},
		{VPATH_NAME_CAMERA, 100, 100, 100, 1},
		{VPATH_NAME_CAMERA, 100, 0, 100, prop.screenHeight}
	};

	return v;
}

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathCameraOutput, VhalApiTestVpathCameraOutputRange, ::testing::ValuesIn(GenerateVhalApiTestCameraOutputParameter()));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathCameraOutput, VhalApiTestVpathCameraOutputNotSet, ::testing::Values(
	VhalApiTestCameraOutputNotSetParameter{VPATH_NAME_CAMERA}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathCameraOutput, VhalApiTestVpathCameraOutputRepeat, ::testing::Values(
	VhalApiTestCameraOutputParameter{VPATH_NAME_CAMERA, 10, 10, 800, 600}
));

std::vector<VhalApiTestCameraOutputParameter> GenerateVhalApiTestCameraOutputAbnormalArgParameter(void)
{
    struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);

	std::vector<VhalApiTestCameraOutputParameter> v = {
		{VPATH_NAME_CAMERA, prop.screenWidth, 100, 100, 100},
		{VPATH_NAME_CAMERA, 100, prop.screenHeight, 100, 100},
		{VPATH_NAME_CAMERA, 0, 100, prop.screenWidth + 1, 100},
		{VPATH_NAME_CAMERA, 100, 0, 100, prop.screenHeight + 1}
	};

	return v;
}

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathCameraOutput, VhalApiTestVpathCameraOutputAbnormalArg, ::testing::ValuesIn(GenerateVhalApiTestCameraOutputAbnormalArgParameter()));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathCameraOutput, VhalApiTestVpathCameraOutputAbnormalClear, ::testing::Values(
	VhalApiTestCameraOutputParameter{VPATH_NAME_CLEAR, 10, 10, 800, 600}
));

/**
 * 基本動作
 */
TEST_P(VhalApiTestVpathCameraOutputNormal, Normal)
{
	VhalApiTestCameraOutputParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	uint32_t x{};
	uint32_t y{};
	uint32_t width{};
	uint32_t height{};
	bool on{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, param.vpath));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetCameraOutput(obj, 2, 2, 2, 2, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.output.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.output.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.output.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.output.height", CallbackHeight));

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

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.x", &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.y", &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
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
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.camera.control.output.x", &control_x, sizeof(control_x), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(x, control_x);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.camera.control.output.y", &control_y, sizeof(control_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(y, control_y);
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.camera.control.output.width", &control_width, sizeof(control_width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(width, control_width);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.camera.control.output.height", &control_height, sizeof(control_height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(height, control_height);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.camera.control.output.on", &control_on, sizeof(control_on), VIDEO_HAL_DATA_TYPE_BOOL));
    
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.output.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.output.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.output.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.output.height", CallbackHeight));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	cockpit::hal::video_hal::Deinit(obj);
}

TEST_P(VhalApiTestVpathCameraOutputRange, Range)
{
	VhalApiTestCameraOutputParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	uint32_t x{};
	uint32_t y{};
	uint32_t width{};
	uint32_t height{};
	bool on{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, param.vpath));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetCameraOutput(obj, 2, 2, 2, 2, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.output.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.output.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.output.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.output.height", CallbackHeight));

	/* 映像出力サイズ設定 */
	status_x = 50;
	status_y = 50;
	status_width = 50;
	status_height = 50;
	status_x_expect = param.x;
	status_y_expect = param.y;
	status_width_expect = param.width;
	status_height_expect = param.height;
	x = status_x_expect;
	y = status_y_expect;
	width = status_width_expect;
	height = status_height_expect;
	on = true;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.x", &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.y", &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
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
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.camera.control.output.x", &control_x, sizeof(control_x), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(x, control_x);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.camera.control.output.y", &control_y, sizeof(control_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(y, control_y);
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.camera.control.output.width", &control_width, sizeof(control_width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(width, control_width);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.camera.control.output.height", &control_height, sizeof(control_height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(height, control_height);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.camera.control.output.on", &control_on, sizeof(control_on), VIDEO_HAL_DATA_TYPE_BOOL));
    
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.output.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.output.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.output.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.output.height", CallbackHeight));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 */
TEST_P(VhalApiTestVpathCameraOutputNotSet, NotSet)
{
	VhalApiTestCameraOutputNotSetParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

    struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, param.vpath));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.camera.status.output.x", &status_x, sizeof(status_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(0, status_x);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.camera.status.output.y", &status_y, sizeof(status_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(0, status_y);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.camera.status.output.width", &status_width, sizeof(status_width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(prop.screenWidth, status_width);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.camera.status.output.height", &status_height, sizeof(status_height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(prop.screenHeight, status_height);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 */
TEST_P(VhalApiTestVpathCameraOutputRepeat, Repeat)
{
	VhalApiTestCameraOutputParameter param = GetParam();
	bool on{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, param.vpath));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetCameraOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, 1, 1, true));

	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		on = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.x", &param.x, sizeof(param.x), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.y", &param.y, sizeof(param.y), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.width", &param.width, sizeof(param.width), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.height", &param.height, sizeof(param.height), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	}

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ異常
 */
TEST_P(VhalApiTestVpathCameraOutputAbnormalArg, AbnormalArg)
{
	VhalApiTestCameraOutputParameter param = GetParam();
	bool on{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, param.vpath));
	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetCameraOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));

	/* 映像出力サイズ設定 */
	on = true;	
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.x", &param.x, sizeof(param.x), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.y", &param.y, sizeof(param.y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.width", &param.width, sizeof(param.width), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.height", &param.height, sizeof(param.height), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 前提を満たさない条件でのコール
 * 映像パス＝クリアで出力サイズ設定
 */
TEST_P(VhalApiTestVpathCameraOutputAbnormalClear, AbnormalClear)
{
	VhalApiTestCameraOutputParameter param = GetParam();
	bool on{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, param.vpath));

	/* 映像出力サイズ設定 */
	on = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.x", &param.x, sizeof(param.x), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.y", &param.y, sizeof(param.y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.width", &param.width, sizeof(param.width), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.height", &param.height, sizeof(param.height), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	cockpit::hal::video_hal::Deinit(obj);
}