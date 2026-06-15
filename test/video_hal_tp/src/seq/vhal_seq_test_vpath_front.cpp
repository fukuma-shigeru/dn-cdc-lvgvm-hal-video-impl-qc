#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;

static std::string cb_available_param;
static bool status_available;
static bool status_available_expect;

static bool status_visible;
static bool status_visible_expect;

static int32_t status_current;
static int32_t status_current_expect;

static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;

static int32_t get_default_videoformat(void)
{
	int32_t	format{SYSINFO_CONN_INFO_VIDEO_FORMAT_1920X1080P};
	if (MKIND_X86 == GetMachine())
	{
		format = SYSINFO_CONN_INFO_VIDEO_FORMAT_1280X720P;
	}
	return format;
}

static void callback_current(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.front.status.current", &status_current, sizeof(status_current), VIDEO_HAL_DATA_TYPE_NUM));
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
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.front.status.available?vpath=" + cb_available_param, &status_available, sizeof(status_available), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(status_available_expect, status_available);
	}

	cond_sync.notify_one();
	PrintMsg("end.");
}

static void callback_visible(void)
{
	PrintMsg("start.");
	{
// WARNING		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.front.status.visible", &status_visible, sizeof(status_visible), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(status_visible_expect, status_visible);
	}

	cond_sync.notify_one();
	PrintMsg("end.");
}

struct VhalSeqTestVpathFrontParameter {
	int32_t surface_id;
	std::string front_vpath_name;
};

class VhalSeqTestVpathFrontApiSpecification : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestVpathFrontParameter> {};
class VhalSeqTestVpathFrontSkip : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestVpathFrontParameter> {};

INSTANTIATE_TEST_CASE_P(VhalSeqTestVpathFront, VhalSeqTestVpathFrontApiSpecification, ::testing::Values(
	VhalSeqTestVpathFrontParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV}
));

INSTANTIATE_TEST_CASE_P(VhalSeqTestVpathFront, VhalSeqTestVpathFrontSkip, ::testing::Values(
	VhalSeqTestVpathFrontParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV}
));

/**
 * API仕様書記載のシーケンス
 * 映像パス設定可否判定で結果trueだった場合 (ユーザーがサーフェス作成) */
/* DTVサーフェスが存在する場合 */
TEST_P(VhalSeqTestVpathFrontApiSpecification, UserCreateSurfaceAvaiableIsTrue)
{
	VhalSeqTestVpathFrontParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.visible", callback_visible));

	/* CARPLAYサーフェス作成 */
	status_available_expect = true;
	status_available = false;
	cb_available_param = VPATH_NAME_CARPLAY;
	ExecuteRenderImage(SURFACE_ID_CARPLAY);
	PrintMsg("render_image CARPLAY. wait_for available.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
	EXPECT_EQ(true, intime);

	/* CARPLAYへ切替 */
	status_visible_expect = false;
	status_visible = true;
	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	vpath = VPATH_NAME_CARPLAY;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.current=CARPLAY. wait_for current");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 可視性ON */
	status_visible_expect = true;
	status_visible = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.visible=true. wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 2秒表示 */
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	/* 7. 可視性OFF */
	status_visible_expect = false;
	status_visible = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.visible=false. wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* DTVサーフェス作成 */
	status_available_expect = true;
	status_available = false;
	cb_available_param = VPATH_NAME_DTV;
	ExecuteRenderImage(SURFACE_ID_FRONT_DTV);
	PrintMsg("render_image DTV. wait_for available.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
	EXPECT_EQ(true, intime);

	/* 21. DTV設定可能チェック */
	available = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.available?vpath=DTV", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(true, available);

	/* 27. DTVへ切替 */
	status_visible_expect = false;
	status_visible = true;
	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	vpath = VPATH_NAME_DTV;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.current=DTV. wait_for current.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 48. 可視性ON */
	status_visible_expect = true;
	status_visible = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.visible=true. wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 2秒表示 */
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.visible", callback_visible));

	KillAllRenderImage(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}
/**
 * API仕様書記載のシーケンス
 * 映像パス設定可否判定で結果trueだった場合 (ユーザーがサーフェス作成)
 * DTVサーフェスが存在しない場合
 */
TEST_P(VhalSeqTestVpathFrontApiSpecification, UserCreateSurfaceAvaiableIsTrueNoSurface)
{
	VhalSeqTestVpathFrontParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.visible", callback_visible));

	/* CARPLAYサーフェス作成 */
	status_available_expect = true;
	status_available = false;
	cb_available_param = VPATH_NAME_CARPLAY;
	ExecuteRenderImage(SURFACE_ID_CARPLAY);
	PrintMsg("render_image CARPLAY. wait_for available.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
	EXPECT_EQ(true, intime);

	/* CARPLAYへ切替 */
	status_visible_expect = false;
	status_visible = true;
	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	vpath = VPATH_NAME_CARPLAY;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.current=CARPLAY. wait_for current.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

		/* 可視性ON */
	status_visible_expect = true;
	status_visible = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.visible=false. wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 7. 可視性OFF */
	status_visible_expect = false;
	status_visible = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.visible=false. wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* DTVサーフェス作成 */
	status_available_expect = true;
	status_available = false;
	cb_available_param = VPATH_NAME_DTV;
	ExecuteRenderImage(SURFACE_ID_FRONT_DTV);
	PrintMsg("render_image DTV. wait_for available.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
	EXPECT_EQ(true, intime);

	/* 21. DTV設定可能チェック */
	available = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.available?vpath=DTV", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(true, available);

	/* ユーザ作成サーフェス削除 */
	status_available_expect = false;
	status_available = true;
	cb_available_param = VPATH_NAME_DTV;
	KillAllRenderImage(obj);
	PrintMsg("kill render_image. wait_for available.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
	EXPECT_EQ(true, intime);

	/* 27. DTVへ切替 */
	status_current_expect = VIDEO_HAL_VPATH_STS_FAILED;
	status_current = VIDEO_HAL_VPATH_STS_SUCCESS;
	vpath = VPATH_NAME_DTV;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.current=DTV. wait_for current.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.visible", callback_visible));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * API仕様書記載のシーケンス
 * 映像パス設定可否判定で結果falseだった場合 (ユーザーがサーフェス作成)
 */
TEST_P(VhalSeqTestVpathFrontApiSpecification, UserCreateSurfaceAvaiableIsFalse)
{
	VhalSeqTestVpathFrontParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.visible", callback_visible));

	/* 7. DTV設定可能チェック */
	available = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.available?vpath=DTV", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, available);

	/* 13. CARPLAYサーフェス作成 */
	status_available_expect = true;
	status_available = false;
	cb_available_param = VPATH_NAME_CARPLAY;
	ExecuteRenderImage(SURFACE_ID_CARPLAY);
	PrintMsg("render_image CARPLAY. wait_for available.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
	EXPECT_EQ(true, intime);

	/* 23. DTVサーフェス作成 */
	status_available_expect = true;
	status_available = false;
	cb_available_param = VPATH_NAME_DTV;
	ExecuteRenderImage(SURFACE_ID_FRONT_DTV);
	PrintMsg("render_image DTV. wait_for available.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
	EXPECT_EQ(true, intime);

	/* ここからは映像パス設定可否判定がtrueのときと同じ */

	/* 21. DTV設定可能チェック */
	available = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.available?vpath=DTV", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(true, available);

	/* 27. DTVへ切替 */
	status_visible_expect = false;
	status_visible = true;
	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	vpath = VPATH_NAME_DTV;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.current=DTV. wait_for current.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 48. 可視性ON */
	status_visible_expect = true;
	status_visible = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.visible=true. wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);	/* WARNING:visibleはパス切替時通知済みであり、これは失敗する */

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.visible", callback_visible));

	KillAllRenderImage(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * API仕様書記載のシーケンス
 * VideoHALがサーフェスを作成する場合
 * HDMIが接続されている場合 
 */
TEST_P(VhalSeqTestVpathFrontApiSpecification, VideoHALCreateSurfaceConnectedHdmi)
{
	VhalSeqTestVpathFrontParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.visible", callback_visible));

	/* 1. HDMI接続確認 */
	int32_t connected = VIDEO_HAL_CONNECTED_HDMI_NONE;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.connected.hdmi", &connected, sizeof(connected), VIDEO_HAL_DATA_TYPE_NUM));
//	EXPECT_EQ(VIDEO_HAL_CONNECTED_HDMI_READY, connected);

	/* DTVサーフェス作成 */
	status_available_expect = true;
	status_available = false;
	cb_available_param = VPATH_NAME_DTV;
	ExecuteRenderImage(SURFACE_ID_FRONT_DTV);
	PrintMsg("render_image DTV. wait_for available.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
	EXPECT_EQ(true, intime);

	/* DTVへ切替 */
	status_visible_expect = false;
	status_visible = true;
	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	vpath = VPATH_NAME_DTV;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.current=DTV. wait_for current.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 可視性ON */
	status_visible_expect = true;
	status_visible = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.visible=true. wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 7. 可視性OFF */
	status_visible_expect = false;
	status_visible = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.visible=true. wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 21. HDMI設定可能チェック */
	available = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.available?vpath=HDMI", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(true, available);

	/* 27. HDMIへ切替 */
	status_visible_expect = false;
	status_visible = true;
	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	vpath = VPATH_NAME_HDMI;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.current=HDMI. wait_for current.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 39. 可視性ON */
	status_visible_expect = true;
	status_visible = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.visible=true. wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.visible", callback_visible));

	KillAllRenderImage(obj);
		
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * API仕様書記載のシーケンス
 * VideoHALがサーフェスを作成する場合
 * HDMIが接続されていない場合
 */
TEST_P(VhalSeqTestVpathFrontApiSpecification, VideoHALCreateSurfaceNotConnectedH)
{
	VhalSeqTestVpathFrontParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.visible", callback_visible));

	/* 1. HDMI接続確認 */
	int32_t connected = VIDEO_HAL_CONNECTED_HDMI_READY;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.connected.hdmi", &connected, sizeof(connected), VIDEO_HAL_DATA_TYPE_NUM));
//	EXPECT_EQ(VIDEO_HAL_CONNECTED_HDMI_NONE, connected);

	/* DTVサーフェス作成 */
	status_available_expect = true;
	status_available = false;
	cb_available_param = VPATH_NAME_DTV;
	ExecuteRenderImage(SURFACE_ID_FRONT_DTV);
	PrintMsg("render_image DTV. wait_for available.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
	EXPECT_EQ(true, intime);

	/* DTVへ切替 */
	status_visible_expect = false;
	status_visible = true;
	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	vpath = VPATH_NAME_DTV;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.current=DTV. wait_for current.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 可視性ON */
	status_visible_expect = true;
	status_visible = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.visible=true. wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 7. 可視性OFF */
	status_visible_expect = false;
	status_visible = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.visible=true. wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 21. HDMI設定可能チェック */
	available = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.available?vpath=HDMI", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(true, available);

	/* 27. HDMIへ切替(接続無しのため失敗) */
	status_current_expect = VIDEO_HAL_VPATH_STS_FAILED;
	status_current = VIDEO_HAL_VPATH_STS_SUCCESS;
	vpath = VPATH_NAME_HDMI;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.current=HDMI. wait_for current.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.visible", callback_visible));

	KillAllRenderImage(obj);
		
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * 連続動作
 * 映像パス設定可否判定で結果trueだった場合 (ユーザーがサーフェス作成)
 * DTVサーフェスが存在する場合
 */
TEST(VhalSeqTestVpathFrontRepeat, UserCreateSurfaceAvaiableIsTrue)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.visible", callback_visible));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		/* CARPLAYサーフェス作成 */
		status_available_expect = true;
		status_available = false;
		cb_available_param = VPATH_NAME_CARPLAY;
		ExecuteRenderImage(SURFACE_ID_CARPLAY);
		PrintMsg("render_image CARPLAY. wait_for available.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
		EXPECT_EQ(true, intime);

		/* CARPLAYへ切替 */
		status_visible_expect = false;
		status_visible = true;
		status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
		status_current = VIDEO_HAL_VPATH_STS_FAILED;
		vpath = VPATH_NAME_CARPLAY;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.front.control.current=CARPLAY. wait_for current");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* 可視性ON */
		status_visible_expect = true;
		status_visible = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.front.control.visible=false. wait_for visible.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* 7. 可視性OFF */
		status_visible_expect = false;
		status_visible = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.front.control.visible=false. wait_for visible.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* DTVサーフェス作成 */
		status_available_expect = true;
		status_available = false;
		cb_available_param = VPATH_NAME_DTV;
		ExecuteRenderImage(SURFACE_ID_FRONT_DTV);
		PrintMsg("render_image DTV. wait_for available.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
		EXPECT_EQ(true, intime);

		/* 21. DTV設定可能チェック */
		available = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.available?vpath=DTV", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(true, available);

		/* 27. DTVへ切替 */
		status_visible_expect = false;
		status_visible = true;
		status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
		status_current = VIDEO_HAL_VPATH_STS_FAILED;
		vpath = VPATH_NAME_DTV;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.front.control.current=DTV. wait_for current.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* 48. 可視性ON */
		status_visible_expect = true;
		status_visible = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.front.control.visible=true. wait_for visible.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* 後始末 */
		status_available_expect = false;
		status_available = true;
		cb_available_param = VPATH_NAME_CARPLAY;
		PrintMsg("kill render_image. wait_for available.");
		KillRenderImage(SURFACE_ID_CARPLAY);
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
		EXPECT_EQ(true, intime);

		status_available_expect = false;
		status_available = true;
		cb_available_param = VPATH_NAME_DTV;
		PrintMsg("kill render_image. wait_for available.");
		KillRenderImage(SURFACE_ID_FRONT_DTV);
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
		EXPECT_EQ(true, intime);

		status_visible_expect = false;
		status_visible = true;
		status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
		status_current = VIDEO_HAL_VPATH_STS_FAILED;
		vpath = VPATH_NAME_CLEAR;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for current=CLEAR.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.visible", callback_visible));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * 連続動作
 * 映像パス設定可否判定で結果trueだった場合 (ユーザーがサーフェス作成)
 * DTVサーフェスが存在しない場合
 */
TEST(VhalSeqTestVpathFrontRepeat, UserCreateSurfaceAvaiableIsTrueNoSurface)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.visible", callback_visible));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		/* CARPLAYサーフェス作成 */
		status_available_expect = true;
		status_available = false;
		cb_available_param = VPATH_NAME_CARPLAY;
		ExecuteRenderImage(SURFACE_ID_CARPLAY);
		PrintMsg("render_image CARPLAY. wait_for available.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
		EXPECT_EQ(true, intime);

		/* CARPLAYへ切替 */
		status_visible_expect = false;
		status_visible = true;
		status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
		status_current = VIDEO_HAL_VPATH_STS_FAILED;
		vpath = VPATH_NAME_CARPLAY;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.front.control.current=CARPLAY. wait_for current.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* 可視性ON */
		status_visible_expect = true;
		status_visible = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.front.control.visible=false. wait_for visible.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* 7. 可視性OFF */
		status_visible_expect = false;
		status_visible = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.front.control.visible=false. wait_for visible.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* DTVサーフェス作成 */
		status_available_expect = true;
		status_available = false;
		cb_available_param = VPATH_NAME_DTV;
		ExecuteRenderImage(SURFACE_ID_FRONT_DTV);
		PrintMsg("render_image DTV. wait_for available.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
		EXPECT_EQ(true, intime);

		/* 21. DTV設定可能チェック */
		available = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.available?vpath=DTV", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(true, available);

		/* ユーザ作成サーフェス削除 */
		status_available_expect = false;
		status_available = true;
		cb_available_param = VPATH_NAME_CARPLAY;
		KillRenderImage(SURFACE_ID_CARPLAY);
		PrintMsg("kill render_image. wait_for available.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
		EXPECT_EQ(true, intime);

		status_available_expect = false;
		status_available = true;
		cb_available_param = VPATH_NAME_DTV;
		KillRenderImage(SURFACE_ID_FRONT_DTV);
		PrintMsg("kill render_image. wait_for available.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
		EXPECT_EQ(true, intime);
		
		/* 27. DTVへ切替 */
		status_current_expect = VIDEO_HAL_VPATH_STS_FAILED;
		status_current = VIDEO_HAL_VPATH_STS_SUCCESS;
		vpath = VPATH_NAME_DTV;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.front.control.current=DTV. wait_for current.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
		EXPECT_EQ(true, intime);

		/* 映像パスクリア */
		status_visible_expect = false;
		status_visible = true;
		status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
		status_current = VIDEO_HAL_VPATH_STS_FAILED;
		vpath = VPATH_NAME_CLEAR;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.front.control.current=CARPLAY. wait_for current.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.visible", callback_visible));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * 連続動作
 * 映像パス設定可否判定で結果falseだった場合 (ユーザーがサーフェス作成)
*/
TEST(VhalSeqTestVpathFrontRepeat, UserCreateSurfaceAvaiableIsFalse)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.visible", callback_visible));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		/* 7. DTV設定可能チェック */
		available = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.available?vpath=DTV", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(false, available);

		/* 13. CARPLAYサーフェス作成 */
		status_available_expect = true;
		status_available = false;
		cb_available_param = VPATH_NAME_CARPLAY;
		ExecuteRenderImage(SURFACE_ID_CARPLAY);
		PrintMsg("render_image CARPLAY. wait_for available.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
		EXPECT_EQ(true, intime);

		/* 23. DTVサーフェス作成 */
		status_available_expect = true;
		status_available = false;
		cb_available_param = VPATH_NAME_DTV;
		ExecuteRenderImage(SURFACE_ID_FRONT_DTV);
		PrintMsg("render_image DTV. wait_for available.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
		EXPECT_EQ(true, intime);

		/* ここからは映像パス設定可否判定がtrueのときと同じ */

		/* 21. DTV設定可能チェック */
		available = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.available?vpath=DTV", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(true, available);

		/* 27. DTVへ切替 */
		status_visible_expect = false;
		status_visible = true;
		status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
		status_current = VIDEO_HAL_VPATH_STS_FAILED;
		vpath = VPATH_NAME_DTV;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.front.control.current=DTV. wait_for current.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* 48. 可視性ON */
		status_visible_expect = true;
		status_visible = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.front.control.visible=true. wait_for visible.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* 後始末 */
		status_available_expect = false;
		status_available = true;
		cb_available_param = VPATH_NAME_CARPLAY;
		PrintMsg("kill render_image. wait_for available.");
		KillRenderImage(SURFACE_ID_CARPLAY);
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
		EXPECT_EQ(true, intime);

		status_available_expect = false;
		status_available = true;
		cb_available_param = VPATH_NAME_DTV;
		PrintMsg("kill render_image. wait_for available.");
		KillRenderImage(SURFACE_ID_FRONT_DTV);
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
		EXPECT_EQ(true, intime);

		status_visible_expect = false;
		status_visible = true;
		status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
		status_current = VIDEO_HAL_VPATH_STS_FAILED;
		vpath = VPATH_NAME_CLEAR;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for current=CLEAR.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.visible", callback_visible));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * 連続動作
 * VideoHALがサーフェスを作成する場合
 * HDMIが接続されている場合
 */
TEST(VhalSeqTestVpathFrontRepeat, VideoHALCreateSurfaceConnectedHdmi)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	DetectConnectedHdmi(DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECT);
	ChangeHdmiVideoFormat(get_default_videoformat());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.visible", callback_visible));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		/* 1. HDMI接続確認 */
		int32_t connected = VIDEO_HAL_CONNECTED_HDMI_NONE;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.connected.hdmi", &connected, sizeof(connected), VIDEO_HAL_DATA_TYPE_NUM));
//		EXPECT_EQ(VIDEO_HAL_CONNECTED_HDMI_READY, connected);

		/* DTVサーフェス作成 */
		status_available_expect = true;
		status_available = false;
		cb_available_param = VPATH_NAME_DTV;
		ExecuteRenderImage(SURFACE_ID_FRONT_DTV);
		PrintMsg("render_image DTV. wait_for available.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
		EXPECT_EQ(true, intime);

		/* DTVへ切替 */
		status_visible_expect = false;
		status_visible = true;
		status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
		status_current = VIDEO_HAL_VPATH_STS_FAILED;
		vpath = VPATH_NAME_DTV;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.front.control.current=DTV. wait_for current.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* 可視性ON */
		status_visible_expect = true;
		status_visible = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.front.control.visible=true. wait_for visible.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* 7. 可視性OFF */
		status_visible_expect = false;
		status_visible = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.front.control.visible=true. wait_for visible.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* 21. HDMI設定可能チェック */
		available = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.available?vpath=HDMI", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(true, available);

		/* 27. HDMIへ切替 */
		status_visible_expect = false;
		status_visible = true;
		status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
		status_current = VIDEO_HAL_VPATH_STS_FAILED;
		vpath = VPATH_NAME_HDMI;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.front.control.current=HDMI. wait_for current.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* 39. 可視性ON */
		status_visible_expect = true;
		status_visible = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.front.control.visible=true. wait_for visible.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* 後始末 */
		status_available_expect = false;
		status_available = true;
		cb_available_param = VPATH_NAME_DTV;
		PrintMsg("kill render_image. wait_for available.");
		KillRenderImage(SURFACE_ID_FRONT_DTV);
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.visible", callback_visible));

	KillAllRenderImage(obj);
		
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * 連続動作
 * VideoHALがサーフェスを作成する場合
 * HDMIが接続されていない場合
 */
TEST(VhalSeqTestVpathFrontRepeat, VideoHALCreateSurfaceNotConnectedH)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool visible{false};
	bool available{false};
	std::string vpath;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	DetectConnectedHdmi(DAT_CNV_SYSINFO_CONN_INFO_STAT_DISCONNECT);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.visible", callback_visible));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		/* 1. HDMI接続確認 */
		int32_t connected = VIDEO_HAL_CONNECTED_HDMI_READY;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.connected.hdmi", &connected, sizeof(connected), VIDEO_HAL_DATA_TYPE_NUM));
//		EXPECT_EQ(VIDEO_HAL_CONNECTED_HDMI_NONE, connected);

		/* DTVサーフェス作成 */
		status_available_expect = true;
		status_available = false;
		cb_available_param = VPATH_NAME_DTV;
		ExecuteRenderImage(SURFACE_ID_FRONT_DTV);
		PrintMsg("render_image DTV. wait_for available.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
		EXPECT_EQ(true, intime);

		/* DTVへ切替 */
		status_visible_expect = false;
		status_visible = true;
		status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
		status_current = VIDEO_HAL_VPATH_STS_FAILED;
		vpath = VPATH_NAME_DTV;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.front.control.current=DTV. wait_for current.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* 可視性ON */
		status_visible_expect = true;
		status_visible = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.front.control.visible=true. wait_for visible.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* 7. 可視性OFF */
		status_visible_expect = false;
		status_visible = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.front.control.visible=true. wait_for visible.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* 21. HDMI設定可能チェック */
		available = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.available?vpath=HDMI", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(true, available);

		/* 27. HDMIへ切替(失敗) */
		status_current_expect = VIDEO_HAL_VPATH_STS_FAILED;
		status_current = VIDEO_HAL_VPATH_STS_SUCCESS;
		vpath = VPATH_NAME_HDMI;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.front.control.current=HDMI. wait_for current.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
		EXPECT_EQ(true, intime);

		/* 後始末 */
		status_available_expect = false;
		status_available = true;
		cb_available_param = VPATH_NAME_DTV;
		PrintMsg("kill render_image. wait_for available.");
		KillRenderImage(SURFACE_ID_FRONT_DTV);
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
		EXPECT_EQ(true, intime);

		/* 映像パスクリア */
		status_visible_expect = false;
		status_visible = true;
		status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
		status_current = VIDEO_HAL_VPATH_STS_FAILED;
		vpath = VPATH_NAME_CLEAR;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.front.control.current=DTV. wait_for current.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.visible", callback_visible));
		
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * シーケンス抜け
 * 前席映像パス設定可否判定を取得せず映像パスを切替える
 */
TEST_P(VhalSeqTestVpathFrontSkip, Skip)
{
	VhalSeqTestVpathFrontParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool visible{false};
	bool available{false};
	std::string vpath;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.visible", callback_visible));

	/* CARPLAYサーフェス作成 */
	status_available_expect = true;
	status_available = false;
	cb_available_param = VPATH_NAME_CARPLAY;
	ExecuteRenderImage(SURFACE_ID_CARPLAY);
	PrintMsg("render_image CARPLAY. wait_for available.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
	EXPECT_EQ(true, intime);

	/* CARPLAYへ切替 */
	status_visible_expect = false;
	status_visible = true;
	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	vpath = VPATH_NAME_CARPLAY;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.current=CARPLAY. wait_for current");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 可視性ON */
	status_visible_expect = true;
	status_visible = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.visible=false. wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 7. 可視性OFF */
	status_visible_expect = false;
	status_visible = true;
	visible = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &visible, sizeof(visible), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.visible=false. wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* DTVサーフェス作成 */
	status_available_expect = true;
	status_available = false;
	cb_available_param = VPATH_NAME_DTV;
	ExecuteRenderImage(SURFACE_ID_FRONT_DTV);
	PrintMsg("render_image DTV. wait_for available.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
	EXPECT_EQ(true, intime);

	/* 27. DTVへ切替 */
	status_visible_expect = false;
	status_visible = true;
	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	vpath = VPATH_NAME_DTV;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.current=DTV. wait_for current.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 48. 可視性ON */
	status_visible_expect = true;
	status_visible = false;
	visible = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &visible, sizeof(visible), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.visible=true. wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.visible", callback_visible));

	KillAllRenderImage(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * 設計書記載のシーケンス
 * HDMI(ビデオフォーマット)
 * 不正なフォーマット
 */
TEST(VhalSeqTestVpathFrontSeqSpecification, HdmiVideoFormatInvalid)
{
	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_HDMI));

	/* 現在のフォーマット取得 */
	int32_t format{};
	ReadHdmiVideoFormat(format);
	PrintDbg("*** Now VideoFormat=%d", format);

	/* HDMIビデオフォーマット変更 */
	ChangeHdmiVideoFormat(SYSINFO_CONN_INFO_VIDEO_FORMAT_INVALID);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	ChangeHdmiVideoFormat(format);
	PrintDbg("*** Now VideoFormat=%d", format);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設計書記載のシーケンス
 * HDMI(ビデオフォーマット)
 */
TEST(VhalSeqTestVpathFrontSeqSpecification, HdmiVideoFormatValid)
{
	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_HDMI));

	/* 現在のフォーマット取得 */
	int32_t format{};
	ReadHdmiVideoFormat(format);

	/* HDMIビデオフォーマット変更 */
	ChangeHdmiVideoFormat(SYSINFO_CONN_INFO_VIDEO_FORMAT_INVALID);
	ChangeHdmiVideoFormat(format);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * HDMI接続検知
*/
TEST(VhalSeqTestVpathFrontSeqSpecification, ConnectedHdmi)
{
	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_HDMI));

	/*HDMI接続状態変更 */
	DetectConnectedHdmi(DAT_CNV_SYSINFO_CONN_INFO_STAT_DISCONNECT);
	DetectConnectedHdmi(DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECT);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 * 設計書記載のシーケンス
 * HDMI(ビデオフォーマット)
 * 不正なフォーマット
 */
TEST(VhalSeqTestVpathFrontRepeat, HdmiVideoFormatInvalid)
{
	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_HDMI));

	/* 現在のフォーマット取得 */
	int32_t format{};
	ReadHdmiVideoFormat(format);

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		/* HDMIビデオフォーマット変更 */
		ChangeHdmiVideoFormat(SYSINFO_CONN_INFO_VIDEO_FORMAT_INVALID);
	}

	ChangeHdmiVideoFormat(format);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 * 設計書記載のシーケンス
 * HDMI(ビデオフォーマット)
 */
TEST(VhalSeqTestVpathFrontRepeat, HdmiVideoFormatValid)
{
	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_HDMI));

	/* 現在のフォーマット取得 */
	int32_t format{};
	ReadHdmiVideoFormat(format);

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		/* HDMIビデオフォーマット変更 */
		ChangeHdmiVideoFormat(SYSINFO_CONN_INFO_VIDEO_FORMAT_INVALID);
		ChangeHdmiVideoFormat(SYSINFO_CONN_INFO_VIDEO_FORMAT_1280X720P);
	}

	ChangeHdmiVideoFormat(format);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 * 設計書記載のシーケンス
 * HDMI接続検知
*/
TEST(VhalSeqTestVpathFrontRepeat, ConnectedHdmi)
{
	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_HDMI));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		/*HDMI接続状態変更 */
		DetectConnectedHdmi(DAT_CNV_SYSINFO_CONN_INFO_STAT_DISCONNECT);
		DetectConnectedHdmi(DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECT);
	}

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * API仕様書記載のシーケンス
 * HDMIが接続され、一定時間後停止する場合 
 * 実機の場合HDMI接続状態であること、またC-Disp認証成功状態であること
 */
TEST(VhalSeqTestVpathFrontApiSpecification, VideoHALHdmiDisplayAndClean)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;

	cockpit::hal::video_hal::Init(obj);

	DetectConnectedHdmi(DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECT);
	ChangeHdmiVideoFormat(get_default_videoformat());

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, false));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.visible", callback_visible));

	/* HDMI接続確認 */
	int32_t connected = VIDEO_HAL_CONNECTED_HDMI_NONE;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.connected.hdmi", &connected, sizeof(connected), VIDEO_HAL_DATA_TYPE_NUM));
//	EXPECT_EQ(VIDEO_HAL_CONNECTED_HDMI_READY, connected);	// BEVstep3 QNXからのHDMI接続通知がなければVIDEO_HAL_CONNECTED_HDMI_NONEのまま

	/* HDMI設定可能チェック */
	available = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.available?vpath=HDMI", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(true, available);

	/* HDMIへ切替 */
	status_visible_expect = false;
	status_visible = true;
	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	vpath = VPATH_NAME_HDMI;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.current=HDMI. wait_for current.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
	EXPECT_EQ(true, intime);

	/* 可視性ON */
	status_visible_expect = true;
	status_visible = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.visible=true. wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 4秒表示 */
	std::this_thread::sleep_for(std::chrono::milliseconds(4000));

	/* HDMIを開放 */
	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	status_visible_expect = false;
	status_visible = true;
	vpath = VPATH_NAME_CLEAR;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for current=CLEAR.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
	EXPECT_EQ(true, intime);

	/* 可視性OFF(HDMI開放時に連動) */
	PrintMsg("SetValue vpath.front.control.visible=false. wait_for unvisible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.visible", callback_visible));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.current", callback_current));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * 連続動作
 * API仕様書記載のシーケンス
 * HDMIが接続され、一定時間後停止する場合 
 * 実機の場合HDMI接続状態であること、またC-Disp認証成功状態であること
 */
TEST(VhalSeqTestVpathFrontRepeat, VideoHALHdmiDisplayAndClean)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;

	cockpit::hal::video_hal::Init(obj);

	DetectConnectedHdmi(DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECT);
	ChangeHdmiVideoFormat(get_default_videoformat());

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, false));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.visible", callback_visible));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		/* HDMI接続確認 */
		int32_t connected = VIDEO_HAL_CONNECTED_HDMI_NONE;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.connected.hdmi", &connected, sizeof(connected), VIDEO_HAL_DATA_TYPE_NUM));
//		EXPECT_EQ(VIDEO_HAL_CONNECTED_HDMI_READY, connected);	// BEVstep3 QNXからのHDMI接続通知がなければVIDEO_HAL_CONNECTED_HDMI_NONEのまま

		/* HDMI設定可能チェック */
		available = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.available?vpath=HDMI", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(true, available);

		/* HDMIへ切替 */
		status_visible_expect = false;
		status_visible = true;
		status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
		status_current = VIDEO_HAL_VPATH_STS_FAILED;
		vpath = VPATH_NAME_HDMI;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.front.control.current=HDMI. wait_for current.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
		EXPECT_EQ(true, intime);

		/* 可視性ON */
		status_visible_expect = true;
		status_visible = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.front.control.visible=true. wait_for visible.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* 1秒表示 */
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		/* HDMIを開放 */
		status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
		status_current = VIDEO_HAL_VPATH_STS_FAILED;
		status_visible_expect = false;
		status_visible = true;
		vpath = VPATH_NAME_CLEAR;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for current=CLEAR.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
		EXPECT_EQ(true, intime);

		/* 可視性OFF(HDMI開放時に連動) */
		PrintMsg("SetValue vpath.front.control.visible=false. wait_for unvisible.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.visible", callback_visible));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.current", callback_current));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * API仕様書記載のシーケンス
 * HDMIが接続され、停止中の時に停止を実施
 */
TEST(VhalSeqTestVpathFrontApiSpecification, VideoHALHdmiStoptoStop)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.current", callback_current));

	/* HDMI接続確認 */
	int32_t connected = VIDEO_HAL_CONNECTED_HDMI_NONE;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.connected.hdmi", &connected, sizeof(connected), VIDEO_HAL_DATA_TYPE_NUM));
//	EXPECT_EQ(VIDEO_HAL_CONNECTED_HDMI_READY, connected);	// BEVstep3 QNXからのHDMI接続通知がなければVIDEO_HAL_CONNECTED_HDMI_NONEのまま

	/* HDMI設定可能チェック */
	available = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.available?vpath=HDMI", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(true, available);

	/* HDMIを開放 */
	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	vpath = VPATH_NAME_CLEAR;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for current=CLEAR.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(300), [] { return (status_current_expect == status_current); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.current", callback_current));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

// -------  異常系 -------------------
/**
 * API仕様書記載のシーケンス
 * 映像パス設定可否判定で結果trueだった場合 (ユーザーがサーフェス作成)
 * DTVサーフェスを表示しようとしてVPath誤りの場合
 */
TEST(VhalSeqTestVpathFrontApiSpecification, UserCreateSurfaceInvalidPath)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.visible", callback_visible));

	/* CARPLAYサーフェス作成 */
	status_available_expect = true;
	status_available = false;
	cb_available_param = VPATH_NAME_CARPLAY;
	ExecuteRenderImage(SURFACE_ID_CARPLAY);
	PrintMsg("render_image CARPLAY. wait_for available.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
	EXPECT_EQ(true, intime);

	/* CARPLAYへ切替 */
	status_visible_expect = false;
	status_visible = true;
	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	vpath = VPATH_NAME_CARPLAY;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.current=CARPLAY. wait_for current.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

		/* 可視性ON */
	status_visible_expect = true;
	status_visible = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.visible=false. wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 7. 可視性OFF */
//	status_visible_expect = false;
//	status_visible = true;
//	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
//	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
//	PrintMsg("SetValue vpath.front.control.visible=false. wait_for visible.");
//	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
//	EXPECT_EQ(true, intime);

	/* DTVサーフェス作成 */
	status_available_expect = true;
	status_available = false;
	cb_available_param = VPATH_NAME_DTV;
	ExecuteRenderImage(SURFACE_ID_FRONT_DTV);
	PrintMsg("render_image DTV. wait_for available.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
	EXPECT_EQ(true, intime);
	std::this_thread::sleep_for(std::chrono::milliseconds(400));

	/* 21. DTV設定可能チェック */
	available = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.available?vpath=DTV", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(true, available);

	/* 27. DTVへ切替 */
	status_current_expect = VIDEO_HAL_VPATH_STS_FAILED;
	status_current = VIDEO_HAL_VPATH_STS_SUCCESS;
	vpath = "DTW";	// fail
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.front.control.current=DTV. wait_for current.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
	EXPECT_EQ(true, intime);

	/* 2秒表示 */
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	/* ユーザ作成サーフェス削除 */
	status_available_expect = false;
	status_available = true;
	cb_available_param = VPATH_NAME_DTV;
	KillAllRenderImage(obj);
	PrintMsg("kill render_image. wait_for available.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.visible", callback_visible));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}


