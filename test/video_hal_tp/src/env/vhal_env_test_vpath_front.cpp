#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};

/**
 * 脱落/通信途絶
 */
TEST(VhalEnvTestVpathFrontDisconnection, ConnectHdmi)
{
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_HDMI));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * 脱落/通信途絶
 * 初期状態:HDMI接続
 */
TEST(VhalEnvTestVpathFrontDisconnection, ConnectAndDisconnectHdmi)
{
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_HDMI));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * 異常データ取得
 * 初期状態:HDMI接続
 */
TEST(VhalEnvTestVpathFrontReceiveHdmiVideoFormat, ReceiveHdmiVideoFormat)
{
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_HDMI));
	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetWideModeFront(obj, VIDEO_HAL_WIDE_MODE_NORMAL));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * デバイス単独リセット
 * 初期状態:HDMI機器電源ON
 */
TEST(VhalEnvTestVpathFrontReset, Hdmi)
{
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_HDMI));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));	
}