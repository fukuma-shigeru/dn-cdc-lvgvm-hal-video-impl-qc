#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static uint32_t status_opacity;
static uint32_t status_opacity_expect;
static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;

static void CallbackOpacity(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.opacity", &status_opacity, sizeof(status_opacity), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_opacity_expect, status_opacity);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

struct VhalSeqTestOpacityParameter {
	int32_t surface_id;
	std::string front_vpath_name;
};

class VhalSeqTestOpacityApiSpecification : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestOpacityParameter> {};

INSTANTIATE_TEST_CASE_P(VhalSeqTestOpacity, VhalSeqTestOpacityApiSpecification, ::testing::Values(
	VhalSeqTestOpacityParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV}
));

/**
 * API仕様書記載のシーケンス
 * 不透明度設定 
 */
TEST_P(VhalSeqTestOpacityApiSpecification, SetOpacity)
{
	VhalSeqTestOpacityParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);


	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityFront(obj, DEFAULT_PROPERTY_OPACITY));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.opacity", CallbackOpacity));

	/* 不透明度設定 */
	status_opacity = 0;
	status_opacity_expect = 50;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.opacity", &status_opacity_expect, sizeof(status_opacity_expect), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for opacity.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_opacity_expect == status_opacity); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.opacity", CallbackOpacity));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityFront(obj, DEFAULT_PROPERTY_OPACITY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 * 不透明度設定
 */
TEST(VhalSeqTestOpacityRepeat, SetOpacity)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);


	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityFront(obj, DEFAULT_PROPERTY_OPACITY));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.opacity", CallbackOpacity));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		status_opacity = 0;
		status_opacity_expect = 50;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.opacity", &status_opacity_expect, sizeof(status_opacity_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for opacity.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_opacity_expect == status_opacity); });
		EXPECT_EQ(true, intime);

		/* もとに戻す */
		status_opacity = 50;
		status_opacity_expect = 0;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.opacity", &status_opacity_expect, sizeof(status_opacity_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for opacity.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_opacity_expect == status_opacity); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.opacity", CallbackOpacity));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityFront(obj, DEFAULT_PROPERTY_OPACITY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}