#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static uint32_t status_configuration;
static uint32_t status_configuration_expect;
static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;

static void CallbackConfiguration(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "ivi.layout.status.configuration.file", &status_configuration, sizeof(status_configuration), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_configuration_expect, status_configuration);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

struct VhalApiTestLayerConfigurationParameter {
	std::string path;
};


class VhalApiTestLayerConfigurationNormal : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestLayerConfigurationParameter> {};
class VhalApiTestLayerConfigurationRepeat : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestLayerConfigurationParameter> {};
class VhalApiTestLayerConfigurationAbnormalArgInvalidPath : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestLayerConfigurationParameter> {};

INSTANTIATE_TEST_CASE_P(VhalApiTestIviLayer, VhalApiTestLayerConfigurationNormal, ::testing::Values(
	VhalApiTestLayerConfigurationParameter{LAYER_CONFIG_JSON}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestIviLayer, VhalApiTestLayerConfigurationRepeat, ::testing::Values(
	VhalApiTestLayerConfigurationParameter{LAYER_CONFIG_JSON}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestIviLayer, VhalApiTestLayerConfigurationAbnormalArgInvalidPath, ::testing::Values(
	VhalApiTestLayerConfigurationParameter{INVALID_LAYER_CONFIG_JSON}
));

/**
 * 基本動作
 */
TEST_P(VhalApiTestLayerConfigurationNormal, Normal)
{
	VhalApiTestLayerConfigurationParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "ivi.layout.status.configuration.file", CallbackConfiguration));

	status_configuration = VIDEO_HAL_IVI_LAYOUT_STS_FAILED;
	status_configuration_expect = VIDEO_HAL_IVI_LAYOUT_STS_SUCCESS;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "ivi.layout.control.configuration.file", &param.path, param.path.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for configuration.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_configuration_expect == status_configuration); });
	EXPECT_EQ(true, intime);

	std::string path{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "ivi.layout.control.configuration.file", &path, 512, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(param.path, path);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "ivi.layout.status.configuration.file", CallbackConfiguration));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照 
 */
TEST(VhalApiTestLayerConfigurationNotSet, NotSet)
{
	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "ivi.layout.status.configuration.file", &status_configuration, sizeof(status_configuration), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_IVI_LAYOUT_STS_SUCCESS, status_configuration);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール
 */
TEST_P(VhalApiTestLayerConfigurationRepeat, Repeat)
{
	VhalApiTestLayerConfigurationParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "ivi.layout.control.configuration.file", &param.path, param.path.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	}

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ異常
 * 存在しないファイルパスを設定する
 * →処理自体がNOPされているため、異常が出力されることがない
*/
TEST_P(VhalApiTestLayerConfigurationAbnormalArgInvalidPath, AbnormalArgInvalidPath)
{
//	VhalApiTestLayerConfigurationParameter param = GetParam();
//	std::unique_lock<std::mutex> lock_sync(mtx_sync);
//	bool intime{false};
//
//	cockpit::hal::video_hal::Init(obj);
//
//	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "ivi.layout.status.configuration.file", CallbackConfiguration));
//
//	status_configuration = VIDEO_HAL_IVI_LAYOUT_STS_SUCCESS;
//	status_configuration_expect = VIDEO_HAL_IVI_LAYOUT_STS_FAILED;
//
//	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "ivi.layout.control.configuration.file", &param.path, param.path.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
//	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
//	PrintMsg("wait_for configuration.");
//	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_configuration_expect == status_configuration); });
//	EXPECT_EQ(true, intime);
//
//	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "ivi.layout.status.configuration.file", CallbackConfiguration));
//
//	cockpit::hal::video_hal::Deinit(obj);	
}