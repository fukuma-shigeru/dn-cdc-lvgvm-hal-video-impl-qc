/*
 * @copyright Copyright (c) 2022 Woven Alpha, Inc.
 */

/*** inclue ***/
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <sys/stat.h>

#include <gtest/gtest.h>
#include "ilm/ilm_common.h"
#include "ilm/ilm_control.h"

#include "video_hal_ct_common.h"
#include "video_hal.h"
#include "vhal_log.h"

/*** structure ***/
typedef struct 
{
    const std::string filepath;                                                              
    const std::string property;
} path_property_str;

typedef struct 
{
	path_property_str path_prop[2];
} path_str;

typedef struct
{
	path_str cap_info;
} capture_str;

std::array<capture_str, 3> cap_array {{
	/* front */
	{"/run/arene/share/capture_front_by_id.bmp", "capture.screen.control.bitmap?ivi_id=0&path=",
	 "/run/arene/share/capture_front_by_name.bmp", "capture.screen.control.bitmap?name=front&path="},
	/* rear */
	{"/run/arene/share/capture_rear_by_id.bmp",    "capture.screen.control.bitmap?ivi_id=1&path=",
	 "/run/arene/share/capture_rear_by_name.bmp",  "capture.screen.control.bitmap?name=rear&path="},
	/* instrument cluster */
	{"/run/arene/share/capture_IC_by_id.bmp",      "capture.screen.control.bitmap?ivi_id=2&path=",
	 "/run/arene/share/capture_IC_by_name.bmp",    "capture.screen.control.bitmap?name=instrumentcluster&path="}
}};

/*** prototype ***/

/*** variables ***/
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static int32_t capture_result;
static cockpit::hal::video_hal::CtlObj Obj;

class VideoHAL_Screen_Capture: public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();
	static void Callback_CaptureScreenStatusBitmap();
protected:
};

/*** Callback ***/
void VideoHAL_Screen_Capture::Callback_CaptureScreenStatusBitmap()
{
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);

		/* Check vpath front availability again */
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "capture.screen.status.bitmap", &capture_result, sizeof(capture_result), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_CAPTURE_STS_SUCCESS	, capture_result);
	}

	cond_sync.notify_one();
}

void VideoHAL_Screen_Capture::SetUp()
{
	/* just in case */
	kill_test_video_client();
}

void VideoHAL_Screen_Capture::TearDown()
{
	kill_test_video_client();
}

//
// Googletest TestCase
//
TEST_F(VideoHAL_Screen_Capture, Lv1Normal014)
{
//	cockpit::hal::video_hal::CtlObj Obj={0, NULL};
	bool set_flag = false;
	bool intime = 0;
	std::string dsp_guide_str = "";
	struct stat st;
	std::string vpath_str;

	/* Halcp Indication */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Test No.14 Start\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Overview : Screen capture Test\n"));

	/* VideoHAL Initialize */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(Obj));
	VHAL_LOGI("/*** Test No.14 Start");

	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	/* Front path setting */
	set_flag = false;
	VHAL_LOGI("/*** phase01A set vpath.front.control.visible=%d", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	/* clear vpath */
	vpath_str = "";
	VHAL_LOGI("/*** phase02A set vpath.front.control.current=[%s]", vpath_str.c_str());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	exec_test_video_client(VHAL_CT_IVIID_CARPLAY, "sample_img_CARPLAY.png");
	/* Update vpath front */
	vpath_str = "CARPLAY";
	VHAL_LOGI("/*** phase03A set vpath.front.control.current=[%s]", vpath_str.c_str());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	set_flag = true;
	VHAL_LOGI("/*** phase04A set vpath.front.control.visible=%d", set_flag);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] The CARPLAY image is displayed on the front. \n"));

	if (false == get_exec_rear())
	{
		EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Test No.14 Skip(REAR)\n"));
		VHAL_LOGI("/*** Test No.14 Skip(REAR)");
	}
	else
	{
		/* Rear path setting */
		set_flag = false;
		VHAL_LOGI("/*** phase01B set vpath.front.control.visible=%d", set_flag);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.rear.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		/* clear vpath */
		vpath_str = "";
		VHAL_LOGI("/*** phase02B set vpath.front.control.current=[%s]", vpath_str.c_str());
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.rear.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		exec_test_video_client(VHAL_CT_IVIID_REAR_DTV, "sample_img_DTV.png");
		/* Update vpath rear */
		vpath_str = "DTV";
		VHAL_LOGI("/*** phase03B set vpath.front.control.current=[%s]", vpath_str.c_str());
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.rear.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		set_flag = true;
		VHAL_LOGI("/*** phase04B set vpath.front.control.visible=%d", set_flag);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.rear.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] The DTV image is displayed on the rear. \n"));
	}

	if (false == get_exec_ic())
	{
		EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Test No.14 Skip(Instrument Cluster)\n"));
		VHAL_LOGI("/*** Test No.14 Skip(Instrument Cluster)");
	}
	else
	{
		/* Instrument Cluster path setting */
		set_flag = false;
		VHAL_LOGI("/*** phase01C set vpath.front.control.visible=%d", set_flag);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		/* clear vpath */
		vpath_str = "";
		VHAL_LOGI("/*** phase02C set vpath.front.control.current=[%s]", vpath_str.c_str());
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		exec_test_video_client(VHAL_CT_IVIID_MAP, "sample_img_MAP.png");
		vpath_str = "MAP";
		VHAL_LOGI("/*** phase03C set vpath.front.control.current=[%s]", vpath_str.c_str());
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		set_flag = true;
		VHAL_LOGI("/*** phase04C set vpath.front.control.visible=%d", set_flag);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] The MAP image is displayed on the Instrument Cluster. \n"));
	}
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTANYKEY("[CertificationTest][Info] Please Input Any Key For Continue Test:"));

	/* ========== Test Start ========== */

	/* Screen capture(front, ivi_id specification) */
	for (std::size_t i = 0; i < cap_array.size(); ++i)
	{
		/* Check if it should be skipped(rear(FullRSE)). */
		if ((i == 1) && (false == get_exec_rear()))
		{
			continue;
		}
		/* Check if it should be skipped(Instrument cluster). */
		if ((i == 2) && (false == get_exec_ic()))
		{
			continue;
		}
		for (int j = 0; j < 2; ++j)
		{
			/* VideoHAL Register callback function */
			capture_result = VIDEO_HAL_VPATH_STS_FAILED;
			EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(Obj, "capture.screen.status.bitmap", Callback_CaptureScreenStatusBitmap));
			/* Screen capture request (one-shot event) */
			set_flag = true;
			VHAL_LOGI("/*** phase05%c idx=[%d] set %s%s=%d", 'A'+i, j, cap_array[i].cap_info.path_prop[j].property.c_str(), cap_array[i].cap_info.path_prop[j].filepath.c_str(), set_flag);
			EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, cap_array[i].cap_info.path_prop[j].property + cap_array[i].cap_info.path_prop[j].filepath,
					&set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
			EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
			/* wait callback */
			intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(VHAL_CT_TIMEOUT_SECONDS), [] { return (VIDEO_HAL_CAPTURE_STS_SUCCESS == capture_result); });
			EXPECT_EQ(true, intime);
			/* ClearCallback() */
			EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(Obj, "capture.screen.status.bitmap", Callback_CaptureScreenStatusBitmap));
			/* Check for the existence of file */
			memset(&st, 0x00, sizeof(st));
			EXPECT_EQ(0, stat(cap_array[i].cap_info.path_prop[j].filepath.c_str(), &st));
			EXPECT_EQ(S_IFREG, (st.st_mode & S_IFMT));
			dsp_guide_str = "Output to " + cap_array[i].cap_info.path_prop[j].filepath + " and hit any key:";
			EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTANYKEY(dsp_guide_str.c_str()));
		}
	}
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Check the contents of the capture ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* ========== Test End ========== */

	set_flag = false;
	vpath_str = "";
	/* Front path clear */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.front.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	if (true == get_exec_rear())
	{
		/* Rear path clear */
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.rear.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.rear.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	}
	if (true == get_exec_ic())
	{
		/* Instrument Cluster path clear */
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.visible", &set_flag, sizeof(set_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vpath.instrumentcluster.control.current", &vpath_str, (uint32_t)vpath_str.length()+1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
	}

	/* Deinit() */
	VHAL_LOGI("/*** Test No.14 Exit");
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(Obj));
}
