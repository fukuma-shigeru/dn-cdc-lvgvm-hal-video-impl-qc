#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static uint32_t status_widemode;
static uint32_t status_widemode_expect;
static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;

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

struct VhalSeqTestWideModeParameter {
	int32_t surface_id;
	std::string front_vpath_name;
};

class VhalSeqTestWideModeApiSpecification : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestWideModeParameter> {};

INSTANTIATE_TEST_CASE_P(VhalSeqTestWideMode, VhalSeqTestWideModeApiSpecification, ::testing::Values(
	VhalSeqTestWideModeParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV}
));

/**
 * API仕様書記載のシーケンス
 * 映像ワイド設定
 */
TEST_P(VhalSeqTestWideModeApiSpecification, SetWideMode)
{
	VhalSeqTestWideModeParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);


	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetWideModeFront(obj, DEFAULT_PROPERTY_WIDEMODE));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.widemode", CallbackWideMode));

	/* 前席映像ワイド設定 */
	status_widemode = VIDEO_HAL_WIDE_MODE_NORMAL;
	status_widemode_expect = VIDEO_HAL_WIDE_MODE_ZOOMED;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.widemode", &status_widemode_expect, sizeof(status_widemode_expect), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for widemode.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_widemode_expect == status_widemode); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.widemode", CallbackWideMode));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetWideModeFront(obj, DEFAULT_PROPERTY_WIDEMODE));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 * 映像ワイド設定
 */
TEST(VhalSeqTestWideModeRepeat, SetWideMode)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);


	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetWideModeFront(obj, DEFAULT_PROPERTY_WIDEMODE));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.widemode", CallbackWideMode));

	/* 前席映像ワイド設定 */
	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		status_widemode = VIDEO_HAL_WIDE_MODE_NORMAL;
		status_widemode_expect = VIDEO_HAL_WIDE_MODE_ZOOMED;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.widemode", &status_widemode_expect, sizeof(status_widemode_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for widemode.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_widemode_expect == status_widemode); });
		EXPECT_EQ(true, intime);

		/* もとに戻す */
		status_widemode = VIDEO_HAL_WIDE_MODE_ZOOMED;
		status_widemode_expect = VIDEO_HAL_WIDE_MODE_NORMAL;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.widemode", &status_widemode_expect, sizeof(status_widemode_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for widemode.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_widemode_expect == status_widemode); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.widemode", CallbackWideMode));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetWideModeFront(obj, DEFAULT_PROPERTY_WIDEMODE));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}