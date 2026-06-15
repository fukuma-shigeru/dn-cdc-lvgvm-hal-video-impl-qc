/*
 * @copyright Copyright (c) 2022 Woven Alpha, Inc
 */

/*** inclue ***/
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <gtest/gtest.h>

#include "ilm/ilm_common.h"
#include "ilm/ilm_control.h"

#include "video_hal_ct_common.h"
#include "video_hal.h"

/*** define ***/

/*** prototype ***/

/*** variables ***/
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static int32_t connected_hdmi;
static int32_t vpath_result;
static cockpit::hal::video_hal::CtlObj Obj;

class VideoHAL_HDMI: public::testing::Test
{
protected:
	static void Callback_ConnectedHdmi();
	static void Callback_DisconnectedHdmi();
	static void Callback_VpathFrontStatusCurrent();
	virtual void SetUp();
	virtual void TearDown();
protected:
	int32_t screen_width;
	int32_t screen_height;
};

/*** Callback ***/
void VideoHAL_HDMI::Callback_ConnectedHdmi()
{
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);

		/* Check hdmi connected status again */
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.connected.hdmi", &connected_hdmi, sizeof(connected_hdmi), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_CONNECTED_HDMI_READY, connected_hdmi);
	}

	cond_sync.notify_one();
}

void VideoHAL_HDMI::Callback_DisconnectedHdmi()
{
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);

		/* Check hdmi connected status again */
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.connected.hdmi", &connected_hdmi, sizeof(connected_hdmi), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_CONNECTED_HDMI_NONE, connected_hdmi);
	}

	cond_sync.notify_one();
}

void VideoHAL_HDMI::Callback_VpathFrontStatusCurrent()
{
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);

		/* Check front vpath switching result */
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.current", &vpath_result, sizeof(vpath_result), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_VPATH_STS_SUCCESS, vpath_result);
	}

	cond_sync.notify_one();
}

void VideoHAL_HDMI::SetUp()
{
	struct ilmScreenProperties screenProp;

	/* Get the screen properties */
	memset(&screenProp, 0x00, sizeof(struct ilmScreenProperties));
	get_screen_properties(VHAL_CT_FRONT_ID, &screenProp);

	/* Set width and height of front screen */
	screen_width  = screenProp.screenWidth;
	screen_height = screenProp.screenHeight;

	set_hmi_visibility(false);
}

void VideoHAL_HDMI::TearDown()
{
	set_hmi_visibility(true);
}

//
// Googletest TestCase
//
TEST_F(VideoHAL_HDMI, Lv1Normal004)
{
//	cockpit::hal::video_hal::CtlObj Obj={0, NULL};
	std::string vpath_str;
	int32_t set_x = 0;
	int32_t set_y = 0;
	int32_t set_w = 0;
	int32_t set_h = 0;
	bool set_flag = false;
	int32_t get_value = 0;
	bool intime = false;

	if (false == get_exec_hdmi())
	{
		EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Test No.4 all Skip(HDMI)\n"));
		return;
	}

	/* Halcp Indication */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Test No.4 Start\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Overview : HDMI Test\n"));

	/* Check screen size */
	EXPECT_GT(screen_width, 0);
	EXPECT_GT(screen_height, 0);

	/* VideoHAL Initialize */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(Obj));

	/* Front video visibility setting(invisible) */
	set_flag = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* clear vpath first just in case */
	vpath_str = "";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	/* ========== Test Start ========== */
	/* Halcp WaitInputAnyKey */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] Make sure that any HDMI device is not connected yet.\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTANYKEY("[CertificationTest][Info] Please Input Any Key For Continue Test:"));

	/* ----- Front Vpath Test ----- */
	/* Check HDMI connection */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.connected.hdmi", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_CONNECTED_HDMI_NONE, get_value);

	/* VideoHAL Register callback function */
	connected_hdmi = VIDEO_HAL_CONNECTED_HDMI_NONE;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(Obj, "vhal.setting.status.connected.hdmi", Callback_ConnectedHdmi));

	/* Connect the HDMI device */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] Connect the HDMI device.\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTANYKEY("[CertificationTest][Info] Please Input Any Key For Continue Test:"));

	/* BEV step3 HDMIを有効とする */
	system("sudo -u default /opt/arene/test/brysh dbwrite /db/sys_info/conn_info/hdmi.dbf -d 0");

	/* wait callback */
//	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(VHAL_CT_TIMEOUT_SECONDS), [] { return (VIDEO_HAL_CONNECTED_HDMI_READY == connected_hdmi); });
//	EXPECT_EQ(true, intime);

	//追記 コマンド
	system("sudo -u default /opt/arene/test/brysh dbwrite /db/sys_info/conn_info/hdmi_info/video_format.dbf -d 1;");

	/* wait callback */
//	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(VHAL_CT_TIMEOUT_SECONDS), [] { return (VIDEO_HAL_VPATH_STS_SUCCESS == vpath_result); });
//	EXPECT_EQ(true, intime);

	/* ClearCallback() */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(Obj, "vhal.setting.status.connected.hdmi", Callback_ConnectedHdmi));

	/* VideoHAL Register callback function */
	vpath_result = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(Obj, "vpath.front.status.current", Callback_VpathFrontStatusCurrent));

	/* Update vpath front */
	vpath_str = "HDMI";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	/* wait callback */
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(VHAL_CT_TIMEOUT_SECONDS), [] { return (VIDEO_HAL_VPATH_STS_SUCCESS == vpath_result); });
	EXPECT_EQ(true, intime);

	/* Front video visibility setting(visible) */
	set_flag = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* also set surface size to full screen */
	set_x = 0,
	set_y = 0,
	set_w = screen_width,
	set_h = screen_height;
	set_flag = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.x",      &set_x, sizeof(set_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.y",      &set_y, sizeof(set_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.width",  &set_w, sizeof(set_w), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.height", &set_h, sizeof(set_h), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.on", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* ClearCallback() */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(Obj, "vpath.front.status.current", Callback_VpathFrontStatusCurrent));

	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Vpath Front Change(HDMI) Complete \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the HDMI surface displayed on the front display ? ('y' or 'Y' is OK, Others are NG):", nullptr));
	wait_msec(3000);
	
	/* Disconnect the HDMI device */
	/* VideoHAL Register callback function */
	connected_hdmi = VIDEO_HAL_CONNECTED_HDMI_READY;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(Obj, "vhal.setting.status.connected.hdmi", Callback_DisconnectedHdmi));

	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] Disconnect the HDMI device.\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTANYKEY("[CertificationTest][Info] Please Input Any Key For Continue Test:"));

	/* BEV step3 HDMIを無効とする */
	system("sudo -u default /opt/arene/test/brysh dbwrite /db/sys_info/conn_info/hdmi.dbf -d 1");

	/* wait callback */
//	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(VHAL_CT_TIMEOUT_SECONDS), [] { return (VIDEO_HAL_CONNECTED_HDMI_NONE == connected_hdmi); });
//	EXPECT_EQ(true, intime);

	/* ClearCallback() */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(Obj, "vhal.setting.status.connected.hdmi", Callback_DisconnectedHdmi));

	set_flag = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* clear vpath first just in case */
	vpath_str = "";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Did the HDMI surface disappear ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* VideoHAL Register callback function */
	connected_hdmi = VIDEO_HAL_CONNECTED_HDMI_NONE;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(Obj, "vhal.setting.status.connected.hdmi", Callback_ConnectedHdmi));

	/* Reconnect the HDMI device */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] Reconnect the HDMI device.\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTANYKEY("[CertificationTest][Info] Please Input Any Key For Continue Test:"));

	/* BEV step3 HDMIを有効とする */
	system("sudo -u default /opt/arene/test/brysh dbwrite /db/sys_info/conn_info/hdmi.dbf -d 0");

	/* wait callback */
//	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(VHAL_CT_TIMEOUT_SECONDS), [] { return (VIDEO_HAL_CONNECTED_HDMI_READY == connected_hdmi); });
//	EXPECT_EQ(true, intime);

	/* ClearCallback() */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(Obj, "vhal.setting.status.connected.hdmi", Callback_ConnectedHdmi));

	/* Update vpath front */
	vpath_str = "HDMI";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* Front video visibility setting(visible) */
	set_flag = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the HDMI surface displayed again ? ('y' or 'Y' is OK, Others are NG):", nullptr));
	wait_msec(2000);

	/* ========== Test End ========== */

	/* BEV step3 HDMIを無効とする */
	system("sudo -u default /opt/arene/test/brysh dbwrite /db/sys_info/conn_info/hdmi.dbf -d 1");
	system("sudo -u default /opt/arene/test/brysh dbwrite /db/sys_info/conn_info/hdmi_info/video_format.dbf -d 0");

	/* Front video visibility setting(invisible) */
	set_flag = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* reset output size */
	set_x = 0;
	set_y = 0;
	set_w = screen_width;
	set_h = screen_height;
	set_flag = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.x",      &set_x, sizeof(set_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.y",      &set_y, sizeof(set_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.width",  &set_w, sizeof(set_w), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.height", &set_h, sizeof(set_h), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.on", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* clear vpath */
	vpath_str = "";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* Deinit() */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(Obj));
}
