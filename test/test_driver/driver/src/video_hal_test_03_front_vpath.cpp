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
#include "vhal_log.h"

/*** define ***/

/*** prototype ***/

/*** variables ***/
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static bool vpath_available;
static int32_t vpath_result;
static cockpit::hal::video_hal::CtlObj Obj;

class VideoHAL_Front_Vpath: public::testing::Test
{
protected:
	static void Callback_VpathFrontStatusAvailable();
	static void Callback_VpathFrontStatusCurrent();
	virtual void SetUp();
	virtual void TearDown();
protected:
	int32_t screen_width;
	int32_t screen_height;
};

/*** Callback ***/
void VideoHAL_Front_Vpath::Callback_VpathFrontStatusAvailable()
{
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);

		VHAL_LOGI("/*** call Callback_VpathFrontStatusAvailable");
		/* Check vpath front availability again */
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.available?vpath=CARPLAY", &vpath_available, sizeof(vpath_available), VIDEO_HAL_DATA_TYPE_BOOL));
	}

	cond_sync.notify_one();
}

void VideoHAL_Front_Vpath::Callback_VpathFrontStatusCurrent()
{
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);

		VHAL_LOGI("/*** call Callback_VpathFrontStatusCurrent");
		/* Check front vpath switching result */
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.current", &vpath_result, sizeof(vpath_result), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_VPATH_STS_SUCCESS, vpath_result);
	}

	cond_sync.notify_one();
}

void VideoHAL_Front_Vpath::SetUp()
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

void VideoHAL_Front_Vpath::TearDown()
{
	kill_test_video_client();

	set_hmi_visibility(true);
}

//
// Googletest TestCase
//
TEST_F(VideoHAL_Front_Vpath, Lv1Normal003)
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
	bool set_enable = false;
	bool get_enable = false;
	bool set_flag = false;
	bool get_flag = false;
	int32_t set_value;
	int32_t get_value;
	bool intime;

	/* Halcp Indication */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Test No.3 Start\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Overview : Front Video Path Test\n"));

	/* Check screen size */
	EXPECT_GT(screen_width, 0);
	EXPECT_GT(screen_height, 0);

	/* VideoHAL Initialize */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(Obj));
	VHAL_LOGI("/*** Test No.03 Start");

	/* Front video visibility setting(invisible) */
	set_flag = false;
	VHAL_LOGI("/*** phase01 set vpath.front.control.visible=%d", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* clear vpath first just in case */
	vpath_str = "";
	VHAL_LOGI("/*** phase02 set vpath.front.control.current=[%s]", vpath_str.c_str());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	/* ========== Test Start ========== */

	/* ----- Front Display Status Check ----- */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.display.front", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(true, get_flag);

	/* ----- Front Vpath Test ----- */
	VHAL_LOGI("/*** phase03 Front Vpath Test");
	/* Check vpath front availability */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.available?vpath=CARPLAY", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, get_flag);

	/* VideoHAL Register callback function */
	vpath_available = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(Obj, "vpath.front.status.available", Callback_VpathFrontStatusAvailable));

	/* start test video client which makes the CARPLAY surface */
	exec_test_video_client(VHAL_CT_IVIID_CARPLAY, "sample_img_CARPLAY.png");

	/* wait callback */
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(VHAL_CT_TIMEOUT_SECONDS), [] { return (true == vpath_available); });
	EXPECT_EQ(true, intime);

	/* ClearCallback() */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(Obj, "vpath.front.status.available", Callback_VpathFrontStatusAvailable));

	/* VideoHAL Register callback function */
	vpath_result = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(Obj, "vpath.front.status.current", Callback_VpathFrontStatusCurrent));

	/* Update vpath front */
	vpath_str = "CARPLAY";
	VHAL_LOGI("/*** phase04 set vpath.front.control.current=[%s]", vpath_str.c_str());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* wait callback */
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(VHAL_CT_TIMEOUT_SECONDS), [] { return (VIDEO_HAL_VPATH_STS_SUCCESS == vpath_result); });
	EXPECT_EQ(true, intime);

	/* ClearCallback() */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(Obj, "vpath.front.status.current", Callback_VpathFrontStatusCurrent));

	/* Front video visibility setting(visible) */
	set_flag = true;
	VHAL_LOGI("/*** phase05 set vpath.front.control.visible=%d", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Vpath Front Change Complete \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the sample CARPLAY surface displayed on the front display ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* ----- Front Output Size Test ----- */
	/* change output size to be fullscreen size */
	set_x = 0,
	set_y = 0,
	set_w = screen_width,
	set_h = screen_height;
	set_flag = true;
	VHAL_LOGI("/*** phase06 Front Output Change(Fullscreen)");
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.x",      &set_x, sizeof(set_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.y",      &set_y, sizeof(set_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.width",  &set_w, sizeof(set_w), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.height", &set_h, sizeof(set_h), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.on", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.control.output.on", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, get_flag);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.output.x",      &get_x, sizeof(get_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.output.y",      &get_y, sizeof(get_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.output.width",  &get_w, sizeof(get_w), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.output.height", &get_h, sizeof(get_h), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_x, get_x);
	EXPECT_EQ(set_y, get_y);
	EXPECT_EQ(set_w, get_w);
	EXPECT_EQ(set_h, get_h);

	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Front Output Change(Fullscreen) Complete \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the client surface displayed in full screen ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* change output size to be smaller square size */
	VHAL_LOGI("/*** phase07 Front Output Change(Smaller Rectangle)");
	set_x = screen_width / 2;
	set_y = screen_height / 6;
	set_w = screen_width / 2 - 50;
	set_h = screen_height / 2;
	set_flag = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.x",      &set_x, sizeof(set_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.y",      &set_y, sizeof(set_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.width",  &set_w, sizeof(set_w), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.height", &set_h, sizeof(set_h), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.on", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.control.output.on", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, get_flag);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.output.x",      &get_x, sizeof(get_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.output.y",      &get_y, sizeof(get_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.output.width",  &get_w, sizeof(get_w), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.output.height", &get_h, sizeof(get_h), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_x, get_x);
	EXPECT_EQ(set_y, get_y);
	EXPECT_EQ(set_w, get_w);
	EXPECT_EQ(set_h, get_h);

	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Front Output Change(Smaller Rectangle) Complete \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the client surface displayed in a smaller rectangle ? ('y' or 'Y' is OK, Others are NG):", nullptr));
	/* ----- Front Clipping Size Test ----- */
	VHAL_LOGI("/*** phase08 Front Clipping Size Change(Enable)");
	/* The sample CARPLAY image size is 800x480 */
	/* clipping on */
	set_x = 100;
	set_y = 50;
	set_w = 800 - 200;
	set_h = 480 - 100;
	set_enable = true;
	set_flag = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.clipping.x",      &set_x, sizeof(set_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.clipping.y",      &set_y, sizeof(set_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.clipping.width",  &set_w, sizeof(set_w), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.clipping.height", &set_h, sizeof(set_h), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.clipping.enable", &set_enable, sizeof(set_enable), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.clipping.on",     &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.control.clipping.on", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, get_flag);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.clipping.x",      &get_x, sizeof(get_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.clipping.y",      &get_y, sizeof(get_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.clipping.width",  &get_w, sizeof(get_w), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.clipping.height", &get_h, sizeof(get_h), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.clipping.enable", &get_enable, sizeof(get_enable), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(set_x, get_x);
	EXPECT_EQ(set_y, get_y);
	EXPECT_EQ(set_w, get_w);
	EXPECT_EQ(set_h, get_h);
	EXPECT_EQ(set_enable, get_enable);

	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Front Clipping Size Change(Enable) Complete \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the client surface displayed with clipping ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* clipping off */
	VHAL_LOGI("/*** phase09 Front Clipping Size Change(Disable)");
	set_enable = false;
	set_flag = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.clipping.enable", &set_enable, sizeof(set_enable), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.clipping.on",     &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.control.clipping.on", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, get_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.clipping.enable", &get_enable, sizeof(get_enable), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(set_enable, get_enable);

	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Front Clipping Size Change(Disable) Complete \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the client surface displayed without clipping ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* ----- Front Visibility Test ----- */
	VHAL_LOGI("/*** phase10 Front Visibility Change(Invisible)");
	/* change surface invisible */
	set_flag = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.visible", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(set_flag, get_flag);

	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Front Visibility Change(Invisible) Complete \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the client surface invisible ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* change surface visible */
	VHAL_LOGI("/*** phase11 Front Visibility Change(Visible)");
	set_flag = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.visible", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(set_flag, get_flag);

	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Front Visibility Change(Visible) Complete \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the client surface visible ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* ----- Front Opacity Test ----- */
	/* change surface opacity to 50 */
	VHAL_LOGI("/*** phase12 Front Opacity Change(50%)");
	set_value = 50;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.opacity", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.opacity", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);

	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Front Opacity Change(50%) Complete \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the client surface translucent ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* change surface opacity to 0 */
	VHAL_LOGI("/*** phase13 Front Opacity Change(0%)");
	set_value = 0;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.opacity", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.opacity", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);

	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Front Opacity Change(0%) Complete \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the client surface transparent ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* change surface opacity to 100 */
	VHAL_LOGI("/*** phase14 Front Opacity Change(100%)");
	set_value = 100;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.opacity", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.opacity", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);

	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Front Opacity Change(100%) Complete \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the client surface transparent ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* ----- Front Widemode Test ----- */
	/* Front video visibility setting(invisible) */
	VHAL_LOGI("/*** phase15 Front Widemode Test");
	set_flag = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	/* clear vpath first just in case */
	vpath_str = "";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* Change CARPLAY surface */
	kill_test_video_client();

	/* Check vpath front availability */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.available?vpath=CARPLAY", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, get_flag);

	/* VideoHAL Register callback function */
	vpath_available = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(Obj, "vpath.front.status.available", Callback_VpathFrontStatusAvailable));

	/* start test video client which makes the CARPLAY surface */
	exec_test_video_client(VHAL_CT_IVIID_CARPLAY, "sample_img_CARPLAY_square.png");

	/* wait callback */
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(VHAL_CT_TIMEOUT_SECONDS), [] { return (true == vpath_available); });
	EXPECT_EQ(true, intime);

	/* ClearCallback() */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(Obj, "vpath.front.status.available", Callback_VpathFrontStatusAvailable));

	/* VideoHAL Register callback function */
	vpath_result = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(Obj, "vpath.front.status.current", Callback_VpathFrontStatusCurrent));

	/* Update vpath front */
	vpath_str = "CARPLAY";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* wait callback */
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(VHAL_CT_TIMEOUT_SECONDS), [] { return (VIDEO_HAL_VPATH_STS_SUCCESS == vpath_result); });
	EXPECT_EQ(true, intime);

	/* ClearCallback() */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(Obj, "vpath.front.status.current", Callback_VpathFrontStatusCurrent));

	set_x = (screen_width - 600) / 2;
	set_y = (screen_height - 150) / 2;
	set_w = 600;
	set_h = 150;
	set_flag = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.x",      &set_x, sizeof(set_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.y",      &set_y, sizeof(set_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.width",  &set_w, sizeof(set_w), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.height", &set_h, sizeof(set_h), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.on", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* change widemode to stretched */
	VHAL_LOGI("/*** phase16 Front Widemode Change(Stretched)");
	set_value = VIDEO_HAL_WIDE_MODE_STRETCHED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.widemode", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.widemode", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);

	/* Front video visibility setting(visible) */
	set_flag = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Front Widemode Change(Stretched) Complete \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the client surface displayed in the stretched mode ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* change widemode to zoomed */
	VHAL_LOGI("/*** phase17 Front Widemode Change(Zoomed)");
	set_value = VIDEO_HAL_WIDE_MODE_ZOOMED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.widemode", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.widemode", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);

	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Front Widemode Change(Zoomed) Complete \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the client surface displayed in the zoomed mode ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* change widemode to normal */
	VHAL_LOGI("/*** phase18 Front Widemode Change(Normal)");
	set_value = VIDEO_HAL_WIDE_MODE_NORMAL;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.widemode", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.widemode", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);

	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Front Widemode Change(Normal) Complete \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the client surface displayed in the normal mode ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* ========== Test End ========== */

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
	VHAL_LOGI("/*** Test No.03 Exit");
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(Obj));
}
