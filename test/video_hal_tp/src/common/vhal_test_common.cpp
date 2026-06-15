#include "vhal_test_common.h"
#include <stdarg.h>
#include <pwd.h>
#include <array>
#include <fstream>
#include <iostream>

#include <dlt/dlt.h>

extern "C"
{
	#include <sys_db_api_public.h>
	#include <sys_db_cnv_value_public.h>
	#include <sys_ev_mng_api_public.h>
	#include <utl_pkg_com_api_public.h>
}

namespace {
	const std::string brysh_command_base{"/opt/arene/test/brysh"}; 
}

/**
 * @brief 画像ファイル情報 
 */
struct ImageFile
{
	std::string name;		/* ファイル名 */
	uint32_t width;			/* 幅 */
	uint32_t height;		/* 高さ */
};

static const ImageFile test_image[] = {
								{TEST_ANDROIDAUTO_IMAGE, 1280, 720},
								{TEST_CARPLAY_IMAGE,     1280, 720},
								{TEST_DRC_IMAGE,         1280, 720},
								{TEST_DTV_IMAGE,         1280, 720},
								{TEST_MEDIAPLAYER_IMAGE, 1280, 720},
								{TEST_MAP_IMAGE,         1280, 720},
								{TEST_WIDE_IMAGE,        800,  600},
								{TEST_CLIPPING_IMAGE,    800,  600},
								{TEST_MULTISENSORY_IMAGE,1280, 720},
								{TEST_FRAGRANCE_IMAGE,   1280, 720},
								{TEST_HUD_IMAGE,         448, 224}
};

static const std::map<uint32_t, ImageFile> test_image_table = {
								{SURFACE_ID_ANDROIDAUTO,       test_image[0]},
								{SURFACE_ID_IC_ANDROIDAUTO,    test_image[0]},
								{SURFACE_ID_CARPLAY,           test_image[1]},
								{SURFACE_ID_IC_CARPLAY,        test_image[1]},
								{SURFACE_ID_DRIVE_RECODER,     test_image[2]},
								{SURFACE_ID_FRONT_DTV,         test_image[3]},
								{SURFACE_ID_REAR_DTV,          test_image[3]},
								{SURFACE_ID_MEDIA_PLAYER,      test_image[4]},
								{SURFACE_ID_IC_MAP,            test_image[5]},
								{SURFACE_ID_MULTISENSORY,      test_image[8]},
								{SURFACE_ID_REAR_MULTISENSORY, test_image[8]},
								{SURFACE_ID_FRAGRANCE,         test_image[9]},
								{SURFACE_ID_HUD_HUD,           test_image[10]}
};

static std::mutex mtx_sync;
static std::condition_variable cond_sync;
static bool ilm_initilized;
static bool available_called{false};
static bool callback_called{false};

static void CallbackAvailable(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		available_called = true;
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

static void Callback(void)
{
	PrintMsg("start.");
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync);
		callback_called = true;
	}
	cond_sync.notify_one();
	PrintMsg("end.");
}

/**
 * @brief メッセージ出力
 * 
 * 主にテストコード自体のデバッグに使用。
 */
void PrintMsg(std::string msg, const std::experimental::source_location location)
{
	struct timespec	ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += (9 * 3600);	/* 本来ならシステム情報を用いて+9時間を加算するべきだが高速化のため簡略 */
	ts.tv_sec %= (24 * 3600);	/* 時分秒しか使わないため年月日を削除 */
	char	tmbuf[64];
	snprintf(tmbuf, sizeof(tmbuf), "%02d:%02d:%02d.%06d ", (int)(ts.tv_sec / 3600), (int)((ts.tv_sec % 3600) / 60), (int)(ts.tv_sec % 60), (int)(ts.tv_nsec / 1000));

	std::string absolute_path = location.file_name();
	std::size_t dir_pos = absolute_path.find_last_of("/");
	std::string file_name = absolute_path.substr(dir_pos + 1);
	std::cout << tmbuf << "[DEBUG][" << file_name << ":" << location.line() << 
		"][" << location.function_name() << "] " << msg << std::endl;
}

void PrintMsgV(const std::experimental::source_location location, const char* fmt, ...)
{
	char* p_buf{nullptr};
	va_list ap;
	va_start(ap, fmt);
	int __attribute__((unused)) len = vasprintf(&p_buf, fmt, ap);
	va_end(ap);
	PrintMsg(std::string(p_buf), location);
	free(p_buf);
}

/**
 * マシン環境判別(qcom, x86判別)
 */
int32_t GetMachine(void)
{
	static int32_t	kind{MKIND_NONE};
	if(MKIND_NONE == kind)
	{
		kind = MKIND_QCOM;
		std::ifstream file("/etc/os-release");
		if (file)
		{
			std::string line;
			const std::string key = "MACHINE=";
			const std::string x86 = "\"genericx86-64\"";
			while (std::getline(file, line))
			{
				// "MACHINE="の直後の値で判断
				if ((0U == line.find(key)) && (x86 == line.substr(key.size())))
				{
					kind = MKIND_X86;
					break;
				}
			}
		}
	}
	return kind;
}

/**
 * 自ユーザ名取得
 */
static std::string GetUserName(void)
{
	static std::string user_name{};

	if (user_name.empty())
	{
		// 推奨バッファサイズを取得
		constexpr size_t	bufsize{16384U};
		std::vector<char>	buf(bufsize);
		struct passwd		pwd{};
		struct passwd*		result{nullptr};
		uid_t				uid = getuid();

		// getpwuid_r() 実行
		int ret{getpwuid_r(uid, &pwd, buf.data(), bufsize, &result)};
		if ((0 == ret) && (nullptr != result))
		{
			user_name = pwd.pw_name;
		}
		else
		{
			user_name = "--unknown--";
		}
	}
	return user_name;
}

/**
 * rootユーザの場合はdefaultユーザに切り替え
 */
static std::string BuildCommand(std::string command)
{
	std::string res_command{command};
	if ("root" == GetUserName())
	{
		res_command = "su default -c \"" + command + "\"";
	}
	return res_command;
}

/**
 * プロセス停止
 */
int32_t SigStop(const std::string& pname)
{
	std::string command{"ps -ef | grep " + pname + " | grep -v grep | awk '{print $2}' | xargs kill -s SIGSTOP"};
	PrintMsg("SigStop. [" + command + "]");
	int ret = system(command.c_str());
	if (WEXITSTATUS(ret) != 0)
	{
		return VHAL_TEST_ERROR;
	}

	return VHAL_TEST_SUCCESS;
}

/**
 * プロセス再開 
 */
int32_t SigCont(const std::string& pname)
{
	std::string command{"ps -ef | grep " + pname + " | grep -v grep | awk '{print $2}' | xargs kill -s SIGCONT"};
	PrintMsg("SigCont. [" + command + "]");
	int ret = system(command.c_str());
	if (WEXITSTATUS(ret) != 0)
	{
		return VHAL_TEST_ERROR;
	}

	return VHAL_TEST_SUCCESS;
}

/**
 * @brief スクリーンショット
 */
int32_t ScreenShot(std::string test_name, int file_number)
{
	std::replace(test_name.begin(), test_name.end(), '/', '_');
	std::string execute_screenshot_command = "/usr/bin/LayerManagerControl dump screen 0 to /tmp/video_hal_tp_" + test_name + "_" + std::to_string(file_number) + ".png";
	PrintMsg("screenshot. [" + execute_screenshot_command + "]");
	int ret = system(execute_screenshot_command.c_str());
	if (WEXITSTATUS(ret) != 0)
	{
		PrintMsg("");
		return VHAL_TEST_ERROR;	
	}

	return VHAL_TEST_SUCCESS;
}

/**
 * @brief スクリーンショット
 */
int32_t ScreenShot(void)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(TEST_RENDER_IMAGE_ST));

	std::string test_name{};
	const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	if (test_info != nullptr)
	{
		/* 余計なスラッシュの置換 */
		test_name = test_info->test_suite_name();
		test_name.append(".");
		test_name.append(test_info->name());
		std::replace(test_name.begin(), test_name.end(), '/', '_');
	}
	else {
		/* テスト実行中以外はnullptrが返る */
		test_name = "unknown";
	}

	int32_t file_number{1};
	static std::map<std::string, int32_t> file_number_map;
	auto it = file_number_map.find(test_name);

	if (it == file_number_map.end())
	{
		file_number_map.insert(std::make_pair("", file_number));
	}
	else {
		++it->second;
		file_number = it->second;
	}

	struct ilmScreenProperties prop{};
	std::string filename{};
	std::string execute_screenshot_command{};
	int ret{};

	/* 前席 */
    GetScreenProperties(SCREEN_ID_FRONT, &prop);
	if (prop.screenWidth > 0)
	{
		filename = "/tmp/" SCREEN_NAME_FRONT "_" + test_name + "_" + std::to_string(file_number) + ".png";
		execute_screenshot_command = "/usr/bin/LayerManagerControl dump screen " + std::to_string(SCREEN_ID_FRONT) + " to " + filename;
		PrintMsg("screenshot. [" + execute_screenshot_command + "]");
		int ret = system(execute_screenshot_command.c_str());
		if (WEXITSTATUS(ret) != 0)
		{
			PrintMsg("ScreenShot error.");
			return VHAL_TEST_ERROR;	
		}
	}

	/* 後席 */
	memset(&prop, 0x00, sizeof(prop));
	GetScreenProperties(SCREEN_ID_REAR, &prop);
	if (prop.screenWidth > 0) 
	{
		filename = "/tmp/" SCREEN_NAME_REAR "_" + test_name + "_" + std::to_string(file_number) + ".png";
		execute_screenshot_command = "/usr/bin/LayerManagerControl dump screen " + std::to_string(SCREEN_ID_REAR) + " to " + filename;
		PrintMsg("screenshot. [" + execute_screenshot_command + "]");
		ret = system(execute_screenshot_command.c_str());
		if (WEXITSTATUS(ret) != 0)
		{
			PrintMsg("ScreenShot error.");
			return VHAL_TEST_ERROR;	
		}
	}

	/* IC */
	memset(&prop, 0x00, sizeof(prop));
	GetScreenProperties(SCREEN_ID_IC, &prop);
	if (prop.screenWidth > 0) 
	{
		filename = "/tmp/"  "ic_" + test_name + "_" + std::to_string(file_number) + ".png";
		execute_screenshot_command = "/usr/bin/LayerManagerControl dump screen " + std::to_string(SCREEN_ID_IC) + " to " + filename;
		PrintMsg("screenshot. [" + execute_screenshot_command + "]");
		ret = system(execute_screenshot_command.c_str());
		if (WEXITSTATUS(ret) != 0)
		{
			PrintMsg("ScreenShot error.");
			return VHAL_TEST_ERROR;	
		}
	}

	return VHAL_TEST_SUCCESS;
}

/**
 * @brief 画像サイズ取得
 */
int32_t GetImageSize(uint32_t surface_id, uint32_t& width, uint32_t& height)
{
	auto itr = test_image_table.find(surface_id);
	if (itr == test_image_table.end())
	{
		return VHAL_TEST_ERROR;
	}

	width = itr->second.width;
	height = itr->second.height;

	return VHAL_TEST_SUCCESS;
}

/**
 * @brief サーフェスIDからスクリーンID取得
*/
static int32_t GetScreenIdBySurfaceId(int32_t surface_id)
{
	/* サーフェスIDから出力先ディスプレイを判断する */
	if (100 <= surface_id && 200 < surface_id) 
	{
		return SCREEN_ID_REAR;
	}
	else if (200 >= surface_id && 300 < surface_id)
	{
		return SCREEN_ID_IC;
	}
	else {
		return SCREEN_ID_FRONT;
	}
}

/**
 * @brief スクリーンIDからavailableのプロパティ名取得
*/
static void GetPropertyNameByScreenId(int32_t screen_id, std::string &property_name)
{
	if (SCREEN_ID_REAR == screen_id)
	{
		property_name = "vpath.rear.status.available";
	}
	else if (SCREEN_ID_IC == screen_id)
	{
		property_name = "vpath.instrumentcluster.status.available";
	}
	else if (SCREEN_ID_FRONT == screen_id)
	{
		property_name = "vpath.front.status.available";
	}
	else
	{
		PrintMsg("undefined screen_id." + std::to_string(screen_id));
	}
}

/**
* @brief 実際にサーフェス生成するまで待つ(local関数)
 */
static int32_t WaitCreatingSurface(uint32_t surface_id, int32_t timeout_msec)
{
	int32_t result{VHAL_TEST_ERROR};
	ilmErrorTypes	ilm_ret{ILM_SUCCESS};
	t_ilm_int		length{0};
	t_ilm_surface*	p_array{nullptr};	/* t_ilm_surfaceの中身はunsigned int */
	int32_t         wait_msec_sum{0};
	constexpr int32_t	wait_msec{50};

	/* wait a moment for weston to be stable. */
	while (VHAL_TEST_SUCCESS != result)
	{
		/* タイムアウトチェック */
		if (wait_msec_sum >= timeout_msec)
		{
			break;	/* タイムアウト時間経過 */
		}

		/* サーフェスIDの一覧を取得 */
		ilm_ret = ilm_getSurfaceIDs(&length, &p_array);
		if (ILM_SUCCESS != ilm_ret)
		{
			break;
		}

		for (t_ilm_int i = 0; i < length; i++)
		{
			/* 対象のsurface_idのサーフェスが生成されているか確認する */
			if ((uint32_t)p_array[i] == surface_id)
			{
				result = VHAL_TEST_SUCCESS;
				break;
			}
		}
		free(p_array);		/* p_arrayがnullptrでもfreeは可 */
		std::this_thread::sleep_for(std::chrono::milliseconds(wait_msec));
		wait_msec_sum += wait_msec;
	}

	return result;
}

/**
 * @brief render_image起動 
 */
static int32_t StartRenderImage(uint32_t surface_id, const std::string& filename, uint32_t width, uint32_t height)
{
	std::string execute_render_image_command = "/opt/dc-ivi-pf/bin/render_image -F " + std::to_string(surface_id) + 
											   " -I " + filename + 
											   " -D " + std::to_string(width) + "x" +std::to_string(height) +
											   " > /dev/null 2>&1 &";
	PrintMsg("execute render_image. " + execute_render_image_command);
	int ret = system(execute_render_image_command.c_str());
	if (WEXITSTATUS(ret) != 0)
	{
		return VHAL_TEST_ERROR;
	}

	return WaitCreatingSurface(surface_id, 1000);
}

/**
 * @brief render_imageの起動(サーフェスID指定)
 * 
 * @param[in] surface_id サーフェスID
 */
int32_t ExecuteRenderImage(uint32_t surface_id)
{
	auto itr = test_image_table.find(surface_id);
	if (itr == test_image_table.end())
	{
		return VHAL_TEST_ERROR;
	}

	return StartRenderImage(surface_id, itr->second.name, itr->second.width, itr->second.height);
}

/**
 * @brief render_image起動(ファイル名指定)
 */
int32_t ExecuteRenderImage(uint32_t surface_id, std::string& filename)
{
	const size_t size = sizeof(test_image) / sizeof(test_image[0]);

	/* ファイル名が同じものを探す */
	for (size_t i = 0; i < size; i++)
	{
		if (test_image[i].name == filename)
		{
			return StartRenderImage(surface_id, test_image[i].name, test_image[i].width, test_image[i].height);
		}
	}

	PrintMsg("Test image is not found.");
	return VHAL_TEST_ERROR;
}

/**
 * @brief render_imageの起動(サーフェスID指定)
 * 
 * @param[in] surface_id サーフェスID
 */
int32_t ExecuteRenderImage(cockpit::hal::video_hal::CtlObj& obj, uint32_t surface_id)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	int32_t screen_id{GetScreenIdBySurfaceId(surface_id)};
	std::string property_name{};
	GetPropertyNameByScreenId(screen_id, property_name);

	available_called = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, property_name, CallbackAvailable));

	int32_t ret = ExecuteRenderImage(surface_id);
	cond_sync.wait_for(lock_sync, std::chrono::seconds(2), [] { return (available_called); });

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, property_name, CallbackAvailable));

	return ret;
}

/**
 * @brief render_imageの起動(ファイル名指定)
 */
int32_t ExecuteRenderImage(cockpit::hal::video_hal::CtlObj& obj, uint32_t surface_id, std::string& filename)
{
	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	int32_t screen_id{GetScreenIdBySurfaceId(surface_id)};
	std::string property_name{};
	GetPropertyNameByScreenId(screen_id, property_name);

	available_called = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, property_name, CallbackAvailable));

	int32_t ret = ExecuteRenderImage(surface_id, filename);
	cond_sync.wait_for(lock_sync, std::chrono::seconds(2), [] { return (available_called); });

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, property_name, CallbackAvailable));

	return ret;
}

/**
 * @brief 起動中のrender_imageをすべてkillする
 */
int32_t KillAllRenderImage(cockpit::hal::video_hal::CtlObj& obj)
{
#if 0
	FILE *fp = NULL;
    std::string cmd = "ps -ef | grep -v grep | grep render_image | awk '{print $10}'";
    if ( (fp=popen(cmd.c_str(), "r")) == NULL)
	{
		PrintMsg("popen error.");
    }
	else 
	{
    	char buf[512];
		while(fgets(buf, sizeof(buf), fp) != NULL)
		{	
			int32_t surface_id = atoi(buf);
			KillRenderImage(obj, surface_id);
		}
	}

    pclose(fp);
#else
	int ret = system("killall render_image");
	if (WEXITSTATUS(ret) != 0)
	{
		PrintMsg("");
		return VHAL_TEST_ERROR;
	}
#endif
	return VHAL_TEST_SUCCESS;
}

/**
 * @brief 起動中のrender_imageをkillする(サーフェスID指定)
 */
int32_t KillRenderImage(cockpit::hal::video_hal::CtlObj& obj, int surface_id)
{
	PrintMsg("surface_id=" + std::to_string(surface_id));
	available_called = false;

	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	int32_t screen_id{GetScreenIdBySurfaceId(surface_id)};
	std::string property_name{};
	GetPropertyNameByScreenId(screen_id, property_name);

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, property_name, CallbackAvailable));

	KillRenderImage(surface_id);

	cond_sync.wait_for(lock_sync, std::chrono::seconds(2), [] { return (available_called); });

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, property_name, CallbackAvailable));

	return VHAL_TEST_SUCCESS;
}

/**
 * @brief 起動中のrender_imageをkillする(サーフェスID指定)
 */
int32_t KillRenderImage(int surface_id)
{
	const std::string kill_render_image_command = "ps -ef | grep -v grep | grep render_image | grep 'F " + std::to_string(surface_id) + "' | awk '{print $2}' | xargs kill";
	int ret = system(kill_render_image_command.c_str());
	if (WEXITSTATUS(ret) != 0)
	{
		PrintMsg("");
		return VHAL_TEST_ERROR;
	}

	return VHAL_TEST_SUCCESS;
}

/** 
 * @brief ilm初期化処理
 */
void IlmInitialize(void)
{
	if (! ilm_initilized)
	{
		ilmErrorTypes ret_ilm = ilm_init();
		if (ILM_SUCCESS != ret_ilm)
		{
			PrintMsg("ilm_init error.");
		}
		ilm_initilized = true;
	}
}

/*
 * @brief ilm終了処理
 */
void IlmFinalize(void)
{
	if (ilm_initilized)
	{
		ilm_destroy();
		ilm_initilized = false;
	}
}

/**
 * @brief スクリーンプロパティ取得
 */
void GetScreenProperties(t_ilm_uint screenId, struct ilmScreenProperties *screenProp)
{
	ilmErrorTypes ret_ilm = ILM_SUCCESS;
	t_ilm_uint screenCount = 0;
	t_ilm_uint cnt = 0;
	t_ilm_uint *screenIds = NULL;
	struct ilmScreenProperties screen_properties;

	screenProp->screenWidth  = 0;
	screenProp->screenHeight = 0;

	IlmInitialize();

	ret_ilm = ilm_getScreenIDs(&screenCount, &screenIds);
	if (ILM_SUCCESS == ret_ilm)
	{
		for (cnt = 0; cnt < screenCount; cnt++)
		{
			if (screenId == screenIds[cnt])
			{
				ret_ilm = ilm_getPropertiesOfScreen(screenId, &screen_properties);
				if (ILM_SUCCESS == ret_ilm)
				{
					screenProp->screenWidth  = screen_properties.screenWidth;
					screenProp->screenHeight = screen_properties.screenHeight;
					if (nullptr != screen_properties.layerIds)
					{
						free(screen_properties.layerIds);
					}
				}
				else 
				{
					PrintMsg("ilm_getPropertiesOfScreen error.");
				}
			}
		}
	}
	else
	{
		PrintMsg("ilm_getScreenIDs error.");
	}
	if (nullptr != screenIds)
	{
		free(screenIds);
	}
	PrintMsg("screenId=" + std::to_string(screenId) + 
			" screenWidth=" + std::to_string(screenProp->screenWidth) +
			" screenHeight=" + std::to_string(screenProp->screenHeight));
}

/**
 * @brief 汎用動画再生機能用ファイル名取得 
 *        前席画面解像度に一致するサイズの動画ファイル名を返す。
 */
const char * GetMovieFileName(void)
{
		return TEST_MOVIE_FILE_NAME;
}

/**
 * @brief 前席映像パス切替
 *        シーケンスの前提条件の設定に使用。
 */
int32_t SwitchVpathFront(cockpit::hal::video_hal::CtlObj& obj, std::string vpath)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};
	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	callback_called = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.front.status.current", Callback));

	ret = cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS)
	{
		result = VHAL_TEST_ERROR;
	}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS)
	{
		result = VHAL_TEST_ERROR;
	}

	cond_sync.wait_for(lock_sync, std::chrono::seconds(2), [] { return (callback_called); });

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.front.status.current", Callback));

	PrintMsg("end.");
	return result;
}

/**
 * @brief 後席映像パス切替
 *        シーケンスの前提条件の設定に使用。
 */
int32_t SwitchVpathRear(cockpit::hal::video_hal::CtlObj& obj, std::string vpath)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};
	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	callback_called = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.current", Callback));

	ret = cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS)
	{
		result = VHAL_TEST_ERROR;
	}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS)
	{
		result = VHAL_TEST_ERROR;
	}

	cond_sync.wait_for(lock_sync, std::chrono::seconds(2), [] { return (callback_called); });

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.current", Callback));

	PrintMsg("end.");
	return result;
}

/**
 * @brief カメラ映像パス切替
 *        シーケンスの前提条件の設定に使用。
 */
int32_t SwitchVpathCamera(cockpit::hal::video_hal::CtlObj& obj, std::string vpath)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};
	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	callback_called = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.camera.status.current", Callback));

	ret = cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS)
	{
		result = VHAL_TEST_ERROR;
	}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS)
	{
		result = VHAL_TEST_ERROR;
	}

	cond_sync.wait_for(lock_sync, std::chrono::seconds(2), [] { return (callback_called); });

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.camera.status.current", Callback));

	PrintMsg("end.");
	return result;
}

/**
 * @brief IC映像パス切替
 *        シーケンスの前提条件の設定に使用。
 */
int32_t SwitchVpathIc(cockpit::hal::video_hal::CtlObj& obj, std::string vpath)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};
	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	callback_called = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.instrumentcluster.status.current", Callback));

	ret = cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.current", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	cond_sync.wait_for(lock_sync, std::chrono::seconds(2), [] { return (callback_called); });

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.instrumentcluster.status.current", Callback));

	PrintMsg("end.");
	return result;
}

/**
 * @brief 後席専用モード向け映像パス切替
 *        シーケンスの前提条件の設定に使用。
 */
int32_t SwitchVpathRse(cockpit::hal::video_hal::CtlObj& obj, std::string vpath)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};
	bool notification{true};
	std::unique_lock<std::mutex> lock_sync(mtx_sync);

	callback_called = false;
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.rse.display", Callback));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RegisterCallback(obj, "vpath.rear.status.rse.notification", Callback));

	ret = cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.display", &vpath, vpath.length() + 1, VIDEO_HAL_DATA_TYPE_STR);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	cond_sync.wait_for(lock_sync, std::chrono::seconds(2), [] { return (callback_called); });

	callback_called = false;
	ret = cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.rse.notification", &notification, sizeof(notification), VIDEO_HAL_DATA_TYPE_BOOL);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	cond_sync.wait_for(lock_sync, std::chrono::seconds(2), [] { return (callback_called); });

	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.rse.display", Callback));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::ClearCallback(obj, "vpath.rear.status.rse.notification", Callback));

	PrintMsg("end.");
	return result;
}

/*
 * @brief 不透明度設定
 */
int32_t SetOpacityFront(cockpit::hal::video_hal::CtlObj& obj, uint32_t opacity)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	ret = cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.opacity", &opacity, sizeof(opacity), VIDEO_HAL_DATA_TYPE_NUM);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/*
 * @brief 後席不透明度設定
 */
int32_t SetOpacityRear(cockpit::hal::video_hal::CtlObj& obj, uint32_t opacity)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	ret = cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.opacity", &opacity, sizeof(opacity), VIDEO_HAL_DATA_TYPE_NUM);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/*
 * @brief IC不透明度設定
 */
int32_t SetOpacityIc(cockpit::hal::video_hal::CtlObj& obj, uint32_t opacity)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	ret = cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.opacity", &opacity, sizeof(opacity), VIDEO_HAL_DATA_TYPE_NUM);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/**
 * @brief 可視性設定
 */
int32_t SetVisibleFront(cockpit::hal::video_hal::CtlObj& obj, bool visible)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	ret = cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.visible", &visible, sizeof(visible), VIDEO_HAL_DATA_TYPE_BOOL);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/**
 * @brief 後席可視性設定
 */
int32_t SetVisibleRear(cockpit::hal::video_hal::CtlObj& obj, bool visible)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	ret = cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.visible", &visible, sizeof(visible), VIDEO_HAL_DATA_TYPE_BOOL);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/**
 * @brief IC可視性設定
 */
int32_t SetVisibleIc(cockpit::hal::video_hal::CtlObj& obj, bool visible)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	ret = cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.visible", &visible, sizeof(visible), VIDEO_HAL_DATA_TYPE_BOOL);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/**
 * @brief 前席映像出力サイズ設定
 */
int32_t SetFrontOutput(cockpit::hal::video_hal::CtlObj& obj, uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool on)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.x", &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM);
    cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.y", &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM);
	cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM);
    cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM);
    cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.output.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL);

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/**
 * @brief 後席映像出力サイズ設定
 */
int32_t SetRearOutput(cockpit::hal::video_hal::CtlObj& obj, uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool on)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.output.x", &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM);
    cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.output.y", &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM);
	cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.output.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM);
    cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.output.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM);
    cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.output.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL);

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/**
 * @brief カメラ映像出力サイズ設定
 */
int32_t SetCameraOutput(cockpit::hal::video_hal::CtlObj& obj, uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool on)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.x", &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM);
    cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.y", &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM);
	cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM);
    cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM);
    cockpit::hal::video_hal::SetValue(obj, "vpath.camera.control.output.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL);

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/**
 * @brief IC映像出力サイズ設定
 */
int32_t SetIcOutput(cockpit::hal::video_hal::CtlObj& obj, uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool on)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.output.x", &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM);
    cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.output.y", &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM);
	cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.output.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM);
    cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.output.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM);
    cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.output.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL);

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/**
 * @brief 前席映像クリッピングサイズ設定
 */
int32_t SetFrontClipping(cockpit::hal::video_hal::CtlObj& obj, uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool enable, bool on)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.x", &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM);
    cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.y", &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM);
	cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM);
    cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM);
    cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.enable", &enable, sizeof(enable), VIDEO_HAL_DATA_TYPE_BOOL);
    cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.clipping.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL);

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/**
 * @brief IC映像クリッピングサイズ設定
 */
int32_t SetIcClipping(cockpit::hal::video_hal::CtlObj& obj, uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool enable, bool on)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.x", &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM);
    cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.y", &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM);
	cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM);
    cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM);
    cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.enable", &enable, sizeof(enable), VIDEO_HAL_DATA_TYPE_BOOL);
    cockpit::hal::video_hal::SetValue(obj, "vpath.instrumentcluster.control.clipping.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL);

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/**
 * @brief 前席映像ワイド設定
 */
int32_t SetWideModeFront(cockpit::hal::video_hal::CtlObj& obj, uint32_t widemode)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	ret = cockpit::hal::video_hal::SetValue(obj, "vpath.front.control.widemode", &widemode, sizeof(widemode), VIDEO_HAL_DATA_TYPE_NUM);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/**
 * @brief 後席映像ワイド設定
 */
int32_t SetWideModeRear(cockpit::hal::video_hal::CtlObj& obj, uint32_t widemode)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	ret = cockpit::hal::video_hal::SetValue(obj, "vpath.rear.control.widemode", &widemode, sizeof(widemode), VIDEO_HAL_DATA_TYPE_NUM);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/**
 * @brief 画質調整(明るさ)
 */
int32_t SetBrightness(cockpit::hal::video_hal::CtlObj& obj, uint32_t brightness)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	ret = cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.brightness.step", &brightness, sizeof(brightness), VIDEO_HAL_DATA_TYPE_NUM);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}


	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/**
 * @brief 画質調整(コントラスト)
 */
int32_t SetContrast(cockpit::hal::video_hal::CtlObj& obj, uint32_t contrast)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	ret = cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.contrast.step", &contrast, sizeof(contrast), VIDEO_HAL_DATA_TYPE_NUM);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}


	PrintMsg("end.");
	return result;
}

/**
 * @brief 画質調整(強制HMI設定)
 */
int32_t SetForcedHmiImgAdj(cockpit::hal::video_hal::CtlObj& obj, bool on)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	ret = cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.forced_hmi_img_adj", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/**
 * @brief 画質調整(強制多感覚連携設定)
 */
int32_t SetForcedMultisensoryImgAdj(cockpit::hal::video_hal::CtlObj& obj, bool on)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	ret = cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.forced_multisensory_img_adj", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/**
 * @brief 映像ソース設定
 */
int32_t SetVideoSourceId(cockpit::hal::video_hal::CtlObj& obj, uint32_t video_source_id)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	ret = cockpit::hal::video_hal::SetValue(obj, "vsrc.front.control.id", &video_source_id, sizeof(video_source_id), VIDEO_HAL_DATA_TYPE_NUM);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/**
 * @brief 前席ディスプレイ全体Mute
 */
int32_t MuteDisplayFront(cockpit::hal::video_hal::CtlObj& obj, bool mute)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	ret = cockpit::hal::video_hal::SetValue(obj, "mute.front.control.display", &mute, sizeof(mute), VIDEO_HAL_DATA_TYPE_BOOL);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/**
 * @brief 後席ディスプレイ全体Mute
 */
int32_t MuteDisplayRear(cockpit::hal::video_hal::CtlObj& obj, bool mute)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	ret = cockpit::hal::video_hal::SetValue(obj, "mute.rear.control.display", &mute, sizeof(mute), VIDEO_HAL_DATA_TYPE_BOOL);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/**
 * @brief 前席映像面Mute
 */
int32_t MuteVideoFront(cockpit::hal::video_hal::CtlObj& obj, bool mute)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	ret = cockpit::hal::video_hal::SetValue(obj, "mute.front.control.video", &mute, sizeof(mute), VIDEO_HAL_DATA_TYPE_BOOL);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/**
 * @brief 後席映像面Mute
 */
int32_t MuteVideoRear(cockpit::hal::video_hal::CtlObj& obj, bool mute)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	ret = cockpit::hal::video_hal::SetValue(obj, "mute.rear.control.video", &mute, sizeof(mute), VIDEO_HAL_DATA_TYPE_BOOL);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/**
 * @brief バックライト点灯状態
 */
int32_t MuteBacklight(cockpit::hal::video_hal::CtlObj& obj, bool on)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	ret = cockpit::hal::video_hal::SetValue(obj, "mute.front.control.backlight", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/**
 * @brief iviレイヤ優先度変更
 */
int32_t SetIviLayerOrder(cockpit::hal::video_hal::CtlObj& obj, int32_t ivi_id, int32_t layer_order)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	ret = cockpit::hal::video_hal::SetValue(obj, "ivi.layer.control.order?ivi_id=" + std::to_string(ivi_id), &layer_order, sizeof(layer_order), VIDEO_HAL_DATA_TYPE_NUM);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/**
 * @brief iviレイヤ優先度変更
 */
int32_t SetIviLayerOrder(cockpit::hal::video_hal::CtlObj& obj, std::string ivi_name, int32_t layer_order)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	ret = cockpit::hal::video_hal::SetValue(obj, "ivi.layer.control.order?name=" + ivi_name, &layer_order, sizeof(layer_order), VIDEO_HAL_DATA_TYPE_NUM);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/**
 * @brief 接続されているカメラ種別
 */
int32_t SetConnectedCamera(cockpit::hal::video_hal::CtlObj& obj, int32_t connected_camera)
{
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};
#if 0 /* NOP_CONNECTED_CAMERA */
	PrintMsg("start.");
	ret = cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.connected.camera", &connected_camera, sizeof(connected_camera), VIDEO_HAL_DATA_TYPE_NUM);
	if (ret != VIDEO_HAL_API_SUCCESS)
	{
		PrintMsg("error 1");
		result = VHAL_TEST_ERROR;
	}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	if (ret != VIDEO_HAL_API_SUCCESS)
	{
		PrintMsg("error 2 camera=" + std::to_string(connected_camera));
		result = VHAL_TEST_ERROR;
	}

	PrintMsg("end.");
#endif /* NOP_CONNECTED_CAMERA */
	return result;
}

/**
 * @brief 接続されているRSE種別
 */
int32_t SetConnectedRse(cockpit::hal::video_hal::CtlObj& obj, int32_t connected_rse)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	ret = cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.connected.rse", &connected_rse, sizeof(connected_rse), VIDEO_HAL_DATA_TYPE_NUM);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/**
 * @brief テーマカラー
 */
int32_t SetThemeColor(cockpit::hal::video_hal::CtlObj& obj, int32_t theme_color)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	ret = cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.theme_color", &theme_color, sizeof(theme_color), VIDEO_HAL_DATA_TYPE_NUM);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/**
 * @brief 昼夜モード
 */
int32_t SetDayNight(cockpit::hal::video_hal::CtlObj& obj, int32_t day_night)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	ret = cockpit::hal::video_hal::SetValue(obj, "vhal.setting.control.day_night", &day_night, sizeof(day_night), VIDEO_HAL_DATA_TYPE_NUM);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/**
 * @brief ヒーコンエリアサイズ設定
 */
int32_t SetHeaconAreaSize(cockpit::hal::video_hal::CtlObj& obj, uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool on)
{
	PrintMsg("start.");
	int32_t ret{};
	int32_t result{VHAL_TEST_SUCCESS};

	cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.x", &x, sizeof(x), VIDEO_HAL_DATA_TYPE_NUM);
    cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.y", &y, sizeof(y), VIDEO_HAL_DATA_TYPE_NUM);
	cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.width", &width, sizeof(width), VIDEO_HAL_DATA_TYPE_NUM);
    cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.height", &height, sizeof(height), VIDEO_HAL_DATA_TYPE_NUM);
    cockpit::hal::video_hal::SetValue(obj, "heacon.front.control.output.on", &on, sizeof(on), VIDEO_HAL_DATA_TYPE_BOOL);

	ret = cockpit::hal::video_hal::RequestUpdate(obj);
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, ret);
	if (ret != VIDEO_HAL_API_SUCCESS) {result = VHAL_TEST_ERROR;}

	PrintMsg("end.");
	return result;
}

/**
 * @brief レイヤ設定ファイル
 */
void SetLayerConfig(cockpit::hal::video_hal::CtlObj& obj, const std::string& path)
{
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "ivi.layout.control.configuration.file", &path, path.length() + 1, VIDEO_HAL_DATA_TYPE_STR));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
}

/**
 * @brief シーン別電源設定
 */
void SetPowerStatusCom(uint32_t status)
{
	std::string command = brysh_command_base + " dbwrite /db/sys_info/power/power_status.dbf -d " + std::to_string(status);
	command = BuildCommand(command);	/* rootユーザ時はdefaultユーザに変更 */
	PrintMsg("SetPowerStatus. [" + command + "]");
	int ret = system(command.c_str());
	EXPECT_EQ(0, ret);
}

/**
 * @brief シーン別電源「通常」
 */
void SetPowerStatusNormal(void)
{
	uint32_t status{DAT_CNV_PWR_STS_NORMAL_DONE};

	PrintMsg("start.");

	SetPowerStatusCom(status);

	PrintMsg("end.");
}

/**
 * @brief シーン別電源「乗車中」
 */
void SetPowerStatusPre(void)
{
	uint32_t status{DAT_CNV_PWR_STS_IN_VEHICLE_DONE};

	PrintMsg("start.");

	SetPowerStatusCom(status);

	PrintMsg("end.");
}

/**
 * @brief シーン別電源「駐乗車起動」
 */
void SetPowerStatusBackGround(void)
{
	uint32_t status{DAT_CNV_PWR_STS_BACKGROUND_DONE};

	PrintMsg("start.");

	SetPowerStatusCom(status);

	PrintMsg("end.");
}

/**
 * @brief カメラ同期検知 
 */
void DetectCameraSync(cockpit::hal::video_hal::CtlObj& obj, int32_t cam_sync)
{
#if 0 /* 検討中のためNOP */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.tp.setting.control.sync.camera", &cam_sync, sizeof(cam_sync), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
#endif /* NOP */
}

/**
 * @brief カメラ種別判別通知 
 */
void NotifyCameraType(cockpit::hal::video_hal::CtlObj& obj, int32_t cam_size)
{
#if 0 /* 検討中のためNOP */
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::SetValue(obj, "vhal.tp.setting.control.size.camera", &cam_size, sizeof(cam_size), VIDEO_HAL_DATA_TYPE_NUM));
	EXPECT_EQ(VIDEO_HAL_API_SUCCESS, cockpit::hal::video_hal::RequestUpdate(obj));
#endif /* 検討中のためNOP */
}

/**
 * @brief HDMIビデオフォーマット変更 
 */
void ChangeHdmiVideoFormat(int32_t format)
{
	std::string command = brysh_command_base + " dbwrite /db/sys_info/conn_info/hdmi_info/video_format.dbf -d " + std::to_string(format);
	command = BuildCommand(command);	/* rootユーザ時はdefaultユーザに変更 */
	PrintMsg("[" + command + "]");
	int ret = system(command.c_str());
	EXPECT_EQ(0, ret);
}

/**
 * @brief HDMI接続検知 
 */
void DetectConnectedHdmi(int32_t conn)
{
	std::string command = brysh_command_base + " dbwrite /db/sys_info/conn_info/hdmi.dbf -d " + std::to_string(conn);
	command = BuildCommand(command);	/* rootユーザ時はdefaultユーザに変更 */
	PrintMsg("[" + command + "]");
	int ret = system(command.c_str());
	EXPECT_EQ(0, ret);
}

/**
 * HDMIビデオフォーマット読み込み 
 */
void ReadHdmiVideoFormat(int32_t& format)
{
	std::string command = brysh_command_base + " dbread /db/sys_info/conn_info/hdmi_info/video_format.dbf -d | cut -d'=' -f 2";
	command = BuildCommand(command);	/* rootユーザ時はdefaultユーザに変更 */
	PrintMsg("[" + command + "]");
	FILE *fp = popen(command.c_str(), "r");
	if (!fp)
	{
		PrintMsg("popen error.");
		return;
	}
	char buf[256] = {0};
	while(fgets(buf, sizeof(buf), fp) != NULL)
	{
		/* 数値取り込み */
		format = atoi(buf);
	}

	pclose(fp);
}

/**
 * @brief サーフェス生成(VideoHAL内部イベント発生)
 */
static void OccurrCreateSurface(InternalEventParameter& param) 
{
	PrintMsg("start.");

	ExecuteRenderImage(param.u.create_surface.surface_id);

	PrintMsg("end.");
}

/**
 * @brief サーフェス削除(VideoHAL内部イベント発生)
 */
static void OccurrDeleteSurface(InternalEventParameter& param)
{
	PrintMsg("start.");

	KillRenderImage(param.u.delete_surface.surface_id);

	PrintMsg("end.");
}

/** 
 * @brief サーフェスサイズ通知(VideoHAL内部イベント発生)
 */
static void OccurrNotifySurfaceSize(InternalEventParameter& param)
{
	PrintMsg("start.");

	ExecuteRenderImage(param.u.notify_surface_size.surface_id);

	PrintMsg("end.");
}

/**
 * @brief カメラ同期検知(VideoHAL内部イベント発生)
 */
static void OccurrDetectCameraSync(InternalEventParameter& param)
{
	PrintMsg("start.");

	DetectCameraSync(param.obj, param.u.detect_camera_sync.cam_sync);

	PrintMsg("end.");
}

/**
 * @brief カメラ種別判別通知(VideoHAL内部イベント発生)
 */
static void OccurrNotifyCameraType(InternalEventParameter& param)
{
	PrintMsg("start.");

	NotifyCameraType(param.obj, param.u.notify_camera_type.cam_size);

	PrintMsg("end.");
}

/**
 * @brief HDMIビデオフォーマット変更(VideoHAL内部イベント発生)
 */
static void OccurrChangeHdmiVideoFormat(InternalEventParameter& param)
{
	PrintMsg("start.");

	ChangeHdmiVideoFormat(param.u.change_hdmi_video_format.format);

	PrintMsg("end.");
}

/**
 * @brief HDMI接続検知(VideoHAL内部イベント発生)
 */
static void OccurrDetectConnectedHdmi(InternalEventParameter& param)
{
	PrintMsg("start.");

	DetectConnectedHdmi(param.u.detect_connected_hdmi.conn);

	PrintMsg("end.");
}

/**
 * @brief VideoHAL内部イベント発生 
 */
void OccurrInternalEvent(InternalEventParameter& param)
{
	switch(param.type)
	{
		case INTERNAL_EVENT_CREATE_SURFACE:
			OccurrCreateSurface(param);
			break;
		case INTERNAL_EVENT_DELETE_SURFACE:
			OccurrDeleteSurface(param);
			break;
		case INTERNAL_EVENT_NOTIFY_SURFACE_SIZE:
			OccurrNotifySurfaceSize(param);
			break;
		case INTERNAL_EVENT_DETECT_CAMERA_SYNC:
			OccurrDetectCameraSync(param);
			break;
		case INTERNAL_EVENT_NOTIFY_CAMERA_TYPE:
			OccurrNotifyCameraType(param);
			break;
		case INTERNAL_EVENT_CHANGE_HDMI_VIDEO_FORMAT:
			OccurrChangeHdmiVideoFormat(param);
			break;
		case INTERNAL_EVENT_DETECT_CONNECTED_HDMI:
			OccurrDetectConnectedHdmi(param);
			break;
		default:
			break;
	}
}

/**
 * @brief homescreen.service起動
 */
int32_t StartHomeScreen(void)
{
	if (0 != system("systemctl start homescreen.service &"))
	{
		return VHAL_TEST_ERROR;
	}
	else 
	{
		return VHAL_TEST_SUCCESS;
	}
}

/**
 * @brief homescreen.service停止
 */
int32_t StopHomeScreen(void)
{
	if (0 != system("systemctl stop homescreen.service &"))
	{
		return VHAL_TEST_ERROR;
	}
	else 
	{
		return VHAL_TEST_SUCCESS;
	}
}
