#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};

/**
 * 脱落/通信途絶
 */
TEST(VhalEnvTestVpathIcDisconnection, ConnectIc)
{
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));

	ExecuteRenderImage(obj, SURFACE_ID_IC_MAP);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, VPATH_NAME_MAP));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleIc(obj, true));

	PrintMsg("Press enter to exit.");
	std::cin.get();
	KillAllRenderImage(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * 脱落/通信途絶
 * 初期状態:IC接続
 */
TEST(VhalEnvTestVpathIcDisconnection, ConnectAndDisconnectIc)
{
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));

	ExecuteRenderImage(obj, SURFACE_ID_IC_MAP);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, VPATH_NAME_MAP));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleIc(obj, true));

	PrintMsg("Press enter to exit.");
	std::cin.get();
	KillAllRenderImage(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * デバイス単独リセット
 * 初期状態:ICON
 */
TEST(VhalEnvTestVpathIcReset, Ic)
{
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));

	ExecuteRenderImage(obj, SURFACE_ID_IC_MAP);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, VPATH_NAME_MAP));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleIc(obj, true));

	PrintMsg("Press enter to exit.");
	std::cin.get();
	KillAllRenderImage(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));	
}
