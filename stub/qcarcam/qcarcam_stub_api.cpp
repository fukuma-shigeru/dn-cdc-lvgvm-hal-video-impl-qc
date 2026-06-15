
#include <stdio.h>
#include <stdarg.h>
//Bev3追加
#include <png.h>

#include "file_observer.h"
#include "qcarcam.h"
#include "qcarcam_metadata.h"
#include <map>

#include "com_stddef.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "sif_thread.h"
#include "sif_thrmsg.h"
#include "spf_timer_public.h"
#include "stub_common.h"
#ifdef __cplusplus
}
#endif


//////////////////////////////////////////////////////////////////////////////////
//	Debug Log
//////////////////////////////////////////////////////////////////////////////////
#if 1	// Log Enable
#define PRINT_LOGE(fmt, ...)	fprintf(stderr, "[QCARCAM][E] %s(%d) " fmt "\n",__FUNCTION__,__LINE__,##__VA_ARGS__)
#define PRINT_LOGI(fmt, ...)	fprintf(stderr, "[QCARCAM][I] %s(%d) " fmt "\n",__FUNCTION__,__LINE__,##__VA_ARGS__)
//#define PRINT_LOGD(fmt, ...)	fprintf(stderr, "[QCARCAM][D] %s(%d) " fmt "\n",__FUNCTION__,__LINE__,##__VA_ARGS__)
#define PRINT_LOGD(fmt, ...)
#else	// Log Disable
#define PRINT_LOGE(fmt, ...)
#define PRINT_LOGI(fmt, ...)
#define PRINT_LOGD(fmt, ...)
#endif

//////////////////////////////////////////////////////////////////////////////////
//	Macro definition
//////////////////////////////////////////////////////////////////////////////////
#define QCARCAM_STUB_CB_THR_NAME			"QCarCamStub_cb"

#define QCARCAM_STUB_CB_THR_MQ_NAME			"QCarCamStub_cb_mq"
#define QCARCAM_STUB_CB_THR_MQ_SIZE			(1024U)
#define QCARCAM_STUB_CB_THR_MQ_NUM			(10U)

#if 0
#define QCARCAM_STUB_CB_CYCLIC_TIME			(16U)	// 60fps(16msec)
#elif 0
#define QCARCAM_STUB_CB_CYCLIC_TIME			(33U)	// 30fps(33msec)
#else
#define QCARCAM_STUB_CB_CYCLIC_TIME			(1000U)	// 1sec(1000msec)
#endif

#define QCARCAM_STUB_AVAILABLE_INPUT_NUM	(2U)	// Available inputs are Camera and HDMI

#define STUB_PATH							"/run/arene/vehicle_fs/var/bev3/stub/qcarcam/"
#define STUB_PATH_FUNC						STUB_PATH "func_ret/"
#define STUB_FUNC(_fnc)						STUB_PATH_FUNC #_fnc
#define QCARCAM_STUB_TEST_CTRL_FILE_EVT_CB	STUB_PATH "QCarCamEvtCB_Test"
#define QCARCAM_STUB_TEST_CTRL_FILE_SIZE	(128U)	// byte

//#define QCARCAM_STUB_INPUT_NAME_CAMERA	"CAMERA"
//#define QCARCAM_STUB_INPUT_NAME_HDMI	"HDMI"
#define QCARCAM_STUB_INPUT_NAME_CAMERA	"INPUT_1_0"
#define QCARCAM_STUB_INPUT_NAME_HDMI	"INPUT_0_0"


//Bev3追加 画像パスのマクロ
#define IMAGE_PATH				"/opt/dc-ivi-pf/tests/dn-cdc-lvgvm-hal-video-impl-qc/certification-program/"
#define CAMERA_PATH				IMAGE_PATH "sample_img_CAMERA.png"
#define CAMERA_PATH2			IMAGE_PATH "sample_img_CAMERA2.png"
#define CAMERA_PATH3			IMAGE_PATH "sample_img_CAMERA3.png"
#define CAMERA_PATH4			IMAGE_PATH "sample_img_CAMERA4.png"
#define CAMERA_PATH5			IMAGE_PATH "sample_img_CAMERA5.png"
#define HDMI_PATH				IMAGE_PATH "sample_img_HDMI.png"
#define HDMI_PATH2				IMAGE_PATH "sample_img_HDMI2.png"
#define HDMI_PATH3				IMAGE_PATH "sample_img_HDMI3.png"
#define HDMI_PATH4				IMAGE_PATH "sample_img_HDMI4.png"
#define HDMI_PATH5				IMAGE_PATH "sample_img_HDMI5.png"


//////////////////////////////////////////////////////////////////////////////////
//	Type definition
//////////////////////////////////////////////////////////////////////////////////
typedef enum : int32_t {
	QCARCAM_STUB_HNDL_TYPE_CAMERA = 0,
	QCARCAM_STUB_HNDL_TYPE_HDMI,
	QCARCAM_STUB_HNDL_TYPE_MAX
} qcarcam_stub_hndl_type;

typedef enum {
	QCARCAM_STUB_CMD_NONE = 0,
	QCARCAM_CMD_START,
	QCARCAM_CMD_STOP,
	QCARCAM_CMD_TIMER_CB,
	QCARCAM_CMD_EXIT,
	QCARCAM_STUB_CMD_TYPE_MAX
} qcarcam_stub_cmd_type;

typedef enum
{
	QCARCAM_INPUT_ID_CAMERA = 0,
	QCARCAM_INPUT_ID_HDMI,
	QCARCAM_INPUT_ID_MAX
} QCarCamInputId;

typedef struct {
	std::string				hndlName;
	bool					isOpen;
	bool					isStart;
	qcarcam_stub_hndl_type	hndlType;
	QCarCamEventCallback_t	eventCb;
	void*					pPrivateData;
	uint32_t				eventMask;
	QCarCamBufferList_t		buffers;
	void*					buffersLocal;
	unsigned int			buffersIdx;
	std::unique_ptr<std::vector<void*>>	bufadr;
} qcarcam_stub_hndl_info;

typedef struct {
	qcarcam_stub_cmd_type	command;
	qcarcam_stub_hndl_info*	pStubHndl;
} qcarcam_stub_thr_msg;

typedef struct {
	spf_timer_t		timerObject;
	bool			isStarted;
} qcarcam_stub_timer_info;

typedef struct {
	uint32_t		inputId;
	QCarCamMode_t	qCarCamMode;
} QCarCamInputIdAndMode;

//Bev3追加 構造体作成
struct PngInfo
{
    png_uint_32 width;
    png_uint_32 height;
    unsigned char *data;
};

namespace
{

using NotifyList = std::map<std::string, StubClientNotifyListener>;
NotifyList list_notify_;

}

//////////////////////////////////////////////////////////////////////////////////
//	Global Valiable
//////////////////////////////////////////////////////////////////////////////////
// Dummy Input Information
const static QCarCamInput_t	dummyInputInf[QCARCAM_STUB_AVAILABLE_INPUT_NUM] =
{
	// CAMERA
	{
		QCARCAM_INPUT_ID_CAMERA,		// inputId
		0,								// devId
		0,								// subdevId
		QCARCAM_STUB_INPUT_NAME_CAMERA,	// inputName
		//Bev3追加・変更 2 -> 4
		4,								// numModes
		0								// flags
	},
	// HDMI
	{
		QCARCAM_INPUT_ID_HDMI,			// inputId
		0,								// devId
		0,								// subdevId
		QCARCAM_STUB_INPUT_NAME_HDMI,	// inputName
		4,								// numModes
		0								// flags
	}
};
const static QCarCamInputIdAndMode dummyQCarCamMode[] =
{
	{
		QCARCAM_INPUT_ID_CAMERA,
		{
			{
				0						// srcId
				, 1920					// width
				, 1080					// height
				, QCARCAM_FMT_UYVY_8	// colorFmt
				, 60.0f					// fps
				, 0						// securityDomain
			}	// sources
			, 1	// numSources
		}
	}
	,
	{
		QCARCAM_INPUT_ID_CAMERA,
		{
			{
				0						// srcId
				, 640					// width
				, 360					// height
				, QCARCAM_FMT_UYVY_8	// colorFmt
				, 60.0f					// fps
				, 0						// securityDomain
			}	// sources
			, 1	// numSources
		}
	}
	,
	{//Bev3追加
		QCARCAM_INPUT_ID_CAMERA,
		{
			{
				0						// srcId
				, 1280					// width
				, 720					// height
				, QCARCAM_FMT_UYVY_8	// colorFmt
				, 60.0f					// fps
				, 0						// securityDomain
			}	// sources
			, 1	// numSources
		}
	}
	,
	{//Bev3追加
		QCARCAM_INPUT_ID_CAMERA,
		{
			{
				0						// srcId
				, 640					// width
				, 480					// height
				, QCARCAM_FMT_UYVY_8	// colorFmt
				, 60.0f					// fps
				, 0						// securityDomain
			}	// sources
			, 1	// numSources
		}
	}
	,
	{
		QCARCAM_INPUT_ID_HDMI,
		{
			{
				0						// srcId
				, 1280					// width
				, 720					// height
				, QCARCAM_FMT_UYVY_8	// colorFmt
				, 60.0f					// fps
				, 0						// securityDomain
			}	// sources
			, 1	// numSources
		}
	}
	,
	{
		QCARCAM_INPUT_ID_HDMI,
		{
			{
				0						// srcId
				, 1920					// width
				, 1080					// height
				, QCARCAM_FMT_UYVY_8	// colorFmt
				, 60.0f					// fps
				, 0						// securityDomain
			}	// sources
			, 1	// numSources
		}
	}
	,
	{
		QCARCAM_INPUT_ID_HDMI,
		{
			{
				0						// srcId
				, 720					// width
				, 480					// height
				, QCARCAM_FMT_UYVY_8	// colorFmt
				, 60.0f					// fps
				, 0						// securityDomain
			}	// sources
			, 1	// numSources
		}
	}
	,
	{
		QCARCAM_INPUT_ID_HDMI,
		{
			{
				0						// srcId
				, 640					// width
				, 480					// height
				, QCARCAM_FMT_UYVY_8	// colorFmt
				, 60.0f					// fps
				, 0						// securityDomain
			}	// sources
			, 1	// numSources
		}
	}
};

// Handle List
static qcarcam_stub_hndl_info	gQcarCamStubHndlList[QCARCAM_STUB_HNDL_TYPE_MAX] =
{
	/*	CAMERA */
	{
		"INPUT_1_0",						// hndlName
		false,								// isOpen
		false,								// isStart
		QCARCAM_STUB_HNDL_TYPE_CAMERA,		// hndlType
		nullptr,							// eventCb
		nullptr,							// pPrivateData
		0,									// eventMask
		{QCARCAM_BUFFERLIST_ID_OUTPUT_START, QCARCAM_FMT_MAX, nullptr, 0, 0},	// buffers
		nullptr,							// buffersLocal
		0,									// buffersIdx
		nullptr,							// bufadr
	},
	/*	HDMI */
	{
		"INPUT_0_0",						// hndlName
		false,								// isOpen
		false,								// isStart
		QCARCAM_STUB_HNDL_TYPE_HDMI,		// hndlType
		nullptr,							// eventCb
		nullptr,							// pPrivateData
		0,									// eventMask
		{QCARCAM_BUFFERLIST_ID_OUTPUT_START, QCARCAM_FMT_MAX, nullptr, 0, 0},	// buffers
		nullptr,							// buffersLocal
		0,									// buffersIdx
		nullptr,							// bufadr
	}
};

static const	std::vector<const char*>	gpPathName_Camera{ CAMERA_PATH, CAMERA_PATH2, CAMERA_PATH3, CAMERA_PATH4, CAMERA_PATH5 };
static const	std::vector<const char*>	gpPathName_Hdmi{ HDMI_PATH, HDMI_PATH2, HDMI_PATH3, HDMI_PATH4, HDMI_PATH5 };
static const	std::vector<const char*>*	gpPathName[]{&gpPathName_Camera, &gpPathName_Hdmi};

// Callback Thread
static sif_thrmsgq_id_t gQCarCamStubCbThrMqId = nullptr;
static sif_thread_id_t gQCarCamStubCbThrId = nullptr;
static qcarcam_stub_timer_info gQCarCamStubTimerInf;

// スタブのlibqcxosal.soとリンクするためのデータ
extern	int32_t g_qcxosal_dat;

//////////////////////////////////////////////////////////////////////////////////
//	Function(ProtoType)
//////////////////////////////////////////////////////////////////////////////////
static QCarCamRet_e CreateQCarCamStubCbThread(void);
static QCarCamRet_e DestroyQCarCamStubCbThread(void);
static void* QCarCamStubCbThreadMain(void* arg);
static void QCarCamStubCyclicProc(void);
static QCarCamRet_e QCarCamStubSendMessage(qcarcam_stub_cmd_type command, qcarcam_stub_hndl_info* pStubHndl);
static void QCarCamStubTimerTimeOut(void* arg);
static QCarCamRet_e QCarCamStubCreateTimer(void);
static QCarCamRet_e QCarCamStubStartTimer(void);
static QCarCamRet_e QCarCamStubStopTimer(void);
static QCarCamRet_e QCarCamStubDeleteTimer(void);
static QCarCamRet_e QCarCamStubMakeReturnValueByFile(const char* p_path, QCarCamRet_e *pRetVal);

//Bev3追加 プロトタイプ宣言0
static int loadPng(const char *filename, struct PngInfo *pngInf);
static void SetBufadr(qcarcam_stub_hndl_type hType);
static void ClearBufadr(qcarcam_stub_hndl_info& hndl);

//////////////////////////////////////////////////////////////////////////////////
//	Function(QCarCam API Stub)
//////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// QCarCamInitialize
///
/// @brief Initializes the QCarCam interface. Must be the first call to the library.
///
/// @param[in] pInitParams   Structure required for initialization.
///
/// @return An appropriate error as defined in #QCarCamRet_e; more specifically,
/// #QCARCAM_RET_OK          Only upon success;
/// #QCARCAM_RET_BADPARAM    If invalid parameters are provided;
/// #QCARCAM_RET_UNSUPPORTED If QCarCam API version mismatch is detected;
/// #QCARCAM_RET_NOMEM       If not enough system memory available;
/// #QCARCAM_RET_NOT_FOUND   If communication to server cannot be established.
///
/// @note If communication fails to be established, this typically suggests server is either not
///       running or not fully initialized to accept clients.
///
/// @see QCarCamUninitialize()
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e QCarCamInitialize(const QCarCamInit_t* pInitParams)
{
	INT32 result = 0;
	QCarCamRet_e localRet = QCARCAM_RET_OK;

	PRINT_LOGD("%s() Start", __FUNCTION__);

	// libqcxosal.soを有効にするための記載
	if (0 != g_qcxosal_dat)
	{
		PRINT_LOGE("%s() Error", __FUNCTION__);
	}

	// Parameter Check
	if(nullptr == pInitParams)
	{
		// Error
		PRINT_LOGE("%s() Error", __FUNCTION__);
		return QCARCAM_RET_BADPARAM;
	}

	/////////////////////////////////////////////////////
	// Create Test Control File
	/////////////////////////////////////////////////////
	system("mkdir -p " STUB_PATH_FUNC);

	// for QCarCamInitialize()
	system("touch " STUB_FUNC(QCarCamInitialize));

	// for QCarCamUninitialize()
	system("touch " STUB_FUNC(QCarCamUninitialize));

	// for QCarCamQueryInputs()
	system("touch " STUB_FUNC(QCarCamQueryInputs));

	// for QCarCamSetParam()
	system("touch " STUB_FUNC(QCarCamSetParam));

	// for qcarcam QCarCamOpen()
	system("touch " STUB_FUNC(QCarCamOpen));

	// for qcarcam QCarCamClose()
	system("touch " STUB_FUNC(QCarCamClose));

	// for qcarcam QCarCamSetBuffers()
	system("touch " STUB_FUNC(QCarCamSetBuffers));

	// for qcarcam QCarCamStart()
	system("touch " STUB_FUNC(QCarCamStart));

	// for qcarcam QCarCamStop()
	system("touch " STUB_FUNC(QCarCamStop));

	// for qcarcam QCarCamGetFrame()
	system("touch " STUB_FUNC(QCarCamGetFrame));

	// for qcarcam QCarCamQueryInputModes()
	system("touch " STUB_FUNC(QCarCamQueryInputModes));

	// for qcarcam QCarCamReserve()
	system("touch " STUB_FUNC(QCarCamReserve));

	// for qcarcam QCarCamRelease()
	system("touch " STUB_FUNC(QCarCamRelease));

	// for qcarcam QCarCamRegisterEventCallback()
	system("touch " STUB_FUNC(QCarCamRegisterEventCallback));

	// for qcarcam QCarCamUnregisterEventCallback()
	system("touch " STUB_FUNC(QCarCamUnregisterEventCallback));

	// for qcarcam Event Callback()
	system("touch " QCARCAM_STUB_TEST_CTRL_FILE_EVT_CB);


	/////////////////////////////////////////////////////
	// Initialize Callback Thread Info
	/////////////////////////////////////////////////////
	gQCarCamStubCbThrMqId = nullptr;
	gQCarCamStubCbThrId = nullptr;
	memset(&gQCarCamStubTimerInf, 0x00, sizeof(gQCarCamStubTimerInf));
	gQCarCamStubTimerInf.isStarted = false;

	/////////////////////////////////////////////////////
	// Initialize SPF Timer
	/////////////////////////////////////////////////////
	result = spf_timer_init();
	if(0 != result)
	{
		// Error
		PRINT_LOGE("%s() spf_timer_init() Failed result[%d]", __FUNCTION__, result);
		return QCARCAM_RET_FAILED;
	}

	PRINT_LOGI("%s() spf_timer_init() Success ", __FUNCTION__);

	localRet = QCarCamStubCreateTimer();

	if(QCARCAM_RET_OK != localRet)
	{
		// Error
		PRINT_LOGE("%s() Error", __FUNCTION__);
		return QCARCAM_RET_FAILED;
	}

	PRINT_LOGI("%s() QCarCamStubCreateTimer() Success ", __FUNCTION__);

	/////////////////////////////////////////////////////
	// Return Value Test
	/////////////////////////////////////////////////////
	QCarCamRet_e ret = QCARCAM_RET_OK;
	localRet = QCarCamStubMakeReturnValueByFile(STUB_FUNC(QCarCamInitialize), &ret);

	if(QCARCAM_RET_OK == localRet)
	{
		if(QCARCAM_RET_OK != ret)
		{
			// Return Value Test
			PRINT_LOGI("%s() return ret[%d]", __FUNCTION__, ret);
			return ret;
		}
	}

	PRINT_LOGD("%s() End", __FUNCTION__);

	return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// QCarCamUninitialize
///
/// @brief Uninitializes the QCarCam interface. This is the last call to the library.
///
/// @pre QCarCamInitialize() was called.
///
/// @return An appropriate error as defined in #QCarCamRet_e; more specifically,
/// #QCARCAM_RET_OK          Only upon success;
/// #QCARCAM_RET_BADSTATE    If QCarCam interface is not initialized yet;
/// #QCARCAM_RET_TIMEOUT     If server does not respond within a certain time.
///
/// @see QCarCamInitialize()
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e QCarCamUninitialize(void)
{
	QCarCamRet_e ret = QCARCAM_RET_OK;
	QCarCamRet_e localRet = QCARCAM_RET_OK;

	PRINT_LOGD("%s() Start", __FUNCTION__);

	/////////////////////////////////////////////////////
	// Return Value Test
	/////////////////////////////////////////////////////
	localRet = QCarCamStubMakeReturnValueByFile(STUB_FUNC(QCarCamUninitialize), &ret);

	if(QCARCAM_RET_OK == localRet)
	{
		if(QCARCAM_RET_OK != ret)
		{
			// Return Value Test
			PRINT_LOGI("%s() return ret[%d]", __FUNCTION__, ret);
			return ret;
		}
	}
	else
	{
		// Error(Continue)
		PRINT_LOGE("%s() QCarCamStubMakeReturnValueByFile() Error(Continue)", __FUNCTION__);
	}

	/////////////////////////////////////////////////////
	// Close Handles
	/////////////////////////////////////////////////////
	for (int32_t htyp = QCARCAM_STUB_HNDL_TYPE_CAMERA; htyp < QCARCAM_STUB_HNDL_TYPE_MAX; ++htyp)
	{
		qcarcam_stub_hndl_info *pStubHndl = &gQcarCamStubHndlList[htyp];
		if (pStubHndl->isOpen)
		{
			(void)system("/usr/bin/echo -n QCARCAM_RET_OK > " STUB_FUNC(QCarCamClose));
			(void)QCarCamClose((QCarCamHndl_t)pStubHndl);
		}
	}

	/////////////////////////////////////////////////////
	// Delete SPF Timer
	/////////////////////////////////////////////////////
	localRet = QCarCamStubDeleteTimer();

	if(QCARCAM_RET_OK != localRet)
	{
		// Error
		PRINT_LOGE("%s() Error", __FUNCTION__);
		return QCARCAM_RET_FAILED;
	}

	PRINT_LOGI("%s() QCarCamStubDeleteTimer() Success ", __FUNCTION__);

	/////////////////////////////////////////////////////
	// Deinitialize SPF Timer
	/////////////////////////////////////////////////////
	spf_timer_deinit();

	PRINT_LOGI("%s() spf_timer_deinit() Success ", __FUNCTION__);

	PRINT_LOGD("%s() End", __FUNCTION__);

	return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// QCarCamQueryInputs
///
/// @brief Queries the available input information.
///
/// @pre QCarCamInitialize() was called.
///
/// @param[out] pInputs  Pointer to the array inputs. If NULL (along with size = 0), *pRetSize
///                      returns the number of available inputs.
/// @param[in]  size     Number of elements pointed to by pInputs.
/// @param[out] pRetSize If pInputs is set, it is the number of elements in an array that were
///                      filled.
///                      If pInputs is NULL, it is the number of available inputs to query.
///
/// @details
/// It is recommended to first query the number of inputs available, followed by their actual
/// information query. E.g.,:
/// @par
/// -# Upon successful return of QCarCamQueryInputs() (pInputs = NULL, size = 0, &retSize),
///    retSize shall contain the number of available inputs.
/// -# Call QCarCamQueryInputs() again with the pInput/size at least large enough to hold
///    the number of inputs returned in Step 1. Upon a successful return, pInput will be
///    filled up to the *pRetSize entries.
///
///
/// @return An appropriate error as defined in #QCarCamRet_e; more specifically,
/// #QCARCAM_RET_OK          Only upon success;
/// #QCARCAM_RET_BADPARAM    If invalid parameters are provided;
/// #QCARCAM_RET_BADSTATE    If QCarCam interface is not initialized yet;
/// #QCARCAM_RET_FAILED      If memory operations failed when populating input information.
/// #QCARCAM_RET_BUSY        If detection of all available inputs is not yet complete. Will only return available
///                          inputs up to this point in time.
/// @see QCarCamInitialize()
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e QCarCamQueryInputs(
    QCarCamInput_t *pInputs,
    const uint32_t size,
    uint32_t *pRetSize)
 {
	QCarCamRet_e ret = QCARCAM_RET_OK;
	QCarCamRet_e localRet = QCARCAM_RET_OK;
	
	
	PRINT_LOGD("%s() Start", __FUNCTION__);

	/////////////////////////////////////////////////////
	// Return Value Test
	/////////////////////////////////////////////////////
	localRet = QCarCamStubMakeReturnValueByFile(STUB_FUNC(QCarCamQueryInputs), &ret);

	if(QCARCAM_RET_OK == localRet)
	{
		if(QCARCAM_RET_OK != ret)
		{
			// Return Value Test
			PRINT_LOGI("%s() return ret[%d]", __FUNCTION__, ret);
			return ret;
		}
	}
	else
	{
		// Error(Continue)
		PRINT_LOGE("%s() QCarCamStubMakeReturnValueByFile() Error(Continue)", __FUNCTION__);
	}

	// Parameter Check
	if(nullptr == pRetSize)
	{
		// Error
		PRINT_LOGE("%s() Error", __FUNCTION__);
		return QCARCAM_RET_BADPARAM;
	}

	// If NULL, then ret_size returns number of available inputs to query
	if(nullptr == pInputs)
	{
		*pRetSize = QCARCAM_STUB_AVAILABLE_INPUT_NUM;

		PRINT_LOGD("%s() End", __FUNCTION__);

		return QCARCAM_RET_OK;
	}

	// If p_inputs is not NULL, size must be QCARCAM_STUB_AVAILABLE_INPUT_NUM
	if(QCARCAM_STUB_AVAILABLE_INPUT_NUM != size)
	{
		// Error
		PRINT_LOGE("%s() Error", __FUNCTION__);
		return QCARCAM_RET_BADPARAM;
	}

	// Output Fixed Value
	memcpy(pInputs, &dummyInputInf[0], sizeof(dummyInputInf));
	*pRetSize = QCARCAM_STUB_AVAILABLE_INPUT_NUM;

	PRINT_LOGD("%s() End", __FUNCTION__);

	return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// QCarCamQueryInputModes
///
/// @brief Queries available modes for the input represented by inputId.
///
/// @pre Successful QCarCamInitialize()
///
/// @param[in] inputId         Input identifier for which to enumerate the available modes.
///                            Available inputs can be queried using QCarCamQueryInputs().
/// @param[out] pInputModes    Pointer to structure that will hold the enumerated modes. Number of modes
///                            available can be queried using QCarCamQueryInputs().
///
/// @return An appropriate error as defined in #QCarCamRet_e; more specifically,
/// #QCARCAM_RET_OK          Only upon success;
/// #QCARCAM_RET_BADPARAM    If invalid parameters are provided;
///
/// @see QCarCamInitialize() and QCarCamQueryInputs()
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e QCarCamQueryInputModes(
    const uint32_t inputId,
    QCarCamInputModes_t* pInputModes)
{
	QCarCamRet_e ret = QCARCAM_RET_OK;
	QCarCamRet_e localRet = QCARCAM_RET_OK;

	PRINT_LOGD("%s() Start", __FUNCTION__);

	/////////////////////////////////////////////////////
	// Return Value Test
	/////////////////////////////////////////////////////
	localRet = QCarCamStubMakeReturnValueByFile(STUB_FUNC(QCarCamQueryInputModes), &ret);

	if(QCARCAM_RET_OK == localRet)
	{
		if(QCARCAM_RET_OK != ret)
		{
			// Return Value Test
			PRINT_LOGI("%s() return ret[%d]", __FUNCTION__, ret);
			return ret;
		}
	}
	else
	{
		// Error(Continue)
		PRINT_LOGE("%s() QCarCamStubMakeReturnValueByFile() Error(Continue)", __FUNCTION__);
	}

	// check parameters
	if (nullptr == pInputModes || 1 > pInputModes->numModes || nullptr == pInputModes->pModes)
	{
		PRINT_LOGE("%s() Error", __FUNCTION__);
		ret = QCARCAM_RET_BADPARAM;
	}
	else
	{
		uint32_t iCount = 0;
		for (QCarCamInputIdAndMode x : dummyQCarCamMode)
		{
			if (x.inputId == inputId)
			{
				// output fixed value
				iCount ++;
				if (iCount > pInputModes->numModes)
				{
					break;
				}
				else
				{
					memcpy(&pInputModes->pModes[iCount - 1], &x.qCarCamMode, sizeof(x.qCarCamMode));
				}
			}
		}
		if (iCount != pInputModes->numModes)
		{
			PRINT_LOGE("%s() Error", __FUNCTION__);
			ret = QCARCAM_RET_BADPARAM;
		}
		else
		{
			ret = QCARCAM_RET_OK;
		}
	}

	PRINT_LOGD("%s() End", __FUNCTION__);
	return ret;
}

///////////////////////////////////////////////////////////////////////////////
/// QCarCamOpen
///
/// @brief Creates a #QCarCamHndl_t session.
///
/// @pre QCarCamInitialize() was called.
///
/// @param[in]  pOpenParams  Open parameters for the session.
/// @param[out]  pHndl  Handle to opened session.
///
///
/// @return An appropriate error as defined in #QCarCamRet_e; more specifically,
/// #QCARCAM_RET_OK on success with #QCarCamHndl_t handle filled with valid handle.
/// #QCARCAM_RET_BADPARAM  If invalid open parameters are provided
/// #QCARCAM_RET_BADSTATE  If QCarCam interface is not initialized.
/// #QCARCAM_RET_FAILED    If failure due to other errors such as out of memory
///
/// @see QCarCamInitialize()
/// @see QCarCamQueryInputs()
/// @see QCarCamClose()
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e QCarCamOpen(const QCarCamOpen_t* pOpenParams, QCarCamHndl_t* pQCarCamHndl)
{
	QCarCamRet_e ret = QCARCAM_RET_OK;
	QCarCamRet_e localRet = QCARCAM_RET_OK;
	qcarcam_stub_hndl_info *pStubHndl = nullptr;

	PRINT_LOGD("%s() Start", __FUNCTION__);

	/////////////////////////////////////////////////////
	// Return Value Test
	/////////////////////////////////////////////////////
	localRet = QCarCamStubMakeReturnValueByFile(STUB_FUNC(QCarCamOpen), &ret);

	if(QCARCAM_RET_OK == localRet)
	{
		if(QCARCAM_RET_OK != ret)
		{
			// Return Value Test
			PRINT_LOGI("%s() return ret[%d]", __FUNCTION__, ret);
			return ret;
		}
	}
	else
	{
		// Error(Continue)
		PRINT_LOGE("%s() QCarCamStubMakeReturnValueByFile() Error(Continue)", __FUNCTION__);
	}

	// check parameters
	if (nullptr == pOpenParams || nullptr == pQCarCamHndl)
	{
		PRINT_LOGE("%s() Error", __FUNCTION__);
		return QCARCAM_RET_FAILED;
	}

	if(QCARCAM_INPUT_ID_CAMERA == pOpenParams->inputs[0].inputId)
	{
		// CAMERA
		PRINT_LOGI("%s() QCARCAM_INPUT_TYPE_EXT_REAR", __FUNCTION__);
		pStubHndl = &gQcarCamStubHndlList[QCARCAM_STUB_HNDL_TYPE_CAMERA];
		SetBufadr(QCARCAM_STUB_HNDL_TYPE_CAMERA);
	}
	else if(QCARCAM_INPUT_ID_HDMI == pOpenParams->inputs[0].inputId)
	{
		// HDMI
		PRINT_LOGI("%s() QCARCAM_INPUT_TYPE_DIGITAL_MEDIA", __FUNCTION__);
		pStubHndl = &gQcarCamStubHndlList[QCARCAM_STUB_HNDL_TYPE_HDMI];
		SetBufadr(QCARCAM_STUB_HNDL_TYPE_HDMI);
	}
	else
	{
		// Error
		PRINT_LOGE("%s() Error", __FUNCTION__);
		return QCARCAM_RET_FAILED;
	}

	// Already Opened
	if(true == pStubHndl->isOpen)
	{
		// Error
		PRINT_LOGE("%s() Already Open Error", __FUNCTION__);
		return QCARCAM_RET_FAILED;
	}

	// Make Callback Thread
	localRet = CreateQCarCamStubCbThread();

	if(QCARCAM_RET_OK != localRet)
	{
		// Error
		PRINT_LOGE("%s() Error", __FUNCTION__);
		return QCARCAM_RET_FAILED;
	}

	// Set Open State
	pStubHndl->isOpen = true;

	// Set return Value
	*pQCarCamHndl = (QCarCamHndl_t) pStubHndl;

	PRINT_LOGD("%s() End", __FUNCTION__);

	return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// QCarCamClose
///
/// @brief Closes the #QCarCamHndl_t session.
///
/// @pre Successful QCarCamOpen() call returning the same handle.
///
/// @param[in]  hndl    QCarCam handle type to close.
///
/// @details
/// The call releases resources associated with the handle returned by a successful QCarCamOpen()
/// call.
///
/// @return An appropriate error as defined in #QCarCamRet_e; more specifically,
/// #QCARCAM_RET_OK          Only upon success;
/// #QCARCAM_RET_BADPARAM    If invalid parameters are provided;
/// #QCARCAM_RET_BADSTATE    If session is not opened.
///
/// @see QCarCamOpen()
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e QCarCamClose(const QCarCamHndl_t hndl)
{
	QCarCamRet_e ret = QCARCAM_RET_OK;
	QCarCamRet_e localRet = QCARCAM_RET_OK;
	bool isOpen = false;
	UINT32 cnt = 0;

	PRINT_LOGD("%s() Start", __FUNCTION__);

	/////////////////////////////////////////////////////
	// Return Value Test
	/////////////////////////////////////////////////////
	localRet = QCarCamStubMakeReturnValueByFile(STUB_FUNC(QCarCamClose), &ret);

	if(QCARCAM_RET_OK == localRet)
	{
		if(QCARCAM_RET_OK != ret)
		{
			// Return Value Test
			PRINT_LOGI("%s() return ret[%d]", __FUNCTION__, ret);
			return ret;
		}
	}
	else
	{
		// Error(Continue)
		PRINT_LOGE("%s() QCarCamStubMakeReturnValueByFile() Error(Continue)", __FUNCTION__);
	}

	// Parameter Check
	if(QCARCAM_HNDL_INVALID == hndl)
	{
		// Error
		PRINT_LOGE("%s() Error", __FUNCTION__);
		return QCARCAM_RET_BADPARAM;
	}

	qcarcam_stub_hndl_info* pStubHndl = (qcarcam_stub_hndl_info*)hndl;
	if(false == pStubHndl->isOpen)
	{
		// Error
		PRINT_LOGE("%s() Already Close Error", __FUNCTION__);
		return QCARCAM_RET_FAILED;
	}

	// Bev3追加 生データ領域の解放
	ClearBufadr(*pStubHndl);

	// Clear Buffers
	free(pStubHndl->buffersLocal);
	pStubHndl->buffersLocal = nullptr;
	pStubHndl->buffers.pBuffers = nullptr;

	// Set Close State
	pStubHndl->isOpen = false;

	for(cnt = 0; cnt < QCARCAM_STUB_HNDL_TYPE_MAX; cnt++)
	{
		if(true == gQcarCamStubHndlList[cnt].isOpen)
		{
			PRINT_LOGI("%s() Still Open Hndl Exists", __FUNCTION__);
			isOpen = true;
			break;
		}
	}

	if(false == isOpen)
	{
		// Stop Callback Thread
		localRet = DestroyQCarCamStubCbThread();

		if(QCARCAM_RET_OK != localRet)
		{
			// Error
			PRINT_LOGE("%s() Error", __FUNCTION__);
			return QCARCAM_RET_FAILED;
		}
	}

	PRINT_LOGD("%s() End", __FUNCTION__);
	return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// QCarCamSetParam
///
/// @brief Sets the parameter value to a session represented by the #QCarCamHndl_t.
///
/// @pre Successful QCarCamOpen() call returning the same handle.
///
/// @param[in] hndl    session handle to which to apply the parameter.
/// @param[in] param   Specifies the parameter to set.
/// @param[in] pValue  Points to the data that will be set.
/// @param[in] size    Memory byte size pointed to by the *pValue; value no bigger than the size of
///                    the largest data structure documented in #QCarCamParamType_e.
///
/// @details
/// For the pValue and size parameters, check the table provided with #QCarCamParamType_e.
///
/// @return An appropriate error as defined in #QCarCamRet_e; more specifically,
/// #QCARCAM_RET_OK          Only upon success;
/// #QCARCAM_RET_BADPARAM    If invalid parameters are provided;
/// #QCARCAM_RET_BADSTATE    If QCarCam interface is not initialized yet;
/// #QCARCAM_RET_INVALID_OP  If set parameter request type is invalid.
/// #QCARCAM_RET_UNSUPPORTED If size of parameter exceeds maximum supported value;
/// #QCARCAM_RET_FAILED      If memory operations failed when reading parameter information.
///
/// @see QCarCamOpen()
/// @see QCarCamGetParam()
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e QCarCamSetParam(
    const QCarCamHndl_t hndl,
    const QCarCamParamType_e param,
    const void *pValue,
    const uint32_t size)
{
	QCarCamRet_e ret = QCARCAM_RET_OK;
	QCarCamRet_e localRet = QCARCAM_RET_OK;
 	qcarcam_stub_hndl_info *pHndlInfo = (qcarcam_stub_hndl_info*)hndl;

	PRINT_LOGD("%s() Start", __FUNCTION__);

	/////////////////////////////////////////////////////
	// Return Value Test
	/////////////////////////////////////////////////////
	localRet = QCarCamStubMakeReturnValueByFile(STUB_FUNC(QCarCamSetParam), &ret);

	if(QCARCAM_RET_OK == localRet)
	{
		if(QCARCAM_RET_OK != ret)
		{
			// Return Value Test
			PRINT_LOGI("%s() return ret[%d]", __FUNCTION__, ret);
			return ret;
		}
	}
	else
	{
		// Error(Continue)
		PRINT_LOGE("%s() QCarCamStubMakeReturnValueByFile() Error(Continue)", __FUNCTION__);
	}

	// Parameter Check
	if(QCARCAM_HNDL_INVALID == hndl)
	{
		// Error
		PRINT_LOGE("%s() nullptr == hndl Error", __FUNCTION__);
		return QCARCAM_RET_BADPARAM;
	}

	if(QCARCAM_PARAM_MAX <= param)
	{
		// Error
		PRINT_LOGE("%s() param[0x%X] Error", __FUNCTION__, param);
		return QCARCAM_RET_BADPARAM;
	}

	switch(param)
	{
 		case QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK:
 			PRINT_LOGI("%s() QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK", __FUNCTION__);
			if(nullptr == pValue || size != sizeof(pHndlInfo->eventMask))
			{
				PRINT_LOGE("%s() Error", __FUNCTION__);
				return QCARCAM_RET_BADPARAM;
			}
			else
				memcpy(&pHndlInfo->eventMask, pValue, size);
 			break;

		case QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE:
		case QCARCAM_VENDOR_PARAM:
			break;	/* 無処理 */

		default:
			// Not Support by Stub
			PRINT_LOGE("%s() param[0x%X] Error", __FUNCTION__, param);
			return QCARCAM_RET_BADPARAM;
	}

	PRINT_LOGD("%s() End", __FUNCTION__);

	return ret;
}

///////////////////////////////////////////////////////////////////////////////
/// QCarCamSetBuffers
///
/// @brief Registers buffers of particular bufferlist for the session.
///
/// @pre Successful QCarCamOpen() call returning the same handle.
///
/// @param[in] hndl         Handle of the session.
/// @param[in] pBuffers     Pointer to set the buffer structure.
///
/// @details
/// This API is used to map client buffers to a specific bufferlist. The available bufferlists are
/// dependent on the operation mode and usecase picked for the session.
/// If a session has more than one output buffer based on its operating mode, individual output
/// buffers will be required to be set for each of the session's selected mode to work.
/// @par
/// This requires multiple calls to QCarCamSetBuffers() per session.
/// @par
/// The maximum number of buffers per session per output mode is defined by #QCARCAM_MAX_NUM_BUFFERS.
/// For ASIL-B operations, the minimum number of buffers per session per output mode is defined
/// by #QCARCAM_MIN_NUM_BUFFERS.
///
/// @return An appropriate error as defined in #QCarCamRet_e; more specifically,
/// #QCARCAM_RET_OK          Only upon success;
/// #QCARCAM_RET_BADPARAM    If invalid parameters are provided;
/// #QCARCAM_RET_BADSTATE    If QCarCam interface is not initialized yet;
/// #QCARCAM_RET_NOMEM       If not enough system memory available;
/// #QCARCAM_RET_FAILED      If memory operations failed when reading buffer list provided.
///
/// @see QCarCamOpen()
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e QCarCamSetBuffers(const QCarCamHndl_t hndl, const QCarCamBufferList_t *pBuffers)
{
	QCarCamRet_e ret = QCARCAM_RET_OK;
	QCarCamRet_e localRet = QCARCAM_RET_OK;

	PRINT_LOGD("%s() Start", __FUNCTION__);

	/////////////////////////////////////////////////////
	// Return Value Test
	/////////////////////////////////////////////////////
	localRet = QCarCamStubMakeReturnValueByFile(STUB_FUNC(QCarCamSetBuffers), &ret);

	if(QCARCAM_RET_OK == localRet)
	{
		if(QCARCAM_RET_OK != ret)
		{
			// Return Value Test
			PRINT_LOGI("%s() return ret[%d]", __FUNCTION__, ret);
			return ret;
		}
	}
	else
	{
		// Error(Continue)
		PRINT_LOGE("%s() QCarCamStubMakeReturnValueByFile() Error(Continue)", __FUNCTION__);
	}

	// Parameter Check
	if((QCARCAM_HNDL_INVALID == hndl) || (nullptr == pBuffers) || (nullptr == pBuffers->pBuffers))
	{
		// Error
		PRINT_LOGE("%s() Error", __FUNCTION__);
		return QCARCAM_RET_BADPARAM;
	}

	// Set Buffer Address to Handle
	qcarcam_stub_hndl_info* pStubHndl = (qcarcam_stub_hndl_info*)hndl;
	pStubHndl->buffers = *pBuffers;
	pStubHndl->buffersLocal = malloc(pBuffers->nBuffers * sizeof(QCarCamBuffer_t));
	memcpy(pStubHndl->buffersLocal, pBuffers->pBuffers, pBuffers->nBuffers * sizeof(QCarCamBuffer_t));
	pStubHndl->buffers.pBuffers = (QCarCamBuffer_t*)(pStubHndl->buffersLocal);
	PRINT_LOGD("%s() End", __FUNCTION__);

	return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// QCarCamStart
///
/// @brief Starts streaming on the session represented by the #QCarCamHndl_t.
///
/// @pre Populate necessary buffers for the session using QCarCamSetBuffers().
///
/// @param[in]  hndl    Handle of the session.
///
/// @return An appropriate error as defined in #QCarCamRet_e; more specifically,
/// #QCARCAM_RET_OK          Only upon success;
/// #QCARCAM_RET_BADPARAM    If invalid parameters are provided;
/// #QCARCAM_RET_BADSTATE    If start was requested in the wrong state or QCarCam interface is not
///                          initialized yet.;
/// #QCARCAM_RET_NOMEM       If no buffers set for the specified session.
///
/// @see QCarCamSetBuffers()
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e QCarCamStart(const QCarCamHndl_t hndl)
{
	QCarCamRet_e ret = QCARCAM_RET_OK;
	QCarCamRet_e localRet = QCARCAM_RET_OK;
	qcarcam_stub_hndl_info* pStubHndl = nullptr;
	
	/////////////////////////////////////////////////////
	// Return Value Test
	/////////////////////////////////////////////////////
	localRet = QCarCamStubMakeReturnValueByFile(STUB_FUNC(QCarCamStart), &ret);

	if(QCARCAM_RET_OK == localRet)
	{
		if(QCARCAM_RET_OK != ret)
		{
			// Return Value Test
			PRINT_LOGI("%s() return ret[%d]", __FUNCTION__, ret);
			return ret;
		}
	}
	else
	{
		// Error(Continue)
		PRINT_LOGE("%s() QCarCamStubMakeReturnValueByFile() Error(Continue)", __FUNCTION__);
	}

	// Parameter Check
	if(QCARCAM_HNDL_INVALID == hndl)
	{
		// Error
		PRINT_LOGE("%s() Error", __FUNCTION__);
		return QCARCAM_RET_BADPARAM;
	}

	pStubHndl = (qcarcam_stub_hndl_info*)hndl;

	PRINT_LOGD("%s(%s) Start", __FUNCTION__, pStubHndl->hndlName.c_str());

	if(true == pStubHndl->isStart)
	{
		PRINT_LOGE("%s() hndlName[%s] is already started!", __FUNCTION__, pStubHndl->hndlName.c_str());
		return QCARCAM_RET_FAILED;
	}

	//Bev3追加
	//バッファ数繰り返す
	if (nullptr != pStubHndl->bufadr.get())
	{
		auto data = *pStubHndl->bufadr.get();
		constexpr std::pair<uint32_t, uint32_t> wh[]{{1920,1080}, {1280,720}};
		const std::string str_camera(QCARCAM_STUB_INPUT_NAME_CAMERA);
		const std::string str_hdmi(QCARCAM_STUB_INPUT_NAME_HDMI);
		qcarcam_stub_hndl_type	hType = 
				pStubHndl->hndlName == str_camera ? QCARCAM_STUB_HNDL_TYPE_CAMERA :
				pStubHndl->hndlName == str_hdmi ? QCARCAM_STUB_HNDL_TYPE_HDMI : QCARCAM_STUB_HNDL_TYPE_MAX;

		for (uint32_t i = 0; i < pStubHndl->buffers.nBuffers && hType != QCARCAM_STUB_HNDL_TYPE_MAX; i++)
		{
			const auto& plane = pStubHndl->buffers.pBuffers[i].planes[0];
			const int64_t memh = (int64_t)plane.memHndl;

			if ((plane.width == wh[hType].first) && (plane.height == wh[hType].second) && (0 < data.size())
				&& (!(memh >= (int64_t)-1 && memh <= (int64_t)USHRT_MAX)) )
			{
				memcpy(reinterpret_cast<void *>(plane.memHndl), data[i % data.size()], plane.stride * plane.height);
			}
		}
	}
	PRINT_LOGD("%s(%s) Send [QCARCAM_CMD_START]", __FUNCTION__, pStubHndl->hndlName.c_str());
	localRet = QCarCamStubSendMessage(QCARCAM_CMD_START, pStubHndl);
	
	if(QCARCAM_RET_OK != localRet)
	{
		// Error
		PRINT_LOGE("%s() Error", __FUNCTION__);
		return QCARCAM_RET_FAILED;
	}

	PRINT_LOGD("%s(%s) End", __FUNCTION__, pStubHndl->hndlName.c_str());
	
	return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// QCarCamStop
///
/// @brief Stops the streaming of the #QCarCamHndl_t session
///
/// @pre Successful QCarCamStart() call on the same handle.
///
/// @param[in]  hndl    Handle of the session.
///
/// @return An appropriate error as defined in #QCarCamRet_e; more specifically,
/// #QCARCAM_RET_OK          Only upon success;
/// #QCARCAM_RET_BADPARAM    If invalid parameters are provided;
/// #QCARCAM_RET_BADSTATE    If session is not started or paused or QCarCam interface is not
///                          initialized yet.
///
/// @see QCarCamStart()
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e QCarCamStop(const QCarCamHndl_t hndl)
{
	QCarCamRet_e ret = QCARCAM_RET_OK;
	QCarCamRet_e localRet = QCARCAM_RET_OK;
	qcarcam_stub_hndl_info* pStubHndl = nullptr;

	/////////////////////////////////////////////////////
	// Return Value Test
	/////////////////////////////////////////////////////
	localRet = QCarCamStubMakeReturnValueByFile(STUB_FUNC(QCarCamStop), &ret);

	if(QCARCAM_RET_OK == localRet)
	{
		if(QCARCAM_RET_OK != ret)
		{
			// Return Value Test
			PRINT_LOGI("%s() return ret[%d]", __FUNCTION__, ret);
			return ret;
		}
	}
	else
	{
		// Error(Continue)
		PRINT_LOGE("%s() QCarCamStubMakeReturnValueByFile() Error(Continue)", __FUNCTION__);
	}

	// Parameter Check
	if(QCARCAM_HNDL_INVALID == hndl)
	{
		// Error
		PRINT_LOGE("%s() Error", __FUNCTION__);
		return QCARCAM_RET_BADPARAM;
	}

	pStubHndl = (qcarcam_stub_hndl_info*)hndl;

	PRINT_LOGD("%s(%s) Start", __FUNCTION__, pStubHndl->hndlName.c_str());

	if(false == pStubHndl->isStart)
	{
		PRINT_LOGE("%s() hndlName[%s] is already stopped!", __FUNCTION__, pStubHndl->hndlName.c_str());
		return QCARCAM_RET_FAILED;
	}

	PRINT_LOGD("%s(%s) Send [QCARCAM_CMD_STOP]", __FUNCTION__, pStubHndl->hndlName.c_str());
	localRet = QCarCamStubSendMessage(QCARCAM_CMD_STOP, pStubHndl);

	if(QCARCAM_RET_OK != localRet)
	{
		// Error
		PRINT_LOGE("%s() Error", __FUNCTION__);
		return QCARCAM_RET_FAILED;
	}

	PRINT_LOGD("%s(%s) End", __FUNCTION__, pStubHndl->hndlName.c_str());

	return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// QCarCamGetFrame
///
/// @brief Gets the available frames for the session represented by the #QCarCamHndl_t.
///
/// @pre Successful QCarCamStart() call on the same handle.
///
/// @param[in]  hndl        Handle of the session.
/// @param[in,out] pFrameInfo  Pointer to the frame information that will be filled.
///                           The bufferlist id will indicate which bufferlist will be dequeued from.
/// @param[in]  timeout     Maximum wait time in nanoseconds for the frame to be available
///                         before timeout.
/// @param[in]  flags       Flags for getting the frame (reserved for future use).
///
/// @details
/// Input frames can only be fetched after a successful QCarCamStart() call - required after either
/// QCarCamOpen() or QCarCamStop() of the same session. Similarly, call to QCarCamResume() is required
/// to fetch frames if session was paused via QCarCamPause().
///
/// @return An appropriate error as defined in #QCarCamRet_e; more specifically,
/// #QCARCAM_RET_OK          Only upon success;
/// #QCARCAM_RET_BADPARAM    If invalid parameters are provided;
/// #QCARCAM_RET_BADSTATE    If session is not started or QCarCam interface is not initialized yet;
/// #QCARCAM_RET_TIMEOUT     If next frame not available within the specified timeout requested.
///
/// @see QCarCamStart()
/// @see QCarCamReleaseFrame()
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e QCarCamGetFrame(const QCarCamHndl_t hndl, QCarCamFrameInfo_t* pFrameInfo, const uint64_t timeout, const uint32_t flags)
{
	QCarCamRet_e ret = QCARCAM_RET_OK;
	QCarCamRet_e localRet = QCARCAM_RET_OK;
	qcarcam_stub_hndl_info* pStubHndl = nullptr;

	PRINT_LOGD("%s() Start", __FUNCTION__);

	/////////////////////////////////////////////////////
	// Return Value Test
	/////////////////////////////////////////////////////
	localRet = QCarCamStubMakeReturnValueByFile(STUB_FUNC(QCarCamGetFrame), &ret);

	if(QCARCAM_RET_OK == localRet)
	{
		if(QCARCAM_RET_OK != ret)
		{
			// Return Value Test
			PRINT_LOGI("%s() return ret[%d]", __FUNCTION__, ret);
			return ret;
		}
	}
	else
	{
		// Error(Continue)
		PRINT_LOGE("%s() QCarCamStubMakeReturnValueByFile() Error(Continue)", __FUNCTION__);
	}

	// Parameter Check
	if((QCARCAM_HNDL_INVALID == hndl) || (nullptr == pFrameInfo))
	{
		// Error
		PRINT_LOGE("%s() Error", __FUNCTION__);
		return QCARCAM_RET_BADPARAM;
	}

	// Get Handle
	pStubHndl = (qcarcam_stub_hndl_info*)hndl;

	// Set idx
	PRINT_LOGD("%s() Set p_frame_info->idx[%u] hndlName[%s]", __FUNCTION__, pStubHndl->buffersIdx, pStubHndl->hndlName.c_str());
	pFrameInfo->bufferIndex = pStubHndl->buffersIdx;

	// Increment idx
	pStubHndl->buffersIdx++;
	
	// Max Check
	if((pStubHndl->buffers.nBuffers-1) < pStubHndl->buffersIdx)
	{
		// Reset idx
		PRINT_LOGD("%s() Reset buffersIdx hndlName[%s]", __FUNCTION__, pStubHndl->hndlName.c_str());
		pStubHndl->buffersIdx = 0;
	}

 	PRINT_LOGD("%s() End", __FUNCTION__);

	return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// QCarCamReleaseFrame
///
/// @brief Signals that the released bufferIndex can be reused for any future frames for session
///        represented by the #QCarCamHndl_t.
///
/// @pre Successful QCarCamGetFrame() call on the same handle.
///
/// @param[in] hndl         Handle of the session.
/// @param[in] id           Buffer list ID associated with the session handle obtained from
///                         QCarCamGetFrame() via the #QCarCamFrameInfo_t structure.
/// @param[in] bufferIndex  Index into the #QCarCamBufferList_t buffers list to re-enqueue buffer
///                         for the specified buffer list ID.
///
/// @details
/// Upon a successful return of QCarCamGetFrame(), and the buffer identified by <id, bufferIndex>
/// has been processed, the buffer should be released back to the QCarcam library using
/// QCarCamReleaseFrame() as soon as possible so that new input frames can be fetched.
///
/// @return An appropriate error as defined in #QCarCamRet_e; more specifically,
/// #QCARCAM_RET_OK          Only upon success;
/// #QCARCAM_RET_BADPARAM    If invalid parameters are provided;
/// #QCARCAM_RET_BADSTATE    If requested buffer is not expected to be released yet.
///
/// @see QCarCamGetFrame()
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e QCarCamReleaseFrame(const QCarCamHndl_t hndl, const uint32_t id, const uint32_t bufferIndex)
{
	PRINT_LOGD("%s() Start", __FUNCTION__);
	PRINT_LOGD("%s() End", __FUNCTION__);
	return QCARCAM_RET_OK;
}


///////////////////////////////////////////////////////////////////////////////
/// QCarCamRegisterEventCallback
///
/// @brief Register a QCarCam event callback for system wide or per session events.
///
/// @pre Successful QCarCamOpen() call returning the same handle.
///
/// @param[in]  hndl            Handle of the session. Set to NULL if registering a system wide event callback.
/// @param[in]  callbackFunc    Callback function provided by the client.
/// @param[in]  pPrivateData    Pointer to the private data provided by the client. Will be returned
///                             with each system or session event.
///
/// @details
/// The QCarCam API supports one system wide event callback to be registered after initializing the
/// QCarCam API using QCarCamInitialize(). The QCarCam API uninitializing using
/// QCarCamUninitialize() will automatically unregister all event callbacks registered by the client.
/// 
/// @par
/// The QCarCam API supports one session event callback to be registered per session after a session has
/// been opened successfully using QCarCamOpen().
///
/// @return An appropriate error as defined in #QCarCamRet_e; more specifically,
/// #QCARCAM_RET_OK          Only upon success;
/// #QCARCAM_RET_BADPARAM    If invalid parameters are provided;
/// #QCARCAM_RET_BADSTATE    If QCarCam interface is not initialized yet;
///
/// @see QCarCamUnregisterEventCallback()
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e QCarCamRegisterEventCallback(
    const QCarCamHndl_t hndl,
    const QCarCamEventCallback_t callbackFunc,
    void  *pPrivateData)
{
	QCarCamRet_e ret = QCARCAM_RET_OK;
	QCarCamRet_e localRet = QCARCAM_RET_OK;

	PRINT_LOGD("%s() Start", __FUNCTION__);

	/////////////////////////////////////////////////////
	// Return Value Test
	/////////////////////////////////////////////////////
	localRet = QCarCamStubMakeReturnValueByFile(STUB_FUNC(QCarCamRegisterEventCallback), &ret);

	if(QCARCAM_RET_OK == localRet)
	{
		if(QCARCAM_RET_OK != ret)
		{
			// Return Value Test
			PRINT_LOGI("%s() return ret[%d]", __FUNCTION__, ret);
			return ret;
		}
	}
	else
	{
		// Error(Continue)
		PRINT_LOGE("%s() QCarCamStubMakeReturnValueByFile() Error(Continue)", __FUNCTION__);
	}

	// check parameters
	if (QCARCAM_HNDL_INVALID == hndl)
	{
		// Error
		PRINT_LOGE("%s() nullptr == hndl Error", __FUNCTION__);
		return QCARCAM_RET_BADPARAM;
	}

	((qcarcam_stub_hndl_info*)hndl)->eventCb = callbackFunc;
	((qcarcam_stub_hndl_info*)hndl)->pPrivateData = pPrivateData;

	PRINT_LOGD("%s() End", __FUNCTION__);
	return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// QCarCamUnregisterEventCallback
///
/// @brief Unregister a QCarCam event callback for system wide or per session events.
///
/// @pre Successful QCarCamRegisterEventCallback() call on the same handle.
///
/// @param[in]  hndl    Handle of the session. Set to NULL if unregistering a system wide event
///                     callback.
///
/// @details
/// A system wide event callback can be unregistered only after all sessions have been closed using
/// QCarCamClose().
/// @par
/// A session event callback can be unregistered only after that session has been closed using
/// QCarCamClose().
///
/// @return An appropriate error as defined in #QCarCamRet_e; more specifically,
/// #QCARCAM_RET_OK          Only upon success;
/// #QCARCAM_RET_BADPARAM    If invalid parameters are provided;
/// #QCARCAM_RET_BADSTATE    If QCarCam interface is not initialized yet.
///
/// @see QCarCamRegisterEventCallback()
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e QCarCamUnregisterEventCallback(const QCarCamHndl_t hndl)
{
	QCarCamRet_e ret = QCARCAM_RET_OK;
	QCarCamRet_e localRet = QCARCAM_RET_OK;

	PRINT_LOGD("%s() Start", __FUNCTION__);

	/////////////////////////////////////////////////////
	// Return Value Test
	/////////////////////////////////////////////////////
	localRet = QCarCamStubMakeReturnValueByFile(STUB_FUNC(QCarCamUnregisterEventCallback), &ret);

	if(QCARCAM_RET_OK == localRet)
	{
		if(QCARCAM_RET_OK != ret)
		{
			// Return Value Test
			PRINT_LOGI("%s() return ret[%d]", __FUNCTION__, ret);
			return ret;
		}
	}
	else
	{
		// Error(Continue)
		PRINT_LOGE("%s() QCarCamStubMakeReturnValueByFile() Error(Continue)", __FUNCTION__);
	}

	// check parameters
	if (QCARCAM_HNDL_INVALID == hndl)
	{
		// Error
		PRINT_LOGE("%s() nullptr == hndl Error", __FUNCTION__);
		return QCARCAM_RET_BADPARAM;
	}

	((qcarcam_stub_hndl_info*)hndl)->eventCb = nullptr;
	((qcarcam_stub_hndl_info*)hndl)->pPrivateData = nullptr;

	PRINT_LOGD("%s() End", __FUNCTION__);
	return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// QCarCamReserve
///
/// @brief Reserves resources required for the usecase for the session represented by the #QCarCamHndl_t.
///
/// @pre Populate necessary buffers for the session using QCarCamSetBuffers() and populated all other parameters
///      required to define the usecase.
///
/// @param[in] hndl         Handle of the session.
///
/// @return An appropriate error as defined in #QCarCamRet_e; more specifically,
/// #QCARCAM_RET_OK          Only upon success;
/// #QCARCAM_RET_BADPARAM    If invalid parameters are provided;
/// #QCARCAM_RET_BADSTATE    If in invalid state when calling this API
///
/// @see QCarCamRelease()
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e QCarCamReserve(const QCarCamHndl_t hndl)
{
	QCarCamRet_e ret = QCARCAM_RET_OK;
	QCarCamRet_e localRet = QCARCAM_RET_OK;

	PRINT_LOGD("%s() Start", __FUNCTION__);

	/////////////////////////////////////////////////////
	// Return Value Test
	/////////////////////////////////////////////////////
	localRet = QCarCamStubMakeReturnValueByFile(STUB_FUNC(QCarCamReserve), &ret);

	if(QCARCAM_RET_OK == localRet)
	{
		if(QCARCAM_RET_OK != ret)
		{
			// Return Value Test
			PRINT_LOGI("%s() return ret[%d]", __FUNCTION__, ret);
			return ret;
		}
	}
	else
	{
		// Error(Continue)
		PRINT_LOGE("%s() QCarCamStubMakeReturnValueByFile() Error(Continue)", __FUNCTION__);
	}

	// check parameters
	if (QCARCAM_HNDL_INVALID == hndl)
	{
		// Error
		PRINT_LOGE("%s() nullptr == hndl Error", __FUNCTION__);
		return QCARCAM_RET_BADPARAM;
	}

	PRINT_LOGD("%s() End", __FUNCTION__);
	return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// QCarCamRelease
///
/// @brief Releases resources previously reserved for the session represented by the #QCarCamHndl_t.
///
/// @pre Successful QCarCamReserve() on the same handle.
///
/// @param[in] hndl         Handle of the session.
///
/// @return An appropriate error as defined in #QCarCamRet_e; more specifically,
/// #QCARCAM_RET_OK          Only upon success;
/// #QCARCAM_RET_BADPARAM    If invalid parameters are provided;
/// #QCARCAM_RET_BADSTATE    If requested buffer is not expected to be released yet.
///
/// @see QCarCamReserve()
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e QCarCamRelease(const QCarCamHndl_t hndl)
{
	QCarCamRet_e ret = QCARCAM_RET_OK;
	QCarCamRet_e localRet = QCARCAM_RET_OK;

	PRINT_LOGD("%s() Start", __FUNCTION__);

	/////////////////////////////////////////////////////
	// Return Value Test
	/////////////////////////////////////////////////////
	localRet = QCarCamStubMakeReturnValueByFile(STUB_FUNC(QCarCamRelease), &ret);

	if(QCARCAM_RET_OK == localRet)
	{
		if(QCARCAM_RET_OK != ret)
		{
			// Return Value Test
			PRINT_LOGI("%s() return ret[%d]", __FUNCTION__, ret);
			return ret;
		}
	}
	else
	{
		// Error(Continue)
		PRINT_LOGE("%s() QCarCamStubMakeReturnValueByFile() Error(Continue)", __FUNCTION__);
	}

	// check parameters
	if (QCARCAM_HNDL_INVALID == hndl)
	{
		// Error
		PRINT_LOGE("%s() nullptr == hndl Error", __FUNCTION__);
		return QCARCAM_RET_BADPARAM;
	}

	PRINT_LOGD("%s() End", __FUNCTION__);
	return QCARCAM_RET_OK;
}

//////////////////////////////////////////////////////////////////////////////////
//	NOP functions
//////////////////////////////////////////////////////////////////////////////////
QCarCamRet_e QCarCamGetParam(const QCarCamHndl_t hndl, const QCarCamParamType_e param, void *pValue, const uint32_t size)
{
	// VideoHAL will not call this function
	return QCARCAM_RET_OK;
}

QCarCamRet_e QCarCamGetParamEx(const QCarCamHndl_t hndl, const QCarCamGetParamEx_t *pParam)
{
	// VideoHAL will not call this function
	return QCARCAM_RET_OK;
}

QCarCamRet_e QCarCamSetParamEx(const QCarCamHndl_t hndl, const QCarCamSetParamEx_t *pParam)
{
	// VideoHAL will not call this function
	return QCARCAM_RET_OK;
}

QCarCamRet_e QCarCamPause(const QCarCamHndl_t hndl)
{
	// VideoHAL will not call this function
	return QCARCAM_RET_OK;
}

QCarCamRet_e QCarCamResume(const QCarCamHndl_t hndl)
{
	// VideoHAL will not call this function
	return QCARCAM_RET_OK;
}

QCarCamRet_e QCarCamSubmitRequest(const QCarCamHndl_t hndl, const QCarCamRequest_t* pRequest)
{
	// VideoHAL will not call this function
	return QCARCAM_RET_OK;
}

QCarCamRet_e QCarCamMetadataGetVendorOps(vendor_tag_ops_t* pVendorTagOps)
{
	// VideoHAL will not call this function
	return QCARCAM_RET_OK;
}

QCarCamRet_e QCarCamGetMetaDataTagId(QCarCamMetadataTagId_e qccId, uint32_t *pTagId)
{
	// VideoHAL will not call this function
	return QCARCAM_RET_OK;
}

QCarCamRet_e QCarCamQueryDiagnostics(const QCarCamDiagInfo_t* pDiag)
{
	// VideoHAL will not call this function
	return QCARCAM_RET_OK;
}

//////////////////////////////////////////////////////////////////////////////////
//	Make Callback Thread
//////////////////////////////////////////////////////////////////////////////////
static QCarCamRet_e CreateQCarCamStubCbThread(void)
{
	INT32 sifRet = 0;
	sif_thrmsgq_attr_t	msgqAttr;
	sif_thread_attr_t	thrAttr;

	PRINT_LOGD("%s() Start", __FUNCTION__);

	memset(&msgqAttr, 0, sizeof(msgqAttr));
	memset(&thrAttr, 0, sizeof(thrAttr));

	// Create Message Que for Callback Thread
	if(nullptr == gQCarCamStubCbThrMqId)
	{
		msgqAttr.maxmsg			= QCARCAM_STUB_CB_THR_MQ_NUM;
		msgqAttr.msgsize		= QCARCAM_STUB_CB_THR_MQ_SIZE;
		msgqAttr.snd_timeout	= UINT32_MAX;	// Never Timeout
		msgqAttr.rcv_timeout	= UINT32_MAX;	// Never Timeout
		
		gQCarCamStubCbThrMqId = sif_thrmsg_open(QCARCAM_STUB_CB_THR_MQ_NAME, &msgqAttr);
		if(nullptr == gQCarCamStubCbThrMqId)
		{
			// Error
			PRINT_LOGE("%s() sif_thrmsg_open() Failed!", __FUNCTION__);
			return QCARCAM_RET_FAILED;
		}

		PRINT_LOGI("%s() sif_thrmsg_open(%s) Success", __FUNCTION__, QCARCAM_STUB_CB_THR_MQ_NAME);
	}
	else
	{
		PRINT_LOGI("%s() Already sif_thrmsg_open() Opend", __FUNCTION__);
	}

	// Create Callback Thread
	if(nullptr == gQCarCamStubCbThrId)
	{
		SIF_THREAD_PRIOATTR_NORMAL(&thrAttr);
		thrAttr.name		= QCARCAM_STUB_CB_THR_NAME;
		thrAttr.group		= NULL;
		thrAttr.cancelmode	= SIF_CANCEL_ENABLE;	// Cancel Enable
		thrAttr.detachstate	= 0;					// Enable join()

		sifRet = sif_thread_create(&gQCarCamStubCbThrId, &thrAttr, QCarCamStubCbThreadMain, NULL);
		if(0 != sifRet)
		{
			// Error
			PRINT_LOGE("%s() sif_thread_create() Failed!", __FUNCTION__);
			return QCARCAM_RET_FAILED;
		}
		if(nullptr == gQCarCamStubCbThrId)
		{
			// Error
			PRINT_LOGE("%s() sif_thread_create() Failed!", __FUNCTION__);
			return QCARCAM_RET_FAILED;
		}

		// Start Callback Thread
		sifRet = sif_thread_run(gQCarCamStubCbThrId);
		if(0 != sifRet)
		{
			// Error
			PRINT_LOGE("%s() sif_thread_run() Failed!", __FUNCTION__);
			return QCARCAM_RET_FAILED;
		}

		PRINT_LOGI("%s() sif_thread_create(QCarCamStub_cb)/sif_thread_run()  Success", __FUNCTION__);
	}
	else
	{
		PRINT_LOGI("%s() Already sif_thread_create() Created", __FUNCTION__);
	}

	PRINT_LOGD("%s() End", __FUNCTION__);

	return QCARCAM_RET_OK;
}

//////////////////////////////////////////////////////////////////////////////////
//	Destroy Callback Thread
//////////////////////////////////////////////////////////////////////////////////
static QCarCamRet_e DestroyQCarCamStubCbThread(void)
{
	QCarCamRet_e localRet = QCARCAM_RET_OK;

	PRINT_LOGD("%s() Start", __FUNCTION__);

	// Stop Callback Thread
	if(nullptr != gQCarCamStubCbThrId)
	{
		// Send Message to Callback Thread
		PRINT_LOGD("%s() Send [QCARCAM_CMD_EXIT]", __FUNCTION__);
		localRet = QCarCamStubSendMessage(QCARCAM_CMD_EXIT, nullptr);

		if(QCARCAM_RET_OK != localRet)
		{
			// Error
			PRINT_LOGE("%s() QCarCamStubSendMessage() Failed!", __FUNCTION__);
			return QCARCAM_RET_FAILED;
		}

		sif_thread_join(gQCarCamStubCbThrId, nullptr);
		PRINT_LOGI("%s() sif_thread_join(QCarCamStub_cb) Success", __FUNCTION__);

		gQCarCamStubCbThrId = nullptr;
	}
	
	// Close Message Que for Callback Thread
	if(nullptr != gQCarCamStubCbThrMqId)
	{
		sif_thrmsg_close(gQCarCamStubCbThrMqId);

		PRINT_LOGI("%s() sif_thrmsg_close(%s) Success", __FUNCTION__, QCARCAM_STUB_CB_THR_MQ_NAME);

		gQCarCamStubCbThrMqId = nullptr;
	}

	PRINT_LOGD("%s() End", __FUNCTION__);

	return QCARCAM_RET_OK;
}

//////////////////////////////////////////////////////////////////////////////////
//	Callback Thread Main
//////////////////////////////////////////////////////////////////////////////////
static void* QCarCamStubCbThreadMain(void* arg)
{
	INT32					ret = 0;
	QCarCamRet_e localRet = QCARCAM_RET_OK;
	bool 					exitFlg = false;
	qcarcam_stub_thr_msg	rcvMsg;
	UINT32					cnt = 0;
	bool 					doStop = false;

	PRINT_LOGD("%s() Start", __FUNCTION__);

	// Main Loop
	while(false == exitFlg)
	{
		ret = sif_thrmsg_recv(gQCarCamStubCbThrMqId, &rcvMsg, (INT32)sizeof(rcvMsg));

		if(0 == ret)
		{
			switch(rcvMsg.command)
			{
				case QCARCAM_CMD_START:
					PRINT_LOGI("%s() Receive [QCARCAM_CMD_START]", __FUNCTION__);

					// Timer Start
					localRet = QCarCamStubStartTimer();
					if(QCARCAM_RET_OK != localRet)
					{
						PRINT_LOGE("%s() QCarCamStubStartTimer() Fail! localRet[%d]", __FUNCTION__, localRet);
					}

					rcvMsg.pStubHndl->isStart = true;

					break;
				case QCARCAM_CMD_STOP:
					PRINT_LOGI("%s() Receive [QCARCAM_CMD_STOP]", __FUNCTION__);

					rcvMsg.pStubHndl->isStart = false;

					doStop = true;
					for(cnt = 0; cnt < QCARCAM_STUB_HNDL_TYPE_MAX; cnt++)
					{
						if(true == gQcarCamStubHndlList[cnt].isStart)
						{
							PRINT_LOGI("%s() Still Start Hndl Exists", __FUNCTION__);
							doStop = false;
							break;
						}
					}

					if(true == doStop)
					{
						PRINT_LOGI("%s() All Hndl Stopped", __FUNCTION__);

						// Timer Stop
						localRet = QCarCamStubStopTimer();
						if(QCARCAM_RET_OK != localRet)
						{
							PRINT_LOGE("%s() QCarCamStubStopTimer() Fail! localRet[%d]", __FUNCTION__, localRet);
						}
					}

					break;
				case QCARCAM_CMD_TIMER_CB:
					PRINT_LOGI("%s() Receive [QCARCAM_CMD_TIMER_CB]", __FUNCTION__);
					QCarCamStubCyclicProc();
					break;
				case QCARCAM_CMD_EXIT:
					PRINT_LOGI("%s() Receive [QCARCAM_CMD_EXIT]", __FUNCTION__);
					exitFlg = true;
					break;
				default:
					PRINT_LOGE("%s() Receive Unknown Command[%d]", __FUNCTION__, rcvMsg.command);
					break;
			}
		}
		else
		{
			PRINT_LOGE("%s() sif_thrmsg_recv)( Fail! ret[%d]", __FUNCTION__, ret);
		}
	}

	PRINT_LOGD("%s() End", __FUNCTION__);

	sif_thread_exit(nullptr);

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////
//	Callback Thread Cyclic Procedure
//////////////////////////////////////////////////////////////////////////////////
static void QCarCamStubCyclicProc(void)
{
	int readRet = 0;
	char testCommand[QCARCAM_STUB_TEST_CTRL_FILE_SIZE];
	uint32_t event_id = QCARCAM_EVENT_FRAME_READY;

//	PRINT_LOGD("%s() Start", __FUNCTION__);

	memset(testCommand, 0x00, sizeof(testCommand));

	// Read File
	readRet = StubClientRead_str(QCARCAM_STUB_TEST_CTRL_FILE_EVT_CB, testCommand);
	if(0 > readRet)
	{
		PRINT_LOGE("%s() StubClientRead_str() Error readRet[%d]", __FUNCTION__, readRet);
		return;
	}

	// Set Event ID
	if(0 == testCommand[0])
	{
		PRINT_LOGD("%s() testCommand is empty, use QCARCAM_EVENT_FRAME_READY", __FUNCTION__);
		event_id = QCARCAM_EVENT_FRAME_READY;
	}
	else if(0 == strncmp("QCARCAM_EVENT_FRAME_READY", testCommand, QCARCAM_STUB_TEST_CTRL_FILE_SIZE))
	{
		event_id = QCARCAM_EVENT_FRAME_READY;
	}
	else if(0 == strncmp("QCARCAM_EVENT_INPUT_SIGNAL", testCommand, QCARCAM_STUB_TEST_CTRL_FILE_SIZE))
	{
		event_id = QCARCAM_EVENT_INPUT_SIGNAL;
	}
	else if(0 == strncmp("QCARCAM_EVENT_ERROR", testCommand, QCARCAM_STUB_TEST_CTRL_FILE_SIZE))
	{
		event_id = QCARCAM_EVENT_ERROR;
	}
	else if(0 == strncmp("QCARCAM_EVENT_VENDOR", testCommand, QCARCAM_STUB_TEST_CTRL_FILE_SIZE))
	{
		event_id = QCARCAM_EVENT_VENDOR;
	}
	else if(0 == strncmp("QCARCAM_EVENT_PROPERTY_NOTIFY", testCommand, QCARCAM_STUB_TEST_CTRL_FILE_SIZE))
	{
		event_id = QCARCAM_EVENT_PROPERTY_NOTIFY;
	}
	else if(0 == strncmp("QCARCAM_EVENT_FRAME_SOF", testCommand, QCARCAM_STUB_TEST_CTRL_FILE_SIZE))
	{
		event_id = QCARCAM_EVENT_FRAME_SOF;
	}
	else if(0 == strncmp("QCARCAM_EVENT_RECOVERY", testCommand, QCARCAM_STUB_TEST_CTRL_FILE_SIZE))
	{
		event_id = QCARCAM_EVENT_RECOVERY;
	}
	// else if(0 == strncmp("QCARCAM_EVENT_RECOVERY_SUCCESS", testCommand, QCARCAM_STUB_TEST_CTRL_FILE_SIZE))
	// {
	// 	event_id = QCARCAM_EVENT_RECOVERY_SUCCESS;
	// }
	// else if(0 == strncmp("QCARCAM_EVENT_ERROR_ABORTED", testCommand, QCARCAM_STUB_TEST_CTRL_FILE_SIZE))
	// {
	// 	event_id = QCARCAM_EVENT_ERROR_ABORTED;
	// }
	// else if(0 == strncmp("QCARCAM_EVENT_FRAME_FREEZE", testCommand, QCARCAM_STUB_TEST_CTRL_FILE_SIZE))
	// {
	// 	event_id = QCARCAM_EVENT_FRAME_FREEZE;
	// }
	// else if(0 == strncmp("QCARCAM_EVENT_FRAME_DROP", testCommand, QCARCAM_STUB_TEST_CTRL_FILE_SIZE))
	// {
	// 	event_id = QCARCAM_EVENT_FRAME_DROP;
	// }
	else
	{
		PRINT_LOGE("%s() [%s] is unknown command, use QCARCAM_EVENT_FRAME_READY", __FUNCTION__, testCommand);
		event_id = QCARCAM_EVENT_FRAME_READY;
	}

	// Callback Call

	if((true == gQcarCamStubHndlList[QCARCAM_STUB_HNDL_TYPE_CAMERA].isStart) &&
		(nullptr != gQcarCamStubHndlList[QCARCAM_STUB_HNDL_TYPE_CAMERA].eventCb))
	{
		PRINT_LOGI("%s() Camera Callback Call testCommand[%s]", __FUNCTION__, testCommand);
		gQcarCamStubHndlList[QCARCAM_STUB_HNDL_TYPE_CAMERA].eventCb(
			(QCarCamHndl_t) &gQcarCamStubHndlList[QCARCAM_STUB_HNDL_TYPE_CAMERA],
			event_id,
			nullptr,
			nullptr);
	}

	if((true == gQcarCamStubHndlList[QCARCAM_STUB_HNDL_TYPE_HDMI].isStart) &&
		(nullptr != gQcarCamStubHndlList[QCARCAM_STUB_HNDL_TYPE_HDMI].eventCb))
	{
		PRINT_LOGI("%s() HDMI Callback Call testCommand[%s]", __FUNCTION__, testCommand);
		gQcarCamStubHndlList[QCARCAM_STUB_HNDL_TYPE_HDMI].eventCb(
			(QCarCamHndl_t) &gQcarCamStubHndlList[QCARCAM_STUB_HNDL_TYPE_HDMI],
			event_id,
			nullptr,
			nullptr);
	}

//	PRINT_LOGD("%s() End", __FUNCTION__);

	return;
}

//////////////////////////////////////////////////////////////////////////////////
//	Send Message to Callback Thread
//////////////////////////////////////////////////////////////////////////////////
static QCarCamRet_e QCarCamStubSendMessage(qcarcam_stub_cmd_type command, qcarcam_stub_hndl_info* pStubHndl)
{
	INT32 sifRet = 0;
	qcarcam_stub_thr_msg sendMsg;

//	PRINT_LOGD("%s() Start", __FUNCTION__);

	// Make Message
	memset(&sendMsg, 0x00, sizeof(sendMsg));
	sendMsg.command	= command;
	sendMsg.pStubHndl = pStubHndl;

	// Send Message
	sifRet = sif_thrmsg_send(gQCarCamStubCbThrMqId, &sendMsg, sizeof(sendMsg));

	if(0 != sifRet)
	{
		// Error
		return QCARCAM_RET_FAILED;
	}

//	PRINT_LOGD("%s() End", __FUNCTION__);

	return QCARCAM_RET_OK;
}

//////////////////////////////////////////////////////////////////////////////////
//	Timer Callback Function
//////////////////////////////////////////////////////////////////////////////////
static void QCarCamStubTimerTimeOut(void* arg)
{
//	PRINT_LOGD("%s() Start", __FUNCTION__);

	QCarCamStubSendMessage(QCARCAM_CMD_TIMER_CB, nullptr);

//	PRINT_LOGD("%s() End", __FUNCTION__);

	return;
}

//////////////////////////////////////////////////////////////////////////////////
// Create Cyclic Timer
//////////////////////////////////////////////////////////////////////////////////
static QCarCamRet_e QCarCamStubCreateTimer(void)
{
	spf_timer_attr attr;
	INT32 result = 0;
	
	PRINT_LOGD("%s() Start", __FUNCTION__);

	memset(&attr, 0, sizeof(attr));

	attr.name = nullptr;
	attr.wake_cnt = 0; 							/* 0:cyclic timer, 1:one shot timer */
	attr.cycle = QCARCAM_STUB_CB_CYCLIC_TIME;	/* Timer value */
	attr.callback = QCarCamStubTimerTimeOut;
	attr.arg = nullptr;

	// Timer Create
	result = spf_timer_create(&gQCarCamStubTimerInf.timerObject, &attr);
	if(0 != result)
	{
		// Error
		PRINT_LOGE("%s() spf_timer_create() Failed result[%d]", __FUNCTION__, result);
		return QCARCAM_RET_FAILED;
	}
	
	PRINT_LOGD("%s() End", __FUNCTION__);
	
	return QCARCAM_RET_OK;
}

//////////////////////////////////////////////////////////////////////////////////
// Start Cyclic Timer
//////////////////////////////////////////////////////////////////////////////////
static QCarCamRet_e QCarCamStubStartTimer(void)
{
	INT32 result = 0;
	
	PRINT_LOGD("%s() Start", __FUNCTION__);

	// Check Timer State
	if(true == gQCarCamStubTimerInf.isStarted)
	{
		PRINT_LOGI("%s() Timer is already started", __FUNCTION__);
		return QCARCAM_RET_OK;
	}
	
	// Timer Start
	result = spf_timer_start(&gQCarCamStubTimerInf.timerObject);
	if(0 != result)
	{
		// Error
		PRINT_LOGE("%s() spf_timer_start() Failed result[%d]", __FUNCTION__, result);
		return QCARCAM_RET_FAILED;
	}

	PRINT_LOGI("%s() spf_timer_start() Success", __FUNCTION__);

	// Set Timer State
	gQCarCamStubTimerInf.isStarted = true;

	PRINT_LOGD("%s() End", __FUNCTION__);
	
	return QCARCAM_RET_OK;
}

//////////////////////////////////////////////////////////////////////////////////
// Stop Cyclic Timer
//////////////////////////////////////////////////////////////////////////////////
static QCarCamRet_e QCarCamStubStopTimer(void)
{
	INT32 result = 0;

	PRINT_LOGD("%s() Start", __FUNCTION__);

	// Check Timer State
	if(false == gQCarCamStubTimerInf.isStarted)
	{
		PRINT_LOGI("%s() Timer is already stopped", __FUNCTION__);
		return QCARCAM_RET_OK;
	}

	// Timer Stop
	result = spf_timer_stop(&gQCarCamStubTimerInf.timerObject);
	if(0 != result)
	{
		// Error
		PRINT_LOGE("%s() spf_timer_stop() Failed result[%d]", __FUNCTION__, result);
		return QCARCAM_RET_FAILED;
	}

	PRINT_LOGI("%s() spf_timer_stop() Success", __FUNCTION__);

	// Set Timer State
	gQCarCamStubTimerInf.isStarted = false;

	PRINT_LOGD("%s() End", __FUNCTION__);
	
	return QCARCAM_RET_OK;
}

//////////////////////////////////////////////////////////////////////////////////
// Delete Cyclic Timer
//////////////////////////////////////////////////////////////////////////////////
static QCarCamRet_e QCarCamStubDeleteTimer(void)
{
	INT32 result = 0;

	PRINT_LOGD("%s() Start", __FUNCTION__);

	// Check Timer State
	if(true == gQCarCamStubTimerInf.isStarted)
	{
		PRINT_LOGE("%s() Timer is still started", __FUNCTION__);
		return QCARCAM_RET_FAILED;
	}

	// Timer Delete
	result = spf_timer_delete(&gQCarCamStubTimerInf.timerObject);
	if(0 != result)
	{
		// Error
		PRINT_LOGE("%s() spf_timer_delete() Failed result[%d]", __FUNCTION__, result);
		return QCARCAM_RET_FAILED;
	}

	PRINT_LOGI("%s() spf_timer_delete() Success", __FUNCTION__);

	PRINT_LOGD("%s() End", __FUNCTION__);
	
	return QCARCAM_RET_OK;
}

//////////////////////////////////////////////////////////////////////////////////
// Make Return Value by File
//////////////////////////////////////////////////////////////////////////////////
static QCarCamRet_e QCarCamStubMakeReturnValueByFile(const char* p_path, QCarCamRet_e *pRetVal)
{
	int readRet = 0;
	char testCommand[QCARCAM_STUB_TEST_CTRL_FILE_SIZE];
	QCarCamRet_e retValue = QCARCAM_RET_OK;

	PRINT_LOGD("%s() Start", __FUNCTION__);

	memset(testCommand, 0x00, sizeof(testCommand));

	// Parameter Check
	if((nullptr == p_path) || (nullptr == pRetVal))
	{
		// Error
		PRINT_LOGE("%s() Error", __FUNCTION__);
		return QCARCAM_RET_BADPARAM;
	}

	// Read File
	readRet = StubClientRead_str(p_path, testCommand);
	if(0 > readRet)
	{
		PRINT_LOGE("%s() StubClientRead_str() Error readRet[%d] p_path[%s]", __FUNCTION__, readRet, p_path);
		return QCARCAM_RET_FAILED;
	}

	// Make Return Value
	if(0 == testCommand[0])
	{
		PRINT_LOGD("%s() testCommand is empty, use QCARCAM_RET_OK", __FUNCTION__);
		retValue = QCARCAM_RET_OK;
	}
	else if(0 == strncmp("QCARCAM_RET_OK", testCommand, QCARCAM_STUB_TEST_CTRL_FILE_SIZE))
	{
		retValue = QCARCAM_RET_OK;
	}
	else if(0 == strncmp("QCARCAM_RET_FAILED", testCommand, QCARCAM_STUB_TEST_CTRL_FILE_SIZE))
	{
		retValue = QCARCAM_RET_FAILED;
	}
	else if(0 == strncmp("QCARCAM_RET_BADPARAM", testCommand, QCARCAM_STUB_TEST_CTRL_FILE_SIZE))
	{
		retValue = QCARCAM_RET_BADPARAM;
	}
	else if(0 == strncmp("QCARCAM_RET_BADSTATE", testCommand, QCARCAM_STUB_TEST_CTRL_FILE_SIZE))
	{
		retValue = QCARCAM_RET_BADSTATE;
	}
	else if(0 == strncmp("QCARCAM_RET_NOMEM", testCommand, QCARCAM_STUB_TEST_CTRL_FILE_SIZE))
	{
		retValue = QCARCAM_RET_NOMEM;
	}
	else if(0 == strncmp("QCARCAM_RET_UNSUPPORTED", testCommand, QCARCAM_STUB_TEST_CTRL_FILE_SIZE))
	{
		retValue = QCARCAM_RET_UNSUPPORTED;
	}
	else if(0 == strncmp("QCARCAM_RET_TIMEOUT", testCommand, QCARCAM_STUB_TEST_CTRL_FILE_SIZE))
	{
		retValue = QCARCAM_RET_TIMEOUT;
	}
	else if(0 == strncmp("QCARCAM_RET_BUSY", testCommand, QCARCAM_STUB_TEST_CTRL_FILE_SIZE))
	{
		retValue = QCARCAM_RET_BUSY;
	}
	else
	{
		PRINT_LOGE("%s() [%s] is unknown command, use QCARCAM_RET_OK", __FUNCTION__, testCommand);
		retValue = QCARCAM_RET_OK;
	}

	// Output Return Value
	*pRetVal = retValue;

	PRINT_LOGD("%s() End", __FUNCTION__);

	return QCARCAM_RET_OK;
}

#if 1
//Bev3追加 loadPng
static int loadPng(const char *filename, struct PngInfo *pngInf )
{
	int ret = -1;
	FILE *fp = nullptr;
	png_structp pngPtr = nullptr;//空の構造体
	png_infop   infPtr = nullptr;//空の構造体
	
	int bit_depth          = 0;
	int color_type         = 0;
	int interlace_type     = 0;
	int compression_type   = 0;
	int filter_method      = 0;
	unsigned int row_bytes = 0;
	unsigned int memSiz    = 0;
	png_bytepp rows        = nullptr;
	
	
	do
	{
		if (nullptr == filename)
		{
			PRINT_LOGE("Err:file is NULL");
			break;
		}
		
		PRINT_LOGD("loading image file %s.", filename);
	
		fp = fopen(filename, "rb");
		if (nullptr == fp)
		{
			PRINT_LOGE("Err:file open %s", filename);
			break;
		}
		
		pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!pngPtr)
		{
			PRINT_LOGE("Err:png_create_read_struct");
			break;// png構造体の作成に失敗した場合、エラーメッセージを表示
		}
		infPtr = png_create_info_struct(pngPtr);
		if (!infPtr)
		{
			PRINT_LOGE("Err:png_create_info_struct");
			break;// info構造体の作成に失敗した場合、エラーメッセージを表示
		}
		
		/* ファイルヘッダ情報の読み込み */
		if (setjmp(png_jmpbuf(pngPtr)) == 0)
		{
			png_init_io(pngPtr, fp);// ファイルポインタをpng構造体に設定
			png_set_sig_bytes(pngPtr, 0);//ファイルストリームから既に読みだしてしまったマジックナンバーの長さを設定
			png_read_png(pngPtr, infPtr, (PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND), NULL);
			//画像データの読み込み
			
			/* 読み込んだ情報をテクスチャ構造体に設定 */
			png_get_IHDR(pngPtr, infPtr, &pngInf->width, &pngInf->height, &bit_depth, &color_type, &interlace_type,
						&compression_type, &filter_method);
			
			row_bytes = (unsigned int)png_get_rowbytes(pngPtr, infPtr);
			memSiz = row_bytes * pngInf->height;
			pngInf->data =static_cast<uint8_t *>(malloc(memSiz));
			rows = png_get_rows(pngPtr, infPtr);//全体分解した生データ
			
			for (unsigned int i = 0; i < pngInf->height; ++i)
			{
				memcpy(pngInf->data + (row_bytes * i), rows[i], row_bytes);
				
				/* ARGBデータのRB交換 */
				for (unsigned int j = 0; j < row_bytes; j+=4)
				{
				    unsigned char *pos = (pngInf->data + (row_bytes * i) + j);
					unsigned char tmp = pos[2];
					pos[2] = pos[0];
					pos[0] = tmp;
				}
			}
			png_destroy_read_struct(&pngPtr, &infPtr, NULL);
			ret = 0;
		}
		else
		{
			PRINT_LOGE("Err:png_jmpbuf");
			break;// エラーが発生した場合、エラーメッセージを表示
		}
	} while(false);
	
	if (fp != nullptr)
	{
		fclose(fp);
	}
	PRINT_LOGD("Here the%p ", pngInf->data);
	PRINT_LOGD("loaded.");
	return ret;
}

static void SetBufadr(qcarcam_stub_hndl_type hType)
{
	gQcarCamStubHndlList[hType].bufadr = std::make_unique<std::vector<void*>>();

	int		ret;
	struct	PngInfo pngInf;
	for (auto fname : *gpPathName[hType])
	{
		ret = loadPng(fname, &pngInf);
		if (ret == 0)
		{
			gQcarCamStubHndlList[hType].bufadr.get()->push_back(pngInf.data);
		}
		else
		{
			PRINT_LOGE("%s()LoadPng Error [%s]", __FUNCTION__, fname);
		}
	}
}

static void ClearBufadr(qcarcam_stub_hndl_info& hndl)
{
	auto pData = hndl.bufadr.get();
	if (nullptr != pData)
	{
		for (auto pngdata : *pData)
		{
			free(pngdata);
		}
		pData->clear();
		hndl.bufadr = nullptr;
	}
}

#endif

