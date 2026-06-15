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
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.front.status.clipping.x", &status_x, sizeof(status_x), VIDEO_HAL_DATA_TYPE_NUM));
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
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.front.status.clipping.y", &status_y, sizeof(status_y), VIDEO_HAL_DATA_TYPE_NUM));
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
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.front.status.clipping.width", &status_width, sizeof(status_width), VIDEO_HAL_DATA_TYPE_NUM));
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

struct VhalSeqTestVpathFrontClippingParameter {
	int32_t surface_id;
	std::string front_vpath_name;
};

class VhalSeqTestVpathFrontClippingApiSpecification : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestVpathFrontClippingParameter> {};

INSTANTIATE_TEST_CASE_P(VhalSeqTestVpathFrontClipping, VhalSeqTestVpathFrontClippingApiSpecification, ::testing::Values(
	VhalSeqTestVpathFrontClippingParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV}
));

/**
 * API仕様書記載のシーケンス
 * クリッピングサイズ変更
 */
TEST_P(VhalSeqTestVpathFrontClippingApiSpecification, SetFrontClipping)
{
	VhalSeqTestVpathFrontClippingParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool on{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontClipping(obj, DEFAULT_PROPERTY_CLIPPING_X, DEFAULT_PROPERTY_CLIPPING_Y, DEFAULT_PROPERTY_CLIPPING_WIDTH, DEFAULT_PROPERTY_CLIPPING_HEIGHT, DEFAULT_PROPERTY_CLIPPING_ENABLE, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.clipping.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.clipping.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.clipping.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.clipping.height", CallbackHeight));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.clipping.enable", CallbackEnable));

	/* 前席映像クリッピングサイズ設定 */
	status_x = 0;
	status_y = 0;
	status_width = 0;
	status_height = 0;
	status_enable = false;
	status_x_expect = 10;
	status_y_expect = 10;
	status_width_expect = 800;
	status_height_expect = 600;
	status_enable_expect = true;
	on = true;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.x", &status_x_expect, sizeof(status_x_expect), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.y", &status_y_expect, sizeof(status_y_expect), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.width", &status_width_expect, sizeof(status_width_expect), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.height", &status_height_expect, sizeof(status_height_expect), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.enable", &status_enable_expect, sizeof(status_enable_expect), VIDEO_HAL_DATA_TYPE_BOOL));
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
 * API仕様書記載のシーケンス
 * クリッピングサイズ変更(前席パス="")
 * enableプロパティ変更false -> true
 */
TEST(VhalSeqTestVpathFrontClippingApiSpecification, SetFrontClippingNone)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool on{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.clipping.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.clipping.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.clipping.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.clipping.height", CallbackHeight));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.clipping.enable", CallbackEnable));

	/* 前席映像クリッピングサイズ設定 */
	status_x = 0;
	status_y = 0;
	status_width = 0;
	status_height = 0;
	status_enable = false;
	status_x_expect = 20;
	status_y_expect = 20;
	status_width_expect = 400;
	status_height_expect = 400;
	status_enable_expect = true;
	on = true;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.x", &status_x_expect, sizeof(status_x_expect), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.y", &status_y_expect, sizeof(status_y_expect), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.width", &status_width_expect, sizeof(status_width_expect), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.height", &status_height_expect, sizeof(status_height_expect), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.enable", &status_enable_expect, sizeof(status_enable_expect), VIDEO_HAL_DATA_TYPE_BOOL));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
	// ここで失敗
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.clipping.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.clipping.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.clipping.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.clipping.height", CallbackHeight));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.clipping.enable", CallbackEnable));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 * クリッピングサイズ変更
 */
TEST(VhalSeqTestVpathFrontClippingRepeat, SetFrontClipping)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool on{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontClipping(obj, DEFAULT_PROPERTY_CLIPPING_X, DEFAULT_PROPERTY_CLIPPING_Y, DEFAULT_PROPERTY_CLIPPING_WIDTH, DEFAULT_PROPERTY_CLIPPING_HEIGHT, DEFAULT_PROPERTY_CLIPPING_ENABLE, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.clipping.x", CallbackX));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.clipping.y", CallbackY));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.clipping.width", CallbackWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.clipping.height", CallbackHeight));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.clipping.enable", CallbackEnable));

	/* 前席映像クリッピングサイズ設定 */
	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		status_x = 0;
		status_y = 0;
		status_width = 1;
		status_height = 1;
		status_enable = false;
		status_x_expect = 10;
		status_y_expect = 10;
		status_width_expect = 800;
		status_height_expect = 600;
		status_enable_expect = true;
		on = true;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.x", &status_x_expect, sizeof(status_x_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.y", &status_y_expect, sizeof(status_y_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.width", &status_width_expect, sizeof(status_width_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.height", &status_height_expect, sizeof(status_height_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.enable", &status_enable_expect, sizeof(status_enable_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for clipping.");

		/* 設定したプロパティがすべて変更されるまで待つ */
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_x_expect == status_x) &&
																								(status_y_expect == status_y) &&
																								(status_width_expect == status_width) &&
																								(status_height_expect == status_height);
																								});
		EXPECT_EQ(true, intime);

		/* もとに戻す */
		status_x = 10;
		status_y = 10;
		status_width = 800;
		status_height = 600;
		status_enable = false;
		status_x_expect = 0;
		status_y_expect = 0;
		status_width_expect = 1;
		status_height_expect = 1;
		status_enable_expect = true;
		on = true;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.x", &status_x_expect, sizeof(status_x_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.y", &status_y_expect, sizeof(status_y_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.width", &status_width_expect, sizeof(status_width_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.height", &status_height_expect, sizeof(status_height_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.enable", &status_enable_expect, sizeof(status_enable_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for clipping.");

		/* 設定したプロパティがすべて変更されるまで待つ */
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_x_expect == status_x) &&
																								(status_y_expect == status_y) &&
																								(status_width_expect == status_width) &&
																								(status_height_expect == status_height);
																								});
		EXPECT_EQ(true, intime);
	}

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