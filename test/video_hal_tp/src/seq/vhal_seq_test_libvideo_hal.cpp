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

struct VhalSeqTestLibVideoHalParameter {
    int32_t dummy;
};

class VhalSeqTestLibVideoHalApiSpecification : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestLibVideoHalParameter> {};

INSTANTIATE_TEST_CASE_P(VhalSeqTestLibVideoHal, VhalSeqTestLibVideoHalApiSpecification, ::testing::Values(
	VhalSeqTestLibVideoHalParameter{0}
));

/**
 * API仕様書記載のシーケンス
 */
TEST_P(VhalSeqTestLibVideoHalApiSpecification, TypicalCase)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	uint32_t video_source_id{};

    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));

    SetVideoSourceId(obj, VIDEO_HAL_VSRC_ID_OTHER);	// BEVstep3 事前準備

    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.id", CallbackVideoSourceId));

	/* 前席映像ソース設定 */
	status_video_source_id = VIDEO_HAL_VSRC_ID_OTHER;
	status_video_source_id_expect = VIDEO_HAL_VSRC_ID_HDMI;
	video_source_id = status_video_source_id_expect;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.id", &video_source_id, sizeof(video_source_id), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for video_source_id.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_video_source_id_expect == status_video_source_id); });
	EXPECT_EQ(true, intime);

    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.id", CallbackVideoSourceId));

    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * 連続動作
 */
TEST(VhalSeqTestLibVideoHalRepeat, TypicalCase)
{
    std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	uint32_t video_source_id{};
    
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));

    SetVideoSourceId(obj, VIDEO_HAL_VSRC_ID_OTHER);
    
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));

    for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
    {   
        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));

        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.id", CallbackVideoSourceId));

        /* 前席映像ソース設定 */
        status_video_source_id = VIDEO_HAL_VSRC_ID_OTHER;
        status_video_source_id_expect = VIDEO_HAL_VSRC_ID_HDMI;
        video_source_id = status_video_source_id_expect;

        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.id", &video_source_id, sizeof(video_source_id), VIDEO_HAL_DATA_TYPE_NUM));
        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
        PrintMsg("wait_for video_source_id.");
        intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_video_source_id_expect == status_video_source_id); });
        EXPECT_EQ(true, intime);

        /* もとに戻す */
        status_video_source_id = VIDEO_HAL_VSRC_ID_HDMI;
        status_video_source_id_expect = VIDEO_HAL_VSRC_ID_OTHER;
        video_source_id = status_video_source_id_expect;

        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.id", &video_source_id, sizeof(video_source_id), VIDEO_HAL_DATA_TYPE_NUM));
        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
        PrintMsg("wait_for video_source_id.");
        intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_video_source_id_expect == status_video_source_id); });
        EXPECT_EQ(true, intime);

        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.id", CallbackVideoSourceId));

        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
    }
}

/**
 * 相手が無応答
 * VideoHALServerの応答がない場合の動作を確認する。
 */
TEST(VhalSeqTestLibVideoHalNoReply, NoReply)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	uint32_t video_source_id{};

    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));
    
    /* VideoHALServer停止 */
    SigStop(PNAME_VIDEO_HAL_SERVER);

    EXPECT_EQ(VIDEO_HAL_API_ERR_TIMEOUT, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.id", CallbackVideoSourceId));

    /* VideoHALServer再開 */
    SigCont(PNAME_VIDEO_HAL_SERVER);

	/* 前席映像ソース設定 */
	status_video_source_id = VIDEO_HAL_VSRC_ID_OTHER;
	status_video_source_id_expect = VIDEO_HAL_VSRC_ID_HDMI;
	video_source_id = status_video_source_id_expect;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.id", &video_source_id, sizeof(video_source_id), VIDEO_HAL_DATA_TYPE_NUM));

    SigStop(PNAME_VIDEO_HAL_SERVER);

	EXPECT_EQ(VIDEO_HAL_API_ERR_TIMEOUT, cockpit::hal::video_hal::RequestUpdate(obj));
    
    SigCont(PNAME_VIDEO_HAL_SERVER);

    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.id", CallbackVideoSourceId));

    SigStop(PNAME_VIDEO_HAL_SERVER);

    EXPECT_EQ(VIDEO_HAL_API_ERR_TIMEOUT, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.id", CallbackVideoSourceId));

    SigCont(PNAME_VIDEO_HAL_SERVER);

    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}