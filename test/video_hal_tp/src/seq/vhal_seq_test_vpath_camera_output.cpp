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

struct VhalSeqTestVpathCameraOutputParameter {
	std::string camera_vpath_name;
};

class VhalSeqTestVpathCameraOutputApiSpecification : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestVpathCameraOutputParameter> {};

INSTANTIATE_TEST_CASE_P(VhalSeqTestVpathCameraOutput, VhalSeqTestVpathCameraOutputApiSpecification, ::testing::Values(
	VhalSeqTestVpathCameraOutputParameter{VPATH_NAME_CAMERA}
));

/**
 * API仕様書記載のシーケンス
 * 映像位置、サイズ変更
 */
TEST_P(VhalSeqTestVpathCameraOutputApiSpecification, SetCameraOutput)
{
	VhalSeqTestVpathCameraOutputParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool on{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CAMERA));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetCameraOutput(obj, 1, 1, 1, 1, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.output.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.output.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.output.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.output.height", CallbackHeight));

	/* 映像出力サイズ設定 */
	status_x = 1;
	status_y = 1;
	status_width = 1;
	status_height = 1;
	status_x_expect = 0;
	status_y_expect = 0;
	status_width_expect = 1280;
	status_height_expect = 720;
	on = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.x", &status_x_expect, sizeof(status_x_expect), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.y", &status_y_expect, sizeof(status_y_expect), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.width", &status_width_expect, sizeof(status_width_expect), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.height", &status_height_expect, sizeof(status_height_expect), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for output.");

	/* 設定したプロパティがすべて変更されるまで待つ */
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_x_expect == status_x) &&
																					          (status_y_expect == status_y) &&
																							  (status_width_expect == status_width) &&
																							  (status_height_expect == status_height); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.output.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.output.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.output.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.output.height", CallbackHeight));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作 
 * 映像位置、サイズ変更
 */
TEST(VhalSeqTestVpathCameraOutputRepeat, SetCameraOutput)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool on{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CAMERA));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetCameraOutput(obj, 1, 1, 1, 1, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.output.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.output.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.output.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.output.height", CallbackHeight));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		status_x = 1;
		status_y = 1;
		status_width = 1;
		status_height = 1;
		status_x_expect = 0;
		status_y_expect = 0;
        status_width_expect = 1280;
        status_height_expect = 720;
		on = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.x", &status_x_expect, sizeof(status_x_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.y", &status_y_expect, sizeof(status_y_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.width", &status_width_expect, sizeof(status_width_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.height", &status_height_expect, sizeof(status_height_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for output.");

		/* 設定したプロパティがすべて変更されるまで待つ */
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_x_expect == status_x) &&
																								(status_y_expect == status_y) &&
																								(status_width_expect == status_width) &&
																								(status_height_expect == status_height); });
		EXPECT_EQ(true, intime);

		/* 元に戻す */
		status_x = 0;
		status_y = 0;
		status_width = 1280;
		status_height = 720;
		status_x_expect = 1;
		status_y_expect = 1;
		status_width_expect = 1;
		status_height_expect = 1;
		on = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.x", &status_x_expect, sizeof(status_x_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.y", &status_y_expect, sizeof(status_y_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.width", &status_width_expect, sizeof(status_width_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.height", &status_height_expect, sizeof(status_height_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for output.");

		/* 設定したプロパティがすべて変更されるまで待つ */
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_x_expect == status_x) &&
																								(status_y_expect == status_y) &&
																								(status_width_expect == status_width) &&
																								(status_height_expect == status_height); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.output.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.output.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.output.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.output.height", CallbackHeight));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	cockpit::hal::video_hal::Deinit(obj);
}
