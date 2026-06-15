#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};

/**
 * 脱落/通信途絶
 */
TEST(VhalEnvTestVpathRearDisconnection, ConnectRear)
{

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));

	ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleRear(obj, true));

	PrintMsg("Press enter to exit.");
	std::cin.get();
	KillAllRenderImage(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * 脱落/通信途絶
 */
TEST(VhalEnvTestVpathRearDisconnection, ConnectAndDisconnectRear)
{
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));

	ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleRear(obj, true));

	PrintMsg("Press enter to exit.");
	std::cin.get();
	KillAllRenderImage(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * デバイス単独リセット
 */
TEST(VhalEnvTestVpathRearReset, Rear)
{
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));

	ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleRear(obj, true));

	PrintMsg("Press enter to exit.");
	std::cin.get();
	KillAllRenderImage(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));	
}