#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static uint32_t status_video_source_id;
static uint32_t status_video_source_id_expect;
static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;

static void CallbackDummy(void)
{
}

static void CallbackVideoSourceId(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.id", &status_video_source_id, sizeof(status_video_source_id), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_video_source_id_expect, status_video_source_id);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

struct VhalApiTestLibVideoHalParameter {
     std::string prop;
     std::string callback;
};

class VhalApiTestLibVideoHalAbnormalArgGetValue : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestLibVideoHalParameter> {};
class VhalApiTestLibVideoHalAbnormalArgSetValue : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestLibVideoHalParameter> {};
class VhalApiTestLibVideoHalAbnormalArgRegisterCallback : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestLibVideoHalParameter> {};
class VhalApiTestLibVideoHalAbnormalArgClearCallback : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestLibVideoHalParameter> {};

INSTANTIATE_TEST_CASE_P(VhalApiTestLibVideoHal, VhalApiTestLibVideoHalAbnormalArgGetValue, ::testing::Values(
	VhalApiTestLibVideoHalParameter{INVALID_PROPERTY}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestLibVideoHal, VhalApiTestLibVideoHalAbnormalArgSetValue, ::testing::Values(
	VhalApiTestLibVideoHalParameter{INVALID_PROPERTY}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestLibVideoHal, VhalApiTestLibVideoHalAbnormalArgRegisterCallback, ::testing::Values(
	VhalApiTestLibVideoHalParameter{INVALID_PROPERTY}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestLibVideoHal, VhalApiTestLibVideoHalAbnormalArgClearCallback, ::testing::Values(
	VhalApiTestLibVideoHalParameter{INVALID_PROPERTY}
));

/**
 * 基本動作
 */
TEST(VhalApiTestLibVideoHalNormal, Normal)
{
    std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	uint32_t video_source_id{};

    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));

    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.id", CallbackVideoSourceId));

	/* 前席映像ソース設定 */
	status_video_source_id = VIDEO_HAL_VSRC_ID_OTHER;
	status_video_source_id_expect = VIDEO_HAL_VSRC_ID_HDMI;
	video_source_id = status_video_source_id_expect;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.id", &video_source_id, sizeof(video_source_id), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for video_source_id.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_video_source_id_expect == status_video_source_id); });
	EXPECT_EQ(true, intime);

    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.id", CallbackVideoSourceId));

    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * 短時間の連続コール
 */
TEST(VhalApiTestLibVideoHalRepeat, Repeat)
{
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));
    for (int i = 0; i < TEST_REPEAT_COUNT; i++)
    {
        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "movie.front.status.result", CallbackDummy));
	    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "movie.front.status.result", CallbackDummy));
    }
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * パラメータ異常
 * NULLポインタ
 */
TEST(VhalApiTestLibVideoHalAbnormalArgNullPointer, AbnormalArg)
{
	int32_t status{};

    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.id", nullptr, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.status.id", nullptr, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.id", nullptr));
    EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.id", nullptr));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * パラメータ異常
 * GetValue
 */
TEST_P(VhalApiTestLibVideoHalAbnormalArgGetValue, AbnormalArg)
{
	VhalApiTestLibVideoHalParameter param = GetParam();
	int32_t status{};

    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::GetValue(obj, param.prop, &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * パラメータ異常 
 * SetValue
 */
TEST_P(VhalApiTestLibVideoHalAbnormalArgSetValue, AbnormalArg)
{
	VhalApiTestLibVideoHalParameter param = GetParam();
	int32_t status{};

    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, param.prop, &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * パラメータ異常
 * RegisterCallback 
 */
//RegisterCallbackはプロパティのチェックは実施していない為、テストから削除
TEST_P(VhalApiTestLibVideoHalAbnormalArgRegisterCallback, AbnormalArg)
{
//	VhalApiTestLibVideoHalParameter param = GetParam();
//
//    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));
//    EXPECT_EQ(VIDEO_HAL_API_ERR_PROP_REG_CB, cockpit::hal::video_hal::RegisterCallback(obj, param.prop, CallbackVideoSourceId));
//    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * パラメータ異常
 * ClearCallback
*/
TEST_P(VhalApiTestLibVideoHalAbnormalArgClearCallback, AbnormalArg)
{
	VhalApiTestLibVideoHalParameter param = GetParam();
	
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));
    EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::ClearCallback(obj, param.prop, CallbackVideoSourceId));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * 前提を満たさない条件でのコール
 * Deinit 
 */
TEST(VhalApiTestLibVideoHalAbnormalNotInitDeinit, AbnormalNotInit)
{
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * 前提を満たさない条件でのコール
 * GetValue
 */
TEST(VhalApiTestLibVideoHalAbnormalNotInitGetValue, AbnormalNotInit)
{
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::GetValue(obj, "vsrc.front.status.id", &status_video_source_id, sizeof(status_video_source_id), VIDEO_HAL_DATA_TYPE_NUM));
}

/**
 * 前提を満たさない条件でのコール
 * SetValue
 */
TEST(VhalApiTestLibVideoHalAbnormalNotInitSetValue, AbnormalNotInit)
{
	uint32_t video_source_id{};
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.id", &video_source_id, sizeof(video_source_id), VIDEO_HAL_DATA_TYPE_NUM));
}

/**
 * 前提を満たさない条件でのコール
 * RequestUpdate
 */
TEST(VhalApiTestLibVideoHalAbnormalNotInitRequestUpdate, AbnormalNotInit)
{
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));
}

/**
 * 前提を満たさない条件でのコール
 * RegisterCallback 
 */
TEST(VhalApiTestLibVideoHalAbnormalNotInitRegisterCallback, AbnormalNotInit)
{
    EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RegisterCallback(obj, "vsrc.front.status.id", CallbackVideoSourceId));
}

/**
 * 前提を満たさない条件でのコール
 * ClearCallback
*/
TEST(VhalApiTestLibVideoHalAbnormalNotInitClearCallback, AbnormalNotInit)
{
    EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::ClearCallback(obj, "vsrc.front.status.id", CallbackVideoSourceId));
}

/**
 * 前提を満たさない条件でのコール
 * SetValueせずRequestUpdate
 */
TEST(VhalApiTestLibVideoHalAbnormalNotInitRequestUpdate, AbnormalNotSetValue)
{
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Init(obj));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PROP_UPDATE, cockpit::hal::video_hal::RequestUpdate(obj));
    EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}
