
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "com_stddef.h"
#include "ccm_api_public.h"
#include "tab_ctrl_api_public.h"
#include "ls1_ctrl_api_public.h"
#include "stub_common.h"

#include "MiscConfig.h"

/*
===== ファイル構成
基点：/run/arene/vehicle_fs/var/bev3/stub/ccm_client
├ func_ret    関数戻り値設定フォルダ
│　├ CcmApiClose               関数の戻り値を設定する
│　├ Ls1CtrlApiCmdSend         　同上
│　├ TabCtrlApiPwCmdSend       　同上
│　├ TabCtrlApiEntPwCmdNtyCB   　同上
│　└ TabCtrlApiDelPwCmdNtyCB   　同上
├ TabCtrlApiPwCmdNtyCB   コールバック関数用フォルダ
│　├ 0_1_param    同期検知・経路情報通知の2バイト目
│　├ 0_2_param    　〃　3バイト目
│　├ 0_3_param    　〃　4バイト目
│　├ 0_4_param    　〃　5バイト目
│　├ 0_control    　〃　コールバック発生トリガ
│　├ 1_1_param    カメラ種別判別通知の2バイト目
│　├ 1_2_param    　〃　3バイト目
│　├ 1_3_param    　〃　4バイト目
│　├ 1_4_param    　〃　5バイト目
│　├ 1_5_param    　〃　6バイト目
│　├ 1_control    　〃　コールバック発生トリガ
│　├ 2_1_param    映像MUTE確認の2バイト目
│　├ 2_control    　〃　コールバック発生トリガ
│　├ 3_1_param    映像MUTE状態通知の2バイト目
│　└ 3_control    　〃　コールバック発生トリガ
├ Ls1CtrlApiCmdSend_call       Ls1CtrlApiCmdSend関数用call
├ TabCtrlApiDelPwCmdNtyCB_call TabCtrlApiDelPwCmdNtyCB関数用call
├ TabCtrlApiEntPwCmdNtyCB_call TabCtrlApiEntPwCmdNtyCB関数用call
└ TabCtrlApiPwCmdSend_call     TabCtrlApiPwCmdSend関数用call
*/

/* CCM情報構造体 */
typedef struct ccmobj_t_ {
	pthread_t			thid;			/* 周期スレッドID */
	pthread_mutex_t		cond_mtx;		/* 周期スレッドイベント待ちミューテックス */
	pthread_cond_t		cond;			/* 周期スレッドイベント待ち状態 */
	int					closing;		/* 周期スレッド終了トリガ */
	
	pthread_t			thid_m;			/* 周期スレッドID */
	pthread_mutex_t		cond_mtx_m;		/* 周期スレッドイベント待ちミューテックス */
	pthread_cond_t		cond_m;			/* 周期スレッドイベント待ち状態 */
	int					closing_m;		/* 周期スレッド終了トリガ */
	
} CcmObj_t;

//変数を追加してあげる。

/* CBパラメータ初期値構造体 */
typedef struct param_tbl_t_ {
	INT32		len;			/* パラメータ長 */
	INT32		param[8];		/* パラメータ初期値 */
} param_tbl_t;



/* CBパラメータの各初期値 */
const static param_tbl_t	cbParams_[4] =
{
	{ 0x05, {0x17, 0x01, 0x01, 0x01, 0x01       } } ,		/* 同期検知・経路情報通知(BEYE/FEYE) */
	{ 0x06, {0x19, 0x05, 0x06, 0x00, 0x00, 0x00 } } ,		/* カメラ種別判別通知 */
	{ 0x02, {0x62, 0x00                         } } ,		/* 映像MUTE確認 */
	{ 0x02, {0x63, 0x00                         } } ,		/* 映像MUTE状態通知 */
};

static pthread_mutex_t	CbMtx_ = PTHREAD_MUTEX_INITIALIZER;	/* コールバック排他用ミューテックス */
static TabCtrlApiPwCmdNtyCB p_TabCtrlCb = NULL;
static int calledFirst_ = {FALSE};

/*---------------------------------------------------- */
/* 関数の戻り値を設定する */
static INT32 ReadFuncResult(const char* path, INT32 defValue)
{
	INT32 value = defValue;
	if (StubClientRead_bin(path, (int*)&value, sizeof(value)))
	{
		value = defValue;
	}
	return value;
}

/*---------------------------------------------------- */
static void WriteBin(const char* path, const void* data, size_t size)
{
	if ((NULL != path) && (NULL != data))
	{
		(void)StubClientWrite_bin(path, (int*)data, (int)size);
	}
}

/*---------------------------------------------------- */
/* パラメータ初期値設定 */
static void SetInitParam(const char* path, INT32 value)
{
	/* ファイルがない場合は生成し、値を書き込む */
	/* 既にファイルが存在する場合はエラーになり無処理とする */
	int fd = open(path, O_CREAT|O_EXCL, 0666);
	if (fd >= 0)
	{
		close(fd);
		WriteBin(path, &value, sizeof(value));
	}
}

/*---------------------------------------------------- */
/* リスナ共通部 */
static void TabCtrlApiNotifyListenerCommon(const param_tbl_t* p_cbParam, const char* p_paths[])
{
	/* CB関数の取得 */
	pthread_mutex_lock(&CbMtx_);
	TabCtrlApiPwCmdNtyCB	p_fp = p_TabCtrlCb;
	pthread_mutex_unlock(&CbMtx_);

	/* CB未設定時は無処理 */
	if (!p_fp)
	{
		return;
	}

	UINT8*	p_cmd = (UINT8*)malloc(p_cbParam->len);
	if (!p_cmd)
	{
		return;
	}
	p_cmd[0] = (UINT8)p_cbParam->param[0];	/* OPCは固定 */

	for (INT32 i=1; i<p_cbParam->len; i++)
	{
		INT32	value;
		if (StubClientRead_i32(p_paths[i-1], &value))
		{
			/* 読み込み失敗時はデフォルト値とする */
			value = p_cbParam->param[i];
		}
		p_cmd[i] = (UINT8)value;
	}

	/* コールバック関数の実施 */
	p_fp(p_cmd, p_cbParam->len);
	free(p_cmd);
}

/*---------------------------------------------------- */

/* 同期検知・経路情報通知(BEYE/FEYE) 取得関数 (OPC = 0x17) */
static void TabCtrlApiNotifyListener0(const char* p_path)
{
	(void)p_path;
	const char*		pathParam[] =
	{
		STUB_PARAM_STS(0, 1),
		STUB_PARAM_STS(0, 2),
		STUB_PARAM_STS(0, 3),
		STUB_PARAM_STS(0, 4),
	};
	TabCtrlApiNotifyListenerCommon(&cbParams_[0], pathParam);
}

/*---------------------------------------------------- */
/* カメラ種別判別通知 取得関数 (OPC = 0x19) */
static void TabCtrlApiNotifyListener1(const char* p_path)
{
	(void)p_path;
	const char*		pathParam[] =
	{
		STUB_PARAM_STS(1, 1),
		STUB_PARAM_STS(1, 2),
		STUB_PARAM_STS(1, 3),
		STUB_PARAM_STS(1, 4),
		STUB_PARAM_STS(1, 5),
	};
	TabCtrlApiNotifyListenerCommon(&cbParams_[1], pathParam);
}

/*---------------------------------------------------- */
/* 映像MUTE確認 取得関数 (OPC = 0x62) */
static void TabCtrlApiNotifyListener2(const char* p_path)
{
	(void)p_path;
	const char*		pathParam[] =
	{
		STUB_PARAM_STS(2, 1),
	};
	TabCtrlApiNotifyListenerCommon(&cbParams_[2], pathParam);
}

/*---------------------------------------------------- */
/* 映像MUTE状態通知 取得関数 (OPC = 0x63) */
static void TabCtrlApiNotifyListener3(const char* p_path)
{
	(void)p_path;
	const char*		pathParam[] =
	{
		STUB_PARAM_STS(3, 1),
	};
	TabCtrlApiNotifyListenerCommon(&cbParams_[3], pathParam);
}

/*---------------------------------------------------- */
/* 周期スレッド(CcmApiOpen時に生成) */
static void* CcmCycleThread(void* arg)
{
	CcmObj_t*			p_ccm = (CcmObj_t*)arg;
	struct timespec		ts;
	int	ret;

	/* TabCtrlApiEntPwCmdNtyCBかCcmApiClose呼び出しまで待つ */
	ret = pthread_cond_wait(&p_ccm->cond, &p_ccm->cond_mtx);
	if (ret)
	{
		return NULL;
	}

	clock_gettime(CLOCK_REALTIME, &ts);

	while (TRUE != p_ccm->closing)
	{
		ts.tv_sec += 3;		/* 3秒周期 */
		ret = pthread_cond_timedwait(&p_ccm->cond, &p_ccm->cond_mtx, &ts);
		if (ETIMEDOUT == ret)
		{
			/* コールバックの実行 */
			TabCtrlApiNotifyListener1(STUB_PARAM_CTL(1));	/* カメラ種別判別通知 */
			usleep(10);		/* 10us wait */
			TabCtrlApiNotifyListener3(STUB_PARAM_CTL(3));	/* 映像MUTE状態通知 */
		}
	}
	return NULL;
}

/* プロトタイプ宣言 */
static void MiscCtrlInit();

/*---------------------------------------------------- */
/* オープン */
INT32	CcmApiOpen( void** p_ccm_obj )
{

	/* 初回のみ実施 */
	if (TRUE != calledFirst_)
	{
		calledFirst_ = TRUE;

		/* FileObserver用フォルダ作成 */
		system("mkdir -p " STUB_PATH_FUNC);
		system("mkdir -p " STUB_PATH_CB);
		
		/* 関数戻り値ファイル作成(すでに存在する場合は未変更) */
		system("touch " STUB_FUNC(CcmApiClose));
		system("touch " STUB_FUNC(TabCtrlApiPwCmdSend));
		system("touch " STUB_FUNC(TabCtrlApiEntPwCmdNtyCB));
		system("touch " STUB_FUNC(TabCtrlApiDelPwCmdNtyCB));
		system("touch " STUB_FUNC(Ls1CtrlApiCmdSend));

		/* controlファイル作成 */
		system("echo test > " STUB_PARAM_CTL(0));
		system("echo test > " STUB_PARAM_CTL(1));
		system("echo test > " STUB_PARAM_CTL(2));
		system("echo test > " STUB_PARAM_CTL(3));
		StubClientRegisterListerner(STUB_PARAM_CTL(0), TabCtrlApiNotifyListener0);
		StubClientRegisterListerner(STUB_PARAM_CTL(1), TabCtrlApiNotifyListener1);
		StubClientRegisterListerner(STUB_PARAM_CTL(2), TabCtrlApiNotifyListener2);
		StubClientRegisterListerner(STUB_PARAM_CTL(3), TabCtrlApiNotifyListener3);

		/* パラメータファイル作成(存在しない場合は初期値設定) */
		SetInitParam(STUB_PARAM_STS(0, 1), cbParams_[0].param[1]);
		SetInitParam(STUB_PARAM_STS(0, 2), cbParams_[0].param[2]);
		SetInitParam(STUB_PARAM_STS(0, 3), cbParams_[0].param[3]);
		SetInitParam(STUB_PARAM_STS(0, 4), cbParams_[0].param[4]);
		SetInitParam(STUB_PARAM_STS(1, 1), cbParams_[1].param[1]);
		SetInitParam(STUB_PARAM_STS(1, 2), cbParams_[1].param[2]);
		SetInitParam(STUB_PARAM_STS(1, 3), cbParams_[1].param[3]);
		SetInitParam(STUB_PARAM_STS(1, 4), cbParams_[1].param[4]);
		SetInitParam(STUB_PARAM_STS(1, 5), cbParams_[1].param[5]);
		SetInitParam(STUB_PARAM_STS(2, 1), cbParams_[2].param[1]);
		SetInitParam(STUB_PARAM_STS(3, 1), cbParams_[3].param[1]);
		MiscCtrlInit();
	}

	CcmObj_t*	p_ccm = NULL;
	INT32		ret = ReadFuncResult(STUB_FUNC(CcmApiOpen), CCM_RET_SUCCESS); //sasax
	do
	{
		if (NULL == p_ccm_obj)
		{
			ret = CCM_RET_ERR_PARAM;
			break;
		}

		/* オブジェクト生成 */
		p_ccm = (CcmObj_t*)malloc(sizeof(*p_ccm));
		if (NULL == p_ccm)
		{
			ret = CCM_RET_ERR_RESOURCE;
			break;
		}

		/* ミューテックス初期化 */
		if (pthread_mutex_init(&p_ccm->cond_mtx, NULL)
		||  pthread_cond_init(&p_ccm->cond, NULL)
		||  pthread_mutex_lock(&p_ccm->cond_mtx))
		{
			ret = CCM_RET_ERR_RESOURCE;
			break;
		}

		/* メンバの初期値設定 */
		p_ccm->closing = FALSE;
		/* 周期スレッド生成 */
		if(pthread_create(&p_ccm->thid, NULL, CcmCycleThread, p_ccm))
		{
			ret = CCM_RET_ERR_RESOURCE;
			break;
		}
		/* 最後まで成功したらp_ccm_objにオブジェクトを設定 */
		*p_ccm_obj = (void*)p_ccm;	
		
	} while(FALSE);

	/* 処理失敗時は確保済のリソースを開放 */
	if (CCM_RET_SUCCESS != ret)
	{
		if (p_ccm)
		{
			pthread_cond_destroy(&p_ccm->cond);
			pthread_mutex_destroy(&p_ccm->cond_mtx);
			free(p_ccm);
		}
		if (p_ccm_obj)
		{
			*p_ccm_obj = NULL;
		}
	}
	return ret;
}

/*---------------------------------------------------- */
/* クローズ */
INT32	CcmApiClose( void* p_ccm_obj )
{
	INT32	ret = CCM_RET_SUCCESS;
	do
	{
		if (!p_ccm_obj)
		{
			ret = CCM_RET_ERR_PARAM;
			break;
		}

		/* ファイル書込みにて戻り値が決まっている場合は戻り値を取得 */
		ret = ReadFuncResult(STUB_FUNC(CcmApiClose), CCM_RET_SUCCESS);
		if (CCM_RET_SUCCESS != ret)
		{
			break;
		}

		CcmObj_t*	p_ccm = (CcmObj_t*)p_ccm_obj;

		/* 周期スレッドを停止させる */
		pthread_mutex_lock(&p_ccm->cond_mtx);
		p_ccm->closing = TRUE;
		pthread_cond_signal(&p_ccm->cond);
		pthread_mutex_unlock(&p_ccm->cond_mtx);
		pthread_join(p_ccm->thid, NULL);

		/* CB削除 */
		pthread_mutex_lock(&CbMtx_);
		p_TabCtrlCb = NULL;
		pthread_mutex_unlock(&CbMtx_);

		/* リソースの開放 */
		pthread_cond_destroy(&p_ccm->cond);
		pthread_mutex_destroy(&p_ccm->cond_mtx);
		free(p_ccm_obj);


	} while(FALSE);

	return ret;
}


/*---------------------------------------------------- */
/* TABコマンド送信 */
INT32	TabCtrlApiPwCmdSend( void* p_ccm_obj, void* p_cmd, UINT32 len )
{
	INT32	ret = TAB_CTRL_RET_SUCCESS;
	do
	{
		/* 引数チェック */
		if ((!p_ccm_obj) || (!p_cmd) || (!len))
		{
			ret = TAB_CTRL_RET_ERR_PARAM;
			break;
		}

		/* ファイル書込みにて戻り値が決まっている場合は戻り値を取得 */
		ret = ReadFuncResult(STUB_FUNC(TabCtrlApiPwCmdSend), TAB_CTRL_RET_SUCCESS);
		if (TAB_CTRL_RET_SUCCESS != ret)
		{
			break;
		}

		/* callイベント発生 */
		system("echo test > " STUB_CALL(TabCtrlApiPwCmdSend));

		UINT8*	p_ccmd = (UINT8*)p_cmd;
		if (0x62U == p_ccmd[0])
		{
			INT32 value = (INT32)p_ccmd[1];
			/* Mute ON/Off設定 */
			WriteBin(STUB_PARAM_STS(2, 1), &value, sizeof(value));
			WriteBin(STUB_PARAM_STS(3, 1), &value, sizeof(value));
			/* トリガ発動 */
			WriteBin(STUB_PARAM_CTL(2), &value, sizeof(value));
		}
	} while(FALSE);

	return ret;
}

/*---------------------------------------------------- */
/* TABコールバック登録 */
INT32	TabCtrlApiEntPwCmdNtyCB( void* p_ccm_obj, TabCtrlApiPwCmdNtyCB p_fp, TabCtrlApiPwNtyFilter* p_filter )
{
	(void)p_filter;

	INT32	ret = TAB_CTRL_RET_SUCCESS;
	do
	{
		/* 引数チェック */
		if ((!p_ccm_obj) || (!p_fp))
		{
			ret = TAB_CTRL_RET_ERR_PARAM;
			break;
		}

		/* ファイル書込みにて戻り値が決まっている場合は戻り値を取得 */
		ret = ReadFuncResult(STUB_FUNC(TabCtrlApiEntPwCmdNtyCB), TAB_CTRL_RET_SUCCESS);
		if (TAB_CTRL_RET_SUCCESS != ret)
		{
			break;
		}

		/* 2byte目を同期ありにする */
		INT32 value = 0x01;
		if (StubClientWrite_bin(STUB_PARAM_STS(0, 1), (int*)(&value),  sizeof(value)))
		{
			ret = TAB_CTRL_RET_ERR_FATAL;
			break;
		}

		/* callイベント発生 */
		system("echo test > " STUB_CALL(TabCtrlApiEntPwCmdNtyCB));

		/* CB登録 */
		pthread_mutex_lock(&CbMtx_);
		p_TabCtrlCb = p_fp;
		pthread_mutex_unlock(&CbMtx_);

		/* 周期スレッド駆動 */
		CcmObj_t*	p_ccm = (CcmObj_t*)p_ccm_obj;
		pthread_mutex_lock(&p_ccm->cond_mtx);
		pthread_cond_signal(&p_ccm->cond);
		pthread_mutex_unlock(&p_ccm->cond_mtx);

		/* 同期検知・経路情報通知(BEYE/FEYE)のCBをコール */
		system("echo test > " STUB_PARAM_CTL(0));

	} while(FALSE);

	return ret;
}

/*---------------------------------------------------- */
/* TABコールバック解除 */
INT32	TabCtrlApiDelPwCmdNtyCB( void* p_ccm_obj )
{
	INT32	ret = TAB_CTRL_RET_SUCCESS;
	do
	{
		/* 引数チェック */
		if (!p_ccm_obj)
		{
			ret = TAB_CTRL_RET_ERR_PARAM;
			break;
		}

		/* ファイル書込みにて戻り値が決まっている場合は戻り値を取得 */
		ret = ReadFuncResult(STUB_FUNC(TabCtrlApiDelPwCmdNtyCB), TAB_CTRL_RET_SUCCESS);
		if (TAB_CTRL_RET_SUCCESS != ret)
		{
			break;
		}

		/* CB削除 */
		pthread_mutex_lock(&CbMtx_);
		p_TabCtrlCb = NULL;
		pthread_mutex_unlock(&CbMtx_);

		/* callイベント発生 */
		system("echo test > " STUB_CALL(TabCtrlApiDelPwCmdNtyCB));
	} while(FALSE);

	return ret;
}

/*---------------------------------------------------- */
/* Ls1Ctrlコマンド送信 */
INT32	Ls1CtrlApiCmdSend( void* p_ccm_obj, void* p_cmd, UINT32 len )
{
	INT32	ret = LS1_CTRL_RET_SUCCESS;
	do
	{
		/* 引数チェック */
		if ((!p_ccm_obj) || (!p_cmd) || (!len))
		{
			ret = LS1_CTRL_RET_ERR_PARAM;
			break;
		}

		/* ファイル書込みにて戻り値が決まっている場合は戻り値を取得 */
		ret = ReadFuncResult(STUB_FUNC(Ls1CtrlApiCmdSend), LS1_CTRL_RET_SUCCESS);
		if (LS1_CTRL_RET_SUCCESS != ret)
		{
			break;
		}

		/* callイベント発生 */
		system("echo test > " STUB_CALL(Ls1CtrlApiCmdSend));
	} while(FALSE);

	return ret;
}


/*---------------------------------------------------- */
/* ここから下は何もしない */
/*---------------------------------------------------- */
INT32	TabCtrlApiEntStsCB( void* p_ccm_obj, TabCtrlApiStsCB p_fp )
{
	return TAB_CTRL_RET_SUCCESS;
}

/*---------------------------------------------------- */
INT32	TabCtrlApiDelStsCB( void* p_ccm_obj )
{
	return TAB_CTRL_RET_SUCCESS;
}

/*---------------------------------------------------- */
INT32	Ls1CtrlApiEntStsCB( void* p_ccm_obj, Ls1CtrlApiStsCB p_fp )
{
	return LS1_CTRL_RET_SUCCESS;
}

/*---------------------------------------------------- */
INT32	Ls1CtrlApiDelStsCB( void* p_ccm_obj )
{
	return LS1_CTRL_RET_SUCCESS;
}

/*---------------------------------------------------- */
INT32	Ls1CtrlApiEntCmdNtyCB( void* p_ccm_obj, Ls1CtrlApiCmdNtyCB p_fp, Ls1CtrlApiNtyFilter* p_filter )
{
	return LS1_CTRL_RET_SUCCESS;
}

/*---------------------------------------------------- */
INT32	Ls1CtrlApiDelCmdNtyCB( void* p_ccm_obj )
{
	return LS1_CTRL_RET_SUCCESS;
}

/*---------------------------------------------------- */
INT32	Ls1CtrlApiFuncIns( void* p_ccm_obj, UINT32 ins )
{
	return LS1_CTRL_RET_SUCCESS;
}

/*---------------------------------------------------- */
INT32	Ls1CtrlApiGetDevInfo( void* p_ccm_obj, Ls1CtrlApiDevInfo* pDevInfo )
{
	return LS1_CTRL_RET_SUCCESS;
}

/*---------------------------------------------------- */


/*---------------------------------------------------- */
#include "misc_ctrl_api_public.h"



/* コールバックを保持するための変数 */
static MiscCtrlApiDataNtyCB p_MiscCtrlCb = NULL;
static MiscCtrlApiStsCB p_MiscCtrlStsCb = NULL;
static pthread_mutex_t	CbMtxm_ = PTHREAD_MUTEX_INITIALIZER;	/* コールバック排他用ミューテックス */

/*コマンド初回判別(カメラ判別要求)*/
int cmdf = 0;
//int mcb =0;
int threadf =0;

	/* CBパラメータ初期値構造体 */
	typedef struct param_misc_t_ {
		UINT8		data_type;		/* データタイプ */
		INT32		len;			/* パラメータ長 */
		INT32		param[32];		/* パラメータ初期値 */
	} param_misc_t;
	
	/* ステータス初期情報構造体 */
	/*typedef struct MiscCtrlApiSts_ {
		UINT32		ctrl_sts;		/* Ctrlステータス */
	//} MiscCtrlApiSts;
	
	//仕様書の構造体の名前を使用しましょう
	static MiscCtrlApiSts		p_sts = 
	{
		0xb,
	};
	
	/* CBパラメータの各初期値 */
	const static param_misc_t	cbParamsSts_[] =
	{
		{ 0x34, 0x06, {0x02, 0x05, 0x06, 0x00, 0x00, 0x00 } } ,		/* カメラ種別判別通知 */
		{ 0x34, 0x06, {0x03, 0x01, 0x00, 0x01, 0x01, 0x00 } } ,		/* 同期検知・経路情報通知(BEYE/FEYE) */
		{ 0x39, 0x02, {0x01, 0x02 } },								/* HDMI接続検知 */
		{ 0x39, 0x02, {0x02, 0x01 } },								/* HDMIビデオフォーマット変更通知 */
		{ 0x39, 0x02, {0x03, 0x03 } },								/* HDMIオーディオフォーマット変更通知 */
		{ 0x39, 0x0B, {0x05, 0x00, 0x00, 0x01, 0x00, 0x01, 0x11, 0x22, 0x33, 0x44, 0x55 } },	/* C-Disp HDCP認証通知 */
		{ 0x39, 0x15, {0x07, 0x00, 0x00, 0x03, 0x00, 0x02, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f } },	/* RSE HDCP認証通知 */
		{ 0x36, 0x02, {0x31, 0x00 } },									/* スクリーンショット応答(成功) */
	};
	
/* リスナ共通部(MISC) */
static void MiscCtrlApiNotifyListenerCommon(const param_misc_t* p_cbParam, const char* p_paths[])
{
	
	/* CB関数の取得 */
	pthread_mutex_lock(&CbMtxm_);
	MiscCtrlApiDataNtyCB	p_fp = p_MiscCtrlCb;
	pthread_mutex_unlock(&CbMtxm_);

	/* CB未設定時は無処理 */
	if (!p_fp)
	{
		return;
	}

	UINT8*	p_data = (UINT8*)malloc(p_cbParam->len);
	if (!p_data)
	{
		return;
	}
	
	p_data[0] = (UINT8)p_cbParam->param[0];	/* OPC(サブタイプ)は固定 */

	for (INT32 i=1; i<p_cbParam->len; i++)
	{
		INT32	value;
		if (StubClientRead_i32(p_paths[i-1], &value))
		{
			/* 読み込み失敗時はデフォルト値とする */
			value = p_cbParam->param[i];
		}
		p_data[i] = (UINT8)value;
	}

	/* コールバック関数の実施 引数確認 */
	p_fp(p_cbParam->data_type, p_data, p_cbParam->len); 
	free(p_data);
}
/*---------------------------------------------------- */
/* リスナ(MISCステータス通知) */
static void MiscCtrlApiStsListenerCommon(MiscCtrlApiSts * p_sts, const char* p_paths[])
{
	/* CB関数の取得 */
	pthread_mutex_lock(&CbMtxm_);
	MiscCtrlApiStsCB	p_fp = p_MiscCtrlStsCb;
	pthread_mutex_unlock(&CbMtxm_);
	
	/* CB未設定時は無処理 */
	if (!p_fp)
	{
		return;
	}

	StubClientRead_i32(p_paths[0], &p_sts->ctrl_sts);
	p_fp(p_sts);
}

/* OS間通信状態通知 */
static void MiscCtrlApiStsNty0(const char* p_path)
{
	(void)p_path;
	const char*		pathParam[] = 
	{
		STUB_PARAM_STS3(0, 1),
	};
	MiscCtrlApiStsListenerCommon(&p_sts, pathParam);
	
}

/*---------------------------------------------------- */

/* カメラ種別判別通知 取得関数 (OPC = 0x34-0x02) */
static void MiscCtrlApiNotifyListener0(const char* p_path)
{
	
	(void)p_path;
	const char*		pathParam[] =
	{
		STUB_PARAM_STS2(0, 1),
		STUB_PARAM_STS2(0, 2),
		STUB_PARAM_STS2(0, 3),
		STUB_PARAM_STS2(0, 4),
		STUB_PARAM_STS2(0, 5),
	};
	MiscCtrlApiNotifyListenerCommon(&cbParamsSts_[0], pathParam);
}


/* 同期検知・経路情報通知(BEYE/FEYE) 取得関数 (OPC = 0x34-0x03) */
static void MiscCtrlApiNotifyListener1(const char* p_path)
{
	(void)p_path;
	const char*		pathParam[] =
	{
		STUB_PARAM_STS2(1, 1),
		STUB_PARAM_STS2(1, 2),
		STUB_PARAM_STS2(1, 3),
		STUB_PARAM_STS2(1, 4),
		STUB_PARAM_STS2(1, 5),
	};
	MiscCtrlApiNotifyListenerCommon(&cbParamsSts_[1], pathParam);
}

/* HDMI接続検知 取得関数 (OPC = 0x39-0x01) */
static void MiscCtrlApiNotifyListener2(const char* p_path)
{
	(void)p_path;
	const char*		pathParam[] =
	{
		STUB_PARAM_STS2(2, 1),
	};
	MiscCtrlApiNotifyListenerCommon(&cbParamsSts_[2], pathParam);
}


/* HDMIビデオフォーマット変更通知 取得関数 (OPC = 0x39-0x02) */
static void MiscCtrlApiNotifyListener3(const char* p_path)
{
	(void)p_path;
	const char*		pathParam[] =
	{
		STUB_PARAM_STS2(3, 1),
	};
	MiscCtrlApiNotifyListenerCommon(&cbParamsSts_[3], pathParam);
}
/* HDMIオーディオフォーマット変更通知 取得関数 (OPC = 0x39-0x03) */
static void MiscCtrlApiNotifyListener4(const char* p_path)
{
	(void)p_path;
	const char*		pathParam[] =
	{
		STUB_PARAM_STS2(4, 1),
	};
	MiscCtrlApiNotifyListenerCommon(&cbParamsSts_[4], pathParam);
}

/* C-Disp_HDCP認証情報 取得関数 (OPC = 0x39-0x05) */
static void MiscCtrlApiNotifyListener5(const char* p_path)
{
	(void)p_path;
	const char*		pathParam[] =
	{
		STUB_PARAM_STS2(5, 1),
		STUB_PARAM_STS2(5, 2),
		STUB_PARAM_STS2(5, 3),
		STUB_PARAM_STS2(5, 4),
		STUB_PARAM_STS2(5, 5),
		STUB_PARAM_STS2(5, 6),
		STUB_PARAM_STS2(5, 7),
		STUB_PARAM_STS2(5, 8),
		STUB_PARAM_STS2(5, 9),
		STUB_PARAM_STS2(5, 10),
	};
	MiscCtrlApiNotifyListenerCommon(&cbParamsSts_[5], pathParam);
}

/* RSE_HDCP認証情報 取得関数 (OPC = 0x39-0x07) */
static void MiscCtrlApiNotifyListener6(const char* p_path)
{
	(void)p_path;
	const char*		pathParam[] =
	{
		STUB_PARAM_STS2(6, 1),
		STUB_PARAM_STS2(6, 2),
		STUB_PARAM_STS2(6, 3),
		STUB_PARAM_STS2(6, 4),
		STUB_PARAM_STS2(6, 5),
		STUB_PARAM_STS2(6, 6),
		STUB_PARAM_STS2(6, 7),
		STUB_PARAM_STS2(6, 8),
		STUB_PARAM_STS2(6, 9),
		STUB_PARAM_STS2(6, 10),
		STUB_PARAM_STS2(6, 11),
		STUB_PARAM_STS2(6, 12),
		STUB_PARAM_STS2(6, 13),
		STUB_PARAM_STS2(6, 14),
		STUB_PARAM_STS2(6, 15),
		STUB_PARAM_STS2(6, 16),
		STUB_PARAM_STS2(6, 17),
		STUB_PARAM_STS2(6, 18),
		STUB_PARAM_STS2(6, 19),
		STUB_PARAM_STS2(6, 20),
	};
	MiscCtrlApiNotifyListenerCommon(&cbParamsSts_[6], pathParam);
}

/* スクリーンショット応答 取得関数 (OPC = 0x36-0x31) */
/* FileObserver通知で実行 */
static void MiscCtrlApiNotifyListener7(const char* p_path)
{
	
	(void)p_path;
	const char*		pathParam[] =
	{
		STUB_PARAM_STS2(7, 1),
	};
	MiscCtrlApiNotifyListenerCommon(&cbParamsSts_[7], pathParam);
}

/* 周期スレッド(MiscCtrlApiEntDataNtyCB時に起動) */
static void* CcmCycleThreadMisc(void* arg)
{
	CcmObj_t*			p_ccm = (CcmObj_t*)arg;
	struct timespec		ts;
	int	ret;
	
	clock_gettime(CLOCK_REALTIME, &ts);
	
	
	ret = pthread_cond_wait(&p_ccm->cond_m, &p_ccm->cond_mtx_m);
	if (ret)
	{
		return NULL;
	}
	while (TRUE != p_ccm->closing_m)
	{
		ts.tv_sec += 3;		/* 3秒周期 */
		ret = pthread_cond_timedwait(&p_ccm->cond_m, &p_ccm->cond_mtx_m, &ts);
		if (ETIMEDOUT == ret)
		{
			/* コールバックの実行 */
			if(cmdf == 1)
			{
				MiscCtrlApiNotifyListener0(STUB_PARAM_CTL2(0));	/* カメラ種別判別通知 */
				usleep(10);		/* 10us wait */
			}
			MiscCtrlApiNotifyListener1(STUB_PARAM_CTL2(1));	/* 同期検知・経路情報通知(BEYE/FEYE) */
			usleep(10);		/* 10us wait */
			//MISCCtrlApiNotifyListener7(STUB_PARAM_CTL2(7));	/* 映像MUTE状態通知 */
			usleep(10);		/* 10us wait */
		}
	}
	
	return NULL;
}



/*---------------------------------------------------- */
/* MiscCtrlの初期化 */
static void MiscCtrlInit()
{	
	
	/* FileObserver用フォルダ作成 */
	system("mkdir -p " STUB_PATH_CB3);
	system("mkdir -p " STUB_PATH_CB2);
	
	/* 関数戻り値ファイル作成(すでに存在する場合は未変更) */
	system("touch " STUB_FUNC(MiscCtrlApiEntStsCB));
	system("touch " STUB_FUNC(MiscCtrlApiDelStsCB));
	system("touch " STUB_FUNC(MiscCtrlApiEntDataNtyCB));
	system("touch " STUB_FUNC(MiscCtrlApiDelDataNtyCB));
	system("touch " STUB_FUNC(MiscCtrlApiDataSend));
	
	/* controlファイル作成(コマンド分)  */
	system("echo test > " STUB_PARAM_CTL2(0));
	system("echo test > " STUB_PARAM_CTL2(1));
	system("echo test > " STUB_PARAM_CTL2(2));
	system("echo test > " STUB_PARAM_CTL2(3));
	system("echo test > " STUB_PARAM_CTL2(4));
	system("echo test > " STUB_PARAM_CTL2(5));
	system("echo test > " STUB_PARAM_CTL2(6));
	system("echo test > " STUB_PARAM_CTL2(7));	// スクリーンショット応答
	system("echo test > " STUB_PARAM_CTL3(0));
	StubClientRegisterListerner(STUB_PARAM_CTL2(0), MiscCtrlApiNotifyListener0);
	StubClientRegisterListerner(STUB_PARAM_CTL2(1), MiscCtrlApiNotifyListener1);
	StubClientRegisterListerner(STUB_PARAM_CTL2(2), MiscCtrlApiNotifyListener2);
	StubClientRegisterListerner(STUB_PARAM_CTL2(3), MiscCtrlApiNotifyListener3);
	StubClientRegisterListerner(STUB_PARAM_CTL2(4), MiscCtrlApiNotifyListener4);
	StubClientRegisterListerner(STUB_PARAM_CTL2(5), MiscCtrlApiNotifyListener5);
	StubClientRegisterListerner(STUB_PARAM_CTL2(6), MiscCtrlApiNotifyListener6);
	StubClientRegisterListerner(STUB_PARAM_CTL2(7), MiscCtrlApiNotifyListener7);	// スクリーンショット応答
	StubClientRegisterListerner(STUB_PARAM_CTL3(0), MiscCtrlApiStsNty0);
	
	/* パラメータファイル作成(存在しない場合は初期値設定) */
#if 1
	SetInitParam(STUB_PARAM_STS2(0, 1), cbParamsSts_[0].param[1]);
	SetInitParam(STUB_PARAM_STS2(0, 2), cbParamsSts_[0].param[2]);
	SetInitParam(STUB_PARAM_STS2(0, 3), cbParamsSts_[0].param[3]);
	SetInitParam(STUB_PARAM_STS2(0, 4), cbParamsSts_[0].param[4]);
	SetInitParam(STUB_PARAM_STS2(0, 5), cbParamsSts_[0].param[5]);
	SetInitParam(STUB_PARAM_STS2(1, 1), cbParamsSts_[1].param[1]);
	SetInitParam(STUB_PARAM_STS2(1, 2), cbParamsSts_[1].param[2]);
	SetInitParam(STUB_PARAM_STS2(1, 3), cbParamsSts_[1].param[3]);
	SetInitParam(STUB_PARAM_STS2(1, 4), cbParamsSts_[1].param[4]);
	SetInitParam(STUB_PARAM_STS2(1, 5), cbParamsSts_[1].param[5]);
	SetInitParam(STUB_PARAM_STS2(2, 1), cbParamsSts_[2].param[1]);
	SetInitParam(STUB_PARAM_STS2(3, 1), cbParamsSts_[3].param[1]);
	SetInitParam(STUB_PARAM_STS2(4, 1), cbParamsSts_[4].param[1]);
	SetInitParam(STUB_PARAM_STS2(5, 1), cbParamsSts_[5].param[1]);
	SetInitParam(STUB_PARAM_STS2(5, 2), cbParamsSts_[5].param[2]);
	SetInitParam(STUB_PARAM_STS2(5, 3), cbParamsSts_[5].param[3]);
	SetInitParam(STUB_PARAM_STS2(5, 4), cbParamsSts_[5].param[4]);
	SetInitParam(STUB_PARAM_STS2(5, 5), cbParamsSts_[5].param[5]);
	SetInitParam(STUB_PARAM_STS2(5, 5), cbParamsSts_[5].param[6]);
	SetInitParam(STUB_PARAM_STS2(5, 5), cbParamsSts_[5].param[7]);
	SetInitParam(STUB_PARAM_STS2(5, 5), cbParamsSts_[5].param[8]);
	SetInitParam(STUB_PARAM_STS2(5, 5), cbParamsSts_[5].param[9]);
	SetInitParam(STUB_PARAM_STS2(5, 5), cbParamsSts_[5].param[10]);
	SetInitParam(STUB_PARAM_STS2(6, 1), cbParamsSts_[6].param[1]);
	SetInitParam(STUB_PARAM_STS2(6, 2), cbParamsSts_[6].param[2]);
	SetInitParam(STUB_PARAM_STS2(6, 3), cbParamsSts_[6].param[3]);
	SetInitParam(STUB_PARAM_STS2(6, 4), cbParamsSts_[6].param[4]);
	SetInitParam(STUB_PARAM_STS2(6, 5), cbParamsSts_[6].param[5]);
	SetInitParam(STUB_PARAM_STS2(6, 6), cbParamsSts_[6].param[6]);
	SetInitParam(STUB_PARAM_STS2(6, 7), cbParamsSts_[6].param[7]);
	SetInitParam(STUB_PARAM_STS2(6, 8), cbParamsSts_[6].param[8]);
	SetInitParam(STUB_PARAM_STS2(6, 9), cbParamsSts_[6].param[9]);
	SetInitParam(STUB_PARAM_STS2(6, 10), cbParamsSts_[6].param[10]);
	SetInitParam(STUB_PARAM_STS2(6, 11), cbParamsSts_[6].param[11]);
	SetInitParam(STUB_PARAM_STS2(6, 12), cbParamsSts_[6].param[12]);
	SetInitParam(STUB_PARAM_STS2(6, 13), cbParamsSts_[6].param[13]);
	SetInitParam(STUB_PARAM_STS2(6, 14), cbParamsSts_[6].param[14]);
	SetInitParam(STUB_PARAM_STS2(6, 15), cbParamsSts_[6].param[15]);
	SetInitParam(STUB_PARAM_STS2(6, 16), cbParamsSts_[6].param[16]);
	SetInitParam(STUB_PARAM_STS2(6, 17), cbParamsSts_[6].param[17]);
	SetInitParam(STUB_PARAM_STS2(6, 18), cbParamsSts_[6].param[18]);
	SetInitParam(STUB_PARAM_STS2(6, 19), cbParamsSts_[6].param[19]);
	SetInitParam(STUB_PARAM_STS2(6, 20), cbParamsSts_[6].param[20]);
	SetInitParam(STUB_PARAM_STS2(7, 1),  cbParamsSts_[7].param[1]); // スクリーンショット応答用
#endif
	SetInitParam(STUB_PARAM_STS3(0, 1), p_sts.ctrl_sts);
	
#if 0
	CcmObj_t*	p_ccm = NULL;
	INT32		ret = ReadFuncResult(STUB_FUNC(MiscCtrlInit), CCM_RET_SUCCESS); 
	
	do
	{
		/* オブジェクト生成 */
		p_ccm = (CcmObj_t*)malloc(sizeof(*p_ccm));
		if (NULL == p_ccm)
		{
			ret = CCM_RET_ERR_RESOURCE;
			break;
		}
		/* ミューテックス初期化 */
		if (pthread_mutex_init(&p_ccm->cond_mtx_m, NULL)
		||  pthread_cond_init(&p_ccm->cond_m, NULL)
		||  pthread_mutex_lock(&p_ccm->cond_mtx_m))
		{
			ret = CCM_RET_ERR_RESOURCE;
			break;
		}

	} while(FALSE);
#endif
}

/*---------------------------------------------------- */

/* Miscステータス通知開始 */
INT32 MiscCtrlApiEntStsCB(void* p_ccm_obj, MiscCtrlApiStsCB p_fp)
{
	INT32 ret = MISC_CTRL_RET_SUCCESS;
	do
	{
		/* 引数チェック */
		if ((!p_ccm_obj) || (!p_fp))
		{
			ret = MISC_CTRL_RET_ERR_PARAM;
			break;
		}
		/* エラーチェック */
		/* ファイル書込みにて戻り値が決まっている場合は戻り値を取得 */
		ret = ReadFuncResult(STUB_FUNC(MiscCtrlApiEntStsCB), MISC_CTRL_RET_SUCCESS);
		
		if (MISC_CTRL_RET_SUCCESS != ret)
		{
			break;
		}
		/* callイベント発生 */
		system("echo test > " STUB_CALL(MiscCtrlApiEntStsCB));
		/* コールバック関数の保持 */
		pthread_mutex_lock(&CbMtxm_); 
		p_MiscCtrlStsCb = p_fp ; 
		pthread_mutex_unlock(&CbMtxm_);

		/* OS間通信状態通知のCB呼び出し */
		system("echo test > " STUB_PARAM_CTL3(0));
		
	} while(FALSE);

	return ret;
}

/*---------------------------------------------------- */
/* Miscステータス通知終了 */
INT32 MiscCtrlApiDelStsCB(void* p_ccm_obj)
{
	INT32 ret = MISC_CTRL_RET_SUCCESS;
	do
	{
		/* 引数チェック */
		if (!p_ccm_obj) 
		{
			ret = MISC_CTRL_RET_ERR_PARAM;
			break;
		}
		/* エラーチェック */
		/* ファイル書込みにて戻り値が決まっている場合は戻り値を取得 */
		ret = ReadFuncResult(STUB_FUNC(MiscCtrlApiDelStsCB), MISC_CTRL_RET_SUCCESS);
		if (MISC_CTRL_RET_SUCCESS != ret)
		{
			break;
		}
		/* callイベント発生 */
		system("echo test > " STUB_CALL(MiscCtrlApiDelStsCB));
		pthread_mutex_lock(&CbMtxm_);
		/* コールバック関数ポインタ破棄 */
		p_MiscCtrlStsCb = NULL;
		pthread_mutex_unlock(&CbMtxm_);

	} while(FALSE);

	return ret;
}

/*---------------------------------------------------- */
/* Miscイベント通知開始 */
INT32 MiscCtrlApiEntDataNtyCB(void* p_ccm_obj, MiscCtrlApiDataNtyCB p_fp, MiscCtrlApiNtyFilter* p_filter)
{
	(void)p_filter;
	
	
	INT32 ret = MISC_CTRL_RET_SUCCESS;
	do
	{

		/* 引数チェック */
		if ((!p_ccm_obj) || (!p_fp) || (!p_filter))
		{
			ret = MISC_CTRL_RET_ERR_PARAM;
			break;
		}
		/* エラーチェック */
		/* ファイル書込みにて戻り値が決まっている場合は戻り値を取得 */
		ret = ReadFuncResult(STUB_FUNC(MiscCtrlApiEntDataNtyCB), MISC_CTRL_RET_SUCCESS);
		
		if (MISC_CTRL_RET_SUCCESS != ret)
		{
			break;
		}
		/* callイベント発生 */
		system("echo test > " STUB_CALL(MiscCtrlApiEntDataNtyCB));

		CcmObj_t*	p_ccm = (CcmObj_t*)p_ccm_obj;

		/* CB関数を保持する前がNULLの場合 */
		if( NULL == p_MiscCtrlCb )
		{
			/* コールバック関数の保持 */
			pthread_mutex_lock(&CbMtxm_);
			p_MiscCtrlCb = p_fp ;
			pthread_mutex_unlock(&CbMtxm_);

			/* 周期スレッド生成 */
			p_ccm->closing_m = FALSE;

			/* ミューテックス初期化 */
			if (pthread_mutex_init(&p_ccm->cond_mtx_m, NULL)
			||  pthread_cond_init(&p_ccm->cond_m, NULL)
			||  pthread_mutex_lock(&p_ccm->cond_mtx_m))
			{
				ret = CCM_RET_ERR_RESOURCE;
				break;
			}

			if(pthread_create(&p_ccm->thid_m, NULL, CcmCycleThreadMisc, p_ccm))
			{
				ret = CCM_RET_ERR_RESOURCE;
				break;
			}

			/* 同期検知・経路情報通知(BEYE/FEYE)のCBをコール */
			system("echo test > " STUB_PARAM_CTL2(1));

			// 周期スレッド駆動 
			pthread_mutex_lock(&p_ccm->cond_mtx_m);
			pthread_cond_signal(&p_ccm->cond_m);
			pthread_mutex_unlock(&p_ccm->cond_mtx_m);
			threadf = 1;
		}

	} while(FALSE);
	
	return ret;
	
}

/*---------------------------------------------------- */
/* Miscイベント通知終了 */
INT32 MiscCtrlApiDelDataNtyCB(void* p_ccm_obj)
{
	INT32 ret = MISC_CTRL_RET_SUCCESS;
	do
	{
		/* 引数チェック */
		if (!p_ccm_obj)
		{
			ret = MISC_CTRL_RET_ERR_PARAM;
			break;
		}
		/* エラーチェック */
		/* ファイル書込みにて戻り値が決まっている場合は戻り値を取得 */
		ret = ReadFuncResult(STUB_FUNC(MiscCtrlApiDelDataNtyCB), MISC_CTRL_RET_SUCCESS);
		if (MISC_CTRL_RET_SUCCESS != ret)
		{
			break;
		}

		/* callイベント発生 */
		system("echo test > " STUB_CALL(MiscCtrlApiDelDataNtyCB));
		CcmObj_t*	p_ccm = (CcmObj_t*)p_ccm_obj;
		
		if (1 == threadf)
		{
			/* 周期スレッドを停止させる */
			pthread_mutex_lock(&p_ccm->cond_mtx_m);
			p_ccm->closing_m = TRUE;
			pthread_cond_signal(&p_ccm->cond_m);
			pthread_mutex_unlock(&p_ccm->cond_mtx_m);
			pthread_join(p_ccm->thid_m, NULL);
			
			cmdf = 0;
			threadf =0;
			
			/* コールバック関数ポインタ破棄 */
			pthread_mutex_lock(&CbMtxm_);
			p_MiscCtrlCb = NULL;
			pthread_mutex_unlock(&CbMtxm_);
			
			/* リソースの開放 */
			pthread_cond_destroy(&p_ccm->cond_m);
			pthread_mutex_destroy(&p_ccm->cond_mtx_m);
			//free(p_ccm_obj);
		}
	
	} while(FALSE);
	
	return ret;
}

/*---------------------------------------------------- */
/* Miscコマンド送信 */
INT32 MiscCtrlApiDataSend(void* p_ccm_obj, UINT8 data_type, void* p_data, UINT32 len)
{
	INT32 ret = MISC_CTRL_RET_SUCCESS;
	do
	{
		/* 引数チェック */
		if ((!p_ccm_obj) || (!data_type) || (!p_data) || (!len))
		{
			ret = MISC_CTRL_RET_ERR_PARAM;
			break;
		}
		/* ファイル書込みにて戻り値が決まっている場合は戻り値を取得 */
		ret = ReadFuncResult(STUB_FUNC(MiscCtrlApiDataSend), MISC_CTRL_RET_SUCCESS);
		if (MISC_CTRL_RET_SUCCESS != ret)
		{
			break;
		}

		/* callイベント発生 */
		system("echo test > " STUB_CALL(MiscCtrlApiDataSend));
		UINT8*	p_datam = (UINT8*)p_data;
		if(0x34U == data_type && 0x01 == p_datam[0] && cmdf == 0)
		{
			/* カメラ種別判別通知 */
			system("echo test > " STUB_PARAM_CTL2(0));
			cmdf = 1;
		}
		if(0x36U == data_type && 0x30 == p_datam[0])
		{
			/* スクリーンショット要求 */
			system("echo test > " STUB_PARAM_CTL2(7));	/* FileObserverからの通知を促す */
		}
	
	} while(FALSE);
	return ret;
}
