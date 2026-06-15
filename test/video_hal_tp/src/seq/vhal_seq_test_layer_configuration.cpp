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

struct VhalSeqTestLayerConfigurationParameter {
	std::string path;
};


class VhalSeqTestLayerConfigurationApiSpecification : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestLayerConfigurationParameter> {};

INSTANTIATE_TEST_CASE_P(VhalSeqTestIviLayer, VhalSeqTestLayerConfigurationApiSpecification, ::testing::Values(
	VhalSeqTestLayerConfigurationParameter{LAYER_CONFIG_JSON}
));


/**
 * API仕様書記載のシーケンス
 * iviレイアウト構成設定
 */
TEST_P(VhalSeqTestLayerConfigurationApiSpecification, SetConfiguration)
{
	VhalSeqTestLayerConfigurationParameter param = GetParam();
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

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "ivi.layout.status.configuration.file", CallbackConfiguration));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 * iviレイアウト構成設定
 */
TEST(VhalSeqTestLayerConfigurationRepeat, SetConfiguration)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "ivi.layout.status.configuration.file", CallbackConfiguration));

    for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
    {     
        status_configuration = VIDEO_HAL_IVI_LAYOUT_STS_FAILED;
        status_configuration_expect = VIDEO_HAL_IVI_LAYOUT_STS_SUCCESS;
        std::string readpath{LAYER_CONFIG_JSON};

        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "ivi.layout.control.configuration.file", &readpath, readpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
        PrintMsg("wait_for configuration.");
        intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_configuration_expect == status_configuration); });
        EXPECT_EQ(true, intime);
    }

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "ivi.layout.status.configuration.file", CallbackConfiguration));
}
