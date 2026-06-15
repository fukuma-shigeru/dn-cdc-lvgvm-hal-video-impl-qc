#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static bool status_visible;
static bool status_visible_expect;
static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;

static void CallbackVisible(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.visible", &status_visible, sizeof(status_visible), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(status_visible_expect, status_visible);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackVisibleRear(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.rear.status.visible", &status_visible, sizeof(status_visible), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(status_visible_expect, status_visible);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackVisibleIc(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.status.visible", &status_visible, sizeof(status_visible), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(status_visible_expect, status_visible);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

struct VhalApiTestVisibleParameter {
    bool visible;
};

struct VhalApiTestVisibleNotSetParameter {
	std::string vpath;
	uint32_t surface_id;
};

struct VhalApiTestVisibleIcNotSetParameter {
	std::string vpath;
	uint32_t surface_id;
};

class VhalApiTestVisibleNormal : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVisibleParameter> {};
class VhalApiTestVisibleNotSet : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVisibleNotSetParameter> {};
class VhalApiTestVisibleNotSetHdmi : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVisibleNotSetParameter> {};
class VhalApiTestVisibleRepeat : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVisibleParameter> {};
class VhalApiTestVisibleRearNormal : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVisibleParameter> {};
class VhalApiTestVisibleRearNotSet : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVisibleNotSetParameter> {};
class VhalApiTestVisibleRearNotSetHdmi : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVisibleNotSetParameter> {};
class VhalApiTestVisibleRearRepeat : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVisibleParameter> {};
class VhalApiTestVisibleIcNormal : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVisibleParameter> {};
class VhalApiTestVisibleIcNotSet : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVisibleIcNotSetParameter> {};
class VhalApiTestVisibleIcRepeat : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVisibleParameter> {};
class VhalApiTestVisibleAbnormalClear : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVisibleParameter> {};
class VhalApiTestVisibleRearAbnormalClear : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVisibleParameter> {};
class VhalApiTestVisibleIcAbnormalClear : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestVisibleParameter> {};

INSTANTIATE_TEST_CASE_P(VhalApiTestVisible, VhalApiTestVisibleNormal, ::testing::Values(
	VhalApiTestVisibleParameter{true},
	VhalApiTestVisibleParameter{false}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVisible, VhalApiTestVisibleRearNormal, ::testing::Values(
	VhalApiTestVisibleParameter{true},
	VhalApiTestVisibleParameter{false}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVisible, VhalApiTestVisibleIcNormal, ::testing::Values(
	VhalApiTestVisibleParameter{true},
	VhalApiTestVisibleParameter{false}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVisible, VhalApiTestVisibleNotSet, ::testing::Values(
	VhalApiTestVisibleNotSetParameter{VPATH_NAME_DTV, SURFACE_ID_FRONT_DTV,},
	VhalApiTestVisibleNotSetParameter{VPATH_NAME_MEDIAPLAYER, SURFACE_ID_MEDIA_PLAYER},
	VhalApiTestVisibleNotSetParameter{VPATH_NAME_DRC, SURFACE_ID_DRIVE_RECODER},
	VhalApiTestVisibleNotSetParameter{VPATH_NAME_CARPLAY, SURFACE_ID_CARPLAY},
	VhalApiTestVisibleNotSetParameter{VPATH_NAME_ANDROIDAUTO, SURFACE_ID_ANDROIDAUTO},
	VhalApiTestVisibleNotSetParameter{VPATH_NAME_MULTISENSORY, SURFACE_ID_MULTISENSORY},
	VhalApiTestVisibleNotSetParameter{VPATH_NAME_FRAGRANCE, SURFACE_ID_FRAGRANCE}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVisible, VhalApiTestVisibleNotSetHdmi, ::testing::Values(
	VhalApiTestVisibleNotSetParameter{VPATH_NAME_HDMI, SURFACE_ID_FRONT_HDMI}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVisible, VhalApiTestVisibleRearNotSet, ::testing::Values(
	VhalApiTestVisibleNotSetParameter{VPATH_NAME_DTV, SURFACE_ID_REAR_DTV},
	VhalApiTestVisibleNotSetParameter{VPATH_NAME_MULTISENSORY, SURFACE_ID_REAR_MULTISENSORY}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVisible, VhalApiTestVisibleRearNotSetHdmi, ::testing::Values(
	VhalApiTestVisibleNotSetParameter{VPATH_NAME_HDMI, SURFACE_ID_REAR_HDMI}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVisible, VhalApiTestVisibleIcNotSet, ::testing::Values(
	VhalApiTestVisibleIcNotSetParameter{VPATH_NAME_MAP, SURFACE_ID_IC_MAP},
	VhalApiTestVisibleIcNotSetParameter{VPATH_NAME_CARPLAY, SURFACE_ID_IC_CARPLAY},
	VhalApiTestVisibleIcNotSetParameter{VPATH_NAME_ANDROIDAUTO, SURFACE_ID_IC_ANDROIDAUTO}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVisible, VhalApiTestVisibleRepeat, ::testing::Values(
	VhalApiTestVisibleParameter{true}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVisible, VhalApiTestVisibleRearRepeat, ::testing::Values(
	VhalApiTestVisibleParameter{true}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVisible, VhalApiTestVisibleIcRepeat, ::testing::Values(
	VhalApiTestVisibleParameter{true}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVisible, VhalApiTestVisibleAbnormalClear, ::testing::Values(
	VhalApiTestVisibleParameter{true}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVisible, VhalApiTestVisibleRearAbnormalClear, ::testing::Values(
	VhalApiTestVisibleParameter{true}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestVisible, VhalApiTestVisibleIcAbnormalClear, ::testing::Values(
	VhalApiTestVisibleParameter{true}
));

/**
 * 基本動作
 * 可視状態設定
 */
TEST_P(VhalApiTestVisibleNormal, Normal)
{
	VhalApiTestVisibleParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);


	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, OTHER_BOOL_VALUE(param.visible)));

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.visible", CallbackVisible));

	/* 前席可視性設定 */
	status_visible = OTHER_BOOL_VALUE(param.visible);
	status_visible_expect = param.visible;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &param.visible, sizeof(param.visible), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);
    
    bool control_visible{param.visible};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.control.visible", &control_visible, sizeof(control_visible), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(param.visible, control_visible);

	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.visible", CallbackVisible));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * 後席可視状態設定
 */
TEST_P(VhalApiTestVisibleRearNormal, Normal)
{
	VhalApiTestVisibleParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);


	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_DTV));
	std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleRear(obj, OTHER_BOOL_VALUE(param.visible)));
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.visible", CallbackVisibleRear));

	/* 後席可視性設定 */
	status_visible = OTHER_BOOL_VALUE(param.visible);
	status_visible_expect = param.visible;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.visible", &param.visible, sizeof(param.visible), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);
    
    bool control_visible{param.visible};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.rear.control.visible", &control_visible, sizeof(control_visible), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(param.visible, control_visible);

	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_REAR, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetRearOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.visible", CallbackVisibleRear));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * IC可視状態設定
 */
TEST_P(VhalApiTestVisibleIcNormal, Normal)
{
	VhalApiTestVisibleParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_IC_MAP));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, VPATH_NAME_MAP));
	std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleIc(obj, OTHER_BOOL_VALUE(param.visible)));
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.visible", CallbackVisibleIc));

	/* IC可視性設定 */
	status_visible = OTHER_BOOL_VALUE(param.visible);
	status_visible_expect = param.visible;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.visible", &param.visible, sizeof(param.visible), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for visible.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_visible_expect == status_visible); });
	EXPECT_EQ(true, intime);
    
    bool control_visible{param.visible};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.control.visible", &control_visible, sizeof(control_visible), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(param.visible, control_visible);

	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_IC, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetIcOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.visible", CallbackVisibleIc));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleIc(obj, true));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 * 前席
 */
TEST_P(VhalApiTestVisibleNotSet, NotSet)
{
	VhalApiTestVisibleNotSetParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, param.vpath));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.visible", &status_visible, sizeof(status_visible), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, status_visible);

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 * 前席(HDMI)
 */
TEST_P(VhalApiTestVisibleNotSetHdmi, NotSet)
{
	VhalApiTestVisibleNotSetParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, param.vpath));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.visible", &status_visible, sizeof(status_visible), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, status_visible);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 * 後席
 */
TEST_P(VhalApiTestVisibleRearNotSet, NotSet)
{
	VhalApiTestVisibleNotSetParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);


	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, param.vpath));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.rear.status.visible", &status_visible, sizeof(status_visible), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, status_visible);

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 * 後席(HDMI)
 */
TEST_P(VhalApiTestVisibleRearNotSetHdmi, NotSet)
{
	VhalApiTestVisibleNotSetParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, param.vpath));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.rear.status.visible", &status_visible, sizeof(status_visible), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, status_visible);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 * IC
 */
TEST_P(VhalApiTestVisibleIcNotSet, NotSet)
{
	VhalApiTestVisibleIcNotSetParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);


	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, param.vpath));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.status.visible", &status_visible, sizeof(status_visible), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(false, status_visible);

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 * 前席
 */
TEST_P(VhalApiTestVisibleRepeat, Repeat)
{
	VhalApiTestVisibleParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);


	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, OTHER_BOOL_VALUE(param.visible)));

	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &param.visible, sizeof(param.visible), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	}

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 * 後席
 */
TEST_P(VhalApiTestVisibleRearRepeat, Repeat)
{
	VhalApiTestVisibleParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);


	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleRear(obj, OTHER_BOOL_VALUE(param.visible)));

	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.visible", &param.visible, sizeof(param.visible), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	}

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleRear(obj, true));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 * IC
 */
TEST_P(VhalApiTestVisibleIcRepeat, Repeat)
{
	VhalApiTestVisibleParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);


	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_IC_MAP));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, VPATH_NAME_MAP));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleIc(obj, OTHER_BOOL_VALUE(param.visible)));

	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.visible", &param.visible, sizeof(param.visible), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	}

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleIc(obj, true));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 前提を満たさない条件でのコール
 * 映像パスクリアで可視状態設定
 * 前席
 */
TEST_P(VhalApiTestVisibleAbnormalClear, AbnormalClear)
{
	VhalApiTestVisibleParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	/* 前席可視性設定 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &param.visible, sizeof(param.visible), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 前提を満たさない条件でのコール
 * 映像パスクリアで可視状態設定
 * 後席
 */
TEST_P(VhalApiTestVisibleRearAbnormalClear, AbnormalClear)
{
	VhalApiTestVisibleParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_CLEAR));

	/* 後席可視性設定 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.visible", &param.visible, sizeof(param.visible), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 前提を満たさない条件でのコール
 * 映像パスクリアで可視状態設定
 * IC
 */
TEST_P(VhalApiTestVisibleIcAbnormalClear, AbnormalClear)
{
	VhalApiTestVisibleParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, VPATH_NAME_CLEAR));

	/* IC可視性設定 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.visible", &param.visible, sizeof(param.visible), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	cockpit::hal::video_hal::Deinit(obj);
}
