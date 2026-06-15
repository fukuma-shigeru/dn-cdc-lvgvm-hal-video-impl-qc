#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static std::string cb_available_param;
static bool status_available;
static bool status_available_expect;
static bool status_visible;
static bool status_visible_expect;
static int32_t status_current;
static int32_t status_current_expect;
static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;
static int32_t callback_current_count;

static void CallbackCurrent(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.rear.status.current", &status_current, sizeof(status_current), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_current_expect, status_current);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackAvailable(void)
{
	PrintMsg("start. cb_available_param=" + cb_available_param);
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.rear.status.available?vpath=" + cb_available_param, &status_available, sizeof(status_available), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(status_available_expect, status_available);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackCurrentRepeat(void)
{
	PrintMsg("start.");
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.rear.status.current", &status_current, sizeof(status_current), VIDEO_HAL_DATA_TYPE_NUM));
		callback_current_count++;
	}
	PrintMsg("end.");
}

struct VhalApiTestVpathRearParameter {
	int32_t surface_id;
	std::string vpath;
};

struct VhalApiTestVpathRearSceneParameter {
	SetPowerStatus SetScene;
	SetPowerStatus ResetScene;
};

struct VhalApiTestVpathRearAbnormalArgParameter {
	std::string vpath;
};

struct VhalApiTestVpathRearAbnormalNoSurfaceParameter {
	std::string vpath;
};

struct VhalApiTestVpathRearAbnormalNotConnectedHParameter {
	std::string vpath;
};

struct VhalApiTestVpathRearAbnormalNotConnectedRParameter {
	std::string vpath;
};

struct VhalApiTestVpathRearCallDoubleParameter {
	uint32_t surface_id;
	std::string vpath;
	std::string other_vpath;
};

struct VhalApiTestVpathRearEventConflictParameter {
	int32_t surface_id;
	std::string vpath;
	InternalEventParameter internal_event_param;
};

class VhalApiTestVpathRearNormalSurfaceUser : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathRearParameter> {};
class VhalApiTestVpathRearNormalSurfaceHdmi : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathRearParameter> {};
class VhalApiTestVpathRearNormalSurfaceNone : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathRearParameter> {};
class VhalApiTestVpathRearRepeat : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathRearParameter> {};
class VhalApiTestVpathRearScene : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathRearSceneParameter> {};
class VhalApiTestVpathRearAbnormalArg : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathRearAbnormalArgParameter> {};
class VhalApiTestVpathRearAbnormalArgAvailable : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathRearAbnormalArgParameter> {};
class VhalApiTestVpathRearAbnormalNoSurface : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathRearAbnormalNoSurfaceParameter> {};
class VhalApiTestVpathRearAbnormalNotConnectedH : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathRearAbnormalNotConnectedHParameter> {};
class VhalApiTestVpathRearAbnormalNotConnectedR : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathRearAbnormalNotConnectedRParameter> {};
class VhalApiTestVpathRearCallDouble : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathRearCallDoubleParameter> {};
class VhalApiTestVpathRearSwEventConflict : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathRearEventConflictParameter> {};
class VhalApiTestVpathRearHwEventConflict : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathRearEventConflictParameter> {};

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathRearNormal, VhalApiTestVpathRearNormalSurfaceUser, ::testing::Values(
	VhalApiTestVpathRearParameter{SURFACE_ID_REAR_DTV, VPATH_NAME_DTV},
	VhalApiTestVpathRearParameter{SURFACE_ID_REAR_MULTISENSORY, VPATH_NAME_MULTISENSORY}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathRearNormal, VhalApiTestVpathRearNormalSurfaceHdmi, ::testing::Values(
	VhalApiTestVpathRearParameter{SURFACE_ID_REAR_HDMI, VPATH_NAME_HDMI}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathRearNormal, VhalApiTestVpathRearNormalSurfaceNone, ::testing::Values(
	VhalApiTestVpathRearParameter{0, VPATH_NAME_CLEAR}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathRearSceneHdmi, VhalApiTestVpathRearScene, ::testing::Values(
	VhalApiTestVpathRearSceneParameter{SetPowerStatusPre, SetPowerStatusNormal},
	VhalApiTestVpathRearSceneParameter{SetPowerStatusBackGround, SetPowerStatusNormal}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathRear, VhalApiTestVpathRearAbnormalArg, ::testing::Values(
	VhalApiTestVpathRearAbnormalArgParameter{VPATH_NAME_UNKNOWN}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathRear, VhalApiTestVpathRearAbnormalArgAvailable, ::testing::Values(
	VhalApiTestVpathRearAbnormalArgParameter{VPATH_NAME_UNKNOWN}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathRear, VhalApiTestVpathRearAbnormalNoSurface, ::testing::Values(
	VhalApiTestVpathRearAbnormalNoSurfaceParameter{VPATH_NAME_DTV},
	VhalApiTestVpathRearAbnormalNoSurfaceParameter{VPATH_NAME_MULTISENSORY}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathRear, VhalApiTestVpathRearAbnormalNotConnectedH, ::testing::Values(
	VhalApiTestVpathRearAbnormalNotConnectedHParameter{VPATH_NAME_HDMI}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathRear, VhalApiTestVpathRearAbnormalNotConnectedR, ::testing::Values(
	VhalApiTestVpathRearAbnormalNotConnectedRParameter{VPATH_NAME_DTV},
	VhalApiTestVpathRearAbnormalNotConnectedRParameter{VPATH_NAME_HDMI},
	VhalApiTestVpathRearAbnormalNotConnectedRParameter{VPATH_NAME_MULTISENSORY}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathRear, VhalApiTestVpathRearCallDouble, ::testing::Values(
	VhalApiTestVpathRearCallDoubleParameter{SURFACE_ID_REAR_DTV, VPATH_NAME_DTV, VPATH_NAME_HDMI}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathRear, VhalApiTestVpathRearSwEventConflict, ::testing::Values(
	VhalApiTestVpathRearEventConflictParameter{SURFACE_ID_REAR_HDMI, VPATH_NAME_HDMI, .internal_event_param{.type = INTERNAL_EVENT_CREATE_SURFACE, .u{.create_surface{.surface_id = SURFACE_ID_REAR_DTV}}}},
	VhalApiTestVpathRearEventConflictParameter{SURFACE_ID_REAR_HDMI, VPATH_NAME_HDMI, .internal_event_param{.type = INTERNAL_EVENT_DELETE_SURFACE, .u{.delete_surface{.surface_id = SURFACE_ID_REAR_DTV}}}},
	VhalApiTestVpathRearEventConflictParameter{SURFACE_ID_REAR_HDMI, VPATH_NAME_HDMI, .internal_event_param{.type = INTERNAL_EVENT_NOTIFY_SURFACE_SIZE, .u{.notify_surface_size{.surface_id = SURFACE_ID_REAR_DTV}}}}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathRear, VhalApiTestVpathRearHwEventConflict, ::testing::Values(
 	VhalApiTestVpathRearEventConflictParameter{SURFACE_ID_REAR_HDMI, VPATH_NAME_HDMI, .internal_event_param{.type = INTERNAL_EVENT_DETECT_CAMERA_SYNC,       .u{.detect_camera_sync{.cam_sync = CAM_SYNC}}}},
 	VhalApiTestVpathRearEventConflictParameter{SURFACE_ID_REAR_HDMI, VPATH_NAME_HDMI, .internal_event_param{.type = INTERNAL_EVENT_NOTIFY_CAMERA_TYPE,       .u{.notify_camera_type{.cam_size = CAM_SIZE_1920_1080}}}},
 	VhalApiTestVpathRearEventConflictParameter{SURFACE_ID_REAR_HDMI, VPATH_NAME_HDMI, .internal_event_param{.type = INTERNAL_EVENT_CHANGE_HDMI_VIDEO_FORMAT, .u{.change_hdmi_video_format{.format = SYSINFO_CONN_INFO_VIDEO_FORMAT_1280X720P}}}},
 	VhalApiTestVpathRearEventConflictParameter{SURFACE_ID_REAR_HDMI, VPATH_NAME_HDMI, .internal_event_param{.type = INTERNAL_EVENT_DETECT_CONNECTED_HDMI,    .u{.detect_connected_hdmi{.conn = DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECT}}}}
));

/**
 * 基本動作
 * サーフェス作成:ユーザ
 */
TEST_P(VhalApiTestVpathRearNormalSurfaceUser, Normal)
{
	VhalApiTestVpathRearParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.available", CallbackAvailable));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.current", CallbackCurrent));

	status_available_expect = true;
	status_available = false;
	cb_available_param = param.vpath;
	ExecuteRenderImage(param.surface_id);
	PrintMsg("wait_for avaiable");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
	EXPECT_EQ(true, intime);

	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.current", &param.vpath, param.vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for current");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
	EXPECT_EQ(true, intime);

	std::string control_vpath{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.rear.control.current", &control_vpath, GETVALSIZE_MAX, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(param.vpath, control_vpath);

	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_REAR, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetRearOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleRear(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.available", CallbackAvailable));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.current", CallbackCurrent));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * サーフェス作成:VideoHAL
 * available通知タイミングは未定義のため確認無し。
 */
TEST_P(VhalApiTestVpathRearNormalSurfaceHdmi, Normal)
{
	VhalApiTestVpathRearParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.current", CallbackCurrent));

	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.current", &param.vpath, param.vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for current");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
	EXPECT_EQ(true, intime);

	std::string control_vpath{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.rear.control.current", &control_vpath, GETVALSIZE_MAX, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(param.vpath, control_vpath);

	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_REAR, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetRearOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleRear(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.current", CallbackCurrent));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * サーフェス作成:なし
 */
TEST_P(VhalApiTestVpathRearNormalSurfaceNone, Normal)
{
	VhalApiTestVpathRearParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.current", CallbackCurrent));

	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.current", &param.vpath, param.vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for current");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
	EXPECT_EQ(true, intime);

	std::string control_vpath{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.rear.control.current", &control_vpath, GETVALSIZE_MAX, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(param.vpath, control_vpath);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.current", CallbackCurrent));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 */
TEST(VhalApiTestVpathRearNotSet, NotSet)
{
	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.rear.status.current", &status_current, sizeof(status_current), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_VPATH_STS_FAILED, status_current);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.rear.status.available?vpath=" VPATH_NAME_CLEAR, &status_available, sizeof(status_available), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(true, status_available);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.rear.status.available?vpath=" VPATH_NAME_DTV, &status_available, sizeof(status_available), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, status_available);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.rear.status.available?vpath=" VPATH_NAME_HDMI, &status_available, sizeof(status_available), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(true, status_available);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 * 映像パス切替の通知を受ける前に映像パスを設定する。
 * 要求成功回数とコールバック呼び出し回数が等しいことを確認する。
 */
TEST_F(VhalApiTestVpathRearRepeat, Repeat)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};
	int32_t success_count{};
	int32_t failed_count{};
	callback_current_count = 0;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_CLEAR));
	ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.current", CallbackCurrentRepeat));

	std::string vpath_names[2] = {VPATH_NAME_DTV, VPATH_NAME_HDMI};

	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
		status_current = VIDEO_HAL_VPATH_STS_FAILED;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.current", &vpath_names[i % 2], vpath_names[i % 2].length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		int32_t ret = cockpit::hal::video_hal::RequestUpdate(obj);
		if (VIDEO_HAL_API_SUCCESS == ret)
		{
			success_count++;
		} 
		else {
			failed_count++;
		}
		std::this_thread::sleep_for(std::chrono::microseconds(175));
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	EXPECT_EQ(success_count, callback_current_count);

	PrintMsg("success_count=" + std::to_string(success_count) + " failed_count=" + std::to_string(failed_count) + " callback_current_count=" + std::to_string(callback_current_count));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.current", CallbackCurrentRepeat));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 状態/イベントマトリクス網羅
 * シーン別電源 
 */
TEST_P(VhalApiTestVpathRearScene, Scene)
{
	VhalApiTestVpathRearSceneParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};
	std::string vpath{};

	param.SetScene();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_HDMI));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.current", CallbackCurrent));

	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	vpath = VPATH_NAME_DTV;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for current");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.current", CallbackCurrent));

	KillAllRenderImage(obj);
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayFront(obj, false));

	cockpit::hal::video_hal::Deinit(obj);

	param.ResetScene();
}

/**
 * パラメータ異常 
 */
TEST_P(VhalApiTestVpathRearAbnormalArg, AbnormalArg)
{
	VhalApiTestVpathRearAbnormalArgParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.current", CallbackCurrent));

	status_current_expect = VIDEO_HAL_VPATH_STS_FAILED;
	status_current = VIDEO_HAL_VPATH_STS_SUCCESS;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.current", &param.vpath, param.vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for current");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.current", CallbackCurrent));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ異常
 * 映像パス設定可否判定
 */
TEST_P(VhalApiTestVpathRearAbnormalArgAvailable, AbnormalArg)
{
	VhalApiTestVpathRearAbnormalArgParameter param = GetParam();
	bool available{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.rear.status.available?vpath=" + param.vpath, &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, available);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 前提を満たさない条件でのコール
 * サーフェス未作成で映像パス切替
 */
TEST_P(VhalApiTestVpathRearAbnormalNoSurface, AbnormalNoSurface)
{
	VhalApiTestVpathRearAbnormalNoSurfaceParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.current", CallbackCurrent));

	status_current_expect = VIDEO_HAL_VPATH_STS_FAILED;
	status_current = VIDEO_HAL_VPATH_STS_SUCCESS;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.current", &param.vpath, param.vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for current");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.current", CallbackCurrent));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 前提を満たさない条件でのコール
 * HDMI未接続でHDMI映像パスへ切替
 * 前提条件:HDMI未接続
 */
TEST_P(VhalApiTestVpathRearAbnormalNotConnectedH, AbnormalNotConnectedH)
{
	VhalApiTestVpathRearAbnormalNotConnectedHParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.current", CallbackCurrent));

	status_current_expect = VIDEO_HAL_VPATH_STS_FAILED;
	status_current = VIDEO_HAL_VPATH_STS_SUCCESS;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.current", &param.vpath, param.vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for current");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.current", CallbackCurrent));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 前提を満たさない条件でのコール
 * RSE未接続でRSE映像パスへ切替
 * 前提条件:RSE未接続
 */
TEST_P(VhalApiTestVpathRearAbnormalNotConnectedR, AbnormalNotConnectedR)
{
	VhalApiTestVpathRearAbnormalNotConnectedRParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.current", CallbackCurrent));

	status_current_expect = VIDEO_HAL_VPATH_STS_FAILED;
	status_current = VIDEO_HAL_VPATH_STS_SUCCESS;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.current", &param.vpath, param.vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for current");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.current", CallbackCurrent));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 別スレッドからパス切替要求
 */
static void SwitchVpathOther(std::string vpath)
{
	cockpit::hal::video_hal::CtlObj obj {0};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * プロセス待ち状態におけるAPI再受信
 * 映像パス切替中に別スレッドから映像パスを切り替える。
 */
TEST_P(VhalApiTestVpathRearCallDouble, CallDouble)
{
	VhalApiTestVpathRearCallDoubleParameter param = GetParam();
	bool intime{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_CLEAR));
	ExecuteRenderImage(obj, param.surface_id);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.current", &param.vpath, param.vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	/* 別スレッドから要求 */
	std::thread th(SwitchVpathOther, param.other_vpath);
	th.join();

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * イベント競合(ソフト要因)
 */
TEST_P(VhalApiTestVpathRearSwEventConflict, SwEventConflict)
{
	VhalApiTestVpathRearEventConflictParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};

	cockpit::hal::video_hal::Init(obj);

	if (INTERNAL_EVENT_DELETE_SURFACE == param.internal_event_param.type)
	{
		ExecuteRenderImage(obj, param.internal_event_param.u.delete_surface.surface_id);
	}

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_CLEAR));


	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.current", CallbackCurrent));

	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.current", &param.vpath, param.vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	param.internal_event_param.obj = obj;
	OccurrInternalEvent(param.internal_event_param);

	PrintMsg("wait_for current");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.current", CallbackCurrent));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);	
}

/**
 * イベント競合(ハード要因)
 */
TEST_P(VhalApiTestVpathRearHwEventConflict, HwEventConflict)
{
	VhalApiTestVpathRearEventConflictParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};

	if (INTERNAL_EVENT_DELETE_SURFACE == param.internal_event_param.type)
	{
		ExecuteRenderImage(param.internal_event_param.u.delete_surface.surface_id);
	}

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.current", CallbackCurrent));

	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.current", &param.vpath, param.vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	param.internal_event_param.obj = obj;
	OccurrInternalEvent(param.internal_event_param);

	PrintMsg("wait_for current");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.current", CallbackCurrent));

	cockpit::hal::video_hal::Deinit(obj);	
}
