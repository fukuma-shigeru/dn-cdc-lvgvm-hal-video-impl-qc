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

struct VhalApiTestHdcpRearParameter {
    uint32_t hdcp;
};

struct VhalApiTestHdcpRearCallDoubleParameter {
	uint32_t dummy;
};

struct VhalApiTestHdcpRearEventConflictParameter {
	InternalEventParameter internal_event_param;
};

class VhalApiTestHdcpRearRepeat : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestHdcpRearParameter> {};
class VhalApiTestHdcpRearCallDouble : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestHdcpRearCallDoubleParameter> {};
class VhalApiTestHdcpRearSwEventConflict : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestHdcpRearEventConflictParameter> {};
class VhalApiTestHdcpRearHwEventConflict : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestHdcpRearEventConflictParameter> {};

INSTANTIATE_TEST_CASE_P(VhalApiTestHdcpRear, VhalApiTestHdcpRearRepeat, ::testing::Values(
	VhalApiTestHdcpRearParameter{0}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestHdcpRear, VhalApiTestHdcpRearCallDouble, ::testing::Values(
	VhalApiTestHdcpRearCallDoubleParameter{0}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestHdcpRear, VhalApiTestHdcpRearSwEventConflict, ::testing::Values(
	VhalApiTestHdcpRearEventConflictParameter{.internal_event_param{.type = INTERNAL_EVENT_CREATE_SURFACE, .u{.create_surface{.surface_id = SURFACE_ID_ANDROIDAUTO}}}},
	VhalApiTestHdcpRearEventConflictParameter{.internal_event_param{.type = INTERNAL_EVENT_DELETE_SURFACE, .u{.delete_surface{.surface_id = SURFACE_ID_ANDROIDAUTO}}}},
	VhalApiTestHdcpRearEventConflictParameter{.internal_event_param{.type = INTERNAL_EVENT_NOTIFY_SURFACE_SIZE, .u{.notify_surface_size{.surface_id = SURFACE_ID_ANDROIDAUTO}}}}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestHdcpRear, VhalApiTestHdcpRearHwEventConflict, ::testing::Values(
 	VhalApiTestHdcpRearEventConflictParameter{.internal_event_param{.type = INTERNAL_EVENT_DETECT_CAMERA_SYNC,       .u{.detect_camera_sync{.cam_sync = CAM_SYNC}}}},
 	VhalApiTestHdcpRearEventConflictParameter{.internal_event_param{.type = INTERNAL_EVENT_NOTIFY_CAMERA_TYPE,       .u{.notify_camera_type{.cam_size = CAM_SIZE_1920_1080}}}},
 	VhalApiTestHdcpRearEventConflictParameter{.internal_event_param{.type = INTERNAL_EVENT_CHANGE_HDMI_VIDEO_FORMAT, .u{.change_hdmi_video_format{.format = SYSINFO_CONN_INFO_VIDEO_FORMAT_1280X720P}}}},
 	VhalApiTestHdcpRearEventConflictParameter{.internal_event_param{.type = INTERNAL_EVENT_DETECT_CONNECTED_HDMI,    .u{.detect_connected_hdmi{.conn = DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECT}}}}
));

/**
 * 設定前の参照
 */
TEST(VhalApiTestHdcpRearNotSet, NotSet)
{
    int32_t result{};
    int32_t device_count{};
    uint64_t receiver_id{};

	cockpit::hal::video_hal::Init(obj);

	SetConnectedRse(obj, VIDEO_HAL_CONNECTED_RSE_FULL);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "hdcp.first_auth.status.rse.result", &result, sizeof(result), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_HDCP_FIRST_AUTH_STS_NONE, result);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "hdcp.first_auth.status.rse.deviceCount", &device_count, sizeof(device_count), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(0, device_count);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "hdcp.first_auth.status.rse.receiverID0", &receiver_id, sizeof(receiver_id), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(0, receiver_id);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "hdcp.first_auth.status.rse.receiverID1", &receiver_id, sizeof(receiver_id), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(0, receiver_id) ;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "hdcp.first_auth.status.rse.receiverID2", &receiver_id, sizeof(receiver_id), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(0, receiver_id);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 */
TEST_P(VhalApiTestHdcpRearRepeat, Repeat)
{
	VhalApiTestHdcpRearParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
    bool start{true};

	cockpit::hal::video_hal::Init(obj);

	SetConnectedRse(obj, VIDEO_HAL_CONNECTED_RSE_FULL);

	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
        start = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "hdcp.first_auth.control.rse", &start, sizeof(start), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	}

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 状態/イベントマトリクス網羅
 * HDCP first認証されていない、もしくは認証成功後に切断された状態（デフォルト値）
 */
TEST(VhalApiTestHdcpRearStateNone, State)
{
    bool start{true};
	int32_t result{};
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

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
 * 状態/イベントマトリクス網羅
 * HDCP first認証に成功した状態
 */
TEST(VhalApiTestHdcpRearStateSuccess, State)
{
    bool start{true};
	int32_t result{};
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

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
 * 状態/イベントマトリクス網羅
 * HDCP first認証に失敗した状態
 */
TEST(VhalApiTestHdcpRearStateFailed, State)
{
    bool start{true};
	int32_t result{};
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

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
 * 状態/イベントマトリクス網羅
 * 下位層の異常により認証解除されてもう一度HDCP first認証が必要な状態
 */
TEST(VhalApiTestHdcpRearStateReauth, State)
{
    bool start{true};
	int32_t result{};
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	SetConnectedRse(obj, VIDEO_HAL_CONNECTED_RSE_FULL);

    /* TODO:状態遷移 */

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "hdcp.first_auth.status.rse.result", CallbackHdcp));

 	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "hdcp.first_auth.control.rse", &start, sizeof(start), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for hdcp.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_hdcp_expect == status_hdcp); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "hdcp.first_auth.status.rse.result", CallbackHdcp));
    
    /* TODO:切断検知 */

    cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 前提を満たさない条件でのコール 
 * RSE未接続でfirst認証開始要求を設定する
 * 前提条件:RSE未接続
 */
TEST(VhalApiTestHdcpRearAbnormalNotConnectedR, AbnormalNotConnectedR)
{
    bool start{true};
	int32_t result{};
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "hdcp.first_auth.status.rse.result", CallbackHdcp));

	status_hdcp = VIDEO_HAL_HDCP_FIRST_AUTH_STS_SUCCESS;
	status_hdcp_expect = VIDEO_HAL_HDCP_FIRST_AUTH_STS_NONE;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "hdcp.first_auth.control.rse", &start, sizeof(start), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for hdcp.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_hdcp_expect == status_hdcp); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "hdcp.first_auth.status.rse.result", CallbackHdcp));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 別スレッドからHDCP認証要求
 */
static void HdcpAuthOther(void)
{
	cockpit::hal::video_hal::CtlObj obj {0};

	cockpit::hal::video_hal::Init(obj);

	SetConnectedRse(obj, VIDEO_HAL_CONNECTED_RSE_FULL);

	bool start = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "hdcp.first_auth.control.rse", &start, sizeof(start), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * プロセス待ち状態におけるAPI再受信
 * HDCP認証中に別スレッドからHDCP認証。
 */
TEST_P(VhalApiTestHdcpRearCallDouble, CallDouble)
{
	VhalApiTestHdcpRearCallDoubleParameter param = GetParam();
	bool intime{};

	cockpit::hal::video_hal::Init(obj);

	SetConnectedRse(obj, VIDEO_HAL_CONNECTED_RSE_FULL);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "hdcp.first_auth.control.rse", &status_hdcp_expect, sizeof(status_hdcp_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	/* 別スレッドから要求 */
	std::thread th(HdcpAuthOther);
	th.join();

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * イベント競合(ソフト要因)
 */
TEST_P(VhalApiTestHdcpRearSwEventConflict, SwEventConflict)
{
	VhalApiTestHdcpRearEventConflictParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};

	cockpit::hal::video_hal::Init(obj);

	if (INTERNAL_EVENT_DELETE_SURFACE == param.internal_event_param.type)
	{
		ExecuteRenderImage(obj, param.internal_event_param.u.delete_surface.surface_id);
	}

	SetConnectedRse(obj, VIDEO_HAL_CONNECTED_RSE_FULL);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "hdcp.first_auth.status.rse.result", CallbackHdcp));

	status_hdcp = VIDEO_HAL_HDCP_FIRST_AUTH_STS_NONE;
	status_hdcp_expect = VIDEO_HAL_HDCP_FIRST_AUTH_STS_SUCCESS;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "hdcp.first_auth.control.rse", &status_hdcp_expect, sizeof(status_hdcp_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	param.internal_event_param.obj = obj;
	OccurrInternalEvent(param.internal_event_param);

	PrintMsg("wait_for hdcp.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_hdcp_expect == status_hdcp); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "hdcp.first_auth.status.rse.result", CallbackHdcp));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);	
}

/**
 * イベント競合(ハード要因)
 */
TEST_P(VhalApiTestHdcpRearHwEventConflict, HwEventConflict)
{
	VhalApiTestHdcpRearEventConflictParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};

	if (INTERNAL_EVENT_DELETE_SURFACE == param.internal_event_param.type)
	{
		ExecuteRenderImage(param.internal_event_param.u.delete_surface.surface_id);
	}

	cockpit::hal::video_hal::Init(obj);

	SetConnectedRse(obj, VIDEO_HAL_CONNECTED_RSE_FULL);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "hdcp.first_auth.status.rse.result", CallbackHdcp));

	status_hdcp = VIDEO_HAL_HDCP_FIRST_AUTH_STS_NONE;
	status_hdcp_expect = VIDEO_HAL_HDCP_FIRST_AUTH_STS_SUCCESS;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "hdcp.first_auth.control.rse", &status_hdcp_expect, sizeof(status_hdcp_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	param.internal_event_param.obj = obj;
	OccurrInternalEvent(param.internal_event_param);

	PrintMsg("wait_for hdcp.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_hdcp_expect == status_hdcp); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "hdcp.first_auth.status.rse.result", CallbackHdcp));

	cockpit::hal::video_hal::Deinit(obj);
}
