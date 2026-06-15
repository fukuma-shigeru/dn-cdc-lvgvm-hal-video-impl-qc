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

static void callback_current(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.rear.status.current", &status_current, sizeof(status_current), VIDEO_HAL_DATA_TYPE_NUM));
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
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.rear.status.available?vpath=" + cb_available_param, &status_available, sizeof(status_available), VIDEO_HAL_DATA_TYPE_BOOL));
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
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.rear.status.visible", &status_visible, sizeof(status_visible), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(status_visible_expect, status_visible);
	}

	cond_sync.notify_one();
	PrintMsg("end.");
}

struct VhalSeqTestVpathRearParameter {
	int32_t surface_id;
	std::string rear_vpath_name;
};

class VhalSeqTestVpathRearApiSpecification : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestVpathRearParameter> {};
class VhalSeqTestVpathRearSkipHdmi : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestVpathRearParameter> {};

INSTANTIATE_TEST_CASE_P(VhalSeqTestVpathRear, VhalSeqTestVpathRearApiSpecification, ::testing::Values(
	VhalSeqTestVpathRearParameter{SURFACE_ID_REAR_DTV, VPATH_NAME_DTV}
));

INSTANTIATE_TEST_CASE_P(VhalSeqTestVpathRear, VhalSeqTestVpathRearSkipHdmi, ::testing::Values(
	VhalSeqTestVpathRearParameter{SURFACE_ID_REAR_DTV, VPATH_NAME_DTV}
));

/**
 * API仕様書記載のシーケンス
 * 映像パス設定可否判定で結果trueだった場合 (ユーザーがサーフェス作成)
 * DTVサーフェスが存在する場合
 */
TEST_P(VhalSeqTestVpathRearApiSpecification, UserCreateSurfaceAvaiableIsTrueHdmi)
{
	VhalSeqTestVpathRearParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_HDMI));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleRear(obj, true));

//	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.visible", callback_visible));

	/* 1. 可視性OFF */
	status_visible_expect = false;
	status_visible = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.rear.control.visible=false. wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 15. DTV設定可能チェック */
	available = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.rear.status.available?vpath=DTV", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(true, available);

	/* 21. DTVへ切替 */
	status_visible_expect = false;
	status_visible = true;
	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	vpath = VPATH_NAME_DTV;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.rear.control.current=DTV. wait_for current.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 42. 可視性ON */
	status_visible_expect = true;
	status_visible = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.rear.control.visible=true. wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

//	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.visible", callback_visible));

	KillAllRenderImage(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * API仕様書記載のシーケンス
 * 映像パス設定可否判定で結果trueだった場合 (ユーザーがサーフェス作成) 
 * DTVサーフェスが存在しない場合
 */
TEST_P(VhalSeqTestVpathRearApiSpecification, UserCreateSurfaceAvaiableIsTrueNoSurfaceHdmi)
{
	VhalSeqTestVpathRearParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_HDMI));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleRear(obj, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.visible", callback_visible));

	/* 1. 可視性OFF */
	status_visible_expect = false;
	status_visible = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.rear.control.visible=false. wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 15. DTV設定可能チェック */
	available = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.rear.status.available?vpath=DTV", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(true, available);

	/* ユーザ作成サーフェス削除 */
	status_available_expect = false;
	status_available = true;
	cb_available_param = VPATH_NAME_DTV;
	KillAllRenderImage(obj);
	PrintMsg("kill render_image. wait_for available.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
	EXPECT_EQ(true, intime);

	/* 21. DTVへ切替 */
	status_current_expect = VIDEO_HAL_VPATH_STS_FAILED;
	status_current = VIDEO_HAL_VPATH_STS_SUCCESS;
	vpath = VPATH_NAME_DTV;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.rear.control.current=DTV. wait_for current.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.visible", callback_visible));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * 連続動作
 * 映像パス設定可否判定で結果trueだった場合 (ユーザーがサーフェス作成)
 * DTVサーフェスが存在する場合
 */
TEST(VhalSeqTestVpathRearRepeat, UserCreateSurfaceAvaiableIsTrueHdmi)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_HDMI));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleRear(obj, true));

//	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.visible", callback_visible));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		/* 1. 可視性OFF */
		status_visible_expect = false;
		status_visible = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.rear.control.visible=false. wait_for visible.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* 15. DTV設定可能チェック */
		available = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.rear.status.available?vpath=DTV", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(true, available);

		/* 21. DTVへ切替 */
		status_visible_expect = false;
		status_visible = true;
		status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
		status_current = VIDEO_HAL_VPATH_STS_FAILED;
		vpath = VPATH_NAME_DTV;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.rear.control.current=DTV. wait_for current.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* 42. 可視性ON */
		status_visible_expect = true;
		status_visible = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.rear.control.visible=true. wait_for visible.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* HDMIへ切替 */
		status_visible_expect = false;
		status_visible = true;
		status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
		status_current = VIDEO_HAL_VPATH_STS_FAILED;
		vpath = VPATH_NAME_HDMI;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.rear.control.current=HDMI. wait_for current.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* 可視性ON */
		status_visible_expect = true;
		status_visible = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.rear.control.visible=true. wait_for visible.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);
	}

//	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.visible", callback_visible));

	KillAllRenderImage(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * 連続動作
 * 映像パス設定可否判定で結果trueだった場合 (ユーザーがサーフェス作成)
 * DTVサーフェスが存在しない場合
 */
TEST(VhalSeqTestVpathRearRepeat, UserCreateSurfaceAvaiableIsTrueNoSurfaceHdmi)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_HDMI));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleRear(obj, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.visible", callback_visible));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		/* DTVサーフェス作成 */
		status_available_expect = true;
		status_available = false;
		cb_available_param = VPATH_NAME_DTV;
		ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV);
		PrintMsg("render_image DTV. wait_for available.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
		EXPECT_EQ(true, intime);

		/* 1. 可視性OFF */
		status_visible_expect = false;
		status_visible = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.rear.control.visible=false. wait_for visible.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* 15. DTV設定可能チェック */
		available = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.rear.status.available?vpath=DTV", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(true, available);

		/* ユーザ作成サーフェス削除 */
		status_available_expect = false;
		status_available = true;
		cb_available_param = VPATH_NAME_DTV;
		KillAllRenderImage(obj);
		PrintMsg("kill render_image. wait_for available.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
		EXPECT_EQ(true, intime);
		
		/* 21. DTVへ切替 */
		status_current_expect = VIDEO_HAL_VPATH_STS_FAILED;
		status_current = VIDEO_HAL_VPATH_STS_SUCCESS;
		vpath = VPATH_NAME_DTV;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.rear.control.current=DTV. wait_for current.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
		EXPECT_EQ(true, intime);

		/* 可視性ON */
		status_visible_expect = true;
		status_visible = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.rear.control.visible=false. wait_for visible.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.visible", callback_visible));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * シーケンス抜け
 * 後席映像パス設定可否判定を取得せず映像パスを切替える
 */
TEST_P(VhalSeqTestVpathRearSkipHdmi, Skip)
{
	VhalSeqTestVpathRearParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_HDMI));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleRear(obj, true));

//	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.visible", callback_visible));

	/* 1. 可視性OFF */
	status_visible_expect = false;
	status_visible = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.rear.control.visible=false. wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 21. DTVへ切替 */
	status_visible_expect = false;
	status_visible = true;
	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	vpath = VPATH_NAME_DTV;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.rear.control.current=DTV. wait_for current.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 42. 可視性ON */
	status_visible_expect = true;
	status_visible = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.rear.control.visible=true. wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

//	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.visible", callback_visible));

	KillAllRenderImage(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}