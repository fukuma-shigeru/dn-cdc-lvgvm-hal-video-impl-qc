#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static int32_t status_capture_bitmap;
static int32_t status_capture_bitmap_expect;
static const int screen_timeout_seconds  = CB_SCREEN_CAPTURE_TIMEOUT_SECOUNDS;
static const int surface_timeout_seconds = CB_TIMEOUT_SECOUNDS;

static void CallbackCapture(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "capture.screen.status.bitmap", &status_capture_bitmap, sizeof(status_capture_bitmap), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_capture_bitmap_expect, status_capture_bitmap);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackSurfaceCapture(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "capture.surface.status.bitmap", &status_capture_bitmap, sizeof(status_capture_bitmap), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_capture_bitmap_expect, status_capture_bitmap);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

struct VhalSeqTestCaptureParameter {
	int32_t screen_id;
	std::string screen_name;
	int32_t surface_id;
};

struct VhalSeqTestSurfaceCaptureParameter {
	int32_t surface_id;
	std::string surface_name;
	std::string screen_capture_name;
};

class VhalSeqTestCaptureApiSpecification : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestCaptureParameter> {};
class VhalSeqTestSurfaceCaptureApiSpecification : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestSurfaceCaptureParameter> {};

INSTANTIATE_TEST_CASE_P(VhalSeqTestCapture, VhalSeqTestCaptureApiSpecification, ::testing::Values(
	VhalSeqTestCaptureParameter{SCREEN_ID_FRONT, SCREEN_NAME_FRONT, SURFACE_ID_FRONT_DTV},
	VhalSeqTestCaptureParameter{SCREEN_ID_REAR,  SCREEN_NAME_REAR,  SURFACE_ID_REAR_DTV},
	VhalSeqTestCaptureParameter{SCREEN_ID_IC,    SCREEN_NAME_IC,    SURFACE_ID_IC_MAP},
	VhalSeqTestCaptureParameter{SCREEN_ID_HUD,   SCREEN_NAME_HUD,   SURFACE_ID_HUD_HUD}
));

INSTANTIATE_TEST_CASE_P(VhalSeqTestCapture, VhalSeqTestSurfaceCaptureApiSpecification, ::testing::Values(
	VhalSeqTestSurfaceCaptureParameter{SURFACE_ID_FRONT_DTV, SURFACE_NAME_FRONT_DTV, VPATH_NAME_DTV},
	VhalSeqTestSurfaceCaptureParameter{SURFACE_ID_REAR_DTV, SURFACE_NAME_REAR_DTV, VPATH_NAME_DTV},
	VhalSeqTestSurfaceCaptureParameter{SURFACE_ID_IC_MAP, SURFACE_NAME_IC_MAP, VPATH_NAME_MAP}
));

/**
 * API仕様書記載のシーケンス(front,rear,IC,HUD)
 * 画面キャプチャ
 */
TEST_P(VhalSeqTestCaptureApiSpecification, GetCapture)
{
	VhalSeqTestCaptureParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool request{};
	std::string path{};

	cockpit::hal::video_hal::Init(obj);

	/* HUDの場合、映像パス設定が無いのでExecuteRenderImagerのみ実行 */
	/* HUDサーフェスの場合、surface-visibility=visibleの為、サーフェス生成直後に表示される */
	if (SCREEN_ID_HUD == param.screen_id)
	{
		EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(param.surface_id));
	}
	else
	{
		EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));
		EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, param.screen_name));
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	/* 画面キャプチャ[ID指定] */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_FAILED;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->name();
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/capture_iviid_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.screen.control.bitmap?ivi_id=" + std::to_string(param.screen_id) + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for capture bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(screen_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);	

	/* 画面キャプチャ[NAME指定] */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_FAILED;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	request = true;
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/capture_name_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.screen.control.bitmap?name=" + param.screen_name + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for capture bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(screen_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作(frontのみ)
 * 画面キャプチャ
 */
TEST(VhalSeqTestCaptureRepeat, GetCapture)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool request{};
	std::string path{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	/* 画面キャプチャ[ID指定] */
	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_FAILED;
		status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_SUCCESS;
		request = true;
		const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
		std::string test_name = test_info->name();
		std::replace(test_name.begin(), test_name.end(), '/', '_');
		path = "/tmp/capture_iviid_" + test_name + ".bmp";
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.screen.control.bitmap?ivi_id=0&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for capture bitmap.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(screen_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
		EXPECT_EQ(true, intime);
	}

	/* 画面キャプチャ[NAME指定] */
	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_FAILED;
		status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_SUCCESS;
		request = true;
		const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
		std::string test_name = test_info->name();
		std::replace(test_name.begin(), test_name.end(), '/', '_');
		path = "/tmp/capture_" + test_name + ".bmp";
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.screen.control.bitmap?name=front&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for capture bitmap.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(screen_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.screen.status.bitmap", CallbackCapture));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * API仕様書記載のシーケンス
 * サーフェスキャプチャ
 */
TEST_P(VhalSeqTestSurfaceCaptureApiSpecification, GetCapture)
{
	VhalSeqTestSurfaceCaptureParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool request{};
	std::string path{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCapture));

	std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));

	/* サーフェスキャプチャ[ID指定] */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_FAILED;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	request = true;
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::string test_name = test_info->name();
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	path = "/tmp/capture_surface_id_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.surface.control.bitmap?ivi_id=" + std::to_string(param.surface_id) + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for capture surface bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(surface_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);	

	/* サーフェスキャプチャ[NAME指定] */
	status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_FAILED;
	status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_SUCCESS;
	request = true;
	path = "/tmp/capture_surface_name_" + test_name + ".bmp";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.surface.control.bitmap?name=" + param.surface_name + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for capture surface bitmap.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(surface_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCapture));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 * サーフェスキャプチャ
 */
TEST(VhalSeqTestSurfaceCaptureRepeat, GetCapture)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool request{};
	std::string path{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCapture));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_FAILED;
		status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_SUCCESS;
		request = true;
		const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
		std::string test_name = test_info->name();
		std::replace(test_name.begin(), test_name.end(), '/', '_');
		path = "/tmp/capture_surface_id_" + test_name + "_" + std::to_string(i) + ".bmp";
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "capture.surface.control.bitmap?ivi_id=" + std::to_string(SURFACE_ID_FRONT_DTV) + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for capture surface bitmap.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(surface_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
		EXPECT_EQ(true, intime);
	}

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		status_capture_bitmap = VIDEO_HAL_CAPTURE_STS_FAILED;
		status_capture_bitmap_expect = VIDEO_HAL_CAPTURE_STS_SUCCESS;
		request = true;
		const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
		std::string test_name = test_info->name();
		std::replace(test_name.begin(), test_name.end(), '/', '_');
		path = "/tmp/capture_surface_name_" + test_name + "_" + std::to_string(i) + ".bmp";
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, std::string("capture.surface.control.bitmap?name=") + SURFACE_NAME_FRONT_DTV + "&path=" + path, &request, sizeof(request), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for capture surface bitmap.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(surface_timeout_seconds), [] { return (status_capture_bitmap_expect == status_capture_bitmap); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "capture.surface.status.bitmap", CallbackSurfaceCapture));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}
