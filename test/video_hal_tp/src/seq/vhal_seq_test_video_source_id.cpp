#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static uint32_t status_video_source_id;
static uint32_t status_video_source_id_expect;
static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;

static void CallbackVideoSourceId(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.id", &status_video_source_id, sizeof(status_video_source_id), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_video_source_id_expect, status_video_source_id);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

struct VhalSeqTestVideoSourceIdParameter {
	int32_t surface_id;
	std::string front_vpath_name;
};

class VhalSeqTestVideoSourceIdApiSpecification : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestVideoSourceIdParameter> {};

INSTANTIATE_TEST_CASE_P(VhalSeqTestVideoSourceId, VhalSeqTestVideoSourceIdApiSpecification, ::testing::Values(
	VhalSeqTestVideoSourceIdParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV}
));

/**
 * API仕様書記載のシーケンス
 * 映像ソース設定
 */
TEST_P(VhalSeqTestVideoSourceIdApiSpecification, SetVideoSourceId)
{
	VhalSeqTestVideoSourceIdParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, DEFAULT_PROPERTY_VSRC_ID));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.id", CallbackVideoSourceId));

	/* 前席映像ソース設定 */
	status_video_source_id = VIDEO_HAL_VSRC_ID_OTHER;
	status_video_source_id_expect = VIDEO_HAL_VSRC_ID_HDMI;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.id", &status_video_source_id_expect, sizeof(status_video_source_id_expect), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for video_source_id.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_video_source_id_expect == status_video_source_id); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.id", CallbackVideoSourceId));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, DEFAULT_PROPERTY_VSRC_ID));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * API仕様書記載のシーケンス
 * 映像ソース設定(エラー)
 */
TEST(VhalSeqTestVideoSourceIdApiSpecification, SetVideoSourceIdParamErr)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, DEFAULT_PROPERTY_VSRC_ID));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.id", CallbackVideoSourceId));

	/* 前席映像ソース設定 */
	status_video_source_id = VIDEO_HAL_VSRC_ID_OTHER;
	status_video_source_id_expect = 0x11;	// ERROR Value

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.id", &status_video_source_id_expect, sizeof(status_video_source_id_expect), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.id", CallbackVideoSourceId));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, DEFAULT_PROPERTY_VSRC_ID));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 * 映像ソース設定
 */
TEST(VhalSeqTestVideoSourceIdRepeat, Repeat)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, DEFAULT_PROPERTY_VSRC_ID));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.id", CallbackVideoSourceId));

	/* 前席映像ソース設定 */
	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		status_video_source_id = VIDEO_HAL_VSRC_ID_OTHER;
		status_video_source_id_expect = VIDEO_HAL_VSRC_ID_HDMI;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.id", &status_video_source_id_expect, sizeof(status_video_source_id_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for video_source_id.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_video_source_id_expect == status_video_source_id); });
		EXPECT_EQ(true, intime);

		/* もとに戻す */
		status_video_source_id = VIDEO_HAL_VSRC_ID_HDMI;
		status_video_source_id_expect = VIDEO_HAL_VSRC_ID_OTHER;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.id", &status_video_source_id_expect, sizeof(status_video_source_id_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for video_source_id.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_video_source_id_expect == status_video_source_id); });
		EXPECT_EQ(true, intime);
	}


	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.id", CallbackVideoSourceId));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, DEFAULT_PROPERTY_VSRC_ID));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}