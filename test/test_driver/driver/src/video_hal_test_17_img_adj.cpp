/*
 * @copyright Copyright (c) 2022 Woven Alpha, Inc.
 */

/*** include ***/
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
#define _SET_VPATH_AREA(obj, pos)	\
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.x",      &pos[0], sizeof(pos[0]), VIDEO_HAL_DATA_TYPE_NUM));	\
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.y",      &pos[1], sizeof(pos[1]), VIDEO_HAL_DATA_TYPE_NUM));	\
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.width",  &pos[2], sizeof(pos[2]), VIDEO_HAL_DATA_TYPE_NUM));	\
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.height", &pos[3], sizeof(pos[3]), VIDEO_HAL_DATA_TYPE_NUM));
#define _GET_VPATH_AREA(obj, pos)	\
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.output.x",      &pos[0], sizeof(pos[0]), VIDEO_HAL_DATA_TYPE_NUM));	\
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.output.y",      &pos[1], sizeof(pos[1]), VIDEO_HAL_DATA_TYPE_NUM));	\
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.output.width",  &pos[2], sizeof(pos[2]), VIDEO_HAL_DATA_TYPE_NUM));	\
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.output.height", &pos[3], sizeof(pos[3]), VIDEO_HAL_DATA_TYPE_NUM));
#define _SET_HEACON_AREA(obj, pos)	\
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.x",      &pos[0], sizeof(pos[0]), VIDEO_HAL_DATA_TYPE_NUM));	\
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.y",      &pos[1], sizeof(pos[1]), VIDEO_HAL_DATA_TYPE_NUM));	\
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.width",  &pos[2], sizeof(pos[2]), VIDEO_HAL_DATA_TYPE_NUM));	\
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.height", &pos[3], sizeof(pos[3]), VIDEO_HAL_DATA_TYPE_NUM));
#define _GET_HEACON_AREA(obj, pos)	\
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "heacon.front.status.output.x",      &pos[0], sizeof(pos[0]), VIDEO_HAL_DATA_TYPE_NUM));	\
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "heacon.front.status.output.y",      &pos[1], sizeof(pos[1]), VIDEO_HAL_DATA_TYPE_NUM));	\
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "heacon.front.status.output.width",  &pos[2], sizeof(pos[2]), VIDEO_HAL_DATA_TYPE_NUM));	\
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "heacon.front.status.output.height", &pos[3], sizeof(pos[3]), VIDEO_HAL_DATA_TYPE_NUM));
#define _SET_POSITION(pos, x, y, w ,h)	\
		pos[0] = x;	\
		pos[1] = y;	\
		pos[2] = w;	\
		pos[3] = h;
#define _CHECK_POSITION(set, get)	\
		EXPECT_EQ(set[0], get[0]);	\
		EXPECT_EQ(set[1], get[1]);	\
		EXPECT_EQ(set[2], get[2]);	\
		EXPECT_EQ(set[3], get[3]);		

/*** prototype ***/

/*** variables ***/
static cockpit::hal::video_hal::CtlObj Obj;
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static bool vpath_available;
static uint32_t vpath_result;

/*** structure ***/
typedef struct
{
	uint32_t id;
	uint32_t brightness;
	uint32_t contrast;
} image_quality_str;
std::array<image_quality_str, 2> image_quality_array
{{
	{VIDEO_HAL_VSRC_ID_OTHER,            10, 10},
	{VIDEO_HAL_VSRC_ID_IMAGE_ADJUST_CAM, 63, 63}
}};

class VideoHAL_ImageAdjust: public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();
	static void Callback_VpathFrontStatusAvailable();
	static void Callback_VpathFrontStatusCurrent();
protected:
	int32_t screen_width;
	int32_t screen_height;
};

/*** Callback ***/
void VideoHAL_ImageAdjust::Callback_VpathFrontStatusAvailable()
{
	VHAL_LOGI("/*** called");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);

		/* Check vpath front availability again */
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(
			Obj,
			"vpath.front.status.available?vpath=CAMERA-IMG-ADJ",
			&vpath_available,
			sizeof(vpath_available),
			VIDEO_HAL_DATA_TYPE_BOOL
			));
	}
	cond_sync.notify_one();
}

void VideoHAL_ImageAdjust::Callback_VpathFrontStatusCurrent()
{
	VHAL_LOGI("/*** called");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);

		/* Check front vpath switching result */
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(
			Obj,
			"vpath.front.status.current",
			&vpath_result,
			sizeof(vpath_result),
			VIDEO_HAL_DATA_TYPE_NUM
			));
		EXPECT_EQ(VIDEO_HAL_VPATH_STS_SUCCESS, vpath_result);
	}
	cond_sync.notify_one();
}

void VideoHAL_ImageAdjust::SetUp()
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

	set_hmi_visibility(false);}

void VideoHAL_ImageAdjust::TearDown()
{
	kill_test_video_client();
}

//
// Googletest TestCase
//
TEST_F(VideoHAL_ImageAdjust, Lv1Normal017)
{
	uint32_t init_video_id_value = 0;
	uint32_t init_brightness = 0;
	uint32_t init_contrast = 0;
	std::vector<uint32_t> heacon_init_pos(4, 0);
	uint32_t set_value = 0;
	uint32_t get_value = 0;
	std::vector<uint32_t> set_vpath_pos(4, 0);
	std::vector<uint32_t> get_vpath_pos(4, 0);
	std::vector<uint32_t> set_heacon_pos(4, 0);
	std::vector<uint32_t> get_heacon_pos(4, 0);
	bool set_flag = false;
	bool get_flag = false;
	std::string vpath_str = "";
	bool intime = false;

	/* Halcp Indication */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Test No.17 Start\n"));
	if (false == get_exec_heacon())
	{
		EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Test No.17 Skip(Heacon area test)\n"));
	}
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Overview : Image adjust Test\n"));

	/* Check screen size */
	EXPECT_GT(screen_width, 0);
	EXPECT_GT(screen_height, 0);

	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	/* VideoHAL Initialize */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(Obj));
	VHAL_LOGI("/*** Test No.17 Start");

	/* Forced HMI image setting(unset request) */
	set_flag = false;
	VHAL_LOGI("/*** phase01 set vsrc.front.control.forced_hmi_img_adj=%d", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vsrc.front.control.forced_hmi_img_adj", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vsrc.front.status.forced_hmi_img_adj", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(get_flag, set_flag);

	/* Initial heacon area(invisible) */
	if (true == get_exec_heacon())
	{
		/* Output video: Current display of heacon area(Used to clean up) */
		_GET_HEACON_AREA(Obj, heacon_init_pos);
		/* Display start x, y, width, height position setting(invisible) */
		/* Output size(width, height) set by 0. */
		/*            out(vector),    x, y, w, h */
		_SET_POSITION(set_heacon_pos, 0, 0, 0, 0);
		_SET_HEACON_AREA(Obj, set_heacon_pos);
		set_flag = true;
		VHAL_LOGI("/*** phase02 set heacon.front.control.output.on=%d", set_flag);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "heacon.front.control.output.on", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "heacon.front.control.output.on", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(false, get_flag);	/* one-shot event */
	}

	/* Current video source(Used to clean up) */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vsrc.front.status.id", &init_video_id_value, sizeof(init_video_id_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vsrc.front.status.brightness.step", &init_brightness, sizeof(init_brightness), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vsrc.front.status.contrast.step", &init_contrast, sizeof(init_contrast), VIDEO_HAL_DATA_TYPE_NUM));
	VHAL_LOGI("/*** phase03 get vsrc.front.status id=%u brightness=%u contrast=%u", init_video_id_value, init_brightness, init_contrast);

	/* Video source setting(VIDEO_HAL_VSRC_ID_OTHER)            : Brightness step setting request(10) Contrast step setting request(10) */
	/* Video source setting(VIDEO_HAL_VSRC_ID_IMAGE_ADJUST_CAM) : Brightness step setting request(63) Contrast step setting request(63) */
	for (std::size_t i = 0; i < image_quality_array.size(); ++i)
	{
		set_value = image_quality_array[i].id;
		VHAL_LOGI("/*** [i=%zu] phase04A set vsrc.front.control.id=%u", i, set_value);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vsrc.front.control.id", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vsrc.front.status.id", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(get_value, set_value);
		set_value = image_quality_array[i].brightness;
		VHAL_LOGI("/*** [i=%zu] phase04B set vsrc.front.control.brightness.step=%u", i, set_value);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vsrc.front.control.brightness.step", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vsrc.front.status.brightness.step", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(get_value, set_value);
		set_value = image_quality_array[i].contrast;
		VHAL_LOGI("/*** [i=%zu] phase04C set vsrc.front.control.contrast.step=%u", i, set_value);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vsrc.front.control.contrast.step", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vsrc.front.status.contrast.step", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(get_value, set_value);
	}
	/* Current Video source id is "VIDEO_HAL_VSRC_ID_IMAGE_ADJUST_CAM". */
	/* Brightness step setting request(63) Contrast step setting request(63) */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] ------------------------------------------------------------------"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Video source ID(Operation Screen)        brightness:10 contrast:10"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Video source ID(Camera image adjustment) brightness:63 contrast:63"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] ------------------------------------------------------------------"));

	/* ========== Test Start ========== */

	/* Create surface of color bar */
	vpath_available = false;
	VHAL_LOGI("/*** phase05 regist vpath.front.status.available=%u with waiting", vpath_available);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(Obj, "vpath.front.status.available", Callback_VpathFrontStatusAvailable));
	exec_test_video_client(VHAL_CT_IVIID_CAMERA_IMG_ADJ, "sample_img_COLOR_BAR.png");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(VHAL_CT_TIMEOUT_SECONDS), [] { return (true == vpath_available); });
	EXPECT_EQ(true, intime);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(Obj, "vpath.front.status.available", Callback_VpathFrontStatusAvailable));

	/* Front seat Video Path name setting(Camera image quality adjustment mode) */
	vpath_result = VIDEO_HAL_VPATH_STS_FAILED;
	VHAL_LOGI("/*** phase06 regist vpath.front.status.current=%u with waiting", vpath_result);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(Obj, "vpath.front.status.current", Callback_VpathFrontStatusCurrent));
	vpath_str = "CAMERA-IMG-ADJ";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	/* wait callback */
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(VHAL_CT_TIMEOUT_SECONDS), [] { return (VIDEO_HAL_VPATH_STS_SUCCESS == vpath_result); });
	EXPECT_EQ(true, intime);
	/* ClearCallback() */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(Obj, "vpath.front.status.current", Callback_VpathFrontStatusCurrent));

	/* Front output size setting(Full screen) */
	/*            out(vector),   x, y,            w,             h */
	_SET_POSITION(set_vpath_pos, 0, 0, screen_width, screen_height);
	_SET_VPATH_AREA(Obj, set_vpath_pos);
	set_flag = true;
	VHAL_LOGI("/*** phase07 set vpath.front.control.output.on=%u", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.on", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.control.output.on", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, get_flag);	/* one-shot event */
	_GET_VPATH_AREA(Obj, get_vpath_pos);
	_CHECK_POSITION(set_vpath_pos, get_vpath_pos);
	/* Front video visibility setting(visible) */
	set_flag = true;
	VHAL_LOGI("/*** phase08 set vpath.front.control.visible=%u", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.visible", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(get_flag, set_flag);
	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Vpath Front Change Complete"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Is the sample CAMERA-IMG-ADJ(color bar) surface displayed on the front display ?"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Video source ID is 'Camera image adjustment'."));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("\n[CertificationTest][Info] Hit any key ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* Forced HMI image setting(setting request) */
	set_flag = true;
	VHAL_LOGI("/*** phase09 set vsrc.front.control.forced_hmi_img_adj=%u", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vsrc.front.control.forced_hmi_img_adj", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vsrc.front.status.forced_hmi_img_adj", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(get_flag, set_flag);
	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Check 'contrast', 'brightness' of the front display."));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Video source ID is 'Operation Screen'(Modified by Forced HMI image setting(true))."));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("\n[CertificationTest][Info] Hit any key ('y' or 'Y' is OK, Others are NG):", nullptr));
	/* Forced HMI image setting(unset request) */
	set_flag = false;
	VHAL_LOGI("/*** phase10 set vsrc.front.control.forced_hmi_img_adj=%u", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vsrc.front.control.forced_hmi_img_adj", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vsrc.front.status.forced_hmi_img_adj", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(get_flag, set_flag);
	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Check 'contrast', 'brightness' of the front display."));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Video source ID is 'Camera image adjustment'(Modified by Forced HMI image setting(false))."));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("\n[CertificationTest][Info] Hit any key ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* Check heacon area setting */
	if (true == get_exec_heacon())
	{
		/* Heacon area size setting(Show in lower half of screen) */
		/*            out(vector),    x,                   y,            w,                   h */
		_SET_POSITION(set_heacon_pos, 0, (screen_height / 2), screen_width, (screen_height / 2));
		_SET_HEACON_AREA(Obj, set_heacon_pos);
		set_flag = true;
		VHAL_LOGI("/*** phase11 set heacon.front.control.output.on=%u", set_flag);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "heacon.front.control.output.on", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "heacon.front.control.output.on", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(false, get_flag);	/* one-shot event */
		_GET_HEACON_AREA(Obj, get_heacon_pos);
		_CHECK_POSITION(set_heacon_pos, get_heacon_pos);
		/* Input the test result */
		EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Is the heacon area displayed (The contrast in the bottom half of the screen is different.) ?"));
		EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("\n[CertificationTest][Info] Hit any key ('y' or 'Y' is OK, Others are NG):", nullptr));

		/* Display start x, y, width, height position setting(invisible) */
		/* Output size(width, height) set by 0. */
		/*            out(vector),    x, y, w, h */
		_SET_POSITION(set_heacon_pos, 0, 0, 0, 0);
		_SET_HEACON_AREA(Obj, set_heacon_pos);
		set_flag = true;
		VHAL_LOGI("/*** phase12 set heacon.front.control.output.on=%u", set_flag);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "heacon.front.control.output.on", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "heacon.front.control.output.on", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(false, get_flag);	/* one-shot event */
		_GET_HEACON_AREA(Obj, get_heacon_pos);
		_CHECK_POSITION(set_heacon_pos, get_heacon_pos);
		/* Input the test result */
		EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Is the heacon area hidden (No difference in contrast in the screen) ?"));
		EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("\n[CertificationTest][Info] Hit any key ('y' or 'Y' is OK, Others are NG):", nullptr));

		/* Heacon area size setting(Show in lower half of screen) */
		/*            out(vector),    x,                   y,            w,                   h */
		_SET_POSITION(set_heacon_pos, 0, (screen_height / 2), screen_width, (screen_height / 2));
		_SET_HEACON_AREA(Obj, set_heacon_pos);
		set_flag = true;
		VHAL_LOGI("/*** phase13 set heacon.front.control.output.on=%u", set_flag);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "heacon.front.control.output.on", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "heacon.front.control.output.on", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(false, get_flag);	/* one-shot event */
		_GET_HEACON_AREA(Obj, get_heacon_pos);
		_CHECK_POSITION(set_heacon_pos, get_heacon_pos);
		/* Input the test result */
		EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Is the heacon area displayed (The contrast in the bottom half of the screen is different.) ?"));
		EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("\n[CertificationTest][Info] Hit any key ('y' or 'Y' is OK, Others are NG):", nullptr));

		/* Display start x, y, width, height position setting(invisible) */
		/* Output size(width, height) set by 0. */
		/*            out(vector),    x, y, w, h */
		_SET_POSITION(set_heacon_pos, 0, 0, 0, 0);
		_SET_HEACON_AREA(Obj, set_heacon_pos);
		set_flag = true;
		VHAL_LOGI("/*** phase14 set heacon.front.control.output.on=%u", set_flag);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "heacon.front.control.output.on", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "heacon.front.control.output.on", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(false, get_flag);	/* one-shot event */
		_GET_HEACON_AREA(Obj, get_heacon_pos);
		_CHECK_POSITION(set_heacon_pos, get_heacon_pos);
		/* Input the test result */
		EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("\n[CertificationTest][Info] Is the heacon area hidden (No difference in contrast in the screen) ?"));
		EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("\n[CertificationTest][Info] Hit any key ('y' or 'Y' is OK, Others are NG):", nullptr));
	}

	/* ========== Test End ========== */

	/* Front video visibility setting(invisible) */
	set_flag = false;
	VHAL_LOGI("/*** phase90 set vpath.front.control.visible=%u", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.visible", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(get_flag, set_flag);
	/* Front seat Video Path name setting(clear path) */
	vpath_result = VIDEO_HAL_VPATH_STS_FAILED;
	VHAL_LOGI("/*** phase91 regist vpath.front.status.current=%u with waiting", vpath_result);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(Obj, "vpath.front.status.current", Callback_VpathFrontStatusCurrent));
	vpath_str = "";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(VHAL_CT_TIMEOUT_SECONDS), [] { return (VIDEO_HAL_VPATH_STS_SUCCESS == vpath_result); });
	EXPECT_EQ(true, intime);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(Obj, "vpath.front.status.current", Callback_VpathFrontStatusCurrent));

	/* Video source setting(undo) */
	VHAL_LOGI("/*** phase92 set vsrc.front.control id=%u brightness=%u contrast=%u", init_video_id_value, init_brightness, init_contrast);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vsrc.front.control.id", &init_video_id_value, sizeof(init_video_id_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vsrc.front.control.brightness.step", &init_brightness, sizeof(init_brightness), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vsrc.front.control.contrast.step", &init_contrast, sizeof(init_contrast), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* Heacon area size setting(undo) */
	if (true == get_exec_heacon())
	{
		/* Heacon area size setting(undo) */
		_SET_HEACON_AREA(Obj, heacon_init_pos);
		set_flag = true;
		VHAL_LOGI("/*** phase93 set heacon.front.control.output.on=%u", set_flag);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "heacon.front.control.output.on", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "heacon.front.control.output.on", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(false, get_flag);	/* one-shot event */
	}

	/* Deinit() */
	VHAL_LOGI("/*** Test No.17 Exit");
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(Obj));
}
