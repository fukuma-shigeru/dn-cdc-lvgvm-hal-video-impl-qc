#include <com_typedef.h>
#include <ls3_ctl_api_public.h>


/*
===== ファイル構成
パス：/run/arene/vehicle_fs/var/bev3/stub/ls3ctl
├ func_ret    関数戻り値設定フォルダ
│　├ Ls3CtlApiClose               関数の戻り値を設定する
│　├ Ls3CtlApiCmdAddCallback      　同上
│　├ Ls3CtlApiCmdDelCallback      　同上
│　├ Ls3CtlApiSndCmd              　同上
│　└ Ls3CtlApiGetSystemState      　同上
├ Ls3CtlApiOpen_call               Ls3CtlApiOpen呼出し
├ Ls3CtlApiClose_call              Ls3CtlApiClose呼出し
├ Ls3CtlApiCmdAddCallback_call     Ls3CtlApiCmdAddCallback呼出し
├ Ls3CtlApiCmdDelCallback_call     Ls3CtlApiCmdDelCallback呼出し
├ Ls3CtlApiSndCmd_call             Ls3CtlApiSndCmd呼出し
├ Ls3CtlApiGetSystemState_status   SystemStateの状態
├ Ls3CtlApiDat_OPType_status       コールバックのOPType値
└ Ls3CtlApiCmdClbk_control         コールバック呼び出しトリガ
*/


#define DEF_STRING(_str)		#_str
#define STUB_PATH				"/run/arene/vehicle_fs/var/bev3/stub/ls3ctl/"
#define STUB_PATH_FUNC			STUB_PATH "func_ret/"
#define STUB_FUNC(_fnc)			STUB_PATH_FUNC DEF_STRING(_fnc)
#define STUB_CTRL				STUB_PATH "Ls3CtlApiCmdClbk_control"
#define STUB_APIDAT_OPTYPE		STUB_PATH "Ls3CtlApiDat_OPType_status"
#define STUB_STS(_fnc)			STUB_PATH DEF_STRING(_fnc) "_status"
#define STUB_CALL(_fnc)			STUB_PATH DEF_STRING(_fnc) "_call"

static int calledFirst_ = {FALSE};
static Ls3CtlApiCmdClbk	ls3ctlApiCmdClbk_ = {NULL};		/* 制約：保持するコールバックは1つだけ */
static Ls3CtlApiDat	ls3ctlApiDat_ = {0};

/*---------------------------------------------------- */
/* ファイルの内容を取得する(32bit整数) */
static INT32 ReadFileInt32(const char* path, INT32 defValue)
{
	INT32 value = defValue;
	if (StubClientRead_bin(path, (int*)&value, sizeof(value)))
	{
		value = defValue;
	}
	return value;
}

/*---------------------------------------------------- */
/* コールバック呼び出し時の動作 */
static void Ls3CtlApiCmdClbkListener(const char* p_path)
{
	if ((!p_path) || (!ls3ctlApiCmdClbk_))
	{
		return;
	}

	Ls3CtlApiDat	apidat = ls3ctlApiDat_;
	apidat.OPType = (UINT8)ReadFileInt32(STUB_APIDAT_OPTYPE, 0x0C); /* 0x0C:APOLICY_LS3_CMD_OP_TYPE_STATUS_RESULT:正常系 */

/*	apidat.OPType = 0x0C;	*//* APOLICY_LS3_CMD_OP_TYPE_STATUS_RESULT:正常系 */
/*	apidat.OPType = 0x0F;	*//* APOLICY_LS3_CMD_OP_TYPE_STATUS_ERROR :異常系1 */
/*	apidat.OPType = 0x0B;	*//* APOLICY_LS3_CMD_OP_TYPE_STATUS_BUSY  :異常系2 */
	ls3ctlApiCmdClbk_(LS3_CTL_EVENT_CMD_RCV, apidat.FBlockID, apidat.InstID, &apidat);
	return;
}



/*---------------------------------------------------- */
/* オープン */
INT32 Ls3CtlApiOpen(Ls3CtlApiObj *pIfObj)
{
	/* 初回のみ実施 */
	if (TRUE != calledFirst_)
	{
		calledFirst_ = TRUE;

		/* FileObserver用フォルダ作成 */
		system("mkdir -p " STUB_PATH_FUNC);

		/* 関数戻り値ファイル作成 */
		system("touch " STUB_FUNC(Ls3CtlApiClose));
		system("touch " STUB_FUNC(Ls3CtlApiCmdAddCallback));
		system("touch " STUB_FUNC(Ls3CtlApiCmdDelCallback));
		system("touch " STUB_FUNC(Ls3CtlApiSndCmd));
		system("touch " STUB_FUNC(Ls3CtlApiGetSystemState));

		/* 各プロパティ用ファイル作成 */
		system("rm -f " STUB_APIDAT_OPTYPE);
		system("rm -f " STUB_STS(Ls3CtlApiGetSystemState));
		system("touch " STUB_APIDAT_OPTYPE);
		system("touch " STUB_STS(Ls3CtlApiGetSystemState));

		/* Ls3CtlApiCmdClbkコールバック用ファイル作成 */
		system("echo test > " STUB_CTRL);
		StubClientRegisterListerner(STUB_CTRL, Ls3CtlApiCmdClbkListener);
	}

	INT32	ret = ReadFileInt32(STUB_FUNC(Ls3CtlApiOpen), LS3_CTL_RET_SUCCESS); //sasax
	do
	{
		if (!pIfObj)
		{
			ret = LS3_CTL_RET_ERR_PARAM;
			break;
		}

		/* 正常の場合のみ各処理を実施 */
		system("echo test > " STUB_CALL(Ls3CtlApiOpen));		/* callイベント発生 */
	} while(FALSE);
	return ret;
}


/*---------------------------------------------------- */
/* クローズ */
INT32 Ls3CtlApiClose(Ls3CtlApiObj *pIfObj)
{
	INT32	ret = LS3_CTL_RET_SUCCESS;
	do
	{
		if (!pIfObj)
		{
			ret = LS3_CTL_RET_ERR_PARAM;
			break;
		}

		ret = ReadFileInt32(STUB_FUNC(Ls3CtlApiClose), LS3_CTL_RET_SUCCESS);
		if (LS3_CTL_RET_SUCCESS != ret)
		{
			break;
		}

		/* 正常の場合のみ各処理を実施 */
		system("echo test > " STUB_CALL(Ls3CtlApiClose));		/* callイベント発生 */
	} while(FALSE);
	return ret;
}

/*---------------------------------------------------- */
/* LS3コマンド送信 */
INT32 Ls3CtlApiSndCmd(Ls3CtlApiObj *pIfObj, Ls3CtlApiDat *pCmdDat)
{
	INT32	ret = LS3_CTL_RET_SUCCESS;
	do
	{
		if ((!pIfObj) || (!pCmdDat))
		{
			ret = LS3_CTL_RET_ERR_PARAM;
			break;
		}

		ret = ReadFileInt32(STUB_FUNC(Ls3CtlApiSndCmd), LS3_CTL_RET_SUCCESS);
		if (LS3_CTL_RET_SUCCESS != ret)
		{
			break;
		}

		/* 正常の場合のみ各処理を実施 */
		ls3ctlApiDat_ = *pCmdDat;
		system("echo test > " STUB_CALL(Ls3CtlApiSndCmd));		/* callイベント発生 */

	} while(FALSE);
	return ret;
}


/*---------------------------------------------------- */
/* LS3コールバック登録 */
INT32 Ls3CtlApiCmdAddCallback(Ls3CtlApiObj *pIfObj, UINT8 FBlockID, UINT8 InstID, Ls3CtlApiCmdClbk pClbkFunc)
{
	(void)FBlockID;
	(void)InstID;
	INT32	ret = LS3_CTL_RET_SUCCESS;
	do
	{
		if ((!pIfObj) || (!pClbkFunc))
		{
			ret = LS3_CTL_RET_ERR_PARAM;
			break;
		}

		ret = ReadFileInt32(STUB_FUNC(Ls3CtlApiCmdAddCallback), LS3_CTL_RET_SUCCESS);
		if(LS3_CTL_RET_SUCCESS != ret)
		{
			break;
		}

		/* 正常の場合のみ各処理を実施 */
		ls3ctlApiCmdClbk_ = pClbkFunc;
		system("echo test > " STUB_CALL(Ls3CtlApiCmdAddCallback));		/* callイベント発生 */

	} while(FALSE);
	return ret;
}

/*---------------------------------------------------- */
/* LS3コールバック解除 */
INT32 Ls3CtlApiCmdDelCallback(Ls3CtlApiObj *pIfObj, UINT8 FBlockID, UINT8 InstID)
{
	(void)FBlockID;
	(void)InstID;
	INT32	ret = LS3_CTL_RET_SUCCESS;
	do
	{
		if (!pIfObj)
		{
			ret = LS3_CTL_RET_ERR_PARAM;
			break;
		}

		ret = ReadFileInt32(STUB_FUNC(Ls3CtlApiCmdDelCallback), LS3_CTL_RET_SUCCESS);
		if(LS3_CTL_RET_SUCCESS != ret)
		{
			break;
		}

		/* 正常の場合のみ各処理を実施 */
		ls3ctlApiCmdClbk_ = NULL;
		system("echo test > " STUB_CALL(Ls3CtlApiCmdDelCallback));		/* callイベント発生 */

	} while(FALSE);
	return ret;
}

/*---------------------------------------------------- */
/* LS3システムステータス取得 */
INT32 Ls3CtlApiGetSystemState(Ls3CtlApiObj *pIfObj, Ls3CtlApiSystemState *pSystemState)
{
	INT32	ret = LS3_CTL_RET_SUCCESS;
	do
	{
		if ((!pIfObj) || (!pSystemState))
		{
			ret = LS3_CTL_RET_ERR_PARAM;
			break;
		}

/*		pSystemState->FrontAVMasterState = LS3_CTL_FRONTAVMASTER_SYSTEM_OK;			*//* 正常系 */
/*		pSystemState->FrontAVMasterState = LS3_CTL_FRONTAVMASTER_SYSTEM_NOTOK;		*//* 異常系 */
		ret = ReadFileInt32(STUB_FUNC(Ls3CtlApiGetSystemState), LS3_CTL_RET_SUCCESS);
		if(LS3_CTL_RET_SUCCESS != ret)
		{
			break;
		}

		/* 正常の場合のみ各処理を実施 */
		pSystemState->FrontAVMasterState = ReadFileInt32(STUB_STS(Ls3CtlApiGetSystemState), LS3_CTL_FRONTAVMASTER_SYSTEM_OK);
	} while(FALSE);
	return ret;
}

/*---------------------------------------------------- */
