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
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static cockpit::hal::video_hal::CtlObj Obj;

class VideoHAL_Setting: public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();
protected:
	int32_t screen_width[3];
	int32_t screen_height[3];
	t_ilm_uint screenCount;
};

void VideoHAL_Setting::SetUp()
{
	ilmErrorTypes ret_ilm = ILM_SUCCESS;
	t_ilm_uint cnt = 0;
	t_ilm_uint *screenIds = NULL;
	struct ilmScreenProperties screenProp;

	memset(screen_width, 0, sizeof(screen_width));
	memset(screen_height, 0, sizeof(screen_height));

	ret_ilm = ilm_getScreenIDs(&screenCount, &screenIds);
	if (ILM_SUCCESS == ret_ilm)
	{
		if (3 < screenCount)
		{
			screenCount = 3;
		}
		for (cnt = 0; cnt < screenCount; cnt++)
		{
			switch(screenIds[cnt])
			{
				case VHAL_CT_FRONT_ID:
					ret_ilm = ilm_getPropertiesOfScreen(screenIds[cnt], &screenProp);
					if (ILM_SUCCESS == ret_ilm)
					{
						screen_width[0] = screenProp.screenWidth;
						screen_height[0] = screenProp.screenHeight;
						if (nullptr != screenProp.layerIds)
						{
							free(screenProp.layerIds);
						}
					}
					break;
				case VHAL_CT_REAR_ID:
					ret_ilm = ilm_getPropertiesOfScreen(screenIds[cnt], &screenProp);
					if (ILM_SUCCESS == ret_ilm)
					{
						screen_width[1] = screenProp.screenWidth;
						screen_height[1] = screenProp.screenHeight;
						if (nullptr != screenProp.layerIds)
						{
							free(screenProp.layerIds);
						}
					}
					break;
				case VHAL_CT_IC_ID:
					ret_ilm = ilm_getPropertiesOfScreen(screenIds[cnt], &screenProp);
					if (ILM_SUCCESS == ret_ilm)
					{
						screen_width[2] = screenProp.screenWidth;
						screen_height[2] = screenProp.screenHeight;
						if (nullptr != screenProp.layerIds)
						{
							free(screenProp.layerIds);
						}
					}
					break;
				default:
					/* Nothing to do */
					break;
			}
		}
	}
	if (NULL != screenIds)
	{
		free(screenIds);
	}

	set_hmi_visibility(false);

	/* just in case */
	kill_test_video_client();
}

void VideoHAL_Setting::TearDown()
{
	set_hmi_visibility(true);
}

//
// Googletest TestCase
//
TEST_F(VideoHAL_Setting, Lv1Normal010)
{
	int32_t get_w = 0;
	int32_t get_h = 0;
	bool get_flag = false;
	int32_t get_value = 0;
	int32_t set_value = 0;

	/* Halcp Indication */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Test No.10 Start\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Overview : VideoHAL Setting Test\n"));

	/* Check screen size */
	/* Check width, height of front */
	EXPECT_GT(screen_width[0], 0);
	EXPECT_GT(screen_height[0], 0);

	/* Check width, height of rear(FullRSE) */
	if (true == get_exec_rear())
	{
		EXPECT_GT(screen_width[1], 0);
		EXPECT_GT(screen_height[1], 0);
	}
	else
	{
		EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Test No.10 Skip(rear(FullRSE))\n"));
	}

	/* Check width, height of Instrument Cluster */
	if (true == get_exec_ic())
	{
		EXPECT_GT(screen_width[2], 0);
		EXPECT_GT(screen_height[2], 0);
	}
	else
	{
		EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Test No.10 Skip(Instrument Cluster)\n"));
	}

	/* VideoHAL Initialize */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(Obj));
	VHAL_LOGI("/*** Test No.10 Start");

	/* ========== Test Start ========== */

	/* ----- Front Display Status Check ----- */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.display.front", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(true, get_flag);
	if (true == get_flag)
	{
		/* ----- Front Display Resolution Test ----- */
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.display.front.width", &get_w, sizeof(get_w), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(get_w, screen_width[0]);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.display.front.height", &get_h, sizeof(get_h), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(get_h, screen_height[0]);
	}

	if (true == get_exec_rear())
	{
		/* ----- Rear Display Status Check ----- */
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.display.rear", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		if (true == get_flag)
		{
			/* ----- Rear Display Resolution Test ----- */
			EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.display.rear.width", &get_w, sizeof(get_w), VIDEO_HAL_DATA_TYPE_NUM));
			EXPECT_EQ(get_w, screen_width[1]);
			EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.display.rear.height", &get_h, sizeof(get_h), VIDEO_HAL_DATA_TYPE_NUM));
			EXPECT_EQ(get_h, screen_height[1]);
		}
	}

	if (true == get_exec_ic())
	{
		/* ----- Instrument Cluster Display Status Check ----- */
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.display.instrumentcluster", &get_flag, sizeof(get_flag), VIDEO_HAL_DATA_TYPE_BOOL));
		if (true == get_flag)
		{
			/* ----- Instrument Cluster Display Resolution Test ----- */
			EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.display.instrumentcluster.width", &get_w, sizeof(get_w), VIDEO_HAL_DATA_TYPE_NUM));
			EXPECT_EQ(get_w, screen_width[2]);
			EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.display.instrumentcluster.height", &get_h, sizeof(get_h), VIDEO_HAL_DATA_TYPE_NUM));
			EXPECT_EQ(get_h, screen_height[2]);
		}
	}

	if (false == get_exec_camera())
	{
		EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Test No.10 Skip(CAMERA)\n"));
	}
	else
	{
		/* ----- Camera type value Check ----- */
		/* Get current camera type value */
		get_value = VIDEO_HAL_CONNECTED_CAMERA_INVALID;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.connected.camera", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
		/* Set display type if default value */
		if (VIDEO_HAL_CONNECTED_CAMERA_INVALID == get_value)
		{
			/* Set the connected camera type */
			set_value = VIDEO_HAL_CONNECTED_CAMERA_DPVM_NORMAL;
			EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vhal.setting.control.connected.camera", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
			EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
			/* Get current camera type value */
			get_value = VIDEO_HAL_CONNECTED_CAMERA_INVALID;
			EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.connected.camera", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
			EXPECT_EQ(set_value, get_value);		
		}
	}

	if (true == get_exec_rear())
	{
		/* -----  RSE type value Check ----- */
		/* Get current RSE type value */
		get_value = VIDEO_HAL_CONNECTED_RSE_INVALID;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.connected.rse", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
		if (VIDEO_HAL_CONNECTED_RSE_INVALID == get_value)
		{
			/* Set the connected RSE type */
			set_value = VIDEO_HAL_CONNECTED_RSE_FULL;
			EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(Obj, "vhal.setting.control.connected.rse", &set_value, sizeof(set_value), VIDEO_HAL_DATA_TYPE_NUM));
			EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(Obj));
			/* Get current RSE type value */
			get_value = VIDEO_HAL_CONNECTED_RSE_INVALID;
			EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "vhal.setting.status.connected.rse", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
			EXPECT_EQ(set_value, get_value);		
		}
	}

	/* ========== Test End ========== */

	/* Deinit() */
	VHAL_LOGI("/*** Test No.10 Exit");
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(Obj));
}
