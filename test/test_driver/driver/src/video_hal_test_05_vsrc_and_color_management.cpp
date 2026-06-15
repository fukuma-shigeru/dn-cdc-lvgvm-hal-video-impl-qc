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

class VideoHAL_Vsrc_And_Color_Management: public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();
protected:
};

void VideoHAL_Vsrc_And_Color_Management::SetUp()
{
	set_hmi_visibility(false);
	kill_test_video_client();
}

void VideoHAL_Vsrc_And_Color_Management::TearDown()
{
	set_hmi_visibility(true);
	kill_test_video_client();
}

//
// Googletest TestCase
//
TEST_F(VideoHAL_Vsrc_And_Color_Management, Lv1Normal005)
{
//	cockpit::hal::video_hal::CtlObj Obj={0, NULL};
	std::string vpath_str;
	int32_t set_value = 0;
	int32_t get_value = 0;
	uint32_t pretest_video_id_value = 0;
	bool set_flag = false;

	/* Halcp Indication */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Test No.5 Start\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Overview : Video Sourced and Color Management Test\n"));

	/* VideoHAL Initialize */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(Obj));
	VHAL_LOGI("/*** Test No.05 Start");
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vsrc.front.status.id", &pretest_video_id_value, sizeof(pretest_video_id_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_NE(0, pretest_video_id_value);

	/* start test video client which makes the DTV surface */
	exec_test_video_client(VHAL_CT_IVIID_FRONT_DTV, "sample_img_DTV.png");

	/* ========== Test Start ========== */
	/* Front video visibility setting(invisible) */
	set_flag = false;
	VHAL_LOGI("/*** phase01 set vpath.front.control.visible=%d", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* Update vpath front */
	vpath_str = "DTV";
	VHAL_LOGI("/*** phase02 set vpath.front.control.current=[%s]", vpath_str.c_str());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* Front video visibility setting(visible) */
	set_flag = true;
	VHAL_LOGI("/*** phase03 set vpath.front.control.visible=%d", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the sample DTV surface displayed on the front display ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	set_value = VIDEO_HAL_VSRC_ID_DTV;
	VHAL_LOGI("/*** phase04 set vsrc.front.control.id=%d", set_value);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vsrc.front.control.id", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vsrc.front.status.id", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);

	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] Change 'brightness' value.\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTANYKEY("Hit any key and check 'brightness' of the front display (1/6):"));
	set_value = 1;
	VHAL_LOGI("/*** phase05 set vsrc.front.control.brightness.step=%d", set_value);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vsrc.front.control.brightness.step", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vsrc.front.status.brightness.step", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTANYKEY("Hit any key and check 'brightness' of the front display (2/6):"));
	set_value = 63;
	VHAL_LOGI("/*** phase06 set vsrc.front.control.brightness.step=%d", set_value);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vsrc.front.control.brightness.step", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vsrc.front.status.brightness.step", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTANYKEY("Hit any key and check 'brightness' of the front display (3/6):"));
	set_value = 32;
	VHAL_LOGI("/*** phase07 set vsrc.front.control.brightness.step=%d", set_value);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vsrc.front.control.brightness.step", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vsrc.front.status.brightness.step", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is 'brightness' of the front display changed ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] Change 'contrast' value.\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTANYKEY("Hit any key and check 'contrast' of the front display (4/6):"));
	set_value = 1;
	VHAL_LOGI("/*** phase08 set vsrc.front.control.contrast.step=%d", set_value);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vsrc.front.control.contrast.step", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vsrc.front.status.contrast.step", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTANYKEY("Hit any key and check 'contrast' of the front display (5/6):"));
	set_value = 63;
	VHAL_LOGI("/*** phase09 set vsrc.front.control.contrast.step=%d", set_value);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vsrc.front.control.contrast.step", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vsrc.front.status.contrast.step", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTANYKEY("Hit any key and check 'contrast' of the front display (6/6):"));
	set_value = 32;
	VHAL_LOGI("/*** phase10 set vsrc.front.control.contrast.step=%d", set_value);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vsrc.front.control.contrast.step", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vsrc.front.status.contrast.step", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is 'contrast' of the front display changed ? ('y' or 'Y' is OK, Others are NG):", nullptr));

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
	VHAL_LOGI("/*** Test No.05 Exit");
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(Obj));
}
