#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static bool status_visible;
static bool status_visible_expect;
static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;

static void CallbackVisible(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.visible", &status_visible, sizeof(status_visible), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(status_visible_expect, status_visible);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

struct VhalSeqTestVisibleParameter {
	int32_t surface_id;
	std::string front_vpath_name;
};

class VhalSeqTestVisibleApiSpecification : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestVisibleParameter> {};

INSTANTIATE_TEST_CASE_P(VhalSeqTestVisible, VhalSeqTestVisibleApiSpecification, ::testing::Values(
	VhalSeqTestVisibleParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV}
));

/**
 * API仕様書記載のシーケンス
 * 可視性設定
 */
TEST_P(VhalSeqTestVisibleApiSpecification, SetVisible)
{
	VhalSeqTestVisibleParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.visible", CallbackVisible));

	/* 前席可視性設定 */
	status_visible = true;
	status_visible_expect = false;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.visible", CallbackVisible));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 * 可視性設定
 */
TEST(VhalSeqTestVisibleRepeat, Repeat)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.visible", CallbackVisible));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		status_visible = true;
		status_visible_expect = false;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for visible.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* もとに戻す */
		status_visible = false;
		status_visible_expect = true;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for visible.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.visible", CallbackVisible));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}
