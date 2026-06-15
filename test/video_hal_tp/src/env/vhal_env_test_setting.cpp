#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static int32_t status_connected_camera;
static int32_t status_connected_camera_expect;
static int32_t status_connected_rse;
static int32_t status_connected_rse_expect;
static int32_t status_connected_hdmi;
static int32_t status_connected_hdmi_expect;
static bool status_display_rear;
static bool status_display_rear_expect;
static int32_t status_display_rear_width;
static int32_t status_display_rear_width_expect;
static int32_t status_display_rear_height;
static int32_t status_display_rear_height_expect;
static bool status_display_ic;
static bool status_display_ic_expect;
static int32_t status_display_ic_width;
static int32_t status_display_ic_width_expect;
static int32_t status_display_ic_height;
static int32_t status_display_ic_height_expect;

static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;

static void CallbackConnectedCamera(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.connected.camera", &status_connected_camera, sizeof(status_connected_camera), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_connected_camera_expect, status_connected_camera);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackConnectedRse(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.connected.rse", &status_connected_rse, sizeof(status_connected_rse), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_connected_rse_expect, status_connected_rse);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackConnectedHdmi(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.connected.hdmi", &status_connected_hdmi, sizeof(status_connected_hdmi), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_connected_hdmi_expect, status_connected_hdmi);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackDisplayRear(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
    	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.display.rear", &status_display_rear, sizeof(status_display_rear), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(status_display_rear_expect, status_display_rear);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackDisplayRearWidth(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
    	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.display.rear.width", &status_display_rear_width, sizeof(status_display_rear_width), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(status_display_rear_width_expect, status_display_rear_width);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackDisplayRearHeight(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
    	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.display.rear.height", &status_display_rear_height, sizeof(status_display_rear_height), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(status_display_rear_height_expect, status_display_rear_height);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackDisplayIc(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
    	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.display.instrumentcluster", &status_display_ic, sizeof(status_display_ic), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(status_display_ic_expect, status_display_ic);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackDisplayIcWidth(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
    	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.display.instrumentcluster.width", &status_display_ic_width, sizeof(status_display_ic_width), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(status_display_ic_width_expect, status_display_ic_width);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackDisplayIcHeight(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
    	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.display.instrumentcluster.height", &status_display_ic_height, sizeof(status_display_ic_height), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(status_display_ic_height_expect, status_display_ic_height);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

/**
 * 脱落/通信途絶
 * カメラ
 */
TEST(VhalEnvTestSettingDisconnection, ConnectAndDisconnectCamera)
{
    std::unique_lock<std::mutex> lock_sync(mtx_sync);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.connected.camera", CallbackConnectedCamera));

	status_connected_camera = VIDEO_HAL_CONNECTED_CAMERA_DBGM;
	status_connected_camera_expect = VIDEO_HAL_CONNECTED_CAMERA_NONE;

	/* ACTION:ケーブルを抜く */

	PrintMsg("After disconnecting cable from camera, press enter.");
	std::cin.get();

	bool intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_connected_camera_expect == status_connected_camera);});
	EXPECT_EQ(true, intime);

	status_connected_camera = VIDEO_HAL_CONNECTED_CAMERA_NONE;
	status_connected_camera_expect = VIDEO_HAL_CONNECTED_CAMERA_DBGM;

	/* ACTION:ケーブルを差す */

	PrintMsg("After connecting cable from camera, press enter.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_connected_camera_expect == status_connected_camera);});
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.connected.camera", CallbackConnectedCamera));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));	
}

/**
 * 脱落/通信途絶
 * HDMI
 * 初期状態:HDMI接続
 */
TEST(VhalEnvTestSettingDisconnection, ConnectAndDisconnectHdmi)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.connected.hdmi", CallbackConnectedHdmi));

	status_connected_hdmi = VIDEO_HAL_CONNECTED_HDMI_READY;
	status_connected_hdmi_expect = VIDEO_HAL_CONNECTED_HDMI_NONE;

	/* ACTION:ケーブルを抜く */

	PrintMsg("After disconnecting cable from hdmi player, press enter.");
	std::cin.get();

	bool intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_connected_hdmi_expect == status_connected_hdmi);});
	EXPECT_EQ(true, intime);

	status_connected_hdmi = VIDEO_HAL_CONNECTED_HDMI_NONE;
	status_connected_hdmi_expect = VIDEO_HAL_CONNECTED_HDMI_READY;

	/* ACTION:ケーブルを差す */

	PrintMsg("After connecting cable from hdmi player, press enter.");
	std::cin.get();

	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_connected_hdmi_expect == status_connected_hdmi);});
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.connected.hdmi", CallbackConnectedHdmi));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));	
}


/**
 * 脱落/通信途絶
 * RSE
 */
TEST(VhalEnvTestSettingDisconnection, ConnectAndDisconnectRear)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.connected.rse", CallbackConnectedRse));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.display.rear", CallbackDisplayRear));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.display.rear.width", CallbackDisplayRearWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.display.rear.height", CallbackDisplayRearHeight));

	status_connected_rse = VIDEO_HAL_CONNECTED_RSE_FULL;
	status_connected_rse_expect = VIDEO_HAL_CONNECTED_RSE_NONE;
	status_display_rear = true;
	status_display_rear_expect = false;
	status_display_rear_width = 1280;
	status_display_rear_width_expect = 0;
	status_display_rear_height = 720;
	status_display_rear_height_expect = 0;

	/* ACTION:ケーブルを抜く */

	PrintMsg("After disconnecting cable from rear display, press enter.");
	std::cin.get();

	bool intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_connected_rse_expect == status_connected_rse &&
																							   status_display_rear_expect == status_display_rear &&
																							   status_display_rear_width_expect == status_display_rear_width &&
																							   status_display_rear_height_expect == status_display_rear_height);
																							});
	EXPECT_EQ(true, intime);

	status_connected_rse = VIDEO_HAL_CONNECTED_RSE_NONE;
	status_connected_rse_expect = VIDEO_HAL_CONNECTED_RSE_FULL;
	status_display_rear = false;
	status_display_rear_expect = true;
	status_display_rear_width = 0;
	status_display_rear_width_expect = 1280;
	status_display_rear_height = 0;
	status_display_rear_height_expect = 720;

	/* ACTION:ケーブルを差す */

	PrintMsg("After connecting cable from rear display, press enter.");
	std::cin.get();

	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_connected_rse_expect == status_connected_rse &&
																							   status_display_rear_expect == status_display_rear &&
																							   status_display_rear_width_expect == status_display_rear_width &&
																							   status_display_rear_height_expect == status_display_rear_height);
																							});
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.connected.rse", CallbackConnectedRse));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.display.rear", CallbackDisplayRear));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.display.rear.width", CallbackDisplayRearWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.display.rear.height", CallbackDisplayRearHeight));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));	
}

/**
 * 脱落/通信途絶
 * IC
 */
TEST(VhalEnvTestSettingDisconnection, ConnectAndDisconnectIc)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.display.instrumentcluster", CallbackDisplayIc));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.display.instrumentcluster.width", CallbackDisplayIcWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.display.instrumentcluster.height", CallbackDisplayIcHeight));

	status_display_ic = true;
	status_display_ic_expect = false;
	status_display_ic_width = 1280;
	status_display_ic_width_expect = 0;
	status_display_ic_height = 720;
	status_display_ic_height_expect = 0;

	/* ACTION:ケーブルを抜く */

	PrintMsg("After disconnecting cable from ic display, press enter.");
	std::cin.get();

	bool intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_display_ic_expect == status_display_ic &&
																							   status_display_ic_width_expect == status_display_ic_width &&
																							   status_display_ic_height_expect == status_display_ic_height); 
																							});
	EXPECT_EQ(true, intime);

	status_display_ic = false;
	status_display_ic_expect = true;
	status_display_ic_width = 0;
	status_display_ic_width_expect = 1280;
	status_display_ic_height = 0;
	status_display_ic_height_expect = 720;

	/* ACTION:ケーブルを差す */

	PrintMsg("After connecting cable from ic display, press enter.");
	std::cin.get();

	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_display_ic_expect == status_display_ic &&
																							   status_display_ic_width_expect == status_display_ic_width &&
																							   status_display_ic_height_expect == status_display_ic_height); 
																							});
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.display.instrumentcluster", CallbackDisplayIc));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.display.instrumentcluster.width", CallbackDisplayIcWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.display.instrumentcluster.height", CallbackDisplayIcHeight));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));	
}

/**
 * デバイス単独リセット
 * HDMI
 */
TEST(VhalEnvTestSettingReset, Hdmi)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.connected.hdmi", CallbackConnectedHdmi));

	status_connected_hdmi = VIDEO_HAL_CONNECTED_HDMI_READY;
	status_connected_hdmi_expect = VIDEO_HAL_CONNECTED_HDMI_NONE;

	/* ACTION:電源OFF */

	PrintMsg("After turn off hdmi player, press enter.");
	std::cin.get();

	bool intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_connected_hdmi_expect == status_connected_hdmi);});
	EXPECT_EQ(true, intime);

	status_connected_hdmi = VIDEO_HAL_CONNECTED_HDMI_NONE;
	status_connected_hdmi_expect = VIDEO_HAL_CONNECTED_HDMI_READY;

	/* ACTION:電源ON */

	PrintMsg("After turning on hdmi player, press enter.");
	std::cin.get();

	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_connected_hdmi_expect == status_connected_hdmi);});
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.connected.hdmi", CallbackConnectedHdmi));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));	
}

/**
 * デバイス単独リセット
 * RSE
 */
TEST(VhalEnvTestSettingReset, Rear)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.connected.rse", CallbackConnectedRse));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.display.rear", CallbackDisplayRear));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.display.rear.width", CallbackDisplayRearWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.display.rear.height", CallbackDisplayRearHeight));

	status_connected_rse = VIDEO_HAL_CONNECTED_RSE_FULL;
	status_connected_rse_expect = VIDEO_HAL_CONNECTED_RSE_NONE;
	status_display_rear = true;
	status_display_rear_expect = false;
	status_display_rear_width = 1280;
	status_display_rear_width_expect = 0;
	status_display_rear_height = 720;
	status_display_rear_height_expect = 0;

	/* ACTION:電源OFF */

	PrintMsg("After turning off rear display, press enter.");
	std::cin.get();

	bool intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_connected_rse_expect == status_connected_rse &&
																							   status_display_rear_expect == status_display_rear &&
																							   status_display_rear_width_expect == status_display_rear_width &&
																							   status_display_rear_height_expect == status_display_rear_height);
																							});
	EXPECT_EQ(true, intime);


	status_connected_rse = VIDEO_HAL_CONNECTED_RSE_NONE;
	status_connected_rse_expect = VIDEO_HAL_CONNECTED_RSE_FULL;
	status_display_rear = false;
	status_display_rear_expect = true;
	status_display_rear_width = 0;
	status_display_rear_width_expect = 1280;
	status_display_rear_height = 0;
	status_display_rear_height_expect = 720;

	/* ACTION:電源ON */

	PrintMsg("turn on rear display.");
	std::cin.get();

	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_connected_rse_expect == status_connected_rse &&
																							   status_display_rear_expect == status_display_rear &&
																							   status_display_rear_width_expect == status_display_rear_width &&
																							   status_display_rear_height_expect == status_display_rear_height);
																							});
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.connected.rse", CallbackConnectedRse));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.display.rear", CallbackDisplayRear));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.display.rear.width", CallbackDisplayRearWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.display.rear.height", CallbackDisplayRearHeight));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));	
}

/**
 * デバイス単独リセット
 * IC
 */
TEST(VhalEnvTestSettingReset, Ic)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.display.instrumentcluster", CallbackDisplayIc));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.display.instrumentcluster.width", CallbackDisplayIcWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.display.instrumentcluster.height", CallbackDisplayIcHeight));

	status_display_ic = true;
	status_display_ic_expect = false;
	status_display_ic_width = 1280;
	status_display_ic_width_expect = 0;
	status_display_ic_height = 720;
	status_display_ic_height_expect = 0;

	/* ACTION:電源OFF */

	PrintMsg("After turning off rear display, press enter.");
	std::cin.get();

	bool intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_display_ic_expect == status_display_ic &&
																							   status_display_ic_width_expect == status_display_ic_width &&
																							   status_display_ic_height_expect == status_display_ic_height); 
																							});
	EXPECT_EQ(true, intime);


	status_display_ic = false;
	status_display_ic_expect = true;
	status_display_ic_width = 0;
	status_display_ic_width_expect = 1280;
	status_display_ic_height = 0;
	status_display_ic_height_expect = 720;

	/* ACTION:電源ON */

	PrintMsg("After turn on rear display, press enter.");
	std::cin.get();

	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_display_ic_expect == status_display_ic &&
																							   status_display_ic_width_expect == status_display_ic_width &&
																							   status_display_ic_height_expect == status_display_ic_height); 
																							});
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.display.instrumentcluster", CallbackDisplayIc));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.display.instrumentcluster.width", CallbackDisplayIcWidth));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.display.instrumentcluster.height", CallbackDisplayIcHeight));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));	
}

