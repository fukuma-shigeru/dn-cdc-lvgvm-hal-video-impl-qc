#include "vhal_test_common.h"


static cockpit::hal::video_hal::CtlObj obj {0};

struct VhalApiTestIviPropertyIdParameter {
	int32_t ivi_id;
};

struct VhalApiTestIviPropertyNameParameter {
	std::string name;
};

class VhalApiTestIviPropertyNormalScreenIviId : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestIviPropertyIdParameter> {};
class VhalApiTestIviPropertyNormalScreenName : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestIviPropertyNameParameter> {};
class VhalApiTestIviPropertyNormalLayerIviId : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestIviPropertyIdParameter> {};
class VhalApiTestIviPropertyNormalLayerName : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestIviPropertyNameParameter> {};
class VhalApiTestIviPropertyNormalSurfaceIviId : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestIviPropertyIdParameter> {};
class VhalApiTestIviPropertyNormalSurfaceName : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestIviPropertyNameParameter> {};

INSTANTIATE_TEST_CASE_P(VhalApiTestIviProperty, VhalApiTestIviPropertyNormalScreenIviId, ::testing::Values(
	VhalApiTestIviPropertyIdParameter{SCREEN_ID_FRONT}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestIviProperty, VhalApiTestIviPropertyNormalScreenName, ::testing::Values(
	VhalApiTestIviPropertyNameParameter{SCREEN_NAME_FRONT}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestIviProperty, VhalApiTestIviPropertyNormalLayerIviId, ::testing::Values(
	VhalApiTestIviPropertyIdParameter{LAYER_ID_VIDEO}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestIviProperty, VhalApiTestIviPropertyNormalLayerName, ::testing::Values(
	VhalApiTestIviPropertyNameParameter{LAYER_NAME_VIDEO}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestIviProperty, VhalApiTestIviPropertyNormalSurfaceIviId, ::testing::Values(
	VhalApiTestIviPropertyIdParameter{SURFACE_ID_FRONT_DTV}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestIviProperty, VhalApiTestIviPropertyNormalSurfaceName, ::testing::Values(
	VhalApiTestIviPropertyNameParameter{SURFACE_NAME_FRONT_DTV}
));

/**
 * 基本動作
 * スクリーンのiviプロパティデータ取得(ivi_id指定)
 */
TEST_P(VhalApiTestIviPropertyNormalScreenIviId, Normal)
{
	VhalApiTestIviPropertyIdParameter param = GetParam();
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
TEST_P(VhalApiTestIviPropertyNormalScreenName, Normal)
{
	VhalApiTestIviPropertyNameParameter param = GetParam();
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
TEST_P(VhalApiTestIviPropertyNormalLayerIviId, Normal)
{
	VhalApiTestIviPropertyIdParameter param = GetParam();
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
TEST_P(VhalApiTestIviPropertyNormalLayerName, Normal)
{
	VhalApiTestIviPropertyNameParameter param = GetParam();
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
TEST_P(VhalApiTestIviPropertyNormalSurfaceIviId, Normal)
{
	VhalApiTestIviPropertyIdParameter param = GetParam();
	std::string ivi_property{};

	cockpit::hal::video_hal::Init(obj);
	ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "ivi.surface.status.properties?ivi_id=" + std::to_string(param.ivi_id), &ivi_property, VIDEO_HAL_IVI_PROP_JSON_MAX, VIDEO_HAL_DATA_TYPE_STR));

	PrintMsg("ivi_property=[" + ivi_property + "]");
	KillRenderImage(obj,SURFACE_ID_FRONT_DTV);
	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * サーフェスのiviプロパティデータ取得(name指定)
 */
TEST_P(VhalApiTestIviPropertyNormalSurfaceName, Normal)
{
	VhalApiTestIviPropertyNameParameter param = GetParam();
	std::string ivi_property{};
	cockpit::hal::video_hal::Init(obj);
	ExecuteRenderImage(obj, SURFACE_ID_FRONT_DTV);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "ivi.surface.status.properties?name=" + param.name, &ivi_property, VIDEO_HAL_IVI_PROP_JSON_MAX, VIDEO_HAL_DATA_TYPE_STR));

	PrintMsg("ivi_property=[" + ivi_property + "]");
	KillRenderImage(obj,SURFACE_ID_FRONT_DTV);	
	cockpit::hal::video_hal::Deinit(obj);
}
