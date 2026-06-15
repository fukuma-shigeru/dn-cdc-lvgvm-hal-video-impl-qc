#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static std::string cb_available_param;
static bool status_available;
static bool status_available_expect;
static bool status_visible;
static bool status_visible_expect;
static int32_t status_display;
static int32_t status_display_expect;
static int32_t status_notification;
static int32_t status_notification_expect;
static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;
static int32_t callback_current_count;

static void CallbackDisplay(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.rear.status.rse.display", &status_display, sizeof(status_display), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_display_expect, status_display);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackNotification(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.rear.status.rse.notification", &status_notification, sizeof(status_notification), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_notification_expect, status_notification);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackDisplayRepeat(void)
{
	PrintMsg("start.");
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.rear.status.rse.display", &status_display, sizeof(status_display), VIDEO_HAL_DATA_TYPE_NUM));
		callback_current_count++;
	}
	PrintMsg("end.");
}

struct VhalApiTestVpathRseParameter {
	std::string vpath;
};

struct VhalApiTestVpathRseAbnormalArgParameter {
	std::string vpath;
};

struct VhalApiTestVpathRseCallDoubleParameter {
	std::string vpath;
	std::string other_vpath;
};

struct VhalApiTestVpathRseEventConflictParameter {
	std::string vpath;
	InternalEventParameter internal_event_param;
};

class VhalApiTestVpathRseNormal : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathRseParameter> {};
class VhalApiTestVpathRseRepeat : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathRseParameter> {};
class VhalApiTestVpathRseAbnormalArg : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathRseAbnormalArgParameter> {};
class VhalApiTestVpathRseCallDoubleVpathSwitch : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathRseCallDoubleParameter> {};
class VhalApiTestVpathRseCallDoubleReflect : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathRseCallDoubleParameter> {};
class VhalApiTestVpathRseSwEventConflictVpathSwitch : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathRseEventConflictParameter> {};
class VhalApiTestVpathRseSwEventConflictReflect : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathRseEventConflictParameter> {};
class VhalApiTestVpathRseHwEventConflictVpathSwitch : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathRseEventConflictParameter> {};
class VhalApiTestVpathRseHwEventConflictReflect : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVpathRseEventConflictParameter> {};

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathRseNormal, VhalApiTestVpathRseNormal, ::testing::Values(
	VhalApiTestVpathRseParameter{VPATH_NAME_REAR_HDMI},
	VhalApiTestVpathRseParameter{VPATH_NAME_REAR_HDMI2},
	VhalApiTestVpathRseParameter{VPATH_NAME_REAR_MIRACAST},
	VhalApiTestVpathRseParameter{VPATH_NAME_REAR_BROWSER},
	VhalApiTestVpathRseParameter{VPATH_NAME_REAR_VDSP},
	VhalApiTestVpathRseParameter{VPATH_NAME_REAR_AUDIOSEL}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathRse, VhalApiTestVpathRseAbnormalArg, ::testing::Values(
	VhalApiTestVpathRseAbnormalArgParameter{VPATH_NAME_UNKNOWN}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathRse, VhalApiTestVpathRseCallDoubleVpathSwitch, ::testing::Values(
	VhalApiTestVpathRseCallDoubleParameter{VPATH_NAME_REAR_HDMI, VPATH_NAME_REAR_HDMI2}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathRse, VhalApiTestVpathRseCallDoubleReflect, ::testing::Values(
	VhalApiTestVpathRseCallDoubleParameter{VPATH_NAME_REAR_HDMI, VPATH_NAME_REAR_HDMI2}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathRse, VhalApiTestVpathRseSwEventConflictVpathSwitch, ::testing::Values(
	VhalApiTestVpathRseEventConflictParameter{VPATH_NAME_REAR_HDMI, .internal_event_param{.type = INTERNAL_EVENT_CREATE_SURFACE, .u{.create_surface{.surface_id = SURFACE_ID_ANDROIDAUTO}}}},
	VhalApiTestVpathRseEventConflictParameter{VPATH_NAME_REAR_HDMI, .internal_event_param{.type = INTERNAL_EVENT_DELETE_SURFACE, .u{.delete_surface{.surface_id = SURFACE_ID_ANDROIDAUTO}}}},
	VhalApiTestVpathRseEventConflictParameter{VPATH_NAME_REAR_HDMI, .internal_event_param{.type = INTERNAL_EVENT_NOTIFY_SURFACE_SIZE, .u{.notify_surface_size{.surface_id = SURFACE_ID_ANDROIDAUTO}}}}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathRse, VhalApiTestVpathRseSwEventConflictReflect, ::testing::Values(
	VhalApiTestVpathRseEventConflictParameter{VPATH_NAME_REAR_HDMI, .internal_event_param{.type = INTERNAL_EVENT_CREATE_SURFACE, .u{.create_surface{.surface_id = SURFACE_ID_ANDROIDAUTO}}}},
	VhalApiTestVpathRseEventConflictParameter{VPATH_NAME_REAR_HDMI, .internal_event_param{.type = INTERNAL_EVENT_DELETE_SURFACE, .u{.delete_surface{.surface_id = SURFACE_ID_ANDROIDAUTO}}}},
	VhalApiTestVpathRseEventConflictParameter{VPATH_NAME_REAR_HDMI, .internal_event_param{.type = INTERNAL_EVENT_NOTIFY_SURFACE_SIZE, .u{.notify_surface_size{.surface_id = SURFACE_ID_ANDROIDAUTO}}}}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathRse, VhalApiTestVpathRseHwEventConflictVpathSwitch, ::testing::Values(
 	VhalApiTestVpathRseEventConflictParameter{VPATH_NAME_REAR_HDMI, .internal_event_param{.type = INTERNAL_EVENT_DETECT_CAMERA_SYNC,       .u{.detect_camera_sync{.cam_sync = CAM_SYNC}}}},
 	VhalApiTestVpathRseEventConflictParameter{VPATH_NAME_REAR_HDMI, .internal_event_param{.type = INTERNAL_EVENT_NOTIFY_CAMERA_TYPE,       .u{.notify_camera_type{.cam_size = CAM_SIZE_1920_1080}}}},
 	VhalApiTestVpathRseEventConflictParameter{VPATH_NAME_REAR_HDMI, .internal_event_param{.type = INTERNAL_EVENT_CHANGE_HDMI_VIDEO_FORMAT, .u{.change_hdmi_video_format{.format = SYSINFO_CONN_INFO_VIDEO_FORMAT_1280X720P}}}},
 	VhalApiTestVpathRseEventConflictParameter{VPATH_NAME_REAR_HDMI, .internal_event_param{.type = INTERNAL_EVENT_DETECT_CONNECTED_HDMI,    .u{.detect_connected_hdmi{.conn = DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECT}}}}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVpathRse, VhalApiTestVpathRseHwEventConflictReflect, ::testing::Values(
 	VhalApiTestVpathRseEventConflictParameter{VPATH_NAME_REAR_HDMI, .internal_event_param{.type = INTERNAL_EVENT_DETECT_CAMERA_SYNC,       .u{.detect_camera_sync{.cam_sync = CAM_SYNC}}}},
 	VhalApiTestVpathRseEventConflictParameter{VPATH_NAME_REAR_HDMI, .internal_event_param{.type = INTERNAL_EVENT_NOTIFY_CAMERA_TYPE,       .u{.notify_camera_type{.cam_size = CAM_SIZE_1920_1080}}}},
 	VhalApiTestVpathRseEventConflictParameter{VPATH_NAME_REAR_HDMI, .internal_event_param{.type = INTERNAL_EVENT_CHANGE_HDMI_VIDEO_FORMAT, .u{.change_hdmi_video_format{.format = SYSINFO_CONN_INFO_VIDEO_FORMAT_1280X720P}}}},
 	VhalApiTestVpathRseEventConflictParameter{VPATH_NAME_REAR_HDMI, .internal_event_param{.type = INTERNAL_EVENT_DETECT_CONNECTED_HDMI,    .u{.detect_connected_hdmi{.conn = DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECT}}}}
));

/**
 * 基本動作
 */
TEST_P(VhalApiTestVpathRseNormal, Normal)
{
	VhalApiTestVpathRseParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};
    bool notification{true};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRse(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.rse.display", CallbackDisplay));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.rse.notification", CallbackNotification));

	status_display_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_display = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.display", &param.vpath, param.vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for display");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_display_expect == status_display); });
	EXPECT_EQ(true, intime);

	status_notification_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_notification = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.notification", &notification, sizeof(notification), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for notification");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_notification_expect == status_notification); });
	EXPECT_EQ(true, intime);

	std::string control_vpath{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.rear.control.rse.display", &control_vpath, GETVALSIZE_MAX, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(param.vpath, control_vpath);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.rse.display", CallbackDisplay));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.rse.notification", CallbackNotification));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 */
TEST(VhalApiTestVpathRseNotSet, NotSet)
{
	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.rear.status.rse.display", &status_display, sizeof(status_display), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_VPATH_STS_FAILED, status_display);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.rear.status.rse.notification", &status_notification, sizeof(status_notification), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_VPATH_STS_FAILED, status_notification);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 * 映像パス切替の通知を受ける前に映像パスを設定する。
 * 要求成功回数とコールバック呼び出し回数が等しいことを確認する。
 */
TEST_F(VhalApiTestVpathRseRepeat, Repeat)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};
	int32_t success_count{};
	int32_t failed_count{};
	callback_current_count = 0;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.rse.display", CallbackDisplayRepeat));

	std::string vpath_names[2] = {VPATH_NAME_REAR_HDMI, VPATH_NAME_REAR_HDMI2};

	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		status_display_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
		status_display = VIDEO_HAL_VPATH_STS_FAILED;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.display", &vpath_names[i % 2], vpath_names[i % 2].length() + 1, VIDEO_HAL_DATA_TYPE_STR));
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

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.rse.display", CallbackDisplayRepeat));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ異常 
 */
TEST_P(VhalApiTestVpathRseAbnormalArg, AbnormalArg)
{
	VhalApiTestVpathRseAbnormalArgParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};
    bool notification{true};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRse(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.rse.display", CallbackDisplay));

	status_display_expect = VIDEO_HAL_VPATH_STS_FAILED;
	status_display = VIDEO_HAL_VPATH_STS_SUCCESS;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.display", &param.vpath, param.vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for display");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_display_expect == status_display); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.rse.display", CallbackDisplay));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 別スレッドからパス切替要求
 */
static void SwitchVpathOther(std::string vpath)
{
	cockpit::hal::video_hal::CtlObj obj {0};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.display", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 別スレッドから反映通知要求
 */
static void ReflectOther(void)
{
	cockpit::hal::video_hal::CtlObj obj {0};
    bool notification{true};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.notification", &notification, sizeof(notification), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * プロセス待ち状態におけるAPI再受信
 * 映像パス切替中に別スレッドから映像パスを切り替える。
 */
TEST_P(VhalApiTestVpathRseCallDoubleVpathSwitch, CallDouble)
{
	VhalApiTestVpathRseCallDoubleParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};
    bool notification{true};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRse(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.rse.notification", CallbackNotification));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.display", &param.vpath, param.vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	/* 別スレッドから要求 */
	std::thread th(SwitchVpathOther, param.other_vpath);
	th.join();

	status_notification_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_notification = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.notification", &notification, sizeof(notification), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for notification");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_notification_expect == status_notification); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.rse.notification", CallbackNotification));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * プロセス待ち状態におけるAPI再受信
 * 映像反映通知中に別スレッドから映像パスを切り替える。
 */
TEST_P(VhalApiTestVpathRseCallDoubleReflect, CallDouble)
{
	VhalApiTestVpathRseCallDoubleParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};
    bool notification{true};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRse(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.rse.display", CallbackDisplay));

	status_display_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_display = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.display", &param.vpath, param.vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for display");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_display_expect == status_display); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.notification", &notification, sizeof(notification), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	/* 別スレッドから要求 */
	std::thread th(ReflectOther);
	th.join();

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.rse.display", CallbackDisplay));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * イベント競合(ソフト要因)
 * 映像パス切替
 */
TEST_P(VhalApiTestVpathRseSwEventConflictVpathSwitch, SwEventConflict)
{
	VhalApiTestVpathRseEventConflictParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};
    bool notification{true};

	cockpit::hal::video_hal::Init(obj);

	if (INTERNAL_EVENT_DELETE_SURFACE == param.internal_event_param.type)
	{
		ExecuteRenderImage(obj, param.internal_event_param.u.delete_surface.surface_id);
	}


	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRse(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.rse.display", CallbackDisplay));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.rse.notification", CallbackNotification));

	status_display_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_display = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.display", &param.vpath, param.vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	param.internal_event_param.obj = obj;
	OccurrInternalEvent(param.internal_event_param);

	PrintMsg("wait_for display");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_display_expect == status_display); });
	EXPECT_EQ(true, intime);

	status_notification_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_notification = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.notification", &notification, sizeof(notification), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for notification");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_notification_expect == status_notification); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.rse.display", CallbackDisplay));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.rse.notification", CallbackNotification));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);	
}

/**
 * イベント競合(ソフト要因)
 * 映像反映通知
 */
TEST_P(VhalApiTestVpathRseSwEventConflictReflect, SwEventConflict)
{
	VhalApiTestVpathRseEventConflictParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};
    bool notification{true};

	cockpit::hal::video_hal::Init(obj);

	if (INTERNAL_EVENT_DELETE_SURFACE == param.internal_event_param.type)
	{
		ExecuteRenderImage(obj, param.internal_event_param.u.delete_surface.surface_id);
	}


	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRse(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.rse.display", CallbackDisplay));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.rse.notification", CallbackNotification));

	status_display_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_display = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.display", &param.vpath, param.vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for display");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_display_expect == status_display); });
	EXPECT_EQ(true, intime);

	status_notification_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_notification = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.notification", &notification, sizeof(notification), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	param.internal_event_param.obj = obj;
	OccurrInternalEvent(param.internal_event_param);

	PrintMsg("wait_for notification");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_notification_expect == status_notification); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.rse.display", CallbackDisplay));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.rse.notification", CallbackNotification));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);	
}

/**
 * イベント競合(ハード要因)
 * 映像パス切替
 */
TEST_P(VhalApiTestVpathRseHwEventConflictVpathSwitch, HwEventConflict)
{
	VhalApiTestVpathRseEventConflictParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};
    bool notification{true};

	if (INTERNAL_EVENT_DELETE_SURFACE == param.internal_event_param.type)
	{
		ExecuteRenderImage(param.internal_event_param.u.delete_surface.surface_id);
	}

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRse(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.rse.display", CallbackDisplay));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.rse.notification", CallbackNotification));

	status_display_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_display = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.display", &param.vpath, param.vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	param.internal_event_param.obj = obj;
	OccurrInternalEvent(param.internal_event_param);

	PrintMsg("wait_for display");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_display_expect == status_display); });
	EXPECT_EQ(true, intime);

	status_notification_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_notification = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.notification", &notification, sizeof(notification), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for notification");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_notification_expect == status_notification); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.rse.display", CallbackDisplay));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.rse.notification", CallbackNotification));

	cockpit::hal::video_hal::Deinit(obj);	
}

/**
 * イベント競合(ハード要因)
 * 映像反映通知
 */
TEST_P(VhalApiTestVpathRseHwEventConflictReflect, HwEventConflict)
{
	VhalApiTestVpathRseEventConflictParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};
    bool notification{true};

	if (INTERNAL_EVENT_DELETE_SURFACE == param.internal_event_param.type)
	{
		ExecuteRenderImage(param.internal_event_param.u.delete_surface.surface_id);
	}

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRse(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.rse.display", CallbackDisplay));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.rse.notification", CallbackNotification));

	status_display_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_display = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.display", &param.vpath, param.vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for display");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_display_expect == status_display); });
	EXPECT_EQ(true, intime);

	status_notification_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_notification = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.notification", &notification, sizeof(notification), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	param.internal_event_param.obj = obj;
	OccurrInternalEvent(param.internal_event_param);

	PrintMsg("wait_for notification");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_notification_expect == status_notification); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.rse.display", CallbackDisplay));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.rse.notification", CallbackNotification));

	cockpit::hal::video_hal::Deinit(obj);	
}
