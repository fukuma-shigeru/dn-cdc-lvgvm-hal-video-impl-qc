#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static int32_t status_layer_order;
static int32_t status_layer_order_expect;
static int32_t cb_ivi_id_param;
static std::string cb_name_param;
static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;

static void CallbackIviLayerOrderIviId(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "ivi.layer.status.order?ivi_id=" + std::to_string(cb_ivi_id_param), &status_layer_order, sizeof(status_layer_order), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_layer_order_expect, status_layer_order);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
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

struct VhalApiTestIviLayerIdParameter {
	int32_t ivi_id;
	int32_t order;
	int32_t prev_order;
};

struct VhalApiTestIviLayerNameParameter {
	std::string name;
	int32_t order;
	int32_t prev_order;
};

class VhalApiTestIviLayerNormalIviId : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestIviLayerIdParameter> {};
class VhalApiTestIviLayerNormalName : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestIviLayerNameParameter> {};
class VhalApiTestIviLayerRangeIviId : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestIviLayerIdParameter> {};
class VhalApiTestIviLayerRangeName : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestIviLayerNameParameter> {};
class VhalApiTestIviLayerRepeatIviId : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestIviLayerIdParameter> {};
class VhalApiTestIviLayerRepeatName : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestIviLayerNameParameter> {};
class VhalApiTestIviLayerAbnormalNotConnectedR : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestIviLayerIdParameter> {};
class VhalApiTestIviLayerAbnormalNotConnectedI : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestIviLayerIdParameter> {};

INSTANTIATE_TEST_CASE_P(VhalApiTestIviLayer, VhalApiTestIviLayerNormalIviId, ::testing::Values(
	VhalApiTestIviLayerIdParameter{LAYER_ID_VIDEO, 10000, 4000}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestIviLayer, VhalApiTestIviLayerNormalName, ::testing::Values(
	VhalApiTestIviLayerNameParameter{LAYER_NAME_VIDEO, 10000, 4000}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestIviLayer, VhalApiTestIviLayerRangeIviId, ::testing::Values(
	VhalApiTestIviLayerIdParameter{LAYER_ID_VIDEO, INT32_MIN, 4000},
	VhalApiTestIviLayerIdParameter{LAYER_ID_VIDEO, INT32_MAX, 4000}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestIviLayer, VhalApiTestIviLayerRangeName, ::testing::Values(
	VhalApiTestIviLayerNameParameter{LAYER_NAME_VIDEO, INT32_MIN, 4000},
	VhalApiTestIviLayerNameParameter{LAYER_NAME_VIDEO, INT32_MAX, 4000}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestIviLayer, VhalApiTestIviLayerRepeatIviId, ::testing::Values(
	VhalApiTestIviLayerIdParameter{LAYER_ID_VIDEO, 10000, 4000}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestIviLayer, VhalApiTestIviLayerRepeatName, ::testing::Values(
	VhalApiTestIviLayerNameParameter{LAYER_NAME_VIDEO, 10000, 4000}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestIviLayer, VhalApiTestIviLayerAbnormalNotConnectedR, ::testing::Values(
	VhalApiTestIviLayerIdParameter{LAYER_ID_REAR_VIDEO, 10000, 4000}
));

INSTANTIATE_TEST_CASE_P(VhalApiTestIviLayer, VhalApiTestIviLayerAbnormalNotConnectedI, ::testing::Values(
	VhalApiTestIviLayerIdParameter{LAYER_ID_IC_MAP, 10000, 4000}
));

/**
 * 基本動作 
 * レイヤーのorder値設定(ivi_id指定)
 */
TEST_P(VhalApiTestIviLayerNormalIviId, Normal)
{
	VhalApiTestIviLayerIdParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "ivi.layer.status.order", CallbackIviLayerOrderIviId));

	/* iviレイヤー優先度変更 */
	status_layer_order = 0;
	status_layer_order_expect = param.order;
	cb_ivi_id_param = param.ivi_id;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "ivi.layer.control.order?ivi_id=" + std::to_string(param.ivi_id), &param.order, sizeof(param.order), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for layer_order.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_layer_order_expect == status_layer_order); });
	EXPECT_EQ(true, intime);

	int32_t control_order{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "ivi.layer.control.order?ivi_id=" + std::to_string(param.ivi_id), &control_order, sizeof(control_order), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(param.order, control_order);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "ivi.layer.status.order", CallbackIviLayerOrderIviId));

	SetIviLayerOrder(obj, param.ivi_id, param.prev_order);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作 
 * レイヤーのorder値設定(name指定)
 */
TEST_P(VhalApiTestIviLayerNormalName, Normal)
{
	VhalApiTestIviLayerNameParameter param = GetParam();
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
 * パラメータ上限下限
 * レイヤーのorder値設定(ivi_id指定)
 */
TEST_P(VhalApiTestIviLayerRangeIviId, Normal)
{
	VhalApiTestIviLayerIdParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "ivi.layer.status.order", CallbackIviLayerOrderIviId));

	/* iviレイヤー優先度変更 */
	status_layer_order = 0;
	status_layer_order_expect = param.order;
	cb_ivi_id_param = param.ivi_id;

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "ivi.layer.control.order?ivi_id=" + std::to_string(param.ivi_id), &param.order, sizeof(param.order), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for layer_order.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_layer_order_expect == status_layer_order); });
	EXPECT_EQ(true, intime);

	int32_t control_order{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "ivi.layer.control.order?ivi_id=" + std::to_string(param.ivi_id), &control_order, sizeof(control_order), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(param.order, control_order);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "ivi.layer.status.order", CallbackIviLayerOrderIviId));

	SetIviLayerOrder(obj, param.ivi_id, param.prev_order);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ上限下限
 * レイヤーのorder値設定(name指定)
 */
TEST_P(VhalApiTestIviLayerRangeName, Normal)
{
	VhalApiTestIviLayerNameParameter param = GetParam();
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
 * 設定前の参照 
 */
TEST(VhalApiTestIviLayerNotSet, NotSet)
{
	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "ivi.layer.status.order?ivi_id=" + std::to_string(cb_ivi_id_param), &status_layer_order, sizeof(status_layer_order), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(status_layer_order_expect, status_layer_order);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "ivi.layer.status.order?name=" + std::to_string(cb_ivi_id_param), &status_layer_order, sizeof(status_layer_order), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(status_layer_order_expect, status_layer_order);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール 
 * レイヤーのorder値設定(ivi_id指定)
 */
TEST_P(VhalApiTestIviLayerRepeatIviId, Repeat)
{
	VhalApiTestIviLayerIdParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	/* iviレイヤー優先度変更 */
	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "ivi.layer.control.order?ivi_id=" + std::to_string(param.ivi_id), &param.order, sizeof(param.order), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	}

	SetIviLayerOrder(obj, param.ivi_id, param.prev_order);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール 
 * レイヤーのorder値設定(name指定)
 */
TEST_P(VhalApiTestIviLayerRepeatName, Repeat)
{
	VhalApiTestIviLayerNameParameter param = GetParam();

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

/**
 * 前提を満たさない条件でのコール
 * RSE未接続でscreen/layer/surface情報を取得する
 * 前提条件:RSE未接続
 */
TEST_P(VhalApiTestIviLayerAbnormalNotConnectedR, AbnormalNotConnectedR)
{
	VhalApiTestIviLayerIdParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	/* iviレイヤー優先度変更 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "ivi.layer.control.order?ivi_id=" + std::to_string(param.ivi_id), &param.order, sizeof(param.order), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PROP_UPDATE, cockpit::hal::video_hal::RequestUpdate(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 前提を満たさない条件でのコール 
 * IC未接続でscreen/layer/surface情報を取得する
 * 前提条件:IC未接続
 */
TEST_P(VhalApiTestIviLayerAbnormalNotConnectedI, AbnormalNotConnectedI)
{
	VhalApiTestIviLayerIdParameter param = GetParam();

	cockpit::hal::video_hal::Init(obj);

	/* iviレイヤー優先度変更 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "ivi.layer.control.order?ivi_id=" + std::to_string(param.ivi_id), &param.order, sizeof(param.order), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PROP_UPDATE, cockpit::hal::video_hal::RequestUpdate(obj));

	cockpit::hal::video_hal::Deinit(obj);
}