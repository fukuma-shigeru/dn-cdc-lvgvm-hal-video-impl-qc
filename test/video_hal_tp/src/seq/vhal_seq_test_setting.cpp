#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static int32_t status_connected_hdmi;
static int32_t status_connected_hdmi_expect;
static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;

static void CallbackConnectedHdmi(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vhal.setting.status.connected.hdmi", &status_connected_hdmi, sizeof(status_connected_hdmi), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_connected_hdmi_expect, status_connected_hdmi);
	}

	cond_sync.notify_one();
	PrintMsg("end.");
}

struct VhalSeqTestSettingParameter {
	int32_t surface_id;
	std::string front_vpath_name;
};

class VhalSeqTestSettingApiSpecification : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestSettingParameter> {};

INSTANTIATE_TEST_CASE_P(VhalSeqTestSetting, VhalSeqTestSettingApiSpecification, ::testing::Values(
	VhalSeqTestSettingParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV}
));

/**
 * API仕様書記載のシーケンス
 * VideoHAL動作設定
 */
TEST_P(VhalSeqTestSettingApiSpecification, CameraRseHdmi)
{
	VhalSeqTestSettingParameter param = GetParam();
	int32_t value{};
	bool flag{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	/* VideoHAL動作設定 */
	value = VIDEO_HAL_VEHICLE_DISPLAY_BLTIN_STD_8IN_HD16_9;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.display_type", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

#if 0 /* NOP_CONNECTED_CAMERA */
	value = VIDEO_HAL_CONNECTED_CAMERA_DPVM_FULLHD;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.connected.camera", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
#endif /* NOP_CONNECTED_CAMERA */

	/* 未使用のためNOP */
#if 0 /* NOP */
	value = VIDEO_HAL_CONNECTED_SEPARATE_DISP_READY;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.connected.separate_disp", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
#endif /* NOP */
	value = VIDEO_HAL_SETTING_NIGHT;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.day_night", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	value = VIDEO_HAL_THEME_COLOR_SELECT_LIGHT;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.theme_color", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.connected.hdmi", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.display.front", &flag, sizeof(flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.display.front.width", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.display.front.height", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM));

	/* 元に戻す */

	value = VIDEO_HAL_VEHICLE_DISPLAY_INVALID;
	cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.display_type", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM);
#if 0 /* NOP_CONNECTED_CAMERA */
	value = VIDEO_HAL_CONNECTED_CAMERA_INVALID;
	cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.connected.camera", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM);
#endif /* NOP_CONNECTED_CAMERA */
	/* 未使用のためNOP */
#if 0 /* NOP */
	value = VIDEO_HAL_CONNECTED_SEPARATE_DISP_INVALID;
	cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.connected.separate_disp", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM);
#endif /* NOP */
	value = VIDEO_HAL_SETTING_NIGHT;
	cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.day_night", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM);
	value = VIDEO_HAL_THEME_COLOR_AUTO_DARK;
	cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.theme_color", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 * VideoHAL動作設定
 */
TEST(VhalSeqTestSettingRepeat, VhalSettingCameraRseHdmi)
{
	int32_t value{};
	bool flag{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	/* VideoHAL動作設定 */
	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		value = VIDEO_HAL_VEHICLE_DISPLAY_BLTIN_STD_8IN_HD16_9;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.display_type", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

#if 0 /* NOP_CONNECTED_CAMERA */
		value = VIDEO_HAL_CONNECTED_CAMERA_DPVM_FULLHD;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.connected.camera", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
#endif /* NOP_CONNECTED_CAMERA */

		/* 未使用のためNOP */
#if 0 /* NOP */
		value = VIDEO_HAL_CONNECTED_SEPARATE_DISP_READY;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.connected.separate_disp", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
#endif /* NOP */

		value = VIDEO_HAL_SETTING_NIGHT;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.day_night", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

		value = VIDEO_HAL_THEME_COLOR_SELECT_LIGHT;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.theme_color", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.connected.hdmi", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.display.front", &flag, sizeof(flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.display.front.width", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.display.front.height", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM));

		/* 元に戻す */
		value = VIDEO_HAL_VEHICLE_DISPLAY_INVALID;
		cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.display_type", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM);
		/* videoHALが異常終了するのでNOP */
#if 0 /* NOP_CONNECTED_CAMERA */
		value = VIDEO_HAL_CONNECTED_CAMERA_INVALID;
		cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.connected.camera", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM);
#endif /* NOP_CONNECTED_CAMERA */		
		/* 未使用のためNOP */
#if 0 /* NOP */		
		value = VIDEO_HAL_CONNECTED_SEPARATE_DISP_INVALID;
		cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.connected.separate_disp", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM);
#endif /* NOP */		
		value = VIDEO_HAL_SETTING_NIGHT;
		cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.day_night", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM);
		value = VIDEO_HAL_THEME_COLOR_AUTO_DARK;
		cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.theme_color", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM);
	}

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設計書記載のシーケンス
 * HDMI接続検知 
 */
TEST(VhalSeqTestSettingSeqSpecification, ConnectedHdmi)
{
	bool intime{false};
	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_HDMI));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.connected.hdmi", CallbackConnectedHdmi));

	status_connected_hdmi_expect = VIDEO_HAL_CONNECTED_HDMI_READY;
	status_connected_hdmi = VIDEO_HAL_CONNECTED_HDMI_NONE;
	DetectConnectedHdmi(DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECT);
	PrintMsg("wait_for connected_hdmi.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_connected_hdmi_expect == status_connected_hdmi); });

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.connected.hdmi", CallbackConnectedHdmi));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 * 設計書記載のシーケンス
 * HDMI接続検知 
 */
TEST(VhalSeqTestSettingRepeat, ConnectedHdmi)
{
	bool intime{false};
	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_HDMI));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.connected.hdmi", CallbackConnectedHdmi));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		status_connected_hdmi_expect = VIDEO_HAL_CONNECTED_HDMI_READY;
		status_connected_hdmi = VIDEO_HAL_CONNECTED_HDMI_NONE;
		DetectConnectedHdmi(DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECT);
		PrintMsg("wait_for connected_hdmi.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_connected_hdmi_expect == status_connected_hdmi); });

		status_connected_hdmi_expect = VIDEO_HAL_CONNECTED_HDMI_NONE;
		status_connected_hdmi = VIDEO_HAL_CONNECTED_HDMI_READY;
		DetectConnectedHdmi(DAT_CNV_SYSINFO_CONN_INFO_STAT_DISCONNECT);
		PrintMsg("wait_for connected_hdmi.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_connected_hdmi_expect == status_connected_hdmi); });
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.connected.hdmi", CallbackConnectedHdmi));

	cockpit::hal::video_hal::Deinit(obj);
}