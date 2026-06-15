
// #include <unistd.h>
// #include <time.h>
#include <sched.h>
#include "test_main.h"
#include "file_observer.h"
// #include "com_stddef.h"
#include "stub_common.h"
#include <condition_variable>

extern "C" {
#include "ccm_public.h"
#include "MiscConfig.h"
}

enum eMiscCtrlApi {
	eDataSend ,
	eEntDataNtyCB ,
	eDelDataNtyCB ,
	eDataNtyCB ,
	eEntStsCB,
	eDelStsCB,
	eStsCB,
	eMAX
};

static std::mutex mtx_sync[eMAX];
static std::condition_variable cond_sync[eMAX];
static bool detect_ntfy = false;
static bool detect_sts  = false;
static bool first_run = true;
static MiscCtrlApiSts cb_status;
static UINT8	cb_data_type;
static UINT8	cb_buf[8];
static UINT32	cb_len;

class CcmTest_Api: public::testing::Test
{
protected:
	virtual void SetUp();
 	virtual void TearDown();
};

void CcmTest_Api::SetUp()
{
	if (true == first_run)
	{
		first_run = false;
		system("rm -fr " STUB_PATH_CB2);
		system("rm -fr " STUB_PATH_CB3);
		system("rm -fr " STUB_PATH_FUNC);
	}
}

void CcmTest_Api::TearDown()
{
}

/*---------------------------------------------------- */
/* 関数戻り値ファイルのクリア(次回読み込み時はデフォルト値になる) */
static void ClearFuncResult(const char* path)
{
	if (!path)
	{
		return;
	}
	size_t	len = strlen(path) + 64;
	char*	cmd = (char*)malloc(len);
	if (!cmd)
	{
		return;
	}
	
	/* pathにはSPACEなどの特殊な文字を含んではいけない */
	/* 含んだ場合は、誤動作する */
	snprintf(cmd, len, "rm -f %s >/dev/null 2>&1", path);
	system(cmd);
	snprintf(cmd, len, "touch %s >/dev/null 2>&1", path);
	system(cmd);
	free(cmd);
}

/*---------------------------------------------------- */
static void StubWrite(const char* path, const INT32 val)
{
	cockpit::bs::CFileObserver::GetInstance()->Write(path, &val, sizeof(val));
}

// /*---------------------------------------------------- */
// static void StubWrite(const char* path, const void* ptr)
// {
// 	cockpit::bs::CFileObserver::GetInstance()->Write(path, (void*)&ptr, sizeof(ptr));
// }

// /*---------------------------------------------------- */
// static void StubWrite(const char* path, const void* ptr, size_t size)
// {
// 	cockpit::bs::CFileObserver::GetInstance()->Write(path, ptr, size);
// }

/*---------------------------------------------------- */
/* MiscCtrlApiDataSend のcall時に実施するCB */
static void MiscCtrlApiDataSendListener(const char* p_path)
{
	if (true == detect_ntfy)
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync[eDataSend]);
		cond_sync[eDataSend].notify_one();
	}
}

static void MiscCtrlApiEntStsCBListener(const char* p_path)
{
	if (true == detect_ntfy)
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync[eEntStsCB]);
		cond_sync[eEntStsCB].notify_one();
	}
}

static void MiscCtrlApiDelStsCBListener(const char* p_path)
{
	if (true == detect_ntfy)
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync[eDelStsCB]);
		cond_sync[eDelStsCB].notify_one();
	}
}

/*---------------------------------------------------- */
/* MiscCtrlApiEntDataNtyCB のcall時に実施するCB */
static void MiscCtrlApiEntDataNtyCBListener(const char* p_path)
{
	if (true == detect_ntfy)
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync[eEntDataNtyCB]);
		cond_sync[eEntDataNtyCB].notify_one();
	}
}

/*---------------------------------------------------- */
/* MiscCtrlApiDelDataNtyCB のcall時に実施するCB */
static void MiscCtrlApiDelDataNtyCBListener(const char* p_path)
{
	if (true == detect_ntfy)
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync[eDelDataNtyCB]);
		cond_sync[eDelDataNtyCB].notify_one();
	}
}

static void MiscCtrlApiStsCB_control(MiscCtrlApiSts* p_sts)
{
	if (true == detect_sts)
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync[eStsCB]);
		if (nullptr != p_sts)
		{
			cb_status = *p_sts;
		}
		cond_sync[eStsCB].notify_one();
	}
}

/*---------------------------------------------------- */
/* control更新時に実施するCBまたは3秒周期で呼ばれるCB */
static void MiscCtrlApiDataNtyCB_control(UINT8 data_type, void* p_cmd, UINT32 len)
{
	char buf[64];
	uint8_t* data{(uint8_t*)p_cmd};

	for (UINT32 i=0; i<len && i<8; i++)
	{
		snprintf(&buf[i*3], sizeof(buf)-(i*3), "%02X ", data[i]);
	}
	printf("DataNtyCB len=%u typ=0x%02X cmd=[%s]\n", len, data_type, buf);

	if (true == detect_sts)
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync[eDataNtyCB]);
		if (len <= sizeof(cb_buf))
		{
			cb_data_type = data_type;
			memcpy(cb_buf, p_cmd, len);
			cb_len = len;
		}
		cond_sync[eDataNtyCB].notify_one();
		sched_yield();
	}
}

/*---------------------------------------------------- */
/* MiscCtrlApiDataSend 関数の調査 */
TEST_F(CcmTest_Api, MiscCtrlApiDataSend)
{
	void*	p_ccm_obj = NULL;
	INT32	ret;
	uint8_t data_type = 0x34;
	uint8_t sub_type = 0x1;
	std::vector<uint8_t> send_data{};
	send_data.push_back(sub_type);
	uint32_t datasize = send_data.size();
	
	/* 準備(Open) */
	ret = CcmApiOpen(&p_ccm_obj);
	ASSERT_EQ(CCM_RET_SUCCESS, ret);
	ASSERT_NE(nullptr, p_ccm_obj);

	/* 戻り値に何も指定せずに実施 */
	ret = MiscCtrlApiDataSend(p_ccm_obj, data_type, send_data.data(), datasize);
	EXPECT_EQ(MISC_CTRL_RET_SUCCESS, ret);

	/* 戻り値に MISC_CTRL_RET_ERR_SYSTEM を指定 */
	StubWrite(STUB_FUNC(MiscCtrlApiDataSend), MISC_CTRL_RET_ERR_SYSTEM);
	ret = MiscCtrlApiDataSend(p_ccm_obj, data_type, send_data.data(), datasize);
	EXPECT_EQ(MISC_CTRL_RET_ERR_SYSTEM, ret);
	ClearFuncResult(STUB_FUNC(MiscCtrlApiDataSend));	/* エラークリア */

	/* 引数がNULL(1) */
	ret = MiscCtrlApiDataSend(nullptr, data_type, send_data.data(), datasize);
	EXPECT_EQ(MISC_CTRL_RET_ERR_PARAM, ret);

	/* 引数がNULL(2) */
	ret = MiscCtrlApiDataSend(p_ccm_obj, data_type, nullptr, datasize);
	EXPECT_EQ(MISC_CTRL_RET_ERR_PARAM, ret);

	/* 引数が0 */
	ret = MiscCtrlApiDataSend(p_ccm_obj, 0, send_data.data(), datasize);
	EXPECT_EQ(MISC_CTRL_RET_ERR_PARAM, ret);

	/* 引数が0 */
	ret = MiscCtrlApiDataSend(p_ccm_obj, data_type, send_data.data(), 0);
	EXPECT_EQ(MISC_CTRL_RET_ERR_PARAM, ret);

	/* _call調査(成功) */
	StubClientRegisterListerner(STUB_CALL(MiscCtrlApiDataSend), MiscCtrlApiDataSendListener);
	{
		detect_ntfy = true;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eDataSend]);
		ret = MiscCtrlApiDataSend(p_ccm_obj, data_type, send_data.data(), datasize);
		EXPECT_EQ(MISC_CTRL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eDataSend].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::no_timeout, intime);
	}

	/* _call調査(タイムアウト) */
	{
		detect_ntfy = false;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eDataSend]);
		ret = MiscCtrlApiDataSend(p_ccm_obj, data_type, send_data.data(), datasize);
		EXPECT_EQ(MISC_CTRL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eDataSend].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::timeout, intime);
	}

	/* 終了処理(Close) */
	ret = CcmApiClose(p_ccm_obj);
	EXPECT_EQ(CCM_RET_SUCCESS, ret);
}

TEST_F(CcmTest_Api, MiscCtrlApiEntStsCB)
{
	void*	p_ccm_obj = NULL;
	INT32	ret;

	/* 準備(Open) */
	ret = CcmApiOpen(&p_ccm_obj);
	ASSERT_EQ(CCM_RET_SUCCESS, ret);
	ASSERT_NE(nullptr, p_ccm_obj);

	/* 戻り値に何も指定せずに実施 */
	ret = MiscCtrlApiEntStsCB(p_ccm_obj, &MiscCtrlApiStsCB_control);
	EXPECT_EQ(MISC_CTRL_RET_SUCCESS, ret);

	/* 戻り値に MISC_CTRL_RET_ERR_OVER を指定 */
	StubWrite(STUB_FUNC(MiscCtrlApiEntStsCB), MISC_CTRL_RET_ERR_OVER);
	ret = MiscCtrlApiEntStsCB(p_ccm_obj, &MiscCtrlApiStsCB_control);
	EXPECT_EQ(MISC_CTRL_RET_ERR_OVER, ret);
	ClearFuncResult(STUB_FUNC(MiscCtrlApiEntStsCB));	/* エラークリア */

	/* 引数がNULL(1) */
	ret = MiscCtrlApiEntStsCB(nullptr, &MiscCtrlApiStsCB_control);
	EXPECT_EQ(MISC_CTRL_RET_ERR_PARAM, ret);

	/* 引数がNULL(2) */
	ret = MiscCtrlApiEntStsCB(p_ccm_obj, nullptr);
	EXPECT_EQ(MISC_CTRL_RET_ERR_PARAM, ret);

	/* _call調査(成功) */
	StubClientRegisterListerner(STUB_CALL(MiscCtrlApiEntStsCB), MiscCtrlApiEntStsCBListener);
	{
		detect_ntfy = true;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eEntStsCB]);
		ret = MiscCtrlApiEntStsCB(p_ccm_obj, &MiscCtrlApiStsCB_control);
		EXPECT_EQ(MISC_CTRL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eEntStsCB].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::no_timeout, intime);
	}

	/* _call調査(タイムアウト) */
	{
		detect_ntfy = false;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eEntStsCB]);
		ret = MiscCtrlApiEntStsCB(p_ccm_obj, &MiscCtrlApiStsCB_control);
		EXPECT_EQ(MISC_CTRL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eEntStsCB].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::timeout, intime);
	}

	/* 終了処理(Close) */
	ret = CcmApiClose(p_ccm_obj);
	EXPECT_EQ(CCM_RET_SUCCESS, ret);
}

TEST_F(CcmTest_Api, MiscCtrlApiDelStsCB)
{
	void*	p_ccm_obj = NULL;
	INT32	ret;
	
	/* 準備(Open) */
	ret = CcmApiOpen(&p_ccm_obj);
	ASSERT_EQ(CCM_RET_SUCCESS, ret);
	ASSERT_NE(nullptr, p_ccm_obj);

	/* 戻り値に何も指定せずに実施 */
	{
		detect_ntfy = true;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eEntStsCB]);
		ret = MiscCtrlApiEntStsCB(p_ccm_obj, &MiscCtrlApiStsCB_control);
		EXPECT_EQ(MISC_CTRL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eEntStsCB].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::no_timeout, intime);
	}

	/* 戻り値に何も指定せずに実施 */
	ret = MiscCtrlApiDelStsCB(p_ccm_obj);
	EXPECT_EQ(MISC_CTRL_RET_SUCCESS, ret);

	/* 戻り値に MISC_CTRL_RET_ERR_BUSY を指定 */
	StubWrite(STUB_FUNC(MiscCtrlApiDelStsCB), MISC_CTRL_RET_ERR_BUSY);
	ret = MiscCtrlApiDelStsCB(p_ccm_obj);
	EXPECT_EQ(MISC_CTRL_RET_ERR_BUSY, ret);
	ClearFuncResult(STUB_FUNC(MiscCtrlApiDelStsCB));	/* エラークリア */

	/* 引数がNULL */
	ret = MiscCtrlApiDelStsCB(nullptr);
	EXPECT_EQ(MISC_CTRL_RET_ERR_PARAM, ret);

	/* _call調査(成功) */
	StubClientRegisterListerner(STUB_CALL(MiscCtrlApiDelStsCB), MiscCtrlApiDelStsCBListener);
	{
		detect_ntfy = true;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eDelStsCB]);
		ret = MiscCtrlApiDelStsCB(p_ccm_obj);
		EXPECT_EQ(MISC_CTRL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eDelStsCB].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::no_timeout, intime);
	}

	/* _call調査(タイムアウト) */
	{
		detect_ntfy = false;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eDelStsCB]);
		ret = MiscCtrlApiDelStsCB(p_ccm_obj);
		EXPECT_EQ(MISC_CTRL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eDelStsCB].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::timeout, intime);
	}

	/* 終了処理(Close) */
	ret = CcmApiClose(p_ccm_obj);
	EXPECT_EQ(ret, CCM_RET_SUCCESS);
}

/*---------------------------------------------------- */
/* MiscCtrlApiEntDataNtyCB 関数の調査 */
TEST_F(CcmTest_Api, MiscCtrlApiEntDataNtyCB)
{
	void*	p_ccm_obj = NULL;
	INT32	ret;
	MiscCtrlApiNtyFilter filter{};

	/* 準備(Open) */
	ret = CcmApiOpen(&p_ccm_obj);
	ASSERT_EQ(CCM_RET_SUCCESS, ret);
	ASSERT_NE(nullptr, p_ccm_obj);

	/* 戻り値に何も指定せずに実施 */
	ret = MiscCtrlApiEntDataNtyCB(p_ccm_obj, &MiscCtrlApiDataNtyCB_control, &filter);
	EXPECT_EQ(MISC_CTRL_RET_SUCCESS, ret);

	/* 戻り値に MISC_CTRL_RET_ERR_OVER を指定 */
	StubWrite(STUB_FUNC(MiscCtrlApiEntDataNtyCB), MISC_CTRL_RET_ERR_OVER);
	ret = MiscCtrlApiEntDataNtyCB(p_ccm_obj, &MiscCtrlApiDataNtyCB_control, &filter);
	EXPECT_EQ(MISC_CTRL_RET_ERR_OVER, ret);
	ClearFuncResult(STUB_FUNC(MiscCtrlApiEntDataNtyCB));	/* エラークリア */

	/* 引数がNULL(1) */
	ret = MiscCtrlApiEntDataNtyCB(nullptr, &MiscCtrlApiDataNtyCB_control, &filter);
	EXPECT_EQ(MISC_CTRL_RET_ERR_PARAM, ret);

	/* 引数がNULL(2) */
	ret = MiscCtrlApiEntDataNtyCB(p_ccm_obj, nullptr, &filter);
	EXPECT_EQ(MISC_CTRL_RET_ERR_PARAM, ret);

	/* 引数がNULL(3) */
	ret = MiscCtrlApiEntDataNtyCB(p_ccm_obj, &MiscCtrlApiDataNtyCB_control, nullptr);
	EXPECT_EQ(MISC_CTRL_RET_ERR_PARAM, ret);

	/* _call調査(成功) */
	StubClientRegisterListerner(STUB_CALL(MiscCtrlApiEntDataNtyCB), MiscCtrlApiEntDataNtyCBListener);
	{
		detect_ntfy = true;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eEntDataNtyCB]);
		ret = MiscCtrlApiEntDataNtyCB(p_ccm_obj, &MiscCtrlApiDataNtyCB_control, &filter);
		EXPECT_EQ(MISC_CTRL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eEntDataNtyCB].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::no_timeout, intime);
	}

	/* _call調査(タイムアウト) */
	{
		detect_ntfy = false;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eEntDataNtyCB]);
		ret = MiscCtrlApiEntDataNtyCB(p_ccm_obj, &MiscCtrlApiDataNtyCB_control, &filter);
		EXPECT_EQ(MISC_CTRL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eEntDataNtyCB].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::timeout, intime);
	}

	/* 終了処理(Close) */
	ret = CcmApiClose(p_ccm_obj);
	EXPECT_EQ(CCM_RET_SUCCESS, ret);
}

/*---------------------------------------------------- */
/* MiscCtrlApiDelDataNtyCB 関数の調査 */
TEST_F(CcmTest_Api, MiscCtrlApiDelDataNtyCB)
{
	void*	p_ccm_obj = NULL;
	INT32	ret;
	MiscCtrlApiNtyFilter filter{};
	
	/* 準備(Open) */
	ret = CcmApiOpen(&p_ccm_obj);
	ASSERT_EQ(CCM_RET_SUCCESS, ret);
	ASSERT_NE(nullptr, p_ccm_obj);

	/* 戻り値に何も指定せずに実施(MiscCtrlApiEntDataNtyCBの実施) */
	{
		detect_ntfy = true;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eEntDataNtyCB]);
		ret = MiscCtrlApiEntDataNtyCB(p_ccm_obj, &MiscCtrlApiDataNtyCB_control, &filter);
		EXPECT_EQ(MISC_CTRL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eEntDataNtyCB].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::no_timeout, intime);
	}

	/* 戻り値に何も指定せずに実施 */
	ret = MiscCtrlApiDelDataNtyCB(p_ccm_obj);
	EXPECT_EQ(MISC_CTRL_RET_SUCCESS, ret);

	/* 戻り値にMISC_CTRL_RET_ERR_BUSYを指定 */
	StubWrite(STUB_FUNC(MiscCtrlApiDelDataNtyCB), MISC_CTRL_RET_ERR_BUSY);
	ret = MiscCtrlApiDelDataNtyCB(p_ccm_obj);
	EXPECT_EQ(MISC_CTRL_RET_ERR_BUSY, ret);
	ClearFuncResult(STUB_FUNC(MiscCtrlApiDelDataNtyCB));	/* エラークリア */

	/* 引数がNULL */
	ret = MiscCtrlApiDelDataNtyCB(nullptr);
	EXPECT_EQ(MISC_CTRL_RET_ERR_PARAM, ret);

	/* _call調査(成功) */
	StubClientRegisterListerner(STUB_CALL(MiscCtrlApiDelDataNtyCB), MiscCtrlApiDelDataNtyCBListener);
	{
		detect_ntfy = true;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eDelDataNtyCB]);
		ret = MiscCtrlApiDelDataNtyCB(p_ccm_obj);
		EXPECT_EQ(MISC_CTRL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eDelDataNtyCB].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::no_timeout, intime);
	}

	/* _call調査(タイムアウト) */
	{
		detect_ntfy = false;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eDelDataNtyCB]);
		ret = MiscCtrlApiDelDataNtyCB(p_ccm_obj);
		EXPECT_EQ(MISC_CTRL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eDelDataNtyCB].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::timeout, intime);
	}

	/* 終了処理(Close) */
	ret = CcmApiClose(p_ccm_obj);
	EXPECT_EQ(ret, CCM_RET_SUCCESS);
}

TEST_F(CcmTest_Api, MiscCtrlApiStsCB)
{
	void*	p_ccm_obj = NULL;
	INT32	ret;
	std::cv_status	intime;

	std::unique_lock<std::mutex> lock_syncEnt(mtx_sync[eEntStsCB]);
	std::unique_lock<std::mutex> lock_syncDel(mtx_sync[eDelStsCB]);
	std::unique_lock<std::mutex> lock_syncSts(mtx_sync[eStsCB]);

	ret = CcmApiOpen(&p_ccm_obj);
	ASSERT_EQ(CCM_RET_SUCCESS, ret);
	ASSERT_NE(nullptr, p_ccm_obj);

	/*コールバック登録*/
	detect_ntfy = true;
	detect_sts = true;
	ret = MiscCtrlApiEntStsCB(p_ccm_obj, &MiscCtrlApiStsCB_control);
	EXPECT_EQ(MISC_CTRL_RET_SUCCESS, ret);
	intime = cond_sync[eEntStsCB].wait_for(lock_syncEnt, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);

	/*MISC_CTRL_STS_EXEC が返される*/
	intime = cond_sync[eStsCB].wait_for(lock_syncSts, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
 	EXPECT_EQ(MISC_CTRL_STS_EXEC, cb_status.ctrl_sts);

	/*コールバック値を変更*/
	UINT32 status = MISC_CTRL_STS_NONE;
	StubWrite(STUB_PARAM_STS3(0, 1), status);	/*コールバック値を設定*/
	system("touch " STUB_PARAM_CTL3(0));	/*イベント発生*/
	intime = cond_sync[eStsCB].wait_for(lock_syncSts, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
 	EXPECT_EQ(status, cb_status.ctrl_sts);

	/*コールバック解除*/
	detect_ntfy = true;
	ret = MiscCtrlApiDelStsCB(p_ccm_obj);
	EXPECT_EQ(MISC_CTRL_RET_SUCCESS, ret);
	intime = cond_sync[eDelStsCB].wait_for(lock_syncDel, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);

	ret = CcmApiClose(p_ccm_obj);
	EXPECT_EQ(CCM_RET_SUCCESS, ret);
}

/*---------------------------------------------------- */
/* MiscCtrlApiDataNtyCBの各値結果調査 */
TEST_F(CcmTest_Api, MiscCtrlApiDataNtyCB)
{
	void*	p_ccm_obj = NULL;
	INT32	ret;
	std::cv_status	intime;
	MiscCtrlApiNtyFilter filter{};
	int val = 0;

	std::unique_lock<std::mutex> lock_syncEnt(mtx_sync[eEntDataNtyCB]);
	std::unique_lock<std::mutex> lock_syncDel(mtx_sync[eDelDataNtyCB]);
	std::unique_lock<std::mutex> lock_syncSts(mtx_sync[eDataNtyCB]);

	ret = CcmApiOpen(&p_ccm_obj);
	ASSERT_EQ(CCM_RET_SUCCESS, ret);
	ASSERT_NE(nullptr, p_ccm_obj);

	/*コールバック登録*/
	detect_ntfy = true;
	detect_sts = true;
	ret = MiscCtrlApiEntDataNtyCB(p_ccm_obj, &MiscCtrlApiDataNtyCB_control, &filter);
	EXPECT_EQ(MISC_CTRL_RET_SUCCESS, ret);
	intime = cond_sync[eEntDataNtyCB].wait_for(lock_syncEnt, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	intime = cond_sync[eDataNtyCB].wait_for(lock_syncSts, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);

	/*コールバック登録後にカメラ同期検知通知*/
	EXPECT_EQ(0x34, cb_data_type);	/*データタイプ*/
	EXPECT_EQ(6,    cb_len);		/*データサイズ*/
	EXPECT_EQ(0x3,  cb_buf[0]);		/*サブタイプ*/
	EXPECT_EQ(0x1,  cb_buf[1]);
	EXPECT_EQ(0x0,  cb_buf[2]);
	EXPECT_EQ(0x1,  cb_buf[3]);
	EXPECT_EQ(0x1,  cb_buf[4]);
	EXPECT_EQ(0x0,  cb_buf[5]);

	/*カメラ種別判別通知：初期値*/
	system("touch " STUB_PARAM_CTL2(0));
	intime = cond_sync[eDataNtyCB].wait_for(lock_syncSts, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	EXPECT_EQ(0x34, cb_data_type);	/*データタイプ*/
	EXPECT_EQ(6,    cb_len);		/*データサイズ*/
	EXPECT_EQ(0x2,  cb_buf[0]);		/*サブタイプ*/
	EXPECT_EQ(0x5,  cb_buf[1]);
	EXPECT_EQ(0x6,  cb_buf[2]);
	EXPECT_EQ(0x0,  cb_buf[3]);
	EXPECT_EQ(0x0,  cb_buf[4]);
	EXPECT_EQ(0x0,  cb_buf[5]);

	/*カメラ同期検知通知：初期値*/
	system("touch " STUB_PARAM_CTL2(1));
	intime = cond_sync[eDataNtyCB].wait_for(lock_syncSts, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	EXPECT_EQ(0x34, cb_data_type);
	EXPECT_EQ(6,    cb_len);
	EXPECT_EQ(0x3,  cb_buf[0]);
	EXPECT_EQ(0x1,  cb_buf[1]);
	EXPECT_EQ(0x0,  cb_buf[2]);
	EXPECT_EQ(0x1,  cb_buf[3]);
	EXPECT_EQ(0x1,  cb_buf[4]);
	EXPECT_EQ(0x0,  cb_buf[5]);

	/*HDMI接続検知：初期値*/
	system("touch " STUB_PARAM_CTL2(2));
	intime = cond_sync[eDataNtyCB].wait_for(lock_syncSts, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	EXPECT_EQ(0x39, cb_data_type);
	EXPECT_EQ(2,    cb_len);
	EXPECT_EQ(0x1,  cb_buf[0]);
	EXPECT_EQ(0x2,  cb_buf[1]);

	/*HDMIビデオフォーマット通知：初期値*/
	system("touch " STUB_PARAM_CTL2(3));
	intime = cond_sync[eDataNtyCB].wait_for(lock_syncSts, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	EXPECT_EQ(0x39, cb_data_type);
	EXPECT_EQ(2,    cb_len);
	EXPECT_EQ(0x2,  cb_buf[0]);
	EXPECT_EQ(0x1,  cb_buf[1]);

	/*HDMIオーディオフォーマット通知：初期値*/
	system("touch " STUB_PARAM_CTL2(4));
	intime = cond_sync[eDataNtyCB].wait_for(lock_syncSts, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	EXPECT_EQ(0x39, cb_data_type);
	EXPECT_EQ(2,    cb_len);
	EXPECT_EQ(0x3,  cb_buf[0]);
	EXPECT_EQ(0x3,  cb_buf[1]);

	/*カメラ種別判別通知：変更値*/
	StubWrite(STUB_PARAM_STS2(0, 1), val + 1);
	StubWrite(STUB_PARAM_STS2(0, 2), val + 2);
	StubWrite(STUB_PARAM_STS2(0, 3), val + 3);
	StubWrite(STUB_PARAM_STS2(0, 4), val + 4);
	StubWrite(STUB_PARAM_STS2(0, 5), val + 5);
	system("touch " STUB_PARAM_CTL2(0));
	intime = cond_sync[eDataNtyCB].wait_for(lock_syncSts, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	EXPECT_EQ(0x34,    cb_data_type);
	EXPECT_EQ(6,       cb_len);
	EXPECT_EQ(0x2,     cb_buf[0]);
	EXPECT_EQ(val + 1, cb_buf[1]);
	EXPECT_EQ(val + 2, cb_buf[2]);
	EXPECT_EQ(val + 3, cb_buf[3]);
	EXPECT_EQ(val + 4, cb_buf[4]);
	EXPECT_EQ(val + 5, cb_buf[5]);

	/*カメラ同期検知通知：変更値*/
	StubWrite(STUB_PARAM_STS2(1, 1), val + 6);
	StubWrite(STUB_PARAM_STS2(1, 2), val + 7);
	StubWrite(STUB_PARAM_STS2(1, 3), val + 8);
	StubWrite(STUB_PARAM_STS2(1, 4), val + 9);
	StubWrite(STUB_PARAM_STS2(1, 5), val + 10);
	system("touch " STUB_PARAM_CTL2(1));
	intime = cond_sync[eDataNtyCB].wait_for(lock_syncSts, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	EXPECT_EQ(0x34,	    cb_data_type);
	EXPECT_EQ(6,        cb_len);
	EXPECT_EQ(0x3,      cb_buf[0]);
	EXPECT_EQ(val + 6,  cb_buf[1]);
	EXPECT_EQ(val + 7,  cb_buf[2]);
	EXPECT_EQ(val + 8,  cb_buf[3]);
	EXPECT_EQ(val + 9,  cb_buf[4]);
	EXPECT_EQ(val + 10, cb_buf[5]);

	/*HDMI接続検知：変更値*/
	val += 11;
	StubWrite(STUB_PARAM_STS2(2, 1), val);
	system("touch " STUB_PARAM_CTL2(2));
	intime = cond_sync[eDataNtyCB].wait_for(lock_syncSts, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	EXPECT_EQ(0x39, cb_data_type);
	EXPECT_EQ(2,    cb_len);
	EXPECT_EQ(0x1,  cb_buf[0]);
	EXPECT_EQ(val,  cb_buf[1]);

	/*HDMIビデオフォーマット通知：変更値*/
	val ++;
	StubWrite(STUB_PARAM_STS2(3, 1), val);
	system("touch " STUB_PARAM_CTL2(3));
	intime = cond_sync[eDataNtyCB].wait_for(lock_syncSts, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	EXPECT_EQ(0x39, cb_data_type);
	EXPECT_EQ(2,    cb_len);
	EXPECT_EQ(0x2,  cb_buf[0]);
	EXPECT_EQ(val,  cb_buf[1]);

	/*HDMIオーディオフォーマット通知：変更値*/
	val ++;
	StubWrite(STUB_PARAM_STS2(4, 1), val);
	system("touch " STUB_PARAM_CTL2(4));
	intime = cond_sync[eDataNtyCB].wait_for(lock_syncSts, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	EXPECT_EQ(0x39, cb_data_type);
	EXPECT_EQ(2,    cb_len);
	EXPECT_EQ(0x3,  cb_buf[0]);
	EXPECT_EQ(val,  cb_buf[1]);

	/* 3秒周期の通知発生調査 */
	printf("wait 6sec...\n");
	/* 3秒後 */
	StubWrite(STUB_PARAM_STS2(0, 1), val + 1); /*カメラ種別判別通知*/
	StubWrite(STUB_PARAM_STS2(0, 2), val + 2);
	StubWrite(STUB_PARAM_STS2(0, 3), val + 3);
	StubWrite(STUB_PARAM_STS2(0, 4), val + 4);
	StubWrite(STUB_PARAM_STS2(0, 5), val + 5);
	StubWrite(STUB_PARAM_STS2(1, 1), val + 6); /*カメラ同期検知通知*/
	StubWrite(STUB_PARAM_STS2(1, 2), val + 7);
	StubWrite(STUB_PARAM_STS2(1, 3), val + 8);
	StubWrite(STUB_PARAM_STS2(1, 4), val + 9);
	StubWrite(STUB_PARAM_STS2(1, 5), val + 10);

#if 0 /* カメラ種別判別要求送信のMiscCtrlApiDataSend()をしていないため、カメラ種別判別通知のCBは発生しません */
	/*カメラ種別判別通知*/
	intime = cond_sync[eDataNtyCB].wait_for(lock_syncSts, std::chrono::milliseconds(5000));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	EXPECT_EQ(0x34,    cb_data_type);
	EXPECT_EQ(6,       cb_len);
	EXPECT_EQ(0x2,     cb_buf[0]);
	EXPECT_EQ(val + 1, cb_buf[1]);
	// EXPECT_EQ(val + 2, cb_buf[2]);
	// EXPECT_EQ(val + 3, cb_buf[3]);
	// EXPECT_EQ(val + 4, cb_buf[4]);
	// EXPECT_EQ(val + 5, cb_buf[5]);
#endif

	/*カメラ同期検知通知*/
	 intime = cond_sync[eDataNtyCB].wait_for(lock_syncSts, std::chrono::milliseconds(5000));
	 EXPECT_EQ(std::cv_status::no_timeout, intime);
	 EXPECT_EQ(0x34,     cb_data_type);
	 EXPECT_EQ(6,        cb_len);
	 EXPECT_EQ(0x3,      cb_buf[0]);
	 EXPECT_EQ(val + 6,  cb_buf[1]);
	 EXPECT_EQ(val + 7,  cb_buf[2]);
	 EXPECT_EQ(val + 8,  cb_buf[3]);
	 EXPECT_EQ(val + 9,  cb_buf[4]);
	 EXPECT_EQ(val + 10, cb_buf[5]);

	/* カメラ種別判別要求送信 */
	uint8_t senddata{0x01};	/* 01h：カメラ種別判別要求 */
	ret = MiscCtrlApiDataSend(p_ccm_obj, 0x34U, &senddata, 1U);
	EXPECT_EQ(MISC_CTRL_RET_SUCCESS, ret);
	/*カメラ種別判別通知*/
	intime = cond_sync[eDataNtyCB].wait_for(lock_syncSts, std::chrono::milliseconds(5000));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	EXPECT_EQ(0x34,    cb_data_type);
	EXPECT_EQ(6,       cb_len);
	EXPECT_EQ(0x2,     cb_buf[0]);

	/* さらに3秒後 */
	val += 10;
	StubWrite(STUB_PARAM_STS2(0, 1), val + 1); /*カメラ種別判別通知*/
	StubWrite(STUB_PARAM_STS2(0, 2), val + 2);
	StubWrite(STUB_PARAM_STS2(0, 3), val + 3);
	StubWrite(STUB_PARAM_STS2(0, 4), val + 4);
	StubWrite(STUB_PARAM_STS2(0, 5), val + 5);
	StubWrite(STUB_PARAM_STS2(1, 1), val + 6); /*カメラ同期検知通知*/
	StubWrite(STUB_PARAM_STS2(1, 2), val + 7);
	StubWrite(STUB_PARAM_STS2(1, 3), val + 8);
	StubWrite(STUB_PARAM_STS2(1, 4), val + 9);
	StubWrite(STUB_PARAM_STS2(1, 5), val + 10);

	/*カメラ種別判別通知*/
	intime = cond_sync[eDataNtyCB].wait_for(lock_syncSts, std::chrono::milliseconds(5000));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	EXPECT_EQ(0x34,    cb_data_type);
	EXPECT_EQ(6,       cb_len);
	EXPECT_EQ(0x2,     cb_buf[0]);
	EXPECT_EQ(val + 1, cb_buf[1]);
	EXPECT_EQ(val + 2, cb_buf[2]);
	EXPECT_EQ(val + 3, cb_buf[3]);
	EXPECT_EQ(val + 4, cb_buf[4]);
	EXPECT_EQ(val + 5, cb_buf[5]);

	/*カメラ同期検知通知*/
	intime = cond_sync[eDataNtyCB].wait_for(lock_syncSts, std::chrono::milliseconds(5000));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	EXPECT_EQ(0x34,     cb_data_type);
	EXPECT_EQ(6,        cb_len);
	EXPECT_EQ(0x3,      cb_buf[0]);
	EXPECT_EQ(val + 6,  cb_buf[1]);
	EXPECT_EQ(val + 7,  cb_buf[2]);
	EXPECT_EQ(val + 8,  cb_buf[3]);
	EXPECT_EQ(val + 9,  cb_buf[4]);
	EXPECT_EQ(val + 10, cb_buf[5]);

	/*コールバック解除*/
	detect_ntfy = true;
	ret = MiscCtrlApiDelDataNtyCB(p_ccm_obj);
	EXPECT_EQ(MISC_CTRL_RET_SUCCESS, ret);
	intime = cond_sync[eDelDataNtyCB].wait_for(lock_syncDel, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);

	ret = CcmApiClose(p_ccm_obj);
	EXPECT_EQ(CCM_RET_SUCCESS, ret);
}
