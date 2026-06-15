#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static uint32_t status_brightness;
static uint32_t status_brightness_expect;
static uint32_t status_contrast;
static uint32_t status_contrast_expect;
static bool status_forced_hmi_img_adj;
static bool status_forced_hmi_img_adj_expect;
static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;

static int32_t status_current;
static int32_t status_current_expect;
static void CallbackCurrent(void)
{
	std::lock_guard<std::mutex> lock_sync(mtx_sync);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.front.status.current", &status_current, sizeof(status_current), VIDEO_HAL_DATA_TYPE_NUM));
	//EXPECT_EQ(status_current_expect, status_current);
	cond_sync.notify_one();
}

static bool status_visible;
static bool status_visible_expect;
static void CallbackVisible(void)
{
	std::lock_guard<std::mutex> lock_sync(mtx_sync);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.visible", &status_visible, sizeof(status_visible), VIDEO_HAL_DATA_TYPE_BOOL));
	//EXPECT_EQ(status_visible_expect, status_visible);
	cond_sync.notify_one();
}

static bool RepeatHdmi(cockpit::hal::video_hal::CtlObj& ctl_obj, std::unique_lock<std::mutex>& unique_lock)
{
	bool ret{true};
	std::string s;
	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	s = "";
	if (VIDEO_HAL_API_SUCCESS != cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &s, s.length() + 1, VIDEO_HAL_DATA_TYPE_STR))
		{ret = false; PrintMsg("\n ERROR 1 \n");}
	if (VIDEO_HAL_API_SUCCESS != cockpit::hal::video_hal::RequestUpdate(ctl_obj))
		{ret = false; PrintMsg("\n ERROR 2 \n");}
	if (!cond_sync.wait_for(unique_lock, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); }))
		{ret = false; PrintMsg("\n ERROR 3 \n");}
	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	s = "HDMI";
	if (VIDEO_HAL_API_SUCCESS != cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &s, s.length() + 1, VIDEO_HAL_DATA_TYPE_STR))
		{ret = false; PrintMsg("\n ERROR 4 \n");}
	if (VIDEO_HAL_API_SUCCESS != cockpit::hal::video_hal::RequestUpdate(ctl_obj))
		{ret = false; PrintMsg("\n ERROR 5 \n");}
	if (!cond_sync.wait_for(unique_lock, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); }))
		{ret = false; PrintMsg("\n ERROR 6 \n");}
	bool visible = true;
	status_visible_expect = true;
	status_visible = !status_visible_expect;
	if (VIDEO_HAL_API_SUCCESS != cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &visible, sizeof(visible), VIDEO_HAL_DATA_TYPE_BOOL))
		{ret = false; PrintMsg("\n ERROR 7 \n");}
	if (VIDEO_HAL_API_SUCCESS != cockpit::hal::video_hal::RequestUpdate(ctl_obj))
		{ret = false; PrintMsg("\n ERROR 8 \n");}
	if (!cond_sync.wait_for(unique_lock, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); }))
		{ret = false; PrintMsg("\n ERROR 9 \n");}
	return ret;
}

static int32_t status_connected_camera;
static int32_t status_connected_camera_expect;
static void CallbackConnectedCamera(void)
{
	std::lock_guard<std::mutex> lock_sync(mtx_sync);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.connected.camera", &status_connected_camera, sizeof(status_connected_camera), VIDEO_HAL_DATA_TYPE_NUM));
	//EXPECT_EQ(status_connected_camera_expect, status_connected_camera);
	cond_sync.notify_one();
}

static bool RepeatCamera(cockpit::hal::video_hal::CtlObj& ctl_obj, std::unique_lock<std::mutex>& unique_lock)
{
	bool ret{true};
	int32_t connected_camera;
	connected_camera = 11;
	status_connected_camera_expect = connected_camera;
	status_connected_camera = status_connected_camera_expect + 1;
	if (VIDEO_HAL_API_SUCCESS != cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.connected.camera", &connected_camera, sizeof(connected_camera), VIDEO_HAL_DATA_TYPE_NUM))
		{ret = false; PrintMsg("\n ERROR 1 \n");}
	if (VIDEO_HAL_API_SUCCESS != cockpit::hal::video_hal::RequestUpdate(ctl_obj))
		{ret = false; PrintMsg("\n ERROR 2 \n");}
	if (!cond_sync.wait_for(unique_lock, std::chrono::seconds(timeout_seconds), [] { return (status_connected_camera_expect == status_connected_camera); }))
		{ret = false; PrintMsg("\n ERROR 3 \n");}
	connected_camera = 10;
	status_connected_camera_expect = connected_camera;
	status_connected_camera = status_connected_camera_expect + 1;
	if (VIDEO_HAL_API_SUCCESS != cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.connected.camera", &connected_camera, sizeof(connected_camera), VIDEO_HAL_DATA_TYPE_NUM))
		{ret = false; PrintMsg("\n ERROR 4 \n");}
	if (VIDEO_HAL_API_SUCCESS != cockpit::hal::video_hal::RequestUpdate(ctl_obj))
		{ret = false; PrintMsg("\n ERROR 5 \n");}
	if (!cond_sync.wait_for(unique_lock, std::chrono::seconds(timeout_seconds), [] { return (status_connected_camera_expect == status_connected_camera); }))
		{ret = false; PrintMsg("\n ERROR 6 \n");}
	return ret;
}

TEST(VhalFunctionTest, Repeat1000Hdm)
{
	std::unique_lock<std::mutex> lock(mtx_sync);
	cockpit::hal::video_hal::Init(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.current", CallbackCurrent));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.visible", CallbackVisible));
	for (int i = 1; i <= 1000; i ++)
	{
		if (!RepeatHdmi(obj, lock))
			break;
		else if (i % 100 == 0)
			PrintMsg(std::to_string(i) + "\n");
	}
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.current", CallbackCurrent));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.visible", CallbackVisible));
	cockpit::hal::video_hal::Deinit(obj);
}

TEST(VhalFunctionTest, Repeat1000Cam)
{
	std::unique_lock<std::mutex> lock(mtx_sync);
	cockpit::hal::video_hal::Init(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.connected.camera", CallbackConnectedCamera));
	for (int i = 1; i <= 1000; i ++)
	{
		if (!RepeatCamera(obj, lock))
			break;
		else if (i % 100 == 0)
			PrintMsg(std::to_string(i) + "\n");
	}
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.connected.camera", CallbackConnectedCamera));
	cockpit::hal::video_hal::Deinit(obj);
}
