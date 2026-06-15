/*
 * @copyright Copyright (c) 2022- Woven Alpha, Inc.
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

class VideoHAL_Illumination: public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();
protected:
};

void VideoHAL_Illumination::SetUp()
{
	kill_test_video_client();

	set_hmi_visibility(false);
}


void VideoHAL_Illumination::TearDown()
{
	set_hmi_visibility(true);

	kill_test_video_client();
}

//
// Googletest TestCase
//
TEST_F(VideoHAL_Illumination, Lv1Normal007)
{
//	cockpit::hal::video_hal::CtlObj Obj={0, NULL};
	std::string vpath_str;
	int32_t set_value = 0;
	int32_t get_value = 0;
	uint32_t pretest_video_id_value = 0;
	bool set_flag = false;

	/* Halcp Indication */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Test No.7 Start\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Overview : Illumination Test\n"));

	/* VideoHAL Initialize */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(Obj));
	VHAL_LOGI("/*** Test No.07 Start");
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vsrc.front.status.id", &pretest_video_id_value, sizeof(pretest_video_id_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_NE(0, pretest_video_id_value);

	/* ========== Test Start ========== */

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

	/* start test video client which makes the CARPLAY surface */
	exec_test_video_client(VHAL_CT_IVIID_CARPLAY, "sample_img_CARPLAY.png");

	vpath_str = "CARPLAY";
	VHAL_LOGI("/*** phase03 set vpath.front.control.current=[%s]", vpath_str.c_str());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* Front video visibility setting(visible) */
	set_flag = true;
	VHAL_LOGI("/*** phase04 set vpath.front.control.visible=%d", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	set_value = VIDEO_HAL_VSRC_ID_OTHER;
	VHAL_LOGI("/*** phase05 set vsrc.front.control.id=%d", set_value);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vsrc.front.control.id", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vsrc.front.status.id", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);

	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the CARPLAY surface visible ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* Dimmer, Night screen(Dark color,Auto) */
	set_value = VIDEO_HAL_SETTING_NIGHT;
	VHAL_LOGI("/*** phase06 set vhal.setting.control.day_night=%d", set_value);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vhal.setting.control.day_night", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.day_night", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);
	set_value = VIDEO_HAL_THEME_COLOR_AUTO_DARK;
	VHAL_LOGI("/*** phase07 set vhal.setting.control.theme_color=%d", set_value);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vhal.setting.control.theme_color", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.theme_color", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the illumincation 'Night screen' (Dark color,Auto) ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* Dimmer, Night screen(Light color) */
	set_value = VIDEO_HAL_SETTING_NIGHT;
	VHAL_LOGI("/*** phase08 set vhal.setting.control.day_night=%d", set_value);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vhal.setting.control.day_night", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.day_night", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);
	set_value = VIDEO_HAL_THEME_COLOR_SELECT_LIGHT;
	VHAL_LOGI("/*** phase09 set vhal.setting.control.theme_color=%d", set_value);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vhal.setting.control.theme_color", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.theme_color", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the illumincation 'Night screen' (Light color) ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* No Dimmer, Daytime screen(Light color,Auto) */
	set_value = VIDEO_HAL_SETTING_DAY;
	VHAL_LOGI("/*** phase10 set vhal.setting.control.day_night=%d", set_value);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vhal.setting.control.day_night", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.day_night", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);
	set_value = VIDEO_HAL_THEME_COLOR_AUTO_LIGHT;
	VHAL_LOGI("/*** phase11 set vhal.setting.control.theme_color=%d", set_value);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vhal.setting.control.theme_color", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.theme_color", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the illumincation 'Daytime screen' (Light color,Auto) ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* No Dimmer, Daytime screen(Dark color) */
	set_value = VIDEO_HAL_SETTING_DAY;
	VHAL_LOGI("/*** phase12 set vhal.setting.control.day_night=%d", set_value);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vhal.setting.control.day_night", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.day_night", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);
	set_value = VIDEO_HAL_THEME_COLOR_SELECT_DARK;
	VHAL_LOGI("/*** phase13 set vhal.setting.control.theme_color=%d", set_value);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vhal.setting.control.theme_color", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.theme_color", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the illumincation 'Daytime screen' (Dark color) ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* start test video client which makes the DTV surface */
	kill_test_video_client();
	exec_test_video_client(VHAL_CT_IVIID_FRONT_DTV, "sample_img_DTV.png");

	/* Front video visibility setting(invisible) */
	set_flag = false;
	VHAL_LOGI("/*** phase14 set vpath.front.control.visible=%d", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	/* clear vpath first just in case */
	vpath_str = "";
	VHAL_LOGI("/*** phase15 set vpath.front.control.current=[%s]", vpath_str.c_str());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	/* Update vpath front */
	vpath_str = "DTV";
	VHAL_LOGI("/*** phase16 set vpath.front.control.current=[%s]", vpath_str.c_str());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	/* Front video visibility setting(visible) */
	set_flag = true;
	VHAL_LOGI("/*** phase17 set vpath.front.control.visible=%d", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	/* Set Video source ID(Video source ID) */
	set_value = VIDEO_HAL_VSRC_ID_DTV;
	VHAL_LOGI("/*** phase18 set vsrc.front.control.id=%d", set_value);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vsrc.front.control.id", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vsrc.front.status.id", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);
	
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the sample DTV surface displayed on the front display ? ('y' or 'Y' is OK, Others are NG):", nullptr));
	
	/* Dimmer, Night screen(Dark color,Auto) */
	set_value = VIDEO_HAL_SETTING_NIGHT;
	VHAL_LOGI("/*** phase19 set vhal.setting.control.day_night=%d", set_value);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vhal.setting.control.day_night", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.day_night", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);
	set_value = VIDEO_HAL_THEME_COLOR_AUTO_DARK;
	VHAL_LOGI("/*** phase20 set vhal.setting.control.theme_color=%d", set_value);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vhal.setting.control.theme_color", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.theme_color", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the illumincation 'Night screen' (Dark color,Auto) ? ('y' or 'Y' is OK, Others are NG):", nullptr));
	
	/* No Dimmer, Daytime screen(Light color,Auto) */
	set_value = VIDEO_HAL_SETTING_DAY;
	VHAL_LOGI("/*** phase21 set vhal.setting.control.day_night=%d", set_value);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vhal.setting.control.day_night", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.day_night", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);
	set_value = VIDEO_HAL_THEME_COLOR_AUTO_LIGHT;
	VHAL_LOGI("/*** phase22 set vhal.setting.control.theme_color=%d", set_value);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vhal.setting.control.theme_color", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.theme_color", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the illumincation 'Daytime screen' (Light color,Auto) ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* ========== Test End ========== */
	/* clear vpath and vsrc */
	/* Front video visibility setting(invisible) */
	set_flag = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	vpath_str = "";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vsrc.front.control.id", &pretest_video_id_value, sizeof(pretest_video_id_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* Deinit() */
	VHAL_LOGI("/*** Test No.07 Exit");
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(Obj));
}
