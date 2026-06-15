
//#include <string>
#include <unistd.h>
#include "test_main.h"
#include "file_observer.h"
#include "com_stddef.h"
#include "stub_common.h"
#include <condition_variable>

extern "C" {
#include "ls3_ctl_api_public.h"
#include "sif_evt.h"
}

#define DEF_STRING(_str)		#_str
#define STUB_PATH				"/run/arene/vehicle_fs/var/bev3/stub/ls3ctl/"
#define STUB_PATH_FUNC			STUB_PATH "func_ret/"
#define STUB_FUNC(_fnc)			STUB_PATH_FUNC DEF_STRING(_fnc)
#define STUB_CTRL				STUB_PATH "Ls3CtlApiCmdClbk_control"
#define STUB_APIDAT_OPTYPE		STUB_PATH "Ls3CtlApiDat_OPType_status"
#define STUB_STS(_fnc)			STUB_PATH DEF_STRING(_fnc) "_status"
#define STUB_CALL(_fnc)			STUB_PATH DEF_STRING(_fnc) "_call"


/* DeviceID */
#define APOLICY_LS3_CMD_DEVICE_ID				(0x0fff)				/* 送信先ノードアドレス */

/* FBlockID＋InstID */
#define APOLICY_LS3_CMD_FBLOCK_ID_FR_AUD_MASTER	(0x22)					/* FrontAudioAmplifier */
#define APOLICY_LS3_CMD_INST_ID_FR_AUD_MASTER	(0x60)					/* FrontAudioAmplifier */
#define APOLICY_LS3_CMD_FBLOCK_ID_CONN_MASTER	(0x03)					/* ConnectionMaster */
#define APOLICY_LS3_CMD_INST_ID_CONN_MASTER		(0x00)					/* ConnectionMaster */

/* FktID */
#define APOLICY_LS3_CMD_FKT_ID_MUTE				(0x113)					/* FrontAudioAmplifier.Mute.SetGet */
#define APOLICY_LS3_CMD_FKT_ID_CONNECT			(0xC00)					/* ConnectionMaster.ExBuildSyncConnection.StartResult */
#define APOLICY_LS3_CMD_FKT_ID_DISCONNECT		(0x201)					/* ConnectionMaster.RemoveSyncConnection.StartResult */
#define APOLICY_LS3_CMD_FKT_ID_INTAUDIO			(0xE02)					/* FrontAudioAmplifier.IntAudioOutChannels.SetGet */

/* OPType */
#define APOLICY_LS3_CMD_OP_TYPE_SETGET			(0x02)					/* MOST OperationType SetGet */
#define APOLICY_LS3_CMD_OP_TYPE_START_RESULT	(0x02)					/* MOST OperationType StartResult */
#define APOLICY_LS3_CMD_OP_TYPE_STATUS_RESULT	(0x0C)					/* MOST OperationType Status/Result */
#define APOLICY_LS3_CMD_OP_TYPE_STATUS_ERROR	(0x0F)					/* MOST OperationType Error */
#define APOLICY_LS3_CMD_OP_TYPE_STATUS_BUSY		(0x0B)					/* MOST OperationType Processing */

/* Event Code */
#define AM_API_EVT_SUB_OK		(0x01)
#define AM_API_EVT_SUB_NG		(0x02)
#define AM_API_EVT_SUB_BUSY		(0x04)
#define AM_API_EVT_SUB			(AM_API_EVT_SUB_OK | AM_API_EVT_SUB_NG | AM_API_EVT_SUB_BUSY)
#define AM_SUB_TIMEOUT (500)


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

enum eLs3 {
	eLs3Open ,
	eLs3Close ,
	eLs3Send ,
	eLs3AddCB ,
	eLs3DelCB ,
	eLs3MAX
};

static std::mutex mtx_sync[eLs3MAX];
static std::condition_variable cond_sync[eLs3MAX];
static bool detect_ntfy = false;
static bool first_run = true;

static sif_evt_id_t	sifEvt_ = nullptr;
static INT32	subChkFktID = {0};
static Ls3CtlApiDat		ApiDat_Result_ = {};

class Ls3CtlTest_Api: public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();

};

void Ls3CtlTest_Api::SetUp()
{
	if (true == first_run)
	{
		first_run = false;
		system("rm -fr " STUB_PATH_FUNC);
	}
	sifEvt_ = sif_evt_open(NULL);
}

void Ls3CtlTest_Api::TearDown()
{
	sif_evt_close(sifEvt_);
	sifEvt_ = nullptr;
}

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

static void StubWrite(const char* path, const INT32 val)
{
	cockpit::bs::CFileObserver::GetInstance()->Write(path, &val, sizeof(val));
}

static void StubWrite(const char* path, const void* ptr)
{
	cockpit::bs::CFileObserver::GetInstance()->Write(path, (void*)&ptr, sizeof(ptr));
}

static void StubWrite(const char* path, const void* ptr, size_t size)
{
	cockpit::bs::CFileObserver::GetInstance()->Write(path, ptr, size);
}

static void TabCtrlApiCommonListener(eLs3 idx)
{
	if (true == detect_ntfy)
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync[idx]);
		cond_sync[idx].notify_one();
	}
}

/* Ls3CtlApiOpenのcall時に実施するCB */
static void Ls3CtlApiOpenListener(const char* p_path)
{
	TabCtrlApiCommonListener(eLs3Open);
}

/* Ls3CtlApiCloseのcall時に実施するCB */
static void Ls3CtlApiCloseListener(const char* p_path)
{
	TabCtrlApiCommonListener(eLs3Close);
}

/* Ls3CtlApiSndCmdのcall時に実施するCB */
static void Ls3CtlApiSndCmdListener(const char* p_path)
{
	TabCtrlApiCommonListener(eLs3Send);
}

/* Ls3CtlApiCmdAddCallbackのcall時に実施するCB */
static void Ls3CtlApiCmdAddCallbackListener(const char* p_path)
{
	TabCtrlApiCommonListener(eLs3AddCB);
}

/* Ls3CtlApiCmdDelCallbackのcall時に実施するCB */
static void Ls3CtlApiCmdDelCallbackListener(const char* p_path)
{
	TabCtrlApiCommonListener(eLs3DelCB);
}


/* コールバック関数 */
void Ls3CtlApiCmdClbkFunc(INT32 event, UINT8 FBlockID ,UINT8 InstID, Ls3CtlApiDat* data)
{
	(void)FBlockID;
	(void)InstID;
	/* FBlockID、InstIDはコールバック登録時に指定のため、送信時と同じFktIDで	*/
	/* かつ、受信応答が正常/異常の場合に内部イベントを発行する。				*/
	if( (data != nullptr) && 
		(event == LS3_CTL_EVENT_CMD_RCV) && 
		(subChkFktID == data->FktID) && 
		( (data->OPType == APOLICY_LS3_CMD_OP_TYPE_STATUS_RESULT) ||
		  (data->OPType == APOLICY_LS3_CMD_OP_TYPE_STATUS_ERROR)  ||
		  (data->OPType == APOLICY_LS3_CMD_OP_TYPE_STATUS_BUSY) ))
	{
		/* イベント応答待ちイベント発行 */
		sif_evt_code_t evtCode =
				(data->OPType == APOLICY_LS3_CMD_OP_TYPE_STATUS_RESULT) ? AM_API_EVT_SUB_OK :
				(data->OPType == APOLICY_LS3_CMD_OP_TYPE_STATUS_ERROR) ? AM_API_EVT_SUB_NG : AM_API_EVT_SUB_BUSY;
		ApiDat_Result_ = *data;
		sif_evt_set(sifEvt_, evtCode);
	}
}

//
// Googletest TestCase
//
/*---------------------------------------------------- */
TEST_F(Ls3CtlTest_Api, Ls3CtlApiOpen)
{
	Ls3CtlApiObj ifObj = {};
	INT32	ret;

	/* 戻り値に何も指定せずに実施 */
	ret = Ls3CtlApiOpen(&ifObj);
	EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);

	/* 引数がNULL */
	ret = Ls3CtlApiOpen(nullptr);
	EXPECT_EQ(LS3_CTL_RET_ERR_PARAM, ret);

	/* _call調査(成功) */
	StubClientRegisterListerner(STUB_CALL(Ls3CtlApiOpen), Ls3CtlApiOpenListener);
	{
		detect_ntfy = true;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eLs3Open]);
		ret = Ls3CtlApiOpen(&ifObj);
		EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eLs3Open].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::no_timeout, intime);
	}

	/* _call調査(タイムアウト) */
	{
		detect_ntfy = false;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eLs3Open]);
		ret = Ls3CtlApiOpen(&ifObj);
		EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eLs3Open].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::timeout, intime);
	}
	StubClientUnRegisterListerner(STUB_CALL(Ls3CtlApiOpen));
}

/*---------------------------------------------------- */
TEST_F(Ls3CtlTest_Api, Ls3CtlApiClose)
{
	Ls3CtlApiObj ifObj = {};
	INT32	ret;

	/* 戻り値に何も指定せずに実施 */
	ret = Ls3CtlApiClose(&ifObj);
	EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);

	/* 戻り値にLS3_CTL_RET_ERR_ERRを指定 */
	StubWrite(STUB_FUNC(Ls3CtlApiClose), LS3_CTL_RET_ERR_ERR);
	ret = Ls3CtlApiClose(&ifObj);
	EXPECT_EQ(LS3_CTL_RET_ERR_ERR, ret);

	/* 戻り値にLS3_CTL_RET_SUCCESSを指定 */
	StubWrite(STUB_FUNC(Ls3CtlApiClose), LS3_CTL_RET_SUCCESS);
	ret = Ls3CtlApiClose(&ifObj);
	EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);

	/* 引数がNULL */
	ret = Ls3CtlApiClose(nullptr);
	EXPECT_EQ(LS3_CTL_RET_ERR_PARAM, ret);

	/* _call調査(成功) */
	StubClientRegisterListerner(STUB_CALL(Ls3CtlApiClose), Ls3CtlApiCloseListener);
	{
		detect_ntfy = true;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eLs3Close]);
		ret = Ls3CtlApiClose(&ifObj);
		EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eLs3Close].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::no_timeout, intime);
	}

	/* _call調査(タイムアウト) */
	{
		detect_ntfy = false;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eLs3Close]);
		ret = Ls3CtlApiClose(&ifObj);
		EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eLs3Close].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::timeout, intime);
	}
	StubClientUnRegisterListerner(STUB_CALL(Ls3CtlApiClose));
}

/*---------------------------------------------------- */
TEST_F(Ls3CtlTest_Api, Ls3CtlApiCmdAddCallback)
{
	Ls3CtlApiObj ifObj = {};
	INT32	ret;

	/* 戻り値に何も指定せずに実施 */
	ret = Ls3CtlApiCmdAddCallback(&ifObj, 1, 2, &Ls3CtlApiCmdClbkFunc);
	EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);

	/* 戻り値にLS3_CTL_RET_ERR_ERRを指定 */
	StubWrite(STUB_FUNC(Ls3CtlApiCmdAddCallback), LS3_CTL_RET_ERR_ERR);
	ret = Ls3CtlApiCmdAddCallback(&ifObj, 1, 2, &Ls3CtlApiCmdClbkFunc);
	EXPECT_EQ(LS3_CTL_RET_ERR_ERR, ret);

	/* 戻り値にLS3_CTL_RET_SUCCESSを指定 */
	StubWrite(STUB_FUNC(Ls3CtlApiCmdAddCallback), LS3_CTL_RET_SUCCESS);
	ret = Ls3CtlApiCmdAddCallback(&ifObj, 1, 2, &Ls3CtlApiCmdClbkFunc);
	EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);

	/* 引数がNULL(1) */
	ret = Ls3CtlApiCmdAddCallback(nullptr, 1, 2, &Ls3CtlApiCmdClbkFunc);
	EXPECT_EQ(LS3_CTL_RET_ERR_PARAM, ret);

	/* 引数がNULL(2) */
	ret = Ls3CtlApiCmdAddCallback(&ifObj, 1, 2, nullptr);
	EXPECT_EQ(LS3_CTL_RET_ERR_PARAM, ret);

	/* _call調査(成功) */
	StubClientRegisterListerner(STUB_CALL(Ls3CtlApiCmdAddCallback), Ls3CtlApiCmdAddCallbackListener);
	{
		detect_ntfy = true;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eLs3AddCB]);
		ret = Ls3CtlApiCmdAddCallback(&ifObj, 1, 2, &Ls3CtlApiCmdClbkFunc);
		EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eLs3AddCB].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::no_timeout, intime);
	}

	/* _call調査(タイムアウト) */
	{
		detect_ntfy = false;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eLs3AddCB]);
		ret = Ls3CtlApiCmdAddCallback(&ifObj, 1, 2, &Ls3CtlApiCmdClbkFunc);
		EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eLs3AddCB].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::timeout, intime);
	}
	StubClientUnRegisterListerner(STUB_CALL(Ls3CtlApiCmdAddCallback));
}

/*---------------------------------------------------- */
TEST_F(Ls3CtlTest_Api, Ls3CtlApiCmdDelCallback)
{
	Ls3CtlApiObj ifObj = {};
	INT32	ret;

	/* 戻り値に何も指定せずに実施 */
	ret = Ls3CtlApiCmdDelCallback(&ifObj, 1, 2);
	EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);

	/* 戻り値にLS3_CTL_RET_ERR_ERRを指定 */
	StubWrite(STUB_FUNC(Ls3CtlApiCmdDelCallback), LS3_CTL_RET_ERR_ERR);
	ret = Ls3CtlApiCmdDelCallback(&ifObj, 1, 2);
	EXPECT_EQ(LS3_CTL_RET_ERR_ERR, ret);

	/* 戻り値にLS3_CTL_RET_SUCCESSを指定 */
	StubWrite(STUB_FUNC(Ls3CtlApiCmdDelCallback), LS3_CTL_RET_SUCCESS);
	ret = Ls3CtlApiCmdDelCallback(&ifObj, 1, 2);
	EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);

	/* 引数がNULL */
	ret = Ls3CtlApiCmdDelCallback(nullptr, 1, 2);
	EXPECT_EQ(LS3_CTL_RET_ERR_PARAM, ret);

	/* _call調査(成功) */
	StubClientRegisterListerner(STUB_CALL(Ls3CtlApiCmdDelCallback), Ls3CtlApiCmdDelCallbackListener);
	{
		detect_ntfy = true;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eLs3DelCB]);
		ret = Ls3CtlApiCmdDelCallback(&ifObj, 1, 2);
		EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eLs3DelCB].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::no_timeout, intime);
	}

	/* _call調査(タイムアウト) */
	{
		detect_ntfy = false;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eLs3DelCB]);
		ret = Ls3CtlApiCmdDelCallback(&ifObj, 1, 2);
		EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eLs3DelCB].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::timeout, intime);
	}
	StubClientUnRegisterListerner(STUB_CALL(Ls3CtlApiCmdDelCallback));
}

/*---------------------------------------------------- */
TEST_F(Ls3CtlTest_Api, Ls3CtlApiGetSystemState)
{
	Ls3CtlApiObj ifObj = {};
	Ls3CtlApiSystemState systemState = {};
	INT32	ret;

	/* 戻り値に何も指定せずに実施 */
	ret = Ls3CtlApiGetSystemState(&ifObj, &systemState);
	EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);
	EXPECT_EQ(LS3_CTL_FRONTAVMASTER_SYSTEM_OK, systemState.FrontAVMasterState);

	/* 戻り値にLS3_CTL_RET_ERR_ERRを指定 */
	StubWrite(STUB_FUNC(Ls3CtlApiGetSystemState), LS3_CTL_RET_ERR_ERR);
	ret = Ls3CtlApiGetSystemState(&ifObj, &systemState);
	EXPECT_EQ(LS3_CTL_RET_ERR_ERR, ret);
	ClearFuncResult(STUB_FUNC(Ls3CtlApiGetSystemState));		/* エラークリア */

	/* 戻り値にLS3_CTL_RET_SUCCESSを指定 */
	/* FrontAVMasterStateはLS3_CTL_FRONTAVMASTER_SYSTEM_NOTOKを返す */
	StubWrite(STUB_FUNC(Ls3CtlApiGetSystemState), LS3_CTL_RET_SUCCESS);
	StubWrite(STUB_STS(Ls3CtlApiGetSystemState), LS3_CTL_FRONTAVMASTER_SYSTEM_NOTOK);
	ret = Ls3CtlApiGetSystemState(&ifObj, &systemState);
	EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);
	EXPECT_EQ(LS3_CTL_FRONTAVMASTER_SYSTEM_NOTOK, systemState.FrontAVMasterState);

	/* 戻り値にLS3_CTL_RET_SUCCESSを指定 */
	/* FrontAVMasterStateはLS3_CTL_FRONTAVMASTER_SYSTEM_OKを返す */
	StubWrite(STUB_FUNC(Ls3CtlApiGetSystemState), LS3_CTL_RET_SUCCESS);
	StubWrite(STUB_STS(Ls3CtlApiGetSystemState), LS3_CTL_FRONTAVMASTER_SYSTEM_OK);
	ret = Ls3CtlApiGetSystemState(&ifObj, &systemState);
	EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);
	EXPECT_EQ(LS3_CTL_FRONTAVMASTER_SYSTEM_OK, systemState.FrontAVMasterState);

	/* 引数がNULL(1) */
	ret = Ls3CtlApiGetSystemState(nullptr, &systemState);
	EXPECT_EQ(LS3_CTL_RET_ERR_PARAM, ret);

	/* 引数がNULL(2) */
	ret = Ls3CtlApiGetSystemState(&ifObj, nullptr);
	EXPECT_EQ(LS3_CTL_RET_ERR_PARAM, ret);
}

/*---------------------------------------------------- */
/* Ls3CtlApiDatの初期化1 (FktID = 0x103) */
static Ls3CtlApiDat& InitLs3CtlApiDat()
{
	static Ls3CtlApiDat cmdDat = {};
	cmdDat.DeviceID		= LS3_CTL_REGISTRATION_DEVICEID;
	cmdDat.FBlockID		= 0x00;
	cmdDat.InstID		= 0x00;
	cmdDat.FktID		= 0x0103;
	cmdDat.OPType 		= 0x02;		/* コールバック呼び出し時に0x0Cになる */
	cmdDat.DataType		= LS3_CTL_TRANSFER_MEMORY;
	cmdDat.Length		= 2;
/*	cmdDat.Data[0]		= 0x00; */
/*	cmdDat.Data[1]		= 0x00; */
	return cmdDat;
}

/*---------------------------------------------------- */
/* Ls3CtlApiDatの初期化2 (FktID = 0xDF8) */
static Ls3CtlApiDat& InitLs3CtlApiDat2()
{
	static Ls3CtlApiDat cmdDat = {};
	cmdDat.DeviceID		= LS3_CTL_REGISTRATION_DEVICEID;
	cmdDat.FBlockID		= 0x00;
	cmdDat.InstID		= 0x00;
	cmdDat.FktID		= 0x0DF8;
	cmdDat.OPType 		= 0x02;		/* コールバック呼び出し時に0x0Cになる */
	cmdDat.DataType		= LS3_CTL_TRANSFER_MEMORY;
	cmdDat.Length		= 16;
/*	cmdDat.Data[0]		= 0x00; */
/*	cmdDat.Data[1]		= 0x00; */
	return cmdDat;
}

/*---------------------------------------------------- */
/* Ls3CtlApiSndCmd 関数エラー応答系 */
TEST_F(Ls3CtlTest_Api, Ls3CtlApiSndCmd_1)
{
	Ls3CtlApiObj ifObj = {};
	Ls3CtlApiDat cmdDat = InitLs3CtlApiDat();
	INT32	ret;

	/* 引数がNULL(1) */
	ret = Ls3CtlApiSndCmd(nullptr, &cmdDat);
	EXPECT_EQ(LS3_CTL_RET_ERR_PARAM, ret);

	/* 引数がNULL(2) */
	ret = Ls3CtlApiSndCmd(&ifObj, nullptr);
	EXPECT_EQ(LS3_CTL_RET_ERR_PARAM, ret);

	/* 戻り値にLS3_CTL_RET_ERR_ERRを指定 */
	StubWrite(STUB_FUNC(Ls3CtlApiSndCmd), LS3_CTL_RET_ERR_ERR);
	ret = Ls3CtlApiSndCmd(&ifObj, &cmdDat);
	EXPECT_EQ(LS3_CTL_RET_ERR_ERR, ret);
	ClearFuncResult(STUB_FUNC(Ls3CtlApiSndCmd));		/* エラークリア */
}

/*---------------------------------------------------- */
/* Ls3CtlApiSndCmd 処理失敗1(CB非登録) */
TEST_F(Ls3CtlTest_Api, Ls3CtlApiSndCmd_2)
{
	Ls3CtlApiObj ifObj = {};
	Ls3CtlApiDat cmdDat = InitLs3CtlApiDat();
	sif_evt_code_t evtCode = 0;
	INT32	ret;

	/* 戻り値に何も指定せずに実施 */
	subChkFktID = cmdDat.FktID;
	ret = Ls3CtlApiSndCmd(&ifObj, &cmdDat);
	EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);

	/* Ls3CtlApiSndCmd成功後はcontrolのトリガを発生させる */
	system("touch " STUB_CTRL);

	/* イベント応答待ち */
	ret = sif_evt_waitAny(sifEvt_, AM_API_EVT_SUB, AM_SUB_TIMEOUT, &evtCode);
	EXPECT_EQ(ETIMEDOUT, ret);
}

/*---------------------------------------------------- */
/* Ls3CtlApiSndCmd 処理失敗2(FktID不一致) */
TEST_F(Ls3CtlTest_Api, Ls3CtlApiSndCmd_3)
{
	Ls3CtlApiObj ifObj = {};
	Ls3CtlApiDat cmdDat = InitLs3CtlApiDat();
	sif_evt_code_t evtCode = 0;
	INT32	ret;

	/* CBを登録 */
	ret = Ls3CtlApiCmdAddCallback(&ifObj, 1, 2, &Ls3CtlApiCmdClbkFunc);
	EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);

	/* 戻り値に何も指定せずに実施 */
	subChkFktID = cmdDat.FktID + 1;		/* FktIDが不一致 */
	ret = Ls3CtlApiSndCmd(&ifObj, &cmdDat);
	EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);

	/* Ls3CtlApiSndCmd成功後はcontrolのトリガを発生させる */
	system("touch " STUB_CTRL);

	/* イベント応答待ち */
	ret = sif_evt_waitAny(sifEvt_, AM_API_EVT_SUB, AM_SUB_TIMEOUT, &evtCode);
	EXPECT_EQ(ETIMEDOUT, ret);

	/* CBを解除 */
	ret = Ls3CtlApiCmdDelCallback(&ifObj, 1, 2);
	EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);
}

/*---------------------------------------------------- */
/* Ls3CtlApiSndCmd 正常系(AM_API_EVT_SUB_OK受信FktID(0x103)) */
TEST_F(Ls3CtlTest_Api, Ls3CtlApiSndCmd_4)
{
	Ls3CtlApiObj ifObj = {};
	Ls3CtlApiDat cmdDat = InitLs3CtlApiDat();
	sif_evt_code_t evtCode = 0;
	INT32	ret;

	/* CBを登録 */
	ret = Ls3CtlApiCmdAddCallback(&ifObj, 1, 2, &Ls3CtlApiCmdClbkFunc);
	EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);

	/* 戻り値に何も指定せずに実施 */
	subChkFktID = cmdDat.FktID;
	ret = Ls3CtlApiSndCmd(&ifObj, &cmdDat);
	EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);

	/* Ls3CtlApiSndCmd成功後はcontrolのトリガを発生させる */
	system("touch " STUB_CTRL);

	/* イベント応答待ち */
	ret = sif_evt_waitAny(sifEvt_, AM_API_EVT_SUB, AM_SUB_TIMEOUT, &evtCode);
	EXPECT_EQ(0, ret);
	EXPECT_EQ(AM_API_EVT_SUB_OK, evtCode);
	EXPECT_EQ(0x00,   ApiDat_Result_.FBlockID);
	EXPECT_EQ(0x00,   ApiDat_Result_.InstID);
	EXPECT_EQ(0x0103, ApiDat_Result_.FktID);
	EXPECT_EQ(0x0C,   ApiDat_Result_.OPType);
	EXPECT_EQ(2,      ApiDat_Result_.Length);

	/* CBを解除 */
	ret = Ls3CtlApiCmdDelCallback(&ifObj, 1, 2);
	EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);

	/* _call調査(成功) */
	StubClientRegisterListerner(STUB_CALL(Ls3CtlApiSndCmd), Ls3CtlApiSndCmdListener);
	{
		detect_ntfy = true;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eLs3Send]);
		ret = Ls3CtlApiSndCmd(&ifObj, &cmdDat);
		EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eLs3Send].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::no_timeout, intime);
	}

	/* _call調査(タイムアウト) */
	{
		detect_ntfy = false;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eLs3Send]);
		ret = Ls3CtlApiSndCmd(&ifObj, &cmdDat);
		EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);
		std::cv_status intime = cond_sync[eLs3Send].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::timeout, intime);
	}
	StubClientUnRegisterListerner(STUB_CALL(Ls3CtlApiSndCmd));
}

/*---------------------------------------------------- */
/* Ls3CtlApiSndCmd 正常系(AM_API_EVT_SUB_NG受信) */
TEST_F(Ls3CtlTest_Api, Ls3CtlApiSndCmd_5)
{
	Ls3CtlApiObj ifObj = {};
	Ls3CtlApiDat cmdDat = InitLs3CtlApiDat();
	sif_evt_code_t evtCode = 0;
	INT32	ret;

	/* CBを登録 */
	ret = Ls3CtlApiCmdAddCallback(&ifObj, 1, 2, &Ls3CtlApiCmdClbkFunc);
	EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);

	/* 戻り値にLS3_CTL_RET_SUCCESSを指定 */
	StubWrite(STUB_FUNC(Ls3CtlApiSndCmd), LS3_CTL_RET_SUCCESS);
	StubWrite(STUB_APIDAT_OPTYPE, APOLICY_LS3_CMD_OP_TYPE_STATUS_ERROR);
	subChkFktID = cmdDat.FktID;
	ret = Ls3CtlApiSndCmd(&ifObj, &cmdDat);
	EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);

	/* Ls3CtlApiSndCmd成功後はcontrolのトリガを発生させる */
	system("touch " STUB_CTRL);

	/* イベント応答待ち */
	ret = sif_evt_waitAny(sifEvt_, AM_API_EVT_SUB, AM_SUB_TIMEOUT, &evtCode);
	EXPECT_EQ(0, ret);
	EXPECT_EQ(AM_API_EVT_SUB_NG, evtCode);
	EXPECT_EQ(0x00,   ApiDat_Result_.FBlockID);
	EXPECT_EQ(0x00,   ApiDat_Result_.InstID);
	EXPECT_EQ(0x0103, ApiDat_Result_.FktID);
	EXPECT_EQ(0x0F,   ApiDat_Result_.OPType);		/* NG */
	EXPECT_EQ(2,      ApiDat_Result_.Length);

	/* CBを解除 */
	ret = Ls3CtlApiCmdDelCallback(&ifObj, 1, 2);
	EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);
}

/*---------------------------------------------------- */
/* Ls3CtlApiSndCmd 正常系(AM_API_EVT_SUB_BUSY受信) */
TEST_F(Ls3CtlTest_Api, Ls3CtlApiSndCmd_6)
{
	Ls3CtlApiObj ifObj = {};
	Ls3CtlApiDat cmdDat = InitLs3CtlApiDat();
	sif_evt_code_t evtCode = 0;
	INT32	ret;

	/* CBを登録 */
	ret = Ls3CtlApiCmdAddCallback(&ifObj, 1, 2, &Ls3CtlApiCmdClbkFunc);
	EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);

	/* 戻り値にLS3_CTL_RET_SUCCESSを指定 */
	StubWrite(STUB_FUNC(Ls3CtlApiSndCmd), LS3_CTL_RET_SUCCESS);
	StubWrite(STUB_APIDAT_OPTYPE, APOLICY_LS3_CMD_OP_TYPE_STATUS_BUSY);
	subChkFktID = cmdDat.FktID;
	ret = Ls3CtlApiSndCmd(&ifObj, &cmdDat);
	EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);

	/* Ls3CtlApiSndCmd成功後はcontrolのトリガを発生させる */
	system("touch " STUB_CTRL);

	/* イベント応答待ち */
	ret = sif_evt_waitAny(sifEvt_, AM_API_EVT_SUB, AM_SUB_TIMEOUT, &evtCode);
	EXPECT_EQ(0, ret);
	EXPECT_EQ(AM_API_EVT_SUB_BUSY, evtCode);
	EXPECT_EQ(0x00,   ApiDat_Result_.FBlockID);
	EXPECT_EQ(0x00,   ApiDat_Result_.InstID);
	EXPECT_EQ(0x0103, ApiDat_Result_.FktID);
	EXPECT_EQ(0x0B,   ApiDat_Result_.OPType);		/* BUSY */
	EXPECT_EQ(2,      ApiDat_Result_.Length);

	/* CBを解除 */
	ret = Ls3CtlApiCmdDelCallback(&ifObj, 1, 2);
	EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);
}


/*---------------------------------------------------- */
/* Ls3CtlApiSndCmd 正常系(AM_API_EVT_SUB_OK受信:FktID(0xDF8)) */
TEST_F(Ls3CtlTest_Api, Ls3CtlApiSndCmd_7)
{
	Ls3CtlApiObj ifObj = {};
	Ls3CtlApiDat cmdDat = InitLs3CtlApiDat2();
	sif_evt_code_t evtCode = 0;
	INT32	ret;

	/* CBを登録 */
	ret = Ls3CtlApiCmdAddCallback(&ifObj, 1, 2, &Ls3CtlApiCmdClbkFunc);
	EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);

	/* 戻り値にLS3_CTL_RET_SUCCESSを指定 */
	StubWrite(STUB_FUNC(Ls3CtlApiSndCmd), LS3_CTL_RET_SUCCESS);
	StubWrite(STUB_APIDAT_OPTYPE, APOLICY_LS3_CMD_OP_TYPE_STATUS_RESULT);
	subChkFktID = cmdDat.FktID;
	ret = Ls3CtlApiSndCmd(&ifObj, &cmdDat);
	EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);

	/* Ls3CtlApiSndCmd成功後はcontrolのトリガを発生させる */
	system("touch " STUB_CTRL);

	/* イベント応答待ち */
	ret = sif_evt_waitAny(sifEvt_, AM_API_EVT_SUB, AM_SUB_TIMEOUT, &evtCode);
	EXPECT_EQ(0, ret);
	EXPECT_EQ(AM_API_EVT_SUB_OK, evtCode);
	EXPECT_EQ(0x00,   ApiDat_Result_.FBlockID);
	EXPECT_EQ(0x00,   ApiDat_Result_.InstID);
	EXPECT_EQ(0x0DF8, ApiDat_Result_.FktID);
	EXPECT_EQ(0x0C,   ApiDat_Result_.OPType);
	EXPECT_EQ(16,     ApiDat_Result_.Length);

	/* _call調査(成功) */
	StubClientRegisterListerner(STUB_CALL(Ls3CtlApiSndCmd), Ls3CtlApiSndCmdListener);
	{
		detect_ntfy = true;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eLs3Send]);
		ret = Ls3CtlApiSndCmd(&ifObj, &cmdDat);
		EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);
		/* Ls3CtlApiSndCmd成功後はcontrolのトリガを発生させる */
		system("touch " STUB_CTRL);

		std::cv_status intime = cond_sync[eLs3Send].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::no_timeout, intime);

		/* イベント応答待ち */
		ret = sif_evt_waitAny(sifEvt_, AM_API_EVT_SUB, AM_SUB_TIMEOUT, &evtCode);
		EXPECT_EQ(0, ret);
		EXPECT_EQ(AM_API_EVT_SUB_OK, evtCode);
		EXPECT_EQ(0x00,   ApiDat_Result_.FBlockID);
		EXPECT_EQ(0x00,   ApiDat_Result_.InstID);
		EXPECT_EQ(0x0DF8, ApiDat_Result_.FktID);
		EXPECT_EQ(0x0C,   ApiDat_Result_.OPType);
		EXPECT_EQ(16,     ApiDat_Result_.Length);
	}

	/* _call調査(タイムアウト) */
	{
		detect_ntfy = false;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eLs3Send]);
		ret = Ls3CtlApiSndCmd(&ifObj, &cmdDat);
		EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);
		/* Ls3CtlApiSndCmd成功後はcontrolのトリガを発生させる */
		system("touch " STUB_CTRL);

		std::cv_status intime = cond_sync[eLs3Send].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::timeout, intime);

		/* イベント応答待ち */
		ret = sif_evt_waitAny(sifEvt_, AM_API_EVT_SUB, AM_SUB_TIMEOUT, &evtCode);
		EXPECT_EQ(0, ret);
		EXPECT_EQ(AM_API_EVT_SUB_OK, evtCode);
		EXPECT_EQ(0x00,   ApiDat_Result_.FBlockID);
		EXPECT_EQ(0x00,   ApiDat_Result_.InstID);
		EXPECT_EQ(0x0DF8, ApiDat_Result_.FktID);
		EXPECT_EQ(0x0C,   ApiDat_Result_.OPType);
		EXPECT_EQ(16,     ApiDat_Result_.Length);
	}
	StubClientUnRegisterListerner(STUB_CALL(Ls3CtlApiSndCmd));

	/* CBを解除 */
	ret = Ls3CtlApiCmdDelCallback(&ifObj, 1, 2);
	EXPECT_EQ(LS3_CTL_RET_SUCCESS, ret);
}

/*---------------------------------------------------- */
