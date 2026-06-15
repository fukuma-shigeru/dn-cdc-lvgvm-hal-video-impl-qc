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
static uint32_t status_daynight;
static uint32_t status_daynight_expect;
static uint32_t status_theme_color;
static uint32_t status_theme_color_expect;
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

static void CallbackDayNight(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.day_night", &status_daynight, sizeof(status_daynight), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_daynight_expect, status_daynight);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackThemeColor(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vhal.setting.status.theme_color", &status_theme_color, sizeof(status_theme_color), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_theme_color_expect, status_theme_color);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

/**
 * API仕様書記載のシーケンス
 * 画質調整(輝度):DTV
 * 映像パス		== DTV
 * 映像ソースID == DTV
 */
TEST(VhalSeqTestAdjustmentApiSpecification, SetBrightness_DTV)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(	obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(	obj, VIDEO_HAL_VSRC_ID_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetBrightness(		obj, DEFAULT_PROPERTY_BRIGHTNESS));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.brightness.step", CallbackBrightness));

	/* 明るさ設定 */
	status_brightness = 32;
	status_brightness_expect = 40;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj,
		"vsrc.front.control.brightness.step", &status_brightness_expect, sizeof(status_brightness_expect), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for brightness.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_brightness_expect == status_brightness); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.brightness.step", CallbackBrightness));

	/* 初期値に戻す */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetBrightness(		obj, DEFAULT_PROPERTY_BRIGHTNESS));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(	obj, DEFAULT_PROPERTY_VSRC_ID));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(	obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * API仕様書記載のシーケンス
 * 画質調整(コントラスト):DTV
 * 映像パス		== DTV
 * 映像ソースID == DTV
 */
TEST(VhalSeqTestAdjustmentApiSpecification, SetContrast_DTV)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(	obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(	obj, VIDEO_HAL_VSRC_ID_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetContrast(		obj, DEFAULT_PROPERTY_CONTRAST));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.contrast.step", CallbackContrast));

	/* 明るさ設定 */
	status_contrast = 32;
	status_contrast_expect = 40;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj,
		"vsrc.front.control.contrast.step", &status_contrast_expect, sizeof(status_contrast_expect), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for contrast.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_contrast_expect == status_contrast); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.contrast.step", CallbackContrast));

	/* 初期値に戻す */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetContrast(		obj, DEFAULT_PROPERTY_CONTRAST));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(	obj, DEFAULT_PROPERTY_VSRC_ID));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(	obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * API仕様書記載のシーケンス
 * 画質調整(輝度):操作画面
 * 映像パス		== CARPLAY
 * 映像ソースID == OTHER
 */
TEST(VhalSeqTestAdjustmentApiSpecification, SetBrightness_CARPLAY)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_CARPLAY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(	obj, VPATH_NAME_CARPLAY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(	obj, VIDEO_HAL_VSRC_ID_OTHER));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetBrightness(		obj, DEFAULT_PROPERTY_BRIGHTNESS));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.brightness.step", CallbackBrightness));

	/* 明るさ設定 */
	status_brightness = 32;
	status_brightness_expect = 40;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj,
		"vsrc.front.control.brightness.step", &status_brightness_expect, sizeof(status_brightness_expect), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for brightness.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_brightness_expect == status_brightness); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.brightness.step", CallbackBrightness));

	/* 初期値に戻す */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetBrightness(		obj, DEFAULT_PROPERTY_BRIGHTNESS));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(	obj, DEFAULT_PROPERTY_VSRC_ID));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(	obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * API仕様書記載のシーケンス
 * 画質調整(コントラスト):操作画面
 * 映像パス		== CARPLAY
 * 映像ソースID == OTHER
 */
TEST(VhalSeqTestAdjustmentApiSpecification, SetContrast_CARPLAY)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_CARPLAY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(	obj, VPATH_NAME_CARPLAY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(	obj, VIDEO_HAL_VSRC_ID_OTHER));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetContrast(		obj, DEFAULT_PROPERTY_CONTRAST));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.contrast.step", CallbackContrast));

	/* 明るさ設定 */
	status_contrast = 32;
	status_contrast_expect = 40;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj,
		"vsrc.front.control.contrast.step", &status_contrast_expect, sizeof(status_contrast_expect), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for contrast.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_contrast_expect == status_contrast); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.contrast.step", CallbackContrast));

	/* 初期値に戻す */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetContrast(		obj, DEFAULT_PROPERTY_CONTRAST));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(	obj, DEFAULT_PROPERTY_VSRC_ID));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(	obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 画質調整(輝度):[画質調整設定(更新対象映像)] ⇔ [表示中映像(TAB2小窓用送信対象) が不一致case
 * 映像パス		== OTHER
 * 映像ソースID == DTV
 */
TEST(VhalSeqTestAdjustment_NotEqual_Vpath2Sourceid, SetBrightness)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_CARPLAY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(	obj, VPATH_NAME_CARPLAY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(	obj, VIDEO_HAL_VSRC_ID_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetBrightness(		obj, DEFAULT_PROPERTY_BRIGHTNESS));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.brightness.step", CallbackBrightness));

	/* 明るさ設定 */
	status_brightness = 1;
	status_brightness_expect = 50;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj,
		"vsrc.front.control.brightness.step", &status_brightness_expect, sizeof(status_brightness_expect), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for brightness.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_brightness_expect == status_brightness); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.brightness.step", CallbackBrightness));

	/* 初期値に戻す */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetBrightness(		obj, DEFAULT_PROPERTY_BRIGHTNESS));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(	obj, DEFAULT_PROPERTY_VSRC_ID));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(	obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 画質調整(コントラスト):[画質調整設定(更新対象映像)] ⇔ [表示中映像(TAB2小窓用送信対象) が不一致case
 * 映像パス		== OTHER
 * 映像ソースID == DTV
 */
TEST(VhalSeqTestAdjustment_NotEqual_Vpath2Sourceid, SetContrast)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_CARPLAY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(	obj, VPATH_NAME_CARPLAY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(	obj, VIDEO_HAL_VSRC_ID_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetContrast(		obj, DEFAULT_PROPERTY_CONTRAST));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.contrast.step", CallbackContrast));

	/* 明るさ設定 */
	status_contrast = 1;
	status_contrast_expect = 55;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj,
		"vsrc.front.control.contrast.step", &status_contrast_expect, sizeof(status_contrast_expect), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for contrast.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_contrast_expect == status_contrast); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.contrast.step", CallbackContrast));

	/* 初期値に戻す */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetContrast(		obj, DEFAULT_PROPERTY_CONTRAST));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(	obj, DEFAULT_PROPERTY_VSRC_ID));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(	obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * API仕様書記載のシーケンス
 * 強制HMI画質設定
 */
TEST(VhalSeqTestForcedHmiImgAdjApiSpecification, SetForcedHmiImgAdj)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
    bool current_forced_hmi_img_adj{};
	bool control_forced_hmi_img_adj{true};

	cockpit::hal::video_hal::Init(obj);

	/* DTV映像 */
	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.forced_hmi_img_adj", CallbackForcedHmiImgAdj));

	/* 強制HMI画質設定 */
	status_forced_hmi_img_adj			= OTHER_BOOL_VALUE(control_forced_hmi_img_adj);
	status_forced_hmi_img_adj_expect	= control_forced_hmi_img_adj;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj,
		"vsrc.front.control.forced_hmi_img_adj", &status_forced_hmi_img_adj_expect, sizeof(status_forced_hmi_img_adj_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for forced_hmi_img_adj.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_forced_hmi_img_adj_expect == status_forced_hmi_img_adj); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj,
		"vsrc.front.status.forced_hmi_img_adj", &current_forced_hmi_img_adj, sizeof(current_forced_hmi_img_adj), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(control_forced_hmi_img_adj, current_forced_hmi_img_adj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.forced_hmi_img_adj", CallbackForcedHmiImgAdj));

	/* 初期値に戻す */
	/* 設定値反転 */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetForcedHmiImgAdj(obj, OTHER_BOOL_VALUE(control_forced_hmi_img_adj)));
	/* DTV映像 */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * API仕様書記載のシーケンス
 * 強制多感覚連携画質設定
 */
TEST(VhalSeqTestForcedMultisensoryImgAdjApiSpecification, SetForcedMultisensoryImgAdj)
{
	/* initialize */
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
    bool current_forced_multisensory_img_adj{};
	bool control_forced_multisensory_img_adj{true};
	cockpit::hal::video_hal::Init(obj);

	/* 多感覚連携 映像 */
	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_MULTISENSORY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_MULTISENSORY));

	/* 強制多感覚連携画質設定 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.forced_multisensory_img_adj", CallbackForcedMultisensoryImgAdj));
	status_forced_multisensory_img_adj = OTHER_BOOL_VALUE(control_forced_multisensory_img_adj);
	status_forced_multisensory_img_adj_expect = control_forced_multisensory_img_adj;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.forced_multisensory_img_adj", &status_forced_multisensory_img_adj_expect, sizeof(status_forced_multisensory_img_adj_expect), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for forced_multisensory_img_adj.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_forced_multisensory_img_adj_expect == status_forced_multisensory_img_adj); });
	EXPECT_EQ(true, intime);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.forced_multisensory_img_adj", &current_forced_multisensory_img_adj, sizeof(current_forced_multisensory_img_adj), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(control_forced_multisensory_img_adj, current_forced_multisensory_img_adj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.forced_multisensory_img_adj", CallbackForcedMultisensoryImgAdj));

	/* 既定値に戻す */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetForcedMultisensoryImgAdj(obj, OTHER_BOOL_VALUE(control_forced_multisensory_img_adj)));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	/* finalize */
	KillAllRenderImage(obj);
	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * API仕様書記載のシーケンス
 * 画質調整(昼夜切替)
 * 映像パス		== DTV
 * 映像ソースID == DTV ※映像パスに紐づく(camera_path_type_table_)
 */
TEST(VhalSeqTestAdjustmentApiSpecification, SetDayNight)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(	obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetDayNight(		obj, DEFAULT_PROPERTY_DAY_NIGHT));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.day_night", CallbackDayNight));

	/* 昼設定 */
	status_daynight			= VIDEO_HAL_SETTING_FORCED_DAY;
	status_daynight_expect	= VIDEO_HAL_SETTING_DAY;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj,
		"vhal.setting.control.day_night", &status_daynight_expect, sizeof(status_daynight_expect), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	PrintMsg("wait_for setting day.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_daynight_expect == status_daynight); });
	EXPECT_EQ(true, intime);	

	/* 夜設定 */
	status_daynight			= VIDEO_HAL_SETTING_FORCED_DAY;
	status_daynight_expect	= VIDEO_HAL_SETTING_NIGHT;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj,
		"vhal.setting.control.day_night", &status_daynight_expect, sizeof(status_daynight_expect), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	PrintMsg("wait_for setting night.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_daynight_expect == status_daynight); });
	EXPECT_EQ(true, intime);	

	/* 強制昼設定 */
	status_daynight			= VIDEO_HAL_SETTING_DAY;
	status_daynight_expect	= VIDEO_HAL_SETTING_FORCED_DAY;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj,
		"vhal.setting.control.day_night", &status_daynight_expect, sizeof(status_daynight_expect), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	PrintMsg("wait_for forced day.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_daynight_expect == status_daynight); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.day_night", CallbackDayNight));

	/* 元に戻す */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetDayNight(	 	obj, DEFAULT_PROPERTY_DAY_NIGHT));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(	obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * API仕様書記載のシーケンス
 * 画質調整(テーマ切替)
 * 映像パス		== CARPLAY
 * 映像ソースID == OTHER ※映像パスに紐づく(camera_path_type_table_)
 */
TEST(VhalSeqTestAdjustmentApiSpecification, SetThemeColor)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_CARPLAY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(	obj, VPATH_NAME_CARPLAY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetThemeColor(		obj, DEFAULT_PROPERTY_THEME_COLOR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.theme_color", CallbackThemeColor));

	/* 自動設定(Light color) */
	status_theme_color			= VIDEO_HAL_THEME_COLOR_FORCED_LIGHT;
	status_theme_color_expect	= VIDEO_HAL_THEME_COLOR_AUTO_LIGHT;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj,
		"vhal.setting.control.theme_color", &status_theme_color_expect, sizeof(status_theme_color_expect), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	PrintMsg("wait_for auto light.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_theme_color_expect == status_theme_color); });
	EXPECT_EQ(true, intime);	

	/* 自動設定(Dark color) */
	status_theme_color 			= VIDEO_HAL_THEME_COLOR_FORCED_LIGHT;
	status_theme_color_expect	= VIDEO_HAL_THEME_COLOR_AUTO_DARK;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj,
		"vhal.setting.control.theme_color", &status_theme_color_expect, sizeof(status_theme_color_expect), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	PrintMsg("wait_for auto dark.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_theme_color_expect == status_theme_color); });
	EXPECT_EQ(true, intime);	

	/* 手動設定(Light color) */
	status_theme_color			= VIDEO_HAL_THEME_COLOR_FORCED_LIGHT;
	status_theme_color_expect	= VIDEO_HAL_THEME_COLOR_SELECT_LIGHT;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj,
		"vhal.setting.control.theme_color", &status_theme_color_expect, sizeof(status_theme_color_expect), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	PrintMsg("wait_for select light.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_theme_color_expect == status_theme_color); });
	EXPECT_EQ(true, intime);	

	/* 手動設定(Dark color) */
	status_theme_color			= VIDEO_HAL_THEME_COLOR_FORCED_LIGHT;
	status_theme_color_expect	= VIDEO_HAL_THEME_COLOR_SELECT_DARK;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj,
		"vhal.setting.control.theme_color", &status_theme_color_expect, sizeof(status_theme_color_expect), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	PrintMsg("wait_for select dark.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_theme_color_expect == status_theme_color); });
	EXPECT_EQ(true, intime);	

	/* 手動設定(強制Light color) */
	status_theme_color			= VIDEO_HAL_THEME_COLOR_AUTO_LIGHT;
	status_theme_color_expect	= VIDEO_HAL_THEME_COLOR_FORCED_LIGHT;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj,
		"vhal.setting.control.theme_color", &status_theme_color_expect, sizeof(status_theme_color_expect), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	PrintMsg("wait_for forced light.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_theme_color_expect == status_theme_color); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.theme_color", CallbackThemeColor));

	/* 元に戻す */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetThemeColor(		obj, DEFAULT_PROPERTY_THEME_COLOR));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 画質調整(昼夜切替):[画質調整設定(更新対象映像)] ⇔ [表示中映像(TAB2小窓用送信対象) が不一致case
 * 映像パス		== CARPLAY
 * 映像ソースID	== OTHER ※映像パスに紐づく(camera_path_type_table_)
 */
TEST(VhalSeqTestAdjustment_NotEqual_Vpath2Sourceid, SetDayNight)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(	obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetDayNight(		obj, DEFAULT_PROPERTY_DAY_NIGHT));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_CARPLAY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(	obj, VPATH_NAME_CARPLAY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(	obj, VIDEO_HAL_VSRC_ID_DTV));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.day_night", CallbackDayNight));

	/* 昼設定 */
	status_daynight			= VIDEO_HAL_SETTING_FORCED_DAY;
	status_daynight_expect	= VIDEO_HAL_SETTING_DAY;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj,
		"vhal.setting.control.day_night", &status_daynight_expect, sizeof(status_daynight_expect), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	PrintMsg("wait_for setting day.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_daynight_expect == status_daynight); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.day_night", CallbackDayNight));

	/* 元に戻す */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetDayNight(		obj, DEFAULT_PROPERTY_DAY_NIGHT));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(	obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 画質調整(テーマ切替):[画質調整設定(更新対象映像)] ⇔ [表示中映像(TAB2小窓用送信対象) が不一致case
 * 映像パス		== DTV
 * 映像ソースID	== DTV ※映像パスに紐づく(camera_path_type_table_)
 */
TEST(VhalSeqTestAdjustment_NotEqual_Vpath2Sourceid, SetThemeColor)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_CARPLAY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(	obj, VPATH_NAME_CARPLAY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetThemeColor(		obj, DEFAULT_PROPERTY_THEME_COLOR));
	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(	obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(	obj, VIDEO_HAL_VSRC_ID_HDMI));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vhal.setting.status.theme_color", CallbackThemeColor));

	/* 自動設定(Light color) */
	status_theme_color			= VIDEO_HAL_THEME_COLOR_FORCED_LIGHT;
	status_theme_color_expect	= VIDEO_HAL_THEME_COLOR_AUTO_LIGHT;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj,
		"vhal.setting.control.theme_color", &status_theme_color_expect, sizeof(status_theme_color_expect), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	PrintMsg("wait_for auto light.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_theme_color_expect == status_theme_color); });
	EXPECT_EQ(true, intime);	

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vhal.setting.status.theme_color", CallbackThemeColor));

	/* 元に戻す */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetThemeColor(		obj, DEFAULT_PROPERTY_THEME_COLOR));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * API仕様書記載のシーケンス
 * 強制HMI画質設定（カメラ）
 */
TEST(VhalSeqTestForcedHmiImgAdjApiSpecificationCamera, SetForcedHmiImgAdj)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
    bool current_forced_hmi_img_adj{};
	bool control_forced_hmi_img_adj{true};

	cockpit::hal::video_hal::Init(obj);

	/* カメラ映像 */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CAMERA));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetCameraOutput(obj, 1, 1, 1, 1, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.forced_hmi_img_adj", CallbackForcedHmiImgAdj));

	/* 強制HMI画質設定 */
	status_forced_hmi_img_adj = OTHER_BOOL_VALUE(control_forced_hmi_img_adj);
	status_forced_hmi_img_adj_expect = control_forced_hmi_img_adj;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.forced_hmi_img_adj", &control_forced_hmi_img_adj, sizeof(control_forced_hmi_img_adj), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for forced_hmi_img_adj.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_forced_hmi_img_adj_expect == status_forced_hmi_img_adj); });
	EXPECT_EQ(true, intime);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.forced_hmi_img_adj", &current_forced_hmi_img_adj, sizeof(current_forced_hmi_img_adj), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(control_forced_hmi_img_adj, current_forced_hmi_img_adj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.forced_hmi_img_adj", CallbackForcedHmiImgAdj));

	/* 設定値反転 */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetForcedHmiImgAdj(obj, OTHER_BOOL_VALUE(control_forced_hmi_img_adj)));
	/* カメラ映像 */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 * 画質調整(輝度):DTV
 * 映像パス		== DTV
 * 映像ソースID == DTV
 */
TEST(VhalSeqTestAdjustmentRepeat, SetBrightness_DTV)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, VIDEO_HAL_VSRC_ID_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetBrightness(obj, DEFAULT_PROPERTY_BRIGHTNESS));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.brightness.step", CallbackBrightness));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		status_brightness = 32;
		status_brightness_expect = 40;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.brightness.step", &status_brightness_expect, sizeof(status_brightness_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for brightness.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_brightness_expect == status_brightness); });
		EXPECT_EQ(true, intime);

		/* もとに戻す */
		status_brightness = 40;
		status_brightness_expect = 32;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.brightness.step", &status_brightness_expect, sizeof(status_brightness_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for brightness.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_brightness_expect == status_brightness); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.brightness.step", CallbackBrightness));

	/* 初期値に戻す */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetBrightness(obj, DEFAULT_PROPERTY_BRIGHTNESS));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, DEFAULT_PROPERTY_VSRC_ID));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

TEST(VhalSeqTestAdjustmentRepeat, SetBrightnessCamera)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CAMERA));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, VIDEO_HAL_VSRC_ID_IMAGE_ADJUST_CAM));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetBrightness(obj, DEFAULT_PROPERTY_BRIGHTNESS));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.brightness.step", CallbackBrightness));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		status_brightness = 32;
		status_brightness_expect = 40;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.brightness.step", &status_brightness_expect, sizeof(status_brightness_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for brightness.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_brightness_expect == status_brightness); });
		EXPECT_EQ(true, intime);

		/* もとに戻す */
		status_brightness = 40;
		status_brightness_expect = 32;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.brightness.step", &status_brightness_expect, sizeof(status_brightness_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for brightness.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_brightness_expect == status_brightness); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.brightness.step", CallbackBrightness));

	/* 初期値に戻す */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetBrightness(obj, DEFAULT_PROPERTY_BRIGHTNESS));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, DEFAULT_PROPERTY_VSRC_ID));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 * 画質調整(コントラスト):DTV
 * 映像パス		== DTV
 * 映像ソースID == DTV
 */
TEST(VhalSeqTestAdjustmentRepeatContrast, SetContrast_DTV)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, VIDEO_HAL_VSRC_ID_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetContrast(obj, DEFAULT_PROPERTY_CONTRAST));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.contrast.step", CallbackContrast));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		status_contrast = 32;
		status_contrast_expect = 40;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj,
			"vsrc.front.control.contrast.step", &status_contrast_expect, sizeof(status_contrast_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for contrast.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_contrast_expect == status_contrast); });
		EXPECT_EQ(true, intime);

		/* もとに戻す */
		status_contrast = 40;
		status_contrast_expect = 32;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj,
			"vsrc.front.control.contrast.step", &status_contrast_expect, sizeof(status_contrast_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for contrast.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_contrast_expect == status_contrast); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.contrast.step", CallbackContrast));

	/* 初期値に戻す */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetContrast(		obj, DEFAULT_PROPERTY_CONTRAST));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(	obj, DEFAULT_PROPERTY_VSRC_ID));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(	obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

TEST(VhalSeqTestAdjustmentRepeat, SetContrastCamera)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CAMERA));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, VIDEO_HAL_VSRC_ID_IMAGE_ADJUST_CAM));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetContrast(obj, DEFAULT_PROPERTY_CONTRAST));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.contrast.step", CallbackContrast));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		status_contrast = 32;
		status_contrast_expect = 40;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj,
			"vsrc.front.control.contrast.step", &status_contrast_expect, sizeof(status_contrast_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for contrast.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_contrast_expect == status_contrast); });
		EXPECT_EQ(true, intime);

		/* もとに戻す */
		status_contrast = 40;
		status_contrast_expect = 32;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj,
			"vsrc.front.control.contrast.step", &status_contrast_expect, sizeof(status_contrast_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for contrast.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_contrast_expect == status_contrast); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.contrast.step", CallbackContrast));

	/* 初期値に戻す */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetContrast(		obj, DEFAULT_PROPERTY_CONTRAST));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(	obj, DEFAULT_PROPERTY_VSRC_ID));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(	obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 * 画質調整(輝度):CARPLAY
 * 映像パス		== CARPLAY
 * 映像ソースID == OTHER
 */
TEST(VhalSeqTestAdjustmentRepeatBrightness, SetBrightness_CARPLAY)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_CARPLAY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(	obj, VPATH_NAME_CARPLAY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(	obj, VIDEO_HAL_VSRC_ID_OTHER));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetBrightness(		obj, DEFAULT_PROPERTY_BRIGHTNESS));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.brightness.step", CallbackBrightness));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		status_brightness = 32;
		status_brightness_expect = 40;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj,
			"vsrc.front.control.brightness.step", &status_brightness_expect, sizeof(status_brightness_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for brightness.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_brightness_expect == status_brightness); });
		EXPECT_EQ(true, intime);

		/* もとに戻す */
		status_brightness = 40;
		status_brightness_expect = 32;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, 
			"vsrc.front.control.brightness.step", &status_brightness_expect, sizeof(status_brightness_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for brightness.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_brightness_expect == status_brightness); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.brightness.step", CallbackBrightness));

	/* 初期値に戻す */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetBrightness(		obj, DEFAULT_PROPERTY_BRIGHTNESS));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(	obj, DEFAULT_PROPERTY_VSRC_ID));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(	obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 * 画質調整(コントラスト):CARPLAY
 * 映像パス		== CARPLAY
 * 映像ソースID == OTHER
 */
TEST(VhalSeqTestAdjustmentRepeatContrast, SetContrast_CARPLAY)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_CARPLAY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CARPLAY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(obj, VIDEO_HAL_VSRC_ID_OTHER));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetContrast(obj, DEFAULT_PROPERTY_CONTRAST));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.contrast.step", CallbackContrast));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		status_contrast = 32;
		status_contrast_expect = 40;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj,
			"vsrc.front.control.contrast.step", &status_contrast_expect, sizeof(status_contrast_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for contrast.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_contrast_expect == status_contrast); });
		EXPECT_EQ(true, intime);

		/* もとに戻す */
		status_contrast = 40;
		status_contrast_expect = 32;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj,
			"vsrc.front.control.contrast.step", &status_contrast_expect, sizeof(status_contrast_expect), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for contrast.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_contrast_expect == status_contrast); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.contrast.step", CallbackContrast));

	/* 初期値に戻す */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetContrast(		obj, DEFAULT_PROPERTY_CONTRAST));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetVideoSourceId(	obj, DEFAULT_PROPERTY_VSRC_ID));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(	obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 * 強制HMI画質設定
 */
TEST(VhalSeqTestForcedHmiImgAdjApiSpecificationRepeat, SetForcedHmiImgAdj)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
    bool current_forced_hmi_img_adj{};
	bool control_forced_hmi_img_adj{true};

	cockpit::hal::video_hal::Init(obj);

	/* DTV映像 */
	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_DTV));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.forced_hmi_img_adj", CallbackForcedHmiImgAdj));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		/* 強制HMI画質設定 */
		status_forced_hmi_img_adj = OTHER_BOOL_VALUE(control_forced_hmi_img_adj);
		status_forced_hmi_img_adj_expect = control_forced_hmi_img_adj;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.forced_hmi_img_adj", &control_forced_hmi_img_adj, sizeof(control_forced_hmi_img_adj), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for forced_hmi_img_adj.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_forced_hmi_img_adj_expect == status_forced_hmi_img_adj); });
		EXPECT_EQ(true, intime);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.forced_hmi_img_adj", &current_forced_hmi_img_adj, sizeof(current_forced_hmi_img_adj), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(control_forced_hmi_img_adj, current_forced_hmi_img_adj);

		/* もとに戻す */
		status_forced_hmi_img_adj = control_forced_hmi_img_adj;
		status_forced_hmi_img_adj_expect = OTHER_BOOL_VALUE(control_forced_hmi_img_adj);

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.forced_hmi_img_adj", &status_forced_hmi_img_adj_expect, sizeof(status_forced_hmi_img_adj_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for forced_hmi_img_adj.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_forced_hmi_img_adj_expect == status_forced_hmi_img_adj); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.forced_hmi_img_adj", CallbackForcedHmiImgAdj));

	/* 設定値反転 */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetForcedHmiImgAdj(obj, OTHER_BOOL_VALUE(control_forced_hmi_img_adj)));
	/* DTV映像 */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));
	KillAllRenderImage(obj);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 * 強制多感覚連携画質設定
 */
TEST(VhalSeqTestForcedMultisensoryImgAdjApiSpecificationRepeat, SetForcedMultisensoryImgAdj)
{
	/* initialize */
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
    bool current_forced_multisensory_img_adj{};
	bool control_forced_multisensory_img_adj{true};
	cockpit::hal::video_hal::Init(obj);

	/* 多感覚連携 映像 */
	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_MULTISENSORY));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_MULTISENSORY));

	/* 強制多感覚連携画質設定 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.forced_multisensory_img_adj", CallbackForcedMultisensoryImgAdj));
	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		status_forced_multisensory_img_adj = OTHER_BOOL_VALUE(control_forced_multisensory_img_adj);
		status_forced_multisensory_img_adj_expect = control_forced_multisensory_img_adj;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.forced_multisensory_img_adj", &control_forced_multisensory_img_adj, sizeof(control_forced_multisensory_img_adj), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for forced_multisensory_img_adj.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_forced_multisensory_img_adj_expect == status_forced_multisensory_img_adj); });
		EXPECT_EQ(true, intime);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.forced_multisensory_img_adj", &current_forced_multisensory_img_adj, sizeof(current_forced_multisensory_img_adj), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(control_forced_multisensory_img_adj, current_forced_multisensory_img_adj);
		/* もとに戻す */
		status_forced_multisensory_img_adj = control_forced_multisensory_img_adj;
		status_forced_multisensory_img_adj_expect = OTHER_BOOL_VALUE(control_forced_multisensory_img_adj);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.forced_multisensory_img_adj", &status_forced_multisensory_img_adj_expect, sizeof(status_forced_multisensory_img_adj_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for forced_multisensory_img_adj.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_forced_multisensory_img_adj_expect == status_forced_multisensory_img_adj); });
 		EXPECT_EQ(true, intime);
	}
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.forced_multisensory_img_adj", CallbackForcedMultisensoryImgAdj));

	/* 既定値に戻す */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetForcedMultisensoryImgAdj(obj, OTHER_BOOL_VALUE(control_forced_multisensory_img_adj)));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathFront(obj, VPATH_NAME_CLEAR));

	/* finalize */
	KillAllRenderImage(obj);
	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 * 強制HMI画質設定（カメラ）
 */
TEST(VhalSeqTestForcedHmiImgAdjApiSpecificationCameraRepeat, SetForcedHmiImgAdj)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
    bool current_forced_hmi_img_adj{};
	bool control_forced_hmi_img_adj{true};

	cockpit::hal::video_hal::Init(obj);

	/* カメラ映像 */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CAMERA));
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetCameraOutput(obj, 1, 1, 1, 1, true));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.forced_hmi_img_adj", CallbackForcedHmiImgAdj));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		/* 強制HMI画質設定 */
		status_forced_hmi_img_adj = OTHER_BOOL_VALUE(control_forced_hmi_img_adj);
		status_forced_hmi_img_adj_expect = control_forced_hmi_img_adj;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.forced_hmi_img_adj", &control_forced_hmi_img_adj, sizeof(control_forced_hmi_img_adj), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for forced_hmi_img_adj.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_forced_hmi_img_adj_expect == status_forced_hmi_img_adj); });
		EXPECT_EQ(true, intime);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.forced_hmi_img_adj", &current_forced_hmi_img_adj, sizeof(current_forced_hmi_img_adj), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(control_forced_hmi_img_adj, current_forced_hmi_img_adj);

		/* もとに戻す */
		status_forced_hmi_img_adj = control_forced_hmi_img_adj;
		status_forced_hmi_img_adj_expect = OTHER_BOOL_VALUE(control_forced_hmi_img_adj);

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.forced_hmi_img_adj", &status_forced_hmi_img_adj_expect, sizeof(status_forced_hmi_img_adj_expect), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for forced_hmi_img_adj.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_forced_hmi_img_adj_expect == status_forced_hmi_img_adj); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.forced_hmi_img_adj", CallbackForcedHmiImgAdj));

	/* 設定値反転 */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SetForcedHmiImgAdj(obj, OTHER_BOOL_VALUE(control_forced_hmi_img_adj)));
	/* カメラ映像 */
	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathCamera(obj, VPATH_NAME_CLEAR));

	cockpit::hal::video_hal::Deinit(obj);
}
