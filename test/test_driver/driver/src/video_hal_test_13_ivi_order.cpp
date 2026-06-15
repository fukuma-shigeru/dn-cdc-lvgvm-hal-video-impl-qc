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

class VideoHAL_Ivi_Order: public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();
protected:
};

void VideoHAL_Ivi_Order::SetUp()
{
	/* just in case */
	kill_test_video_client();
}

void VideoHAL_Ivi_Order::TearDown()
{
	kill_test_video_client();
}

//
// Googletest TestCase
//
TEST_F(VideoHAL_Ivi_Order, Lv1Normal013)
{
//	cockpit::hal::video_hal::CtlObj Obj={0, NULL};
	std::string vpath_str;
	int32_t get_value = 0;
	int32_t set_value = 0;
	bool set_flag = false;

	/* Halcp Indication */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Test No.13 Start\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Overview : Ivi order Test\n"));

	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] If the HMI screen does not appear, execute \"systemctl start homescreen\" to display it.\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTANYKEY("[CertificationTest][Info] Please Input Any Key For Continue Test:"));

	/* VideoHAL Initialize */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(Obj));
	VHAL_LOGI("/*** Test No.13 Start");

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
	exec_test_video_client(VHAL_CT_IVIID_CARPLAY, "sample_img_CARPLAY.png");
	/* Update vpath front */
	vpath_str = "CARPLAY";
	VHAL_LOGI("/*** phase03 set vpath.front.control.current=[%s]", vpath_str.c_str());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	/* Front video visibility setting(visible) */
	set_flag = true;
	VHAL_LOGI("/*** phase04 set vpath.front.control.visible=%d", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* ========== Test Start ========== */

	/* Change layer priority by order value(ID specification). */

	/* Get with ivi_id(ivi_id=6) */
	VHAL_LOGI("/*** phase05 get with ivi_id(ivi_id=6)");
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "ivi.layer.status.order?ivi_id=6", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(4000, get_value);
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the HMI surface displayed on the front display ? ('y' or 'Y' is OK, Others are NG):", nullptr));
	/* Set and get with ivi_id(ivi_id=6, order=10000) */
	set_value = 10000;
	get_value = 0;
	VHAL_LOGI("/*** phase06 Set and get with ivi_id(ivi_id=6, order=10000)");
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "ivi.layer.control.order?ivi_id=6", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "ivi.layer.status.order?ivi_id=6", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the sample CARPLAY surface displayed on the front display(specified ivi_id) ? ('y' or 'Y' is OK, Others are NG):", nullptr));
	/* Set and get with ivi_id(ivi_id=6, order=4000) */
	set_value = 4000;
	get_value = 0;
	VHAL_LOGI("/*** phase07 Set and get with ivi_id(ivi_id=6, order=4000)");
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "ivi.layer.control.order?ivi_id=6", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "ivi.layer.status.order?ivi_id=6", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the HMI surface displayed on the front display(specified ivi_id) ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* Change layer priority by order value(name specification). */

	/* Get with name(name=front_video) */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "ivi.layer.status.order?name=front_video", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(4000, get_value);
	/* Set and get with ivi_id(name=front_video, order=10000) */
	set_value = 10000;
	get_value = 0;
	VHAL_LOGI("/*** phase08 Set and get with ivi_id(name=front_video, order=10000)");
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "ivi.layer.control.order?name=front_video", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "ivi.layer.status.order?name=front_video", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the sample CARPLAY surface displayed on the front display(specified name) ? ('y' or 'Y' is OK, Others are NG):", nullptr));
	/* Set and get with ivi_id(name=front_video, order=4000) */
	set_value = 4000;
	get_value = 0;
	VHAL_LOGI("/*** phase09 Set and get with ivi_id(name=front_video, order=4000)");
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "ivi.layer.control.order?name=front_video", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "ivi.layer.status.order?name=front_video", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the HMI surface displayed on the front display(specified name) ? ('y' or 'Y' is OK, Others are NG):", nullptr));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTANYKEY("[CertificationTest][Info] If \"systemctl start homescreen\" is running, stop at \"systemctl stop homescreen\". "));

	/* ========== Test End ========== */

	/* Front video visibility setting(invisible) */
	set_flag = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	/* clear vpath */
	vpath_str = "";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* Deinit() */
	VHAL_LOGI("/*** Test No.13 Exit");
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(Obj));
}