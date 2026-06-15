//////////////////////////////////////////////////////////////////////////////////
//	include
//////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qcarcam.h"
#include "qcarcam_types.h"
#include "wl_renderer_public.h"

//////////////////////////////////////////////////////////////////////////////////
//	Debug Log
//////////////////////////////////////////////////////////////////////////////////
#if 1	// Log Enable
#define PRINT_LOGE(fmt, ...)	fprintf(stderr, "[QCARCAM][E] %s(%d) " fmt "\n",__FUNCTION__,__LINE__,##__VA_ARGS__)
#define PRINT_LOGI(fmt, ...)	fprintf(stderr, "[QCARCAM][I] %s(%d) " fmt "\n",__FUNCTION__,__LINE__,##__VA_ARGS__)
#define PRINT_LOGD(fmt, ...)	fprintf(stderr, "[QCARCAM][D] %s(%d) " fmt "\n",__FUNCTION__,__LINE__,##__VA_ARGS__)
#else	// Log Disable
#define PRINT_LOGE(fmt, ...)
#define PRINT_LOGI(fmt, ...)
#define PRINT_LOGD(fmt, ...)
#endif

//////////////////////////////////////////////////////////////////////////////////
//	Define
//////////////////////////////////////////////////////////////////////////////////
#define D_MAX_BUFFER_PLANES		3
#define D_BUFFUER_NUM			5
#define D_MAX_LINE_SIZE			100
#define	D_UINT32_INITIAL_VAL	0xffffffff
#define	D_UINT64_INITIAL_VAL	0xffffffffffffffff
#define D_BEV_QCARCAM_NUM_CAM	0
#define D_BEV_QCARCAM_NUM_HDMI	1

#define D_BEV_QCARCAM_CHAR_CAM	"CAMERA"
#define D_BEV_QCARCAM_CHAR_HDMI	"HDMI"

#define D_BEV_FRONT_SURFACE_ID	(20000000)

typedef struct
{
	uint32_t inputId;
	uint32_t srcID;
	uint32_t inputMod;
	QCarCamColorFmt_e colorFmt;
}S_BEV_QCARCAMMOPEN_INPUT_DATA;

/* バッファのプレーン情報 */
typedef struct {
	uint32_t stride;
	void*	 virtAddr;
	int32_t	 fd;
} Plane;

/* バッファ情報 */
typedef struct {
	uint32_t width;
	uint32_t height;
	uint32_t stride;
//	uint32_t format;
	uint32_t plane_count;
	Plane	 plane[D_MAX_BUFFER_PLANES];
	wlrenderer::CWaylandRendererVideoBuffer* p_video_buffer_;
} Buffer;

/* レンダラー情報 */
typedef struct {
	wlrenderer::CWaylandRenderer*		p_renderer_;
	wlrenderer::CWaylandRendererConfig*	p_renderer_config_;
	wlrenderer::CWaylandRendererVideo*	p_renderer_video_;
	uint32_t buffer_count;
	Buffer	 buffer[D_BUFFUER_NUM];
} Renderer;


/* プロトタイプ宣言 */
QCarCamRet_e	BEV_Capture_Initializaion( void );	//シーケンス:[Capture_Initializaion]
QCarCamRet_e	BEV_Capture_Setting( int inputType, const uint32_t width, const uint32_t height, const char *kind, const char *buftype  );	//シーケンス:[Capture_Setting]
QCarCamRet_e	BEV_Capture_Start( void );			//シーケンス:[Capture_Start]
QCarCamRet_e	BEV_Capture_Stop( void );			//シーケンス:[Capture_Stop]
QCarCamRet_e	BEV_Capture_End( void );			//シーケンス:[Capture_End]
QCarCamRet_e	BEV_Capture_Interrupt( const QCarCamHndl_t hndl, const uint32_t eventId, const QCarCamEventPayload_t *pPayload, void *pPrivateData );	//シーケンス:[Capture_Interrupt]

/* 内部関数 */
static void			 bev_Initialization( void );			//qcarcam_seq.cpp内部のG変数初期化
static bool			 bev_make_buffer( QCarCamBuffer_t* pbuffer, const uint32_t buff_num, const uint32_t width, const uint32_t height );
static void			 bev_Set_hndl( QCarCamHndl_t Hndl );	//Setter:G_BEV_Hndl
static QCarCamHndl_t bev_Get_hndl( void );					//Getter:G_BEV_Hndl
static bool bev_Start_Renderer( const uint32_t width, const uint32_t height );
static void bev_Stop_Renderer( void );
static void bev_Redraw( uint32_t	index );

static QCarCamRet_e	bev_Internal_QCarCamInitialize( void );
static QCarCamRet_e bev_Internal_QCarCamQueryInputs( QCarCamInput_t **ppinputs, uint32_t *pnum_inputs, uint32_t *pret_size );
static QCarCamRet_e	bev_Internal_QCarCamQueryInputModes( 
					S_BEV_QCARCAMMOPEN_INPUT_DATA *pdata, const int inputType, const uint32_t width, const uint32_t height, const uint32_t size, QCarCamInput_t *pinputs, const char *kind );
static QCarCamRet_e	bev_Internal_QCarCamOpen( S_BEV_QCARCAMMOPEN_INPUT_DATA input_data );
static QCarCamRet_e	bev_Internal_QCarCamSetBuffers( const QCarCamHndl_t input_hndl, const uint32_t input_width, const uint32_t input_height, const QCarCamColorFmt_e color_fmt);
static QCarCamRet_e	bev_Internal_QCarCamReserve( const QCarCamHndl_t input_hndl );
static QCarCamRet_e	bev_Internal_QCarCamRegisterEventCallback( const QCarCamHndl_t input_hndl );
static QCarCamRet_e	bev_Internal_QCarCamSetParam( const QCarCamHndl_t input_hndl );
static QCarCamRet_e	bev_Internal_QCarCamStart( const QCarCamHndl_t input_hndl );
static QCarCamRet_e	bev_Internal_QCarCamGetFrame( const QCarCamHndl_t input_hndl, QCarCamFrameInfo_t *pframeInfo );
static QCarCamRet_e	bev_Internal_QCarCamReleaseFrame( const QCarCamHndl_t input_hndl, QCarCamFrameInfo_t *pframeInfo );
static QCarCamRet_e	bev_Internal_QCarCamStop( const QCarCamHndl_t input_hndl );
static QCarCamRet_e	bev_Internal_QCarCamUnregisterEventCallback( const QCarCamHndl_t input_hndl );
static QCarCamRet_e	bev_Internal_QCarCamRelease( const QCarCamHndl_t input_hndl );
static QCarCamRet_e	bev_Internal_QCarCamClose( const QCarCamHndl_t input_hndl );
static QCarCamRet_e	bev_Internal_QCarCamUninitialize( void );