#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static uint32_t status_video_source_id;
static uint32_t status_video_source_id_expect;
static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;

#define ABNORMAL_ARG_VSRC_ID (VIDEO_HAL_VSRC_ID_DRC + 1)

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

struct VhalApiTestVideoSourceParameter {
    uint32_t video_source_id;
};

class VhalApiTestVideoSourceIdNormal : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVideoSourceParameter> {};
class VhalApiTestVideoSourceIdRepeat : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVideoSourceParameter> {};
class VhalApiTestVideoSourceIdAbnormalArg : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVideoSourceParameter> {};

INSTANTIATE_TEST_CASE_P(VhalApiTestVideoSourceId, VhalApiTestVideoSourceIdNormal, ::testing::Values(
    VhalApiTestVideoSourceParameter{VIDEO_HAL_VSRC_ID_DTV},
    VhalApiTestVideoSourceParameter{VIDEO_HAL_VSRC_ID_OTHER},
    VhalApiTestVideoSourceParameter{VIDEO_HAL_VSRC_ID_IMAGE_ADJUST_CAM},
    VhalApiTestVideoSourceParameter{VIDEO_HAL_VSRC_ID_HDMI}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVideoSourceId, VhalApiTestVideoSourceIdRepeat, ::testing::Values(
    VhalApiTestVideoSourceParameter{VIDEO_HAL_VSRC_ID_DTV}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVideoSourceId, VhalApiTestVideoSourceIdAbnormalArg, ::testing::Values(

    VhalApiTestVideoSourceParameter{ABNORMAL_ARG_VSRC_ID},
    VhalApiTestVideoSourceParameter{VIDEO_HAL_VSRC_ID_BUILTIN_BACK_CAM},
    VhalApiTestVideoSourceParameter{VIDEO_HAL_VSRC_ID_OTV},
    VhalApiTestVideoSourceParameter{VIDEO_HAL_VSRC_ID_BLURAY},
    VhalApiTestVideoSourceParameter{VIDEO_HAL_VSRC_ID_EXTERNAL_BACK_CAM_GVIF},
    VhalApiTestVideoSourceParameter{VIDEO_HAL_VSRC_ID_EXTERNAL_BACK_CAM_NTSC},
    VhalApiTestVideoSourceParameter{VIDEO_HAL_VSRC_ID_CAM_DIAG_GVIF_BGM},
    VhalApiTestVideoSourceParameter{VIDEO_HAL_VSRC_ID_CAM_DIAG_GVIF_PVM},
    VhalApiTestVideoSourceParameter{VIDEO_HAL_VSRC_ID_CAM_DIAG_NTSC},
    VhalApiTestVideoSourceParameter{VIDEO_HAL_VSRC_ID_DRC}
));

/**
 * 基本動作
 * 映像ソース設定
 */
TEST_P(VhalApiTestVideoSourceIdNormal, Normal)
{
	VhalApiTestVideoSourceParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, DEFAULT_PROPERTY_VSRC_ID));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.id", CallbackVideoSourceId));

	/* 前席映像ソース設定 */
	status_video_source_id = DEFAULT_PROPERTY_VSRC_ID;
	status_video_source_id_expect = param.video_source_id;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.id", &param.video_source_id, sizeof(param.video_source_id), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for video_source_id.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_video_source_id_expect == status_video_source_id); });
	EXPECT_EQ(true, intime);

    uint32_t control_video_source_id{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.control.id", &control_video_source_id, sizeof(control_video_source_id), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(param.video_source_id, control_video_source_id);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.id", CallbackVideoSourceId));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, DEFAULT_PROPERTY_VSRC_ID));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 */
TEST(VhalApiTestVideoSourceIdNotSet, NotSet)
{
	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.id", &status_video_source_id, sizeof(status_video_source_id), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_VSRC_ID_OTHER, status_video_source_id);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 */
TEST_P(VhalApiTestVideoSourceIdRepeat, Repeat)
{
	VhalApiTestVideoSourceParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);


	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, DEFAULT_PROPERTY_VSRC_ID));

	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.id", &param.video_source_id, sizeof(param.video_source_id), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	}

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, DEFAULT_PROPERTY_VSRC_ID));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ異常 
 */
TEST_P(VhalApiTestVideoSourceIdAbnormalArg, AbnormalArg)
{
	VhalApiTestVideoSourceParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);


	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, DEFAULT_PROPERTY_VSRC_ID));

	/* 前席映像ソース設定 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.id", &param.video_source_id, sizeof(param.video_source_id), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, DEFAULT_PROPERTY_VSRC_ID));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}
