#include "vhal_test_common.h"

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;

static int32_t status_display;
static int32_t status_display_expect;
static int32_t status_notification;
static int32_t status_notification_expect;

static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;

static void CallbackDisplay(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.rear.status.rse.display", &status_display, sizeof(status_display), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_display_expect, status_display);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void CallbackNotification(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue( obj, "vpath.rear.status.rse.notification", &status_notification, sizeof(status_notification), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_notification_expect, status_notification);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

struct VhalSeqTestVpathRseParameter {
	int32_t dummy;
};

class VhalSeqTestVpathRseApiSpecification : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestVpathRseParameter> {};

INSTANTIATE_TEST_CASE_P(VhalSeqTestVpathRse, VhalSeqTestVpathRseApiSpecification, ::testing::Values(
	VhalSeqTestVpathRseParameter{0}
));

/**
 * API仕様書記載のシーケンス
 */
TEST_P(VhalSeqTestVpathRseApiSpecification, RseDisplay)
{
	VhalSeqTestVpathRseParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	std::string vpath;
    bool notification{true};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRse(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.rse.display", CallbackDisplay));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.rse.notification", CallbackNotification));

	status_display_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_display = VIDEO_HAL_VPATH_STS_FAILED;
	vpath = VPATH_NAME_REAR_VDSP;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.display", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.rear.control.rse.display=REAR-VDSP. wait_for display.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_display_expect == status_display); });
	EXPECT_EQ(true, intime);

	status_notification_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_notification = VIDEO_HAL_VPATH_STS_FAILED;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.notification", &notification, sizeof(notification), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for notification.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_notification_expect == status_notification); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.rse.display", CallbackDisplay));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.rse.notification", CallbackNotification));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * 連続動作
 * API仕様書記載のシーケンス
 */
TEST(VhalSeqTestVpathRseRepeat, RseDisplay)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	std::string vpath;
    bool notification{true};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRse(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.rse.display", CallbackDisplay));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.rse.notification", CallbackNotification));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
        status_display_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
        status_display = VIDEO_HAL_VPATH_STS_FAILED;
        vpath = VPATH_NAME_REAR_VDSP;
        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.display", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
        PrintMsg("SetValue vpath.rear.control.rse.display=REAR-VDSP. wait_for display.");
        intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_display_expect == status_display); });
        EXPECT_EQ(true, intime);

        status_notification_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
        status_notification = VIDEO_HAL_VPATH_STS_FAILED;
        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.notification", &notification, sizeof(notification), VIDEO_HAL_DATA_TYPE_BOOL));
        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
        PrintMsg("wait_for notification.");
        intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_notification_expect == status_notification); });
        EXPECT_EQ(true, intime);

		/* パスクリア */
        status_display_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
        status_display = VIDEO_HAL_VPATH_STS_FAILED;
        vpath = VPATH_NAME_CLEAR;
        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.display", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
        PrintMsg("SetValue vpath.rear.control.rse.display=REAR-VDSP. wait_for display.");
        intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_display_expect == status_display); });
        EXPECT_EQ(true, intime);

        status_notification_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
        status_notification = VIDEO_HAL_VPATH_STS_FAILED;
        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.notification", &notification, sizeof(notification), VIDEO_HAL_DATA_TYPE_BOOL));
        EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
        PrintMsg("wait_for notification.");
        intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_notification_expect == status_notification); });
        EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.rse.display", CallbackDisplay));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.rse.notification", CallbackNotification));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * 順番違い
 * 後席専用映像反映通知、後席専用モード向け映像パス設定の順で設定する。
 */
TEST(VhalSeqTestVpathRseShuffleOrder, RseDisplay)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	std::string vpath;
    bool notification{true};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRse(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.rse.display", CallbackDisplay));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.rse.notification", CallbackNotification));

	status_notification_expect = VIDEO_HAL_VPATH_STS_FAILED;
	status_notification = VIDEO_HAL_VPATH_STS_SUCCESS;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.notification", &notification, sizeof(notification), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for notification.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_notification_expect == status_notification); });
	EXPECT_EQ(true, intime);

	status_display_expect = VIDEO_HAL_VPATH_STS_SUCCESS;
	status_display = VIDEO_HAL_VPATH_STS_FAILED;
	vpath = VPATH_NAME_REAR_VDSP;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.display", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("SetValue vpath.rear.control.rse.display=REAR-VDSP. wait_for display.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_display_expect == status_display); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.rse.display", CallbackDisplay));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.rse.notification", CallbackNotification));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}

/**
 * シーケンス抜け
 * 後席専用モード向け映像パス設定せず、後席専用映像反映通知を設定する。
 */
TEST(VhalSeqTestVpathRseSkip, RseDisplay)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	std::string vpath;
    bool notification{true};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VHAL_TEST_SUCCESS, SwitchVpathRse(obj, VPATH_NAME_CLEAR));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.rse.display", CallbackDisplay));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.rse.notification", CallbackNotification));

	status_notification_expect = VIDEO_HAL_VPATH_STS_FAILED;
	status_notification = VIDEO_HAL_VPATH_STS_SUCCESS;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.notification", &notification, sizeof(notification), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for notification.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_notification_expect == status_notification); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.rse.display", CallbackDisplay));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.rse.notification", CallbackNotification));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::Deinit(obj));
}
