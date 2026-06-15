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
static bool status_forced_multisensory_img_adj;
static bool status_forced_multisensory_img_adj_expect;
static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;

static void CallbackBrightness(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &status_brightness, sizeof(status_brightness), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_brightness_expect, status_brightness);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackContrast(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &status_contrast, sizeof(status_contrast), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_contrast_expect, status_contrast);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackForcedHmiImgAdj(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.forced_hmi_img_adj", &status_forced_hmi_img_adj, sizeof(status_forced_hmi_img_adj), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(status_forced_hmi_img_adj_expect, status_forced_hmi_img_adj);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackForcedMultisensoryImgAdj(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.forced_multisensory_img_adj", &status_forced_multisensory_img_adj, sizeof(status_forced_multisensory_img_adj), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(status_forced_multisensory_img_adj_expect, status_forced_multisensory_img_adj);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

struct VhalApiTestAdjustmentParameter {
	uint32_t brightness;
	uint32_t contrast;
};

struct VhalApiTestForcedHmiImgAdjParameter {
	bool enable;
};

struct VhalApiTestForcedMultisensoryImgAdjParameter {
	bool enable;
};

struct VhalApiTestAdjustmentBrightnessParameter {
	uint32_t brightness;
};

struct VhalApiTestAdjustmentContrastParameter {
	uint32_t contrast;
};

class VhalApiTestAdjustmentNormal : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestAdjustmentParameter> {};
class VhalApiTestForcedHmiImgAdj : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestForcedHmiImgAdjParameter> {};
class VhalApiTestForcedMultisensoryImgAdj : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestForcedMultisensoryImgAdjParameter> {};
class VhalApiTestAdjustmentRange : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestAdjustmentParameter> {};
class VhalApiTestAdjustmentRepeat : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestAdjustmentParameter> {};
class VhalApiTestForcedHmiImgAdjRepeat : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestForcedHmiImgAdjParameter> {};
class VhalApiTestForcedMultisensoryImgAdjRepeat : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestForcedMultisensoryImgAdjParameter> {};
class VhalApiTestAdjustmentBrightnessAbnormalArg : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestAdjustmentBrightnessParameter> {};
class VhalApiTestAdjustmentContrastAbnormalArg : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestAdjustmentContrastParameter> {};

INSTANTIATE_TEST_CASE_P(VhalApiTestAdjustment, VhalApiTestAdjustmentNormal, ::testing::Values(
	VhalApiTestAdjustmentParameter{40, 40}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestAdjustment, VhalApiTestForcedHmiImgAdj, ::testing::Values(
	VhalApiTestForcedHmiImgAdjParameter{true},
	VhalApiTestForcedHmiImgAdjParameter{false}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestAdjustment, VhalApiTestForcedMultisensoryImgAdj, ::testing::Values(
	VhalApiTestForcedMultisensoryImgAdjParameter{true},
	VhalApiTestForcedMultisensoryImgAdjParameter{false}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestAdjustment, VhalApiTestAdjustmentRange, ::testing::Values(
	VhalApiTestAdjustmentParameter{1, 40},
	VhalApiTestAdjustmentParameter{63, 40},
	VhalApiTestAdjustmentParameter{40, 1},
	VhalApiTestAdjustmentParameter{1, 63}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestAdjustment, VhalApiTestAdjustmentRepeat, ::testing::Values(
	VhalApiTestAdjustmentParameter{40, 40}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestAdjustment, VhalApiTestForcedHmiImgAdjRepeat, ::testing::Values(
	VhalApiTestForcedHmiImgAdjParameter{true},
	VhalApiTestForcedHmiImgAdjParameter{false}
));
INSTANTIATE_TEST_CASE_P(VhalApiTestAdjustment, VhalApiTestForcedMultisensoryImgAdjRepeat, ::testing::Values(
	VhalApiTestForcedMultisensoryImgAdjParameter{true},
	VhalApiTestForcedMultisensoryImgAdjParameter{false}
));
INSTANTIATE_TEST_CASE_P(VhalApiTestAdjustment, VhalApiTestAdjustmentBrightnessAbnormalArg, ::testing::Values(
	VhalApiTestAdjustmentBrightnessParameter{0},
	VhalApiTestAdjustmentBrightnessParameter{64}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestAdjustment, VhalApiTestAdjustmentContrastAbnormalArg, ::testing::Values(
	VhalApiTestAdjustmentContrastParameter{0},
	VhalApiTestAdjustmentContrastParameter{64}
));

/* 基本動作 */
TEST_P(VhalApiTestAdjustmentNormal, Normal)
{
	VhalApiTestAdjustmentParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);


	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, VIDEO_HAL_VSRC_ID_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetBrightness(obj, DEFAULT_PROPERTY_BRIGHTNESS));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetContrast(obj, DEFAULT_PROPERTY_CONTRAST));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.brightness.step", CallbackBrightness));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.contrast.step", CallbackContrast));

	/* 明るさ設定 */
	status_brightness = DEFAULT_PROPERTY_BRIGHTNESS;
	status_brightness_expect = param.brightness;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.brightness.step", &param.brightness, sizeof(param.brightness), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for brightness.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_brightness_expect == status_brightness); });
	EXPECT_EQ(true, intime);

    uint32_t control_brightness{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.control.brightness.step", &control_brightness, sizeof(control_brightness), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(param.brightness, control_brightness);

    /* コントラスト設定 */
	status_contrast = DEFAULT_PROPERTY_CONTRAST;
	status_contrast_expect = param.contrast;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.contrast.step", &param.contrast, sizeof(param.contrast), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for contrast.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_contrast_expect == status_contrast); });
	EXPECT_EQ(true, intime);

    uint32_t control_contrast{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.control.contrast.step", &control_contrast, sizeof(control_contrast), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(param.contrast, control_contrast);

	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.brightness.step", CallbackBrightness));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.contrast.step", CallbackContrast));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetContrast(obj, DEFAULT_PROPERTY_CONTRAST));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetBrightness(obj, DEFAULT_PROPERTY_BRIGHTNESS));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, DEFAULT_PROPERTY_VSRC_ID));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/* 基本動作 */
TEST_P(VhalApiTestForcedHmiImgAdj, Normal)
{
	VhalApiTestForcedHmiImgAdjParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
    bool current_forced_hmi_img_adj{};

	cockpit::hal::video_hal::Init(obj);

	/* HMI画質調整 */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, VIDEO_HAL_VSRC_ID_OTHER));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetBrightness(obj, DEFAULT_PROPERTY_BRIGHTNESS));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetContrast(obj, DEFAULT_PROPERTY_CONTRAST));

	/* 映像画質調整 */
	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, VIDEO_HAL_VSRC_ID_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetBrightness(obj, (DEFAULT_PROPERTY_BRIGHTNESS + 1)));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetContrast(obj, (DEFAULT_PROPERTY_BRIGHTNESS - 1)));

	/* 設定値反転 */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetForcedHmiImgAdj(obj, OTHER_BOOL_VALUE(param.enable)));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.forced_hmi_img_adj", CallbackForcedHmiImgAdj));

	/* 強制HMI画質設定 */
	status_forced_hmi_img_adj = OTHER_BOOL_VALUE(param.enable);
	status_forced_hmi_img_adj_expect = param.enable;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.forced_hmi_img_adj", &param.enable, sizeof(param.enable), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for forced_hmi_img_adj.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_forced_hmi_img_adj_expect == status_forced_hmi_img_adj); });
	EXPECT_EQ(true, intime);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.forced_hmi_img_adj", &current_forced_hmi_img_adj, sizeof(current_forced_hmi_img_adj), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(param.enable, current_forced_hmi_img_adj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.forced_hmi_img_adj", CallbackForcedHmiImgAdj));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, VIDEO_HAL_VSRC_ID_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetContrast(obj, DEFAULT_PROPERTY_CONTRAST));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetBrightness(obj, DEFAULT_PROPERTY_BRIGHTNESS));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, DEFAULT_PROPERTY_VSRC_ID));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/* 多感覚連携 */
TEST_P(VhalApiTestForcedMultisensoryImgAdj, Normal)
{
	/* initialize */
	VhalApiTestForcedMultisensoryImgAdjParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
    bool current_forced_multisensory_img_adj{};
	cockpit::hal::video_hal::Init(obj);

	/* 多感覚連携 映像 */
	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_MULTISENSORY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, VIDEO_HAL_VSRC_ID_MULTISENSORY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetBrightness(obj, (DEFAULT_PROPERTY_BRIGHTNESS + 1)));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetContrast(obj, (DEFAULT_PROPERTY_BRIGHTNESS - 1)));

	/* 強制多感覚連携画質設定 */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetForcedMultisensoryImgAdj(obj, OTHER_BOOL_VALUE(param.enable)));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.forced_multisensory_img_adj", CallbackForcedMultisensoryImgAdj));
	status_forced_multisensory_img_adj = OTHER_BOOL_VALUE(param.enable);
	status_forced_multisensory_img_adj_expect = param.enable;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.forced_multisensory_img_adj", &param.enable, sizeof(param.enable), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for forced_multisensory_img_adj.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_forced_multisensory_img_adj_expect == status_forced_multisensory_img_adj); });
	EXPECT_EQ(true, intime);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.forced_multisensory_img_adj", &current_forced_multisensory_img_adj, sizeof(current_forced_multisensory_img_adj), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(param.enable, current_forced_multisensory_img_adj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.forced_multisensory_img_adj", CallbackForcedMultisensoryImgAdj));

	/* 既定値に戻す */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, VIDEO_HAL_VSRC_ID_MULTISENSORY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetContrast(obj, DEFAULT_PROPERTY_CONTRAST));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetBrightness(obj, DEFAULT_PROPERTY_BRIGHTNESS));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, DEFAULT_PROPERTY_VSRC_ID));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	/* finalize */
	KillAllRenderImage(obj);
	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ上限下限
 */
TEST_P(VhalApiTestAdjustmentRange, Range)
{
	VhalApiTestAdjustmentParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);


	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, VIDEO_HAL_VSRC_ID_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetBrightness(obj, DEFAULT_PROPERTY_BRIGHTNESS));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetContrast(obj, DEFAULT_PROPERTY_CONTRAST));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.brightness.step", CallbackBrightness));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.contrast.step", CallbackContrast));

	/* 明るさ設定 */
	status_brightness = DEFAULT_PROPERTY_BRIGHTNESS;
	status_brightness_expect = param.brightness;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.brightness.step", &param.brightness, sizeof(param.brightness), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for brightness.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_brightness_expect == status_brightness); });
	EXPECT_EQ(true, intime);

    uint32_t control_brightness{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.control.brightness.step", &control_brightness, sizeof(control_brightness), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(param.brightness, control_brightness);

    /* コントラスト設定 */
	status_contrast = DEFAULT_PROPERTY_CONTRAST;
	status_contrast_expect = param.contrast;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.contrast.step", &param.contrast, sizeof(param.contrast), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for contrast.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_contrast_expect == status_contrast); });
	EXPECT_EQ(true, intime);

    uint32_t control_contrast{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.control.contrast.step", &control_contrast, sizeof(control_contrast), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(param.contrast, control_contrast);

	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetFrontOutput(obj, DEFAULT_PROPERTY_OUTPUT_X, DEFAULT_PROPERTY_OUTPUT_Y, prop.screenWidth, prop.screenHeight, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVisibleFront(obj, true));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.brightness.step", CallbackBrightness));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.contrast.step", CallbackContrast));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetContrast(obj, DEFAULT_PROPERTY_CONTRAST));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetBrightness(obj, DEFAULT_PROPERTY_BRIGHTNESS));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, DEFAULT_PROPERTY_VSRC_ID));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照 
 */
TEST(VhalApiTestAdjustmentNotSet, NotSet)
{
	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &status_brightness, sizeof(status_brightness), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(32, status_brightness);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &status_contrast, sizeof(status_contrast), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(32, status_contrast);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照 
 * 強制HMI画質設定
 */
TEST(VhalApiTestForcedHmiImgAdjNotSet, NotSet)
{
	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.forced_hmi_img_adj", &status_forced_hmi_img_adj, sizeof(status_forced_hmi_img_adj), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(DEFAULT_PROPERTY_FORCED_HMI_IMG_ADJ, status_forced_hmi_img_adj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照 
 * 強制 多感覚連携 画質設定
 */
TEST(VhalApiTestForcedMultisensoryImgAdjNotSet, NotSet)
{
	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.forced_multisensory_img_adj", &status_forced_multisensory_img_adj, sizeof(status_forced_multisensory_img_adj), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(DEFAULT_PROPERTY_FORCED_HMI_IMG_ADJ, status_forced_multisensory_img_adj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 */
TEST_P(VhalApiTestAdjustmentRepeat, Repeat)
{
	VhalApiTestAdjustmentParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, VIDEO_HAL_VSRC_ID_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetBrightness(obj, DEFAULT_PROPERTY_BRIGHTNESS));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetContrast(obj, DEFAULT_PROPERTY_CONTRAST));

	/* 明るさ設定 */
	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.brightness.step", &param.brightness, sizeof(param.brightness), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	}

    /* コントラスト設定 */
	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.contrast.step", &param.contrast, sizeof(param.contrast), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	}

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetContrast(obj, DEFAULT_PROPERTY_CONTRAST));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetBrightness(obj, DEFAULT_PROPERTY_BRIGHTNESS));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, DEFAULT_PROPERTY_VSRC_ID));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 * 強制HMI画質設定
 */
TEST_P(VhalApiTestForcedHmiImgAdjRepeat, Repeat)
{
	VhalApiTestForcedHmiImgAdjParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.forced_hmi_img_adj", &param.enable, sizeof(param.enable), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	}

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 * 強制多感覚連携画質設定
 */
TEST_P(VhalApiTestForcedMultisensoryImgAdjRepeat, Repeat)
{
	VhalApiTestForcedMultisensoryImgAdjParameter param = GetParam();
	cockpit::hal::video_hal::Init(obj);
	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_MULTISENSORY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_MULTISENSORY));
	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.forced_multisensory_img_adj", &param.enable, sizeof(param.enable), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	}
	KillAllRenderImage(obj);
	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ異常
 * 明るさ
 */
TEST_P(VhalApiTestAdjustmentBrightnessAbnormalArg, AbnormalArg)
{
	VhalApiTestAdjustmentBrightnessParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, VIDEO_HAL_VSRC_ID_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetBrightness(obj, DEFAULT_PROPERTY_BRIGHTNESS));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetContrast(obj, DEFAULT_PROPERTY_CONTRAST));

	/* 明るさ設定 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.brightness.step", &param.brightness, sizeof(param.brightness), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetContrast(obj, DEFAULT_PROPERTY_CONTRAST));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetBrightness(obj, DEFAULT_PROPERTY_BRIGHTNESS));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, DEFAULT_PROPERTY_VSRC_ID));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ異常
 * コントラスト
 */
TEST_P(VhalApiTestAdjustmentContrastAbnormalArg, AbnormalArg)
{
	VhalApiTestAdjustmentContrastParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);


	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, VIDEO_HAL_VSRC_ID_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetBrightness(obj, DEFAULT_PROPERTY_BRIGHTNESS));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetContrast(obj, DEFAULT_PROPERTY_CONTRAST));

    /* コントラスト設定 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.contrast.step", &param.contrast, sizeof(param.contrast), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	EXPECT_EQ(VHAL_TEST_SUCCESS, SetContrast(obj, DEFAULT_PROPERTY_CONTRAST));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetBrightness(obj, DEFAULT_PROPERTY_BRIGHTNESS));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, DEFAULT_PROPERTY_VSRC_ID));

	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

static uint32_t status_video_source_id;
static uint32_t status_video_source_id_expect;
static uint32_t status_daynight;
static uint32_t status_daynight_expect;
static uint32_t status_theme_color;
static uint32_t status_theme_color_expect;

static void CallbackVideoSourceId(void)
{
	std::lock_guard<std::mutex> lock_sync(mtx_sync);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.id", &status_video_source_id, sizeof(status_video_source_id), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(status_video_source_id_expect, status_video_source_id);
	cond_sync.notify_one();
}

static void CallbackDayNight(void)
{
	std::lock_guard<std::mutex> lock_sync(mtx_sync);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.day_night", &status_daynight, sizeof(status_daynight), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(status_daynight_expect, status_daynight);
	cond_sync.notify_one();
}

static void CallbackThemeColor(void)
{
	std::lock_guard<std::mutex> lock_sync(mtx_sync);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.theme_color", &status_theme_color, sizeof(status_theme_color), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(status_theme_color_expect, status_theme_color);
	cond_sync.notify_one();
}

/* 連動機能により期待値に設定されない可能性があるため、設定後の値を確認しない */
static void CallbackBrightness_NoCheck(void)
{
	std::lock_guard<std::mutex> lock_sync(mtx_sync);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &status_brightness, sizeof(status_brightness), VIDEO_HAL_DATA_TYPE_NUM));
	cond_sync.notify_one();
}

/* 連動機能により期待値に設定されない可能性があるため、設定後の値を確認しない */
static void CallbackContrast_NoCheck(void)
{
	std::lock_guard<std::mutex> lock_sync(mtx_sync);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &status_contrast, sizeof(status_contrast), VIDEO_HAL_DATA_TYPE_NUM));
	cond_sync.notify_one();
}

static bool SetVideoSource(	cockpit::hal::video_hal::CtlObj& ctl_obj,
							std::unique_lock<std::mutex>& unique_lock,
							uint32_t video_source_id)
{
	bool ret{true};
	uint32_t video_source_id_current;
	if (VIDEO_HAL_API_SUCCESS != cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.id", &video_source_id_current, sizeof(video_source_id_current), VIDEO_HAL_DATA_TYPE_NUM))
		ret = false;
	else if (video_source_id_current == video_source_id)
		; // 設定してもコールバックが発生せずエラーとなる
	else
	{
		status_video_source_id_expect = video_source_id;
		status_video_source_id = status_video_source_id_expect + 1;
		if (VIDEO_HAL_API_SUCCESS != cockpit::hal::video_hal::SetValue(ctl_obj, "vsrc.front.control.id", &status_video_source_id_expect, sizeof(status_video_source_id_expect), VIDEO_HAL_DATA_TYPE_NUM))
			ret = false;
		if (VIDEO_HAL_API_SUCCESS != cockpit::hal::video_hal::RequestUpdate(ctl_obj))
			ret = false;
		if (!cond_sync.wait_for(unique_lock, std::chrono::seconds(timeout_seconds), [] { return (status_video_source_id_expect == status_video_source_id); }))
			ret = false;
	}
	return ret;
}

static bool SetDayNight(cockpit::hal::video_hal::CtlObj& ctl_obj,
						std::unique_lock<std::mutex>& unique_lock,
						uint32_t day_night)
{
	bool ret{true};
	uint32_t day_night_current;
	if (VIDEO_HAL_API_SUCCESS != cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.day_night", &day_night_current, sizeof(day_night_current), VIDEO_HAL_DATA_TYPE_NUM))
		ret = false;
	else if (day_night_current == day_night)
		; // 設定してもコールバックが発生せずエラーとなる
	else
	{
		status_daynight_expect = day_night;
		status_daynight = status_daynight_expect + 1;
		if (VIDEO_HAL_API_SUCCESS != cockpit::hal::video_hal::SetValue(ctl_obj, "vhal.setting.control.day_night", &status_daynight_expect, sizeof(status_daynight_expect), VIDEO_HAL_DATA_TYPE_NUM))
			ret = false;
		if (VIDEO_HAL_API_SUCCESS != cockpit::hal::video_hal::RequestUpdate(ctl_obj))
			ret = false;
		if (!cond_sync.wait_for(unique_lock, std::chrono::seconds(timeout_seconds), [] { return (status_daynight_expect == status_daynight); }))
			ret = false;
	}
	return ret;
}

static bool SetThemeColor(	cockpit::hal::video_hal::CtlObj& ctl_obj,
							std::unique_lock<std::mutex>& unique_lock,
							uint32_t theme_color)
{
	bool ret{true};
	uint32_t theme_color_current;
	if (VIDEO_HAL_API_SUCCESS != cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.theme_color", &theme_color_current, sizeof(theme_color_current), VIDEO_HAL_DATA_TYPE_NUM))
		ret = false;
	else if (theme_color_current == theme_color)
		; // 設定してもコールバックが発生せずエラーとなる
	else
	{
		status_theme_color_expect = theme_color;
		status_theme_color = status_theme_color_expect + 1;
		if (VIDEO_HAL_API_SUCCESS != cockpit::hal::video_hal::SetValue(ctl_obj, "vhal.setting.control.theme_color", &status_theme_color_expect, sizeof(status_theme_color_expect), VIDEO_HAL_DATA_TYPE_NUM))
			ret = false;
		if (VIDEO_HAL_API_SUCCESS != cockpit::hal::video_hal::RequestUpdate(ctl_obj))
			ret = false;
		if (!cond_sync.wait_for(unique_lock, std::chrono::seconds(timeout_seconds), [] { return (status_theme_color_expect == status_theme_color); }))
			ret = false;
	}
	return ret;
}

static bool SetBrightness(	cockpit::hal::video_hal::CtlObj& ctl_obj,
							std::unique_lock<std::mutex>& unique_lock,
							uint32_t brightness)
{
	bool ret{true};
	uint32_t brightness_current;
	if (VIDEO_HAL_API_SUCCESS != cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &brightness_current, sizeof(brightness_current), VIDEO_HAL_DATA_TYPE_NUM))
		ret = false;
	else if (brightness_current == brightness)
		; // 設定してもコールバックが発生せずエラーとなる
	else
	{
		status_brightness_expect = brightness;
		status_brightness = status_brightness_expect + 1;
		if (VIDEO_HAL_API_SUCCESS != cockpit::hal::video_hal::SetValue(ctl_obj, "vsrc.front.control.brightness.step", &status_brightness_expect, sizeof(status_brightness_expect), VIDEO_HAL_DATA_TYPE_NUM))
			ret = false;
		if (VIDEO_HAL_API_SUCCESS != cockpit::hal::video_hal::RequestUpdate(ctl_obj))
			ret = false;
		if (!cond_sync.wait_for(unique_lock, std::chrono::seconds(timeout_seconds), [] { return (status_brightness_expect == status_brightness); }))
			ret = false;
	}
	return ret;
}

static bool SetContrast(cockpit::hal::video_hal::CtlObj& ctl_obj,
						std::unique_lock<std::mutex>& unique_lock,
						uint32_t contrast)
{
	bool ret{true};
	uint32_t contrast_current;
	if (VIDEO_HAL_API_SUCCESS != cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &contrast_current, sizeof(contrast_current), VIDEO_HAL_DATA_TYPE_NUM))
		ret = false;
	else if (contrast_current == contrast)
		; // 設定してもコールバックが発生せずエラーとなる
	else
	{
		status_contrast_expect = contrast;
		status_contrast = status_contrast_expect + 1;
		if (VIDEO_HAL_API_SUCCESS != cockpit::hal::video_hal::SetValue(ctl_obj, "vsrc.front.control.contrast.step", &status_contrast_expect, sizeof(status_contrast_expect), VIDEO_HAL_DATA_TYPE_NUM))
			ret = false;
		if (VIDEO_HAL_API_SUCCESS != cockpit::hal::video_hal::RequestUpdate(ctl_obj))
			ret = false;
		if (!cond_sync.wait_for(unique_lock, std::chrono::seconds(timeout_seconds), [] { return (status_contrast_expect == status_contrast); }))
			ret = false;
	}
	return ret;
}

/* 
画質調整ステップ値の連動、非連動評価。
　各画質調整モード×昼夜/テーマカラーに対して、一意の輝度=1,2,3...10 コントラスト=31,32,33...40 をステップ値として書き込む。
　書き込んだステップ値を読み込んで、ステップ値が変化していないかを確認する。（変化していない＝連動していない）
　DTVとHDMIは連動する仕様のため、固定値で連動していることを評価する。
*/
TEST(VhalApiTestAdjustment, Link)
{
	const uint32_t BRIGHTNESS_START = 1;
	const uint32_t CONTRAST_START = 31;
	const uint32_t DTV_BRIGHTNESS_DAY = 5;
	const uint32_t DTV_BRIGHTNESS_NIGHT = 6;
	const uint32_t HDMI_BRIGHTNESS_NIGHT = 7;
	const uint32_t HDMI_BRIGHTNESS_DAY = 8;
	const uint32_t DTV_CONTRAST_DAY = 35;
	const uint32_t DTV_CONTRAST_NIGHT = 36;
	const uint32_t HDMI_CONTRAST_NIGHT = 37;
	const uint32_t HDMI_CONTRAST_DAY = 38;
	bool intime{false};
	std::unique_lock<std::mutex> lock(mtx_sync);
	uint32_t brightness;
	uint32_t contrast;
	uint32_t num;

	// initialize
	cockpit::hal::video_hal::Init(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.id", CallbackVideoSourceId));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.day_night", CallbackDayNight));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.theme_color", CallbackThemeColor));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.brightness.step", CallbackBrightness_NoCheck));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.contrast.step", CallbackContrast_NoCheck));

	// set (ascending) ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// CAMERA
	EXPECT_EQ(true, SetVideoSource(obj, lock, VIDEO_HAL_VSRC_ID_IMAGE_ADJUST_CAM));
	// day
	EXPECT_EQ(true, SetDayNight(obj, lock, VIDEO_HAL_SETTING_DAY));
	brightness = BRIGHTNESS_START;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast = CONTRAST_START;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));
	// night
	EXPECT_EQ(true, SetDayNight(obj, lock, VIDEO_HAL_SETTING_NIGHT));
	brightness ++;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast ++;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));

	// DTV
	EXPECT_EQ(true, SetVideoSource(obj, lock, VIDEO_HAL_VSRC_ID_DTV));
	// day
	EXPECT_EQ(true, SetDayNight(obj, lock, VIDEO_HAL_SETTING_DAY));
	brightness ++;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast ++;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));
	// night
	EXPECT_EQ(true, SetDayNight(obj, lock, VIDEO_HAL_SETTING_NIGHT));
	brightness ++;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast ++;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));

	// HDMI
	EXPECT_EQ(true, SetVideoSource(obj, lock, VIDEO_HAL_VSRC_ID_HDMI));
	// day
	EXPECT_EQ(true, SetDayNight(obj, lock, VIDEO_HAL_SETTING_DAY));
	brightness ++;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast ++;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));
	// night
	EXPECT_EQ(true, SetDayNight(obj, lock, VIDEO_HAL_SETTING_NIGHT));
	brightness ++;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast ++;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));

	// HMI
	EXPECT_EQ(true, SetVideoSource(obj, lock, VIDEO_HAL_VSRC_ID_OTHER));
	// theme 0
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_AUTO_LIGHT));
	brightness ++;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast ++;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));
	// theme 1
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_AUTO_DARK));
	brightness ++;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast ++;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));
	// theme 2
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_SELECT_LIGHT));
	brightness ++;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast ++;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));
	// theme 3
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_SELECT_DARK));
	brightness ++;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast ++;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));

	// Multisensory
	EXPECT_EQ(true, SetVideoSource(obj, lock, VIDEO_HAL_VSRC_ID_MULTISENSORY));
	// theme 0
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_AUTO_LIGHT));
	brightness ++;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast ++;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));
	// theme 1
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_AUTO_DARK));
	brightness ++;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast ++;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));
	// theme 2
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_SELECT_LIGHT));
	brightness ++;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast ++;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));
	// theme 3
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_SELECT_DARK));
	brightness ++;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast ++;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));

	// get (ascending) ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// CAMERA
	EXPECT_EQ(true, SetVideoSource(obj, lock, VIDEO_HAL_VSRC_ID_IMAGE_ADJUST_CAM));
	// day
	EXPECT_EQ(true, SetDayNight(obj, lock, VIDEO_HAL_SETTING_DAY));
	brightness = BRIGHTNESS_START;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(brightness, num); PrintMsg(std::to_string(num) + "\n");
	contrast = CONTRAST_START;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(contrast, num); PrintMsg(std::to_string(num) + "\n");
	// night
	EXPECT_EQ(true, SetDayNight(obj, lock, VIDEO_HAL_SETTING_NIGHT));
	brightness ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(brightness, num); PrintMsg(std::to_string(num) + "\n");
	contrast ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(contrast, num); PrintMsg(std::to_string(num) + "\n");

	// DTV
	EXPECT_EQ(true, SetVideoSource(obj, lock, VIDEO_HAL_VSRC_ID_DTV));
	// day
	EXPECT_EQ(true, SetDayNight(obj, lock, VIDEO_HAL_SETTING_DAY));
	brightness ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(DTV_BRIGHTNESS_DAY, num); PrintMsg(std::to_string(num) + "\n");
	contrast ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(DTV_CONTRAST_DAY, num); PrintMsg(std::to_string(num) + "\n");
	// night
	EXPECT_EQ(true, SetDayNight(obj, lock, VIDEO_HAL_SETTING_NIGHT));
	brightness ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(DTV_BRIGHTNESS_NIGHT, num); PrintMsg(std::to_string(num) + "\n");
	contrast ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(DTV_CONTRAST_NIGHT, num); PrintMsg(std::to_string(num) + "\n");

	// HDMI
	EXPECT_EQ(true, SetVideoSource(obj, lock, VIDEO_HAL_VSRC_ID_HDMI));
	// day
	EXPECT_EQ(true, SetDayNight(obj, lock, VIDEO_HAL_SETTING_DAY));
	brightness ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(brightness, num); PrintMsg(std::to_string(num) + "\n");
	contrast ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(contrast, num); PrintMsg(std::to_string(num) + "\n");
	// night
	EXPECT_EQ(true, SetDayNight(obj, lock, VIDEO_HAL_SETTING_NIGHT));
	brightness ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(brightness, num); PrintMsg(std::to_string(num) + "\n");
	contrast ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(contrast, num); PrintMsg(std::to_string(num) + "\n");

	// HMI
	EXPECT_EQ(true, SetVideoSource(obj, lock, VIDEO_HAL_VSRC_ID_OTHER));
	// theme 0
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_AUTO_LIGHT));
	brightness ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(brightness, num); PrintMsg(std::to_string(num) + "\n");
	contrast ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(contrast, num); PrintMsg(std::to_string(num) + "\n");
	// theme 1
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_AUTO_DARK));
	brightness ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(brightness, num); PrintMsg(std::to_string(num) + "\n");
	contrast ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(contrast, num); PrintMsg(std::to_string(num) + "\n");
	// theme 2
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_SELECT_LIGHT));
	brightness ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(brightness, num); PrintMsg(std::to_string(num) + "\n");
	contrast ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(contrast, num); PrintMsg(std::to_string(num) + "\n");
	// theme 3
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_SELECT_DARK));
	brightness ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(brightness, num); PrintMsg(std::to_string(num) + "\n");
	contrast ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(contrast, num); PrintMsg(std::to_string(num) + "\n");
	

	// Multisensory
	EXPECT_EQ(true, SetVideoSource(obj, lock, VIDEO_HAL_VSRC_ID_MULTISENSORY));
	// theme 0
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_AUTO_LIGHT));
	brightness ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(brightness, num); PrintMsg(std::to_string(num) + "\n");
	contrast ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(contrast, num); PrintMsg(std::to_string(num) + "\n");
	// theme 1
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_AUTO_DARK));
	brightness ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(brightness, num); PrintMsg(std::to_string(num) + "\n");
	contrast ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(contrast, num); PrintMsg(std::to_string(num) + "\n");
	// theme 2
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_SELECT_LIGHT));
	brightness ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(brightness, num); PrintMsg(std::to_string(num) + "\n");
	contrast ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(contrast, num); PrintMsg(std::to_string(num) + "\n");
	// theme 3
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_SELECT_DARK));
	brightness ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(brightness, num); PrintMsg(std::to_string(num) + "\n");
	contrast ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(contrast, num); PrintMsg(std::to_string(num) + "\n");

	// set (descending) ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Multisensory
	EXPECT_EQ(true, SetVideoSource(obj, lock, VIDEO_HAL_VSRC_ID_MULTISENSORY));
	// theme 3
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_SELECT_DARK));
	brightness = BRIGHTNESS_START;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast = CONTRAST_START;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));
	// theme 2
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_SELECT_LIGHT));
	brightness ++;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast ++;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));
	// theme 1
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_AUTO_DARK));
	brightness ++;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast ++;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));
	// theme 0
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_AUTO_LIGHT));
	brightness ++;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast ++;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));

	// HMI
	EXPECT_EQ(true, SetVideoSource(obj, lock, VIDEO_HAL_VSRC_ID_OTHER));
	// theme 3
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_SELECT_DARK));
	brightness = BRIGHTNESS_START;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast = CONTRAST_START;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));
	// theme 2
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_SELECT_LIGHT));
	brightness ++;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast ++;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));
	// theme 1
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_AUTO_DARK));
	brightness ++;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast ++;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));
	// theme 0
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_AUTO_LIGHT));
	brightness ++;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast ++;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));

	// HDMI
	EXPECT_EQ(true, SetVideoSource(obj, lock, VIDEO_HAL_VSRC_ID_HDMI));
	// night
	EXPECT_EQ(true, SetDayNight(obj, lock, VIDEO_HAL_SETTING_NIGHT));
	brightness ++;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast ++;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));
	// day
	EXPECT_EQ(true, SetDayNight(obj, lock, VIDEO_HAL_SETTING_DAY));
	brightness ++;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast ++;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));

	// DTV
	EXPECT_EQ(true, SetVideoSource(obj, lock, VIDEO_HAL_VSRC_ID_DTV));
	// night
	EXPECT_EQ(true, SetDayNight(obj, lock, VIDEO_HAL_SETTING_NIGHT));
	brightness ++;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast ++;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));
	// day
	EXPECT_EQ(true, SetDayNight(obj, lock, VIDEO_HAL_SETTING_DAY));
	brightness ++;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast ++;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));

	// CAMERA
	EXPECT_EQ(true, SetVideoSource(obj, lock, VIDEO_HAL_VSRC_ID_IMAGE_ADJUST_CAM));
	// night
	EXPECT_EQ(true, SetDayNight(obj, lock, VIDEO_HAL_SETTING_NIGHT));
	brightness ++;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast ++;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));
	// day
	EXPECT_EQ(true, SetDayNight(obj, lock, VIDEO_HAL_SETTING_DAY));
	brightness ++;
	EXPECT_EQ(true, SetBrightness(obj, lock, brightness));
	contrast ++;
	EXPECT_EQ(true, SetContrast(obj, lock, contrast));

	// get (descending) ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Multisensory
	EXPECT_EQ(true, SetVideoSource(obj, lock, VIDEO_HAL_VSRC_ID_MULTISENSORY));
	// theme 3
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_SELECT_DARK));
	brightness = BRIGHTNESS_START;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(brightness, num); PrintMsg(std::to_string(num) + "\n");
	contrast = CONTRAST_START;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(contrast, num); PrintMsg(std::to_string(num) + "\n");
	// theme 2
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_SELECT_LIGHT));
	brightness ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(brightness, num); PrintMsg(std::to_string(num) + "\n");
	contrast ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(contrast, num); PrintMsg(std::to_string(num) + "\n");
	// theme 1
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_AUTO_DARK));
	brightness ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(brightness, num); PrintMsg(std::to_string(num) + "\n");
	contrast ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(contrast, num); PrintMsg(std::to_string(num) + "\n");
	// theme 0
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_AUTO_LIGHT));
	brightness ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(brightness, num); PrintMsg(std::to_string(num) + "\n");
	contrast ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(contrast, num); PrintMsg(std::to_string(num) + "\n");

	// HMI
	EXPECT_EQ(true, SetVideoSource(obj, lock, VIDEO_HAL_VSRC_ID_OTHER));
	// theme 3
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_SELECT_DARK));
	brightness = BRIGHTNESS_START;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(brightness, num); PrintMsg(std::to_string(num) + "\n");
	contrast = CONTRAST_START;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(contrast, num); PrintMsg(std::to_string(num) + "\n");
	// theme 2
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_SELECT_LIGHT));
	brightness ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(brightness, num); PrintMsg(std::to_string(num) + "\n");
	contrast ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(contrast, num); PrintMsg(std::to_string(num) + "\n");
	// theme 1
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_AUTO_DARK));
	brightness ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(brightness, num); PrintMsg(std::to_string(num) + "\n");
	contrast ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(contrast, num); PrintMsg(std::to_string(num) + "\n");
	// theme 0
	EXPECT_EQ(true, SetThemeColor(obj, lock, VIDEO_HAL_THEME_COLOR_AUTO_LIGHT));
	brightness ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(brightness, num); PrintMsg(std::to_string(num) + "\n");
	contrast ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(contrast, num); PrintMsg(std::to_string(num) + "\n");

	// HDMI
	EXPECT_EQ(true, SetVideoSource(obj, lock, VIDEO_HAL_VSRC_ID_HDMI));
	// night
	EXPECT_EQ(true, SetDayNight(obj, lock, VIDEO_HAL_SETTING_NIGHT));
	brightness ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(HDMI_BRIGHTNESS_NIGHT, num); PrintMsg(std::to_string(num) + "\n");
	contrast ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(HDMI_CONTRAST_NIGHT, num); PrintMsg(std::to_string(num) + "\n");
	// day
	EXPECT_EQ(true, SetDayNight(obj, lock, VIDEO_HAL_SETTING_DAY));
	brightness ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(HDMI_BRIGHTNESS_DAY, num); PrintMsg(std::to_string(num) + "\n");
	contrast ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(HDMI_CONTRAST_DAY, num); PrintMsg(std::to_string(num) + "\n");

	// DTV
	EXPECT_EQ(true, SetVideoSource(obj, lock, VIDEO_HAL_VSRC_ID_DTV));
	// night
	EXPECT_EQ(true, SetDayNight(obj, lock, VIDEO_HAL_SETTING_NIGHT));
	brightness ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(brightness, num); PrintMsg(std::to_string(num) + "\n");
	contrast ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(contrast, num); PrintMsg(std::to_string(num) + "\n");
	// day
	EXPECT_EQ(true, SetDayNight(obj, lock, VIDEO_HAL_SETTING_DAY));
	brightness ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(brightness, num); PrintMsg(std::to_string(num) + "\n");
	contrast ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(contrast, num); PrintMsg(std::to_string(num) + "\n");

	// CAMERA
	EXPECT_EQ(true, SetVideoSource(obj, lock, VIDEO_HAL_VSRC_ID_IMAGE_ADJUST_CAM));
	// night
	EXPECT_EQ(true, SetDayNight(obj, lock, VIDEO_HAL_SETTING_NIGHT));
	brightness ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(brightness, num); PrintMsg(std::to_string(num) + "\n");
	contrast ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(contrast, num); PrintMsg(std::to_string(num) + "\n");
	// day
	EXPECT_EQ(true, SetDayNight(obj, lock, VIDEO_HAL_SETTING_DAY));
	brightness ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.brightness.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(brightness, num); PrintMsg(std::to_string(num) + "\n");
	contrast ++;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.contrast.step", &num, sizeof(num), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(contrast, num); PrintMsg(std::to_string(num) + "\n");

	// finalize
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.id", CallbackVideoSourceId));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.day_night", CallbackDayNight));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.theme_color", CallbackThemeColor));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.brightness.step", CallbackBrightness_NoCheck));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.contrast.step", CallbackContrast_NoCheck));
	cockpit::hal::video_hal::Deinit(obj);
}
