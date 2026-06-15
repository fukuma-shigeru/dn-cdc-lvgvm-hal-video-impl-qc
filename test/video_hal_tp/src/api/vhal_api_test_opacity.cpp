#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static uint32_t status_opacity;
static uint32_t status_opacity_expect;
static std::string cb_available_param;
static bool status_available;
static bool status_available_expect;
static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;

#define ABNORMAL_ARG_OPACITY (101)	/* 上限値＋１ */

static void CallbackOpacity(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.opacity", &status_opacity, sizeof(status_opacity), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_opacity_expect, status_opacity);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackOpacityRear(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.rear.status.opacity", &status_opacity, sizeof(status_opacity), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_opacity_expect, status_opacity);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackOpacityIc(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.status.opacity", &status_opacity, sizeof(status_opacity), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_opacity_expect, status_opacity);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackAvailable(void)
{
	PrintMsg("start. cb_available_param=" + cb_available_param);
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.instrumentcluster.status.available?vpath=" + cb_available_param, &status_available, sizeof(status_available), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(status_available_expect, status_available);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

struct VhalApiTestOpacityParameter {
	uint32_t opacity;
};

struct VhalApiTestOpacityNotSetParameter {
	int32_t surface_id;
	std::string vpath;
};

struct VhalApiTestOpacityIcNotSetParameter {
	std::string vpath;
	uint32_t surface_id;
};

class VhalApiTestOpacityNormal : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestOpacityParameter> {};
class VhalApiTestOpacityRange : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestOpacityParameter> {};
class VhalApiTestOpacityNotSet : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestOpacityNotSetParameter> {};
class VhalApiTestOpacityNotSetHdmi : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestOpacityNotSetParameter> {};
class VhalApiTestOpacityRepeat : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestOpacityParameter> {};
class VhalApiTestOpacityRearNormal : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestOpacityParameter> {};
class VhalApiTestOpacityRearRange : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestOpacityParameter> {};
class VhalApiTestOpacityRearNotSet : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestOpacityNotSetParameter> {};
class VhalApiTestOpacityRearNotSetHdmi : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestOpacityNotSetParameter> {};
class VhalApiTestOpacityRearRepeat : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestOpacityParameter> {};
class VhalApiTestOpacityIcNormal : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestOpacityParameter> {};
class VhalApiTestOpacityIcRange : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestOpacityParameter> {};
class VhalApiTestOpacityIcNotSet : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestOpacityIcNotSetParameter> {};
class VhalApiTestOpacityIcRepeat : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestOpacityParameter> {};
class VhalApiTestOpacityAbnormalArg : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestOpacityParameter> {};
class VhalApiTestOpacityAbnormalClear : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestOpacityParameter> {};
class VhalApiTestOpacityRearAbnormalArg : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestOpacityParameter> {};
class VhalApiTestOpacityRearAbnormalClear : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestOpacityParameter> {};
class VhalApiTestOpacityIcAbnormalArg : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestOpacityParameter> {};
class VhalApiTestOpacityIcAbnormalClear : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestOpacityParameter> {};

INSTANTIATE_TEST_CASE_P(VhalApiTestOpacity, VhalApiTestOpacityNormal, ::testing::Values(
	VhalApiTestOpacityParameter{50}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestOpacity, VhalApiTestOpacityRearNormal, ::testing::Values(
	VhalApiTestOpacityParameter{50}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestOpacity, VhalApiTestOpacityIcNormal, ::testing::Values(
	VhalApiTestOpacityParameter{50}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestOpacity, VhalApiTestOpacityRange, ::testing::Values(
	VhalApiTestOpacityParameter{0},
	VhalApiTestOpacityParameter{100}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestOpacity, VhalApiTestOpacityRearRange, ::testing::Values(
	VhalApiTestOpacityParameter{0},
	VhalApiTestOpacityParameter{100}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestOpacity, VhalApiTestOpacityIcRange, ::testing::Values(
	VhalApiTestOpacityParameter{0},
	VhalApiTestOpacityParameter{100}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestOpacity, VhalApiTestOpacityNotSet, ::testing::Values(
	VhalApiTestOpacityNotSetParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV},
	VhalApiTestOpacityNotSetParameter{SURFACE_ID_MEDIA_PLAYER, VPATH_NAME_MEDIAPLAYER},
	VhalApiTestOpacityNotSetParameter{SURFACE_ID_DRIVE_RECODER, VPATH_NAME_DRC},
	VhalApiTestOpacityNotSetParameter{SURFACE_ID_CARPLAY, VPATH_NAME_CARPLAY},
	VhalApiTestOpacityNotSetParameter{SURFACE_ID_ANDROIDAUTO, VPATH_NAME_ANDROIDAUTO},
	VhalApiTestOpacityNotSetParameter{SURFACE_ID_MULTISENSORY, VPATH_NAME_MULTISENSORY},
	VhalApiTestOpacityNotSetParameter{SURFACE_ID_FRAGRANCE, VPATH_NAME_FRAGRANCE}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestOpacity, VhalApiTestOpacityNotSetHdmi, ::testing::Values(
	VhalApiTestOpacityNotSetParameter{SURFACE_ID_FRONT_HDMI, VPATH_NAME_HDMI}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestOpacity, VhalApiTestOpacityRearNotSet, ::testing::Values(
	VhalApiTestOpacityNotSetParameter{SURFACE_ID_REAR_DTV, VPATH_NAME_DTV},
	VhalApiTestOpacityNotSetParameter{SURFACE_ID_REAR_MULTISENSORY, VPATH_NAME_MULTISENSORY}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestOpacity, VhalApiTestOpacityRearNotSetHdmi, ::testing::Values(
	VhalApiTestOpacityNotSetParameter{SURFACE_ID_REAR_HDMI, VPATH_NAME_HDMI}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestOpacity, VhalApiTestOpacityIcNotSet, ::testing::Values(
	VhalApiTestOpacityIcNotSetParameter{VPATH_NAME_MAP, SURFACE_ID_IC_MAP},
	VhalApiTestOpacityIcNotSetParameter{VPATH_NAME_CARPLAY, SURFACE_ID_IC_CARPLAY},
	VhalApiTestOpacityIcNotSetParameter{VPATH_NAME_ANDROIDAUTO, SURFACE_ID_IC_ANDROIDAUTO}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestOpacity, VhalApiTestOpacityRepeat, ::testing::Values(
	VhalApiTestOpacityParameter{50}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestOpacity, VhalApiTestOpacityRearRepeat, ::testing::Values(
	VhalApiTestOpacityParameter{50}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestOpacity, VhalApiTestOpacityIcRepeat, ::testing::Values(
	VhalApiTestOpacityParameter{50}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestOpacity, VhalApiTestOpacityAbnormalArg, ::testing::Values(
	VhalApiTestOpacityParameter{ABNORMAL_ARG_OPACITY}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestOpacity, VhalApiTestOpacityAbnormalClear, ::testing::Values(
	VhalApiTestOpacityParameter{50}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestOpacity, VhalApiTestOpacityRearAbnormalArg, ::testing::Values(
	VhalApiTestOpacityParameter{ABNORMAL_ARG_OPACITY}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestOpacity, VhalApiTestOpacityRearAbnormalClear, ::testing::Values(
	VhalApiTestOpacityParameter{50}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestOpacity, VhalApiTestOpacityIcAbnormalArg, ::testing::Values(
	VhalApiTestOpacityParameter{ABNORMAL_ARG_OPACITY}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestOpacity, VhalApiTestOpacityIcAbnormalClear, ::testing::Values(
	VhalApiTestOpacityParameter{50}
));

/**
 * 基本動作
 * 前席不透明度設定
 */
TEST_P(VhalApiTestOpacityNormal, Normal)
{
	VhalApiTestOpacityParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityFront(obj, DEFAULT_PROPERTY_OPACITY));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.opacity", CallbackOpacity));

	/* 不透明度設定 */
	status_opacity = param.opacity + 1;
	status_opacity_expect = param.opacity;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.opacity", &param.opacity, sizeof(param.opacity), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for opacity.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_opacity_expect == status_opacity); });
	EXPECT_EQ(true, intime);	

    uint32_t control_opacity{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.control.opacity", &control_opacity, sizeof(control_opacity), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(param.opacity, control_opacity);

	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.opacity", CallbackOpacity));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityFront(obj, DEFAULT_PROPERTY_OPACITY));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * 後席不透明度設定
 */
TEST_P(VhalApiTestOpacityRearNormal, Normal)
{
	VhalApiTestOpacityParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityRear(obj, DEFAULT_PROPERTY_OPACITY));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.opacity", CallbackOpacityRear));

	/* 不透明度設定 */
	status_opacity = param.opacity + 1;
	status_opacity_expect = param.opacity;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.opacity", &param.opacity, sizeof(param.opacity), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for opacity.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_opacity_expect == status_opacity); });
	EXPECT_EQ(true, intime);

    uint32_t control_opacity{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.rear.control.opacity", &control_opacity, sizeof(control_opacity), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(param.opacity, control_opacity);

	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_REAR, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetRearOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleRear(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.opacity", CallbackOpacityRear));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityRear(obj, DEFAULT_PROPERTY_OPACITY));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * IC不透明度設定
 */
TEST_P(VhalApiTestOpacityIcNormal, Normal)
{
	VhalApiTestOpacityParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_IC_MAP));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, VPATH_NAME_MAP));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityIc(obj, DEFAULT_PROPERTY_OPACITY));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.opacity", CallbackOpacityIc));

	/* 不透明度設定 */
	status_opacity = param.opacity + 1;
	status_opacity_expect = param.opacity;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.opacity", &param.opacity, sizeof(param.opacity), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for opacity.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_opacity_expect == status_opacity); });
	EXPECT_EQ(true, intime);

    uint32_t control_opacity{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.control.opacity", &control_opacity, sizeof(control_opacity), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(param.opacity, control_opacity);

	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_IC, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetIcOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleIc(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.opacity", CallbackOpacityIc));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityIc(obj, DEFAULT_PROPERTY_OPACITY));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ上限下限
 * 前席
 */
TEST_P(VhalApiTestOpacityRange, Range)
{
	VhalApiTestOpacityParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityFront(obj, 50));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.opacity", CallbackOpacity));

	/* 不透明度設定 */
	status_opacity = param.opacity + 1;
	status_opacity_expect = param.opacity;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.opacity", &param.opacity, sizeof(param.opacity), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for opacity.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_opacity_expect == status_opacity); });
	EXPECT_EQ(true, intime);	

    uint32_t control_opacity{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.control.opacity", &control_opacity, sizeof(control_opacity), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(param.opacity, control_opacity);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.opacity", CallbackOpacity));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityFront(obj, DEFAULT_PROPERTY_OPACITY));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ上限下限
 * 後席
 */
TEST_P(VhalApiTestOpacityRearRange, Range)
{
	VhalApiTestOpacityParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityRear(obj, 50));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.opacity", CallbackOpacityRear));

	/* 不透明度設定 */
	status_opacity = param.opacity + 1;
	status_opacity_expect = param.opacity;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.opacity", &param.opacity, sizeof(param.opacity), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for opacity.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_opacity_expect == status_opacity); });
	EXPECT_EQ(true, intime);	

    uint32_t control_opacity{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.rear.control.opacity", &control_opacity, sizeof(control_opacity), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(param.opacity, control_opacity);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.opacity", CallbackOpacityRear));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityRear(obj, DEFAULT_PROPERTY_OPACITY));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ上限下限
 * IC
 */
TEST_P(VhalApiTestOpacityIcRange, Range)
{
	VhalApiTestOpacityParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_IC_MAP));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, VPATH_NAME_MAP));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityIc(obj, 50));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.opacity", CallbackOpacityIc));

	/* 不透明度設定 */
	status_opacity = param.opacity + 1;
	status_opacity_expect = param.opacity;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.opacity", &param.opacity, sizeof(param.opacity), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for opacity.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_opacity_expect == status_opacity); });
	EXPECT_EQ(true, intime);	

    uint32_t control_opacity{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.control.opacity", &control_opacity, sizeof(control_opacity), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(param.opacity, control_opacity);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.opacity", CallbackOpacityIc));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityIc(obj, DEFAULT_PROPERTY_OPACITY));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 * 前席
 */
TEST_P(VhalApiTestOpacityNotSet, NotSet)
{
	VhalApiTestOpacityNotSetParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, param.vpath));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.opacity", &status_opacity, sizeof(status_opacity), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(100, status_opacity);

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 * 前席(HDMI)
 */
TEST_P(VhalApiTestOpacityNotSetHdmi, NotSet)
{
	VhalApiTestOpacityNotSetParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, param.vpath));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.front.status.opacity", &status_opacity, sizeof(status_opacity), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(100, status_opacity);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 * 後席
 */
TEST_P(VhalApiTestOpacityRearNotSet, NotSet)
{
	VhalApiTestOpacityNotSetParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, param.vpath));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.rear.status.opacity", &status_opacity, sizeof(status_opacity), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(100, status_opacity);

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 * 後席(HDMI)
 */
TEST_P(VhalApiTestOpacityRearNotSetHdmi, NotSet)
{
	VhalApiTestOpacityNotSetParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, param.vpath));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.rear.status.opacity", &status_opacity, sizeof(status_opacity), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(100, status_opacity);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 * IC
 */
TEST_P(VhalApiTestOpacityIcNotSet, NotSet)
{
	VhalApiTestOpacityIcNotSetParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.surface_id));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, param.vpath));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vpath.instrumentcluster.status.opacity", &status_opacity, sizeof(status_opacity), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(100, status_opacity);

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 * 前席
 */
TEST_P(VhalApiTestOpacityRepeat, Repeat)
{
	VhalApiTestOpacityParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityFront(obj, DEFAULT_PROPERTY_OPACITY));

	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.opacity", &param.opacity, sizeof(param.opacity), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	}

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityFront(obj, DEFAULT_PROPERTY_OPACITY));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 * 後席
 */
TEST_P(VhalApiTestOpacityRearRepeat, Repeat)
{
	VhalApiTestOpacityParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityRear(obj, DEFAULT_PROPERTY_OPACITY));

	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.opacity", &param.opacity, sizeof(param.opacity), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	}

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityRear(obj, DEFAULT_PROPERTY_OPACITY));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 * IC
 */
TEST_P(VhalApiTestOpacityIcRepeat, Repeat)
{
	VhalApiTestOpacityParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_IC_MAP));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, VPATH_NAME_MAP));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityIc(obj, DEFAULT_PROPERTY_OPACITY));

	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.opacity", &param.opacity, sizeof(param.opacity), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	}

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityIc(obj, DEFAULT_PROPERTY_OPACITY));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ異常
 * 前席
 */
TEST_P(VhalApiTestOpacityAbnormalArg, AbnormalArg)
{
	VhalApiTestOpacityParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityFront(obj, DEFAULT_PROPERTY_OPACITY));

	/* 不透明度設定 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.opacity", &param.opacity, sizeof(param.opacity), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityFront(obj, DEFAULT_PROPERTY_OPACITY));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ異常
 * 後席
 */
TEST_P(VhalApiTestOpacityRearAbnormalArg, AbnormalArg)
{
	VhalApiTestOpacityParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_REAR_DTV));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityRear(obj, DEFAULT_PROPERTY_OPACITY));

	/* 不透明度設定 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.opacity", &param.opacity, sizeof(param.opacity), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityRear(obj, DEFAULT_PROPERTY_OPACITY));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}


/**
 * パラメータ異常
 * IC
 */
TEST_P(VhalApiTestOpacityIcAbnormalArg, AbnormalArg)
{
	VhalApiTestOpacityParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_IC_MAP));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, VPATH_NAME_MAP));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityIc(obj, DEFAULT_PROPERTY_OPACITY));

	/* 不透明度設定 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.opacity", &param.opacity, sizeof(param.opacity), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetOpacityIc(obj, DEFAULT_PROPERTY_OPACITY));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 前提を満たさない条件でのコール
 * 映像パスクリアで不透明度設定
 * 前席
 */
TEST_P(VhalApiTestOpacityAbnormalClear, AbnormalClear)
{
	VhalApiTestOpacityParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	/* 不透明度設定 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.opacity", &param.opacity, sizeof(param.opacity), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 前提を満たさない条件でのコール
 * 映像パスクリアで不透明度設定
 * 後席
 */
TEST_P(VhalApiTestOpacityRearAbnormalClear, AbnormalClear)
{
	VhalApiTestOpacityParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRear(obj, VPATH_NAME_CLEAR));

	/* 不透明度設定 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.opacity", &param.opacity, sizeof(param.opacity), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 前提を満たさない条件でのコール
 * 映像パスクリアで不透明度設定
 * IC
 */
TEST_P(VhalApiTestOpacityIcAbnormalClear, AbnormalClear)
{
	VhalApiTestOpacityParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathIc(obj, VPATH_NAME_CLEAR));

	/* 不透明度設定 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.opacity", &param.opacity, sizeof(param.opacity), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	cockpit::hal::video_hal::Deinit(obj);
}