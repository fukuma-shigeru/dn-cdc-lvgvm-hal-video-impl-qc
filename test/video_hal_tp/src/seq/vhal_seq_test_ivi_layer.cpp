#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static int32_t status_layer_order;
static int32_t status_layer_order_expect;
static std::string cb_name_param;
static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;

static void CallbackIviLayerOrder(void)
{
	PrintMsg("CallbackIviLayerOrder start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "ivi.layer.status.order?ivi_id=3", &status_layer_order, sizeof(status_layer_order), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_layer_order_expect, status_layer_order);
	}
	cond_sync.notify_one();
	PrintMsg("CallbackIviLayerOrder end.");
}


static void CallbackIviLayerOrderName(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "ivi.layer.status.order?name=" + cb_name_param, &status_layer_order, sizeof(status_layer_order), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_layer_order_expect, status_layer_order);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

struct VhalSeqTestIviLayerParameter {
	int32_t surface_id;
	std::string front_vpath_name;
};

struct VhalSeqTestIviLayerNameParameter {
	std::string name;
	int32_t order;
	int32_t prev_order;
};

class VhalSeqTestIviLayerApiSpecification : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestIviLayerParameter> {};
class VhalSeqTestIviLayerNormalName : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestIviLayerNameParameter> {};
class VhalSeqTestIviLayerRepeatName : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestIviLayerNameParameter> {};



INSTANTIATE_TEST_CASE_P(VhalSeqTestIviLayer, VhalSeqTestIviLayerApiSpecification, ::testing::Values(
	VhalSeqTestIviLayerParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV}
));

INSTANTIATE_TEST_CASE_P(VhalSeqTestIviLayer, VhalSeqTestIviLayerNormalName, ::testing::Values(
	VhalSeqTestIviLayerNameParameter{LAYER_NAME_VIDEO, 10000, 4000}
));

INSTANTIATE_TEST_CASE_P(VhalSeqTestIviLayer, VhalSeqTestIviLayerRepeatName, ::testing::Values(
	VhalSeqTestIviLayerNameParameter{LAYER_NAME_VIDEO, 10000, 4000}
));

/**
 * API仕様書記載のシーケンス
 * iviレイヤー優先度変更
 */
TEST_P(VhalSeqTestIviLayerApiSpecification, ChangeIviLayerOrder)
{
	VhalSeqTestIviLayerParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	int32_t layer_order{};
	int32_t save_layer_order{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "ivi.layer.status.order", CallbackIviLayerOrder));

	/* iviレイヤー取得 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "ivi.layer.status.order?ivi_id=3", &status_layer_order, sizeof(status_layer_order), VIDEO_HAL_DATA_TYPE_NUM));
	save_layer_order = status_layer_order;

	/* iviレイヤー優先度変更 */
	status_layer_order = 0;
	status_layer_order_expect = 10000;
	layer_order = status_layer_order_expect;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "ivi.layer.control.order?ivi_id=3", &layer_order, sizeof(layer_order), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for layer_order.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_layer_order_expect == status_layer_order); });
	EXPECT_EQ(true, intime);	

    EXPECT_EQ(VHAL_TEST_SUCCESS, SetIviLayerOrder(obj, 3, save_layer_order));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "ivi.layer.status.order", CallbackIviLayerOrder));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作
 * iviレイヤー優先度変更
*/
TEST(VhalSeqTestIviLayerRepeat, ChangeIviLayerOrder)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	int32_t layer_order{};
	int32_t save_layer_order{};

	cockpit::hal::video_hal::Init(obj);

	/* iviレイヤー取得 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "ivi.layer.status.order?ivi_id=3", &status_layer_order, sizeof(status_layer_order), VIDEO_HAL_DATA_TYPE_NUM));
	save_layer_order = status_layer_order;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "ivi.layer.status.order", CallbackIviLayerOrder));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		/* iviレイヤー優先度変更 */
		status_layer_order = 0;
		status_layer_order_expect = 10000;
		layer_order = status_layer_order_expect;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "ivi.layer.control.order?ivi_id=3", &layer_order, sizeof(layer_order), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for layer_order.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_layer_order_expect == status_layer_order); });
		EXPECT_EQ(true, intime);

		/* もとに戻す */
		status_layer_order = 10000;
		status_layer_order_expect = 0;
		layer_order = status_layer_order_expect;

		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "ivi.layer.control.order?ivi_id=3", &layer_order, sizeof(layer_order), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for layer_order.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_layer_order_expect == status_layer_order); });
		EXPECT_EQ(true, intime);
	}

    EXPECT_EQ(VHAL_TEST_SUCCESS, SetIviLayerOrder(obj, 3, save_layer_order));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "ivi.layer.status.order", CallbackIviLayerOrder));

	cockpit::hal::video_hal::Deinit(obj);
}


/**
 * 基本動作 
 * レイヤーのorder値設定(name指定)
 */
TEST_P(VhalSeqTestIviLayerNormalName, ChangeIviLayerOrder)
{
	VhalSeqTestIviLayerNameParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "ivi.layer.status.order", CallbackIviLayerOrderName));

	/* iviレイヤー優先度変更 */
	status_layer_order = 0;
	status_layer_order_expect = param.order;
	cb_name_param = param.name;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "ivi.layer.control.order?name=" + param.name, &param.order, sizeof(param.order), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for layer_order.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_layer_order_expect == status_layer_order); });
	EXPECT_EQ(true, intime);

	int32_t control_order{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "ivi.layer.control.order?name=" + param.name, &control_order, sizeof(control_order), VIDEO_HAL_DATA_TYPE_NUM));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "ivi.layer.status.order", CallbackIviLayerOrderName));

	SetIviLayerOrder(obj, param.name, param.prev_order);

	cockpit::hal::video_hal::Deinit(obj);
}



/**
 * 短時間の連続コール 
 * レイヤーのorder値設定(name指定)
 */
TEST_P(VhalSeqTestIviLayerRepeatName, Repeat)
{
	VhalSeqTestIviLayerNameParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	/* iviレイヤー優先度変更 */
	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "ivi.layer.control.order?name=" + param.name, &param.order, sizeof(param.order), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	}

	SetIviLayerOrder(obj, param.name, param.prev_order);

	cockpit::hal::video_hal::Deinit(obj);
}



