#include "vhal_test_common.h"
#include <iostream>
#include <filesystem>

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static int32_t status_movie;
static int32_t status_movie_expect;
static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;

static void CallbackMovie(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status_movie, sizeof(status_movie), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(status_movie_expect, status_movie);
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void RequestClear()
{
	bool oneshot{true};
	
	int32_t ret = cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status_movie, sizeof(status_movie), VIDEO_HAL_DATA_TYPE_NUM);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (VIDEO_HAL_MOVIE_STS_PLAYING  == status_movie)
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.cancel", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
		ret = cockpit::hal::video_hal::RequestUpdate(obj);
	}
	
	if (VIDEO_HAL_MOVIE_STS_NONE != status_movie)
	{
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.clear", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
		ret = cockpit::hal::video_hal::RequestUpdate(obj);
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	PrintMsg("return:" + std::to_string(ret));
}

struct VhalSeqTestMovieParameter {
	int32_t surface_id;
	std::string front_vpath_name;
};

class VhalSeqTestMovieApiSpecification : public VhalSeqTest, public ::testing::WithParamInterface<VhalSeqTestMovieParameter> {};

INSTANTIATE_TEST_CASE_P(VhalSeqTestMovie, VhalSeqTestMovieApiSpecification, ::testing::Values(
	VhalSeqTestMovieParameter{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV}
));

/**
 * API仕様書記載のシーケンス
 * 汎用動画再生(最後まで再生)
 */
TEST_P(VhalSeqTestMovieApiSpecification, PlayMovieCompleted)
{
	VhalSeqTestMovieParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	std::string path{};
	uint32_t x{};
	uint32_t y{};
	uint32_t width{};
	uint32_t height{};
	bool oneshot{};

	cockpit::hal::video_hal::Init(obj);

	RequestClear();
	
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "movie.front.status.result", CallbackMovie));

	/* 動画再生 */
	status_movie = VIDEO_HAL_MOVIE_STS_NONE;
	status_movie_expect = VIDEO_HAL_MOVIE_STS_READY;
	path = GetMovieFileName();
	x = 0;
	y = 0;
	width = 1280;
	height = 720;
	oneshot = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.file.path", &path, path.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.x", &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.y", &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.prepare", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for prepare.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
	EXPECT_EQ(true, intime);

	/* 再生 */
	status_movie = VIDEO_HAL_MOVIE_STS_READY;
	status_movie_expect = VIDEO_HAL_MOVIE_STS_PLAYING;
	oneshot = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.start", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for start.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
	EXPECT_EQ(true, intime);

	/* 動画再生完了まで待つ */
	status_movie = VIDEO_HAL_MOVIE_STS_PLAYING;
	status_movie_expect = VIDEO_HAL_MOVIE_STS_FINISHED;
	oneshot = true;
	PrintMsg("wait_for finish.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(TEST_MOVIE_PLAYTIME_SECOND), [] { return (status_movie_expect == status_movie); });
	EXPECT_EQ(true, intime);

	/* クリア */
	status_movie = VIDEO_HAL_MOVIE_STS_CANCELED;
	status_movie_expect = VIDEO_HAL_MOVIE_STS_NONE;
	oneshot = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.clear", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for clear.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "movie.front.status.result", CallbackMovie));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * API仕様書記載のシーケンス
 * 汎用動画再生(再生途中で中止)
 */
TEST_P(VhalSeqTestMovieApiSpecification, PlayMovieCancel)
{
	VhalSeqTestMovieParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	std::string path{};
	uint32_t x{};
	uint32_t y{};
	uint32_t width{};
	uint32_t height{};
	bool oneshot{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "movie.front.status.result", CallbackMovie));
	
	/* 汎用動画再生 */
	status_movie = VIDEO_HAL_MOVIE_STS_NONE;
	status_movie_expect = VIDEO_HAL_MOVIE_STS_READY;
	path = GetMovieFileName();
	x = 0;
	y = 0;
	width = 1280;
	height = 720;
	oneshot = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.file.path", &path, path.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.x", &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.y", &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.prepare", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for prepare.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
	EXPECT_EQ(true, intime);

	/* 再生 */
	status_movie = VIDEO_HAL_MOVIE_STS_READY;
	status_movie_expect = VIDEO_HAL_MOVIE_STS_PLAYING;
	oneshot = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.start", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for start.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
	EXPECT_EQ(true, intime);

	/* 再生中止 */
	status_movie = VIDEO_HAL_MOVIE_STS_PLAYING;
	status_movie_expect = VIDEO_HAL_MOVIE_STS_CANCELED;
	oneshot = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.cancel", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for cancel.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
	EXPECT_EQ(true, intime);

	/* クリア */
	status_movie = VIDEO_HAL_MOVIE_STS_CANCELED;
	status_movie_expect = VIDEO_HAL_MOVIE_STS_NONE;
	oneshot = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.clear", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for clear.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
	EXPECT_EQ(true, intime);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "movie.front.status.result", CallbackMovie));

	cockpit::hal::video_hal::Deinit(obj);
}

/** 
 * 連続動作
 * 汎用動画再生(最後まで再生)
 */
TEST(VhalSeqTestMovieRepeat, PlayMovieCompleted)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	std::string path{};
	uint32_t x{};
	uint32_t y{};
	uint32_t width{};
	uint32_t height{};
	bool oneshot{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "movie.front.status.result", CallbackMovie));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		/* 動画再生 */
		status_movie = VIDEO_HAL_MOVIE_STS_NONE;
		status_movie_expect = VIDEO_HAL_MOVIE_STS_READY;
		path = GetMovieFileName();
		x = 0;
		y = 0;
		width = 1280;
		height = 720;
		oneshot = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.file.path", &path, path.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.x", &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.y", &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.prepare", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for prepare.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
		EXPECT_EQ(true, intime);

		/* 再生 */
		status_movie = VIDEO_HAL_MOVIE_STS_READY;
		status_movie_expect = VIDEO_HAL_MOVIE_STS_PLAYING;
		oneshot = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.start", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for start.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
		EXPECT_EQ(true, intime);
		
		/* 動画再生完了まで待つ */
		status_movie = VIDEO_HAL_MOVIE_STS_PLAYING;
		status_movie_expect = VIDEO_HAL_MOVIE_STS_FINISHED;
		oneshot = true;
		PrintMsg("wait_for finish.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(TEST_MOVIE_PLAYTIME_SECOND), [] { return (status_movie_expect == status_movie); });
		EXPECT_EQ(true, intime);

		/* クリア */
		status_movie = VIDEO_HAL_MOVIE_STS_CANCELED;
		status_movie_expect = VIDEO_HAL_MOVIE_STS_NONE;
		oneshot = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.clear", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for clear.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "movie.front.status.result", CallbackMovie));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 連続動作 
 * 汎用動画再生(再生途中で中止)
 */
TEST(VhalSeqTestMovieRepeat, PlayMovieCancel)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	std::string path{};
	uint32_t x{};
	uint32_t y{};
	uint32_t width{};
	uint32_t height{};
	bool oneshot{};

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "movie.front.status.result", CallbackMovie));

	for (size_t i = 0; i < TEST_SEQUENCE_REPEAT_COUNT; i++)
	{
		/* 汎用動画再生 */
		status_movie = VIDEO_HAL_MOVIE_STS_NONE;
		status_movie_expect = VIDEO_HAL_MOVIE_STS_READY;
		path = GetMovieFileName();
		x = 0;
		y = 0;
		width = 1280;
		height = 720;
		oneshot = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.file.path", &path, path.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.x", &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.y", &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.prepare", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for prepare.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
		EXPECT_EQ(true, intime);

		/* 再生 */
		status_movie = VIDEO_HAL_MOVIE_STS_READY;
		status_movie_expect = VIDEO_HAL_MOVIE_STS_PLAYING;
		oneshot = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.start", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for start.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
		EXPECT_EQ(true, intime);

		/* 再生中止 */
		status_movie = VIDEO_HAL_MOVIE_STS_PLAYING;
		status_movie_expect = VIDEO_HAL_MOVIE_STS_CANCELED;
		oneshot = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.cancel", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for cancel.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
		EXPECT_EQ(true, intime);

		/* クリア */
		status_movie = VIDEO_HAL_MOVIE_STS_CANCELED;
		status_movie_expect = VIDEO_HAL_MOVIE_STS_NONE;
		oneshot = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.clear", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
		PrintMsg("wait_for clear.");
		intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
		EXPECT_EQ(true, intime);
	}

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "movie.front.status.result", CallbackMovie));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 順番違いヘルパ
 * 準備 
 */
static int32_t Prepare(cockpit::hal::video_hal::CtlObj& obj)
{
	int32_t ret{};
	int32_t request_expect{};
	
	ret = cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status_movie, sizeof(status_movie), VIDEO_HAL_DATA_TYPE_NUM);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (VIDEO_HAL_MOVIE_STS_NONE == status_movie)
	{
		request_expect = VIDEO_HAL_API_SUCCESS;
	}
	else 
	{
		request_expect = VIDEO_HAL_API_ERR_PROP_UPDATE;
	}
	
	PrintMsg("prepare(status_movie:" + std::to_string(status_movie) + ")");
	std::string path{GetMovieFileName()};
	uint32_t x{0};
	uint32_t y{0};
	uint32_t width{100};
	uint32_t height{100};
	bool oneshot = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.file.path", &path, path.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.x", &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.y", &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.prepare", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(request_expect, cockpit::hal::video_hal::RequestUpdate(obj));
	return VHAL_TEST_SUCCESS;
}

/**
 * 順番違いヘルパ
 * 再生 
 */
static int32_t Play(cockpit::hal::video_hal::CtlObj& obj)
{
	int32_t ret{};
	int32_t request_expect{};

	ret = cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status_movie, sizeof(status_movie), VIDEO_HAL_DATA_TYPE_NUM);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (VIDEO_HAL_MOVIE_STS_READY == status_movie)
	{
		request_expect = VIDEO_HAL_API_SUCCESS;
	}
	else 
	{
		request_expect = VIDEO_HAL_API_ERR_PROP_UPDATE;
	}
	PrintMsg("start(status_movie:" + std::to_string(status_movie) + ")");
	bool oneshot{true};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.start", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(request_expect, cockpit::hal::video_hal::RequestUpdate(obj));
	if (VIDEO_HAL_API_SUCCESS == request_expect)
	{
		for (int32_t i=0; i<20 && VIDEO_HAL_MOVIE_STS_PLAYING != status_movie ; ++i)
		{
			EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status_movie, sizeof(status_movie), VIDEO_HAL_DATA_TYPE_NUM));
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	}
	return VHAL_TEST_SUCCESS;
}

/**
 * 順番違いヘルパ
 * 再生中止 
 */
static int32_t Cancel(cockpit::hal::video_hal::CtlObj& obj)
{
	int32_t ret{};
	int32_t request_expect{};

	ret = cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status_movie, sizeof(status_movie), VIDEO_HAL_DATA_TYPE_NUM);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (VIDEO_HAL_MOVIE_STS_PLAYING == status_movie)
	{
		request_expect = VIDEO_HAL_API_SUCCESS;
	}
	else 
	{
		request_expect = VIDEO_HAL_API_ERR_PROP_UPDATE;
	}
	PrintMsg("cancel(status_movie:" + std::to_string(status_movie) + ")");
	bool oneshot{true};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.cancel", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(request_expect, cockpit::hal::video_hal::RequestUpdate(obj));
	return VHAL_TEST_SUCCESS;
}

/**
 * 順番違いヘルパ
 * クリア 
 */
static int32_t Clear(cockpit::hal::video_hal::CtlObj& obj)
{
	int32_t ret{};
	int32_t request_expect{};

	ret = cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status_movie, sizeof(status_movie), VIDEO_HAL_DATA_TYPE_NUM);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (VIDEO_HAL_MOVIE_STS_READY    == status_movie ||
	    VIDEO_HAL_MOVIE_STS_FINISHED == status_movie ||
	    VIDEO_HAL_MOVIE_STS_CANCELED == status_movie ||
	    VIDEO_HAL_MOVIE_STS_FAILED   == status_movie)
	{
		request_expect = VIDEO_HAL_API_SUCCESS;
	}
	else 
	{
		request_expect = VIDEO_HAL_API_ERR_PROP_UPDATE;
	}
	PrintMsg("clear(status_movie:" + std::to_string(status_movie) + ")");
	bool oneshot{true};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.clear", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(request_expect, cockpit::hal::video_hal::RequestUpdate(obj));
	return VHAL_TEST_SUCCESS;
}

/**
 * 順番違い
 * 制御要求(準備、再生、再生中止、クリア)を入れ替えて実行
 */
TEST(VhalSeqTestMovieShuffleOrder, ShuffleOrder)
{
	cockpit::hal::video_hal::Init(obj);

	/* 順列計算のための昇順 */
	std::vector<int> v = {1, 2, 3, 4};

	for(;;)
	{
		if (std::next_permutation(v.begin(), v.end()) == false) 
		{
			break;
		}
		else
		{
			RequestClear();
		}
		std::for_each(v.begin(), v.end(), [&](int x)
		{
			if (1 == x) Prepare(obj);
			if (2 == x) Play(obj);
			if (3 == x) Cancel(obj);
			if (4 == x) Clear(obj);
		});
	}

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * シーケンス抜け
 * 再生準備を行わず再生
 */
TEST(VhalSeqTestMovieSkip, Skip)
{
	cockpit::hal::video_hal::Init(obj);

	RequestClear();

	cockpit::hal::video_hal::Deinit(obj);
}

/*===================================================*/
/* ここから異常系試験                                */
/*===================================================*/

/**
 * Prepare失敗
 */
TEST(VhalSeqTestMovieFail, PrepareFrontMovie)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	std::string path{};
	uint32_t x{};
	uint32_t y{};
	uint32_t width{};
	uint32_t height{};
	bool oneshot{};

	cockpit::hal::video_hal::Init(obj);
	RequestClear();		/* VIDEO_HAL_MOVIE_STS_NONE状態にする */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "movie.front.status.result", CallbackMovie));

	/* 動画再生準備失敗 */
	status_movie = VIDEO_HAL_MOVIE_STS_NONE;
	status_movie_expect = VIDEO_HAL_MOVIE_STS_READY;
	path = GetMovieFileName();
	x = 0;
	y = 0;
	width = 1280;
	height = 0;		/* ← fail */
	oneshot = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.file.path", &path, path.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.x", &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.y", &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.prepare", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	/* 終了処理 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "movie.front.status.result", CallbackMovie));
	RequestClear();		/* VIDEO_HAL_MOVIE_STS_NONE状態にする */
	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * Start失敗(状態エラー)
 */
TEST(VhalSeqTestMovieFail, StartFrontMovie1)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	bool oneshot{};

	cockpit::hal::video_hal::Init(obj);
	RequestClear();		/* VIDEO_HAL_MOVIE_STS_NONE状態にする */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "movie.front.status.result", CallbackMovie));

	/* 再生開始 */
	status_movie = VIDEO_HAL_MOVIE_STS_NONE;	/* ← fail */
	status_movie_expect = VIDEO_HAL_MOVIE_STS_PLAYING;
	oneshot = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.start", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PROP_UPDATE, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for start.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
	EXPECT_EQ(false, intime);

	/* 終了処理 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "movie.front.status.result", CallbackMovie));
	RequestClear();		/* VIDEO_HAL_MOVIE_STS_NONE状態にする */
	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * Start失敗(再生ファイル無し)
 */
TEST(VhalSeqTestMovieFail, StartFrontMovie2)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	std::string path{};
	uint32_t x{};
	uint32_t y{};
	uint32_t width{};
	uint32_t height{};
	bool oneshot{};

	cockpit::hal::video_hal::Init(obj);
	RequestClear();		/* VIDEO_HAL_MOVIE_STS_NONE状態にする */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "movie.front.status.result", CallbackMovie));

	/* 動画再生準備 */
	status_movie = VIDEO_HAL_MOVIE_STS_NONE;
	status_movie_expect = VIDEO_HAL_MOVIE_STS_READY;
	path = "/sys/none/movie_not_found.mp4";	/* ← fail */
	x = 0;
	y = 0;
	width = 1280;
	height = 720;
	oneshot = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.file.path", &path, path.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.x", &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.y", &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.prepare", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for prepare.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
	EXPECT_EQ(true, intime);

	/* 再生 */
	status_movie = VIDEO_HAL_MOVIE_STS_READY;
	status_movie_expect = VIDEO_HAL_MOVIE_STS_FAILED;
	oneshot = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.start", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for start.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
	EXPECT_EQ(true, intime);

	/* 終了処理 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "movie.front.status.result", CallbackMovie));
	RequestClear();
	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * Start失敗(壊れファイル再生)
 */
TEST(VhalSeqTestMovieFail, StartFrontMovie3)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	std::string path{};
	uint32_t x{};
	uint32_t y{};
	uint32_t width{};
	uint32_t height{};
	bool oneshot{};

	path = GetMovieFileName();
	path.append("broken.mp4");
	if (!std::filesystem::exists(path)) {
		PrintMsg("skiped VhalSeqTestMovieFail/StartFrontMovie3");
		return;
	}

	cockpit::hal::video_hal::Init(obj);
	RequestClear();		/* VIDEO_HAL_MOVIE_STS_NONE状態にする */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "movie.front.status.result", CallbackMovie));

	/* 動画再生準備 */
	status_movie = VIDEO_HAL_MOVIE_STS_NONE;
	status_movie_expect = VIDEO_HAL_MOVIE_STS_READY;
	x = 0;
	y = 0;
	width = 1280;
	height = 720;
	oneshot = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.file.path", &path, path.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.x", &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.y", &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.prepare", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for prepare.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
	EXPECT_EQ(true, intime);

	/* 再生 */
	status_movie = VIDEO_HAL_MOVIE_STS_READY;
	status_movie_expect = VIDEO_HAL_MOVIE_STS_PLAYING;
	oneshot = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.start", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for start.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
	EXPECT_EQ(true, intime);

	/* 中断待ち */
	status_movie = VIDEO_HAL_MOVIE_STS_PLAYING;
	status_movie_expect = VIDEO_HAL_MOVIE_STS_FINISHED;
	PrintMsg("wait_for finish.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds+10), [] { return (status_movie_expect == status_movie); });
	EXPECT_EQ(true, intime);

	/* 結果出力 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status_movie, sizeof(status_movie), VIDEO_HAL_DATA_TYPE_NUM));
	std::string str_status("movie-status is ");
	str_status.append(std::to_string(status_movie));
	PrintMsg(str_status);

	/* 終了処理 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "movie.front.status.result", CallbackMovie));
	RequestClear();		/* VIDEO_HAL_MOVIE_STS_NONE状態にする */
	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * Cancel失敗(状態エラー)
 */
TEST(VhalSeqTestMovieFail, CancelFrontMovie)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	cockpit::hal::video_hal::Init(obj);
	RequestClear();		/* VIDEO_HAL_MOVIE_STS_NONE状態にする */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "movie.front.status.result", CallbackMovie));

	/* 再生キャンセル */
	EXPECT_EQ(VHAL_TEST_SUCCESS, Cancel(obj));

	/* 終了処理 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "movie.front.status.result", CallbackMovie));
	RequestClear();		/* VIDEO_HAL_MOVIE_STS_NONE状態にする */
	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * Clear失敗(状態エラー)
 */
TEST(VhalSeqTestMovieFail, ClearFrontMovie)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	cockpit::hal::video_hal::Init(obj);
	Clear(obj);		/* VIDEO_HAL_MOVIE_STS_NONE状態にする (ここでも失敗する可能性は高い) */

	/* 終了処理 */
	Clear(obj);		/* ← 確実にfail */
	cockpit::hal::video_hal::Deinit(obj);
}
