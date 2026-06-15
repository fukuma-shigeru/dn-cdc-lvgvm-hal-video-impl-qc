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
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.instrumentcluster.status.current", &status_current, sizeof(status_current), VIDEO_HAL_DATA_TYPE_NUM));
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
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.instrumentcluster.status.available?vpath=" + cb_available_param, &status_available, sizeof(status_available), VIDEO_HAL_DATA_TYPE_BOOL));
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
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.instrumentcluster.status.visible", &status_visible, sizeof(status_visible), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(status_visible_expect, status_visible);
	}

	cond_sync.notify_one();
	PrintMsg("end.");
}

struct VhalSeqTestVpathIcParameter {
	int32_t surface_id;
	std::string instrumentcluster_vpath_name;
};

class VhalSeqTestVpathIcApiSpecification : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestVpathIcParameter> {};
class VhalSeqTestVpathIcSkip : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestVpathIcParameter> {};

INSTANTIATE_TEST_CASE_P(VhalSeqTestVpathIc, VhalSeqTestVpathIcApiSpecification, ::testing::Values(
	VhalSeqTestVpathIcParameter{SURFACE_ID_IC_MAP, VPATH_NAME_MAP}
));

INSTANTIATE_TEST_CASE_P(VhalSeqTestVpathIc, VhalSeqTestVpathIcSkip, ::testing::Values(
	VhalSeqTestVpathIcParameter{SURFACE_ID_IC_MAP, VPATH_NAME_MAP}
));

/**
 * API仕様書記載のシーケンス
 * 映像パス設定可否判定で結果trueだった場合 (ユーザーがサーフェス作成)
 * CARPLAYサーフェスが存在する場合
 */
TEST_P(VhalSeqTestVpathIcApiSpecification, UserCreateSurfaceAvaiableIsTrue)
{
	VhalSeqTestVpathIcParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_IC_CARPLAY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_IC_MAP));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, VPATH_NAME_MAP));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleIc(obj, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.visible", callback_visible));

	/* 1. 可視性OFF */
	status_visible_expect = false;
	status_visible = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.instrumentcluster.control.visible=false. wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 15. CARPLAY設定可能チェック */
	available = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.status.available?vpath=CARPLAY", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(true, available);

	/* 21. CARPLAYへ切替 */
	status_visible_expect = false;
	status_visible = true;
	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	vpath = VPATH_NAME_CARPLAY;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.instrumentcluster.control.current=CARPLAY. wait_for current.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 42. 可視性ON */
	status_visible_expect = true;
	status_visible = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.instrumentcluster.control.visible=true. wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.visible", callback_visible));

	KillAllRenderImage(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * API仕様書記載のシーケンス
 * 映像パス設定可否判定で結果trueだった場合 (ユーザーがサーフェス作成) 
 * CARPLAYサーフェスが存在しない場合
 */
TEST_P(VhalSeqTestVpathIcApiSpecification, UserCreateSurfaceAvaiableIsTrueNoSurface)
{
	VhalSeqTestVpathIcParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_IC_CARPLAY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_IC_MAP));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, VPATH_NAME_MAP));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleIc(obj, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.visible", callback_visible));

	/* 1. 可視性OFF */
	status_visible_expect = false;
	status_visible = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.instrumentcluster.control.visible=false. wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 15. CARPLAY設定可能チェック */
	available = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.status.available?vpath=CARPLAY", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(true, available);

	/* ユーザ作成サーフェス削除 */
	status_available_expect = false;
	status_available = true;
	cb_available_param = VPATH_NAME_CARPLAY;
	KillRenderImage(SURFACE_ID_IC_CARPLAY);
	PrintMsg("kill render_image. wait_for available.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
	EXPECT_EQ(true, intime);

	/* 21. CARPLAYへ切替 */
	status_current_expect = VIDEO_HAL_VPATH_STS_FAILED;
	status_current = VIDEO_HAL_VPATH_STS_SUCCESS;
	vpath = VPATH_NAME_CARPLAY;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.instrumentcluster.control.current=CARPLAY. wait_for current.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.visible", callback_visible));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * 連続動作
 * 映像パス設定可否判定で結果trueだった場合 (ユーザーがサーフェス作成)
 * CARPLAYサーフェスが存在する場合
 */
TEST(VhalSeqTestVpathIcRepeat, UserCreateSurfaceAvaiableIsTrue)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_IC_CARPLAY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_IC_MAP));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, VPATH_NAME_MAP));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleIc(obj, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.visible", callback_visible));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		/* 1. 可視性OFF */
		status_visible_expect = false;
		status_visible = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.instrumentcluster.control.visible=false. wait_for visible.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* 15. CARPLAY設定可能チェック */
		available = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.status.available?vpath=CARPLAY", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(true, available);

		/* 21. CARPLAYへ切替 */
		status_visible_expect = false;
		status_visible = true;	
		status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
		status_current = VIDEO_HAL_VPATH_STS_FAILED;
		vpath = VPATH_NAME_CARPLAY;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.instrumentcluster.control.current=CARPLAY. wait_for current.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* 42. 可視性ON */
		status_visible_expect = true;
		status_visible = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.instrumentcluster.control.visible=true. wait_for visible.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* MAPへ切替 */
		status_visible_expect = false;
		status_visible = true;	
		status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
		status_current = VIDEO_HAL_VPATH_STS_FAILED;
		vpath = VPATH_NAME_MAP;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });

		/* 可視性ON */
		status_visible_expect = true;
		status_visible = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.instrumentcluster.control.visible=false. wait_for visible.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.visible", callback_visible));

	KillAllRenderImage(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * 連続動作
 * 映像パス設定可否判定で結果trueだった場合 (ユーザーがサーフェス作成)
 * CARPLAYサーフェスが存在しない場合
 */
TEST(VhalSeqTestVpathIcRepeat, UserCreateSurfaceAvaiableIsTrueNoSurface)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_IC_CARPLAY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_IC_MAP));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, VPATH_NAME_MAP));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleIc(obj, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.visible", callback_visible));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		/* 1. 可視性OFF */
		status_visible_expect = false;
		status_visible = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.instrumentcluster.control.visible=false. wait_for visible.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);

		/* 15. CARPLAY設定可能チェック */
		available = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.status.available?vpath=CARPLAY", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(true, available);

		/* ユーザ作成サーフェス削除 */
		status_available_expect = false;
		status_available = true;
		cb_available_param = VPATH_NAME_CARPLAY;
		KillRenderImage(SURFACE_ID_IC_CARPLAY);
		PrintMsg("kill render_image. wait_for available.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
		EXPECT_EQ(true, intime);
		
		/* 21. CARPLAYへ切替 */
		status_current_expect = VIDEO_HAL_VPATH_STS_FAILED;
		status_current = VIDEO_HAL_VPATH_STS_SUCCESS;
		vpath = VPATH_NAME_CARPLAY;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.instrumentcluster.control.current=CARPLAY. wait_for current.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
		EXPECT_EQ(true, intime);

		/* CARPLAYサーフェス作成 */
		status_available_expect = true;
		status_available = false;
		cb_available_param = VPATH_NAME_CARPLAY;
		ExecuteRenderImage(obj, SURFACE_ID_IC_CARPLAY);
		PrintMsg("render_image CARPLAY. wait_for available.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_available_expect == status_available); });
		EXPECT_EQ(true, intime);

		/* MAPへ切替 */
		status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
		status_current = VIDEO_HAL_VPATH_STS_FAILED;
		vpath = VPATH_NAME_MAP;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.instrumentcluster.control.current=MAP. wait_for current.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current); });
		EXPECT_EQ(true, intime);

		/* 可視性ON */
		status_visible_expect = true;
		status_visible = false;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("SetValue vpath.instrumentcluster.control.visible=false. wait_for visible.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.visible", callback_visible));

	KillAllRenderImage(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * シーケンス抜け
 * 映像パス設定可否判定を取得せずに映像パスを切替える
 */
TEST_P(VhalSeqTestVpathIcSkip, Skip)
{
	VhalSeqTestVpathIcParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool available{false};
	std::string vpath;

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_IC_CARPLAY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_IC_MAP));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, VPATH_NAME_MAP));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleIc(obj, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.visible", callback_visible));

	/* 1. 可視性OFF */
	status_visible_expect = false;
	status_visible = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.instrumentcluster.control.visible=false. wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 15. CARPLAY設定可能チェック */
	available = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.status.available?vpath=CARPLAY", &available, sizeof(available), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(true, available);

	/* 21. CARPLAYへ切替 */
	status_visible_expect = false;
	status_visible = true;
	status_current_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_current = VIDEO_HAL_VPATH_STS_FAILED;
	vpath = VPATH_NAME_CARPLAY;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.instrumentcluster.control.current=CARPLAY. wait_for current.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_current_expect == status_current && status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	/* 42. 可視性ON */
	status_visible_expect = true;
	status_visible = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.visible", &status_visible_expect, sizeof(status_visible_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.instrumentcluster.control.visible=true. wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.available", callback_available));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.current", callback_current));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.visible", callback_visible));

	KillAllRenderImage(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}
