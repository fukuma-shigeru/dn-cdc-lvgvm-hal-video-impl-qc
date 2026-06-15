//////////////////////////////////////////////////////////////////////////////////
//	include
//////////////////////////////////////////////////////////////////////////////////
#include "qcarcam_seq.h"

//////////////////////////////////////////////////////////////////////////////////
//	Grobal variable
//////////////////////////////////////////////////////////////////////////////////
static QCarCamHndl_t G_BEV_Hndl;	//QCarCamOpen()で取得したhndl最終受信値
static void*	G_BEV_PlaneBuf[D_BUFFUER_NUM];		// Planeバッファ
static Renderer		G_BEV_Renderer;
static QCarCamBuffer_t*	G_BEV_Buffer = nullptr;
static bool		G_BEV_IsSHM = false;

/****************************************************************************/
/*			bev_Initialization												*/
/****************************************************************************/
static void bev_Initialization( void )
{
	G_BEV_Hndl = D_UINT64_INITIAL_VAL;
	for (int i = 0; i < D_BUFFUER_NUM; i++)
	{
		G_BEV_PlaneBuf[i] = nullptr;
	}
}

/****************************************************************************/
/*			BEV_Capture_Initializaion										*/
/****************************************************************************/
QCarCamRet_e BEV_Capture_Initializaion( void )
{
	QCarCamRet_e ret = QCARCAM_RET_OK;

	/* qcarcam_seq.cpp内のG変数初期化 */
	bev_Initialization();

	PRINT_LOGD("[START]");

	/***********************/
	/* QCarCamInitialize() */
	/***********************/
	ret = bev_Internal_QCarCamInitialize();
	if( ret != QCARCAM_RET_OK )
	{
		return ret;
	}

	PRINT_LOGD("[END]");

	return ret;
}

/****************************************************************************/
/*			BEV_Capture_Setting												*/
/****************************************************************************/
QCarCamRet_e BEV_Capture_Setting( int inputType, const uint32_t width, const uint32_t height, const char *kind, const char *buftype )
{
	QCarCamRet_e					ret				= QCARCAM_RET_OK;		//all func
	QCarCamHndl_t					l_hndl			= D_UINT64_INITIAL_VAL;	//all func

	QCarCamInput_t					*pinputs		= nullptr;				//QCarCamQueryInputs()
	uint32_t						l_num_inputs	= D_UINT32_INITIAL_VAL;	//QCarCamQueryInputs()
	uint32_t						ret_size		= D_UINT32_INITIAL_VAL;	//QCarCamQueryInputs()

	S_BEV_QCARCAMMOPEN_INPUT_DATA	l_input_data;							//QCarCamQueryInputModes()

	PRINT_LOGD("[START] inputType=%d width=%u height=%u",inputType, width, height );

	/************************/
	/* 使用バッファ調査     */
	/************************/
	G_BEV_IsSHM = false;
	if ( strcmp(buftype, "shm") == 0)
	{
		G_BEV_IsSHM = true;
	}

	/************************/
	/* QCarCamQueryInputs() */
	/************************/
	ret = bev_Internal_QCarCamQueryInputs( &pinputs, &l_num_inputs, &ret_size );
	if( ret != QCARCAM_RET_OK )
	{
		return ret;
	}

	/****************************/
	/* QCarCamQueryInputModes()	*/
	/****************************/
	ret = bev_Internal_QCarCamQueryInputModes( &l_input_data, inputType, width, height, ret_size, pinputs, kind );
	if( ret != QCARCAM_RET_OK )
	{
		return ret;
	}

	/*****************/
	/* QCarCamOpen() */
	/*****************/
	ret = bev_Internal_QCarCamOpen( l_input_data );
	if( ret != QCARCAM_RET_OK )
	{
		return ret;
	}

	l_hndl = bev_Get_hndl();

	/* bev_Internal_QCarCamOpen()によってhndlが初期値から変更がない場合 */
	if( l_hndl == D_UINT64_INITIAL_VAL )
	{
		PRINT_LOGE("[END] hndl is not set l_hndl=%llu", ( unsigned long long )l_hndl );

		return QCARCAM_RET_NOMEM;
	}

	/***********************/
	/* QCarCamSetBuffers() */
	/***********************/
	ret = bev_Internal_QCarCamSetBuffers( l_hndl, width, height, l_input_data.colorFmt );
	if( ret != QCARCAM_RET_OK )
	{
		return ret;
	}

	/**********************************/
	/* QCarCamRegisterEventCallback() */
	/**********************************/
	ret = bev_Internal_QCarCamRegisterEventCallback( l_hndl );
	if( ret != QCARCAM_RET_OK )
	{
		return ret;
	}

	/*********************/
	/* QCarCamSetParam() */
	/*********************/
	ret = bev_Internal_QCarCamSetParam( l_hndl );
	if( ret != QCARCAM_RET_OK )
	{
		return ret;
	}

	/********************/
	/* QCarCamReserve() */
	/********************/
	ret = bev_Internal_QCarCamReserve( l_hndl );
	if( ret != QCARCAM_RET_OK )
	{
		return ret;
	}

	PRINT_LOGD("[END]");

	return ret;
}

/****************************************************************************/
/*			BEV_Capture_Start												*/
/****************************************************************************/
QCarCamRet_e BEV_Capture_Start( void )
{
	QCarCamRet_e 	ret		= QCARCAM_RET_OK;		//all func
	QCarCamHndl_t	l_hndl	= D_UINT64_INITIAL_VAL;	//all func

	PRINT_LOGD("[START]");

	/* G変数から値を受け取る */
	l_hndl = bev_Get_hndl();

	/* bev_Internal_QCarCamOpen()によってhndlが初期値から変更がない場合 */
	if( l_hndl == D_UINT64_INITIAL_VAL )
	{
		PRINT_LOGE("[END] hndl is not set l_hndl=%llu", ( unsigned long long )l_hndl );

		return QCARCAM_RET_NOMEM;
	}

	/*******************/
	/*  QCarCamStart() */
	/*******************/
	ret = bev_Internal_QCarCamStart( l_hndl );
	if( ret != QCARCAM_RET_OK )
	{
		return ret;
	}

	PRINT_LOGD("[END]");

	return ret;
}

/****************************************************************************/
/*			BEV_Capture_Interrupt											*/
/****************************************************************************/
/*-----------------------------------------------------------------------------------------*/
/* シーケンス：Capture_Interrupt	 	*/
/*-----------------------------------------------------------------------------------------*/
QCarCamRet_e  BEV_Capture_Interrupt( const QCarCamHndl_t hndl, const uint32_t eventId, const QCarCamEventPayload_t *pPayload, void *pPrivateData )
{
	QCarCamRet_e		ret = QCARCAM_RET_OK;	//all func

	QCarCamFrameInfo_t	FrameInfo;				//QCarCamGetFrame()

	PRINT_LOGI("[START]Sequence Capture_Interrupt hndl=%llu eventId=%u", ( unsigned long long )hndl, eventId );

	/* bev_Internal_QCarCamOpen()によってhndlが初期値から変更がない場合 */
	if( hndl == D_UINT64_INITIAL_VAL )
	{
		PRINT_LOGE("[END] hndl is not set hndl=%llu", ( unsigned long long )hndl );

		return QCARCAM_RET_NOMEM;
	}

	if( eventId == QCARCAM_EVENT_FRAME_READY )
	{
		/*********************/
		/* QCarCamGetFrame() */
		/*********************/
		ret = bev_Internal_QCarCamGetFrame( hndl, &FrameInfo );
		if( ret != QCARCAM_RET_OK )
		{
			return ret;
		}

		/***********/
		/* Drawing */
		/***********/
		bev_Redraw(FrameInfo.bufferIndex);

		/***********************/
		/* QCarCamReleaseFrame */
		/***********************/
		ret = bev_Internal_QCarCamReleaseFrame( hndl, &FrameInfo );
		if( ret != QCARCAM_RET_OK )
		{
			return ret;
		}
	}

	PRINT_LOGD("[END]");

	return ret;
}

/****************************************************************************/
/*			BEV_Capture_Stop												*/
/****************************************************************************/
QCarCamRet_e BEV_Capture_Stop( void )
{
	QCarCamRet_e	ret		= QCARCAM_RET_OK;		//all func
	QCarCamHndl_t	l_hndl	= D_UINT64_INITIAL_VAL;	//all func

	PRINT_LOGD("[START]");

	/* G変数から値を受け取る */
	l_hndl = bev_Get_hndl();

	/* bev_Internal_QCarCamOpen()によってhndlが初期値から変更がない場合 */
	if( l_hndl == D_UINT64_INITIAL_VAL )
	{
		PRINT_LOGE("[END] hndl is not set l_hndl=%llu", ( unsigned long long )l_hndl );

		return QCARCAM_RET_NOMEM;
	}

	/*****************/
	/* QCarCamStop() */
	/*****************/
	ret = bev_Internal_QCarCamStop( l_hndl );
	if( ret != QCARCAM_RET_OK )
	{
		return ret;
	}

	PRINT_LOGD("[END]");

	return ret;
}

/****************************************************************************/
/*			BEV_Capture_End													*/
/****************************************************************************/
QCarCamRet_e BEV_Capture_End( void )
{
	QCarCamRet_e	ret		= QCARCAM_RET_OK;		//all func
	QCarCamHndl_t	l_hndl	= D_UINT64_INITIAL_VAL;	//all func

	PRINT_LOGD("[START]");

	/* G変数から値を受け取る */
	l_hndl = bev_Get_hndl();

	/* bev_Internal_QCarCamOpen()によってhndlが初期値から変更されている場合 */
	if( l_hndl != D_UINT64_INITIAL_VAL )
	{
		/********************/
		/* QCarCamRelease() */
		/********************/
		ret = bev_Internal_QCarCamRelease( l_hndl );
		if( ret != QCARCAM_RET_OK )
		{
			return ret;
		}

		/************************************/
		/* QCarCamUnregisterEventCallback() */
		/************************************/
		ret = bev_Internal_QCarCamUnregisterEventCallback( l_hndl );
		if( ret != QCARCAM_RET_OK )
		{
			return ret;
		}

		/******************/
		/* QCarCamClose() */
		/******************/
		ret = bev_Internal_QCarCamClose( l_hndl );
		if( ret != QCARCAM_RET_OK )
		{
			return ret;
		}
	}
	else
	{
		PRINT_LOGD("hndl is not set l_hndl=%llu", ( unsigned long long )l_hndl );
	}

	/*************************/
	/* QCarCamUninitialize() */
	/*************************/
	ret = bev_Internal_QCarCamUninitialize();
	if( ret != QCARCAM_RET_OK )
	{
		return ret;
	}

	PRINT_LOGD("[END]");

	return ret;
}

/* 内部関数 */
/****************************************************************************/
/*			bev_Internal_QCarCamInitialize()								*/
/****************************************************************************/
static QCarCamRet_e bev_Internal_QCarCamInitialize( void )
{
	QCarCamRet_e	ret = QCARCAM_RET_OK;
	QCarCamInit_t	InitParams;

	PRINT_LOGD("[START]");

	/* 初期化 */
	memset( &InitParams, 0, sizeof( InitParams ) );

	InitParams.apiVersion = QCARCAM_VERSION;

	PRINT_LOGD("Call QCarCamInitialize() [apiVersion=%u]", InitParams.apiVersion );

	ret = QCarCamInitialize( &InitParams );

	if( ret != QCARCAM_RET_OK )
	{
		PRINT_LOGE("QCarCamInitialize() ret=%d", ret );

		return ret;
	}

	PRINT_LOGD("[END]");

	return ret;
}

/****************************************************************************/
/*			bev_Internal_QCarCamQueryInputs()								*/
/****************************************************************************/
static QCarCamRet_e bev_Internal_QCarCamQueryInputs( QCarCamInput_t **ppinputs, uint32_t *pnum_inputs, uint32_t *pret_size )
{
	QCarCamRet_e ret = QCARCAM_RET_OK;

	QCarCamInput_t *pinputs = nullptr;	//何配列分データが格納されるか不明のためポインタ変数宣言

	PRINT_LOGD("[START]");

	/* 1回目の関数コール */
	/* num_inputsには接続中の入力数が格納される */
	ret = QCarCamQueryInputs( NULL, 0, pnum_inputs );

	if( ( ret != QCARCAM_RET_OK ) || ( *pnum_inputs == 0 ) )
	{
		PRINT_LOGE("QCarCamQueryInputs():first call ret=%d num_inputs=%u", ret, *pnum_inputs );

		return QCARCAM_RET_LAST;
	}

	/* 1回目のコールで得た外部接続数 */
	PRINT_LOGI("QCarCamQueryInputs():first call return data [l_num_inputs = %u]", *pnum_inputs );

	/* <memo> mallocの0クリアするバージョン : memory確保後に0クリアする必要がない */
	pinputs = ( QCarCamInput_t* )calloc( *pnum_inputs, sizeof( *pinputs ) );

	/* 2回目の関数コール */
	/* ret_sizeにはpinputsの要素数が格納される */
	ret = QCarCamQueryInputs( pinputs, *pnum_inputs, pret_size );

	if( ret != QCARCAM_RET_OK )
	{
		PRINT_LOGE("QCarCamQueryInputs():second call ret=%d ret_size=%u", ret, *pret_size );

		return ret;
	}

	PRINT_LOGI("QCarCamQueryInputs():second call return data [num_inputs=%u ret_size=%u]", *pnum_inputs, *pret_size );

	/* CAMERA, HDMIに関するデータ出力 */
	for( uint32_t loop = 0; loop < *pnum_inputs; loop++ )
	{
		PRINT_LOGI("loop[%u]: inputId=%u devId=%u subdevId=%u inputName=%s numModes=%u flags=%u"
			, loop, pinputs[loop].inputId, pinputs[loop].devId, pinputs[loop].subdevId, pinputs[loop].inputName, pinputs[loop].numModes, pinputs[loop].flags );
	}

	/* *ppinputsにpinputsのアドレスを格納することで、各配列の先頭アドレスが格納される */
	*ppinputs = pinputs;

	/* pinputs は次関数bev_Internal_QCarCamQueryInputModes()で使用するためfree(メモリ解放)しない*/

	PRINT_LOGD("[END]");

	return ret;
}

/****************************************************************************/
/*			bev_Internal_QCarCamQueryInputModes()							*/
/****************************************************************************/
static QCarCamRet_e bev_Internal_QCarCamQueryInputModes(
	S_BEV_QCARCAMMOPEN_INPUT_DATA *pdata, const int inputType, const uint32_t width, const uint32_t height, const uint32_t size, QCarCamInput_t *pinputs, const char *kind)
{
	QCarCamRet_e ret = QCARCAM_RET_OK;

	QCarCamInputModes_t		InputModes;
	QCarCamMode_t			*pModes			= nullptr;
	bool					notfound		= true;

	PRINT_LOGD("[START]");

	/* 初期化 */
	memset( &InputModes, 0,	  sizeof( InputModes ) );

	/* 構造体ポインタ変数の各要素をdefine値で初期化 */
	pdata->inputId	= D_UINT32_INITIAL_VAL;
	pdata->inputMod	= D_UINT32_INITIAL_VAL;
	pdata->srcID	= D_UINT32_INITIAL_VAL;

	/* nullチェック */
	if( pinputs == nullptr )
	{
		PRINT_LOGE("pinputs is nullptr");

		return QCARCAM_RET_BADPARAM;
	}

	/* size : 2回目のQCarCamQueryInputs()で得た値 */
	for( uint32_t loop = 0; loop < size && notfound; loop++ )
	{
		PRINT_LOGD( "loop[%u] : inputId=%u devId=%u subdevId=%u inputName=%s numModes=%u flags=%u"
			, loop, pinputs[loop].inputId, pinputs[loop].devId, pinputs[loop].subdevId, pinputs[loop].inputName, pinputs[loop].numModes, pinputs[loop].flags );

		pModes = ( QCarCamMode_t* )calloc( pinputs[loop].numModes, sizeof( *pModes ) );

		/* nullチェック */
		if( pModes == nullptr )
		{
			PRINT_LOGE("QCarCamQueryInputModes() pModes=nullptr");

			return QCARCAM_RET_BADPARAM;
		}

		InputModes.pModes	= pModes;	
		InputModes.numModes	= pinputs[loop].numModes;

		/* 下記2点が一致する場合のみQCarCamQueryInputModes()をコール */
		/* 引数:inputType(0:CAMERA or 1:HDMI) */
		/* 文字列:"CAMERA" or "HDMI" */
		/* kindになにもデータが存在しない場合(nullチェック) */
		/* kindが.inputNameと一致している場合 */
		if( ( kind != nullptr ) && ( strcmp( pinputs[loop].inputName, kind ) == 0 ) )
		{
			PRINT_LOGI("inputId=%u inputName=%s", pinputs[loop].inputId, pinputs[loop].inputName );

			ret = QCarCamQueryInputModes( pinputs[loop].inputId, &InputModes );

			if( ret != QCARCAM_RET_OK )
			{
				PRINT_LOGE("QCarCamQueryInputModes() ret=%d", ret );

				return ret;
			}
			else
			{
				PRINT_LOGI("QCarCamQueryInputModes() OK currentMode=%u numModes=%u", InputModes.currentMode, InputModes.numModes );

				for( uint32_t j = 0; j < InputModes.numModes && notfound; j++ )
				{
					for( uint32_t k = 0; k < pModes[j].numSources && notfound; k++ )
					{
						PRINT_LOGI("All sources [input_idx=%u, pModes_idx=%u, source_idx=%u srcId=%u colorFmt=0x%08X fps=%.2f width=%u height=%u securityDomain=%u]"
							, loop, j, k, pModes[j].sources[k].srcId, pModes[j].sources[k].colorFmt, pModes[j].sources[k].fps, pModes[j].sources[k].width, pModes[j].sources[k].height, pModes[j].sources[k].securityDomain);
					}
				}

				for( uint32_t j = 0; j < InputModes.numModes && notfound; j++ )
				{
					for( uint32_t k = 0; k < pModes[j].numSources && notfound; k++ )
					{
						if( ( pModes[j].sources[k].width == width ) && ( pModes[j].sources[k].height == height ) )
						{
							notfound = false;
							pdata->inputMod = j;
							pdata->inputId	= loop;
							pdata->srcID = pModes[j].sources[k].srcId;
							pdata->colorFmt = pModes[j].sources[k].colorFmt;
							PRINT_LOGI("[input_idx=%u, pModes_idx=%u, source_idx=%u srcID=%u colorFmt=0x%08X fps=%.2f width=%u height=%u securityDomain=%u]"
								, loop, j, k, pdata->srcID, pdata->colorFmt, pModes[j].sources[k].fps, pModes[j].sources[k].width, pModes[j].sources[k].height, pModes[j].sources[k].securityDomain);
							break;
						}
					}
				}
			}
		}
	}

	/* 上ループ探索でヒットしなかった場合 */
	if( ( pdata->srcID == D_UINT32_INITIAL_VAL ) || ( pdata->inputMod == D_UINT32_INITIAL_VAL ) || ( pdata->inputId == D_UINT32_INITIAL_VAL ) )
	{
		PRINT_LOGE("[END] No results were found as a result of search.");

		return QCARCAM_RET_BADPARAM;
	}

	/* 後始末 */
	if( pinputs	!= nullptr )
	{
		free( pinputs );
		pinputs = nullptr;

		PRINT_LOGD("free( pinputs ) pinputs = nullptr");
	}
	if( pModes	!= nullptr )
	{
		free( pModes );
		pModes	= nullptr;

		PRINT_LOGD("free( pModes  ) pModes  = nullptr");
	}

	PRINT_LOGD("[END]");

	return ret;
}

/****************************************************************************/
/*			bev_Internal_QCarCamOpen()										*/
/****************************************************************************/
static QCarCamRet_e bev_Internal_QCarCamOpen( S_BEV_QCARCAMMOPEN_INPUT_DATA input_data )
{
	QCarCamRet_e	ret			= QCARCAM_RET_OK;

	QCarCamHndl_t	l_hndl		= D_UINT64_INITIAL_VAL;
	QCarCamOpen_t	OpenParams;

	PRINT_LOGD("[START]");

	/* 初期化 */
	memset( &OpenParams, 0, sizeof( OpenParams ) );

//	OpenParams.opMode				= QCARCAM_OPMODE_RAW_DUMP;
	OpenParams.opMode				= QCARCAM_OPMODE_ISP;		// (=1)
	OpenParams.numInputs			= 1;
	OpenParams.inputs[0].inputId	= input_data.inputId;		//result of search
	OpenParams.inputs[0].srcId		= input_data.srcID;		//result of search
	OpenParams.inputs[0].inputMode	= input_data.inputMod;	//result of search

	PRINT_LOGD("Call QCarCamOpen() [opMode=%llu numInputs=%u inputId=%u srcId=%u inputMode=%u]",
		( unsigned long long )OpenParams.opMode, OpenParams.numInputs, OpenParams.inputs[0].inputId, OpenParams.inputs[0].srcId, OpenParams.inputs[0].inputMode );

	/* オブジェクトハンドルを取得 */
	ret = QCarCamOpen( &OpenParams, &l_hndl );

	/* G変数へ代入 */
	bev_Set_hndl( l_hndl );

	if( ( ret != QCARCAM_RET_OK ) || ( l_hndl == D_UINT64_INITIAL_VAL ) )
	{
		PRINT_LOGE("QCarCamOpen() ret=%d l_hndl=%llu", ret, ( unsigned long long )l_hndl );

		return QCARCAM_RET_LAST;
	}

	/*オブジェクトハンドル値を出力 */
	PRINT_LOGI("QCarCamOpen() Hndl=%llu(0x%016lX)", ( unsigned long long )l_hndl, l_hndl );

	PRINT_LOGD("[END]");

	return ret;
}

/****************************************************************************/
/*			bev_Internal_QCarCamSetBuffers()								*/
/****************************************************************************/
static QCarCamRet_e bev_Internal_QCarCamSetBuffers( const QCarCamHndl_t input_hndl, const uint32_t input_width, const uint32_t input_height, const QCarCamColorFmt_e color_fmt)
{
	QCarCamRet_e		ret			= QCARCAM_RET_OK;

	bool				ret_m_buff	= true;
	QCarCamBufferList_t	bufList;

	PRINT_LOGD("[START]");

	/* 初期化 */
	memset( &bufList, 0, sizeof( bufList ) );
	G_BEV_Buffer = (QCarCamBuffer_t*)calloc(D_BUFFUER_NUM, sizeof(QCarCamBuffer_t));

	/* buff設定 */
	ret_m_buff = bev_make_buffer(G_BEV_Buffer, D_BUFFUER_NUM, input_width, input_height );
	if( ret_m_buff != true )
	{
		return QCARCAM_RET_FAILED;
	}

//	bufList.id			= 0;					//QCARCAM_BUFFERLIST_ID_OUTPUT_START?
//	bufList.id			= QCARCAM_BUFFERLIST_ID_INPUT_OUTPUT_START;
	bufList.id			= QCARCAM_BUFFERLIST_ID_OUTPUT_START;
	bufList.colorFmt	= color_fmt;
	bufList.pBuffers	= G_BEV_Buffer;
	bufList.nBuffers	= D_BUFFUER_NUM;
	bufList.flags		= QCARCAM_BUFFER_FLAG_OS_HNDL;

	PRINT_LOGD("Call QCarCamSetBuffers() [id=%u colorFmt=0x%08X nBuffers=%u flags=%u]",
		bufList.id, bufList.colorFmt, bufList.nBuffers, bufList.flags );

	ret = QCarCamSetBuffers( input_hndl, &bufList );

	if( ret != QCARCAM_RET_OK )
	{
		PRINT_LOGE("QCarCamSetBuffers() ret=%d", ret );

		return ret;
	}

	PRINT_LOGD("[END]");

	return ret;
}

/****************************************************************************/
/*			bev_Internal_QCarCamReserve()									*/
/****************************************************************************/
static QCarCamRet_e bev_Internal_QCarCamReserve( const QCarCamHndl_t input_hndl )
{
	QCarCamRet_e ret = QCARCAM_RET_OK;

	PRINT_LOGD("[START]");

	/* Hndl : QCarCamOpen()で取得した値 */
	PRINT_LOGD("hndl=0x%016lX", input_hndl);
	ret = QCarCamReserve( input_hndl );

	if( ret != QCARCAM_RET_OK )
	{
		PRINT_LOGE("QCarCamReserve() ret=%d", ret );

		return ret;
	}

	PRINT_LOGD("[END]");

	return ret;
}

/****************************************************************************/
/*			bev_Internal_QCarCamRegisterEventCallback()						*/
/****************************************************************************/
static QCarCamRet_e bev_Internal_QCarCamRegisterEventCallback( const QCarCamHndl_t input_hndl )
{
	QCarCamRet_e ret = QCARCAM_RET_OK;

	PRINT_LOGD("[START]");

	/* input_hndl : QCarCamOpen()で取得した値 */
	/* 内部関数:BEV_Capture_Interrupt()を登録 */
	/* QCarCam側でイベントがあった際にコールされる */
	ret = QCarCamRegisterEventCallback( input_hndl, BEV_Capture_Interrupt, NULL );

	if( ret != QCARCAM_RET_OK )
	{
		PRINT_LOGE("QCarCamRegisterEventCallback() ret=%d", ret );

		return ret;
	}

	PRINT_LOGD("[END]");

	return ret;
}

/****************************************************************************/
/*			bev_Internal_QCarCamSetParam()									*/
/****************************************************************************/
static QCarCamRet_e bev_Internal_QCarCamSetParam( const QCarCamHndl_t input_hndl )
{
	QCarCamRet_e	ret		= QCARCAM_RET_OK;

	uint32_t		param	= 0;

	PRINT_LOGD("[START]");

	param = ( QCARCAM_EVENT_FRAME_READY | QCARCAM_EVENT_INPUT_SIGNAL | QCARCAM_EVENT_ERROR | QCARCAM_EVENT_VENDOR );

	ret = QCarCamSetParam(
		input_hndl,
		QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK,
		&param,
		sizeof( param ) );

	if( ret != QCARCAM_RET_OK )
	{
		PRINT_LOGE("QCarCamSetParam() ret=%d", ret );

		return ret;
	}
	PRINT_LOGD("QCarCamSetParam(QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK) ret=%d", ret );

	// opModeがQCARCAM_OPMODE_ISPの場合は必要
	QCarCamIspUsecaseConfig_t	ispConfig{};
	constexpr QCarCamIspUsecase_e kUsecaseId{static_cast<QCarCamIspUsecase_e>(16)};
	ispConfig.usecaseId = kUsecaseId;

	ret = QCarCamSetParam(
		input_hndl,
		QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE,
		&ispConfig,
		sizeof( ispConfig ) );

	if( ret != QCARCAM_RET_OK )
	{
		PRINT_LOGE("QCarCamSetParam(QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE) ret=%d", ret );
		return ret;
	}
	PRINT_LOGD("QCarCamSetParam(QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE) ret=%d", ret );

	PRINT_LOGD("[END]");

	return ret;
}

/****************************************************************************/
/*			bev_Internal_QCarCamStart()										*/
/****************************************************************************/
static QCarCamRet_e bev_Internal_QCarCamStart( const QCarCamHndl_t input_hndl )
{
	QCarCamRet_e ret = QCARCAM_RET_OK;

	PRINT_LOGD("[START]");

	ret = QCarCamStart( input_hndl );

	if( ret != QCARCAM_RET_OK )
	{
		PRINT_LOGE("QCarCamStart() ret=%d", ret );

		return ret;
	}

	PRINT_LOGD("[END]");

	return ret;
}

/****************************************************************************/
/*			bev_Internal_QCarCamGetFrame()									*/
/****************************************************************************/
static QCarCamRet_e bev_Internal_QCarCamGetFrame( const QCarCamHndl_t input_hndl, QCarCamFrameInfo_t *pframeInfo )
{
	QCarCamRet_e ret = QCARCAM_RET_OK;

	PRINT_LOGD("[START]");

	/* 初期化 */
	memset( pframeInfo, 0, sizeof( *pframeInfo ) );

	ret = QCarCamGetFrame( input_hndl, pframeInfo, 0U, 0U );

	if( ret != QCARCAM_RET_OK )
	{
		PRINT_LOGE("QCarCamGetFrame() ret=%d", ret );

		return ret;
	}

	/* FrameInfo.bufferIndexに転送対象のフレーム番号が格納 */
	PRINT_LOGI("QCarCamGetFrame() OK [FrameInfo.id=%u FrameInfo.bufferIndex=%u]", pframeInfo->id, pframeInfo->bufferIndex );

	PRINT_LOGD("[END]");

	return ret;
}

/****************************************************************************/
/*			bev_Internal_QCarCamReleaseFrame()								*/
/****************************************************************************/
static QCarCamRet_e bev_Internal_QCarCamReleaseFrame( const QCarCamHndl_t input_hndl, QCarCamFrameInfo_t *pframeInfo )
{
	QCarCamRet_e ret = QCARCAM_RET_OK;

	PRINT_LOGD("[START]");

	ret = QCarCamReleaseFrame( input_hndl, pframeInfo->id, pframeInfo->bufferIndex );

	if( ret != QCARCAM_RET_OK )
	{
		PRINT_LOGE("QCarCamReleaseFrame() ret=%d", ret );

		return ret;
	}

	PRINT_LOGD("[END]");

	return ret;
}

/****************************************************************************/
/*			bev_Internal_QCarCamStop()										*/
/****************************************************************************/
static QCarCamRet_e bev_Internal_QCarCamStop( const QCarCamHndl_t input_hndl )
{
	QCarCamRet_e ret = QCARCAM_RET_OK;

	PRINT_LOGD("[START]");

	ret = QCarCamStop( input_hndl );

	if( ret != QCARCAM_RET_OK )
	{
		PRINT_LOGE("QCarCamStop() ret=%d", ret );

		return ret;
	}

	PRINT_LOGD("[END]");

	return ret;
}

/****************************************************************************/
/*			bev_Internal_QCarCamUnregisterEventCallback()					*/
/****************************************************************************/
static QCarCamRet_e bev_Internal_QCarCamUnregisterEventCallback( const QCarCamHndl_t input_hndl )
{
	QCarCamRet_e ret = QCARCAM_RET_OK;

	PRINT_LOGD("[START]");

	ret = QCarCamUnregisterEventCallback( input_hndl );

	if( ret != QCARCAM_RET_OK )
	{
		PRINT_LOGE("QCarCamUnregisterEventCallback() ret=%d", ret );

		return ret;
	}

	PRINT_LOGD("[END]");

	return ret;
}

/****************************************************************************/
/*			bev_Internal_QCarCamRelease()									*/
/****************************************************************************/
static QCarCamRet_e bev_Internal_QCarCamRelease( const QCarCamHndl_t input_hndl )
{
	QCarCamRet_e ret = QCARCAM_RET_OK;

	PRINT_LOGD("[START]");

	ret = QCarCamRelease( input_hndl );

	if( ret != QCARCAM_RET_OK )
	{
		PRINT_LOGE("QCarCamRelease() ret=%d", ret );

		return ret;
	}

	PRINT_LOGD("[END]");

	return ret;
}

/****************************************************************************/
/*			bev_Internal_QCarCamClose()										*/
/****************************************************************************/
static QCarCamRet_e bev_Internal_QCarCamClose( const QCarCamHndl_t input_hndl )
{
	QCarCamRet_e ret = QCARCAM_RET_OK;

	PRINT_LOGD("[START]");

	ret = QCarCamClose( input_hndl );

	if( ret != QCARCAM_RET_OK )
	{
		PRINT_LOGE("QCarCamClose() ret=%d", ret );

		return ret;
	}

	/* 描画終了処理 */
	bev_Stop_Renderer();
	free(G_BEV_Buffer);
	G_BEV_Buffer = nullptr;

	PRINT_LOGD("[END]");

	return ret;
}

/****************************************************************************/
/*			bev_Internal_QCarCamUninitialize()								*/
/****************************************************************************/
static QCarCamRet_e bev_Internal_QCarCamUninitialize( void )
{
	QCarCamRet_e ret = QCARCAM_RET_OK;

	PRINT_LOGD("[START]");

	ret = QCarCamUninitialize();

	if( ret != QCARCAM_RET_OK )
	{
		PRINT_LOGE("QCarCamUninitialize() ret=%d", ret );

		return ret;
	}

	PRINT_LOGD("[END]");

	return ret;
}
//END 内部関数

/****************************************************************************/
/*			BEV_make_buffer													*/
/****************************************************************************/
static bool bev_make_buffer( QCarCamBuffer_t* pbuffer, const uint32_t buff_num, const uint32_t width, const uint32_t height )
{
	PRINT_LOGD("[START] buff_num=%u width=%u height=%u", buff_num, width, height );

	/* nullチェック */
	if( pbuffer == nullptr )
	{
		PRINT_LOGE("BEV_make_buffer() pbuffer==nullptr");

		return false;
	}

	/* 描画初期化 */
	if(true != bev_Start_Renderer(width, height))
	{
		PRINT_LOGE("bev_Start_Renderer() failed. w=[%u] h=[%u]",width, height);
		return false;
	}

	/* 同じデータをbuff_num数分生成する */
	/* InputModeで設定したデータサイズと異なると描画できなくなる */
	/* つまり、main()からコールされた際のwidth, heightの値*/
	for( int loop = 0; loop < buff_num; loop++)
	{
		pbuffer->numPlanes			= 1;
		pbuffer->planes[0].width	= width;
		pbuffer->planes[0].height	= height;
		pbuffer->planes[0].stride	= G_BEV_Renderer.buffer[loop].plane[0].stride;
		if (G_BEV_IsSHM)
		{
			pbuffer->planes[0].memHndl	= (uint64_t)((uintptr_t)G_BEV_Renderer.buffer[loop].plane[0].virtAddr);
		}
		else
		{
			pbuffer->planes[0].memHndl	= (uint64_t)(G_BEV_Renderer.buffer[loop].plane[0].fd);
		}
		pbuffer->planes[0].size		= pbuffer->planes[0].stride * height;

		PRINT_LOGD("buffer[%d]: numPlanes=%u width=%u height=%u stride=%u size=%u memHndl=0x%llX",
			loop, pbuffer->numPlanes, pbuffer->planes[0].width, pbuffer->planes[0].height, pbuffer->planes[0].stride, pbuffer->planes[0].size, ( unsigned long long )pbuffer->planes[0].memHndl);

		/* 配列をインクリメント */
		/* 実行しないと配列(buffer[D_BUFFUER_NUM]のindex(0)番目)のみbuff_num回数分ループする */
		pbuffer++;
	}

	PRINT_LOGD("[END]");

	return true;
}

/****************************************************************************/
/*			BEV_Set_hndl													*/
/****************************************************************************/
static void bev_Set_hndl( QCarCamHndl_t Hndl )
{
	PRINT_LOGD("[START] Hndl=%llu", ( unsigned long long )Hndl );

	if( Hndl != G_BEV_Hndl )
	{
		G_BEV_Hndl = Hndl;

		PRINT_LOGI("G_BEV_Hndl=%llu", ( unsigned long long )G_BEV_Hndl );
	}
	else
	{
		PRINT_LOGE("Hndl == G_BEV_Hndl Hndl=%llu", ( unsigned long long )Hndl );
	}

	PRINT_LOGD("[END]");
}

/****************************************************************************/
/*			BEV_Get_hndl													*/
/****************************************************************************/
static QCarCamHndl_t bev_Get_hndl( void )
{
	PRINT_LOGD("[START]");

	return G_BEV_Hndl;
}

/****************************************************************************/
/*			BEV_Start_Renderer												*/
/****************************************************************************/
static bool bev_Start_Renderer( const uint32_t width, const uint32_t height )
{
	PRINT_LOGD("[START] width=%u height=%u", width, height );

	Renderer*	p_data = &G_BEV_Renderer;

	p_data->buffer_count = D_BUFFUER_NUM;

	p_data->p_renderer_ = new wlrenderer::CWaylandRenderer;
	p_data->p_renderer_->Initialize();

	p_data->p_renderer_config_ = new wlrenderer::CWaylandRendererConfig();
	if (G_BEV_IsSHM)
	{
		p_data->p_renderer_config_->SetBufferType(wlrenderer::CWaylandRendererVideo::VIDEO_BUFFER_TYPE_SHM);
		p_data->p_renderer_config_->SetFormat(WL_SHM_FORMAT_XRGB8888);
	}
	else
	{
		p_data->p_renderer_config_->SetBufferType(wlrenderer::CWaylandRendererVideo::VIDEO_BUFFER_TYPE_GBM);
		p_data->p_renderer_config_->SetFormat(DRM_FORMAT_UYVY);
	}
	p_data->p_renderer_config_->SetBufferCount(D_BUFFUER_NUM);
	p_data->p_renderer_config_->SetWidth(width);
	p_data->p_renderer_config_->SetHeight(height);
	p_data->p_renderer_config_->AddSurfaceId(D_BEV_FRONT_SURFACE_ID);
	p_data->p_renderer_config_->SetLoopBuffer(true);

	/* 映像バッファ領域の取得 */
	p_data->p_renderer_video_ = p_data->p_renderer_->CreateRendererVideo(*p_data->p_renderer_config_);


	/* バッファ数分 */
	for (uint32_t i = 0; i < p_data->buffer_count; ++i)
	{
		p_data->buffer[i].width = width;
		p_data->buffer[i].height= height;
//		p_data->buffer[i].format= file_format;
		p_data->buffer[i].p_video_buffer_ = p_data->p_renderer_video_->GetBuffer();
		uint32_t plane_count = p_data->buffer[i].p_video_buffer_->GetPlaneCount();	/* プレーン配列数 */
		p_data->buffer[i].plane_count = plane_count;
		if (nullptr == p_data->buffer[i].p_video_buffer_)
		{
			PRINT_LOGE("GetBuffer failed idx=%d",i);
			return false;
		}

		/* バッファプレーンの設定 */
		for (uint32_t planeNo = 0; planeNo < plane_count; ++planeNo)
		{
			p_data->buffer[i].plane[planeNo].stride    = p_data->buffer[i].p_video_buffer_->GetStride(planeNo);
			if (G_BEV_IsSHM)
			{
				p_data->buffer[i].plane[planeNo].virtAddr  = p_data->buffer[i].p_video_buffer_->GetPlaneAddr(planeNo);
				p_data->buffer[i].plane[planeNo].fd        = -1;
			}
			else
			{
				p_data->buffer[i].plane[planeNo].virtAddr  = nullptr;
				p_data->buffer[i].plane[planeNo].fd        = p_data->buffer[i].p_video_buffer_->GetDmafd(planeNo);
			}

			PRINT_LOGD("idx=[%u] p_addr[%p] fd=[%d]", i, p_data->buffer[i].plane[planeNo].virtAddr, p_data->buffer[i].plane[planeNo].fd);
		}
	}

	PRINT_LOGD("[END]");

	return true;
}

/****************************************************************************/
/*			BEV_Stop_Renderer												*/
/****************************************************************************/
static void bev_Stop_Renderer()
{
	PRINT_LOGD("finalize start.");

	Renderer*	p_data = &G_BEV_Renderer;

	delete p_data->p_renderer_config_;

	if ( nullptr != p_data->p_renderer_ )
	{
		p_data->p_renderer_->RemoveRendererVideo(p_data->p_renderer_video_);
		p_data->p_renderer_->Finalize();
		delete p_data->p_renderer_;
	}

	PRINT_LOGD("finalize end.");
}

/****************************************************************************/
/*			BEV_Redraw														*/
/****************************************************************************/
static void bev_Redraw(uint32_t	index)
{
//	PRINT_LOGD("redraw start.");

	Renderer*	p_data = &G_BEV_Renderer;

	if ((index < D_BUFFUER_NUM) && (nullptr != p_data->buffer[index].p_video_buffer_))
	{
		std::vector<int32_t>	surface_ids{};
		p_data->p_renderer_config_->GetSurfaceIds(surface_ids);
		p_data->p_renderer_video_->SendBuffer(p_data->buffer[index].p_video_buffer_, surface_ids);
	}

//	PRINT_LOGD("redraw end.");
}
