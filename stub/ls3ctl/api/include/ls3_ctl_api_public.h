#ifndef LS3_CTL_API_PUBLIC_H
#define LS3_CTL_API_PUBLIC_H

#include <com_typedef.h>
#include <utl_rpc_public.h>

#include <sif_mutex.h>

#ifndef LS3_CTL_MAX_CMD_DATA_LENGTH
#define LS3_CTL_MAX_CMD_DATA_LENGTH         512U
#endif

#ifndef LS3_CTL_MAX_EVT_DATA_LENGTH
#define LS3_CTL_MAX_EVT_DATA_LENGTH         512U
#endif

#ifndef LS3_CTL_MAX_PROPERTY_DATA_LENGTH
#define LS3_CTL_MAX_PROPERTY_DATA_LENGTH    512U
#endif

#ifndef LS3_CTL_MAX_FILE_NAME_LENGTH
#define LS3_CTL_MAX_FILE_NAME_LENGTH        64U
#endif

#ifndef LS3_MAX_MHP_TX_HANDLE
#define LS3_MAX_MHP_TX_HANDLE               4
#endif

#ifndef LS3_MAX_MHP_RX_HANDLE
#define LS3_MAX_MHP_RX_HANDLE               4
#endif

#ifndef LS3_CTL_MAX_AllSOURCE_SINK_NUM
#define LS3_CTL_MAX_AllSOURCE_SINK_NUM 200
#endif

#define LS3_CTL_TRANSFER_MEMORY             0U
#define LS3_CTL_TRANSFER_FILE               1U
#define LS3_CTL_TRANSFER_SHARE_MEMORY       2U

#define LS3_CTL_RET_SUCCESS                 0
#define LS3_CTL_RET_ERR_PARAM              -1
#define LS3_CTL_RET_ERR_NOSVC              -2
#define LS3_CTL_RET_OUT_OF_MEMORY          -3
#define LS3_CTL_RET_ERR_ERR                -4
#define LS3_CTL_RET_INITIAL_VALUE          -5
#define LS3_CTL_RET_ERR_SYS_CALL           -6
#define LS3_CTL_RET_ERR_FBLOCK_INST_ID     -7
#define LS3_CTL_RET_ERR_DEVICE_ID          -8

#define LS3_CTL_EVENT_CMD_RCV                      0
#define LS3_CTL_EVENT_ACTIVATECOMP                 1
#define LS3_CTL_EVENT_FRONTAVMASTER_SYSTEM_OK      2
#define LS3_CTL_EVENT_ALLSOURCE_SINK_INFO_GET_COMP 3
#define LS3_CTL_EVENT_SYSTEM_NOTOK                 4
#define LS3_CTL_EVENT_SYSTEM_OK                    5
#define LS3_CTL_EVENT_SYSTEM_INVALID               6
#define LS3_CTL_EVENT_SYSTEM_NEW                   7
#define LS3_CTL_EVENT_REARAVMASTER_SYSTEM_OK       8
#define LS3_CTL_EVENT_ALLSOURCE_SINK_RCV           9
#define LS3_CTL_EVENT_SYSTEM_OK_EXPIRED           10
#define LS3_CTL_EVENT_MHPCMD_RCV                  11
#define LS3_CTL_EVENT_NET_OFF                     12


#define LS3_CTL_FBLOCK_NETBLOCK             1
#define LS3_CTL_FBLOCK_NETWORKMASTER        2
#define LS3_CTL_FBLOCK_CONNECTIONMANAGER    3
#define LS3_CTL_FBLOCK_POWERMASTER          4
#define LS3_CTL_FBLOCK_ALL               0xFF
#define LS3_CTL_INSTID_ALL               0xFF


#define LS3_CTL_SYSTEMSTATE_NOTOK           0U 
#define LS3_CTL_SYSTEMSTATE_OK              1U


#define LS3_CTL_FRONTAVMASTER_SYSTEM_NOTOK  0
#define LS3_CTL_FRONTAVMASTER_SYSTEM_OK     1

#define LS3_CTL_ALLSOURCE_SINK_INFO_NOTOK   0
#define LS3_CTL_ALLSOURCE_SINK_INFO_OK      1

#define LS3_CTL_REARAVMASTER_SYSTEM_OK      1
#define LS3_CTL_REARAVMASTER_SYSTEM_NOTOK   0

#define LS3_CTL_SYSTEM_OK_UNEXPIRED         0
#define LS3_CTL_SYSTEM_OK_EXPIRED           1

#define LS3_CTL_RCV_ALLSOURCEINFO           0
#define LS3_CTL_RCV_ALLSINKINFO             1

#define LS3_CTL_FRONT_SYSTEM                0
#define LS3_CTL_REAR_SYSTEM                 1


#define LS3_CTL_BROADCAST_DEVICEID       0x3C8U
#define LS3_CTL_REGISTRATION_DEVICEID    0xFFFU

#define LS3CTL_MAX_OPEN_NAME_NUM 24
#define LS3CTL_SIF_MUTEX_NUM 8

typedef struct tagLs3CtlApiObj
{
    UtlRpcClientObj  rpc_client_obj;
    UINT32  client_func_id;
    INT32   call_id_cmd;
    INT32   call_id_evt;

    INT8    endpoint_open_name[LS3CTL_MAX_OPEN_NAME_NUM];
    sif_mutex_t     *sif_mutex[LS3CTL_SIF_MUTEX_NUM];
} Ls3CtlApiObj;


typedef struct tagLs3CtlApiDat
{
    UINT16 DeviceID;
    UINT8  FBlockID;
    UINT8  InstID;
    UINT16 FktID;
    UINT8  OPType;
    UINT16 DataType;
    UINT16 Length;
    UINT8  Data[LS3_CTL_MAX_CMD_DATA_LENGTH];
} Ls3CtlApiDat;

typedef struct tagLs3CtlApiDatEx
{
    UINT16 DeviceID;
    UINT8  FBlockID;
    UINT8  InstID;
    UINT16 FktID;
    UINT8  OPType;
    UINT8  RcvType;
    UINT16 DataType;
    UINT16 Length;
    UINT8  Data[LS3_CTL_MAX_CMD_DATA_LENGTH];
} Ls3CtlApiDatEx;

typedef struct tagLs3CtlApiEvtDat
{
    UINT16 Length;
    UINT8  Data[LS3_CTL_MAX_EVT_DATA_LENGTH];
} Ls3CtlApiEvtDat;

typedef struct tagLs3CtlApiSystemState
{
    UINT32 CentralRegistryState;
    UINT32 FrontAVMasterState;
    UINT32 AllSourceSinkInfoState;
    UINT32 AllSourceSinkInfoNum;
    UINT32 RearAVMasterState;
    UINT32 SystemOK_expired;
    UINT8  Rcv_AllSourceSinkInfo[LS3_CTL_MAX_AllSOURCE_SINK_NUM];
} Ls3CtlApiSystemState;

typedef struct tagLs3CtlApiPropertyDat
{
    UINT8  FBlockID;
    UINT8  InstID;
    UINT16 FktID;
    UINT16 DataType;
    UINT16 Length;
    UINT8  Data[LS3_CTL_MAX_PROPERTY_DATA_LENGTH];
} Ls3CtlApiPropertyDat;

typedef struct tagLs3CtlApiMhpObj
{
    UtlRpcClientObj  rpc_client_obj;
    UINT32  client_func_id;
    INT32   call_id_cmd;

    INT8    endpoint_open_name[LS3CTL_MAX_OPEN_NAME_NUM];
    sif_mutex_t     *sif_mutex[LS3CTL_SIF_MUTEX_NUM];
} Ls3CtlApiMhpObj;

typedef struct LS3_Mhp_Rx_Type
{
    UINT8  SegIDRx;
    UINT16 BlockCntRel;
    UINT16 PacketCnt;
    UINT16 DeviceID;
    UINT8  FBlockID;
    UINT8  InstID;
    UINT16 Func_ID;
    UINT8  Operation;
    UINT16 Length;
    UINT8  Data[LS3_CTL_MAX_FILE_NAME_LENGTH];
    UINT8  RxHandle[LS3_MAX_MHP_RX_HANDLE];
} Ls3CtlApiMhpRxDat;



typedef struct LS3_Mhp_Tx_Type
{
    UINT8  Options;
    UINT8  Priority;
    UINT16 MinDelay;
    UINT16 MaxDelay;
    UINT16 DeviceID;
    UINT8  FBlockID;
    UINT8  InstID;
    UINT16 FktID;
    UINT8  OPType;
    UINT16 Length;
    UINT8  Data[LS3_CTL_MAX_FILE_NAME_LENGTH];
    UINT8  TxHandle[LS3_MAX_MHP_TX_HANDLE];
} Ls3CtlApiMhpTxDat;

typedef void (*Ls3CtlApiCmdClbk)( INT32 event, UINT8 FBlockID , UINT8 InstID, Ls3CtlApiDat* data );
typedef void (*Ls3CtlApiCmdClbkEx)( INT32 event, UINT8 FBlockID , UINT8 InstID, Ls3CtlApiDatEx* data );
typedef void (*Ls3CtlApiEvtClbk)( INT32 event,Ls3CtlApiEvtDat *data);
typedef void (*Ls3CtlApiMhpCmdClbk)( INT32 event, UINT8 FBlockID , UINT8 InstID, Ls3CtlApiMhpRxDat* data );


INT32 Ls3CtlApiOpen(Ls3CtlApiObj *pIfObj);
INT32 Ls3CtlApiClose(Ls3CtlApiObj *pIfObj);
INT32 Ls3CtlApiSndCmd(Ls3CtlApiObj *pIfObj,Ls3CtlApiDat * pCmdDat);
INT32 Ls3CtlApiCmdAddCallback (Ls3CtlApiObj *pIfObj,UINT8 FBlockID ,UINT8 InstID ,
                               Ls3CtlApiCmdClbk pClbkFunc);
INT32 Ls3CtlApiCmdDelCallback(Ls3CtlApiObj *pIfObj, UINT8 FBlockID , UINT8 InstID);
INT32 Ls3CtlApiEvtAddCallback(Ls3CtlApiObj *pIfObj,Ls3CtlApiEvtClbk pClbkFunc);
INT32 Ls3CtlApiEvtDelCallback(Ls3CtlApiObj *pIfObj);
INT32 Ls3CtlApiGetSystemState (Ls3CtlApiObj *pIfObj, Ls3CtlApiSystemState *pSystemState);
INT32 Ls3CtlApiSetProperty(Ls3CtlApiObj *pIfObj,Ls3CtlApiPropertyDat * pPropertyDat);
INT32 Ls3CtlApiGetProperty(Ls3CtlApiObj *pIfObj,Ls3CtlApiPropertyDat * pPropertyDat);


INT32 Ls3CtlApiMhpOpen(Ls3CtlApiMhpObj *pIfObj);
INT32 Ls3CtlApiMhpClose(Ls3CtlApiMhpObj *pIfObj);
INT32 Ls3CtlApiMhpSndCmd (Ls3CtlApiMhpObj *pIfObj,Ls3CtlApiMhpTxDat * pMhpCmdDat);

INT32 Ls3CtlApiMhpCmdAddCallback (Ls3CtlApiMhpObj *pIfObj,UINT8 FBlockID , UINT8 InstID ,
                                  Ls3CtlApiMhpCmdClbk pClbkFunc);
INT32 Ls3CtlApiMhpCmdDelCallback(Ls3CtlApiMhpObj *pIfObj, UINT8 FBlockID , UINT8 InstID);
INT32 Ls3CtlApiCmdAddCallbackEx (Ls3CtlApiObj *pIfObj,UINT8 FBlockID ,UINT8 InstID ,
                               Ls3CtlApiCmdClbkEx pClbkFuncEx);
INT32 Ls3CtlApiCmdDelCallbackEx(Ls3CtlApiObj *pIfObj, UINT8 FBlockID , UINT8 InstID);
INT32 Ls3CtlApiSetBusReset(Ls3CtlApiObj *pIfObj);


#endif
