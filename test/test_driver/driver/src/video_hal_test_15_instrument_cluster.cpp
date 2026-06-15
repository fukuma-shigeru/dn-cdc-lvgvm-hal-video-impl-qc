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
static bool vpath_available;
static uint32_t vpath_result;
static cockpit::hal::video_hal::CtlObj Obj;

class VideoHAL_Instrument_Cluster: public::testing::Test
{
protected:
	static void Callback_VpathIcStatusAvailable();
	static void Callback_VpathIcStatusCurrent();
	virtual void SetUp();
	virtual void TearDown();
protected:
	int32_t screen_width;
	int32_t screen_height;
};

/*** Callback ***/
void VideoHAL_Instrument_Cluster::Callback_VpathIcStatusAvailable()
{
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);

		/* Check vpath IC availability again */
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.instrumentcluster.status.available?vpath=MAP", &vpath_available, sizeof(vpath_available), VIDEO_HAL_DATA_TYPE_BOOL));
	}

	cond_sync.notify_one();
}

void VideoHAL_Instrument_Cluster::Callback_VpathIcStatusCurrent()
{
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);

		/* Check Instrument Cluster vpath switching result */
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.instrumentcluster.status.current", &vpath_result, sizeof(vpath_result), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_VPATH_STS_SUCCESS, vpath_result);
	}

	cond_sync.notify_one();
}

void VideoHAL_Instrument_Cluster::SetUp()
{
	struct ilmScreenProperties screenProp;

	/* Get the screen properties */
	memset(&screenProp, 0x00, sizeof(struct ilmScreenProperties));
	get_screen_properties(VHAL_CT_IC_ID, &screenProp);

	/* Set width and height of Instrument Cluster screen */
	screen_width  = screenProp.screenWidth;
	screen_height = screenProp.screenHeight;

	/* just in case */
	kill_test_video_client();

	set_hmi_visibility(false);
}

void VideoHAL_Instrument_Cluster::TearDown()
{
	kill_test_video_client();

	set_hmi_visibility(true);
}

//
// Googletest TestCase
//
TEST_F(VideoHAL_Instrument_Cluster, Lv1Normal015)
{
//	cockpit::hal::video_hal::CtlObj Obj={0, NULL};
	std::string vpath_str;
	uint32_t set_x = 0;
	uint32_t set_y = 0;
	uint32_t set_w = 0;
	uint32_t set_h = 0;
	uint32_t get_x = 0;
	uint32_t get_y = 0;
	uint32_t get_w = 0;
	uint32_t get_h = 0;
	bool set_enable = false;
	bool get_enable = false;
	bool set_flag = false;
	bool get_flag = false;
	uint32_t set_value = 0;
	uint32_t get_value = 0;
	bool intime = false;

	if (false == get_exec_ic())
	{
		EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Test No.15 all Skip(Instrument Cluster)\n"));
		return;
	}

	/* Halcp Indication */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Test No.15 Start\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Overview : Instrument_Cluster Video Path Test\n"));

	/* Check screen size */
	EXPECT_GT(screen_width, 0);
	EXPECT_GT(screen_height, 0);

	/* VideoHAL Initialize */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(Obj));

	/* Instrumentcluster video visibility setting(invisible) */
	set_flag = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* clear Instrument Cluster first just in case */
	vpath_str = "";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	/* ========== Test Start ========== */

	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] The InstrumentCluster display must be ready to check the images transferred from the IVI side.\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTANYKEY("[CertificationTest][Info] Please Input Any Key For Continue Test:"));

	/* ----- Instrument Cluster Display Status Check ----- */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.display.instrumentcluster", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(true, get_flag);

	/* ----- Instrument Cluster Vpath Test ----- */
	/* Check vpath Instrument Cluster availability */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.instrumentcluster.status.available?vpath=MAP", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, get_flag);

	/* VideoHAL Register callback function */
	vpath_available = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(Obj, "vpath.instrumentcluster.status.available", Callback_VpathIcStatusAvailable));

	/* start test video client which makes the MAP surface */
	exec_test_video_client(VHAL_CT_IVIID_MAP, "sample_img_MAP.png");

	/* wait callback */
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(VHAL_CT_TIMEOUT_SECONDS), [] { return vpath_available; });
	EXPECT_EQ(true, intime);

	/* ClearCallback() */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(Obj, "vpath.instrumentcluster.status.available", Callback_VpathIcStatusAvailable));

	/* VideoHAL Register callback function */
	vpath_result = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(Obj, "vpath.instrumentcluster.status.current", Callback_VpathIcStatusCurrent));

	/* Update vpath Instrument Cluster */
	vpath_str = "MAP";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* wait callback */
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(VHAL_CT_TIMEOUT_SECONDS), [] { return (VIDEO_HAL_VPATH_STS_SUCCESS == vpath_result); });
	EXPECT_EQ(true, intime);

	/* ClearCallback() */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(Obj, "vpath.instrumentcluster.status.current", Callback_VpathIcStatusCurrent));

	/* Instrumentcluster video visibility setting(visible) */
	set_flag = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Vpath Instrument Cluster Change Complete \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the sample MAP surface displayed on the Instrument Cluster display ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* ----- Instrument Cluster Output Size Test ----- */
	/* change output size to be fullscreen size */
	set_x = 0,
	set_y = 0,
	set_w = screen_width,
	set_h = screen_height;
	set_flag = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.output.x",      &set_x, sizeof(set_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.output.y",      &set_y, sizeof(set_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.output.width",  &set_w, sizeof(set_w), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.output.height", &set_h, sizeof(set_h), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.output.on",     &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.instrumentcluster.control.output.on", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, get_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.instrumentcluster.status.output.x",      &get_x, sizeof(get_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.instrumentcluster.status.output.y",      &get_y, sizeof(get_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.instrumentcluster.status.output.width",  &get_w, sizeof(get_w), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.instrumentcluster.status.output.height", &get_h, sizeof(get_h), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_x, get_x);
	EXPECT_EQ(set_y, get_y);
	EXPECT_EQ(set_w, get_w);
	EXPECT_EQ(set_h, get_h);

	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Instrument Cluster Output Change(Fullscreen) Complete \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the client surface displayed in full screen ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* change output size to be smaller square size */
	set_x = screen_width / 2;
	set_y = screen_height / 6;
	set_w = screen_width / 2 - 50;
	set_h = screen_height / 2;
	set_flag = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.output.x",      &set_x, sizeof(set_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.output.y",      &set_y, sizeof(set_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.output.width",  &set_w, sizeof(set_w), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.output.height", &set_h, sizeof(set_h), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.output.on",     &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.instrumentcluster.control.output.on", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, get_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.instrumentcluster.status.output.x",      &get_x, sizeof(get_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.instrumentcluster.status.output.y",      &get_y, sizeof(get_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.instrumentcluster.status.output.width",  &get_w, sizeof(get_w), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.instrumentcluster.status.output.height", &get_h, sizeof(get_h), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_x, get_x);
	EXPECT_EQ(set_y, get_y);
	EXPECT_EQ(set_w, get_w);
	EXPECT_EQ(set_h, get_h);

	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Instrument Cluster Output Change(Smaller Rectangle) Complete \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the client surface displayed in a smaller rectangle ? ('y' or 'Y' is OK, Others are NG):", nullptr));
	/* ----- Instrument Cluster Clipping Size Test ----- */
	/* The sample MAP image size is 800x480 */
	/* clipping on */
	set_x = 100;
	set_y = 50;
	set_w = 800 - 200;
	set_h = 480 - 100;
	set_enable = true;
	set_flag = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.clipping.x",      &set_x, sizeof(set_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.clipping.y",      &set_y, sizeof(set_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.clipping.width",  &set_w, sizeof(set_w), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.clipping.height", &set_h, sizeof(set_h), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.clipping.enable", &set_enable, sizeof(set_enable), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.clipping.on",     &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.instrumentcluster.control.clipping.on", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, get_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.instrumentcluster.status.clipping.x",      &get_x, sizeof(get_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.instrumentcluster.status.clipping.y",      &get_y, sizeof(get_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.instrumentcluster.status.clipping.width",  &get_w, sizeof(get_w), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.instrumentcluster.status.clipping.height", &get_h, sizeof(get_h), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.instrumentcluster.status.clipping.enable", &get_enable, sizeof(get_enable), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(set_x, get_x);
	EXPECT_EQ(set_y, get_y);
	EXPECT_EQ(set_w, get_w);
	EXPECT_EQ(set_h, get_h);
	EXPECT_EQ(set_enable, get_enable);

	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Instrument Cluster Clipping Size Change(Enable) Complete \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the client surface displayed with clipping ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* clipping off */
	set_enable = false;
	set_flag = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.clipping.enable", &set_enable, sizeof(set_enable), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.clipping.on",     &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.instrumentcluster.control.clipping.on", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, get_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.instrumentcluster.status.clipping.enable", &get_enable, sizeof(get_enable), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(set_enable, get_enable);

	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Instrument Cluster Clipping Size Change(Disable) Complete \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the client surface displayed without clipping ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* -----Instrument Cluster Visibility Test ----- */
	set_flag = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.instrumentcluster.status.visible", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(set_flag, get_flag);
	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Instrument Cluster Visibility Change(Invisible) Complete \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the client surface invisible ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* change surface visible */
	set_flag = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.instrumentcluster.status.visible", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(set_flag, get_flag);
	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Instrument Cluster Visibility Change(Visible) Complete \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the client surface visible ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* ----- Instrument Cluster Opacity Test ----- */
	/* change surface opacity to 50 */
	set_value = 50;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.opacity", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.instrumentcluster.status.opacity", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);
	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Instrument Cluster Opacity Change(50%) Complete \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the client surface translucent ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* change surface opacity to 0 */
	set_value = 0;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.opacity", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.instrumentcluster.status.opacity", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);
	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Instrument Cluster Opacity Change(0%) Complete \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the client surface transparent ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* change surface opacity to 100 */
	set_value = 100;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.opacity", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.instrumentcluster.status.opacity", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);

	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Instrument Cluster Opacity Change(100%) Complete \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the client surface opaque ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* ========== Test End ========== */
	/* Instrumentcluster video visibility setting(invisible) */
	set_flag = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* reset output size */
	set_x = 0;
	set_y = 0;
	set_w = screen_width;
	set_h = screen_height;
	set_flag = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.output.x",      &set_x, sizeof(set_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.output.y",      &set_y, sizeof(set_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.output.width",  &set_w, sizeof(set_w), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.output.height", &set_h, sizeof(set_h), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.output.on", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* clear vpath */
	vpath_str = "";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* Deinit() */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(Obj));
}
