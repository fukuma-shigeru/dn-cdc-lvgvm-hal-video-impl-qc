#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;

static std::string cb_available_param;
static bool status_available;
static bool status_available_expect;

static int32_t status_current;
static int32_t status_current_expect;

static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;

static void callback_current(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.camera.status.current", &status_current, sizeof(status_current), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_current_expect, status_current);
	}

	cond_sync.notify_one();
	PrintMsg("end.");
}

static void callback_available(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.camera.status.available?vpath=" + cb_available_param, &status_available, sizeof(status_available), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(status_available_expect, status_available);
	}

	cond_sync.notify_one();
	PrintMsg("end.");
}

struct VhalSeqTestVpathCameraParameter {
	int32_t  dummy;
};

class VhalSeqTestVpathCameraApiSpecification : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestVpathCameraParameter> {};
class VhalSeqTestVpathCameraSkip : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestVpathCameraParameter> {};

INSTANTIATE_TEST_CASE_P(VhalSeqTestVpathCamera, VhalSeqTestVpathCameraApiSpecification, ::testing::Values(
	VhalSeqTestVpathCameraParameter{0}
));

INSTANTIATE_TEST_CASE_P(VhalSeqTestVpathCamera, VhalSeqTestVpathCameraSkip, ::testing::Values(
	VhalSeqTestVpathCameraParameter{0}
));

/**
 * API仕様書記載のシーケンス
 * カメラ電源ON
 */
TEST_P(VhalSeqTestVpathCameraApiSpecification, CameraOn)
{
	VhalSeqTestVpathCameraParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;
    int32_t value{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.current", callback_current));

    /* 1. カメラ種別設定 */
#if 0 /* NOP_CONNECTED_CAMERA */
	value = VIDEO_HAL_CONNECTED_CAMERA_DPVM_FULLHD;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.connected.camera", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
#endif /* NOP_CONNECTED_CAMERA */
	/* 9. カメラ設定可能チェック */
	available = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.camera.status.available?vpath=CAMERA", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(true, available);

    /* 15. カメラ映像パス名設定 */
	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	vpath = "CAMERA";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.current", callback_current));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * API仕様書記載のシーケンス
 * カメラ電源OFF
 */
TEST_P(VhalSeqTestVpathCameraApiSpecification, CameraOff)
{
	VhalSeqTestVpathCameraParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;
    int32_t value{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

#if 0 /* NOP_CONNECTED_CAMERA */
    EXPECT_EQ(VHAL_TEST_SUCCESS, SetConnectedCamera(obj, VIDEO_HAL_CONNECTED_CAMERA_NONE));
#endif /* NOP_CONNECTED_CAMERA */

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.current", callback_current));

    /* 1. カメラ種別設定 */
#if 0 /* NOP_CONNECTED_CAMERA */
    EXPECT_EQ(VHAL_TEST_SUCCESS, SetConnectedCamera(obj, VIDEO_HAL_CONNECTED_CAMERA_NONE));
#endif /* NOP_CONNECTED_CAMERA */

	/* 9. カメラ設定可能チェック */
	available = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.camera.status.available?vpath=CAMERA", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(true, available);

    /* 15. カメラ映像パス名設定 */
	status_current_expect = VIDEO_HAL_VPATH_STS_FAILED;
	status_current = VIDEO_HAL_VPATH_STS_SUCCESS;
	vpath = "CAMERA";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.current", callback_current));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * 連続動作
 * カメラ電源ON
 */
TEST(VhalSeqTestVpathCameraRepeat, CameraOn)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;
    int32_t value{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.current", callback_current));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		/* 1. カメラ種別設定 */
#if 0 /* NOP_CONNECTED_CAMERA */
		value = VIDEO_HAL_CONNECTED_CAMERA_DPVM_FULLHD;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.connected.camera", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
#endif /* NOP_CONNECTED_CAMERA */
		/* 9. カメラ設定可能チェック */
		available = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.camera.status.available?vpath=CAMERA", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(true, available);

		/* 15. カメラ映像パス名設定 */
		status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
		status_current = VIDEO_HAL_VPATH_STS_FAILED;
		vpath = "CAMERA";
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.current", callback_current));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * 連続動作
 * カメラ電源OFF
 */
TEST(VhalSeqTestVpathCameraRepeat, CameraOff)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;
    int32_t value{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

#if 0 /* NOP_CONNECTED_CAMERA */
    EXPECT_EQ(VHAL_TEST_SUCCESS, SetConnectedCamera(obj, VIDEO_HAL_CONNECTED_CAMERA_NONE));
#endif /* NOP_CONNECTED_CAMERA */

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.current", callback_current));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		/* BEVstep3 実際のカメラ接続状況に合わせる */
		bool except_available{false};
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.connected.camera", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM));
		if ((VIDEO_HAL_CONNECTED_CAMERA_INVALID == value) || (VIDEO_HAL_CONNECTED_CAMERA_NONE == value))
		{
			except_available = false;
		}
		else
		{
			except_available = true;
		}

		/* 9. カメラ設定可能チェック */
		available = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.camera.status.available?vpath=CAMERA", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(except_available, available);

		/* 15. カメラ映像パス名設定 */
		status_current_expect = VIDEO_HAL_VPATH_STS_FAILED;
		status_current = VIDEO_HAL_VPATH_STS_SUCCESS;
		vpath = "CAMERA";
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.current", callback_current));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * API仕様書記載のシーケンス
 * カメラ電源ON and OFF
 */
TEST(VhalSeqTestVpathCameraApiSpecification, CameraOnOff)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;
    int32_t value{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

#if NOP_CONNECTED_CAMERA
    EXPECT_EQ(VHAL_TEST_SUCCESS, SetConnectedCamera(obj, VIDEO_HAL_CONNECTED_CAMERA_NONE));
#endif /* NOP_CONNECTED_CAMERA */

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.current", callback_current));

    /* 1. カメラ種別設定 */
#if NOP_CONNECTED_CAMERA
	status_available_expect = true;
	status_available = false;
	cb_available_param = "";
	value = VIDEO_HAL_CONNECTED_CAMERA_DPVM_FULLHD;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.connected.camera", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
	EXPECT_EQ(true, intime);
#endif /* NOP_CONNECTED_CAMERA */

	/* 9. カメラ設定可能チェック */
	available = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.camera.status.available?vpath=CAMERA", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(true, available);

    /* 15. カメラ映像パス名設定 */
	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	vpath = "CAMERA";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });

	/* 4秒表示 */
	std::this_thread::sleep_for(std::chrono::milliseconds(4000));

    /* 15. カメラ映像パス名設定(停止) */
	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	vpath = "";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });

    /* 1. カメラ種別設定 */
#if NOP_CONNECTED_CAMERA
	status_available_expect = true;
	status_available = false;
	cb_available_param = "";
	value = VIDEO_HAL_CONNECTED_CAMERA_NONE;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.connected.camera", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
	EXPECT_EQ(true, intime);
#endif /* NOP_CONNECTED_CAMERA */

	/* 9. カメラ設定可能チェック */
	available = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.camera.status.available?vpath=CAMERA", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(true, available);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.current", callback_current));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * 連続動作
 * API仕様書記載のシーケンス
 * カメラ電源ON and OFF
 */
TEST(VhalSeqTestVpathCameraRepeat, CameraOnOff)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;
    int32_t value{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

#if NOP_CONNECTED_CAMERA
    EXPECT_EQ(VHAL_TEST_SUCCESS, SetConnectedCamera(obj, VIDEO_HAL_CONNECTED_CAMERA_NONE));
#endif /* NOP_CONNECTED_CAMERA */

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.current", callback_current));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{

	    /* 1. カメラ種別設定 */
#if NOP_CONNECTED_CAMERA
		status_available_expect = true;
		status_available = false;
		cb_available_param = "";
		value = VIDEO_HAL_CONNECTED_CAMERA_DPVM_FULLHD;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.connected.camera", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
		EXPECT_EQ(true, intime);
#endif /* NOP_CONNECTED_CAMERA */

		/* 9. カメラ設定可能チェック */
		available = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.camera.status.available?vpath=CAMERA", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(true, available);

	    /* 15. カメラ映像パス名設定 */
		status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
		status_current = VIDEO_HAL_VPATH_STS_FAILED;
		vpath = "CAMERA";
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });

		/* 1秒表示 */
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	    /* 15. カメラ映像パス名設定(停止) */
		status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
		status_current = VIDEO_HAL_VPATH_STS_FAILED;
		vpath = "";
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });

	    /* 1. カメラ種別設定 */
#if NOP_CONNECTED_CAMERA
		status_available_expect = true;
		status_available = false;
		cb_available_param = "";
		value = VIDEO_HAL_CONNECTED_CAMERA_NONE;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.connected.camera", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
		EXPECT_EQ(true, intime);
#endif /* NOP_CONNECTED_CAMERA */

		/* 9. カメラ設定可能チェック */
		available = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.camera.status.available?vpath=CAMERA", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(true, available);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.current", callback_current));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * シーケンス抜け
 * 映像パス設定可否判定を取得せずに映像パスを切替える
 */
TEST_P(VhalSeqTestVpathCameraSkip, Skip)
{
	VhalSeqTestVpathCameraParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;
    int32_t value{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.current", callback_current));

    /* 1. カメラ種別設定 */
#if 0 /* NOP_CONNECTED_CAMERA */
	value = VIDEO_HAL_CONNECTED_CAMERA_DPVM_FULLHD;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.connected.camera", &value, sizeof(value), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
#endif /* NOP_CONNECTED_CAMERA */
    /* 15. カメラ映像パス名設定 */
	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	vpath = "CAMERA";
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.current", callback_current));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * 設計書記載のシーケンス
 * カメラ同期検知 
 * 同期異常(正常→異常)
 */
TEST(VhalSeqTestVpathCameraSeqSpecification, CameraSyncNo)
{
	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CAMERA));

	DetectCameraSync(obj, CAM_SYNC_NO);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設計書記載のシーケンス
 * カメラ同期検知 
 * 同期復帰(異常→正常)
 */
TEST(VhalSeqTestVpathCameraSeqSpecification, CameraSync)
{
	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CAMERA));

	DetectCameraSync(obj, CAM_SYNC_NO);
	DetectCameraSync(obj, CAM_SYNC);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設計書記載のシーケンス
 * 種別判別通知
 */
TEST(VhalSeqTestVpathCameraSeqSpecification, CameraType)
{
	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CAMERA));

	NotifyCameraType(obj, CAM_SIZE_1280_635);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 * 設計書記載のシーケンス
 * カメラ同期検知 
 * 同期異常(正常→異常)
 */
TEST(VhalSeqTestVpathCameraRepeat, CameraSyncNo)
{
	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CAMERA));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		DetectCameraSync(obj, CAM_SYNC_NO);
		DetectCameraSync(obj, CAM_SYNC);
	}

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 * 設計書記載のシーケンス
 * 種別判別通知
 */
TEST(VhalSeqTestVpathCameraRepeat, CameraType)
{
	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CAMERA));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		NotifyCameraType(obj, CAM_SIZE_1280_635);
	}

	cockpit::hal::video_hal::Deinit(obj);
}