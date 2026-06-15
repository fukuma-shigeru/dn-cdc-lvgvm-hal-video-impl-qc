#include "vhal_test_common.h"

#define DEBUG_STATUS_MESSAGE(status) (status==VIDEO_HAL_MOVIE_STS_NONE?   "NONE":\
							status==VIDEO_HAL_MOVIE_STS_READY?    "READY":\
							status==VIDEO_HAL_MOVIE_STS_PLAYING?  "PLAYING":\
							status==VIDEO_HAL_MOVIE_STS_FINISHED? "FINISHED":\
							status==VIDEO_HAL_MOVIE_STS_CANCELED? "CANCELED":\
							status==VIDEO_HAL_MOVIE_STS_FAILED?   "FAILED":\
																		"UNKNOWN")

static cockpit::hal::video_hal::CtlObj obj {0};
static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static int32_t status_movie;
static int32_t status_movie_expect;
static const int timeout_seconds = CB_TIMEOUT_SECOUNDS;
static int32_t current_movie_status = VIDEO_HAL_MOVIE_STS_NONE;

static void PrintStatus(const int32_t base_status, const int32_t current_status)
{
	std::string before{DEBUG_STATUS_MESSAGE(base_status)};
	std::string current{DEBUG_STATUS_MESSAGE(current_status)};

	PrintMsg(before + "->" + current);
}

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

static int32_t RequestPrepare()
{
	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);

	uint32_t x{0};
	uint32_t y{0};
	uint32_t width{prop.screenWidth};
	uint32_t height{prop.screenHeight};
	bool oneshot{true};
	std::string path{GetMovieFileName()};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.file.path", &path, path.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.x", &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.y", &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.prepare", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	int32_t ret = cockpit::hal::video_hal::RequestUpdate(obj);

	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	PrintMsg("return:" + std::to_string(ret));

	return ret;
}

static int32_t RequestPrepareInvalidParameter()
{
	uint32_t x{0};
	uint32_t y{0};
	uint32_t width{0};
	uint32_t height{0};
	bool oneshot{true};
	std::string path{GetMovieFileName()};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.file.path", &path, path.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.x", &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.y", &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.prepare", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	int32_t ret = cockpit::hal::video_hal::RequestUpdate(obj);

	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	PrintMsg("return:" + std::to_string(ret));

	return ret;
}

static int32_t RequestStart()
{
	bool oneshot{true};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.start", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	int32_t ret = cockpit::hal::video_hal::RequestUpdate(obj);

	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	PrintMsg("return:" + std::to_string(ret));

	return ret;
}

static int32_t RequestCancel()
{
	bool oneshot{true};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.cancel", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	int32_t ret = cockpit::hal::video_hal::RequestUpdate(obj);

	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	PrintMsg("return:" + std::to_string(ret));

	return ret;
}

static int32_t RequestClear()
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
	return ret;
}

/**
 * @brief 動画再生状態をデフォルトに戻す
 * @param[in] movie_status VideoHAL動画再生状態
 */
static void ChangeMovieStatusNone()
{
	int32_t status{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	switch(status)
	{
		case VIDEO_HAL_MOVIE_STS_NONE:
		case VIDEO_HAL_MOVIE_STS_READY:
		case VIDEO_HAL_MOVIE_STS_FINISHED:
		case VIDEO_HAL_MOVIE_STS_CANCELED:
		case VIDEO_HAL_MOVIE_STS_FAILED:
			RequestClear();
			break;
		case VIDEO_HAL_MOVIE_STS_PLAYING:
			RequestCancel();
			RequestClear();
			break;
	}
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
}

static void ChangeMovieStatusReady(void)
{
	RequestPrepare();
}

static void ChangeMovieStatusPlaying(void)
{
	RequestPrepare();
	RequestStart();
}

static void ChangeMovieStatusCanceled(void)
{
	RequestPrepare();
	RequestStart();
	RequestCancel();
}

static void ChangeMovieStatusFailed(void)
{
	uint32_t x{0};
	uint32_t y{0};
	bool oneshot{true};
	std::string path{TEST_MOVIE_NONEXISTANT_FILE};	/* 存在しないファイルを指定することで失敗させる。 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.file.path", &path, path.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.x", &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.y", &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM));
	struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.width", &prop.screenWidth, sizeof(prop.screenWidth), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.height", &prop.screenHeight, sizeof(prop.screenHeight), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.prepare", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));

	RequestStart();

}

/**
 * @brief 完了状態にする 
 */
static void ChangeMovieStatusFinished(void) 
{
	RequestPrepare();
	RequestStart();

	/* 再生完了まで待つ */
	PrintMsg("wait_for finish.");
	std::this_thread::sleep_for(std::chrono::seconds(TEST_MOVIE_PLAYTIME_SECOND));
}

/**
 * @brief 指定の動画再生状態へ変更する
 * @param[in] request_status 変更先の動画再生状態 
 */
static void ChangeMovieStatus(int32_t request_status)
{
	std::string request{DEBUG_STATUS_MESSAGE(request_status)};
	std::string current{};

	PrintMsg("request:" + request);

	SCOPED_TRACE("test preparation.");
	/* いったんデフォルトに戻す */
	ChangeMovieStatusNone();

	switch(request_status)
	{
		case VIDEO_HAL_MOVIE_STS_NONE:
			break;
		case VIDEO_HAL_MOVIE_STS_READY:
			ChangeMovieStatusReady();
			break;
		case VIDEO_HAL_MOVIE_STS_PLAYING:
			ChangeMovieStatusPlaying();
			break;
		case VIDEO_HAL_MOVIE_STS_FINISHED:
			ChangeMovieStatusFinished();
			break;
		case VIDEO_HAL_MOVIE_STS_CANCELED:
			ChangeMovieStatusCanceled();
			break;
		case VIDEO_HAL_MOVIE_STS_FAILED:
			ChangeMovieStatusFailed();
			break;
	}
	/**
	 * ・CANCELEDの反映にラグあり。
	 * ・開始要求後、FAILEDに遷移するときPLAYINGを経由しており、即座に取得するとPLAYINGが取れてしまう。
	 *   FAILEDに遷移するまで適当な時間待つ。
	 */
	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	/* 確認 */
	int32_t status{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(request_status, status);
	current = DEBUG_STATUS_MESSAGE(status);
	PrintMsg("current:" + current);
}

struct VhalApiTestMovieParameter {
	int32_t surface_id;
	std::string front_vpath_name;
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
};

class VhalApiTestMovieNormal : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestMovieParameter> {};
class VhalApiTestMovieRange : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestMovieParameter> {};
class VhalApiTestMovieRepeat : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestMovieParameter> {};
class VhalApiTestMovieAbnormalArg : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestMovieParameter> {};
class VhalApiTestMovieAbnormalArgInvalidPath : public VhalApiTest, public ::testing::WithParamInterface<VhalApiTestMovieParameter> {};


std::vector<VhalApiTestMovieParameter> GenerateVhalApiTestMovieParameter(void)
{
	std::vector<VhalApiTestMovieParameter> v = {
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 0, 0, TEST_MOVIE_FILE_WIDTH, TEST_MOVIE_FILE_HEIGHT}
	};

	return v;
}

std::vector<VhalApiTestMovieParameter> GenerateVhalApiTestMovieRangeParameter(void)
{
    struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);

	std::vector<VhalApiTestMovieParameter> v = {
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 0, 100, 100, 100},
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, prop.screenWidth - 1, 100, 1, 100},
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 100, 0, 100, 100},
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 100, prop.screenHeight - 1, 100, 1},
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 100, 100, 1, 100},
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 0, 100, prop.screenWidth, 100},
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 100, 100, 100, 1},
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 100, 0, 100, prop.screenHeight}
	};

	return v;	
}

std::vector<VhalApiTestMovieParameter> GenerateVhalApiTestMovieAbnormalArgParameter(void)
{
    struct ilmScreenProperties prop{};
    GetScreenProperties(SCREEN_ID_FRONT, &prop);

	std::vector<VhalApiTestMovieParameter> v = {
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, prop.screenWidth + 1, 100, 100, 100},
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 0, prop.screenHeight + 1, 100, 100},
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 0, 0, prop.screenWidth + 1, 100},
		{SURFACE_ID_FRONT_DTV, VPATH_NAME_DTV, 0, 0, 100, prop.screenHeight + 1}
	};

	return v;	
}

INSTANTIATE_TEST_CASE_P(VhalApiTestMovie, VhalApiTestMovieNormal, ::testing::ValuesIn(GenerateVhalApiTestMovieParameter()));
INSTANTIATE_TEST_CASE_P(VhalApiTestMovie, VhalApiTestMovieRange, ::testing::ValuesIn(GenerateVhalApiTestMovieRangeParameter()));
INSTANTIATE_TEST_CASE_P(VhalApiTestMovie, VhalApiTestMovieRepeat, ::testing::ValuesIn(GenerateVhalApiTestMovieParameter()));
INSTANTIATE_TEST_CASE_P(VhalApiTestMovie, VhalApiTestMovieAbnormalArg, ::testing::ValuesIn(GenerateVhalApiTestMovieAbnormalArgParameter()));
INSTANTIATE_TEST_CASE_P(VhalApiTestMovie, VhalApiTestMovieAbnormalArgInvalidPath, ::testing::ValuesIn(GenerateVhalApiTestMovieParameter()));

/**
 * 基本動作 
 * 最後まで再生
 */
TEST_P(VhalApiTestMovieNormal, NormalPlayMovieCompleted)
{
	VhalApiTestMovieParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	std::string path{};
	uint32_t x{};
	uint32_t y{};
	uint32_t width{};
	uint32_t height{};
	bool oneshot{};

	cockpit::hal::video_hal::Init(obj);

	ChangeMovieStatus(VIDEO_HAL_MOVIE_STS_NONE);
	
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "movie.front.status.result", CallbackMovie));
	
	/* 動画再生 */
	status_movie = VIDEO_HAL_MOVIE_STS_NONE;
	status_movie_expect = VIDEO_HAL_MOVIE_STS_READY;
	path = TEST_MOVIE_FILE_NAME;
	x = param.x;
	y = param.y;
	width = param.width;
	height = param.height;
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

	std::string control_path{};
	uint32_t control_x{};
	uint32_t control_y{};
	uint32_t control_width{};
	uint32_t control_height{};
	bool control_prepare{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.file.path", &control_path, GETVALSIZE_MAX, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(path, control_path);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.output.x", &control_x, sizeof(control_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(x, control_x);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.output.y", &control_y, sizeof(control_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(y, control_y);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.output.width", &control_width, sizeof(control_width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(width, control_width);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.output.height", &control_height, sizeof(control_height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(height, control_height);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.prepare", &control_prepare, sizeof(control_prepare), VIDEO_HAL_DATA_TYPE_BOOL));

	/* 再生 */
	status_movie = VIDEO_HAL_MOVIE_STS_READY;
	status_movie_expect = VIDEO_HAL_MOVIE_STS_PLAYING;
	oneshot = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.start", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for start.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
	EXPECT_EQ(true, intime);

	bool control_start{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.start", &control_start, sizeof(control_start), VIDEO_HAL_DATA_TYPE_BOOL));

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

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

	bool control_clear{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.clear", &control_clear, sizeof(control_clear), VIDEO_HAL_DATA_TYPE_BOOL));

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "movie.front.status.result", CallbackMovie));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * 再生途中で中止
 */
TEST_P(VhalApiTestMovieNormal, NormalPlayMovieCancel)
{
	VhalApiTestMovieParameter param = GetParam();
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
	x = param.x;
	y = param.y;
	width = param.width;
	height = param.height;
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

	std::string control_path{};
	uint32_t control_x{};
	uint32_t control_y{};
	uint32_t control_width{};
	uint32_t control_height{};
	bool control_prepare{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.file.path", &control_path, GETVALSIZE_MAX, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(path, control_path);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.output.x", &control_x, sizeof(control_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(x, control_x);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.output.y", &control_y, sizeof(control_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(y, control_y);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.output.width", &control_width, sizeof(control_width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(width, control_width);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.output.height", &control_height, sizeof(control_height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(height, control_height);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.prepare", &control_prepare, sizeof(control_prepare), VIDEO_HAL_DATA_TYPE_BOOL));


	/* 再生 */
	status_movie = VIDEO_HAL_MOVIE_STS_READY;
	status_movie_expect = VIDEO_HAL_MOVIE_STS_PLAYING;
	oneshot = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.start", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for start.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
	EXPECT_EQ(true, intime);

	bool control_start{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.start", &control_start, sizeof(control_start), VIDEO_HAL_DATA_TYPE_BOOL));


	std::this_thread::sleep_for(std::chrono::milliseconds(300));

	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	std::this_thread::sleep_for(std::chrono::milliseconds(300));

	/* 再生中止 */
	status_movie = VIDEO_HAL_MOVIE_STS_PLAYING;
	status_movie_expect = VIDEO_HAL_MOVIE_STS_CANCELED;
	oneshot = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.cancel", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for cancel.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
	EXPECT_EQ(true, intime);

	bool control_cancel{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.cancel", &control_cancel, sizeof(control_cancel), VIDEO_HAL_DATA_TYPE_BOOL));

	/* クリア */
	status_movie = VIDEO_HAL_MOVIE_STS_CANCELED;
	status_movie_expect = VIDEO_HAL_MOVIE_STS_NONE;
	oneshot = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.clear", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for clear.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
	EXPECT_EQ(true, intime);

	bool control_clear{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.clear", &control_clear, sizeof(control_clear), VIDEO_HAL_DATA_TYPE_BOOL));


	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "movie.front.status.result", CallbackMovie));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 基本動作
 * one-shot-eventがfalse
 */
TEST_P(VhalApiTestMovieNormal, NormalOnshotFalse)
{
	VhalApiTestMovieParameter param = GetParam();
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
	x = param.x;
	y = param.y;
	width = param.width;
	height = param.height;
	oneshot = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.file.path", &path, path.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.x", &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.y", &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.prepare", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for prepare.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
	EXPECT_EQ(false, intime);

	std::string control_path{};
	uint32_t control_x{};
	uint32_t control_y{};
	uint32_t control_width{};
	uint32_t control_height{};
	bool control_prepare{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.file.path", &control_path, GETVALSIZE_MAX, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(path, control_path);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.output.x", &control_x, sizeof(control_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(x, control_x);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.output.y", &control_y, sizeof(control_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(y, control_y);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.output.width", &control_width, sizeof(control_width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(width, control_width);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.output.height", &control_height, sizeof(control_height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(height, control_height);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.prepare", &control_prepare, sizeof(control_prepare), VIDEO_HAL_DATA_TYPE_BOOL));


	/* 再生 */
	status_movie = VIDEO_HAL_MOVIE_STS_READY;
	status_movie_expect = VIDEO_HAL_MOVIE_STS_PLAYING;
	oneshot = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.start", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for start.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
	EXPECT_EQ(false, intime);

	bool control_start{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.start", &control_start, sizeof(control_start), VIDEO_HAL_DATA_TYPE_BOOL));


	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	/* 再生中止 */
	status_movie = VIDEO_HAL_MOVIE_STS_PLAYING;
	status_movie_expect = VIDEO_HAL_MOVIE_STS_CANCELED;
	oneshot = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.cancel", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for cancel.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
	EXPECT_EQ(false, intime);

	bool control_cancel{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.cancel", &control_cancel, sizeof(control_cancel), VIDEO_HAL_DATA_TYPE_BOOL));


	/* クリア */
	status_movie = VIDEO_HAL_MOVIE_STS_CANCELED;
	status_movie_expect = VIDEO_HAL_MOVIE_STS_NONE;
	oneshot = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.clear", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for clear.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
	EXPECT_EQ(false, intime);

	bool control_clear{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.clear", &control_clear, sizeof(control_clear), VIDEO_HAL_DATA_TYPE_BOOL));


	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "movie.front.status.result", CallbackMovie));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ上限下限
 * 再生途中で中止
 */
TEST_P(VhalApiTestMovieRange, RangePlayMovieCancel)
{
	VhalApiTestMovieParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	std::string path{};
	uint32_t x{};
	uint32_t y{};
	uint32_t width{};
	uint32_t height{};
	bool oneshot{};

	PrintMsg("x=" + std::to_string(param.x) + " y=" + std::to_string(param.y) + " width=" + std::to_string(param.width) + " height=" + std::to_string(param.height));

	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "movie.front.status.result", CallbackMovie));

	/* 汎用動画再生 */
	status_movie = VIDEO_HAL_MOVIE_STS_NONE;
	status_movie_expect = VIDEO_HAL_MOVIE_STS_READY;
	path = GetMovieFileName();
	x = param.x;
	y = param.y;
	width = param.width;
	height = param.height;
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

	std::string control_path{};
	uint32_t control_x{};
	uint32_t control_y{};
	uint32_t control_width{};
	uint32_t control_height{};
	bool control_prepare{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.file.path", &control_path, GETVALSIZE_MAX, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(path, control_path);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.output.x", &control_x, sizeof(control_x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(x, control_x);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.output.y", &control_y, sizeof(control_y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(y, control_y);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.output.width", &control_width, sizeof(control_width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(width, control_width);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.output.height", &control_height, sizeof(control_height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(height, control_height);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.prepare", &control_prepare, sizeof(control_prepare), VIDEO_HAL_DATA_TYPE_BOOL));


	/* 再生 */
	status_movie = VIDEO_HAL_MOVIE_STS_READY;
	status_movie_expect = VIDEO_HAL_MOVIE_STS_PLAYING;
	oneshot = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.start", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for start.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
	EXPECT_EQ(true, intime);

	bool control_start{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.start", &control_start, sizeof(control_start), VIDEO_HAL_DATA_TYPE_BOOL));


	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	EXPECT_EQ(VHAL_TEST_SUCCESS, ScreenShot());

	std::this_thread::sleep_for(std::chrono::milliseconds(200));

	/* 再生中止 */
	status_movie = VIDEO_HAL_MOVIE_STS_PLAYING;
	status_movie_expect = VIDEO_HAL_MOVIE_STS_CANCELED;
	oneshot = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.cancel", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for cancel.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
	EXPECT_EQ(true, intime);

	bool control_cancel{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.cancel", &control_cancel, sizeof(control_cancel), VIDEO_HAL_DATA_TYPE_BOOL));


	/* クリア */
	status_movie = VIDEO_HAL_MOVIE_STS_CANCELED;
	status_movie_expect = VIDEO_HAL_MOVIE_STS_NONE;
	oneshot = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.clear", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
	PrintMsg("wait_for clear.");
	intime = cond_sync.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] { return (status_movie_expect == status_movie); });
	EXPECT_EQ(true, intime);

	bool control_clear{};
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.control.clear", &control_clear, sizeof(control_clear), VIDEO_HAL_DATA_TYPE_BOOL));


	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "movie.front.status.result", CallbackMovie));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 設定前の参照
 */
TEST(VhalApiTestMovieNotSet, NotSet)
{
	cockpit::hal::video_hal::Init(obj);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status_movie, sizeof(status_movie), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_MOVIE_STS_NONE, status_movie);	

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 短時間の連続コール 
 * 再生途中で中止
 */
TEST_P(VhalApiTestMovieRepeat, Repeat)
{
	VhalApiTestMovieParameter param = GetParam();
	std::string path{};
	bool oneshot{};
	int32_t	excepted{VIDEO_HAL_API_SUCCESS};	// BEVstep3 (初回は成功)

	cockpit::hal::video_hal::Init(obj);

	/* 準備 */
	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		path = GetMovieFileName();
		oneshot = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.file.path", &path, path.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.x", &param.x, sizeof(param.x), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.y", &param.y, sizeof(param.y), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.width", &param.width, sizeof(param.width), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.height", &param.height, sizeof(param.height), VIDEO_HAL_DATA_TYPE_NUM));
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.prepare", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(excepted, cockpit::hal::video_hal::RequestUpdate(obj));
		excepted = VIDEO_HAL_API_ERR_PROP_UPDATE;	// BEVstep3 (2回目以降はエラー)
	}

	/* 再生 */
	excepted = VIDEO_HAL_API_SUCCESS;	// BEVstep3 (初回は成功)
	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		oneshot = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.start", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(excepted, cockpit::hal::video_hal::RequestUpdate(obj));
		excepted = VIDEO_HAL_API_ERR_PROP_UPDATE;	// BEVstep3 (2回目以降はエラー)
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	KillAllRenderImage(obj);	// BEVstep3 add

	/* 再生中止 */
	excepted = VIDEO_HAL_API_SUCCESS;	// BEVstep3 (初回は成功)
	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		oneshot = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.cancel", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(excepted, cockpit::hal::video_hal::RequestUpdate(obj));
		excepted = VIDEO_HAL_API_ERR_PROP_UPDATE;	// BEVstep3 (2回目以降はエラー)
	}

	/* クリア */
	excepted = VIDEO_HAL_API_SUCCESS;	// BEVstep3 (初回は成功)
	for (size_t i = 0; i < TEST_REPEAT_COUNT; i++)
	{
		oneshot = true;
		EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.clear", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
		EXPECT_EQ(excepted, cockpit::hal::video_hal::RequestUpdate(obj));
		excepted = VIDEO_HAL_API_ERR_PROP_UPDATE;	// BEVstep3 (2回目以降はエラー)
	}

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 状態/イベントマトリクス網羅
 * 動画再生機能未使用(None)
 */
TEST(VhalApiTestMovieStateNone, State)
{
	SCOPED_TRACE("test body.");
	const int32_t base_status = VIDEO_HAL_MOVIE_STS_NONE;
	int32_t status{};

	cockpit::hal::video_hal::Init(obj);

	PrintMsg(DEBUG_STATUS_MESSAGE(base_status));

	ChangeMovieStatus(base_status);

	/* 無効 */
	EXPECT_EQ(VIDEO_HAL_API_ERR_PROP_UPDATE, RequestStart());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(base_status, status);
	PrintStatus(base_status, status);

	EXPECT_EQ(VIDEO_HAL_API_ERR_PROP_UPDATE, RequestCancel());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(base_status, status);
	PrintStatus(base_status, status);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, RequestClear());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(base_status, status);
	PrintStatus(base_status, status);

	/* 有効[正常] */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, RequestPrepare());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_MOVIE_STS_READY, status);
	PrintStatus(base_status, status);

	ChangeMovieStatus(VIDEO_HAL_MOVIE_STS_NONE);

	/* 有効[異常] */
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, RequestPrepareInvalidParameter());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(base_status, status);
	PrintStatus(base_status, status);

	ChangeMovieStatus(VIDEO_HAL_MOVIE_STS_NONE);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 状態/イベントマトリクス網羅
 * 動画再生を開始できる(Ready)
 */
TEST(VhalApiTestMovieStateReady, State)
{
	SCOPED_TRACE("test body.");
	const int32_t base_status = VIDEO_HAL_MOVIE_STS_READY;
	int32_t status{};

	cockpit::hal::video_hal::Init(obj);

	PrintMsg(DEBUG_STATUS_MESSAGE(base_status));

	ChangeMovieStatus(base_status);

	/* 無効 */
	EXPECT_EQ(VIDEO_HAL_API_ERR_PROP_UPDATE, RequestPrepare());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(base_status, status);
	PrintStatus(base_status, status);

	EXPECT_EQ(VIDEO_HAL_API_ERR_PROP_UPDATE, RequestCancel());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(base_status, status);
	PrintStatus(base_status, status);

	/* 有効 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, RequestStart());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_MOVIE_STS_PLAYING, status);
	PrintStatus(base_status, status);
	/* 再生完了まで待つ */
	PrintMsg("wait_for finish.");
	std::this_thread::sleep_for(std::chrono::seconds(TEST_MOVIE_PLAYTIME_SECOND));

	ChangeMovieStatus(base_status);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, RequestClear());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_MOVIE_STS_NONE, status);
	PrintStatus(base_status, status);

	ChangeMovieStatus(VIDEO_HAL_MOVIE_STS_NONE);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 状態/イベントマトリクス網羅
 * 動画再生中(Playing)
 */
TEST(VhalApiTestMovieStatePlaying, State)
{
	SCOPED_TRACE("test body.");
	const int32_t base_status = VIDEO_HAL_MOVIE_STS_PLAYING;
	int32_t status{};

	cockpit::hal::video_hal::Init(obj);

	PrintMsg(DEBUG_STATUS_MESSAGE(base_status));

	ChangeMovieStatus(base_status);

	/* 無効 */
	EXPECT_EQ(VIDEO_HAL_API_ERR_PROP_UPDATE, RequestPrepare());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(base_status, status);
	PrintStatus(base_status, status);

	EXPECT_EQ(VIDEO_HAL_API_ERR_PROP_UPDATE, RequestStart());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(base_status, status);
	PrintStatus(base_status, status);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, RequestClear());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_MOVIE_STS_NONE, status);
	PrintStatus(base_status, status);

	/* 再生完了まで待つ */
	PrintMsg("wait_for finish.");
	std::this_thread::sleep_for(std::chrono::seconds(TEST_MOVIE_PLAYTIME_SECOND));

	ChangeMovieStatus(base_status);

	/* 有効 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, RequestCancel());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_MOVIE_STS_CANCELED, status);
	PrintStatus(base_status, status);

	/* 状態のみ確認 */
	ChangeMovieStatus(VIDEO_HAL_MOVIE_STS_FAILED);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_MOVIE_STS_FAILED, status);
	PrintStatus(base_status, status);

	ChangeMovieStatus(VIDEO_HAL_MOVIE_STS_FINISHED);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_MOVIE_STS_FINISHED, status);
	PrintStatus(base_status, status);

	ChangeMovieStatus(VIDEO_HAL_MOVIE_STS_NONE);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 状態/イベントマトリクス網羅
 * 動画再生完了(Finished)
 */
TEST(VhalApiTestMovieStateFinished, State)
{
	SCOPED_TRACE("test body.");
	const int32_t base_status = VIDEO_HAL_MOVIE_STS_FINISHED;
	int32_t status{};

	cockpit::hal::video_hal::Init(obj);

	PrintMsg(DEBUG_STATUS_MESSAGE(base_status));

	ChangeMovieStatus(base_status);

	/* 無効 */
	EXPECT_EQ(VIDEO_HAL_API_ERR_PROP_UPDATE, RequestPrepare());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(base_status, status);
	PrintStatus(base_status, status);

	EXPECT_EQ(VIDEO_HAL_API_ERR_PROP_UPDATE, RequestStart());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(base_status, status);
	PrintStatus(base_status, status);

	EXPECT_EQ(VIDEO_HAL_API_ERR_PROP_UPDATE, RequestCancel());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(base_status, status);
	PrintStatus(base_status, status);

	/* 有効 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, RequestClear());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_MOVIE_STS_NONE, status);
	PrintStatus(base_status, status);

	ChangeMovieStatus(VIDEO_HAL_MOVIE_STS_NONE);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 状態/イベントマトリクス網羅
 * キャンセルされた(Canceled)
 */
TEST(VhalApiTestMovieStateCanceled, State)
{
	SCOPED_TRACE("test body.");
	const int32_t base_status = VIDEO_HAL_MOVIE_STS_CANCELED;
	int32_t status{};

	cockpit::hal::video_hal::Init(obj);

	PrintMsg(DEBUG_STATUS_MESSAGE(base_status));

	ChangeMovieStatus(base_status);

	/* 無効 */
	EXPECT_EQ(VIDEO_HAL_API_ERR_PROP_UPDATE, RequestPrepare());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(base_status, status);
	PrintStatus(base_status, status);

	EXPECT_EQ(VIDEO_HAL_API_ERR_PROP_UPDATE, RequestStart());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(base_status, status);
	PrintStatus(base_status, status);

	EXPECT_EQ(VIDEO_HAL_API_ERR_PROP_UPDATE, RequestCancel());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(base_status, status);
	PrintStatus(base_status, status);

	/* 有効 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, RequestClear());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_MOVIE_STS_NONE, status);
	PrintStatus(base_status, status);

	ChangeMovieStatus(VIDEO_HAL_MOVIE_STS_NONE);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * 状態/イベントマトリクス網羅
 * 再生中エラー(Failed)
 */
TEST(VhalApiTestMovieStateFailed, State)
{
	SCOPED_TRACE("test body.");
	const int32_t base_status = VIDEO_HAL_MOVIE_STS_FAILED;
	int32_t status{};

	cockpit::hal::video_hal::Init(obj);

	PrintMsg(DEBUG_STATUS_MESSAGE(base_status));

	ChangeMovieStatus(base_status);

	/* 無効 */
	EXPECT_EQ(VIDEO_HAL_API_ERR_PROP_UPDATE, RequestPrepare());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(base_status, status);
	PrintStatus(base_status, status);

	EXPECT_EQ(VIDEO_HAL_API_ERR_PROP_UPDATE, RequestStart());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(base_status, status);
	PrintStatus(base_status, status);

	EXPECT_EQ(VIDEO_HAL_API_ERR_PROP_UPDATE, RequestCancel());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(base_status, status);
	PrintStatus(base_status, status);

	/* 有効 */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, RequestClear());
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::GetValue(obj, "movie.front.status.result", &status, sizeof(status), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_MOVIE_STS_NONE, status);
	PrintStatus(base_status, status);

	ChangeMovieStatus(VIDEO_HAL_MOVIE_STS_NONE);

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ異常
 */
TEST_P(VhalApiTestMovieAbnormalArg, AbnormalArg)
{
	VhalApiTestMovieParameter param = GetParam();
	std::string path{};
	bool oneshot{};

	cockpit::hal::video_hal::Init(obj);

	ChangeMovieStatus(VIDEO_HAL_MOVIE_STS_NONE);

	/* 動画再生 */
	path = GetMovieFileName();
	oneshot = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.file.path", &path, path.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.x", &param.x, sizeof(param.x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.y", &param.y, sizeof(param.y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.width", &param.width, sizeof(param.width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.height", &param.height, sizeof(param.height), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.prepare", &oneshot, sizeof(oneshot), VIDEO_HAL_DATA_TYPE_BOOL));
	EXPECT_EQ(VIDEO_HAL_API_ERR_PARAM, cockpit::hal::video_hal::RequestUpdate(obj));

	cockpit::hal::video_hal::Deinit(obj);
}

/**
 * パラメータ異常
 * 存在しないファイルパス
 */
TEST_P(VhalApiTestMovieAbnormalArgInvalidPath, AbnormalArgInvalidPath)
{
	VhalApiTestMovieParameter param = GetParam();
	std::unique_lock<std::mutex> lock_sync(mtx_sync);
	bool intime{false};
	std::string path{};
	bool oneshot{};

	cockpit::hal::video_hal::Init(obj);

	ChangeMovieStatus(VIDEO_HAL_MOVIE_STS_NONE);
	
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "movie.front.status.result", CallbackMovie));
	
	/* 動画再生 */
	status_movie = VIDEO_HAL_MOVIE_STS_NONE;
	status_movie_expect = VIDEO_HAL_MOVIE_STS_READY;
	path = "/tmp/invalid_path";
	oneshot = true;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.file.path", &path, path.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.x", &param.x, sizeof(param.x), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.y", &param.y, sizeof(param.y), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.width", &param.width, sizeof(param.width), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "movie.front.control.output.height", &param.height, sizeof(param.height), VIDEO_HAL_DATA_TYPE_NUM));
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

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "movie.front.status.result", CallbackMovie));

	cockpit::hal::video_hal::Deinit(obj);
}
