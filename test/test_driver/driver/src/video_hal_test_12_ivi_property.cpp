/*
 * @copyright Copyright (c) 2022 Woven Alpha, Inc.
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
static cockpit::hal::video_hal::CtlObj Obj;

class VideoHAL_Ivi_Property: public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();
protected:
};

void VideoHAL_Ivi_Property::SetUp()
{
	/* just in case */
	kill_test_video_client();
}

void VideoHAL_Ivi_Property::TearDown()
{
	kill_test_video_client();
}

//
// Googletest TestCase
//
TEST_F(VideoHAL_Ivi_Property, Lv1Normal012)
{
//	cockpit::hal::video_hal::CtlObj Obj={0, NULL};
	std::string vpath_str;
	std::string get_str;
	std::string dsp_json_str;
	const std::string dsp_screen_json_comment_str = "[CertificationTest][Info] ivi.screen.status.properties(json data)=";
	const std::string dsp_layer_json_comment_str = "[CertificationTest][Info] ivi.layer.status.properties(json data)=";
	const std::string dsp_surface_json_comment_str = "[CertificationTest][Info] ivi.surface.status.properties(json data)=";
	const std::string dsp_check_comment_str = "[CertificationTest][Info] Please check json file(/run/arene/share/layer_config.json).\n";
	bool set_flag = false;

	/* Halcp Indication */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Test No.12 Start\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Overview : Ivi property Test\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] Please check ivi property values with LayerManagerControl command.\n"));

	/* VideoHAL Initialize */
	VHAL_LOGI("/*** ready01 video_hal::Init");
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(Obj));

	/* Front video visibility setting(invisible) */
	VHAL_LOGI("/*** ready02 vpath.front.control.visible(false)");
	set_flag = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* clear vpath first just in case */
	VHAL_LOGI("/*** ready03 vpath.front.control.current(empty)");
	vpath_str = "";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	VHAL_LOGI("/*** ready04 makes the CARPLAY surface(sample_img_CARPLAY.png)");
	/* start test video client which makes the CARPLAY surface */
	exec_test_video_client(VHAL_CT_IVIID_CARPLAY, "sample_img_CARPLAY.png");

	/* Update vpath front */
	VHAL_LOGI("/*** ready05 vpath.front.control.current(CARPLAY)");
	vpath_str = "CARPLAY";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* Front video visibility setting(visible) */
	VHAL_LOGI("/*** ready06 vpath.front.control.visible(true)");
	set_flag = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	VHAL_LOGI("/*** Test No.12 Start");
	/* ========== Test Start ========== */

	/* Get ivi properties of the screen(ivi_id=0) */
	VHAL_LOGI("/*** phase01 check ivi.screen.status.properties ivi_id=0");
	get_str = "";
	dsp_json_str = "";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "ivi.screen.status.properties?ivi_id=0", &get_str, (uint32_t)get_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	dsp_json_str = dsp_screen_json_comment_str + get_str + "\n";

	VHAL_LOGI("/*** phase02 check json data the same to the screen(ivi_id=0)");
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION(dsp_json_str.c_str()));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION(dsp_check_comment_str.c_str()));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info]Are json data the same to the screen(ivi_id=0)? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* Get ivi properties of the name(front) */
	VHAL_LOGI("/*** phase03 check ivi.screen.status.properties name=front");
	get_str = "";
	dsp_json_str = "";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "ivi.screen.status.properties?name=front", &get_str, (uint32_t)get_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));

	VHAL_LOGI("/*** phase04 check json data the same to the name(front)");
	dsp_json_str = dsp_screen_json_comment_str + get_str + "\n";
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION(dsp_json_str.c_str()));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION(dsp_check_comment_str.c_str()));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info]Are json data the same to the name(front)? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* Get ivi properties of the layer(ivi_id=6) */
	VHAL_LOGI("/*** phase05 check ivi.screen.status.properties ivi_id=6");
	get_str = "";
	dsp_json_str = "";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "ivi.layer.status.properties?ivi_id=6", &get_str, (uint32_t)get_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));

	VHAL_LOGI("/*** phase06 check json data the same to the layer(ivi_id=6)");
	dsp_json_str = dsp_layer_json_comment_str + get_str + "\n";
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION(dsp_json_str.c_str()));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION(dsp_check_comment_str.c_str()));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Are json data the same to the layer(ivi_id=6)? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* Get ivi properties of the name(front_video) */
	VHAL_LOGI("/*** phase07 check ivi.layer.status.properties name=front_video");
	get_str = "";
	dsp_json_str = "";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "ivi.layer.status.properties?name=front_video", &get_str, (uint32_t)get_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));

	VHAL_LOGI("/*** phase08 check json data the same to the name(front_video)");
	dsp_json_str = dsp_layer_json_comment_str + get_str + "\n";
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION(dsp_json_str.c_str()));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION(dsp_check_comment_str.c_str()));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Are json data the same to the name(front_video)? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* Get ivi properties of the surface(ivi_id=31) */
	VHAL_LOGI("/*** phase09 check ivi.layer.status.properties ivi_id=31");
	get_str = "";
	dsp_json_str = "";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "ivi.surface.status.properties?ivi_id=31", &get_str, (uint32_t)get_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));

	VHAL_LOGI("/*** phase10 check json data the same to the surface(ivi_id=31)");
	dsp_json_str = dsp_surface_json_comment_str + get_str + "\n";
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION(dsp_json_str.c_str()));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION(dsp_check_comment_str.c_str()));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Are json data the same to the surface(ivi_id=31)? ('y' or 'Y' is OK, Others are NG):", nullptr));

	VHAL_LOGI("/*** phase11 check ivi.layer.status.properties name=carplay");
	get_str = "";
	dsp_json_str = "";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "ivi.surface.status.properties?name=carplay", &get_str, (uint32_t)get_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));

	VHAL_LOGI("/*** phase12 check json data the same to the name(carplay)");
	dsp_json_str = dsp_surface_json_comment_str + get_str + "\n";
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION(dsp_json_str.c_str()));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION(dsp_check_comment_str.c_str()));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Are name and Landscape the same to the name(carplay)? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* ========== Test End ========== */

	/* Front video visibility setting(invisible) */
	VHAL_LOGI("/*** finish01 vpath.front.control.visible(false)");
	set_flag = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* clear vpath */
	VHAL_LOGI("/*** finish02 vpath.front.control.current(empty)");
	vpath_str = "";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* Deinit() */
	VHAL_LOGI("/*** Test No.12 Exit");
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(Obj));
}
