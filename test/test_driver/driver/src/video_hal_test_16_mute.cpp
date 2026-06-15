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

#include "test_main.h"
#include "test_util.h"
#include "test_worker_thread.h"
#include "file_observer.h"
#include "com_stddef.h"
#include "stub_common.h"

/*** define ***/
#define EXEC_REAR_MUTE_DISPLAY 0

/*** prototype ***/

/*** variables ***/
static cockpit::hal::video_hal::CtlObj Obj;
static std::mutex mtx_sync;											/* (STUB_PATH)/TabCtrlApiPwCmdSend_callのコールバック用ミューテックス */
static std::condition_variable cond_sync;							/* (STUB_PATH)/TabCtrlApiPwCmdSend_callのコールバック用状態待機 */
static const int timeout_seconds = 3;								/* cond_waitのタイムアウト時間(秒) */

/* _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/ */
class VideoHAL_Mute: public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();
protected:
};

void VideoHAL_Mute::SetUp()
{
	/* just in case */
	kill_test_video_client();
}

void VideoHAL_Mute::TearDown()
{
	kill_test_video_client();
}

//
// Googletest TestCase
//
TEST_F(VideoHAL_Mute, Lv1Normal016)
{
//	cockpit::hal::video_hal::CtlObj Obj={0, NULL};
	bool intime;
	bool set_flag = false;
	bool get_flag = false;
	std::string vpath_str = "";
	uint32_t set_x = 0;
	uint32_t set_y = 0;
	uint32_t set_w = 0;
	uint32_t set_h = 0;

	/* Halcp Indication */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Test No.16 Start\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Overview : Mute Test\n"));

	/* VideoHAL Initialize */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(Obj));
	VHAL_LOGI("/*** Test No.16 Start");

	/* TabCtrlApiPwCmdSend_callのコールバック登録 */
	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	// 初期状態の取得
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "mute.front.status.display", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));

	/* clear vpath first just in case */
	vpath_str = "";
	VHAL_LOGI("/*** phase01 set vpath.front.control.current=[%s]", vpath_str.c_str());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.rear.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* Front video visibility setting(invisible) */
	set_flag = false;
	VHAL_LOGI("/*** phase02 set vpath.front.control.visible=%d", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* Releasing MUTE for the entire front display */
	set_flag = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "mute.front.control.display", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	VHAL_LOGI("/*** phase03 set mute.front.control.display=%d with waiting", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "mute.front.status.display", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(set_flag, get_flag);

	/* start test video client which makes the CARPLAY surface */
	exec_test_video_client(VHAL_CT_IVIID_CARPLAY, "sample_img_CARPLAY.png");
	/* Update vpath front */
	vpath_str = "CARPLAY";
	VHAL_LOGI("/*** phase04 set vpath.front.control.current=[%s]", vpath_str.c_str());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	/* Front video visibility setting(visible) */
	set_flag = true;
	VHAL_LOGI("/*** phase05 set vpath.front.control.visible=%d", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the sample CARPLAY surface displayed on the front display ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* ========== Test Start ========== */

	/* MUTE check of the entire front display */
	set_flag = true;
	VHAL_LOGI("/*** phase06 set mute.front.control.display=%d with waiting", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "mute.front.control.display", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "mute.front.status.display", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(set_flag, get_flag);

	set_flag = false;
	VHAL_LOGI("/*** phase07 set vpath.front.control.visible=%d", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] Check that the entire front screen is black.\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] Visually check if the screen is black.\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] The surface for the whole MUTE of the front display is the full screen size. \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* Releasing MUTE for the entire front display */
	set_flag = false;
	VHAL_LOGI("/*** phase08 set mute.front.control.display=%d with waiting", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "mute.front.control.display", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "mute.front.status.display", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(set_flag, get_flag);

	set_flag = true;
	VHAL_LOGI("/*** phase08 set vpath.front.control.visible=%d", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the sample CARPLAY surface displayed on the front display ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* MUTE check of the front video */
	set_x = 0;
	set_y = 0;
	set_w = 800;
	set_h = 480;
	set_flag = true;
	VHAL_LOGI("/*** phase09 set x=%u y=%u w=%u h=%u", set_x, set_y, set_w, set_h);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.x",      &set_x, sizeof(set_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.y",      &set_y, sizeof(set_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.width",  &set_w, sizeof(set_w), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.height", &set_h, sizeof(set_h), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.output.on", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	set_flag = true;
	VHAL_LOGI("/*** phase10 set mute.front.control.video=%d", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "mute.front.control.video", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "mute.front.status.video", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(set_flag, get_flag);
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] Check that only the front video is black.\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] Please check ivi object tree with LayerManagerControl command.\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] The surface (ivi_id=81) must have a visibility of 1. \n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] The entire MUTE surface of the front video is 800x480 in size.\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* Releasing MUTE for the entire front video */
	set_flag = false;
	VHAL_LOGI("/*** phase11 set mute.front.control.video=%d", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "mute.front.control.video", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "mute.front.status.video", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(set_flag, get_flag);
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the sample CARPLAY surface displayed on the front video ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* Checking the front backlight setting */
	set_flag = false;
	VHAL_LOGI("/*** phase11 set mute.front.control.backlight=%d with waiting", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "mute.front.control.backlight", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "mute.front.status.backlight", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(set_flag, get_flag);
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Check that the front backlight is off ? ('y' or 'Y' is OK, Others are NG):", nullptr));
	set_flag = true;
	VHAL_LOGI("/*** phase12 set mute.front.control.backlight=%d with waiting", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "mute.front.control.backlight", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "mute.front.status.backlight", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(set_flag, get_flag);
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Check that the front backlight is on ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	if (false == get_exec_rear())
	{
		EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Test No.16 Skip(rear(FullRSE))\n"));
		VHAL_LOGI("/*** Test No.16 Skip(rear(FullRSE))");
	}
	else
	{
		/* MUTE check of the entire rear display(Use DTV) */
		/* Rear video visibility setting(invisible) */
		set_flag = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.rear.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		/* start test video client which makes the DTV surface */
		exec_test_video_client(VHAL_CT_IVIID_REAR_DTV, "sample_img_DTV.png");
		vpath_str = "DTV";
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.rear.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		/* Rear video visibility setting(visible) */
		set_flag = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.rear.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the sample DTV surface displayed on the rear display ? ('y' or 'Y' is OK, Others are NG):", nullptr));

#if EXEC_REAR_MUTE_DISPLAY
		set_flag = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "mute.rear.control.display", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "mute.rear.status.display", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(set_flag, get_flag);
		EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] Check that the entire rear screen is black. \n"));
		EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] Visually check if the screen is black.\n"));
		EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] The surface for the whole MUTE of the rear display is the full screen size. \n"));
		EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] ('y' or 'Y' is OK, Others are NG):", nullptr));
		/* Releasing MUTE for the entire rear display */
		set_flag = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "mute.rear.control.display", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "mute.rear.status.display", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(set_flag, get_flag);
		EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the DTV surface displayed on the rear display ? ('y' or 'Y' is OK, Others are NG):", nullptr));
#endif /* EXEC_REAR_MUTE_DISPLAY */

		/* MUTE check of the rear video */
		/* change output size to be smaller square size */
		set_x = 0;
		set_y = 0;
		/* BEV step3 set_wの値を800から640に変更しました。 */
		set_w = 640;
		set_h = 480;
		set_flag = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.rear.control.output.x",      &set_x, sizeof(set_x), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.rear.control.output.y",      &set_y, sizeof(set_y), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.rear.control.output.width",  &set_w, sizeof(set_w), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.rear.control.output.height", &set_h, sizeof(set_h), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.rear.control.output.on", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		set_flag = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "mute.rear.control.video", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "mute.rear.status.video", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(set_flag, get_flag);
		EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] Check that only the rear video is black.\n"));
		EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] Please check ivi object tree with LayerManagerControl command.\n"));
		EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] The surface (ivi_id=181) must have a visibility of 1. \n"));
		EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] The entire MUTE surface of the rear video is 640x480 in size.\n"));
		EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] ('y' or 'Y' is OK, Others are NG):", nullptr));
		/* Releasing MUTE for the entire rear video */
		set_flag = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "mute.rear.control.video", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "mute.rear.status.video", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(set_flag, get_flag);
		EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the DTV surface displayed on the rear video ? ('y' or 'Y' is OK, Others are NG):", nullptr));
	}

	/* ========== Test End ========== */
	set_flag = false;
	vpath_str = "";
	/* Front video visibility setting(invisible) */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	/* clear vpath */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	if (true == get_exec_rear())
	{
		/* Rear video visibility setting(invisible) */
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.rear.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		/* clear vpath */
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.rear.control.current",  &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	}

	/* Deinit() */
	VHAL_LOGI("/*** Test No.16 Exit");
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(Obj));
}
