/*
 * QCarCamスタブテスト
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdint.h>
#include <png.h>

#include "qcarcam.h"

/* ログ出力 */
#if 1
#define PRINT_LOGE(fmt, ...)	fprintf(stderr, "[QCCTEST][E] %s(%d) " fmt "\n",__FUNCTION__,__LINE__,##__VA_ARGS__)
#define PRINT_LOGI(fmt, ...)	fprintf(stderr, "[QCCTEST][I] %s(%d) " fmt "\n",__FUNCTION__,__LINE__,##__VA_ARGS__)
#define PRINT_LOGD(fmt, ...)	fprintf(stderr, "[QCCTEST][D] %s(%d) " fmt "\n",__FUNCTION__,__LINE__,##__VA_ARGS__)
#else
#define PRINT_LOGE(fmt, ...)
#define PRINT_LOGI(fmt, ...)
#define PRINT_LOGD(fmt, ...)
#endif

/* デファイン定義 */
#define QCARCAM_TEST_BUF_SIZ_1920_1080	(1920*4*1080)
#define QCARCAM_TEST_BUF_SIZ_1280_720	(1280*4*720)

/* enum定義 */
typedef enum {
	QCARCAM_TEST_HNDL_TYPE_CAMERA = 0,
	QCARCAM_TEST_HNDL_TYPE_HDMI,
	QCARCAM_TEST_HNDL_TYPE_MAX
} qcarcam_test_hndl_type;

/* 関数プロトタイプ */
static int initialize(void);
static void finalize(void);
static int InitializeQcarcam(void);
static int UnInitializeQcarcam(void);
static int OpenQcarCam(void);
static int CloseQcarCam(void);
static int QueryQcarCamInput(void);
static int SetQcarCamBuffer(void);
static int SetQcarCamParameter(void);
static int StartQcarCam(void);
static int StopQcarCam(void);
static QCarCamRet_e QCarCamCallbackCamera(const QCarCamHndl_t hndl, const uint32_t eventId, const QCarCamEventPayload_t *pPayload, void  *pPrivateData);
static QCarCamRet_e QCarCamCallbackHdmi  (const QCarCamHndl_t hndl, const uint32_t eventId, const QCarCamEventPayload_t *pPayload, void  *pPrivateData);;
static int GetFrameQCarcamCAMERA(void);
static int GetFrameQCarcamHDMI(void);
static int ReserveQcarCam(void);
static int ReleaseQcarCam(void);
static int RegisterQCarCamEventCallback(void);
static int UnregisterQCarCamEventCallback(void);

/* グローバル変数 */
QCarCamHndl_t gQcarCamTestHndl[QCARCAM_TEST_HNDL_TYPE_MAX];	// QCarCamハンドル(0番目:カメラ、1番目：HDMI)

QCarCamBuffer_t gQcarCamTestBufCamera[5+1] =	// QCarCamバッファ(カメラ用)
{
	{// buffers[0]
		{// planes[3]
			{	// planes[0]
				1920,								// width in pixels
				1080,								// height in pixels
				1920*4,								// stride in bytes
//				1920,								// stride in bytes
				QCARCAM_TEST_BUF_SIZ_1920_1080,		// size in bytes
				0,									// memHndl
				0									// offset
			},
			{	// planes[1]
				0,									// width in pixels
				0,									// height in pixels
				0,									// stride in bytes
				0,									// size in bytes
				0,									// memHndl
				0									// offset
			},
			{	// planes[2]
				0,									// width in pixels
				0,									// height in pixels
				0,									// stride in bytes
				0,									// size in bytes
				0,									// memHndl
				0									// offset
			}
		},
		1												// n_planes
	},
	{// buffers[1]
		{// planes[3]
			{	// planes[0]
				1920,								// width in pixels
				1080,								// height in pixels
				1920*4,								// stride in bytes
//				1920,								// stride in bytes
				QCARCAM_TEST_BUF_SIZ_1920_1080,		// size in bytes
				0,									// memHndl
				0									// offset
			},
			{	// planes[1]
				0,									// width in pixels
				0,									// height in pixels
				0,									// stride in bytes
				0,									// size in bytes
				0,									// memHndl
				0									// offset
			},
			{	// planes[2]
				0,									// width in pixels
				0,									// height in pixels
				0,									// stride in bytes
				0,									// size in bytes
				0,									// memHndl
				0									// offset
			}
		},
		1												// n_planes
	},
	{// buffers[2]
		{// planes[3]
			{	// planes[0]
				1920,								// width in pixels
				1080,								// height in pixels
				1920*4,								// stride in bytes
//				1920,								// stride in bytes
				QCARCAM_TEST_BUF_SIZ_1920_1080,		// size in bytes
				0,									// memHndl
				0									// offset
			},
			{	// planes[1]
				0,									// width in pixels
				0,									// height in pixels
				0,									// stride in bytes
				0,									// size in bytes
				0,									// memHndl
				0									// offset
			},
			{	// planes[2]
				0,									// width in pixels
				0,									// height in pixels
				0,									// stride in bytes
				0,									// size in bytes
				0,									// memHndl
				0									// offset
			}
		},
		1												// n_planes
	},
	{// buffers[3]
		{// planes[3]
			{	// planes[0]
				1920,								// width in pixels
				1080,								// height in pixels
				1920*4,								// stride in bytes
//				1920,								// stride in bytes
				QCARCAM_TEST_BUF_SIZ_1920_1080,		// size in bytes
				0,									// memHndl
				0									// offset
			},
			{	// planes[1]
				0,									// width in pixels
				0,									// height in pixels
				0,									// stride in bytes
				0,									// size in bytes
				0,									// memHndl
				0									// offset
			},
			{	// planes[2]
				0,									// width in pixels
				0,									// height in pixels
				0,									// stride in bytes
				0,									// size in bytes
				0,									// memHndl
				0									// offset
			}
		},
		1												// n_planes
	},
	{// buffers[4]
		{// planes[3]
			{	// planes[0]
				1920,								// width in pixels
				1080,								// height in pixels
				1920*4,								// stride in bytes
//				1920,								// stride in bytes
				QCARCAM_TEST_BUF_SIZ_1920_1080,		// size in bytes
				0,									// memHndl
				0									// offset
			},
			{	// planes[1]
				0,									// width in pixels
				0,									// height in pixels
				0,									// stride in bytes
				0,									// size in bytes
				0,									// memHndl
				0									// offset
			},
			{	// planes[2]
				0,									// width in pixels
				0,									// height in pixels
				0,									// stride in bytes
				0,									// size in bytes
				0,									// memHndl
				0									// offset
			}
		},
		1												// n_planes
	},
	{// buffers[5]
		{// planes[3]
			{	// planes[0]
				1920,								// width in pixels
				1080,								// height in pixels
				1920*4,								// stride in bytes
//				1920,								// stride in bytes
				QCARCAM_TEST_BUF_SIZ_1920_1080,		// size in bytes
				0,									// memHndl
				0									// offset
			},
			{	// planes[1]
				0,									// width in pixels
				0,									// height in pixels
				0,									// stride in bytes
				0,									// size in bytes
				0,									// memHndl
				0									// offset
			},
			{	// planes[2]
				0,									// width in pixels
				0,									// height in pixels
				0,									// stride in bytes
				0,									// size in bytes
				0,									// memHndl
				0									// offset
			}
		},
		1												// n_planes
	}
};

QCarCamBuffer_t gQcarCamTestBufHdmi[5+1] =	// QCarCamバッファ(HDMI用)
{
	{// buffers[0]
		{// planes[3]
			{	// planes[0]
				1280,								// width in pixels
				720,								// height in pixels
				1280*4,								// stride in bytes
//				1280,								// stride in bytes
				QCARCAM_TEST_BUF_SIZ_1280_720,		// size in bytes
				0,									// memHndl
				0									// offset
			},
			{	// planes[1]
				0,									// width in pixels
				0,									// height in pixels
				0,									// stride in bytes
				0,									// size in bytes
				0,									// memHndl
				0									// offset
			},
			{	// planes[2]
				0,									// width in pixels
				0,									// height in pixels
				0,									// stride in bytes
				0,									// size in bytes
				0,									// memHndl
				0									// offset
			}
		},
		1												// n_planes
	},
	{// buffers[1]
		{// planes[3]
			{	// planes[0]
				1280,								// width in pixels
				720,								// height in pixels
				1280*4,								// stride in bytes
//				1280,								// stride in bytes
				QCARCAM_TEST_BUF_SIZ_1280_720,		// size in bytes
				0,									// memHndl
				0									// offset
			},
			{	// planes[1]
				0,									// width in pixels
				0,									// height in pixels
				0,									// stride in bytes
				0,									// size in bytes
				0,									// memHndl
				0									// offset
			},
			{	// planes[2]
				0,									// width in pixels
				0,									// height in pixels
				0,									// stride in bytes
				0,									// size in bytes
				0,									// memHndl
				0									// offset
			}
		},
		1												// n_planes
	},
	{// buffers[2]
		{// planes[3]
			{	// planes[0]
				1280,								// width in pixels
				720,								// height in pixels
				1280*4,								// stride in bytes
//				1280,								// stride in bytes
				QCARCAM_TEST_BUF_SIZ_1280_720,		// size in bytes
				0,									// memHndl
				0									// offset
			},
			{	// planes[1]
				0,									// width in pixels
				0,									// height in pixels
				0,									// stride in bytes
				0,									// size in bytes
				0,									// memHndl
				0									// offset
			},
			{	// planes[2]
				0,									// width in pixels
				0,									// height in pixels
				0,									// stride in bytes
				0,									// size in bytes
				0,									// memHndl
				0									// offset
			}
		},
		1												// n_planes
	},
	{// buffers[3]
		{// planes[3]
			{	// planes[0]
				1280,								// width in pixels
				720,								// height in pixels
				1280*4,								// stride in bytes
//				1280,								// stride in bytes
				QCARCAM_TEST_BUF_SIZ_1280_720,		// size in bytes
				0,									// memHndl
				0									// offset
			},
			{	// planes[1]
				0,									// width in pixels
				0,									// height in pixels
				0,									// stride in bytes
				0,									// size in bytes
				0,									// memHndl
				0									// offset
			},
			{	// planes[2]
				0,									// width in pixels
				0,									// height in pixels
				0,									// stride in bytes
				0,									// size in bytes
				0,									// memHndl
				0									// offset
			}
		},
		1												// n_planes
	},
	{// buffers[4]
		{// planes[3]
			{	// planes[0]
				1280,								// width in pixels
				720,								// height in pixels
				1280*4,								// stride in bytes
//				1280,								// stride in bytes
				QCARCAM_TEST_BUF_SIZ_1280_720,		// size in bytes
				0,									// memHndl
				0									// offset
			},
			{	// planes[1]
				0,									// width in pixels
				0,									// height in pixels
				0,									// stride in bytes
				0,									// size in bytes
				0,									// memHndl
				0									// offset
			},
			{	// planes[2]
				0,									// width in pixels
				0,									// height in pixels
				0,									// stride in bytes
				0,									// size in bytes
				0,									// memHndl
				0									// offset
			}
		},
		1												// n_planes
	},
	{// buffers[5]
		{// planes[3]
			{	// planes[0]
				1280,								// width in pixels
				720,								// height in pixels
				1280*4,								// stride in bytes
//				1280,								// stride in bytes
				QCARCAM_TEST_BUF_SIZ_1280_720,		// size in bytes
				0,									// memHndl
				0									// offset
			},
			{	// planes[1]
				0,									// width in pixels
				0,									// height in pixels
				0,									// stride in bytes
				0,									// size in bytes
				0,									// memHndl
				0									// offset
			},
			{	// planes[2]
				0,									// width in pixels
				0,									// height in pixels
				0,									// stride in bytes
				0,									// size in bytes
				0,									// memHndl
				0									// offset
			}
 		},
		1												// n_planes
	},
};

QCarCamBufferList_t gQcarCamTestBufs[QCARCAM_TEST_HNDL_TYPE_MAX]=	// QCarCamバッファリスト(0番目:カメラ、1番目：HDMI)
 {
	// CAMERA
	{
		QCARCAM_BUFFERLIST_ID_OUTPUT_START,
		QCARCAM_FMT_UYVY_8,						// color_fmt (0x07080102)
		gQcarCamTestBufCamera,					// buffers
		5+1,									// n_buffers
		QCARCAM_BUFFER_FLAG_SECURE				// flags
	},
	// HDMI
	{
		QCARCAM_BUFFERLIST_ID_OUTPUT_START,
		QCARCAM_FMT_UYVY_8,						// color_fmt (0x07080102)
		gQcarCamTestBufHdmi,					// buffers
		5+1,									// n_buffers
		QCARCAM_BUFFER_FLAG_SECURE				// flags
	}
};

/*****************************************************************************
 処理概要：	テストプログラム初期化処理
*****************************************************************************/
static int initialize(void)
{
	unsigned int typeCnt = 0;
	unsigned int buffCnt = 0;

//	PRINT_LOGD("initialize start.");

	memset(gQcarCamTestHndl, 0x00, sizeof(gQcarCamTestHndl));

	// Malloc QcarCamTestBuffers
	for(typeCnt = 0; typeCnt < QCARCAM_TEST_HNDL_TYPE_MAX; typeCnt++)
	{
		for(buffCnt = 0; buffCnt < gQcarCamTestBufs[typeCnt].nBuffers; buffCnt++)
		{
			gQcarCamTestBufs[typeCnt].pBuffers[buffCnt].planes[0].memHndl
				= (uint64_t) malloc(gQcarCamTestBufs[typeCnt].pBuffers[buffCnt].planes[0].size);

			if((uint64_t) nullptr == gQcarCamTestBufs[typeCnt].pBuffers[buffCnt].planes[0].memHndl)
			{
				PRINT_LOGE("initialize() malloc() failed! typeCnt[%u] buffCnt[%u] size[%u]", typeCnt, buffCnt, gQcarCamTestBufs[typeCnt].pBuffers[buffCnt].planes[0].size);
				return -1;
			}
		}
	}

//	PRINT_LOGD("initialize end.");

	return 0;
}

/*****************************************************************************
 処理概要：	テストプログラム終了処理
*****************************************************************************/
static void finalize(void)
{
	unsigned int typeCnt = 0;
	unsigned int buffCnt = 0;

//	PRINT_LOGD("finalize start.");

	memset(gQcarCamTestHndl, 0x00, sizeof(gQcarCamTestHndl));

	// Free QcarCamTestBuffers
	for(typeCnt = 0; typeCnt < QCARCAM_TEST_HNDL_TYPE_MAX; typeCnt++)
	{
		for(buffCnt = 0; buffCnt < gQcarCamTestBufs[typeCnt].nBuffers; buffCnt++)
		{
			if((uint64_t) nullptr != gQcarCamTestBufs[typeCnt].pBuffers[buffCnt].planes[0].memHndl)
			{
				free((void*) gQcarCamTestBufs[typeCnt].pBuffers[buffCnt].planes[0].memHndl);
				gQcarCamTestBufs[typeCnt].pBuffers[buffCnt].planes[0].memHndl = (uint64_t) nullptr;
			}
		}
	}

//	PRINT_LOGD("finalize end.");
}

/*****************************************************************************
 処理概要：	メイン関数
*****************************************************************************/
int main(int argc, char *argv[])
{
	int ret;
	int iRtn = -1;

	PRINT_LOGD("starting...");

	/* テストプログラム初期化 */
	ret = initialize();
	if (ret < 0) {
		finalize();
		return -1;
	}

	/* QCarCam初期化 */
	ret = InitializeQcarcam();
	if (ret < 0) {
		finalize();
		return -1;
	}

	/* QCarCamオープン */
	ret = OpenQcarCam();
	if (ret < 0) {
		finalize();
		return -1;
	}

	/* QCarCamインプット照会 */
	ret = QueryQcarCamInput();
	if (ret < 0) {
		finalize();
		return -1;
	}

	/* QCarCamバッファセット */
	ret = SetQcarCamBuffer();
	if (ret < 0) {
		finalize();
		return -1;
	}

	/* QCarCamパラメータセット */
	ret = SetQcarCamParameter();
	if (ret < 0) {
		finalize();
		return -1;
	}

	if (0 > RegisterQCarCamEventCallback()) {finalize(); return iRtn;}
	if (0 > ReserveQcarCam()) {finalize(); return iRtn;}

	/* QCarCamスタート */
	ret = StartQcarCam();
	if (ret < 0) {
		finalize();
		return -1;
	}

	sleep(10);

	/* QCarCamストップ */
	ret = StopQcarCam();
	if (ret < 0) {
		finalize();
		return -1;
	}

	if (0 > ReleaseQcarCam()) {finalize(); return iRtn;}

	/* QCarCamクローズ */
	ret = CloseQcarCam();
	if (ret < 0) {
		finalize();
		return -1;
	}

	if (0 > UnregisterQCarCamEventCallback()) {finalize(); return iRtn;}

	/* QCarCam終了 */
	ret = UnInitializeQcarcam();
	if (ret < 0) {
		finalize();
		return -1;
	}

	/*  終了 */
	PRINT_LOGD("ending...");
	finalize();

	return 0;
}

/*****************************************************************************
 処理概要：	QCarCam初期化処理
*****************************************************************************/
static int InitializeQcarcam(void)
{
	int ret = 0;

	QCarCamRet_e	qret{QCARCAM_RET_OK};
	QCarCamInit_t	qcarcam_init{};

	//qcarcam_init.version	= QCARCAM_VERSION;
	//qcarcam_init.debug_tag	= (char *)"QCARCAM_VHAL";

	PRINT_LOGI("qcarcam_initialize call");
	qret = QCarCamInitialize(&qcarcam_init);

	if(QCARCAM_RET_OK == qret)
	{
		PRINT_LOGI("qcarcam_initialize success. ret = %d", qret);
		ret = 0;
	}
	else
	{
		PRINT_LOGE("qcarcam_initialize failed! ret = %d", qret);
		ret = -1;
	}

	return ret;
}

/*****************************************************************************
 処理概要：	QCarCam終了処理
*****************************************************************************/
static int UnInitializeQcarcam(void)
{
	int ret = 0;
	QCarCamRet_e	qret{QCARCAM_RET_OK};

	PRINT_LOGI("qcarcam_uninitialize call");
	qret = QCarCamUninitialize();

	if(QCARCAM_RET_OK == qret)
	{
		PRINT_LOGI("qcarcam_uninitialize success. ret = %d", qret);
		ret = 0;
	}
	else
	{
		PRINT_LOGE("qcarcam_uninitialize failed! ret = %d", qret);
		ret = -1;
	}

	return ret;
}

/*****************************************************************************
 処理概要：	QCarCamオープン処理
*****************************************************************************/
static int OpenQcarCam(void)
{
	int ret = 0;

	/* QCarCamオープン(カメラ) */
	PRINT_LOGI("qcarcam_open(CAMERA) call");
	QCarCamOpen_t  qCarCamOpen;
	QCarCamRet_e qCarCamRet = QCARCAM_RET_FAILED;
	qCarCamOpen.inputs[0].inputId = QCARCAM_TEST_HNDL_TYPE_CAMERA;
	qCarCamRet = QCarCamOpen(&qCarCamOpen, &gQcarCamTestHndl[QCARCAM_TEST_HNDL_TYPE_CAMERA]);

	if(QCARCAM_RET_OK != qCarCamRet)
	{
		PRINT_LOGE("qcarcam_open(CAMERA) failed!");
		return(-1);
	}

	/* QCarCamオープン(HDMI) */
	PRINT_LOGI("qcarcam_open(HDMI) call");
	qCarCamOpen.inputs[0].inputId = QCARCAM_TEST_HNDL_TYPE_HDMI;
	qCarCamRet = QCarCamOpen(&qCarCamOpen, &gQcarCamTestHndl[QCARCAM_TEST_HNDL_TYPE_HDMI]);

	if(QCARCAM_RET_OK != qCarCamRet)
	{
		PRINT_LOGE("qcarcam_open(HDMI) failed!");
		return(-1);
	}

	return ret;
}

/*****************************************************************************
 処理概要：	QCarCamクローズ処理
*****************************************************************************/
static int CloseQcarCam(void)
{
	int ret = 0;
	QCarCamRet_e	qret{QCARCAM_RET_OK};

	/* QCarCamクローズ(カメラ) */
	PRINT_LOGI("qcarcam_close(CAMERA) call");
	qret = QCarCamClose(gQcarCamTestHndl[QCARCAM_TEST_HNDL_TYPE_CAMERA]);

	if(QCARCAM_RET_OK != qret)
	{
		PRINT_LOGE("qcarcam_close(CAMERA) failed!");
		return(-1);
	}

	/* QCarCamクローズ(HDMI) */
	PRINT_LOGI("qcarcam_close(HDMI) call");
	qret = QCarCamClose(gQcarCamTestHndl[QCARCAM_TEST_HNDL_TYPE_HDMI]);

	if(QCARCAM_RET_OK != qret)
	{
		PRINT_LOGE("qcarcam_close(HDMI) failed!");
		return(-1);
	}

	return ret;
}

/*****************************************************************************
 処理概要：	QCarCamインプット照会処理
*****************************************************************************/
static int QueryQcarCamInput(void)
{
	int ret = 0;
	QCarCamRet_e qret{QCARCAM_RET_OK};
	unsigned int inputNum = 0;
	unsigned int resultNum = 0;
	QCarCamInput_t *pInputs = nullptr;

	PRINT_LOGI("qcarcam_query_inputs(1st) call");
	qret = QCarCamQueryInputs(NULL, 0, &inputNum);

	if(QCARCAM_RET_OK != qret)
	{
		PRINT_LOGE("qcarcam_query_inputs(1st) failed! qret[%d]", qret);
		return(-1);
	}

	if(0 == inputNum)
	{
		PRINT_LOGE("inputNum is zero!!");
		return(-1);
	}

	pInputs = (QCarCamInput_t *)calloc(inputNum, sizeof(QCarCamInput_t)); 

	if(nullptr == pInputs)
	{
		PRINT_LOGE("calloc() failed! inputNum[%u] sizeof(qcarcam_input_t)[%lu]", inputNum, sizeof(QCarCamInput_t));
		return(-1);
	}

	PRINT_LOGI("qcarcam_query_inputs(2nd) call");
	qret = QCarCamQueryInputs(pInputs, inputNum, &resultNum);

	if(QCARCAM_RET_OK != qret)
	{
		PRINT_LOGE("qcarcam_query_inputs(2nd) failed! qret[%d]", qret);
		return(-1);
	}

	if(inputNum != resultNum)
	{
		PRINT_LOGE("inputNum[%u] != resultNum[%u]!!", inputNum, resultNum);
		return(-1);
	}

	for (unsigned int id = 0; id < inputNum; id ++)
	{
		if (id == 0)
		{
			PRINT_LOGD("---------------------------");
			PRINT_LOGD("qcarcam_query_inputs Result");
			PRINT_LOGD("---------------------------");
		}
		PRINT_LOGD("pInputs[%u].inputId   = [%u]", id, pInputs[id].inputId);
		PRINT_LOGD("pInputs[%u].inputName = [%s]", id, pInputs[id].inputName);
		PRINT_LOGD("pInputs[%u].numModes  = [%u]", id, pInputs[id].numModes);
		PRINT_LOGD("---------------------------");

		QCarCamInputModes_t qCarCamInputModes;
		QCarCamMode_t qCarCamMode[pInputs[id].numModes];
		qCarCamInputModes.pModes = qCarCamMode;
		qCarCamInputModes.numModes = pInputs[id].numModes;
		PRINT_LOGI("QCarCamQueryInputModes call");
		qret = QCarCamQueryInputModes(pInputs[id].inputId, &qCarCamInputModes);
		if(QCARCAM_RET_OK != qret)
		{
			PRINT_LOGE("QCarCamQueryInputModes failed! qret[%d]", qret);
			ret = -1;
			break;
		}
		else
		{
			for (unsigned int mid = 0; mid < pInputs[id].numModes; mid ++)
			{
				PRINT_LOGD("qCarCamMode[%u].numSources = [%u]",   mid, qCarCamMode[mid].numSources);
				PRINT_LOGD("qCarCamMode[%u].width      = [%u]",   mid, qCarCamMode[mid].sources->width);
				PRINT_LOGD("qCarCamMode[%u].height     = [%u]",   mid, qCarCamMode[mid].sources->height);
				PRINT_LOGD("qCarCamMode[%u].colorFmt   = [0x%X]", mid, qCarCamMode[mid].sources->colorFmt);
				PRINT_LOGD("qCarCamMode[%u].fps        = [%f]",   mid, qCarCamMode[mid].sources->fps);
			}
			PRINT_LOGD("---------------------------");
		}
	}

	free(pInputs);

	return ret;
}

/*****************************************************************************
 処理概要：	QCarCamバッファセット処理
*****************************************************************************/
static int SetQcarCamBuffer(void)
{
	int ret = 0;
	QCarCamRet_e	qret{QCARCAM_RET_OK};

#if 1	// TK add
	for (int i = QCARCAM_TEST_HNDL_TYPE_CAMERA; i < QCARCAM_TEST_HNDL_TYPE_MAX; i++)
	{
		for (uint32_t j = 0; j < gQcarCamTestBufs[i].nBuffers; j++)
		{
			QCarCamBuffer_t&	buf = gQcarCamTestBufs[i].pBuffers[j];
			for (uint32_t k = 0; k < buf.numPlanes; k++)
			{
				buf.planes[k].memHndl = (uint64_t)(uintptr_t)calloc(buf.planes[k].stride, buf.planes[k].height);
			}
		}
	}
#endif

	/* QCarCamバッファセット(カメラ) */
	PRINT_LOGI("qcarcam_s_buffers(CAMERA) call");
	qret = QCarCamSetBuffers(gQcarCamTestHndl[QCARCAM_TEST_HNDL_TYPE_CAMERA], &gQcarCamTestBufs[QCARCAM_TEST_HNDL_TYPE_CAMERA]);

	if(QCARCAM_RET_OK != qret)
	{
		PRINT_LOGE("qcarcam_s_buffers(CAMERA) failed!");
		return(-1);
	}

	/* QCarCamバッファセット(HDMI) */
	PRINT_LOGI("qcarcam_s_buffers(HDMI) call");
	qret = QCarCamSetBuffers(gQcarCamTestHndl[QCARCAM_TEST_HNDL_TYPE_HDMI], &gQcarCamTestBufs[QCARCAM_TEST_HNDL_TYPE_HDMI]);

	if(QCARCAM_RET_OK != qret)
	{
		PRINT_LOGE("qcarcam_s_buffers(HDMI) failed!");
		return(-1);
	}

	return ret;
}

/*****************************************************************************
 処理概要：	QCarCamパラメータセット処理
*****************************************************************************/
static int SetQcarCamParameter(void)
{
	int ret = 0;
	QCarCamRet_e	qret{QCARCAM_RET_OK};
	// qcarcam_param_value_t paramValue;
	uint32_t eventMask;

	/*********************************/
	/* QCarCamパラメータセット(カメラ) */
	/*********************************/

	PRINT_LOGI("qcarcam_s_param(CAMERA) QCARCAM_PARAM_EVENT_MASK call");
	eventMask = (QCARCAM_EVENT_FRAME_READY|QCARCAM_EVENT_ERROR|QCARCAM_EVENT_VENDOR);

	qret = QCarCamSetParam(gQcarCamTestHndl[QCARCAM_TEST_HNDL_TYPE_CAMERA], QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK, &eventMask, sizeof(eventMask));

	if(QCARCAM_RET_OK != qret)
	{
		PRINT_LOGE("qcarcam_s_param(CAMERA) failed!");
		return(-1);
	}

	/*********************************/
	/* QCarCamパラメータセット(HDMI) */
	/*********************************/

	PRINT_LOGI("qcarcam_s_param(HDMI) QCARCAM_PARAM_EVENT_MASK call");
	eventMask = (QCARCAM_EVENT_FRAME_READY|QCARCAM_EVENT_ERROR|QCARCAM_EVENT_VENDOR);

	qret = QCarCamSetParam(gQcarCamTestHndl[QCARCAM_TEST_HNDL_TYPE_HDMI], QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK, &eventMask, sizeof(eventMask));

	if(QCARCAM_RET_OK != qret)
	{
		PRINT_LOGE("qcarcam_s_param(HDMI) failed!");
		return(-1);
	}

	return ret;
}

/*****************************************************************************
 処理概要：	QCarCamsコールバック関数(カメラ用)
*****************************************************************************/
static QCarCamRet_e QCarCamCallbackCamera
(
    const QCarCamHndl_t hndl,
    const uint32_t eventId,
    const QCarCamEventPayload_t *pPayload,
    void  *pPrivateData
)
{
	QCarCamRet_e qCarCamRet = QCARCAM_RET_FAILED;
	if(QCARCAM_HNDL_INVALID == hndl)
	{
		PRINT_LOGE("%s() Error!", __FUNCTION__);
		return qCarCamRet;
	}

	switch(eventId)
	{
		case QCARCAM_EVENT_FRAME_READY:
			PRINT_LOGI("%s(QCARCAM_EVENT_FRAME_READY) Called", __FUNCTION__);
			GetFrameQCarcamCAMERA();
			break;
		case QCARCAM_EVENT_INPUT_SIGNAL:
			PRINT_LOGI("%s(QCARCAM_EVENT_INPUT_SIGNAL) Called", __FUNCTION__);
			break;
		case QCARCAM_EVENT_ERROR:
			PRINT_LOGI("%s(QCARCAM_EVENT_ERROR) Called", __FUNCTION__);
			break;
		case QCARCAM_EVENT_VENDOR:
			PRINT_LOGI("%s(QCARCAM_EVENT_VENDOR) Called", __FUNCTION__);
			break;
		case QCARCAM_EVENT_PROPERTY_NOTIFY:
			PRINT_LOGI("%s(QCARCAM_EVENT_PROPERTY_NOTIFY) Called", __FUNCTION__);
			break;
		case QCARCAM_EVENT_FRAME_SOF:
			PRINT_LOGI("%s(QCARCAM_EVENT_FRAME_SOF) Called", __FUNCTION__);
			break;
		case QCARCAM_EVENT_RECOVERY:
			PRINT_LOGI("%s(QCARCAM_EVENT_RECOVERY) Called", __FUNCTION__);
			break;
		// case QCARCAM_EVENT_RECOVERY_SUCCESS:
		// 	PRINT_LOGI("%s(QCARCAM_EVENT_RECOVERY_SUCCESS) Called", __FUNCTION__);
		// 	break;
		// case QCARCAM_EVENT_ERROR_ABORTED:
		// 	PRINT_LOGI("%s(QCARCAM_EVENT_ERROR_ABORTED) Called", __FUNCTION__);
		// 	break;
		// case QCARCAM_EVENT_FRAME_FREEZE:
		// 	PRINT_LOGI("%s(QCARCAM_EVENT_FRAME_FREEZE) Called", __FUNCTION__);
		// 	break;
		// case QCARCAM_EVENT_FRAME_DROP:
		// 	PRINT_LOGI("%s(QCARCAM_EVENT_FRAME_DROP) Called", __FUNCTION__);
		// 	break;
		default:
			PRINT_LOGE("%s(UnknownEvent) Called event_id[0x%x]", __FUNCTION__, eventId);
			break;
	}

	return qCarCamRet;
}

/*****************************************************************************
 処理概要：	QCarCamsコールバック関数(HDMI用)
*****************************************************************************/
static QCarCamRet_e QCarCamCallbackHdmi
(
    const QCarCamHndl_t hndl,
    const uint32_t eventId,
    const QCarCamEventPayload_t *pPayload,
    void  *pPrivateData
)
{
	QCarCamRet_e qCarCamRet = QCARCAM_RET_FAILED;
	if(QCARCAM_HNDL_INVALID == hndl)
	{
		PRINT_LOGE("%s() Error!", __FUNCTION__);
		return qCarCamRet;
	}

	switch(eventId)
	{
		case QCARCAM_EVENT_FRAME_READY:
			PRINT_LOGI("%s(QCARCAM_EVENT_FRAME_READY) Called", __FUNCTION__);
			GetFrameQCarcamHDMI();
			break;
		case QCARCAM_EVENT_INPUT_SIGNAL:
			PRINT_LOGI("%s(QCARCAM_EVENT_INPUT_SIGNAL) Called", __FUNCTION__);
			break;
		case QCARCAM_EVENT_ERROR:
			PRINT_LOGI("%s(QCARCAM_EVENT_ERROR) Called", __FUNCTION__);
			break;
		case QCARCAM_EVENT_VENDOR:
			PRINT_LOGI("%s(QCARCAM_EVENT_VENDOR) Called", __FUNCTION__);
			break;
		case QCARCAM_EVENT_PROPERTY_NOTIFY:
			PRINT_LOGI("%s(QCARCAM_EVENT_PROPERTY_NOTIFY) Called", __FUNCTION__);
			break;
		case QCARCAM_EVENT_FRAME_SOF:
			PRINT_LOGI("%s(QCARCAM_EVENT_FRAME_SOF) Called", __FUNCTION__);
			break;
		case QCARCAM_EVENT_RECOVERY:
			PRINT_LOGI("%s(QCARCAM_EVENT_RECOVERY) Called", __FUNCTION__);
			break;
		// case QCARCAM_EVENT_RECOVERY_SUCCESS:
		// 	PRINT_LOGI("%s(QCARCAM_EVENT_RECOVERY_SUCCESS) Called", __FUNCTION__);
		// 	break;
		// case QCARCAM_EVENT_ERROR_ABORTED:
		// 	PRINT_LOGI("%s(QCARCAM_EVENT_ERROR_ABORTED) Called", __FUNCTION__);
		// 	break;
		// case QCARCAM_EVENT_FRAME_FREEZE:
		// 	PRINT_LOGI("%s(QCARCAM_EVENT_FRAME_FREEZE) Called", __FUNCTION__);
		// 	break;
		// case QCARCAM_EVENT_FRAME_DROP:
		// 	PRINT_LOGI("%s(QCARCAM_EVENT_FRAME_DROP) Called", __FUNCTION__);
		// 	break;
		default:
			PRINT_LOGE("%s(UnknownEvent) Called event_id[0x%x]", __FUNCTION__, eventId);
			break;
	}

	return qCarCamRet;
}

/*****************************************************************************
 処理概要：	QCarCamスタート処理
*****************************************************************************/
static int StartQcarCam(void)
{
	int ret = 0;
	QCarCamRet_e	qret{QCARCAM_RET_OK};

	PRINT_LOGI("qcarcam_start(CAMERA) call");
	qret = QCarCamStart(gQcarCamTestHndl[QCARCAM_TEST_HNDL_TYPE_CAMERA]);

	if(QCARCAM_RET_OK != qret)
	{
		PRINT_LOGE("qcarcam_start(CAMERA) failed!");
		return(-1);
	}

	PRINT_LOGI("qcarcam_start(HDMI) call");
	qret = QCarCamStart(gQcarCamTestHndl[QCARCAM_TEST_HNDL_TYPE_HDMI]);

	if(QCARCAM_RET_OK != qret)
	{
		PRINT_LOGE("qcarcam_start(HDMI) failed!");
		return(-1);
	}

	return ret;
}

/*****************************************************************************
 処理概要：	QCarCamストップ処理
*****************************************************************************/
static int StopQcarCam(void)
{
	int ret = 0;
	QCarCamRet_e	qret{QCARCAM_RET_OK};

	PRINT_LOGI("qcarcam_stop(CAMERA) call");
	qret = QCarCamStop(gQcarCamTestHndl[QCARCAM_TEST_HNDL_TYPE_CAMERA]);

	if(QCARCAM_RET_OK != qret)
	{
		PRINT_LOGE("qcarcam_stop(CAMERA) failed!");
		return(-1);
	}

	PRINT_LOGI("qcarcam_stop(HDMI) call");
	qret = QCarCamStop(gQcarCamTestHndl[QCARCAM_TEST_HNDL_TYPE_HDMI]);

	if(QCARCAM_RET_OK != qret)
	{
		PRINT_LOGE("qcarcam_stop(HDMI) failed!");
		return(-1);
	}

	return ret;
}

/*****************************************************************************
 処理概要：	QCarCamフレーム取得処理(CAMERA用)
*****************************************************************************/
static int GetFrameQCarcamCAMERA(void)
{
	int ret = 0;
	QCarCamRet_e	qret{QCARCAM_RET_OK};
	QCarCamFrameInfo_t frame_info;

	memset(&frame_info, 0x00, sizeof(frame_info));

	PRINT_LOGI("qcarcam_get_frame(CAMERA) call");
	qret = QCarCamGetFrame(gQcarCamTestHndl[QCARCAM_TEST_HNDL_TYPE_CAMERA], &frame_info, 0, 0);

	if(QCARCAM_RET_OK != qret)
	{
		PRINT_LOGE("qcarcam_get_frame(CAMERA) failed!");
		return(-1);
	}

	PRINT_LOGI("qcarcam_get_frame(CAMERA) Success idx[%u]", frame_info.bufferIndex);

	return ret;
}

/*****************************************************************************
 処理概要：	QCarCamフレーム取得処理(HDMI用)
*****************************************************************************/
static int GetFrameQCarcamHDMI(void)
{
	int ret = 0;
	QCarCamRet_e	qret{QCARCAM_RET_OK};
	QCarCamFrameInfo_t frame_info;

	memset(&frame_info, 0x00, sizeof(frame_info));

	PRINT_LOGI("qcarcam_get_frame(HDMI) call");
	qret = QCarCamGetFrame(gQcarCamTestHndl[QCARCAM_TEST_HNDL_TYPE_HDMI], &frame_info, 0, 0);

	if(QCARCAM_RET_OK != qret)
	{
		PRINT_LOGE("qcarcam_get_frame(HDMI) failed!");
		return(-1);
	}

	PRINT_LOGI("qcarcam_get_frame(HDMI) Success idx[%u]", frame_info.bufferIndex);

	return ret;
}

static int ReserveQcarCam(void)
{
	int iRtn = -1;
	PRINT_LOGI("QCarCamReserve(CAMERA) call");
	QCarCamRet_e qCarCamRet = QCarCamReserve(gQcarCamTestHndl[QCARCAM_TEST_HNDL_TYPE_CAMERA]);
	if(QCARCAM_RET_OK != qCarCamRet)
		PRINT_LOGE("QCarCamReserve(CAMERA) failed!");
	else
	{
		PRINT_LOGI("QCarCamReserve(HDMI) call");
		qCarCamRet = QCarCamReserve(gQcarCamTestHndl[QCARCAM_TEST_HNDL_TYPE_HDMI]);
		if(QCARCAM_RET_OK != qCarCamRet)
			PRINT_LOGE("QCarCamReserve(HDMI) failed!");
		else
			iRtn = 0;
	}
	return iRtn;
}

static int ReleaseQcarCam(void)
{
	int iRtn = -1;
	PRINT_LOGI("QCarCamRelease(CAMERA) call");
	QCarCamRet_e qCarCamRet = QCarCamRelease(gQcarCamTestHndl[QCARCAM_TEST_HNDL_TYPE_CAMERA]);
	if(QCARCAM_RET_OK != qCarCamRet)
		PRINT_LOGE("QCarCamRelease(CAMERA) failed!");
	else
	{
		PRINT_LOGI("QCarCamRelease(HDMI) call");
		qCarCamRet = QCarCamRelease(gQcarCamTestHndl[QCARCAM_TEST_HNDL_TYPE_HDMI]);
		if(QCARCAM_RET_OK != qCarCamRet)
			PRINT_LOGE("QCarCamRelease(HDMI) failed!");
		else
			iRtn = 0;
	}
	return iRtn;
}

static int RegisterQCarCamEventCallback(void)
{
	int iRtn = -1;
	PRINT_LOGI("QCarCamRegisterEventCallback(CAMERA) call");
	QCarCamRet_e qCarCamRet = QCarCamRegisterEventCallback(gQcarCamTestHndl[QCARCAM_TEST_HNDL_TYPE_CAMERA], &QCarCamCallbackCamera, nullptr);
	if(QCARCAM_RET_OK != qCarCamRet)
		PRINT_LOGE("QCarCamRegisterEventCallback(CAMERA) failed!");
	else
	{
		PRINT_LOGI("QCarCamRegisterEventCallback(HDMI) call");
		qCarCamRet = QCarCamRegisterEventCallback(gQcarCamTestHndl[QCARCAM_TEST_HNDL_TYPE_HDMI], &QCarCamCallbackHdmi, nullptr);
		if(QCARCAM_RET_OK != qCarCamRet)
			PRINT_LOGE("QCarCamRegisterEventCallback(HDMI) failed!");
		else
			iRtn = 0;
	}
	return iRtn;
}

static int UnregisterQCarCamEventCallback(void)
{
	int iRtn = -1;
	PRINT_LOGI("QCarCamUnregisterEventCallback(CAMERA) call");
	QCarCamRet_e qCarCamRet = QCarCamUnregisterEventCallback(gQcarCamTestHndl[QCARCAM_TEST_HNDL_TYPE_CAMERA]);
	if(QCARCAM_RET_OK != qCarCamRet)
		PRINT_LOGE("QCarCamUnregisterEventCallback(CAMERA) failed!");
	else
	{
		PRINT_LOGI("QCarCamUnregisterEventCallback(HDMI) call");
		qCarCamRet = QCarCamUnregisterEventCallback(gQcarCamTestHndl[QCARCAM_TEST_HNDL_TYPE_HDMI]);
		if(QCARCAM_RET_OK != qCarCamRet)
			PRINT_LOGE("QCarCamUnregisterEventCallback(HDMI) failed!");
		else
			iRtn = 0;
	}
	return iRtn;
}
