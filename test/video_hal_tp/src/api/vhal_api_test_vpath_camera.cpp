#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static std::string cb_available_param;
static bool status_available;
static bool status_available_expect;
static int32_t status_current;
static int32_t status_current_expect;
static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;
static int32_t callback_current_count;

static void CallbackCurrent(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.camera.status.current", &status_current, sizeof(status_current), VIDEO_HAL_DATA_TYPE_NUM));
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
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.camera.status.available?vpath=" + cb_available_param, &status_available, sizeof(status_available), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(status_available_expect, status_available);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackCurrentRepeat(void)
{
	PrintMsg("start.");
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.camera.status.current", &status_current, sizeof(status_current), VIDEO_HAL_DATA_TYPE_NUM));
		callback_current_count++;
	}
	PrintMsg("end.");
}

struct VhalApiTestVpathCameraParameter {
	int32_t surface_id;
	std::string vpath;
};

struct VhalApiTestVpathCameraSceneParameter {
	SetPowerStatus SetScene;
	SetPowerStatus ResetScene;
};

struct VhalApiTestVpathCameraAbnormalArgParameter {
	std::string vpath;
};

struct VhalApiTestVpathCameraAbnormalNotConnectedCParameter {
	std::string vpath;
};

struct VhalApiTestVpathCameraCallDoubleParameter {
	std::string vpath;
	std::string other_vpath;
};

struct VhalApiTestVpathCameraEventConflictParameter {
	std::string vpath;
	InternalEventParameter internal_event_param;
};

class VhalApiTestVpathCameraNormalSurfaceVhal : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathCameraParameter> {};
class VhalApiTestVpathCameraNormalSurfaceNone : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathCameraParameter> {};
class VhalApiTestVpathCameraRepeat : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathCameraParameter> {};
class VhalApiTestVpathCameraScene : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathCameraSceneParameter> {};
class VhalApiTestVpathCameraAbnormalArg : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathCameraAbnormalArgParameter> {};
class VhalApiTestVpathCameraAbnormalArgAvailable : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathCameraAbnormalArgParameter> {};
class VhalApiTestVpathCameraAbnormalNotConnectedC : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathCameraAbnormalNotConnectedCParameter> {};
class VhalApiTestVpathCameraCallDouble : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathCameraCallDoubleParameter> {};
class VhalApiTestVpathCameraSwEventConflict : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathCameraEventConflictParameter> {};
class VhalApiTestVpathCameraHwEventConflict : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathCameraEventConflictParameter> {};

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathCameraNormal, VhalApiTestVpathCameraNormalSurfaceVhal, ::testing::Values(
	VhalApiTestVpathCameraParameter{0, VPATH_NAME_CAMERA}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathCameraNormal, VhalApiTestVpathCameraNormalSurfaceNone, ::testing::Values(
	VhalApiTestVpathCameraParameter{0, VPATH_NAME_CLEAR}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathCameraScene, VhalApiTestVpathCameraScene, ::testing::Values(
	VhalApiTestVpathCameraSceneParameter{SetPowerStatusPre, SetPowerStatusNormal},
	VhalApiTestVpathCameraSceneParameter{SetPowerStatusBackGround, SetPowerStatusNormal}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathCamera, VhalApiTestVpathCameraAbnormalArg, ::testing::Values(
	VhalApiTestVpathCameraAbnormalArgParameter{VPATH_NAME_UNKNOWN}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathCamera, VhalApiTestVpathCameraAbnormalArgAvailable, ::testing::Values(
	VhalApiTestVpathCameraAbnormalArgParameter{VPATH_NAME_UNKNOWN}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathCamera, VhalApiTestVpathCameraAbnormalNotConnectedC, ::testing::Values(
	VhalApiTestVpathCameraAbnormalNotConnectedCParameter{VPATH_NAME_CAMERA}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathCamera, VhalApiTestVpathCameraCallDouble, ::testing::Values(
	VhalApiTestVpathCameraCallDoubleParameter{VPATH_NAME_CAMERA, VPATH_NAME_CLEAR}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathCamera, VhalApiTestVpathCameraSwEventConflict, ::testing::Values(
	VhalApiTestVpathCameraEventConflictParameter{VPATH_NAME_CAMERA, .internal_event_param{.type = INTERNAL_EVENT_CREATE_SURFACE, .u{.create_surface{.surface_id = SURFACE_ID_ANDROIDAUTO}}}},
	VhalApiTestVpathCameraEventConflictParameter{VPATH_NAME_CAMERA, .internal_event_param{.type = INTERNAL_EVENT_DELETE_SURFACE, .u{.delete_surface{.surface_id = SURFACE_ID_ANDROIDAUTO}}}},
	VhalApiTestVpathCameraEventConflictParameter{VPATH_NAME_CAMERA, .internal_event_param{.type = INTERNAL_EVENT_NOTIFY_SURFACE_SIZE, .u{.notify_surface_size{.surface_id = SURFACE_ID_ANDROIDAUTO}}}}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathCamera, VhalApiTestVpathCameraHwEventConflict, ::testing::Values(
 	VhalApiTestVpathCameraEventConflictParameter{VPATH_NAME_CAMERA, .internal_event_param{.type = INTERNAL_EVENT_DETECT_CAMERA_SYNC,       .u{.detect_camera_sync{.cam_sync = CAM_SYNC}}}},
 	VhalApiTestVpathCameraEventConflictParameter{VPATH_NAME_CAMERA, .internal_event_param{.type = INTERNAL_EVENT_NOTIFY_CAMERA_TYPE,       .u{.notify_camera_type{.cam_size = CAM_SIZE_1920_1080}}}},
 	VhalApiTestVpathCameraEventConflictParameter{VPATH_NAME_CAMERA, .internal_event_param{.type = INTERNAL_EVENT_CHANGE_HDMI_VIDEO_FORMAT, .u{.change_hdmi_video_format{.format = SYSINFO_CONN_INFO_VIDEO_FORMAT_1280X720P}}}},
 	VhalApiTestVpathCameraEventConflictParameter{VPATH_NAME_CAMERA, .internal_event_param{.type = INTERNAL_EVENT_DETECT_CONNECTED_HDMI,    .u{.detect_connected_hdmi{.conn = DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECT}}}}
));

/**
 * 基本動作
 * サーフェス作成:VideoHAL
 * available通知タイミングは未定義のため確認無し。
 */
TEST_P(VhalApiTestVpathCameraNormalSurfaceVhal, Normal)
{
	VhalApiTestVpathCameraParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.current", CallbackCurrent));

	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.current", &param.vpath, param.vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for current");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
	EXPECT_EQ(true, intime);

	std::string control_vpath{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.camera.control.current", &control_vpath, GETVALSIZE_MAX, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(param.vpath, control_vpath);

	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetCameraOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.current", CallbackCurrent));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * サーフェス作成:なし
 */
TEST_P(VhalApiTestVpathCameraNormalSurfaceNone, Normal)
{
	VhalApiTestVpathCameraParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.current", CallbackCurrent));

	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.current", &param.vpath, param.vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for current");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
	EXPECT_EQ(true, intime);

	std::string control_vpath{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.camera.control.current", &control_vpath, GETVALSIZE_MAX, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(param.vpath, control_vpath);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.current", CallbackCurrent));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 */
TEST(VhalApiTestVpathCameraNotSet, NotSet)
{
	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.camera.status.current", &status_current, sizeof(status_current), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_VPATH_STS_FAILED, status_current);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.camera.status.available?vpath=" VPATH_NAME_CLEAR, &status_available, sizeof(status_available), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(true, status_available);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.camera.status.available?vpath=" VPATH_NAME_CAMERA, &status_available, sizeof(status_available), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(true, status_available);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 * 映像パス切替の通知を受ける前に映像パスを設定する。
 * 要求成功回数とコールバック呼び出し回数が等しいことを確認する。
 */
TEST_F(VhalApiTestVpathCameraRepeat, Repeat)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};
	int32_t success_count{};
	int32_t failed_count{};
	callback_current_count = 0;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.current", CallbackCurrentRepeat));

	std::string vpath_names[2] = {VPATH_NAME_CAMERA, VPATH_NAME_CLEAR};

	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
		status_current = VIDEO_HAL_VPATH_STS_FAILED;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.current", &vpath_names[i % 2], vpath_names[i % 2].length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		int32_t ret = cockpit::hal::video_hal::RequestUpdate(obj);
		if (VIDEO_HAL_API_SUCCESS == ret)
		{
			success_count++;
		} 
		else {
			failed_count++;
		}
		if(0 == (i % 2))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(240));
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(40));
		}
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	EXPECT_EQ(success_count, callback_current_count);

	PrintMsg("success_count=" + std::to_string(success_count) + " failed_count=" + std::to_string(failed_count) + " callback_current_count=" + std::to_string(callback_current_count));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.current", CallbackCurrentRepeat));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 状態/イベントマトリクス網羅
 * シーン別電源 
 */
TEST_P(VhalApiTestVpathCameraScene, Scene)
{
	VhalApiTestVpathCameraSceneParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};
	std::string vpath{};

	param.SetScene();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.current", CallbackCurrent));

	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	vpath = VPATH_NAME_CAMERA;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for current");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.current", CallbackCurrent));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));
	EXPECT_EQ(VHAL_TEST_SUCCESS, MuteDisplayFront(obj, false));

	cockpit::hal::video_hal::Deinit(obj);

	param.ResetScene();
}

/**
 * パラメータ異常 
 */
TEST_P(VhalApiTestVpathCameraAbnormalArg, AbnormalArg)
{
	VhalApiTestVpathCameraAbnormalArgParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.current", CallbackCurrent));

	status_current_expect = VIDEO_HAL_VPATH_STS_FAILED;
	status_current = VIDEO_HAL_VPATH_STS_SUCCESS;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.current", &param.vpath, param.vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for current");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.current", CallbackCurrent));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ異常
 * 映像パス設定可否判定
 */
TEST_P(VhalApiTestVpathCameraAbnormalArgAvailable, AbnormalArg)
{
	VhalApiTestVpathCameraAbnormalArgParameter param = GetParam();
	bool available{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.camera.status.available?vpath=" + param.vpath, &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, available);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 前提を満たさない条件でのコール
 * カメラ未接続でカメラ映像パスへ切替
 * 前提条件:カメラ未接続
 */
TEST_P(VhalApiTestVpathCameraAbnormalNotConnectedC, AbnormalNotConnectedC)
{
	VhalApiTestVpathCameraAbnormalNotConnectedCParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

#if 0 /* NOP_CONNECTED_CAMERA */
    EXPECT_EQ(VHAL_TEST_SUCCESS, SetConnectedCamera(obj, VIDEO_HAL_CONNECTED_CAMERA_NONE));
#endif /* NOP_CONNECTED_CAMERA */

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.current", CallbackCurrent));

	status_current_expect = VIDEO_HAL_VPATH_STS_FAILED;
	status_current = VIDEO_HAL_VPATH_STS_SUCCESS;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.current", &param.vpath, param.vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for current");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.current", CallbackCurrent));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 別スレッドからパス切替要求
 */
static void SwitchVpathOther(std::string vpath)
{
	cockpit::hal::video_hal::CtlObj obj {0};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * プロセス待ち状態におけるAPI再受信
 * 映像パス切替中に別スレッドから映像パスを切り替える。
 */
TEST_P(VhalApiTestVpathCameraCallDouble, CallDouble)
{
	VhalApiTestVpathCameraCallDoubleParameter param = GetParam();
	bool intime{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.current", &param.vpath, param.vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	/* 別スレッドから要求 */
	std::thread th(SwitchVpathOther, param.other_vpath);
	th.join();

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * イベント競合(ソフト要因)
 */
TEST_P(VhalApiTestVpathCameraSwEventConflict, SwEventConflict)
{
	VhalApiTestVpathCameraEventConflictParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};

	cockpit::hal::video_hal::Init(obj);

	if (INTERNAL_EVENT_DELETE_SURFACE == param.internal_event_param.type)
	{
		ExecuteRenderImage(obj, param.internal_event_param.u.delete_surface.surface_id);
	}

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.current", CallbackCurrent));

	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.current", &param.vpath, param.vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	param.internal_event_param.obj = obj;
	OccurrInternalEvent(param.internal_event_param);

	PrintMsg("wait_for current");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.current", CallbackCurrent));

	KillAllRenderImage(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	cockpit::hal::video_hal::Deinit(obj);	
}

/**
 * イベント競合(ハード要因)
 */
TEST_P(VhalApiTestVpathCameraHwEventConflict, HwEventConflict)
{
	VhalApiTestVpathCameraEventConflictParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};

	if (INTERNAL_EVENT_DELETE_SURFACE == param.internal_event_param.type)
	{
		ExecuteRenderImage(param.internal_event_param.u.delete_surface.surface_id);
	}

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.current", CallbackCurrent));

	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.current", &param.vpath, param.vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	param.internal_event_param.obj = obj;
	OccurrInternalEvent(param.internal_event_param);

	PrintMsg("wait_for current");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.current", CallbackCurrent));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	cockpit::hal::video_hal::Deinit(obj);	
}
