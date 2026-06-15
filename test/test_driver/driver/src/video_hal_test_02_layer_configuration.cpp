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

/*** define ***/

/*** prototype ***/

/*** variables ***/
static cockpit::hal::video_hal::CtlObj Obj;

class VideoHAL_Layer_Configuration: public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();
};

void VideoHAL_Layer_Configuration::SetUp()
{
	/* just in case */
	kill_test_video_client();
}

void VideoHAL_Layer_Configuration::TearDown()
{
	kill_test_video_client();
}

//
// Googletest TestCase
//
TEST_F(VideoHAL_Layer_Configuration, Lv1Normal002)
{
//	cockpit::hal::video_hal::CtlObj Obj={0, NULL};
	int32_t get_value = 0;

	/* Halcp Indication */
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Test No.2 Start\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] /*** Overview : Layer Configuration Test\n"));

	/* VideoHAL Initialize */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(Obj));
	VHAL_LOGI("/*** Test No.2 Start");

	/* ========== Test Start ========== */

	VHAL_LOGI("/*** phase01 get ivi.layout.status.configuration.file");
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(Obj, "ivi.layout.status.configuration.file", &get_value, sizeof(get_value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_IVI_LAYOUT_STS_SUCCESS, get_value);

	VHAL_LOGI("/*** phase02 check all layers(ivi_id=1-8,500,505,507,510,515,600,605) created and added to the screen(ivi_id=0)");
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] Please check ivi object tree with LayerManagerControl command.\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Are all layers(ivi_id=1-8,500,505,507,510,515,600,605) created and added to the screen(ivi_id=0)? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* start test video client which makes the CARPLAY surface */
	exec_test_video_client(VHAL_CT_IVIID_CARPLAY, "sample_img_CARPLAY.png");

	VHAL_LOGI("/*** phase03 check the surface(ivi_id=31) added to the layer(ivi_id=6)");
	EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest][Info] Please check ivi object tree again.\n"));
	EXPECT_EQ(HALCP_RET_OK, HALCP_WAITINPUTRESULT("[CertificationTest][Info] Is the surface(ivi_id=31) added to the layer(ivi_id=6) ? ('y' or 'Y' is OK, Others are NG):", nullptr));

	/* ========== Test End ========== */

	/* Deinit() */
	VHAL_LOGI("/*** Test No.2 Exit");
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(Obj));
}
