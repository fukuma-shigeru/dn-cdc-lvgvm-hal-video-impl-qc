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

#include "test_main.h"
#include "test_util.h"
#include "test_worker_thread.h"
#include "file_observer.h"
#include "com_stddef.h"
#include "stub_common.h"


/*** define ***/
/*
===== ファイル構成
基点：/run/arene/vehicle_fs/var/bev3/stub/ccm_client
├ TabCtrlApiPwCmdNtyCB   コールバック関数用フォルダ
│　├ 2_1_param    映像MUTE確認の2バイト目
│　└ 2_control    　〃　コールバック発生トリガ
└ TabCtrlApiPwCmdSend_call     TabCtrlApiPwCmdSend関数用call
*/

/* 本試験で扱うフォルダやファイル名 */
#define STUB_PATH					"/run/arene/vehicle_fs/var/bev3/stub/ccm_client/"
#define STUB_PATH_CB				STUB_PATH "TabCtrlApiPwCmdNtyCB/"
#define STUB_PARAM_STS(_no, _itm)	STUB_PATH_CB #_no "_" #_itm "_param"
#define STUB_PARAM_CTL(_no)			STUB_PATH_CB #_no "_control"
#define STUB_CALL(_fnc)				STUB_PATH #_fnc "_call"

/*** define ***/
#define BOOLtoINT32(_bval)	((INT32)(_bval ? 1 : 0))		/* bool値をINT32に変換してファイル出力するためのマクロ */

enum LISTENER_STATUS {
	LISTENER_STATUS_INIT = 0 ,
	LISTENER_STATUS_WAITING ,
	LISTENER_STATUS_CALLED ,
};

/*** prototype ***/

/*** variables ***/
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static int32_t vpath_result;
static cockpit::hal::video_hal::CtlObj Obj;
static LISTENER_STATUS listerner_status = LISTENER_STATUS_INIT;		/* (STUB_PATH)/TabCtrlApiPwCmdSend_callのコールバック関数実施フラグ */
static const int timeout_seconds = 3;								/* cond_waitのタイムアウト時間(秒) */

/* Work Parameter */
struct Tab2Param_VideoHAL_Api
{
	uint32_t	wait_us;
	bool		param1;
};

/* _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/ */
class VideoHAL_ApiThread: public CTestWorkerThread<Tab2Param_VideoHAL_Api>
{
private:
	int32_t Execute(const Tab2Param_VideoHAL_Api& data) const noexcept 
	{
		std::string path;

		VHAL_LOGI("/*** write stub-file. param1=%d", data.param1);
		usleep(data.wait_us);

		// stubからの応答を設定 (OPCは0x62固定のため非出力 */
		path = STUB_PARAM_STS(2,1);			/* /run/arene/vehicle_fs/var/bev3/stub/ccm_client/TabCtrlApiPwCmdNtyCB/2_1_param */
		const int32_t param1{BOOLtoINT32(data.param1)};
		cockpit::bs::CFileObserver::GetInstance()->Write(path, &param1, sizeof(param1));

		system("echo test > " STUB_PARAM_CTL(2));	/* /run/arene/vehicle_fs/var/bev3/stub/ccm_client/TabCtrlApiPwCmdNtyCB/2_control */

		return 0;
	}
};

/* _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/ */
class VideoHAL_Api: public::testing::Test
{
protected:
	static void Callback_VpathFrontStatusCurrent();
	virtual void SetUp();
	virtual void TearDown();
};

void VideoHAL_Api::Callback_VpathFrontStatusCurrent()
{
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);

		/* Check vpath front current again */
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.status.current", &vpath_result, sizeof(vpath_result), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_VPATH_STS_SUCCESS, vpath_result);
	}

	cond_sync.notify_one();
}

void VideoHAL_Api::SetUp()
{
	/* just in case */
	kill_test_video_client();

	/* start test video client which makes the CARPLAY surface */
	exec_test_video_client(VHAL_CT_IVIID_CARPLAY, "sample_img_CARPLAY.png");
}

void VideoHAL_Api::TearDown()
{
	kill_test_video_client();
}

/* (STUB_PATH)/TabCtrlApiPwCmdSend_callのコールバック */
static void TabCtrlApiPwCmdSendListener(const char* path)
{
	VHAL_LOGI("/*** call status=%d", listerner_status);
	if (LISTENER_STATUS_WAITING == listerner_status)
	{
		const std::lock_guard<std::mutex> lock_sync{mtx_sync};
		listerner_status = LISTENER_STATUS_CALLED;
		cond_sync.notify_one();
		VHAL_LOGI("/*** set notify");
	}
}

//
// Googletest TestCase
//
TEST_F(VideoHAL_Api, Lv1Normal001)
{
//	cockpit::hal::video_hal::CtlObj Obj={0, NULL};
	std::string set_str;
	std::string get_str;
	bool intime = false;
	bool set_flag = false;
	bool get_flag = false;
	int32_t set_value = 0;
	int32_t get_value = 0;

	/* Halcp Indication */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Test No.1 Start\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Overview : Video HAL API Test\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Detail : Call all VideoHAL APIs. If there is any error at calling Init(), check if the VideoHAL server is running.\n"));

	/* ========== Test Start ========== */

	/* VideoHAL Initialize */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(Obj));
	VHAL_LOGI("/*** Test No.1 Start");

	/* TabCtrlApiPwCmdSend_callのコールバック登録 */
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	StubClientRegisterListerner(STUB_CALL(TabCtrlApiPwCmdSend), TabCtrlApiPwCmdSendListener);

	/* Workerスレッド起動 (応答通知を非同期で送信) */
	std::unique_ptr<VideoHAL_ApiThread> p_wnd_thread = std::make_unique<VideoHAL_ApiThread>();
	Tab2Param_VideoHAL_Api tab2_param;
	tab2_param.wait_us   = 50000;		// 50msのウェイト
	p_wnd_thread->Start();

	/* 初期状態の取得 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "mute.front.status.display", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));

	/* Since the initial value of the mute display is true, it is set to false first. */
	set_flag = tab2_param.param1 = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "mute.front.control.display", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	p_wnd_thread->Notify(tab2_param);	/* RequestUpdateの前に、stub応答を裏（別スレッド）で設定 */
	if (true == get_flag)
	{
		VHAL_LOGI("/*** phase01A set mute.front.control.display=%d with waiting", set_flag);
		listerner_status = LISTENER_STATUS_WAITING;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	}
	else
	{
		VHAL_LOGI("/*** phase01B set mute.front.control.display=%d without waiting", set_flag);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	}
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "mute.front.status.display", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));

	/* Since the initial value of the backlight is false, it is set to true first. */
	set_flag = true;
	VHAL_LOGI("/*** phase02 set mute.front.control.backlight=%d", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "mute.front.control.backlight", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	listerner_status = LISTENER_STATUS_WAITING;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "mute.front.status.backlight", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(set_flag, get_flag);

	/* Since the initial value of the day/night is VIDEO_HAL_SETTING_DAY , it is set to true first. */
	set_value = VIDEO_HAL_SETTING_DAY;
	VHAL_LOGI("/*** phase03 set vhal.setting.control.day_night=%d", set_value);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vhal.setting.control.day_night", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.day_night", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);

	/* Since the initial value of the theme color is VIDEO_HAL_THEME_COLOR_AUTO_LIGHT , it is set to true first. */
	set_value = VIDEO_HAL_THEME_COLOR_AUTO_LIGHT;
	VHAL_LOGI("/*** phase04 set vhal.setting.control.theme_color=%d", set_value);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vhal.setting.control.theme_color", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.theme_color", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(set_value, get_value);

	/* Front video visibility setting(invisible) */
	set_flag = false;
	VHAL_LOGI("/*** phase05 set vpath.front.control.visible=%d", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* clear vpath first just in case */
	set_str = "";
	VHAL_LOGI("/*** phase06 set vpath.front.control.current=[%s]", set_str.c_str());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &set_str, (uint32_t)set_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* Update vpath front */
	set_str = "CARPLAY";
	VHAL_LOGI("/*** phase07 set vpath.front.control.current=[%s]", set_str.c_str());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &set_str, (uint32_t)set_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* Update vpath front */
	get_str = "";
	VHAL_LOGI("/*** phase08 set vpath.front.control.current=[%s]", set_str.c_str());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vpath.front.control.current", &get_str, (uint32_t)set_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_STREQ(set_str.c_str(), get_str.c_str());

	/* VideoHAL Register callback function */
	vpath_result = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(Obj, "vpath.front.status.current", Callback_VpathFrontStatusCurrent));

	/* Update vpath front */
	set_str = "";
	VHAL_LOGI("/*** phase09 set vpath.front.control.current=[%s] with waiting", set_str.c_str());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &set_str, (uint32_t)set_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* wait callback */
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(VHAL_CT_TIMEOUT_SECONDS), [] { return (VIDEO_HAL_VPATH_STS_SUCCESS == vpath_result); });
	EXPECT_EQ(true, intime);

	/* ClearCallback() */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(Obj, "vpath.front.status.current", Callback_VpathFrontStatusCurrent));

	vpath_result = VIDEO_HAL_VPATH_STS_FAILED;
	set_str = "CARPLAY";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &set_str, (uint32_t)set_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* Front video visibility setting(visible) */
	set_flag = true;
	VHAL_LOGI("/*** phase10 set vpath.front.control.visible=%d with waiting and timeout", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* wait callback */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("please wait a moment until a callback wait is timed out.\n"));
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(VHAL_CT_TIMEOUT_SECONDS), [] { return (VIDEO_HAL_VPATH_STS_SUCCESS == vpath_result); });
	EXPECT_EQ(false, intime);

	/* Input the test result */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] VideoHAL API TEST Complete \n"));

	/* Front video visibility setting(invisible) */
	set_flag = false;
	VHAL_LOGI("/*** phase11 set vpath.front.control.visible=%d", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* clear vpath */
	set_str = "";
	VHAL_LOGI("/*** phase12 set vpath.front.control.current=[%s]", set_str.c_str());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &set_str, (uint32_t)set_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));

	/* Workerスレッド終了 */
	p_wnd_thread->End();

	/* TabCtrlApiPwCmdSend_callのコールバック解除 */
	StubClientUnRegisterListerner(STUB_CALL(TabCtrlApiPwCmdSend));

	/* Deinit() */
	VHAL_LOGI("/*** Test No.1 Exit");
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(Obj));
}
