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
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "heacon.front.status.output.x", &status_x, sizeof(status_x), VIDEO_HAL_DATA_TYPE_NUM));
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
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "heacon.front.status.output.y", &status_y, sizeof(status_y), VIDEO_HAL_DATA_TYPE_NUM));
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
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "heacon.front.status.output.width", &status_width, sizeof(status_width), VIDEO_HAL_DATA_TYPE_NUM));
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
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "heacon.front.status.output.height", &status_height, sizeof(status_height), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_height_expect, status_height);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

struct VhalApiTestHeaconFrontOutputParameter {
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

class VhalApiTestHeaconFrontOutputNormal : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestHeaconFrontOutputParameter> {};
class VhalApiTestHeaconFrontOutputRange : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestHeaconFrontOutputParameter> {};
class VhalApiTestHeaconFrontOutputRepeat : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestHeaconFrontOutputParameter> {};
class VhalApiTestHeaconFrontOutputAbnormalArg : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestHeaconFrontOutputParameter> {};

/* BEVstep3 */
std::vector<VhalApiTestHeaconFrontOutputParameter> GenerateVhalApiTestHeaconFrontOutputNormalParameter(void)
{
    struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);

	std::vector<VhalApiTestHeaconFrontOutputParameter> v = {
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 0, (prop.screenHeight - 120), 	prop.screenWidth, 	120}
	};

	return v;
}

#if 1	/* BEVstep3 */
INSTANTIATE_TEST_CASE_P(VhalApiTestHeaconFrontOutput, VhalApiTestHeaconFrontOutputNormal, ::testing::ValuesIn(GenerateVhalApiTestHeaconFrontOutputNormalParameter()));
#else
INSTANTIATE_TEST_CASE_P(VhalApiTestHeaconFrontOutput, VhalApiTestHeaconFrontOutputNormal, ::testing::Values(
	VhalApiTestHeaconFrontOutputParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 0, 600, 1920, 120}
));
#endif

std::vector<VhalApiTestHeaconFrontOutputParameter> GenerateVhalApiTestHeaconFrontOutputParameter(void)
{
    struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);

	std::vector<VhalApiTestHeaconFrontOutputParameter> v = {
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 0, 600, 						prop.screenWidth, 	(prop.screenHeight - 600)},	/* x(0固定) */
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 0, 0, 						prop.screenWidth, 	prop.screenHeight},			/* y下限、h上限 */
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 0, (prop.screenHeight - 1), 	prop.screenWidth, 	1},							/* y上限、h下限 */
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 0, 0, 						0, 					0},							/* w下限 */
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 0, 0, 						prop.screenWidth, 	prop.screenHeight}			/* w上限 */
	};

	return v;
}

INSTANTIATE_TEST_CASE_P(VhalApiTestHeaconFrontOutput, VhalApiTestHeaconFrontOutputRange, ::testing::ValuesIn(GenerateVhalApiTestHeaconFrontOutputParameter()));

#if 1	/* BEVstep3 */
INSTANTIATE_TEST_CASE_P(VhalApiTestHeaconFrontOutput, VhalApiTestHeaconFrontOutputRepeat, ::testing::ValuesIn(GenerateVhalApiTestHeaconFrontOutputNormalParameter()));
#else
INSTANTIATE_TEST_CASE_P(VhalApiTestHeaconFrontOutput, VhalApiTestHeaconFrontOutputRepeat, ::testing::Values(
	VhalApiTestHeaconFrontOutputParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 0, 600, 1920, 120}
));
#endif

std::vector<VhalApiTestHeaconFrontOutputParameter> GenerateVhalApiTestHeaconFrontOutputAbnormalArgParameter(void)
{
    struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);

	std::vector<VhalApiTestHeaconFrontOutputParameter> v = {
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, (0 + 1), 600, 				prop.screenWidth, 		120},						/* x異常 */
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 0, 		prop.screenHeight, 	prop.screenWidth, 		0},							/* y異常 */
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 0, 		600, 				(prop.screenWidth + 1), 120},						/* w異常 */
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 0, 		0, 					prop.screenWidth, 		(prop.screenHeight + 1)}	/* h異常 */
	};

	return v;
}

INSTANTIATE_TEST_CASE_P(VhalApiTestHeaconFrontOutput, VhalApiTestHeaconFrontOutputAbnormalArg, ::testing::ValuesIn(GenerateVhalApiTestHeaconFrontOutputAbnormalArgParameter()));

/**
 * 基本動作
 */
TEST_P(VhalApiTestHeaconFrontOutputNormal, Normal)
{
	VhalApiTestHeaconFrontOutputParameter param = GetParam();
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

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "heacon.front.status.output.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "heacon.front.status.output.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "heacon.front.status.output.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "heacon.front.status.output.height", CallbackHeight));

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

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.x", &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.y", &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for heacon output.");

	/* 設定したプロパティがすべて変更されるまで待つ */
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_x_expect == status_x) &&
																					          (status_y_expect == status_y) &&
																							  (status_width_expect == status_width) &&
																							  (status_height_expect == status_height); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "heacon.front.status.output.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "heacon.front.status.output.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "heacon.front.status.output.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "heacon.front.status.output.height", CallbackHeight));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetHeaconAreaSize(obj, 0, 0, 0, 0, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ上下限
 */
TEST_P(VhalApiTestHeaconFrontOutputRange, Range)
{
	VhalApiTestHeaconFrontOutputParameter param = GetParam();
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

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "heacon.front.status.output.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "heacon.front.status.output.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "heacon.front.status.output.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "heacon.front.status.output.height", CallbackHeight));

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

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.x", &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.y", &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for heacon output.");

	/* 設定したプロパティがすべて変更されるまで待つ */
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_x_expect == status_x) &&
																					          (status_y_expect == status_y) &&
																							  (status_width_expect == status_width) &&
																							  (status_height_expect == status_height); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "heacon.front.status.output.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "heacon.front.status.output.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "heacon.front.status.output.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "heacon.front.status.output.height", CallbackHeight));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetHeaconAreaSize(obj, 0, 0, 0, 0, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 */
TEST(VhalApiTestHeaconFrontOutputNotSet, NotSet)
{
	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "heacon.front.status.output.x", &status_x, sizeof(status_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(0, status_x);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "heacon.front.status.output.y", &status_y, sizeof(status_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(0, status_y);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "heacon.front.status.output.width", &status_width, sizeof(status_width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(0, status_width);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "heacon.front.status.output.height", &status_height, sizeof(status_height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(0, status_height);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 */
TEST_P(VhalApiTestHeaconFrontOutputRepeat, Repeat)
{
	VhalApiTestHeaconFrontOutputParameter param = GetParam();
	bool on{};

	cockpit::hal::video_hal::Init(obj);


	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, param.vpath));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, 10, 10, true));

	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		on = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.x", &param.x, sizeof(param.x), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.y", &param.y, sizeof(param.y), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.width", &param.width, sizeof(param.width), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.height", &param.height, sizeof(param.height), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	}

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetHeaconAreaSize(obj, 0, 0, 0, 0, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ異常
 */
TEST_P(VhalApiTestHeaconFrontOutputAbnormalArg, AbnormalArg)
{
	VhalApiTestHeaconFrontOutputParameter param = GetParam();
	bool on{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, param.vpath));
	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));

	/* 映像出力サイズ設定 */
	on = true;	
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.x", &param.x, sizeof(param.x), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.y", &param.y, sizeof(param.y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.width", &param.width, sizeof(param.width), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.height", &param.height, sizeof(param.height), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetHeaconAreaSize(obj, 0, 0, 0, 0, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}
