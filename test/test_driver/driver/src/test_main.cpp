#if 0	// original
#include "test_main.h"
#include <dlt/dlt.h>

#define VHAL_TP_DLT_ID				"HVDC"

int main(int argc, char **argv)
{

	::testing::InitGoogleTest(&argc, argv);
	::testing::AddGlobalTestEnvironment(new VhalTestEnvironment());

	return RUN_ALL_TESTS();
}

void VhalTestEnvironment::SetUp(void)
{
	DLT_REGISTER_APP(VHAL_TP_DLT_ID, "VideoHAL test");
}

void VhalTestEnvironment::TearDown(void)
{
	DLT_UNREGISTER_APP();
	usleep(200 * 500);
}

#endif

/*
 * @copyright Copyright (c) 2022 Woven Alpha, Inc
 */

#include <gtest/gtest.h>
#include <dlt/dlt.h>

#include "video_hal_ct_common.h"
#include "file_observer.h"

#define PATH_METER_GVIF_STATUS		"/run/arene/vehicle_fs/var/instrument_cluster/common/meter_gvif_status"
#define VHAL_CT_COMMAND_WAIT		(200 * 1000)	//us
#define VHAL_CT_COMMAND_BUF_MAX		(256)
#define VHAL_CT_COMMAND_FINISH		(50  * 1000)

static bool _exec_hdmi   = false;
static bool _exec_rear   = false;
static bool _exec_ic     = false;
static bool _exec_camera = false;
static bool _exec_heacon = false;
static bool _enable_key  = false;

void exec_test_video_client(int32_t ivi_id, const char *filename)
{
	char buf[VHAL_CT_COMMAND_BUF_MAX];
	std::string strhalcp = VHAL_EXEC_PATH;
	std::string strfname = VHAL_EXEC_PATH;

	strhalcp += "halcp_video_client";
	strfname += filename;
	/* コマンドを生成している */
	std::sprintf(buf, "%s -S %d -F %s > /dev/null 2>&1 &", strhalcp.c_str(), ivi_id, strfname.c_str());
	/* プロセスを立ち上げて実行している */
	int32_t ret = system(buf);
	/* VHAL_CT_COMMAND_WAITよりも後にサーフェスが生成される場合があり、*/
	/* テスト項目が失敗することがあるため実装変更 */
//	if (ret == 0)
//	{
//		/* wait a moment for weston to be stable. */
//		usleep(VHAL_CT_COMMAND_WAIT);
//	}
	if (ret != 0)
	{
		return;
	}

	ilmErrorTypes	ilm_ret = ILM_SUCCESS;
	t_ilm_int		length = 0;
	t_ilm_surface*	p_array = nullptr;	/* t_ilm_surfaceの中身はunsigned int */
	bool			sok_flag = false;

	struct timespec	st, ed;
	clock_gettime(CLOCK_MONOTONIC, &st);

	/* wait a moment for weston to be stable. */
	while (!sok_flag)
	{
		/* タイムアウトチェック */
		clock_gettime(CLOCK_MONOTONIC, &ed);
		uint64_t tout = ((((uint64_t)ed.tv_sec) * 1000000000) + ed.tv_nsec) -
						((((uint64_t)st.tv_sec) * 1000000000) + st.tv_nsec);
		/* 3秒(=30億ナノ秒)以上経過時はタイムアウト中断 */
		if (tout >= (3 * 1000 * 1000 * 1000ull))
		{
			break;
		}

		/* サーフェスIDの一覧を取得 */
		ilm_ret = ilm_getSurfaceIDs(&length, &p_array);
		if (ILM_SUCCESS != ilm_ret)
		{
			break;
		}

		for (t_ilm_int i = 0; i < length; i++)
		{
			/* 対象のivi_idのサーフェスが生成されているか確認する */
			if ((int32_t)p_array[i] == ivi_id)
			{
				sok_flag = true;
				break;
			}
		}
		free(p_array);		/* p_arrayがnullptrでもfreeは可 */
		usleep(VHAL_CT_COMMAND_FINISH);
	}
}

void kill_test_video_client()
{
	/* kill test video client */
	int32_t ret = system("kill -KILL `pidof halcp_video_client` > /dev/null 2>&1");
	if (ret == 0)
	{
		/* wait a moment for weston to be stable. */
		usleep(VHAL_CT_COMMAND_WAIT);
	}
}

void set_hmi_visibility(bool visible)
{
	ilmErrorTypes ret_ilm = ILM_SUCCESS;

	/* make "hmi_flutter" layer invisible */
	ret_ilm = ilm_layerSetVisibility(VHAL_CT_IVIID_HMI_LAYER, visible);
	if (ret_ilm != ILM_SUCCESS)
	{
		EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] hmi_flutter layer control failed.\n"));
	}
	ret_ilm = ilm_commitChanges();
	if (ret_ilm == ILM_SUCCESS)
	{
		/* wait a moment for weston to be stable. */
		usleep(VHAL_CT_COMMAND_WAIT);
	}
	else
	{
		EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] hmi_flutter layer control failed.\n"));
	}
}

void get_screen_properties(t_ilm_uint screenId, struct ilmScreenProperties *screenProp)
{
	ilmErrorTypes ret_ilm = ILM_SUCCESS;
	t_ilm_uint screenCount = 0;
	t_ilm_uint cnt = 0;
	t_ilm_uint *screenIds = NULL;
	struct ilmScreenProperties screen_properties;

	screenProp->screenWidth  = 0;
	screenProp->screenHeight = 0;

	/* Get the screen Ids */
	ret_ilm = ilm_getScreenIDs(&screenCount, &screenIds);
	if (ILM_SUCCESS == ret_ilm)
	{
		for (cnt = 0; cnt < screenCount; cnt++)
		{
			if (screenId == screenIds[cnt])
			{
				/* Get the screen properties from the Layermanagement */
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
			}
		}
	}
	if (nullptr != screenIds)
	{
		free(screenIds);
	}
}

bool get_exec_hdmi(void) {return _exec_hdmi;}
bool get_exec_rear(void) {return _exec_rear;}
bool get_exec_ic(void) {return _exec_ic;}
bool get_exec_camera(void) {return _exec_camera;}
bool get_exec_heacon(void) {return _exec_heacon;}
bool get_enable_key(void) {return _enable_key;}

void wait_msec(int32_t msec)
{
	if (!_enable_key)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(msec));
	}
}

///
// Test Program Main
///
int main(int argc, char* argv[])
{
	int32_t ret;
	ilmErrorTypes ret_ilm = ILM_SUCCESS;

	DLT_REGISTER_APP("HVDC", "VideoHAL test");

	ret_ilm = ilm_init();
	if (ILM_SUCCESS != ret_ilm)
	{
		EXPECT_EQ(HALCP_RET_OK, HALCP_INDICATION("[CertificationTest] ilm_init() error.\n"));
	}

    // Googletest command line initialization.
    testing::InitGoogleTest(&argc, argv);

	std::string meter_gvif_status(PATH_METER_GVIF_STATUS);
	const int32_t init_value{2};
	cockpit::bs::CFileObserver::GetInstance()->Write(meter_gvif_status, &init_value, sizeof(init_value));

	/* Select test case */
	HALCP_INDICATION("[CertificationTest] /*** Select test case\n");
#if 0
	ret = HALCP_WAITINPUTRESULT("[CertificationTest] /***   HDMI ? ('y' or 'Y' is select, Others are no select):", nullptr);
	if (HALCP_RET_OK == ret) { _exec_hdmi = true; }
	ret = HALCP_WAITINPUTRESULT("[CertificationTest] /***   rear(FullRSE) ? ('y' or 'Y' is select, Others are no select):", nullptr);
	if (HALCP_RET_OK == ret) { _exec_rear = true; }
	ret = HALCP_WAITINPUTRESULT("[CertificationTest] /***   Instrument_cluster ? ('y' or 'Y' is select, Others are no select):", nullptr);
	if (HALCP_RET_OK == ret) { _exec_ic = true; }
	ret = HALCP_WAITINPUTRESULT("[CertificationTest] /***   CAMERA ? ('y' or 'Y' is select, Others are no select):", nullptr);
	if (HALCP_RET_OK == ret) { _exec_camera = true; }
	ret = HALCP_WAITINPUTRESULT("[CertificationTest] /***   heacon ? ('y' or 'Y' is select, Others are no select):", nullptr);
	if (HALCP_RET_OK == ret) { _exec_heacon = true; }
#else
	_exec_hdmi   = true;
//	_exec_rear   = true;
//	_exec_ic     = true;
	_exec_camera = true;
	_exec_heacon = true;
#endif

	// get COCKPIT_RUNTIME_ENABLE_CERTIFICATION
	const char* const p_env = getenv("COCKPIT_RUNTIME_ENABLE_CERTIFICATION");
	if ((nullptr != p_env) && (0 == strcmp(p_env,"1"))) { _enable_key = true; }

	// Googletest Run.
    ret = RUN_ALL_TESTS();

	ilm_destroy();

	DLT_UNREGISTER_APP();
	usleep(VHAL_CT_COMMAND_WAIT);

	return ret;
}
