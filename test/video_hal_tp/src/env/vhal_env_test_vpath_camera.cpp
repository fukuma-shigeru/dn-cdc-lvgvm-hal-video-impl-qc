#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};

/**
 * 脱落/通信途絶
 */
TEST(VhalEnvTestVpathCameraDisconnection, ConnectCamera)
{
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CAMERA));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * 脱落/通信途絶
 */
TEST(VhalEnvTestVpathCameraDisconnection, ConnectAndDisconnectCamera)
{
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CAMERA));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * デバイス単独リセット
 */
TEST(VhalEnvTestVpathCameraReset, Camera)
{
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CAMERA));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * 異常検知
 * カメラ同期無し
 */
TEST(VhalEnvTestVpathCameraRecieve, NoSync)
{
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));

	EXPECT_EQ(VHAL_TEST_SUCCESS, StartHomeScreen());

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CAMERA));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetCameraOutput(obj, 100, 100, 600, 500, true));

	PrintMsg("Press enter to exit.");
	std::cin.get();
	
	EXPECT_EQ(VHAL_TEST_SUCCESS, StopHomeScreen());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}