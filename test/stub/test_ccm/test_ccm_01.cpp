
#include <unistd.h>
#include <time.h>
#include "test_main.h"
#include "file_observer.h"
#include "com_stddef.h"
#include "stub_common.h"
#include <condition_variable>

extern "C" {
#include "ccm_public.h"
}

#define STUB_PATH					"/run/arene/vehicle_fs/var/bev3/stub/ccm_client/"
#define STUB_PATH_FUNC				STUB_PATH "func_ret/"
#define STUB_FUNC(_fnc)				STUB_PATH_FUNC #_fnc
#define STUB_PATH_CB				STUB_PATH "TabCtrlApiPwCmdNtyCB/"
#define STUB_PARAM_STS(_no, _itm)	STUB_PATH_CB #_no "_" #_itm "_param"
#define STUB_PARAM_CTL(_no)			STUB_PATH_CB #_no "_control"
#define STUB_CALL(_fnc)				STUB_PATH #_fnc "_call"


#if 0
namespace
{

void PrintMsg(std::string msg, const std::experimental::source_location location = std::experimental::source_location::current())
{
	std::string absolute_path = location.file_name();
	std::size_t dir_pos = absolute_path.find_last_of("/");
	std::string file_name = absolute_path.substr(dir_pos + 1);
	std::cout << "[DEBUG][" << file_name << ":" << location.line() << 
		"][" << location.function_name() << "] " << msg << std::endl;
}

}
#endif

enum eTab {
	eTabSend ,
	eTabEntCB ,
	eTabDelCB ,
	eLs1Send ,
	eTabSts ,
	eTabMAX
};

static std::mutex mtx_sync[eTabMAX];
static std::condition_variable cond_sync[eTabMAX];
static bool detect_ntfy = false;
static bool detect_sts  = false;
static bool first_run = true;
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
		system("rm -fr " STUB_PATH_CB);
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

/*---------------------------------------------------- */
static void StubWrite(const char* path, const void* ptr)
{
	cockpit::bs::CFileObserver::GetInstance()->Write(path, (void*)&ptr, sizeof(ptr));
}

/*---------------------------------------------------- */
static void StubWrite(const char* path, const void* ptr, size_t size)
{
	cockpit::bs::CFileObserver::GetInstance()->Write(path, ptr, size);
}

/*---------------------------------------------------- */
/* TabCtrlApiPwCmdSendのcall時に実施するCB */
static void TabCtrlApiPwCmdSendListener(const char* p_path)
{
	if (true == detect_ntfy)
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync[eTabSend]);
		cond_sync[eTabSend].notify_one();
	}
}

/*---------------------------------------------------- */
/* TabCtrlApiEntPwCmdNtyCBのcall時に実施するCB */
static void TabCtrlApiEntPwCmdNtyCBListener(const char* p_path)
{
	if (true == detect_ntfy)
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync[eTabEntCB]);
		cond_sync[eTabEntCB].notify_one();
	}
}

/*---------------------------------------------------- */
/* TabCtrlApiDelPwCmdNtyCBのcall時に実施するCB */
static void TabCtrlApiDelPwCmdNtyCBListener(const char* p_path)
{
	if (true == detect_ntfy)
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync[eTabDelCB]);
		cond_sync[eTabDelCB].notify_one();
	}
}

/*---------------------------------------------------- */
/* Ls1CtrlApiCmdSendのcall時に実施するCB */
static void Ls1CtrlApiCmdSendListener(const char* p_path)
{
	if (true == detect_ntfy)
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync[eLs1Send]);
		cond_sync[eLs1Send].notify_one();
	}
}

/*---------------------------------------------------- */
/* control更新時に実施するCBまたは3秒周期で呼ばれるCB */
static void TabCtrlApiPwCmdNtyCB_control(void* p_cmd, UINT32 len)
{
	if (true == detect_sts)
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync[eTabSts]);
		if (len <= sizeof(cb_buf))
		{
			memcpy(cb_buf, p_cmd, len);
			cb_len = len;
		}
		cond_sync[eTabSts].notify_one();
	}
}


//
// Googletest TestCase
//
/*---------------------------------------------------- */
/* CcmApiOpen + CcmApiClose関数の調査 */
TEST_F(CcmTest_Api, CcmApiOpen_Close)
{
	void*	p_ccm_obj = NULL;
	INT32	ret;

	/* 戻り値に何も指定せずに実施(Open後即Close) */
	ret = CcmApiOpen(&p_ccm_obj);
	EXPECT_EQ(CCM_RET_SUCCESS, ret);
	EXPECT_NE(nullptr, p_ccm_obj);
	ret = CcmApiClose(p_ccm_obj);
	EXPECT_EQ(CCM_RET_SUCCESS, ret);

	/* 戻り値に何も指定せずに実施(Open後、一定時間待ってClose) */
	ret = CcmApiOpen(&p_ccm_obj);
	EXPECT_EQ(CCM_RET_SUCCESS, ret);
	EXPECT_NE(nullptr, p_ccm_obj);
	usleep(200000);			/* 200ミリ秒wait */
	ret = CcmApiClose(p_ccm_obj);
	EXPECT_EQ(CCM_RET_SUCCESS, ret);

	/* Closeの戻り値にCCM_RET_ERR_STATEを指定 */
	StubWrite(STUB_FUNC(CcmApiClose), CCM_RET_ERR_STATE);
	ret = CcmApiOpen(&p_ccm_obj);
	EXPECT_EQ(CCM_RET_SUCCESS, ret);
	EXPECT_NE(nullptr, p_ccm_obj);
	ret = CcmApiClose(p_ccm_obj);
	EXPECT_EQ(CCM_RET_ERR_STATE, ret);
	ClearFuncResult(STUB_FUNC(CcmApiClose));	/* エラークリア */

	/* Closeの戻り値にCCM_RET_SUCCESSを指定 */
	StubWrite(STUB_FUNC(CcmApiClose), CCM_RET_SUCCESS);
	ret = CcmApiClose(p_ccm_obj);
	EXPECT_EQ(CCM_RET_SUCCESS, ret);

	/* 引数がNULL(Open) */
	ret = CcmApiOpen(nullptr);
	EXPECT_EQ(CCM_RET_ERR_PARAM, ret);

	/* 引数がNULL(Close) */
	ret = CcmApiClose(nullptr);
	EXPECT_EQ(CCM_RET_ERR_PARAM, ret);
}

/*---------------------------------------------------- */
/* TabCtrlApiPwCmdSend関数の調査 */
TEST_F(CcmTest_Api, TabCtrlApiPwCmdSend)
{
	void*	p_ccm_obj = NULL;
	INT32	ret;
	void*	p_cmd = (void*)&ret;
	UINT32	len = (UINT32)sizeof(ret);
	
	/* 準備(Open) */
	ret = CcmApiOpen(&p_ccm_obj);
	ASSERT_EQ(CCM_RET_SUCCESS, ret);
	ASSERT_NE(nullptr, p_ccm_obj);

	/* 戻り値に何も指定せずに実施 */
	ret = TabCtrlApiPwCmdSend(p_ccm_obj, p_cmd, len);
	EXPECT_EQ(TAB_CTRL_RET_SUCCESS, ret);

	/* 戻り値にTAB_CTRL_RET_ERR_SYSTEMを指定 */
	StubWrite(STUB_FUNC(TabCtrlApiPwCmdSend), TAB_CTRL_RET_ERR_SYSTEM);
	ret = TabCtrlApiPwCmdSend(p_ccm_obj, p_cmd, len);
	EXPECT_EQ(TAB_CTRL_RET_ERR_SYSTEM, ret);
	ClearFuncResult(STUB_FUNC(TabCtrlApiPwCmdSend));	/* エラークリア */

	/* 引数がNULL(1) */
	ret = TabCtrlApiPwCmdSend(nullptr, p_cmd, len);
	EXPECT_EQ(TAB_CTRL_RET_ERR_PARAM, ret);

	/* 引数がNULL(2) */
	ret = TabCtrlApiPwCmdSend(p_ccm_obj, nullptr, len);
	EXPECT_EQ(TAB_CTRL_RET_ERR_PARAM, ret);

	/* 引数が0 */
	ret = TabCtrlApiPwCmdSend(p_ccm_obj, p_cmd, 0);
	EXPECT_EQ(TAB_CTRL_RET_ERR_PARAM, ret);

	/* _call調査(成功) */
	StubClientRegisterListerner(STUB_CALL(TabCtrlApiPwCmdSend), TabCtrlApiPwCmdSendListener);
	{
		detect_ntfy = true;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eTabSend]);
		ret = TabCtrlApiPwCmdSend(p_ccm_obj, p_cmd, len);
		EXPECT_EQ(TAB_CTRL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eTabSend].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::no_timeout, intime);
	}

	/* _call調査(タイムアウト) */
	{
		detect_ntfy = false;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eTabSend]);
		ret = TabCtrlApiPwCmdSend(p_ccm_obj, p_cmd, len);
		EXPECT_EQ(TAB_CTRL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eTabSend].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::timeout, intime);
	}

	/* 終了処理(Close) */
	ret = CcmApiClose(p_ccm_obj);
	EXPECT_EQ(CCM_RET_SUCCESS, ret);

}

/*---------------------------------------------------- */
/* TabCtrlApiEntPwCmdNtyCB関数の調査 */
TEST_F(CcmTest_Api, TabCtrlApiEntPwCmdNtyCB)
{
	void*	p_ccm_obj = NULL;
	INT32	ret;

	/* 準備(Open) */
	ret = CcmApiOpen(&p_ccm_obj);
	ASSERT_EQ(CCM_RET_SUCCESS, ret);
	ASSERT_NE(nullptr, p_ccm_obj);

	/* 戻り値に何も指定せずに実施 */
	ret = TabCtrlApiEntPwCmdNtyCB(p_ccm_obj, &TabCtrlApiPwCmdNtyCB_control, nullptr);
	EXPECT_EQ(TAB_CTRL_RET_SUCCESS, ret);

	/* 戻り値にTAB_CTRL_RET_ERR_OVERを指定 */
	StubWrite(STUB_FUNC(TabCtrlApiEntPwCmdNtyCB), TAB_CTRL_RET_ERR_OVER);
	ret = TabCtrlApiEntPwCmdNtyCB(p_ccm_obj, &TabCtrlApiPwCmdNtyCB_control, nullptr);
	EXPECT_EQ(TAB_CTRL_RET_ERR_OVER, ret);
	ClearFuncResult(STUB_FUNC(TabCtrlApiEntPwCmdNtyCB));	/* エラークリア */

	/* 引数がNULL(1) */
	ret = TabCtrlApiEntPwCmdNtyCB(nullptr, &TabCtrlApiPwCmdNtyCB_control, nullptr);
	EXPECT_EQ(TAB_CTRL_RET_ERR_PARAM, ret);

	/* 引数がNULL(2) */
	ret = TabCtrlApiEntPwCmdNtyCB(p_ccm_obj, nullptr, nullptr);
	EXPECT_EQ(TAB_CTRL_RET_ERR_PARAM, ret);

	/* _call調査(成功) */
	StubClientRegisterListerner(STUB_CALL(TabCtrlApiEntPwCmdNtyCB), TabCtrlApiEntPwCmdNtyCBListener);
	{
		detect_ntfy = true;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eTabEntCB]);
		ret = TabCtrlApiEntPwCmdNtyCB(p_ccm_obj, &TabCtrlApiPwCmdNtyCB_control, nullptr);
		EXPECT_EQ(TAB_CTRL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eTabEntCB].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::no_timeout, intime);
	}

	/* _call調査(タイムアウト) */
	{
		detect_ntfy = false;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eTabEntCB]);
		ret = TabCtrlApiEntPwCmdNtyCB(p_ccm_obj, &TabCtrlApiPwCmdNtyCB_control, nullptr);
		EXPECT_EQ(TAB_CTRL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eTabEntCB].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::timeout, intime);
	}

	/* 終了処理(Close) */
	ret = CcmApiClose(p_ccm_obj);
	EXPECT_EQ(CCM_RET_SUCCESS, ret);
}

/*---------------------------------------------------- */
/* TabCtrlApiDelPwCmdNtyCB関数の調査 */
TEST_F(CcmTest_Api, TabCtrlApiDelPwCmdNtyCB)
{
	void*	p_ccm_obj = NULL;
	INT32	ret;
	
	/* 準備(Open) */
	ret = CcmApiOpen(&p_ccm_obj);
	ASSERT_EQ(CCM_RET_SUCCESS, ret);
	ASSERT_NE(nullptr, p_ccm_obj);

	/* 戻り値に何も指定せずに実施(TabCtrlApiEntPwCmdNtyCBの実施) */
	{
		detect_ntfy = true;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eTabEntCB]);
		ret = TabCtrlApiEntPwCmdNtyCB(p_ccm_obj, &TabCtrlApiPwCmdNtyCB_control, nullptr);
		EXPECT_EQ(TAB_CTRL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eTabEntCB].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::no_timeout, intime);
	}

	/* 戻り値に何も指定せずに実施 */
	ret = TabCtrlApiDelPwCmdNtyCB(p_ccm_obj);
	EXPECT_EQ(TAB_CTRL_RET_SUCCESS, ret);

	/* 戻り値にTAB_CTRL_RET_ERR_BUSYを指定 */
	StubWrite(STUB_FUNC(TabCtrlApiDelPwCmdNtyCB), TAB_CTRL_RET_ERR_BUSY);
	ret = TabCtrlApiDelPwCmdNtyCB(p_ccm_obj);
	EXPECT_EQ(TAB_CTRL_RET_ERR_BUSY, ret);
	ClearFuncResult(STUB_FUNC(TabCtrlApiDelPwCmdNtyCB));	/* エラークリア */

	/* 引数がNULL */
	ret = TabCtrlApiDelPwCmdNtyCB(nullptr);
	EXPECT_EQ(TAB_CTRL_RET_ERR_PARAM, ret);

	/* _call調査(成功) */
	StubClientRegisterListerner(STUB_CALL(TabCtrlApiDelPwCmdNtyCB), TabCtrlApiDelPwCmdNtyCBListener);
	{
		detect_ntfy = true;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eTabDelCB]);
		ret = TabCtrlApiDelPwCmdNtyCB(p_ccm_obj);
		EXPECT_EQ(TAB_CTRL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eTabDelCB].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::no_timeout, intime);
	}

	/* _call調査(タイムアウト) */
	{
		detect_ntfy = false;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eTabDelCB]);
		ret = TabCtrlApiDelPwCmdNtyCB(p_ccm_obj);
		EXPECT_EQ(TAB_CTRL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eTabDelCB].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::timeout, intime);
	}

	/* 終了処理(Close) */
	ret = CcmApiClose(p_ccm_obj);
	EXPECT_EQ(ret, CCM_RET_SUCCESS);
}

/*---------------------------------------------------- */
/* Ls1CtrlApiCmdSend関数の調査 */
TEST_F(CcmTest_Api, Ls1CtrlApiCmdSend)
{
	void*	p_ccm_obj = NULL;
	INT32	ret;
	void*	p_cmd = (void*)&ret;
	UINT32	len = (UINT32)sizeof(ret);
	
	/* 準備(Open) */
	ret = CcmApiOpen(&p_ccm_obj);
	ASSERT_EQ(CCM_RET_SUCCESS, ret);
	ASSERT_NE(nullptr, p_ccm_obj);

	/* 戻り値に何も指定せずに実施 */
	ret = Ls1CtrlApiCmdSend(p_ccm_obj, p_cmd, len);
	EXPECT_EQ(LS1_CTRL_RET_SUCCESS, ret);

	/* 戻り値にLS1_CTRL_RET_ERR_RESOURCEを指定 */
	StubWrite(STUB_FUNC(Ls1CtrlApiCmdSend), LS1_CTRL_RET_ERR_RESOURCE);
	ret = Ls1CtrlApiCmdSend(p_ccm_obj, p_cmd, len);
	EXPECT_EQ(LS1_CTRL_RET_ERR_RESOURCE, ret);
	ClearFuncResult(STUB_FUNC(Ls1CtrlApiCmdSend));	/* エラークリア */

	/* 引数がNULL(1) */
	ret = Ls1CtrlApiCmdSend(nullptr, p_cmd, len);
	EXPECT_EQ(LS1_CTRL_RET_ERR_PARAM, ret);

	/* 引数がNULL(2) */
	ret = Ls1CtrlApiCmdSend(p_ccm_obj, nullptr, len);
	EXPECT_EQ(LS1_CTRL_RET_ERR_PARAM, ret);

	/* 引数が0 */
	ret = Ls1CtrlApiCmdSend(p_ccm_obj, p_cmd, 0);
	EXPECT_EQ(LS1_CTRL_RET_ERR_PARAM, ret);

	/* _call調査(成功) */
	StubClientRegisterListerner(STUB_CALL(Ls1CtrlApiCmdSend), Ls1CtrlApiCmdSendListener);
	{
		detect_ntfy = true;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eLs1Send]);
		ret = Ls1CtrlApiCmdSend(p_ccm_obj, p_cmd, len);
		EXPECT_EQ(LS1_CTRL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eLs1Send].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::no_timeout, intime);
	}

	/* _call調査(タイムアウト) */
	{
		detect_ntfy = false;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eLs1Send]);
		ret = Ls1CtrlApiCmdSend(p_ccm_obj, p_cmd, len);
		EXPECT_EQ(LS1_CTRL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eLs1Send].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::timeout, intime);
	}

	/* 終了処理(Close) */
	ret = CcmApiClose(p_ccm_obj);
	EXPECT_EQ(CCM_RET_SUCCESS, ret);
}

/*---------------------------------------------------- */
/* TabCtrlApiPwCmdNtyCBの各値結果調査 */
TEST_F(CcmTest_Api, TabCtrlApiPwCmdNtyCB)
{
	void*	p_ccm_obj = NULL;
	INT32	ret;
	std::cv_status	intime;

	std::unique_lock<std::mutex> lock_syncEnt(mtx_sync[eTabEntCB]);
	std::unique_lock<std::mutex> lock_syncDel(mtx_sync[eTabDelCB]);
	std::unique_lock<std::mutex> lock_syncSts(mtx_sync[eTabSts]);

	/* 準備(Open) */
	ret = CcmApiOpen(&p_ccm_obj);
	ASSERT_EQ(CCM_RET_SUCCESS, ret);
	ASSERT_NE(nullptr, p_ccm_obj);

	/* controlのCB登録 */
	/* CB登録時に同期検知・経路情報通知(BEYE/FEYE)のCBがあること */
	detect_ntfy = true;
	detect_sts = true;
	ret = TabCtrlApiEntPwCmdNtyCB(p_ccm_obj, &TabCtrlApiPwCmdNtyCB_control, nullptr);
	EXPECT_EQ(TAB_CTRL_RET_SUCCESS, ret);
	intime = cond_sync[eTabEntCB].wait_for(lock_syncEnt, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	intime = cond_sync[eTabSts].wait_for(lock_syncSts, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	EXPECT_EQ(5, cb_len);
	EXPECT_EQ(0x17, cb_buf[0]);
	EXPECT_EQ(0x01, cb_buf[1]);
	EXPECT_EQ(0x01, cb_buf[2]);
	EXPECT_EQ(0x01, cb_buf[3]);
	EXPECT_EQ(0x01, cb_buf[4]);

	/* 各controlを更新し、各値の確認 */

	/* 同期検知・経路情報通知(BEYE/FEYE) */
	system("touch " STUB_PARAM_CTL(0));
	intime = cond_sync[eTabSts].wait_for(lock_syncSts, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	EXPECT_EQ(5, cb_len);
	EXPECT_EQ(0x17, cb_buf[0]);
	EXPECT_EQ(0x01, cb_buf[1]);
	EXPECT_EQ(0x01, cb_buf[2]);
	EXPECT_EQ(0x01, cb_buf[3]);
	EXPECT_EQ(0x01, cb_buf[4]);
	/* カメラ種別判別通知 */
	system("touch " STUB_PARAM_CTL(1));
	intime = cond_sync[eTabSts].wait_for(lock_syncSts, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	EXPECT_EQ(5, cb_len);
	EXPECT_EQ(0x19, cb_buf[0]);
	EXPECT_EQ(0x05, cb_buf[1]);
	EXPECT_EQ(0x06, cb_buf[2]);
	EXPECT_EQ(0x00, cb_buf[3]);
	EXPECT_EQ(0x00, cb_buf[4]);
	/* 映像MUTE確認 */
	system("touch " STUB_PARAM_CTL(2));
	intime = cond_sync[eTabSts].wait_for(lock_syncSts, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	EXPECT_EQ(2, cb_len);
	EXPECT_EQ(0x62, cb_buf[0]);
	EXPECT_EQ(0x00, cb_buf[1]);
	/* 映像MUTE状態通知 */
	system("touch " STUB_PARAM_CTL(3));
	intime = cond_sync[eTabSts].wait_for(lock_syncSts, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	EXPECT_EQ(2, cb_len);
	EXPECT_EQ(0x63, cb_buf[0]);
	EXPECT_EQ(0x00, cb_buf[1]);

	/* 各値の書き換え */
	/* 同期検知・経路情報通知(BEYE/FEYE) */
	StubWrite(STUB_PARAM_STS(0, 0), 0xAAA0);
	StubWrite(STUB_PARAM_STS(0, 1), 0xAAA1);
	StubWrite(STUB_PARAM_STS(0, 2), 0xAAA2);
	StubWrite(STUB_PARAM_STS(0, 3), 0xAAA3);
	StubWrite(STUB_PARAM_STS(0, 4), 0xAAA4);
	/* カメラ種別判別通知 */
	StubWrite(STUB_PARAM_STS(1, 0), 0xBBB0);
	StubWrite(STUB_PARAM_STS(1, 1), 0xBBB1);
	StubWrite(STUB_PARAM_STS(1, 2), 0xBBB2);
	StubWrite(STUB_PARAM_STS(1, 3), 0xBBB3);
	StubWrite(STUB_PARAM_STS(1, 4), 0xBBB4);
	/* 映像MUTE確認 */
	StubWrite(STUB_PARAM_STS(2, 0), 0xCCC0);
	StubWrite(STUB_PARAM_STS(2, 1), 0xCCC1);
	/* 映像MUTE状態通知 */
	StubWrite(STUB_PARAM_STS(3, 0), 0xDDD0);
	StubWrite(STUB_PARAM_STS(3, 1), 0xDDD1);

	/* 更新後の値確認 */
	/* 同期検知・経路情報通知(BEYE/FEYE) */
	system("touch " STUB_PARAM_CTL(0));
	intime = cond_sync[eTabSts].wait_for(lock_syncSts, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	EXPECT_EQ(5, cb_len);
	EXPECT_EQ(0x17, cb_buf[0]);		/* OPCは更新されないこと */
	EXPECT_EQ(0xA1, cb_buf[1]);
	EXPECT_EQ(0xA2, cb_buf[2]);
	EXPECT_EQ(0xA3, cb_buf[3]);
	EXPECT_EQ(0xA4, cb_buf[4]);
	/* カメラ種別判別通知 */
	system("touch " STUB_PARAM_CTL(1));
	intime = cond_sync[eTabSts].wait_for(lock_syncSts, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	EXPECT_EQ(5, cb_len);
	EXPECT_EQ(0x19, cb_buf[0]);		/* OPCは更新されないこと */
	EXPECT_EQ(0xB1, cb_buf[1]);
	EXPECT_EQ(0xB2, cb_buf[2]);
	EXPECT_EQ(0xB3, cb_buf[3]);
	EXPECT_EQ(0xB4, cb_buf[4]);
	/* 映像MUTE確認 */
	system("touch " STUB_PARAM_CTL(2));
	intime = cond_sync[eTabSts].wait_for(lock_syncSts, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	EXPECT_EQ(2, cb_len);
	EXPECT_EQ(0x62, cb_buf[0]);		/* OPCは更新されないこと */
	EXPECT_EQ(0xC1, cb_buf[1]);
	/* 映像MUTE状態通知 */
	system("touch " STUB_PARAM_CTL(3));
	intime = cond_sync[eTabSts].wait_for(lock_syncSts, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	EXPECT_EQ(2, cb_len);
	EXPECT_EQ(0x63, cb_buf[0]);		/* OPCは更新されないこと */
	EXPECT_EQ(0xD1, cb_buf[1]);

	/* 3秒周期の通知発生調査 */
	/* 対象は、カメラ種別判別通知と映像MUTE状態通知 */
	printf("wait 6sec...\n");
	/* 3秒後 */
	StubWrite(STUB_PARAM_STS(1, 2), 0x01);
	StubWrite(STUB_PARAM_STS(3, 1), 0x02);
	intime = cond_sync[eTabSts].wait_for(lock_syncSts, std::chrono::milliseconds(5000));
	EXPECT_EQ(std::cv_status::no_timeout, intime);

	EXPECT_EQ(0x19, cb_buf[0]);
	EXPECT_EQ(0x01, cb_buf[2]);
	intime = cond_sync[eTabSts].wait_for(lock_syncSts, std::chrono::milliseconds(5000));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	EXPECT_EQ(0x63, cb_buf[0]);
	EXPECT_EQ(0x02, cb_buf[1]);

	/* さらに3秒後 */
	StubWrite(STUB_PARAM_STS(1, 1), 0xE1);
	StubWrite(STUB_PARAM_STS(1, 2), 0xE2);
	StubWrite(STUB_PARAM_STS(1, 3), 0xE3);
	StubWrite(STUB_PARAM_STS(1, 4), 0xE4);
	StubWrite(STUB_PARAM_STS(3, 1), 0xF1);
	intime = cond_sync[eTabSts].wait_for(lock_syncSts, std::chrono::milliseconds(5000));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	EXPECT_EQ(0x19, cb_buf[0]);
	EXPECT_EQ(0xE1, cb_buf[1]);
	EXPECT_EQ(0xE2, cb_buf[2]);
	EXPECT_EQ(0xE3, cb_buf[3]);
	EXPECT_EQ(0xE4, cb_buf[4]);

	intime = cond_sync[eTabSts].wait_for(lock_syncSts, std::chrono::milliseconds(5000));
	EXPECT_EQ(std::cv_status::no_timeout, intime);
	EXPECT_EQ(0x63, cb_buf[0]);
	EXPECT_EQ(0xF1, cb_buf[1]);

	/* CB登録解除 */
	detect_ntfy = true;
	ret = TabCtrlApiDelPwCmdNtyCB(p_ccm_obj);
	EXPECT_EQ(TAB_CTRL_RET_SUCCESS, ret);
	intime = cond_sync[eTabDelCB].wait_for(lock_syncDel, std::chrono::milliseconds(200));
	EXPECT_EQ(std::cv_status::no_timeout, intime);

	/* 終了処理(Close) */
	ret = CcmApiClose(p_ccm_obj);
	EXPECT_EQ(CCM_RET_SUCCESS, ret);
}

/*---------------------------------------------------- */
