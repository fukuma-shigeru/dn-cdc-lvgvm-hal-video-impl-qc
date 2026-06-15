#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};

struct VhalSeqTestIviPropertyParameter {
	int32_t surface_id;
	std::string front_vpath_name;
};

struct VhalSeqTestIviPropertyIdParameter {
	int32_t ivi_id;
};

struct VhalSeqTestIviPropertyNameParameter {
	std::string name;
};

class VhalSeqTestIviPropertyApiSpecification : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestIviPropertyParameter> {};
class VhalSepTestIviPropertyNormalScreenIviId : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestIviPropertyIdParameter> {};
class VhalSeqTestIviPropertyNormalScreenName : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestIviPropertyNameParameter> {};
class VhalSeqTestIviPropertyNormalLayerIviId : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestIviPropertyIdParameter> {};
class VhalSeqTestIviPropertyNormalLayerName : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestIviPropertyNameParameter> {};

class VhalSeqTestIviPropertyNormalSurfaceIviId : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestIviPropertyIdParameter> {};
class VhalSeqTestIviPropertyNormalSurfaceName : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestIviPropertyNameParameter> {};


INSTANTIATE_TEST_CASE_P(VhalSeqTestIviProperty, VhalSeqTestIviPropertyApiSpecification, ::testing::Values(
	VhalSeqTestIviPropertyParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV}
));

/**
 *screen
 */
INSTANTIATE_TEST_CASE_P(VhalSeqTestIviProperty, VhalSepTestIviPropertyNormalScreenIviId, ::testing::Values(
	VhalSeqTestIviPropertyIdParameter{SCREEN_ID_FRONT}
));

INSTANTIATE_TEST_CASE_P(VhalSeqTestIviProperty, VhalSeqTestIviPropertyNormalScreenName, ::testing::Values(
	VhalSeqTestIviPropertyNameParameter{SCREEN_NAME_FRONT}
));

/**
 *layer
 */
INSTANTIATE_TEST_CASE_P(VhalSeqTestIviProperty, VhalSeqTestIviPropertyNormalLayerIviId, ::testing::Values(
	VhalSeqTestIviPropertyIdParameter{LAYER_ID_VIDEO}
));

INSTANTIATE_TEST_CASE_P(VhalSeqTestIviProperty, VhalSeqTestIviPropertyNormalLayerName, ::testing::Values(
	VhalSeqTestIviPropertyNameParameter{LAYER_NAME_VIDEO}
));

/**
 *surface
 */
INSTANTIATE_TEST_CASE_P(VhalSeqTestIviProperty, VhalSeqTestIviPropertyNormalSurfaceIviId, ::testing::Values(
	VhalSeqTestIviPropertyIdParameter{SURFACE_ID_FRONT_DTV}
));

INSTANTIATE_TEST_CASE_P(VhalSeqTestIviProperty, VhalSeqTestIviPropertyNormalSurfaceName, ::testing::Values(
	VhalSeqTestIviPropertyNameParameter{SURFACE_NAME_FRONT_DTV}
));


/**
 * API仕様書記載のシーケンス
 * iviプロパティ取得
 */
TEST_P(VhalSeqTestIviPropertyApiSpecification, GetIviProperty)
{
	VhalSeqTestIviPropertyParameter param = GetParam();
	std::string ivi_property{};

	cockpit::hal::video_hal::Init(obj);

	/* iviプロパティ取得 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "ivi.layer.status.properties?ivi_id=260", &ivi_property, VIDEO_HAL_IVI_PROP_JSON_MAX, VIDEO_HAL_DATA_TYPE_STR));

	PrintMsg("ivi_property=[" + ivi_property + "]");

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 * iviプロパティ取得
 */
TEST(VhalSeqTestIviPropertyRepeat, GetIviProperty)
{
	std::string ivi_property{};

	cockpit::hal::video_hal::Init(obj);

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		/* iviプロパティ取得 */
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "ivi.layer.status.properties?ivi_id=260", &ivi_property, VIDEO_HAL_IVI_PROP_JSON_MAX, VIDEO_HAL_DATA_TYPE_STR));
	}


	cockpit::hal::video_hal::Deinit(obj);
}



/**
 * 基本動作
 * スクリーンのiviプロパティデータ取得(ivi_id指定)
 */
TEST_P(VhalSepTestIviPropertyNormalScreenIviId, GetIviProperty)
{
	VhalSeqTestIviPropertyIdParameter param = GetParam();
	std::string ivi_property{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "ivi.screen.status.properties?ivi_id=" + std::to_string(param.ivi_id), &ivi_property, VIDEO_HAL_IVI_PROP_JSON_MAX, VIDEO_HAL_DATA_TYPE_STR));

	PrintMsg("ivi_property=[" + ivi_property + "]");

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * スクリーンのiviプロパティデータ取得(name指定)
 */
TEST_P(VhalSeqTestIviPropertyNormalScreenName, GetIviProperty)
{
	VhalSeqTestIviPropertyNameParameter param = GetParam();
	std::string ivi_property{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "ivi.screen.status.properties?name=" + param.name, &ivi_property, VIDEO_HAL_IVI_PROP_JSON_MAX, VIDEO_HAL_DATA_TYPE_STR));

	PrintMsg("ivi_property=[" + ivi_property + "]");

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * レイヤのiviプロパティデータ取得(ivi_id指定)
 */
TEST_P(VhalSeqTestIviPropertyNormalLayerIviId, GetIviProperty)
{
	VhalSeqTestIviPropertyIdParameter param = GetParam();
	std::string ivi_property{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "ivi.layer.status.properties?ivi_id=" + std::to_string(param.ivi_id), &ivi_property, VIDEO_HAL_IVI_PROP_JSON_MAX, VIDEO_HAL_DATA_TYPE_STR));

	PrintMsg("ivi_property=[" + ivi_property + "]");

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * レイヤのiviプロパティデータ取得(name指定)
 */
TEST_P(VhalSeqTestIviPropertyNormalLayerName, GetIviProperty)
{
	VhalSeqTestIviPropertyNameParameter param = GetParam();
	std::string ivi_property{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "ivi.layer.status.properties?name=" + param.name, &ivi_property, VIDEO_HAL_IVI_PROP_JSON_MAX, VIDEO_HAL_DATA_TYPE_STR));

	PrintMsg("ivi_property=[" + ivi_property + "]");

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * サーフェスのiviプロパティデータ取得(ivi_id指定)
 */
TEST_P(VhalSeqTestIviPropertyNormalSurfaceIviId, GetIviProperty)
{
	VhalSeqTestIviPropertyIdParameter param = GetParam();
	std::string ivi_property{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, param.ivi_id));	// BEVstep3 add

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "ivi.surface.status.properties?ivi_id=" + std::to_string(param.ivi_id), &ivi_property, VIDEO_HAL_IVI_PROP_JSON_MAX, VIDEO_HAL_DATA_TYPE_STR));

	PrintMsg("ivi_property=[" + ivi_property + "]");

	KillAllRenderImage(obj);	// BEVstep3 add

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * サーフェスのiviプロパティデータ取得(name指定)
 */
TEST_P(VhalSeqTestIviPropertyNormalSurfaceName, GetIviProperty)
{
	VhalSeqTestIviPropertyNameParameter param = GetParam();
	std::string ivi_property{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV));	// BEVstep3 add

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "ivi.surface.status.properties?name=" + param.name, &ivi_property, VIDEO_HAL_IVI_PROP_JSON_MAX, VIDEO_HAL_DATA_TYPE_STR));

	PrintMsg("ivi_property=[" + ivi_property + "]");

	KillAllRenderImage(obj);	// BEVstep3 add

	cockpit::hal::video_hal::Deinit(obj);
}
