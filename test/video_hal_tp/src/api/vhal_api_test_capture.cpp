#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static int32_t status_capture_bitmap;
static int32_t status_capture_bitmap_expect;
static const int screen_timeout_seconds  = CB_SCREEN_CAPTURE_TIMEOUT_SECOUNDS;
static const int surface_timeout_seconds = CB_TIMEOUT_SECOUNDS;
static int32_t callback_bitmap_count;
static int32_t callback_capture_count;

static void CallbackCapture(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "capture.screen.status.bitmap", &status_capture_bitmap, sizeof(status_capture_bitmap), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_capture_bitmap_expect, status_capture_bitmap);
		callback_capture_count++;
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackCaptureRepeat(void)
{
	PrintMsg("start.");
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "capture.screen.status.bitmap", &status_capture_bitmap, sizeof(status_capture_bitmap), VIDEO_HAL_DATA_TYPE_NUM));
		callback_bitmap_count++;
	}
	PrintMsg("end.");
}

static void CallbackSurfaceCapture(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "capture.surface.status.bitmap", &status_capture_bitmap, sizeof(status_capture_bitmap), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_capture_bitmap_expect, status_capture_bitmap);
		callback_capture_count++;
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackSurfaceCaptureRepeat(void)
{
	PrintMsg("start.");
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "capture.surface.status.bitmap", &status_capture_bitmap, sizeof(status_capture_bitmap), VIDEO_HAL_DATA_TYPE_NUM));
		callback_bitmap_count++;
	}
	PrintMsg("end.");
}

struct VhalApiTestCaptureIviIdParameter {
	int32_t surface_id;
	std::string screen_capture_name;
	int32_t ivi_id;
};

struct VhalApiTestCaptureIviIdStringParameter {
	int32_t surface_id;
	std::string screen_capture_name;
	std::string ivi_id;
};

struct VhalApiTestCaptureIviNameParameter {
	int32_t surface_id;
	std::string screen_capture_name;
	std::string name;
};

struct VhalApiTestCaptureCallDoubleIviIdParameter {
	int32_t ivi_id;
	int32_t other_ivi_id;
};

struct VhalApiTestCaptureEventConflictIviIdParameter {
	int32_t ivi_id;
	InternalEventParameter internal_event_param;
};

struct VhalApiTestCaptureCallDoubleIviNameParameter {
	std::string name;
	std::string other_name;
};

struct VhalApiTestCaptureEventConflictIviNameParameter {
	std::string name;
	InternalEventParameter internal_event_param;
};


/* サーフェス用構造体 */

struct VhalApiTestSurfaceCaptureIviIdParameter {
	int32_t surface_id;
	std::string surface_capture_name;
	int32_t ivi_id;
};

struct VhalApiTestSurfaceCaptureIviNameParameter {
	int32_t surface_id;
	std::string surface_capture_name;
	std::string name;
};

struct VhalApiTestSurfaceCaptureCallDoubleIviIdParameter {
	int32_t ivi_id;
	int32_t other_ivi_id;
};

struct VhalApiTestSurfaceCaptureEventConflictIviIdParameter {
	int32_t ivi_id;
	InternalEventParameter internal_event_param;
};

struct VhalApiTestSurfaceCaptureCallDoubleIviNameParameter {
	int32_t surface_id;
	std::string name;
	std::string other_name;
};

struct VhalApiTestSurfaceCaptureEventConflictIviNameParameter {
	int32_t surface_id;
	std::string name;
	InternalEventParameter internal_event_param;
};

class VhalApiTestCaptureNormalIviId : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestCaptureIviIdParameter> {};
class VhalApiTestCaptureNormalIviName : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestCaptureIviNameParameter> {};
class VhalApiTestCaptureRepeat : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestCaptureIviIdParameter> {};
class VhalApiTestCaptureAbnormalArgIviId : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestCaptureIviIdStringParameter> {};
class VhalApiTestCaptureAbnormalArgIviName : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestCaptureIviNameParameter> {};
class VhalApiTestCaptureAbnormalArgInvalidPath : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestCaptureIviIdParameter> {};
class VhalApiTestCaptureAbnormalNotConnectedR : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestCaptureIviIdParameter> {};
class VhalApiTestCaptureAbnormalNotConnectedI : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestCaptureIviIdParameter> {};
class VhalApiTestCaptureCallDoubleIviId : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestCaptureCallDoubleIviIdParameter> {};
class VhalApiTestCaptureSwEventConflictIviId : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestCaptureEventConflictIviIdParameter> {};
class VhalApiTestCaptureHwEventConflictIviId : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestCaptureEventConflictIviIdParameter> {};
class VhalApiTestCaptureCallDoubleIviName : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestCaptureCallDoubleIviNameParameter> {};
class VhalApiTestCaptureSwEventConflictIviName : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestCaptureEventConflictIviNameParameter> {};
class VhalApiTestCaptureHwEventConflictIviName : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestCaptureEventConflictIviNameParameter> {};

class VhalApiTestSurfaceCaptureNormalIviId : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestSurfaceCaptureIviIdParameter> {};
class VhalApiTestSurfaceCaptureNormalIviName : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestSurfaceCaptureIviNameParameter> {};
class VhalApiTestSurfaceCaptureRepeat : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestSurfaceCaptureIviIdParameter> {};
class VhalApiTestSurfaceCaptureAbnormalArgIviId : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestSurfaceCaptureIviIdParameter> {};
class VhalApiTestSurfaceCaptureAbnormalArgIviName : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestSurfaceCaptureIviNameParameter> {};
class VhalApiTestSurfaceCaptureAbnormalArgInvalidPath : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestSurfaceCaptureIviIdParameter> {};
class VhalApiTestSurfaceCaptureAbnormalNotConnectedR : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestSurfaceCaptureIviIdParameter> {};
class VhalApiTestSurfaceCaptureAbnormalNotConnectedI : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestSurfaceCaptureIviIdParameter> {};
class VhalApiTestSurfaceCaptureCallDoubleIviId : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestSurfaceCaptureCallDoubleIviIdParameter> {};
class VhalApiTestSurfaceCaptureSwEventConflictIviId : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestSurfaceCaptureEventConflictIviIdParameter> {};
class VhalApiTestSurfaceCaptureHwEventConflictIviId : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestSurfaceCaptureEventConflictIviIdParameter> {};
class VhalApiTestSurfaceCaptureCallDoubleIviName : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestSurfaceCaptureCallDoubleIviNameParameter> {};
class VhalApiTestSurfaceCaptureSwEventConflictIviName : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestSurfaceCaptureEventConflictIviNameParameter> {};
class VhalApiTestSurfaceCaptureHwEventConflictIviName : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestSurfaceCaptureEventConflictIviNameParameter> {};

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestCaptureNormalIviId, ::testing::Values(
	VhalApiTestCaptureIviIdParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, SCREEN_ID_FRONT}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestCaptureNormalIviName, ::testing::Values(
	VhalApiTestCaptureIviNameParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, SCREEN_NAME_FRONT}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestCaptureRepeat, ::testing::Values(
	VhalApiTestCaptureIviIdParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, SCREEN_ID_FRONT}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestCaptureAbnormalArgIviId, ::testing::Values(
	VhalApiTestCaptureIviIdStringParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, ""},				/* ヌル */
	VhalApiTestCaptureIviIdStringParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, "abc"},			/* 数値以外 */
	VhalApiTestCaptureIviIdStringParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, "2147483648"},		/* int32_tを超える値 */
	VhalApiTestCaptureIviIdStringParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, "-2147483649"}		/* int32_tを超える値 */
));

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestCaptureAbnormalArgIviName, ::testing::Values(
	VhalApiTestCaptureIviNameParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, "invalid_screen_id"}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestCaptureAbnormalArgInvalidPath, ::testing::Values(
	VhalApiTestCaptureIviIdParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, SCREEN_ID_FRONT}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestCaptureAbnormalNotConnectedR, ::testing::Values(
	VhalApiTestCaptureIviIdParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, SCREEN_ID_REAR}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestCaptureAbnormalNotConnectedI, ::testing::Values(
	VhalApiTestCaptureIviIdParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, SCREEN_ID_IC}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestCaptureCallDoubleIviId, ::testing::Values(
	VhalApiTestCaptureCallDoubleIviIdParameter{SCREEN_ID_FRONT, SCREEN_ID_REAR}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestCaptureSwEventConflictIviId, ::testing::Values(
	VhalApiTestCaptureEventConflictIviIdParameter{SCREEN_ID_FRONT, .internal_event_param{.type = INTERNAL_EVENT_CREATE_SURFACE, .u{.create_surface{.surface_id = SURFACE_ID_ANDROIDAUTO}}}},
	VhalApiTestCaptureEventConflictIviIdParameter{SCREEN_ID_FRONT, .internal_event_param{.type = INTERNAL_EVENT_DELETE_SURFACE, .u{.delete_surface{.surface_id = SURFACE_ID_ANDROIDAUTO}}}},
	VhalApiTestCaptureEventConflictIviIdParameter{SCREEN_ID_FRONT, .internal_event_param{.type = INTERNAL_EVENT_NOTIFY_SURFACE_SIZE, .u{.notify_surface_size{.surface_id = SURFACE_ID_ANDROIDAUTO}}}}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestCaptureHwEventConflictIviId, ::testing::Values(
 	VhalApiTestCaptureEventConflictIviIdParameter{SCREEN_ID_FRONT, .internal_event_param{.type = INTERNAL_EVENT_DETECT_CAMERA_SYNC,       .u{.detect_camera_sync{.cam_sync = CAM_SYNC}}}},
 	VhalApiTestCaptureEventConflictIviIdParameter{SCREEN_ID_FRONT, .internal_event_param{.type = INTERNAL_EVENT_NOTIFY_CAMERA_TYPE,       .u{.notify_camera_type{.cam_size = CAM_SIZE_1920_1080}}}},
 	VhalApiTestCaptureEventConflictIviIdParameter{SCREEN_ID_FRONT, .internal_event_param{.type = INTERNAL_EVENT_CHANGE_HDMI_VIDEO_FORMAT, .u{.change_hdmi_video_format{.format = SYSINFO_CONN_INFO_VIDEO_FORMAT_1280X720P}}}},
 	VhalApiTestCaptureEventConflictIviIdParameter{SCREEN_ID_FRONT, .internal_event_param{.type = INTERNAL_EVENT_DETECT_CONNECTED_HDMI,    .u{.detect_connected_hdmi{.conn = DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECT}}}}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestCaptureCallDoubleIviName, ::testing::Values(
	VhalApiTestCaptureCallDoubleIviNameParameter{SCREEN_NAME_FRONT, SCREEN_NAME_REAR}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestCaptureSwEventConflictIviName, ::testing::Values(
	VhalApiTestCaptureEventConflictIviNameParameter{SCREEN_NAME_FRONT, .internal_event_param{.type = INTERNAL_EVENT_CREATE_SURFACE, .u{.create_surface{.surface_id = SURFACE_ID_ANDROIDAUTO}}}},
	VhalApiTestCaptureEventConflictIviNameParameter{SCREEN_NAME_FRONT, .internal_event_param{.type = INTERNAL_EVENT_DELETE_SURFACE, .u{.delete_surface{.surface_id = SURFACE_ID_ANDROIDAUTO}}}},
	VhalApiTestCaptureEventConflictIviNameParameter{SCREEN_NAME_FRONT, .internal_event_param{.type = INTERNAL_EVENT_NOTIFY_SURFACE_SIZE, .u{.notify_surface_size{.surface_id = SURFACE_ID_ANDROIDAUTO}}}}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestCaptureHwEventConflictIviName, ::testing::Values(
 	VhalApiTestCaptureEventConflictIviNameParameter{SCREEN_NAME_FRONT, .internal_event_param{.type = INTERNAL_EVENT_DETECT_CAMERA_SYNC,       .u{.detect_camera_sync{.cam_sync = CAM_SYNC}}}},
 	VhalApiTestCaptureEventConflictIviNameParameter{SCREEN_NAME_FRONT, .internal_event_param{.type = INTERNAL_EVENT_NOTIFY_CAMERA_TYPE,       .u{.notify_camera_type{.cam_size = CAM_SIZE_1920_1080}}}},
 	VhalApiTestCaptureEventConflictIviNameParameter{SCREEN_NAME_FRONT, .internal_event_param{.type = INTERNAL_EVENT_CHANGE_HDMI_VIDEO_FORMAT, .u{.change_hdmi_video_format{.format = SYSINFO_CONN_INFO_VIDEO_FORMAT_1280X720P}}}},
 	VhalApiTestCaptureEventConflictIviNameParameter{SCREEN_NAME_FRONT, .internal_event_param{.type = INTERNAL_EVENT_DETECT_CONNECTED_HDMI,    .u{.detect_connected_hdmi{.conn = DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECT}}}}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestSurfaceCaptureNormalIviId, ::testing::Values(
	VhalApiTestSurfaceCaptureIviIdParameter{SURFACE_ID_FRONT_DTV, SURFACE_NAME_FRONT_DTV, SURFACE_ID_FRONT_DTV},
	VhalApiTestSurfaceCaptureIviIdParameter{SURFACE_ID_REAR_DTV,  SURFACE_NAME_REAR_DTV,  SURFACE_ID_REAR_DTV},
	VhalApiTestSurfaceCaptureIviIdParameter{SURFACE_ID_IC_MAP,    SURFACE_NAME_IC_MAP,    SURFACE_ID_IC_MAP}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestSurfaceCaptureNormalIviName, ::testing::Values(
	VhalApiTestSurfaceCaptureIviNameParameter{SURFACE_ID_FRONT_DTV, SURFACE_NAME_FRONT_DTV, SURFACE_NAME_FRONT_DTV},
	VhalApiTestSurfaceCaptureIviNameParameter{SURFACE_ID_REAR_DTV,  SURFACE_NAME_REAR_DTV,  SURFACE_NAME_REAR_DTV},
	VhalApiTestSurfaceCaptureIviNameParameter{SURFACE_ID_IC_MAP,    SURFACE_NAME_IC_MAP,    SURFACE_NAME_IC_MAP}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestSurfaceCaptureRepeat, ::testing::Values(
	VhalApiTestSurfaceCaptureIviIdParameter{SURFACE_ID_FRONT_DTV, SURFACE_NAME_FRONT_DTV, SURFACE_ID_FRONT_DTV},
	VhalApiTestSurfaceCaptureIviIdParameter{SURFACE_ID_REAR_DTV,  SURFACE_NAME_REAR_DTV,  SURFACE_ID_REAR_DTV},
	VhalApiTestSurfaceCaptureIviIdParameter{SURFACE_ID_IC_MAP,    SURFACE_NAME_IC_MAP,    SURFACE_ID_IC_MAP}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestSurfaceCaptureAbnormalArgIviId, ::testing::Values(
	VhalApiTestSurfaceCaptureIviIdParameter{SURFACE_ID_FRONT_DTV, SURFACE_NAME_FRONT_DTV, 99999},
	VhalApiTestSurfaceCaptureIviIdParameter{SURFACE_ID_REAR_DTV,  SURFACE_NAME_REAR_DTV,  99999},
	VhalApiTestSurfaceCaptureIviIdParameter{SURFACE_ID_IC_MAP,    SURFACE_NAME_IC_MAP,    99999}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestSurfaceCaptureAbnormalArgIviName, ::testing::Values(
	VhalApiTestSurfaceCaptureIviNameParameter{SURFACE_ID_FRONT_DTV, SURFACE_NAME_FRONT_DTV, "invalid_surface_name"},
	VhalApiTestSurfaceCaptureIviNameParameter{SURFACE_ID_REAR_DTV,  SURFACE_NAME_REAR_DTV,  "invalid_surface_name"},
	VhalApiTestSurfaceCaptureIviNameParameter{SURFACE_ID_IC_MAP,    SURFACE_NAME_IC_MAP,    "invalid_surface_name"}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestSurfaceCaptureAbnormalArgInvalidPath, ::testing::Values(
	VhalApiTestSurfaceCaptureIviIdParameter{SURFACE_ID_FRONT_DTV, SURFACE_NAME_FRONT_DTV, SURFACE_ID_FRONT_DTV},
	VhalApiTestSurfaceCaptureIviIdParameter{SURFACE_ID_REAR_DTV,  SURFACE_NAME_REAR_DTV,  SURFACE_ID_REAR_DTV},
	VhalApiTestSurfaceCaptureIviIdParameter{SURFACE_ID_IC_MAP,    SURFACE_NAME_IC_MAP,    SURFACE_ID_IC_MAP}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestSurfaceCaptureAbnormalNotConnectedR, ::testing::Values(
	VhalApiTestSurfaceCaptureIviIdParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, SCREEN_ID_REAR}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestSurfaceCaptureAbnormalNotConnectedI, ::testing::Values(
	VhalApiTestSurfaceCaptureIviIdParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, SCREEN_ID_IC}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestSurfaceCaptureCallDoubleIviId, ::testing::Values(
	VhalApiTestSurfaceCaptureCallDoubleIviIdParameter{SURFACE_ID_FRONT_DTV, SURFACE_ID_ANDROIDAUTO},
	VhalApiTestSurfaceCaptureCallDoubleIviIdParameter{SURFACE_ID_REAR_DTV,  SURFACE_ID_REAR_MULTISENSORY},
	VhalApiTestSurfaceCaptureCallDoubleIviIdParameter{SURFACE_ID_IC_MAP,    SURFACE_ID_IC_CARPLAY}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestSurfaceCaptureSwEventConflictIviId, ::testing::Values(
	VhalApiTestSurfaceCaptureEventConflictIviIdParameter{SURFACE_ID_FRONT_DTV, .internal_event_param{.type = INTERNAL_EVENT_CREATE_SURFACE, .u{.create_surface{.surface_id = SURFACE_ID_ANDROIDAUTO}}}},
	VhalApiTestSurfaceCaptureEventConflictIviIdParameter{SURFACE_ID_FRONT_DTV, .internal_event_param{.type = INTERNAL_EVENT_DELETE_SURFACE, .u{.delete_surface{.surface_id = SURFACE_ID_ANDROIDAUTO}}}},
	VhalApiTestSurfaceCaptureEventConflictIviIdParameter{SURFACE_ID_FRONT_DTV, .internal_event_param{.type = INTERNAL_EVENT_NOTIFY_SURFACE_SIZE, .u{.notify_surface_size{.surface_id = SURFACE_ID_ANDROIDAUTO}}}},
	VhalApiTestSurfaceCaptureEventConflictIviIdParameter{SURFACE_ID_REAR_DTV,  .internal_event_param{.type = INTERNAL_EVENT_CREATE_SURFACE, .u{.create_surface{.surface_id = SURFACE_ID_REAR_MULTISENSORY}}}},
	VhalApiTestSurfaceCaptureEventConflictIviIdParameter{SURFACE_ID_REAR_DTV,  .internal_event_param{.type = INTERNAL_EVENT_DELETE_SURFACE, .u{.delete_surface{.surface_id = SURFACE_ID_REAR_MULTISENSORY}}}},
	VhalApiTestSurfaceCaptureEventConflictIviIdParameter{SURFACE_ID_REAR_DTV,  .internal_event_param{.type = INTERNAL_EVENT_NOTIFY_SURFACE_SIZE, .u{.notify_surface_size{.surface_id = SURFACE_ID_REAR_MULTISENSORY}}}},
	VhalApiTestSurfaceCaptureEventConflictIviIdParameter{SURFACE_ID_IC_MAP,    .internal_event_param{.type = INTERNAL_EVENT_CREATE_SURFACE, .u{.create_surface{.surface_id = SURFACE_ID_IC_CARPLAY}}}},
	VhalApiTestSurfaceCaptureEventConflictIviIdParameter{SURFACE_ID_IC_MAP,    .internal_event_param{.type = INTERNAL_EVENT_DELETE_SURFACE, .u{.delete_surface{.surface_id = SURFACE_ID_IC_CARPLAY}}}},
	VhalApiTestSurfaceCaptureEventConflictIviIdParameter{SURFACE_ID_IC_MAP,    .internal_event_param{.type = INTERNAL_EVENT_NOTIFY_SURFACE_SIZE, .u{.notify_surface_size{.surface_id = SURFACE_ID_IC_CARPLAY}}}}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestSurfaceCaptureHwEventConflictIviId, ::testing::Values(
 	VhalApiTestSurfaceCaptureEventConflictIviIdParameter{SURFACE_ID_FRONT_DTV, .internal_event_param{.type = INTERNAL_EVENT_DETECT_CAMERA_SYNC,       .u{.detect_camera_sync{.cam_sync = CAM_SYNC}}}},
 	VhalApiTestSurfaceCaptureEventConflictIviIdParameter{SURFACE_ID_FRONT_DTV, .internal_event_param{.type = INTERNAL_EVENT_NOTIFY_CAMERA_TYPE,       .u{.notify_camera_type{.cam_size = CAM_SIZE_1920_1080}}}},
 	VhalApiTestSurfaceCaptureEventConflictIviIdParameter{SURFACE_ID_FRONT_DTV, .internal_event_param{.type = INTERNAL_EVENT_CHANGE_HDMI_VIDEO_FORMAT, .u{.change_hdmi_video_format{.format = SYSINFO_CONN_INFO_VIDEO_FORMAT_1280X720P}}}},
 	VhalApiTestSurfaceCaptureEventConflictIviIdParameter{SURFACE_ID_FRONT_DTV, .internal_event_param{.type = INTERNAL_EVENT_DETECT_CONNECTED_HDMI,    .u{.detect_connected_hdmi{.conn = DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECT}}}},
 	VhalApiTestSurfaceCaptureEventConflictIviIdParameter{SURFACE_ID_REAR_DTV,  .internal_event_param{.type = INTERNAL_EVENT_DETECT_CAMERA_SYNC,       .u{.detect_camera_sync{.cam_sync = CAM_SYNC}}}},
 	VhalApiTestSurfaceCaptureEventConflictIviIdParameter{SURFACE_ID_REAR_DTV,  .internal_event_param{.type = INTERNAL_EVENT_NOTIFY_CAMERA_TYPE,       .u{.notify_camera_type{.cam_size = CAM_SIZE_1920_1080}}}},
 	VhalApiTestSurfaceCaptureEventConflictIviIdParameter{SURFACE_ID_REAR_DTV,  .internal_event_param{.type = INTERNAL_EVENT_CHANGE_HDMI_VIDEO_FORMAT, .u{.change_hdmi_video_format{.format = SYSINFO_CONN_INFO_VIDEO_FORMAT_1280X720P}}}},
 	VhalApiTestSurfaceCaptureEventConflictIviIdParameter{SURFACE_ID_REAR_DTV,  .internal_event_param{.type = INTERNAL_EVENT_DETECT_CONNECTED_HDMI,    .u{.detect_connected_hdmi{.conn = DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECT}}}},
 	VhalApiTestSurfaceCaptureEventConflictIviIdParameter{SURFACE_ID_IC_MAP,    .internal_event_param{.type = INTERNAL_EVENT_DETECT_CAMERA_SYNC,       .u{.detect_camera_sync{.cam_sync = CAM_SYNC}}}},
 	VhalApiTestSurfaceCaptureEventConflictIviIdParameter{SURFACE_ID_IC_MAP,    .internal_event_param{.type = INTERNAL_EVENT_NOTIFY_CAMERA_TYPE,       .u{.notify_camera_type{.cam_size = CAM_SIZE_1920_1080}}}},
 	VhalApiTestSurfaceCaptureEventConflictIviIdParameter{SURFACE_ID_IC_MAP,    .internal_event_param{.type = INTERNAL_EVENT_CHANGE_HDMI_VIDEO_FORMAT, .u{.change_hdmi_video_format{.format = SYSINFO_CONN_INFO_VIDEO_FORMAT_1280X720P}}}},
 	VhalApiTestSurfaceCaptureEventConflictIviIdParameter{SURFACE_ID_IC_MAP,    .internal_event_param{.type = INTERNAL_EVENT_DETECT_CONNECTED_HDMI,    .u{.detect_connected_hdmi{.conn = DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECT}}}}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestSurfaceCaptureCallDoubleIviName, ::testing::Values(
	VhalApiTestSurfaceCaptureCallDoubleIviNameParameter{SURFACE_ID_FRONT_DTV, SURFACE_NAME_FRONT_DTV, SURFACE_NAME_ANDROIDAUTO},
	VhalApiTestSurfaceCaptureCallDoubleIviNameParameter{SURFACE_ID_REAR_DTV,  SURFACE_NAME_REAR_DTV,  SURFACE_NAME_REAR_MULTISENSORY},
	VhalApiTestSurfaceCaptureCallDoubleIviNameParameter{SURFACE_ID_IC_MAP,    SURFACE_NAME_IC_MAP,    SURFACE_NAME_IC_CARPLAY}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestSurfaceCaptureSwEventConflictIviName, ::testing::Values(
	VhalApiTestSurfaceCaptureEventConflictIviNameParameter{SURFACE_ID_FRONT_DTV, SURFACE_NAME_FRONT_DTV, .internal_event_param{.type = INTERNAL_EVENT_CREATE_SURFACE, .u{.create_surface{.surface_id = SURFACE_ID_ANDROIDAUTO}}}},
	VhalApiTestSurfaceCaptureEventConflictIviNameParameter{SURFACE_ID_FRONT_DTV, SURFACE_NAME_FRONT_DTV, .internal_event_param{.type = INTERNAL_EVENT_DELETE_SURFACE, .u{.delete_surface{.surface_id = SURFACE_ID_ANDROIDAUTO}}}},
	VhalApiTestSurfaceCaptureEventConflictIviNameParameter{SURFACE_ID_FRONT_DTV, SURFACE_NAME_FRONT_DTV, .internal_event_param{.type = INTERNAL_EVENT_NOTIFY_SURFACE_SIZE, .u{.notify_surface_size{.surface_id = SURFACE_ID_ANDROIDAUTO}}}},
	VhalApiTestSurfaceCaptureEventConflictIviNameParameter{SURFACE_ID_REAR_DTV,  SURFACE_NAME_REAR_DTV,  .internal_event_param{.type = INTERNAL_EVENT_CREATE_SURFACE, .u{.create_surface{.surface_id = SURFACE_ID_REAR_MULTISENSORY}}}},
	VhalApiTestSurfaceCaptureEventConflictIviNameParameter{SURFACE_ID_REAR_DTV,  SURFACE_NAME_REAR_DTV,  .internal_event_param{.type = INTERNAL_EVENT_DELETE_SURFACE, .u{.delete_surface{.surface_id = SURFACE_ID_REAR_MULTISENSORY}}}},
	VhalApiTestSurfaceCaptureEventConflictIviNameParameter{SURFACE_ID_REAR_DTV,  SURFACE_NAME_REAR_DTV,  .internal_event_param{.type = INTERNAL_EVENT_NOTIFY_SURFACE_SIZE, .u{.notify_surface_size{.surface_id = SURFACE_ID_REAR_MULTISENSORY}}}},
	VhalApiTestSurfaceCaptureEventConflictIviNameParameter{SURFACE_ID_IC_MAP,    SURFACE_NAME_IC_MAP,    .internal_event_param{.type = INTERNAL_EVENT_CREATE_SURFACE, .u{.create_surface{.surface_id = SURFACE_ID_IC_CARPLAY}}}},
	VhalApiTestSurfaceCaptureEventConflictIviNameParameter{SURFACE_ID_IC_MAP,    SURFACE_NAME_IC_MAP,    .internal_event_param{.type = INTERNAL_EVENT_DELETE_SURFACE, .u{.delete_surface{.surface_id = SURFACE_ID_IC_CARPLAY}}}},
	VhalApiTestSurfaceCaptureEventConflictIviNameParameter{SURFACE_ID_IC_MAP,    SURFACE_NAME_IC_MAP,    .internal_event_param{.type = INTERNAL_EVENT_NOTIFY_SURFACE_SIZE, .u{.notify_surface_size{.surface_id = SURFACE_ID_IC_CARPLAY}}}}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestCapture, VhalApiTestSurfaceCaptureHwEventConflictIviName, ::testing::Values(
 	VhalApiTestSurfaceCaptureEventConflictIviNameParameter{SURFACE_ID_FRONT_DTV, SURFACE_NAME_FRONT_DTV, .internal_event_param{.type = INTERNAL_EVENT_DETECT_CAMERA_SYNC,       .u{.detect_camera_sync{.cam_sync = CAM_SYNC}}}},
 	VhalApiTestSurfaceCaptureEventConflictIviNameParameter{SURFACE_ID_FRONT_DTV, SURFACE_NAME_FRONT_DTV, .internal_event_param{.type = INTERNAL_EVENT_NOTIFY_CAMERA_TYPE,       .u{.notify_camera_type{.cam_size = CAM_SIZE_1920_1080}}}},
 	VhalApiTestSurfaceCaptureEventConflictIviNameParameter{SURFACE_ID_FRONT_DTV, SURFACE_NAME_FRONT_DTV, .internal_event_param{.type = INTERNAL_EVENT_CHANGE_HDMI_VIDEO_FORMAT, .u{.change_hdmi_video_format{.format = SYSINFO_CONN_INFO_VIDEO_FORMAT_1280X720P}}}},
 	VhalApiTestSurfaceCaptureEventConflictIviNameParameter{SURFACE_ID_FRONT_DTV, SURFACE_NAME_FRONT_DTV, .internal_event_param{.type = INTERNAL_EVENT_DETECT_CONNECTED_HDMI,    .u{.detect_connected_hdmi{.conn = DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECT}}}},
 	VhalApiTestSurfaceCaptureEventConflictIviNameParameter{SURFACE_ID_REAR_DTV,  SURFACE_NAME_REAR_DTV,  .internal_event_param{.type = INTERNAL_EVENT_DETECT_CAMERA_SYNC,       .u{.detect_camera_sync{.cam_sync = CAM_SYNC}}}},
 	VhalApiTestSurfaceCaptureEventConflictIviNameParameter{SURFACE_ID_REAR_DTV,  SURFACE_NAME_REAR_DTV,  .internal_event_param{.type = INTERNAL_EVENT_NOTIFY_CAMERA_TYPE,       .u{.notify_camera_type{.cam_size = CAM_SIZE_1920_1080}}}},
 	VhalApiTestSurfaceCaptureEventConflictIviNameParameter{SURFACE_ID_REAR_DTV,  SURFACE_NAME_REAR_DTV,  .internal_event_param{.type = INTERNAL_EVENT_CHANGE_HDMI_VIDEO_FORMAT, .u{.change_hdmi_video_format{.format = SYSINFO_CONN_INFO_VIDEO_FORMAT_1280X720P}}}},
 	VhalApiTestSurfaceCaptureEventConflictIviNameParameter{SURFACE_ID_REAR_DTV,  SURFACE_NAME_REAR_DTV,  .internal_event_param{.type = INTERNAL_EVENT_DETECT_CONNECTED_HDMI,    .u{.detect_connected_hdmi{.conn = DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECT}}}},
 	VhalApiTestSurfaceCaptureEventConflictIviNameParameter{SURFACE_ID_IC_MAP,    SURFACE_NAME_IC_MAP,    .internal_event_param{.type = INTERNAL_EVENT_DETECT_CAMERA_SYNC,       .u{.detect_camera_sync{.cam_sync = CAM_SYNC}}}},
 	VhalApiTestSurfaceCaptureEventConflictIviNameParameter{SURFACE_ID_IC_MAP,    SURFACE_NAME_IC_MAP,    .internal_event_param{.type = INTERNAL_EVENT_NOTIFY_CAMERA_TYPE,       .u{.notify_camera_type{.cam_size = CAM_SIZE_1920_1080}}}},
 	VhalApiTestSurfaceCaptureEventConflictIviNameParameter{SURFACE_ID_IC_MAP,    SURFACE_NAME_IC_MAP,    .internal_event_param{.type = INTERNAL_EVENT_CHANGE_HDMI_VIDEO_FORMAT, .u{.change_hdmi_video_format{.format = SYSINFO_CONN_INFO_VIDEO_FORMAT_1280X720P}}}},
 	VhalApiTestSurfaceCaptureEventConflictIviNameParameter{SURFACE_ID_IC_MAP,    SURFACE_NAME_IC_MAP,    .internal_event_param{.type = INTERNAL_EVENT_DETECT_CONNECTED_HDMI,    .u{.detect_connected_hdmi{.conn = DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECT}}}}
));

/**
 * 基本動作
 * ivi_id指定
 * 要求がtrue
 */
TEST_P(VhalApiTestCaptureNormalIviId, NormalNormalRequestIsTrue)
{
	VhalApiTestCaptureIviIdParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool request{};
	std::string path{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));

	/* 画面キャプチャ */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_FAILED;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->test_suite_name();
	test_name.append(test_info->name());
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/capture_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.screen.control.bitmap?ivi_id=" + std::to_string(param.ivi_id) + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for capture bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(screen_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);

	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * name指定
 * 要求がtrue
 */
TEST_P(VhalApiTestCaptureNormalIviName, NormalNormalRequestIsTrue)
{
	VhalApiTestCaptureIviNameParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool request{};
	std::string path{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));

	/* 画面キャプチャ */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_FAILED;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->test_suite_name();
	test_name.append(test_info->name());
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/capture_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.screen.control.bitmap?name=" + param.name + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for capture bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(screen_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);	

	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 */
/**
 * 画面キャプチャ結果はデフォルト値未定義であるため無効化。
 * 現在、実際の動作はVIDEO_HAL_CAPTURE_STS_FAILEDが返るため有効にすると失敗する。
 */
#if 0 /* NOP */
TEST(VhalApiTestCaptureNotSet, NotSet)
{
	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "capture.screen.status.bitmap", &status_capture_bitmap, sizeof(status_capture_bitmap), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_CAPTURE_STS_SUCCESS, status_capture_bitmap);

	cockpit::hal::video_hal::Deinit(obj);
}
#endif /* NOP */

/**
 * 連続コール(2回目の画面キャプチャ要求はRequestUpdateで失敗)
 */
TEST_P(VhalApiTestCaptureRepeat, Repeat)
{
	VhalApiTestCaptureIviIdParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool request{};
	std::string path{};
	int32_t success_count{};
	int32_t failed_count{};
	callback_bitmap_count = 0;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.screen.status.bitmap", CallbackCaptureRepeat));

	/* 画面キャプチャ(10ms間隔で100回連続要求) */
	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_FAILED;
		status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_SUCCESS;
		request = true;
		const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
		std::string test_name = test_info->test_suite_name();
		test_name.append(test_info->name());
		std::replace(test_name.begin(), test_name.end(), '/', '_');
		path = "/tmp/capture_" + test_name + ".bmp";
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.screen.control.bitmap?ivi_id=" + std::to_string(param.ivi_id) + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
		int32_t ret = cockpit::hal::video_hal::RequestUpdate(obj);
		if (VIDEO_HAL_API_SUCCESS == ret)
		{
			success_count++;	/* 最初の画面キャプチャ要求は成功 */
		} 
		else {
			failed_count++;		/* ２回目以降の画面キャプチャ要求は失敗 */
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	/* コールバック待ち */
//	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	/* コールバック内でカウントしているため、成功した回数分のコールバックが呼ばれるのを待つ */
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(screen_timeout_seconds), [&] { return (success_count == callback_bitmap_count); });
	EXPECT_EQ(true, intime);
	EXPECT_EQ(success_count, callback_bitmap_count);
	EXPECT_EQ(failed_count, (TEST_REPEAT_COUNT - success_count));

	PrintMsg("success_count=" + std::to_string(success_count) + " failed_count=" + std::to_string(failed_count) + " callback_bitmap_count=" + std::to_string(callback_bitmap_count));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.screen.status.bitmap", CallbackCaptureRepeat));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ異常
 * ivi_id指定(99999)
 */
TEST_P(VhalApiTestCaptureAbnormalArgIviId, AbnormalArg)
{
	VhalApiTestCaptureIviIdStringParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool request{};
	std::string path{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));

	/* 画面キャプチャ */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_FAILED;
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->test_suite_name();
	test_name.append(test_info->name());
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/capture_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.screen.control.bitmap?ivi_id=" + param.ivi_id + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for capture bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(screen_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ異常
 * name指定("invalid_screen_id")
 */
TEST_P(VhalApiTestCaptureAbnormalArgIviName, AbnormalArg)
{
	VhalApiTestCaptureIviNameParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool request{};
	std::string path{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));

	/* 画面キャプチャ */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_FAILED;
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->test_suite_name();
	test_name.append(test_info->name());
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/capture_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.screen.control.bitmap?name=" + param.name + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for capture bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(screen_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ異常
 * 存在しないファイルパス("/tmp/invalid_path/...")
 */
TEST_P(VhalApiTestCaptureAbnormalArgInvalidPath, AbnormalArg)
{
	VhalApiTestCaptureIviIdParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool request{};
	std::string path{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));

	/* 画面キャプチャ */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_FAILED;
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->test_suite_name();
	test_name.append(test_info->name());
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/invalid_path/capture_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.screen.control.bitmap?ivi_id=" + std::to_string(param.ivi_id) + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for capture bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(screen_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 前提を満たさない条件でのコール
 * RSE未接続でキャプチャ要求
 * 前提条件:RSE未接続
*/
TEST_P(VhalApiTestCaptureAbnormalNotConnectedR, AbnormalNotConnectedR)
{
	VhalApiTestCaptureIviIdParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool request{};
	std::string path{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	/* 画面キャプチャ */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_FAILED;
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->test_suite_name();
	test_name.append(test_info->name());
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/capture_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.screen.control.bitmap?ivi_id=" + std::to_string(param.ivi_id) + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for capture bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(screen_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 前提を満たさない条件でのコール
 * IC未接続でキャプチャ要求
 * 前提条件:IC未接続
*/
TEST_P(VhalApiTestCaptureAbnormalNotConnectedI, AbnormalNotConnectedI)
{
	VhalApiTestCaptureIviIdParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool request{};
	std::string path{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	/* 画面キャプチャ */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_FAILED;
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->test_suite_name();
	test_name.append(test_info->name());
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/capture_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.screen.control.bitmap?ivi_id=" + std::to_string(param.ivi_id) + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for capture bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(screen_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 別スレッドからキャプチャ要求(ivi_id指定)
 * 既に画面キャプチャ要求中の為、別スレッドから画面キャプチャ要求はRequestUpdateで失敗すること
 */
static void CaptureIviIdOther(int32_t ivi_id)
{
	bool request{true};
	std::string path{};
	cockpit::hal::video_hal::CtlObj obj {0};

	cockpit::hal::video_hal::Init(obj);

	path = "/tmp/capture_other.bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.screen.control.bitmap?ivi_id=" + std::to_string(ivi_id) + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_NE(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));	/* 画面キャプチャ要求中の為、失敗することを期待 */

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * プロセス待ち状態におけるAPI再受信
 * 画面キャプチャ要求中に、別スレッドから画面キャプチャ要求する(ivi_id指定)
 */
TEST_P(VhalApiTestCaptureCallDoubleIviId, CallDouble)
{
	VhalApiTestCaptureCallDoubleIviIdParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};
	bool request{};
	std::string path{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	/* 画面キャプチャ */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_FAILED;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->test_suite_name();
	test_name.append(test_info->name());
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/capture_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.screen.control.bitmap?ivi_id=" + std::to_string(param.ivi_id) + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	/* 別スレッドから要求 */
	std::thread th(CaptureIviIdOther, param.other_ivi_id);
	th.join();

	/* 画面キャプチャ完了待ち */
	PrintMsg("wait_for capture bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(screen_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	EXPECT_EQ(VHAL_TEST_SUCCESS, KillAllRenderImage(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * イベント競合(ソフト要因)
 * ivi_id指定
 */
TEST_P(VhalApiTestCaptureSwEventConflictIviId, SwEventConflict)
{
	VhalApiTestCaptureEventConflictIviIdParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};
	bool request{};
	std::string path{};

	cockpit::hal::video_hal::Init(obj);

	if (INTERNAL_EVENT_DELETE_SURFACE == param.internal_event_param.type)
	{
		ExecuteRenderImage(obj, param.internal_event_param.u.delete_surface.surface_id);
		std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));
	}

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));

	/* 画面キャプチャ */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_FAILED;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->test_suite_name();
	test_name.append(test_info->name());
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/capture_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.screen.control.bitmap?ivi_id=" + std::to_string(param.ivi_id) + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	/* VideoHAL内部イベント発生 */
	/* イベント：サーフェス生成、サーフェス削除、サーフェスサイズ通知 */
	param.internal_event_param.obj = obj;
	OccurrInternalEvent(param.internal_event_param);

	PrintMsg("wait_for capture bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(screen_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * イベント競合(ハード要因)
 * ivi_id指定
 */
TEST_P(VhalApiTestCaptureHwEventConflictIviId, HwEventConflict)
{
	VhalApiTestCaptureEventConflictIviIdParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};
	bool request{};
	std::string path{};

	if (INTERNAL_EVENT_DELETE_SURFACE == param.internal_event_param.type)
	{
		ExecuteRenderImage(param.internal_event_param.u.delete_surface.surface_id);
		std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));
	}

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));

	/* 画面キャプチャ */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_FAILED;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->test_suite_name();
	test_name.append(test_info->name());
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/capture_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.screen.control.bitmap?ivi_id=" + std::to_string(param.ivi_id) + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	/* VideoHAL内部イベント発生 */
	/* イベント：カメラ同期検知、カメラ種別判別通知、HDMIビデオフォーマット変更、HDMI接続検知 */
	param.internal_event_param.obj = obj;
	OccurrInternalEvent(param.internal_event_param);

	PrintMsg("wait_for capture bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(screen_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 別スレッドからキャプチャ要求(name指定)
 * 既に画面キャプチャ要求中の為、別スレッドから画面キャプチャ要求はRequestUpdateで失敗すること
*/
static void CaptureIviNameOther(std::string name)
{
	bool request{true};
	std::string path{};
	cockpit::hal::video_hal::CtlObj obj {0};

	cockpit::hal::video_hal::Init(obj);

	path = "/tmp/capture_other.bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.screen.control.bitmap?name=" + name + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_NE(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));	/* 画面キャプチャ要求中の為、失敗することを期待 */

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * プロセス待ち状態におけるAPI再受信
 * 画面キャプチャ要求中に、別スレッドから画面キャプチャ要求する(name指定)
 */
TEST_P(VhalApiTestCaptureCallDoubleIviName, CallDouble)
{
	VhalApiTestCaptureCallDoubleIviNameParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};
	bool request{};
	std::string path{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	/* 画面キャプチャ */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_FAILED;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->test_suite_name();
	test_name.append(test_info->name());
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/capture_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.screen.control.bitmap?name=" + param.name + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	/* 別スレッドから要求 */
	std::thread th(CaptureIviNameOther, param.other_name);
	th.join();

	/* 画面キャプチャ完了待ち */
	PrintMsg("wait_for capture bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(screen_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	EXPECT_EQ(VHAL_TEST_SUCCESS, KillAllRenderImage(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * イベント競合(ソフト要因)
 * name指定
 */
TEST_P(VhalApiTestCaptureSwEventConflictIviName, SwEventConflict)
{
	VhalApiTestCaptureEventConflictIviNameParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};
	bool request{};
	std::string path{};

	cockpit::hal::video_hal::Init(obj);

	if (INTERNAL_EVENT_DELETE_SURFACE == param.internal_event_param.type)
	{
		ExecuteRenderImage(obj, param.internal_event_param.u.delete_surface.surface_id);
		std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));
	}

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));

	/* 画面キャプチャ */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_FAILED;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->test_suite_name();
	test_name.append(test_info->name());
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/capture_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.screen.control.bitmap?name=" + param.name + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	/* VideoHAL内部イベント発生 */
	/* イベント：サーフェス生成、サーフェス削除、サーフェスサイズ通知 */
	param.internal_event_param.obj = obj;
	OccurrInternalEvent(param.internal_event_param);

	PrintMsg("wait_for capture bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(screen_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	EXPECT_EQ(VHAL_TEST_SUCCESS, KillAllRenderImage(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * イベント競合(ハード要因)
 * name指定
 */
TEST_P(VhalApiTestCaptureHwEventConflictIviName, HwEventConflict)
{
	VhalApiTestCaptureEventConflictIviNameParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};
	bool request{};
	std::string path{};

	if (INTERNAL_EVENT_DELETE_SURFACE == param.internal_event_param.type)
	{
		ExecuteRenderImage(param.internal_event_param.u.delete_surface.surface_id);
		std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));
	}

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));

	/* 画面キャプチャ */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_FAILED;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->test_suite_name();
	test_name.append(test_info->name());
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/capture_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.screen.control.bitmap?name=" + param.name + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	/* VideoHAL内部イベント発生 */
	/* イベント：カメラ同期検知、カメラ種別判別通知、HDMIビデオフォーマット変更、HDMI接続検知 */
	param.internal_event_param.obj = obj;
	OccurrInternalEvent(param.internal_event_param);

	PrintMsg("wait_for capture bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(screen_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	EXPECT_EQ(VHAL_TEST_SUCCESS, KillAllRenderImage(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * ivi_id指定
 * 要求がtrue
 */
TEST_P(VhalApiTestSurfaceCaptureNormalIviId, NormalNormalRequestIsTrue)
{
	VhalApiTestSurfaceCaptureIviIdParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool request{};
	std::string path{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCapture));

	std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));

	/* サーフェスキャプチャ */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_FAILED;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->test_suite_name();
	test_name.append(test_info->name());
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/capture_surface_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.surface.control.bitmap?ivi_id=" + std::to_string(param.ivi_id) + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for capture surface bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(surface_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCapture));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * name指定
 * 要求がtrue
 */
TEST_P(VhalApiTestSurfaceCaptureNormalIviName, NormalNormalRequestIsTrue)
{
	VhalApiTestSurfaceCaptureIviNameParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool request{};
	std::string path{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCapture));

	std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));

	/* サーフェスキャプチャ */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_FAILED;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->test_suite_name();
	test_name.append(test_info->name());
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/capture_surface_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.surface.control.bitmap?name=" + param.name + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for capture surface bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(surface_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCapture));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 * ivi_id指定
 */
TEST_P(VhalApiTestSurfaceCaptureRepeat, Repeat)
{
	VhalApiTestSurfaceCaptureIviIdParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool request{};
	std::string path{};
	int32_t success_count{};
	int32_t failed_count{};
	callback_bitmap_count = 0;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCaptureRepeat));

	/* サーフェスキャプチャ */
	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_FAILED;
		status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_SUCCESS;
		request = true;
		const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
		std::string test_name = test_info->test_suite_name();
		test_name.append(test_info->name());
		std::replace(test_name.begin(), test_name.end(), '/', '_');
		path = "/tmp/capture_surface_" + test_name + "_" + std::to_string(i) + ".bmp";
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.surface.control.bitmap?ivi_id=" + std::to_string(param.ivi_id) + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
		int32_t ret = cockpit::hal::video_hal::RequestUpdate(obj);
		if (VIDEO_HAL_API_SUCCESS == ret)
		{
			success_count++;
		} 
		else {
			failed_count++;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(75));
	}

	/* コールバック待ち */
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	EXPECT_EQ(success_count, callback_bitmap_count);

	PrintMsg("success_count=" + std::to_string(success_count) + " failed_count=" + std::to_string(failed_count) + " callback_bitmap_count=" + std::to_string(callback_bitmap_count));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCaptureRepeat));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ異常
 * ivi_id指定
 */
TEST_P(VhalApiTestSurfaceCaptureAbnormalArgIviId, AbnormalArg)
{
	VhalApiTestSurfaceCaptureIviIdParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool request{};
	std::string path{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCapture));

	std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));

	/* サーフェスキャプチャ */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_FAILED;
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->test_suite_name();
	test_name.append(test_info->name());
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/capture_surface_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.surface.control.bitmap?ivi_id=" + std::to_string(param.ivi_id) + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for capture surface bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(surface_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCapture));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ異常
 * name指定
 */
TEST_P(VhalApiTestSurfaceCaptureAbnormalArgIviName, AbnormalArg)
{
	VhalApiTestSurfaceCaptureIviNameParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool request{};
	std::string path{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCapture));

	std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));

	/* サーフェスキャプチャ */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_FAILED;
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->test_suite_name();
	test_name.append(test_info->name());
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/capture_surface_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.surface.control.bitmap?name=" + param.name + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for capture surface bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(surface_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCapture));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ異常
 * 存在しないファイルパス
 */
TEST_P(VhalApiTestSurfaceCaptureAbnormalArgInvalidPath, AbnormalArg)
{
	VhalApiTestSurfaceCaptureIviIdParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool request{};
	std::string path{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCapture));

	std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));

	/* サーフェスキャプチャ */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_FAILED;
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->test_suite_name();
	test_name.append(test_info->name());
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/invalid_path/capture_surface_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.surface.control.bitmap?ivi_id=" + std::to_string(param.ivi_id) + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for capture surface bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(surface_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCapture));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 前提を満たさない条件でのコール
 * RSE未接続でキャプチャ要求
 * 前提条件:RSE未接続
*/
TEST_P(VhalApiTestSurfaceCaptureAbnormalNotConnectedR, AbnormalNotConnectedR)
{
	VhalApiTestSurfaceCaptureIviIdParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool request{};
	std::string path{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCapture));

	/* サーフェスキャプチャ */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_FAILED;
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->test_suite_name();
	test_name.append(test_info->name());
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/capture_surface_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.surface.control.bitmap?ivi_id=" + std::to_string(param.surface_id) + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for capture surface bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(surface_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCapture));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 前提を満たさない条件でのコール
 * IC未接続でキャプチャ要求
 * 前提条件:IC未接続
*/
TEST_P(VhalApiTestSurfaceCaptureAbnormalNotConnectedI, AbnormalNotConnectedI)
{
	VhalApiTestSurfaceCaptureIviIdParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool request{};
	std::string path{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCapture));

	/* サーフェスキャプチャ */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_FAILED;
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->test_suite_name();
	test_name.append(test_info->name());
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/capture_surface_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.surface.control.bitmap?ivi_id=" + std::to_string(param.surface_id) + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for capture surface bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(surface_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCapture));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 別スレッドからサーフェスキャプチャ要求
 */
static void CaptureSurfaceIviIdOther(int32_t ivi_id)
{
	bool request{true};
	std::string path{};
	cockpit::hal::video_hal::CtlObj obj {0};

	cockpit::hal::video_hal::Init(obj);

	path = "/tmp/capture_surface_other.bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.surface.control.bitmap?ivi_id=" + std::to_string(ivi_id) + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * プロセス待ち状態におけるAPI再受信
 * サーフェスキャプチャ中に別スレッドからサーフェスキャプチャを要求する。
 */
TEST_P(VhalApiTestSurfaceCaptureCallDoubleIviId, CallDouble)
{
	VhalApiTestSurfaceCaptureCallDoubleIviIdParameter param = GetParam();
	bool intime{};
	bool request{};
	std::string path{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.ivi_id));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));

	/* サーフェスキャプチャ */
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->test_suite_name();
	test_name.append(test_info->name());
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/capture_surface_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.surface.control.bitmap?ivi_id=" + std::to_string(param.ivi_id) + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	/* 別スレッドから要求 */
	std::thread th(CaptureSurfaceIviIdOther, param.other_ivi_id);
	th.join();

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * イベント競合(ソフト要因)
 * ivi_id指定
 */
TEST_P(VhalApiTestSurfaceCaptureSwEventConflictIviId, SwEventConflict)
{
	VhalApiTestSurfaceCaptureEventConflictIviIdParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};
	bool request{};
	std::string path{};

	cockpit::hal::video_hal::Init(obj);

	if (INTERNAL_EVENT_DELETE_SURFACE == param.internal_event_param.type)
	{
		ExecuteRenderImage(obj, param.internal_event_param.u.delete_surface.surface_id);
	}

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.ivi_id));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCapture));

	std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));

	/* サーフェスキャプチャ */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_FAILED;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->test_suite_name();
	test_name.append(test_info->name());
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/capture_surface_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.surface.control.bitmap?ivi_id=" + std::to_string(param.ivi_id) + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	param.internal_event_param.obj = obj;
	OccurrInternalEvent(param.internal_event_param);

	PrintMsg("wait_for capture surface bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(surface_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCapture));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * イベント競合(ハード要因)
 * surface_id指定
 */
TEST_P(VhalApiTestSurfaceCaptureHwEventConflictIviId, HwEventConflict)
{
	VhalApiTestSurfaceCaptureEventConflictIviIdParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};
	bool request{};
	std::string path{};

	if (INTERNAL_EVENT_DELETE_SURFACE == param.internal_event_param.type)
	{
		ExecuteRenderImage(param.internal_event_param.u.delete_surface.surface_id);
	}

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.ivi_id));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCapture));

	std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));

	/* サーフェスキャプチャ */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_FAILED;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->test_suite_name();
	test_name.append(test_info->name());
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/capture_surface_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.surface.control.bitmap?ivi_id=" + std::to_string(param.ivi_id) + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	param.internal_event_param.obj = obj;
	OccurrInternalEvent(param.internal_event_param);

	PrintMsg("wait_for capture surface bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(surface_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCapture));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 別スレッドからキャプチャ要求
 * name指定
 */
static void CaptureSurfaceIviNameOther(std::string name)
{
	bool request{true};
	std::string path{};
	cockpit::hal::video_hal::CtlObj obj {0};

	cockpit::hal::video_hal::Init(obj);

	path = "/tmp/capture_surface_other.bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.surface.control.bitmap?name=" + name + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * プロセス待ち状態におけるAPI再受信
 * name指定
 */
TEST_P(VhalApiTestSurfaceCaptureCallDoubleIviName, CallDouble)
{
	VhalApiTestSurfaceCaptureCallDoubleIviNameParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};
	bool request{};
	std::string path{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));

	/* 画面キャプチャ */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_FAILED;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->test_suite_name();
	test_name.append(test_info->name());
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/capture_surface_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.surface.control.bitmap?name=" + param.name + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	/* 別スレッドから要求 */
	std::thread th(CaptureSurfaceIviNameOther, param.other_name);

	th.join();

	EXPECT_EQ(VHAL_TEST_SUCCESS, KillAllRenderImage(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * イベント競合(ソフト要因)
 * name指定
 */
TEST_P(VhalApiTestSurfaceCaptureSwEventConflictIviName, SwEventConflict)
{
	VhalApiTestSurfaceCaptureEventConflictIviNameParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};
	bool request{};
	std::string path{};

	cockpit::hal::video_hal::Init(obj);

	if (INTERNAL_EVENT_DELETE_SURFACE == param.internal_event_param.type)
	{
		ExecuteRenderImage(obj, param.internal_event_param.u.delete_surface.surface_id);
	}

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCapture));

	std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));

	/* 画面キャプチャ */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_FAILED;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->test_suite_name();
	test_name.append(test_info->name());
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/capture_surface_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.surface.control.bitmap?name=" + param.name + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	param.internal_event_param.obj = obj;
	OccurrInternalEvent(param.internal_event_param);

	PrintMsg("wait_for capture surface bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(surface_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCapture));

	EXPECT_EQ(VHAL_TEST_SUCCESS, KillAllRenderImage(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * イベント競合(ハード要因)
 * name指定
 */
TEST_P(VhalApiTestSurfaceCaptureHwEventConflictIviName, HwEventConflict)
{
	VhalApiTestSurfaceCaptureEventConflictIviNameParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{};
	bool request{};
	std::string path{};

	if (INTERNAL_EVENT_DELETE_SURFACE == param.internal_event_param.type)
	{
		ExecuteRenderImage(param.internal_event_param.u.delete_surface.surface_id);
	}

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCapture));

	std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));

	/* 画面キャプチャ */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_FAILED;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->test_suite_name();
	test_name.append(test_info->name());
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/capture_surface_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.surface.control.bitmap?name=" + param.name + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	param.internal_event_param.obj = obj;
	OccurrInternalEvent(param.internal_event_param);

	PrintMsg("wait_for capture surface bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(surface_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCapture));

	EXPECT_EQ(VHAL_TEST_SUCCESS, KillAllRenderImage(obj));

	cockpit::hal::video_hal::Deinit(obj);
}
