#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static int32_t status_hdcp;
static int32_t status_hdcp_expect;
static const int timeout_seconds = CB_HDCP_TIMEOUT_SECOUNDS;

static void CallbackHdcp(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "hdcp.first_auth.status.rse.result", &status_hdcp, sizeof(status_hdcp), VIDEO_HAL_DATA_TYPE_NUM));
		PrintMsg("status_hdcp=" + std::to_string(status_hdcp));
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

struct VhalSeqTestHdcpRearParameter {
	int32_t surface_id;
	std::string front_vpath_name;
};

class VhalSeqTestHdcpRearApiSpecification : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestHdcpRearParameter> {};

INSTANTIATE_TEST_CASE_P(VhalSeqTestHdcpRear, VhalSeqTestHdcpRearApiSpecification, ::testing::Values(
	VhalSeqTestHdcpRearParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV}
));

/**
 * API仕様書記載のシーケンス
 */
TEST_P(VhalSeqTestHdcpRearApiSpecification, HdcpAuthentication)
{
	VhalSeqTestHdcpRearParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool start{true};

	cockpit::hal::video_hal::Init(obj);

	SetConnectedRse(obj, VIDEO_HAL_CONNECTED_RSE_FULL);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "hdcp.first_auth.status.rse.result", CallbackHdcp));

	status_hdcp = VIDEO_HAL_HDCP_FIRST_AUTH_STS_NONE;
	status_hdcp_expect = VIDEO_HAL_HDCP_FIRST_AUTH_STS_SUCCESS;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "hdcp.first_auth.control.rse", &start, sizeof(start), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for hdcp.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_hdcp_expect == status_hdcp); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "hdcp.first_auth.status.rse.result", CallbackHdcp));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 */
TEST(VhalSeqTestHdcpRearRepeat, HdcpAuthentication)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool start{true};

	cockpit::hal::video_hal::Init(obj);

	SetConnectedRse(obj, VIDEO_HAL_CONNECTED_RSE_FULL);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "hdcp.first_auth.status.rse.result", CallbackHdcp));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
        status_hdcp = VIDEO_HAL_HDCP_FIRST_AUTH_STS_NONE;
        status_hdcp_expect = VIDEO_HAL_HDCP_FIRST_AUTH_STS_SUCCESS;

        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "hdcp.first_auth.control.rse", &start, sizeof(start), VIDEO_HAL_DATA_TYPE_BOOL));
        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
        PrintMsg("wait_for hdcp.");
        intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_hdcp_expect == status_hdcp); });
        EXPECT_EQ(true, intime);	
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "hdcp.first_auth.status.rse.result", CallbackHdcp));

	cockpit::hal::video_hal::Deinit(obj);
}
