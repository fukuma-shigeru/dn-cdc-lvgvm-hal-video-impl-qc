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
static int32_t vpath_result;
static cockpit::hal::video_hal::CtlObj Obj;

class VideoHAL_Camera_Vpath: public::testing::Test
{
protected:
	static void Callback_VpathCameraStatusCurrent();
	virtual void SetUp();
	virtual void TearDown();
protected:
	int32_t screen_width;
	int32_t screen_height;
};

/*** Callback ***/
void VideoHAL_Camera_Vpath::Callback_VpathCameraStatusCurrent()
{
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);

		/* Check camera vpath switching result */
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.camera.status.current", &vpath_result, sizeof(vpath_result), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_VPATH_STS_SUCCESS, vpath_result);
	}

	cond_sync.notify_one();
}

void VideoHAL_Camera_Vpath::SetUp()
{
	struct ilmScreenProperties screenProp;

	/* Get the screen properties */
	memset(&screenProp, 0x00, sizeof(struct ilmScreenProperties));
	get_screen_properties(VHAL_CT_FRONT_ID, &screenProp);

	/* Set width and height of front screen */
	screen_width  = screenProp.screenWidth;
	screen_height = screenProp.screenHeight;

	/* just in case */
	kill_test_video_client();

	set_hmi_visibility(false);
}

void VideoHAL_Camera_Vpath::TearDown()
{
	kill_test_video_client();

	set_hmi_visibility(true);
}

//
// Googletest TestCase
//
TEST_F(VideoHAL_Camera_Vpath, Lv1Normal006)
{
//	cockpit::hal::video_hal::CtlObj Obj={0, NULL};
	std::string vpath_str;
	int32_t set_x = 0;
	int32_t set_y = 0;
	int32_t set_w = 0;
	int32_t set_h = 0;
	int32_t get_x = 0;
	int32_t get_y = 0;
	int32_t get_w = 0;
	int32_t get_h = 0;
	bool set_flag = false;
	bool get_flag = false;
	bool intime = false;

	if (false == get_exec_camera())
	{
		EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Test No.6 all Skip(CAMERA)\n"));
		return;
	}

	/* Halcp Indication */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Test No.6 Start\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Overview : Camera Path Test\n"));

	/* Check screen size */
	EXPECT_GT(screen_width, 0);
	EXPECT_GT(screen_height, 0);

	/* VideoHAL Initialize */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(Obj));

	/* clear vpath first just in case */
	vpath_str = "";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.camera.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	/* ========== Test Start ========== */
	/* Halcp WaitInputAnyKey */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] Make sure that a camera device is connected.\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTANYKEY("[CertificationTest][Info] Please Input Any Key For Continue Test:"));

	/* Front video visibility setting(invisible) */
	set_flag = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* clear vpath first just in case */
	vpath_str = "";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* start test video client which makes the CARPLAY surface */
	exec_test_video_client(VHAL_CT_IVIID_CARPLAY, "sample_img_CARPLAY.png");

	/* Update vpath front */
	vpath_str = "CARPLAY";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* Front video visibility setting(visible) */
	set_flag = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the sample CARPLAY surface displayed on the front display ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* BEV step3 CAMERAを有効とする */
	system("/opt/dc-ivi-pf/bin/video_hal_control -V -W vhal.setting.control.connected.camera:num=11");
	system("echo -en '\\x01\\x00\\x00\\x00' > /run/arene/vehicle_fs/var/system/camera/display/status");
	system("echo -en '\\x01\\x00\\x00\\x00' > /run/arene/vehicle_fs/var/vehicle/signal/ig;");

	/* ----- Camera Path Test ----- */
	/* Check vpath camera availability */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.camera.status.available?vpath=CAMERA", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(true, get_flag);

	/* VideoHAL Register callback function */
	vpath_result = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(Obj, "vpath.camera.status.current", Callback_VpathCameraStatusCurrent));

	/* Update vpath camera */
	vpath_str = "CAMERA";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.camera.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* wait callback */
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(VHAL_CT_TIMEOUT_SECONDS), [] { return (VIDEO_HAL_VPATH_STS_SUCCESS == vpath_result); });
	EXPECT_EQ(true, intime);

	/* ClearCallback() */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(Obj, "vpath.camera.status.current", Callback_VpathCameraStatusCurrent));

	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Vpath Camera Change Complete \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Once the front display has become MUTE, is the camera surface displayed ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* ----- Camera Output Size Test ----- */
	/* change output size to be fullscreen size */
	set_x = 0,
	set_y = 0,
	set_w = screen_width,
	set_h = screen_height;
	set_flag = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.camera.control.output.x",      &set_x, sizeof(set_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.camera.control.output.y",      &set_y, sizeof(set_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.camera.control.output.width",  &set_w, sizeof(set_w), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.camera.control.output.height", &set_h, sizeof(set_h), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.camera.control.output.on", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.camera.control.output.on", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, get_flag);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.camera.status.output.x",      &get_x, sizeof(get_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.camera.status.output.y",      &get_y, sizeof(get_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.camera.status.output.width",  &get_w, sizeof(get_w), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.camera.status.output.height", &get_h, sizeof(get_h), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_x, get_x);
	EXPECT_EQ(set_y, get_y);
	EXPECT_EQ(set_w, get_w);
	EXPECT_EQ(set_h, get_h);

	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Camera Output Change(Fullscreen) Complete \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the camera surface displayed in full screen ? ('y' or 'Y' is OK, Others are NG):", nullptr));
	wait_msec(3000);

	/* change output size to be smaller square size */
	set_x = screen_width / 2;
	set_y = screen_height / 6;
	set_w = screen_width / 2 - 50;
	set_h = screen_height / 2;
	set_flag = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.camera.control.output.x",      &set_x, sizeof(set_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.camera.control.output.y",      &set_y, sizeof(set_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.camera.control.output.width",  &set_w, sizeof(set_w), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.camera.control.output.height", &set_h, sizeof(set_h), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.camera.control.output.on", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.camera.control.output.on", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, get_flag);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.camera.status.output.x",      &get_x, sizeof(get_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.camera.status.output.y",      &get_y, sizeof(get_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.camera.status.output.width",  &get_w, sizeof(get_w), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.camera.status.output.height", &get_h, sizeof(get_h), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_x, get_x);
	EXPECT_EQ(set_y, get_y);
	EXPECT_EQ(set_w, get_w);
	EXPECT_EQ(set_h, get_h);

	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Camera Output Change(Smaller rectangle) Complete \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the camera surface displayed in a smaller rectangle ? ('y' or 'Y' is OK, Others are NG):", nullptr));
	wait_msec(2000);

	/* reset output size */
	set_x = 0;
	set_y = 0;
	set_w = screen_width;
	set_h = screen_height;
	set_flag = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.camera.control.output.x",      &set_x, sizeof(set_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.camera.control.output.y",      &set_y, sizeof(set_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.camera.control.output.width",  &set_w, sizeof(set_w), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.camera.control.output.height", &set_h, sizeof(set_h), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.camera.control.output.on", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* clear vpath */
	vpath_str = "";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.camera.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Once the front display has become MUTE, is the sample CARPLAY surface displayed on the front display ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* ========== Test End ========== */

	/* BEV step3 CAMERAを無効とする */
	system("echo -en '\\x00\\x00\\x00\\x00' > /run/arene/vehicle_fs/var/system/camera/display/status");
	system("echo -en '\\x00\\x00\\x00\\x00' > /run/arene/vehicle_fs/var/vehicle/signal/ig");
	system("/opt/dc-ivi-pf/bin/video_hal_control -V -W vhal.setting.control.connected.camera:num=0");

	/* Front video visibility setting(invisible) */
	set_flag = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* clear vpath first just in case */
	vpath_str = "";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* Deinit() */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(Obj));
}
