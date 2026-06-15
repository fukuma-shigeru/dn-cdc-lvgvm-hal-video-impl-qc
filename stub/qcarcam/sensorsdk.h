/**
 * @file sensorsdk.h
 *
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef SENSORSDK_H
#define SENSORSDK_H

#include <stdint.h>
#include <stdbool.h>
#include "qcarcam_types.h"

/** @addtogroup SensorLib_Constants
@{ */

/** @brief Maximum number of sensor modes */
#define SENSORLIB_MAX_SUBDEV_MODES              16U

/** @brief Maximum number of sensor sources per mode */
#define SENSORLIB_MAX_SUBDEV_SOURCES            16U

/** @brief Maximum number of subdevices per device */
#define SENSORLIB_MAX_NUM_DEVICE                8U

/** @brief Maximum number of subdevices per device */
#define SENSORLIB_MAX_SUBDEV_PER_DEVICE         4U

/** @brief Maximum number of internal GPIOs per device */
#define SENSORLIB_MAX_DEVICE_INTERNAL_GPIO      3U

/** @brief Maximum number of SOC GPIOs per device */
#define SENSORLIB_MAX_NUM_GPIO_PER_DEVICE       24U

/** @brief Maximum number of SOC GPIO interrupts per device */
#define SENSORLIB_MAX_NUM_INTR_PER_DEVICE       24U

/** @brief Maximum number of I2C ports per device */
#define SENSORLIB_MAX_NUM_I2C_PER_DEVICE        3U

/** @brief Maximum number of SOC CSIs per device */
#define SENSORLIB_MAX_NUM_CSI                   4U

/** @brief Maxmimum number of FSYNC Groups per device */
#define SENSORLIB_MAX_FSYNC_GROUP_NUM           4U


/** @brief CSI data type for embedded data. */
#define CSI_DT_EMBED_DATA        0x12
/** @brief CSI data type for reserved data. */
#define CSI_DT_RESERVED_DATA_0   0x13
/** @brief CSI data type for YUV422 8-bit data. */
#define CSI_DT_YUV422_8          0x1E
/** @brief CSI data type for YUV422 10-bit data. */
#define CSI_DT_YUV422_10         0x1F
/** @brief CSI data type for RGB888 data. */
#define CSI_DT_RGB888            0x24
/** @brief CSI data type for Raw 8-bit data. */
#define CSI_DT_RAW8              0x2A
/** @brief CSI data type for Raw 10-bit data. */
#define CSI_DT_RAW10             0x2B
/** @brief CSI data type for Raw 12-bit data. */
#define CSI_DT_RAW12             0x2C
/** @brief CSI data type for Raw 14-bit data. */
#define CSI_DT_RAW14             0x2D
/** @brief CSI data type for Raw 16-bit data. */
#define CSI_DT_RAW16             0x2E
/** @brief CSI data type for Raw 20-bit data. */
#define CSI_DT_RAW20             0x2F
/** @brief CSI data type for embedded data. */
#define CSI_DT_EMBED_REAR_DATA        0x35

/** @} */ /* end_addtogroup SensorLib_Constants */

/** @addtogroup SensorLib_Datatypes
@{ */

/** @brief Sensor Lib handle type */
typedef void* SensorLibHndl_t;

/** @brief Sensor lib interrupt callback prototype
 *
 * @param pHndl   Sensor library handle
 * @param pData   Private data that was registered using #CameraGpioIntrSetup_t
 */
typedef void (*SensorIntrCallback_t)(SensorLibHndl_t pHndl, void* pData);


/** @brief SensorLib and SendorPlatform API return code definition. */
typedef enum
{
    SENSOR_RET_SUCCESS               = 0, /**< API return type for success. */
    SENSOR_RET_FAILURE               = 1, /**< API return type for any general/I2C failure. */
    SENSOR_RET_INVALID               = 2, /**< API return type for invalid operation or state. */
    SENSOR_RET_BAD_PARAM             = 3, /**< API return type if bad parameter(s). */
    SENSOR_RET_UNKNOWN_REV           = 4, /**< API return type if hardware revision is unknown. */
    SENSOR_RET_UNSUPPORTED_REV       = 5, /**< API return type if hardware revision is unsupported. */
    SENSOR_RET_ERROR_I2C_TRANSACTION = 6, /**< API return type if a successful I2C write value
                                                  did not match its corresponding I2C read value
                                                  (i.e., read-back failure). */
    SENSOR_RET_UNSUPPORTED_PARAM     = 7, /**< API return type if unsupported parameter */
    SENSOR_RET_LAST    = 0x7FFFFFFF       /**< Last or invalid return type. */
} SensorRet_e;


/** @brief CSI link information */
typedef struct {
    uint8_t csiIdx; /**< CSI index */
    uint8_t vc;     /**< CSI virtual channel */
    uint8_t dt;     /**< CSI data type */
    uint8_t dtId;   /**< CSI data type mapped to unique 2bit ID */
    uint8_t secureDomain; /**< secure domain for this stream */
} SensorLibSrcCsiInfo_t;

/** @brief Image resolution and crop information definition. */
typedef struct
{
    uint32_t    width;          /**< Image width in pixels. */
    uint32_t    height;         /**< Image height in pixels. */
    uint32_t    widthCrop;      /**< Image cropped width in pixels. */
    uint32_t    heightCrop;     /**< Image cropped height in pixels. */
    uint32_t    xOffsetCrop;    /**< X axis crop offset in pixels. */
    uint32_t    yOffsetCrop;    /**< Y axis crop offset in pixels. */
} SensorLibImageResInfo_t;

/** @brief Defines power saving modes */
typedef enum
{
    /**< no powerSave mode */
    SENSOR_POWERSAVE_MODE_NONE = 0,
    /**< low power state where devices are put to sleep*/
    SENSOR_POWERSAVE_MODE_SLEEP,
    /**< turn off power to device and subdevice, in this mode
    all gpio's including intr pins would be pulled down */    
    SENSOR_POWERSAVE_MODE_POWEROFF,
    /**< only turn off power to device but keep subdevice power on,
    in this mode all gpio's including intr pins would be pulled down */
    SENSOR_POWERSAVE_MODE_DEVICE_OFF,
} SensorLibPowerSaveMode_e;

/** @brief defines if it is interlaced content and which API should be used */
typedef enum
{
    SENSOR_INTERLACED_NONE = 0,            /**< not interlaced field */
    SENSOR_INTERLACED_QUERY_FIELD_TYPE,    /**< query command for field type */
    SENSOR_INTERLACED_PROCESS_FIELD_TYPE,  /**< process frame data for field type */
} SensorLibQueryInterlaced_e;

/** @brief sensor stream source type */
typedef enum
{
    SENSOR_SOURCE_TYPE_IMAGE,       /**< Image data stream */
    SENSOR_SOURCE_TYPE_METADATA,    /**< Metadata stream */
    SENSOR_SOURCE_TYPE_METADATA_HEADER,   /**< Metadata header stream */
    SENSOR_SOURCE_TYPE_METADATA_FOOTER,   /**< Metadata footer stream */
    SENSOR_SOURCE_TYPE_METADATA_STATS,    /**< Metadata stats stream */
    SENSOR_SOURCE_TYPE_TEST_PATTERN,    /**< Test Pattern stream */
} SensorLibStreamType_e;

/** @brief describe source stream uniquely identified by srcId */
typedef struct
{
    uint8_t srcId;              /**< source ID */
    SensorLibStreamType_e type; /**< stream type */
    QCarCamColorFmt_e fmt;      /**< QCarCam format to represent content */
    float fps;                           /**< Frame rate in frames per second */
    SensorLibImageResInfo_t resolution;  /**< Resolution */
    SensorLibSrcCsiInfo_t   csiInfo;     /**< csi information */
    SensorLibQueryInterlaced_e interlaced; /**< interlaced type */
} SensorLibSrc_t;

/** @brief describe a mode
    A mode may have multiple sources each with their own resolution
 */
typedef struct
{
    uint32_t numSources;            /**< number of available source streams */
    SensorLibSrc_t  sources[SENSORLIB_MAX_SUBDEV_SOURCES]; /**< details of available source streams */
} SensorLibMode_t;

/** @brief State of a device/subdevice */
typedef enum
{
    SENSOR_STATE_UNUSED,    /**< unused device/subdevice */
    SENSOR_STATE_UNKNOWN,   /**< inital state of device/subdevice */
    SENSOR_STATE_DETECTED,  /**< device/subdevice is detected but not yet initialized */
    SENSOR_STATE_READY,     /**< device/subdevice is initialized and ready to go */
    SENSOR_STATE_ERROR,     /**< device/subdevice in error state */
    SENSOR_STATE_STREAMING  /**< device/subdevice in streaming state */
} SensorLibDeviceState_e;

/** @brief Sensor lib subdevice enumeration */
typedef struct
{
    uint8_t         devId;          /**< Sensor lib device ID */
    uint8_t         subdevId;       /**< subdevice ID */
    uint32_t        numModes;       /**< number of available modes */
    SensorLibMode_t modes[SENSORLIB_MAX_SUBDEV_MODES]; /**< array of modes */
    SensorLibDeviceState_e state;   /**< state of subdevice */
} SensorLibSubDevice_t;

/** @brief Signal state for the lock status definition. */
typedef enum
{
    SENSOR_SIGNAL_LOCKED = 0,          /**< Lock obtained signal state. */
    SENSOR_SIGNAL_LOST,                /**< Lock released signal state. */
    SENSOR_SIGNAL_MAX    = 0xFFFFFFFF  /**< Last or invalid signal state. */
} SignalLockStatus_e;


/** @brief Device power command definition. */
typedef enum
{
    SENSORLIB_PWRCMD_POWER_OFF              = 0,    /**< Device power OFF command. */
    SENSORLIB_PWRCMD_POWER_ON               = 1,    /**< Device power ON command. */
    SENSORLIB_PWRCMD_POWER_SUSPEND          = 2,    /**< Device power suspend command (sleep or low power mode). */
    SENSORLIB_PWRCMD_POWER_RESUME           = 3,    /**< Device power resume from suspend command. */
    SENSORLIB_PWRCMD_POWER_SUSPEND_SUBDEV   = 4,    /**< Device power suspend command subdev level. */
    SENSORLIB_PWRCMD_POWER_RESUME_SUBDEV    = 5,    /**< Device power resume command subdev level. */
    SENSORLIB_PWRCMD_POWER_MAX              = 0xFF, /**< Last or invalid device power command. */
} SensorLibPowerCmd_e;

/** @brief Device power control definition. */
typedef struct
{
    SensorLibPowerCmd_e cmd;       /**< Power command. */
    uint8_t             devId;     /**< Device Id */
    uint8_t             subdevId;  /**< Subdevice Id */
} SensorLibPowerCtrl_t;

/** @brief Device stream command definition. */
typedef enum
{
    SENSORLIB_STREAMCMD_STOP  = 0,      /**< Device stream stop command. */
    SENSORLIB_STREAMCMD_START = 1,      /**< Device stream start command. */
    SENSORLIB_STREAMCMD_MAX   = 0xFF,   /**< Last or invalid stream command. */
} SensorLibStreamCmd_e;

/** @brief Device stream control definition. */
typedef struct
{
    SensorLibStreamCmd_e cmd;       /**< Power command. */
    uint8_t              devId;     /**< Device Id */
    uint8_t              subdevId;  /**< Subdevice Id */
} SensorLibStreamCtrl_t;

/** @brief HotPlug device definition. */
typedef struct
{
    uint8_t devId;    /**< Device Id */
    uint8_t subdevId; /**< Subdevice Id */
} SensorLibHotPlugDetect_t;

/** @brief Device enumeration definition. */
typedef struct
{
    /** state of each device */
    SensorLibDeviceState_e state[SENSORLIB_MAX_NUM_DEVICE];
} SensorLibEnumDevices_t;

/** @brief Device enumeration definition. */
typedef struct
{
    uint8_t devId;              /**< device Id */
    uint8_t numSubDevices;      /**< number of subdevices */
    SensorLibSubDevice_t subDevices[SENSORLIB_MAX_SUBDEV_PER_DEVICE];   /**< subdevice information */
} SensorLibEnumSubDevices_t;

/** @brief Device set subdevice mode. */
typedef struct
{
    uint8_t              devId;     /**< Device Id */
    uint8_t              subdevId;  /**< Subdevice Id */
    uint32_t             modeIdx;   /**< subdevice mode index */
} SensorLibSubdeviceModeCtrl_t;

/** @brief Device set QCarCam param. */
typedef struct
{
    uint8_t              devId;     /**< Device Id */
    uint8_t              subdevId;  /**< Subdevice Id */
    uint8_t              srcId;     /**< Source stream Id */
    QCarCamParamType_e   paramId;   /**< QCarCam parameter ID */
    const void*          pParam;    /**< param payload */
    uint32_t             size;      /**< size of pParam */
} SensorLibQCarCamSetParam_t;

/** @brief Device get QCarCam param. */
typedef struct
{
    uint8_t              devId;     /**< Device Id */
    uint8_t              subdevId;  /**< Subdevice Id */
    uint8_t              srcId;     /**< Source stream Id */
    QCarCamParamType_e   paramId;   /**< QCarCam parameter ID */
    void*                pParam;    /**< param payload */
    uint32_t             size;      /**< size of pParam */
} SensorLibQCarCamGetParam_t;

/** @brief MIPI CSI type */
typedef enum
{
    SENSORLIB_MIPI_CSI_DPHY = 0, /**< DPHY transmission */
    SENSORLIB_MIPI_CSI_CPHY,     /**< CPHY transmission */
} SensorLibMipiCsi_e;

/** @brief CSI parameters */
typedef struct
{
    uint8_t   devId;            /**< device id */
    uint8_t   csiIdx;           /**< CSI index */
    uint8_t   laneCnt;          /**< number of CSI lanes used */
    uint8_t   settleCnt;        /**< Settle count in ns */
    uint16_t  laneMask;         /**< Mask of CSI lanes used */
    SensorLibMipiCsi_e  csiType; /**< MIPI CSI type */
    uint8_t   isComboMode;      /**< CSI combo mode */
    uint8_t   isVcx;            /**< VC Extension enabled */
    uint32_t  mipiRateMhz;      /**< MIPI TX rate per lane in MHz */
    uint8_t   firstFramesSkip;  /**< Number of frames to skip on start of transmission */
} SensorLibCsiTxParams_t;


/** @brief Process frame command */
typedef struct
{
    uint8_t         devId;     /**< device id */
    uint8_t         subdevId;  /**< subdevice id */
    uint8_t         srcId;     /**< source stream id */
    uint64_t        hStream;   /**< stream handle */
    const void*     pBuffer;   /**< frame buffer virtual address */
    const QCarCamFrameInfo_t*  pFrameInfo; /**< frame information */
} SensorLibProcessFrame_t;

/** @brief Query interlaced field command */
typedef struct
{
    uint8_t         devId;     /**< device id */
    uint8_t         subdevId;  /**< subdevice id */
    uint8_t         srcId;     /**< source stream id */

    QCarCamInterlaceField_e fieldType;  /**< interlaced field type */
    uint64_t        timestamp;   /**< stream handle */
} SensorLibQueryInterlacedField_t;


/** @brief Payload to notify sensor library which stream is going
 * through which CSID core. This is relevant in case the sensor library
 * wishes to sync I2C or framesync start to a particular stream. */
typedef struct
{
    uint8_t         devId;     /**< device id */
    uint8_t         subdevId;  /**< subdevice id */
    uint8_t         srcId;     /**< source stream id */

    uint16_t        csid;      /**< CSID core */
}SensorLibNotifyCsiInfo_t;

/** @brief Sensor IFE Data */
typedef struct
{
    uint8_t         devId;     /**< device id */
    uint8_t         subdevId;  /**< subdevice id */
    uint8_t         vc;        /**< source stream id */
    uint8_t         dt;        /**< CSI data type */
    uint8_t         dtId;      /**< CSI data type mapped to unique 2bit ID */

    uint16_t        csid;      /**< CSID core */
    void*           hDeviceCtx; /**< Handle of device that created this mapping */
}SensorLibNotifyCsiInfoWithVcDt_t;

/** @brief Notify sensor lib to clear IFE mappings for a released device */
typedef struct
{
    void*           hDeviceCtx; /**< Device whose mappings have been cleared */
} SensorLibClearCsiInfo_t;

/** @brief Sensor Lib event definition. */
typedef enum
{
    SENSORLIB_EVENT_STATUS_CHANGE = 0,      /**< Event on signal signal change to lost or valid */
    SENSORLIB_EVENT_MODE_CHANGE,            /**< Event on mode change of a subdevice */
    SENSORLIB_EVENT_FATAL_ERROR,            /**< Event to indicate a fatal error */
    SENSORLIB_EVENT_DEVICE_DETECTED,        /**< Event on device detected */
    SENSORLIB_EVENT_SUBDEVICE_DETECTED,     /**< Event on subdevice detected */
    SENSORLIB_EVENT_SUBDEVICE_STATE_CHANGE, /**< Event to report state change on subdevice */
    SENSORLIB_EVENT_DEVICE_FUSA_ERROR,      /**< Event to report device FUSA safety mechanism error */
    SENSORLIB_EVENT_SUBDEV_FUSA_ERROR,      /**< Event to report subdevice FUSA safety mechanism error */
    SENSORLIB_EVENT_SENSOR_FRAME_BATCH_END, /**< Event to notify sensor a frame batch finished */
    SENSORLIB_EVENT_VENDOR = 255,           /**< Vendor defined event */
} SensorLibEvent_e;

/**
 * @brief Payload for specific error codes
 */
typedef struct
{
    uint32_t errorCode;      /**< Error code ID */
}SensorLibFUSA_t;

/**
 * @brief Sensor event callback payload
 *
 *     EVENT ID                         |    Payload
 * -------------------------------------|--------------
 * SENSORLIB_EVENT_STATUS_CHANGE        | #SignalLockStatus_e
 * SENSORLIB_EVENT_MODE_CHANGE          | u32Data
 * SENSORLIB_EVENT_FATAL_ERROR          | u32Data
 * SENSORLIB_EVENT_DEVICE_DETECTED      |
 * SENSORLIB_EVENT_SUBDEVICE_DETECTED   | #SensorLibSubDevice_t
 * SENSORLIB_EVENT_VENDOR               | #QCarCamVendorParam_t
 * SENSORLIB_EVENT_SENSOR_FUSA_ERROR    | #SensorLibFUSA_t
 * SENSORLIB_EVENT_DESER_FUSA_ERROR     | #SensorLibFUSA_t
 * SENSORLIB_EVENT_SENSOR_FRAME_BATCH_END |
 */
typedef struct
{
    uint8_t devId;          /**< Sensorlib device ID */
    uint8_t subdevId;       /**< subdevice ID */
    uint64_t streamHndl;    /**< stream handle */

    union
    {
        uint32_t             u32Data;       /**< unsigned int data */
        SignalLockStatus_e   lockStatus;    /**< lock status change */
        QCarCamVendorParam_t vendorData;    /**< vendor data */
        SensorLibSubDevice_t subDevice;     /**< subdevice information */
        SensorLibFUSA_t      fusaError;     /**< fusa error information */
    };
} SensorLibEventPayload_t;

/** @brief Thread priority */
typedef enum
{
    SENSORLIB_THREAD_PRIORITY_CALLER,
    SENSORLIB_THREAD_PRIORITY_BACKGROUND,
    SENSORLIB_THREAD_PRIORITY_DEFAULT,
    SENSORLIB_THREAD_PRIORITY_NORMAL,
    SENSORLIB_THREAD_PRIORITY_HIGH,
    SENSORLIB_THREAD_PRIORITY_REALTIME,
    SENSORLIB_THREAD_PRIORITY_IST,
    SENSORLIB_THREAD_PRIORITY_MAX
} SensorLibThreadPriority_e;

/** @brief CSI command type */
typedef enum
{
    SENSORLIB_CSI_COMMAND_HALT_AND_WAIT,
    SENSORLIB_CSI_COMMAND_RESUME,
    SENSORLIB_CSI_COMMAND_MAX
} SensorLibCSICommand_e;

/******* PLATFORM ******/

/** @brief GPIO types */
typedef enum
{
    CAMERA_GPIO_INVALID         = 0,
    CAMERA_GPIO_RESET           = 1,
    CAMERA_GPIO_FSYNC           = 2,
    CAMERA_GPIO_ERROR           = 4,
    CAMERA_GPIO_LOCK            = 5,
    CAMERA_GPIO_CUSTOM1         = 6,
    CAMERA_GPIO_CUSTOM2         = 7,
    CAMERA_GPIO_CUSTOM3         = 8,
    CAMERA_GPIO_CUSTOM4         = 9,
    CAMERA_GPIO_CUSTOM5         = 10,
    CAMERA_GPIO_CUSTOM6         = 11,
    CAMERA_GPIO_CUSTOM7         = 12,
    CAMERA_GPIO_CUSTOM8         = 13,
    CAMERA_GPIO_CUSTOM9         = 14,
    CAMERA_GPIO_CUSTOM10        = 15,
    CAMERA_GPIO_CUSTOM11        = 16,
    CAMERA_GPIO_CUSTOM12        = 17,
    CAMERA_GPIO_MAX             = 0x7FFFFFFF
} CameraGpioType_e;

/** @brief GPIO pin value */
typedef enum
{
    CAMERA_GPIO_LOW  = 0,    /**< GPIO pin value low */
    CAMERA_GPIO_HIGH = 1,   /**< GPIO pin value high */
} CameraGpioValue_e;

/** GPIO Interrupt enumeration */
typedef enum
{
    CAMERA_GPIO_INTR_1 = 0,
    CAMERA_GPIO_INTR_2 = 1,
    CAMERA_GPIO_INTR_3 = 2,
    CAMERA_GPIO_INTR_4 = 3,
    CAMERA_GPIO_INTR_5 = 4,
    CAMERA_GPIO_INTR_6 = 5,
    CAMERA_GPIO_INTR_MAX = 0x7FFFFFFF
} CameraGpioIntrType_e;

/**
 * @brief Defines GPIO interrupt trigger
 */
typedef enum {
    CAMERA_GPIO_TRIGGER_NONE,
    CAMERA_GPIO_TRIGGER_RISING,     /**< trigger on rising edge */
    CAMERA_GPIO_TRIGGER_FALLING,    /**< trigger on falling edge */
    CAMERA_GPIO_TRIGGER_EDGE,       /**< trigger on either rising or falling edge */
    CAMERA_GPIO_TRIGGER_LEVEL_LOW,  /**< trigger on level low */
    CAMERA_GPIO_TRIGGER_LEVEL_HIGH  /**< trigger on level high */
} CameraGpioTrigger_e;

/** @brief I2C Mode */
typedef enum
{
    CAMERA_I2C_MODE_STANDARD,   /**< 100KHz */
    CAMERA_I2C_MODE_FAST,       /**< 400KHz */
    CAMERA_I2C_MODE_CUSTOM,     /**< Custom mode definition for platform */
    CAMERA_I2C_MODE_FAST_PLUS,  /**< 1000KHz */
    CAMERA_I2C_MODE_MAX
} CameraI2CMode_e;

/** @brief Data size of I2C data */
typedef enum
{
    CAMERA_I2C_BYTE_DATA = 1,
    CAMERA_I2C_WORD_DATA,
    CAMERA_I2C_3BYTE_DATA,
    CAMERA_I2C_DWORD_DATA,
    CAMERA_I2C_DATA_TYPE_MAX
}CameraI2CDataType_e;

/** @brief Endianess of I2C data */
typedef enum
{
    CAMERA_I2C_ENDIAN_LITTLE = 0,   /**< Data is little endian */
    CAMERA_I2C_ENDIAN_BIG    = 1,   /**< Data is big endian */
}CameraI2CEndian_e;


/** @brief I2C device definition. */
typedef  struct
{
    uint8_t    devId;               /**< Sensor lib device ID */
    uint8_t    i2cIdx;              /**< I2C Port Index. */
    uint8_t    i2cAddr;             /**< I2C slave address */
    CameraI2CMode_e clockMode;      /**< I2C device clock mode. */
} CameraI2CDevice_t;

/** @brief I2C settings definition. */
typedef struct
{
    CameraI2CDataType_e     regAddrType;    /**< I2C device register type. */
    CameraI2CDataType_e     dataType;       /**< I2C device data type. */
    CameraI2CEndian_e       dataEndian;     /**< I2C data endianess */
} CameraI2CSettings_t;

/** @brief I2C slave device register read and write definition. */
typedef struct
{
    uint32_t   regAddr;     /**< Register address. */
    uint32_t   regData;     /**< Register data. */
    uint32_t   delay;       /**< Delay time in micro seconds. A value of 0 will skip the delay. */
} CameraI2CReg_t;

/** @brief I2C slave device register data and settings */
typedef struct
{
    CameraI2CReg_t      regs;           /**< Register sequence */
    CameraI2CSettings_t i2cSetting;     /**< I2C Settings */
} CameraI2CRegSetting_t;

/** @brief I2C transaction type definition. */
typedef enum
{
    CAMERA_I2C_READ_BULK,                 /**< Custom read - no stop bit required. */
    CAMERA_I2C_WRITE_BULK,                /**< Custom write - no stop bit. */
    CAMERA_I2C_WRITE_THEN_READ_BULK,      /**< Custom read after write - no stop bit. */
    CAMERA_I2C_TRANSACT_MAX = 0X7FFFFFFF  /**< Last or invalid I2C transaction type. */
} CameraI2CTransactType_e;

/** @brief I2C slave device register read and write definition. */
typedef struct
{
    CameraI2CSettings_t i2cSetting;     /**< I2C Settings */
    uint32_t    regAddr;                /**< Register address. */
    uint8_t*    regData;                /**< Register data. */
    uint8_t     regDataStore;           /**< Register data store size. */
    uint32_t    size;                   /**< Number of entries of regData */
    uint8_t     execPendingI2cTransact; /**< Flushes all operations */
} CameraI2CBurstSetting_t;

/** @brief I2C transaction command */
typedef struct
{
    CameraI2CTransactType_e  transactType;  /**< transaction type */
    CameraI2CBurstSetting_t  readOp;        /**< burst read operation */
    CameraI2CBurstSetting_t  writeOp;       /**< burst write operation */
} CameraI2CTransact_t;

/** @brief I2C sync operation trigger type */
typedef enum
{
    CAMERA_I2C_SYNC_TRIGGER_DEFAULT = 0, /**< triggered by default */
    CAMERA_I2C_SYNC_TRIGGER_CSI,         /**< triggered by CSI frame */
    CAMERA_I2C_SYNC_TRIGGER_GPIO,        /**< triggered by GPIO */
    CAMERA_I2C_SYNC_TRIGGER_MAX
} CameraI2CSyncTrigger_e;

/** @brief Sync command triggered by CSI */
typedef struct
{
    uint16_t csid;  /**< CSID core */
    uint8_t  vc;    /**< CSI virtual channel */
    uint8_t  dtId;  /**< CSI data type mapped to unique 2bit ID */
    uint16_t line;  /**< line to trigger */
    uint32_t delay; /**< Delay time in micro seconds */
} CameraI2CSyncTriggerCsi_t;

/** @brief Sync command triggered by GPIO */
typedef struct
{
    CameraGpioType_e    gpio;           /**< GPIO type that will sync against */
    CameraGpioTrigger_e gpioTrigger;    /**< GPIO trigger type */
    uint32_t delay;                     /**< Delay time in micro seconds */
} CameraI2CSyncTriggerGpio_t;

/** @brief I2C sync configuration */
typedef struct
{
    uint8_t devId;      /**< Sensor lib device ID */
    uint8_t i2cIdx;     /**< I2C Port Index. */

    CameraI2CSyncTrigger_e  triggerType;    /**< trigger type */

    union
    {
        CameraI2CSyncTriggerCsi_t   csiTrigger;     /**< CSI trigger for sync */
        CameraI2CSyncTriggerGpio_t  gpioTrigger;    /**< GPIO trigger for sync */
    };
} CameraI2CDeviceSyncCfg_t;

/** @brief I2C sync operation */
typedef struct
{
    uint8_t             i2cAddr;    /**< I2C slave address */
    CameraI2CMode_e     clockMode;  /**< I2C device clock mode. */
    CameraI2CSettings_t i2cSetting; /**< I2C settings */
    CameraI2CReg_t*     pRegs;      /**< Register sequence */
    uint32_t            size;       /**< size of register sequence */
} CameraI2CSyncSequence_t;


/** @brief This enum describes I2C port types */
typedef enum {
    CAMERA_I2C_TYPE_CCI = 0,  /** Camera CCI bus */
    CAMERA_I2C_TYPE_I2C,      /** Generic I2C bus */
} CameraI2CType_t;

/** @brief This structure describes I2C port used by the camera */
typedef struct {
    CameraI2CType_t i2ctype;    /**< CCI or I2C port */
    uint32_t deviceId;          /**< CCI device number */
    uint32_t portId;            /**< I2C port number */
} CameraI2CPort_t;

/** @brief GPIO device information */
typedef struct
{
    CameraGpioType_e    gpioType;   /**< GPIO type */
    uint32_t            num;        /**< GPIO pin number */
    uint32_t            config;     /**< GPIO pin configuration */
    uint32_t            configMask; /**< GPIO config mask */
} CameraGpioInfo_t;

/** @brief Power voltage regulator type */
typedef enum
{
    CAMERA_POWER_INVALID = 0,
    CAMERA_POWER_VANA,
    CAMERA_POWER_VDIG,
    CAMERA_POWER_VIO,
    CAMERA_POWER_VREG_CUSTOM1,
    CAMERA_POWER_VREG_CUSTOM2,
} CameraPowerVregType_e;

/** @brief Power Sequence Type */
typedef enum
{
    CAMERA_POWER_SEQ_GPIO,  /**< Selects type as #CameraGpioType_e */
    CAMERA_POWER_SEQ_VREG   /**< Selects type as #CameraPowerVregType_e */
} CameraPowerSeqType_e;

/** @brief Power and GPIO config sequence */
typedef struct
{
    uint8_t  devId;         /**< Sensor lib device ID */
    CameraPowerSeqType_e type;  /**< power configuration */
    uint32_t configType;    /**< config type as #CameraGpioType_e or #CameraPowerVregType_e */
    uint32_t configVal;     /**< configuration value to apply */
    uint32_t delay;         /**< delay in ms after applying the value */
} CameraPowerSequence_t;

/** @brief GPIO command */
typedef struct
{
    uint8_t devId;              /**< Sensor lib device ID */
    CameraGpioType_e gpio;      /**< GPIO type */
    CameraGpioValue_e value;    /**< GPIO value */
} CameraGpioCommand_t;

/** @brief GPIO interrupt setup for specific device interrupt gpio pin
 *
 *  @see #SensorIntrCallback_t
 */
typedef struct
{
    uint8_t              devId;         /**< Sensor lib device ID */
    CameraGpioIntrType_e intr;          /**< device interrupt pin */
    SensorIntrCallback_t pIntrCallback; /**< callback function */
    void * pData;                       /**< private data to be invoked */
} CameraGpioIntrSetup_t;

/** @brief CCI Frame sync setup */
typedef struct
{
    uint8_t             devId;      /**< Sensor lib device ID */
    CameraGpioType_e    gpioId;     /**< GPIO id to be setup  */

    uint64_t            fsyncPeriodNs;      /**< frame sync period in nanoseconds*/
    uint64_t            fsyncHighDutyNs;    /**< frame sync high duty time in nanoseconds*/

    CameraI2CSyncTrigger_e  triggerType;    /**< trigger type */

    union
    {
        CameraI2CSyncTriggerCsi_t   csiTrigger;     /**< CSI trigger for sync */
        CameraI2CSyncTriggerGpio_t  gpioTrigger;    /**< GPIO trigger for sync */
    };

} CameraCCIFrameSyncSetup_t;


/** @brief CCI timer enumerator */
typedef enum
{
    CCI_TIMER_0 = 0,     /**< Corresponds to CCI timer 0 GPIO */
    CCI_TIMER_1,         /**< Corresponds to CCI timer 1 GPIO */
    CCI_TIMER_2,         /**< Corresponds to CCI timer 2 GPIO */
    CCI_TIMER_3,         /**< Corresponds to CCI timer 3 GPIO */
    CCI_TIMER_4,         /**< Corresponds to CCI timer 4 GPIO */
    CCI_TIMER_5,         /**< Corresponds to CCI timer 5 GPIO */
    CCI_TIMER_6,         /**< Corresponds to CCI timer 6 GPIO */
    CCI_TIMER_7,         /**< Corresponds to CCI timer 7 GPIO */
    CCI_TIMER_8,         /**< Corresponds to CCI timer 8 GPIO */
    CCI_TIMER_9,         /**< Corresponds to CCI timer 9 GPIO */
    CCI_TIMER_MAX        /**< MAX CCI timer GPIO */
} CCITimerList_e;

/** @brief structure defining methods to start CCI timer */
typedef enum {
    CCI_TIMER_START_DURING_INIT = 0,  /**< CCI timer started during sensor lib init */
    CCI_TIMER_START_VENDOR_PARAM = 1,  /**< CCI timer started during set vendor param call */
} CameraTimerStartMode_e;

/** @brief structure defined to store CCI timer config info */
typedef struct
{
    CCITimerList_e timerId; /** specifies CCI timer*/
    uint64_t period; /** time of FSYNC cycle in terms of ns*/
    uint64_t highTime; /** time of FSYNC cycle of high signal*/
    uint64_t offset; /** specifies timer offset in ns*/
    CameraTimerStartMode_e startMode; /** 0 is default start, 1 is starting via vendor params*/
} CameraTimer_t;

/** @brief structure defined to store fsync groups config info */
typedef struct
{
    uint8_t gpio; /** deserializer MFP/GPIO used for FSYNC signal forwarding*/
    uint8_t links; /** 4 bit bitmap for LINK 0-3, bit 0 for link 0 to bit 3 for link 3 */
    uint8_t channelId; /** FSYNC transmission channel id to serializer*/
} CameraFsyncGroup_t;

/** @brief enumeration of frame synchronization modes */
typedef enum {
    CAMERA_FSYNC_MODE_FSYNC_OFF = 0,  /* no frame synchronization */
    CAMERA_FSYNC_MODE_CCI_FSYNC = 1,  /* frame synchronization using CCI timer as the source*/
    CAMERA_FSYNC_MODE_DES_FSYNC = 2,  /* frame synchronization using DESERIALIZER timer */
    CAMERA_FSYNC_MODE_GPIO_FSYNC = 3  /* frame synchronization using external GPIO */
} CameraFsyncMode_e;

/** @brief structure defined to fsync config info */
typedef struct
{
    CameraFsyncMode_e mode; /** FSYNC mode. 0 is no FSNC, 1-3 are FSYNC modes*/
    uint8_t numTimers; /** number of timers to start, only valid in CCI FSYNC*/
    uint8_t numGroups; /** number of fsync groups*/
    CameraTimer_t timers[CCI_TIMER_MAX];
    CameraFsyncGroup_t groups[SENSORLIB_MAX_FSYNC_GROUP_NUM];
} CameraFsyncInfo_t;

/** @brief CSI Sync params */
typedef struct
{
    uint8_t     devId;                     /**< Sensor lib device ID */
    uint32_t    *hwBlockIdList;            /**< list of hwBlockIds to wait for sync */
    uint8_t     numBlockIds;               /**< number of entries in hwBlockIdList */
    uint8_t     i2cWriteSync;              /**< whether i2c write sync method was employed */
    uint32_t    csiIdx;                    /**< csi device id */
    SensorLibCSICommand_e     command;  /**< command type */
    SensorLibThreadPriority_e priority; /**< thread priority */
} CameraCSISyncParams_t;

/******* CONFIG ******/

/**
 * @brief Defines type of GPIO interrupt it is.
 */
typedef enum
{
    CAMERA_GPIO_INTR_MODE_NONE, /**< Do not config interrupt at all */
    CAMERA_GPIO_INTR_MODE_POLL, /**< Use polling */
    CAMERA_GPIO_INTR_MODE_TLMM, /**< configure with TLMM Interrupt */
    CAMERA_GPIO_INTR_MODE_PMIC, /**< configure with PMIC Interrupt */
} CameraGpioIntrMode_e;

/**
 * @brief Describes Camera SOC GPIO interrupt configuration
 */
typedef struct
{
    CameraGpioIntrType_e intrType;          /**< gpio interrupt identifier that input driver will refer to */
    uint32_t             gpioPin;           /**< gpio number */
    CameraGpioTrigger_e  triggerType;       /**< trigger type specific to module */
    CameraGpioIntrMode_e intrMode;          /**< how the interrupt based pin to be configured */
} CameraGpioIntrPin_t;

/**
 * @brief This structure defines CSI info used by camera
 */
typedef struct {
    uint8_t  csiId;      /**< CSI port id */
    uint8_t  numLanes;   /**< Number of MIPI CSI physical lanes connected */
    uint16_t laneAssign; /**< CSI Lane mapping configuration where each nibble (4bits), starting from the LSB
                            up to numLanes, assigns the physical lane.
                            The default is 0x3210 (no swaps).
                            For example 0x3120 can be set in case of lane 1 and 2 are swapped
                            */
    uint32_t ifeMap;     /**< Mappings to IFE interfaces where each IFE index is expressed as a nibble (4bits).
                            The IFE priority is set in priority starting from the nibble at the LSB.
                            For instance, 0x31 would map starting with IFE 1 then IFE 3.
                            */
    uint8_t  numIfeMap;  /**< Number of IFE mappings in ifeMap (number of nibbles valid from LSB)
                            If 0, ifeMap is ignored and the IFE mapping will default to csiId */

    uint8_t  forceHSmode;  /**< Force high speed mode for CSIPHY */
} CameraCsiInfo_t;

/** @brief This structure defines subdevice configuration */
typedef struct {
    uint8_t subdevId;    /**< subdevice id */

    uint32_t type;       /**< subdevice type */

    uint8_t slaveI2cAddr;   /**< default i2c address */
    uint8_t serI2cAddr;     /**< default serializer i2c address */
    uint8_t eepromI2cAddr;  /**< default EEPROM i2c address */
    uint8_t pmicI2cAddr;    /**< default PMIC i2c address */

    uint8_t snsrAlias;   /**< sensor alias i2c address */
    uint8_t serAlias;    /**< serializer alias i2c address */
    uint8_t eepromAlias; /**< EEPROM alias i2c address */
    uint8_t pmicAlias;   /**< PMIC alias i2c address */

    uint8_t broadcastI2cAddr; /**< broadcast i2c address */

    uint8_t snsrModeId;  /**< initial sensor mode */

    uint8_t fsyncMode;   /**< fsync mode */
    uint8_t fsyncFreq;   /**< fsync frequency
                              @todo: how does this change with sensor mode change? */

    uint8_t colorSpace;  /**< color space */

    uint8_t csiInfoBitMap;  /**< bit mask of indices of device csiInfo that this subdev will route its output */

    QCarCamSensorOrientation_e orientation; /**< subdevice orientation */

    uint32_t custom[16];    /**< custom settings */

    uint32_t maxNumContexts;   /**< contexts required for handling SoF Event */

} SensorLibSubdevConfig_t;

/** @brief This enum defines I2C Operations */
typedef enum
{
    CAMERA_DEVICE_I2C_WRITE,  /**< I2C Write to device */
    CAMERA_DEVICE_I2C_READ,   /**< I2C Read to device */
}SensorLibI2COperation_t;

/** @brief This enum defines Device Type */
typedef enum
{
    CAMERA_DEVICE_TYPE_SENSOR, /**< Device Type sensor */
    CAMERA_DEVICE_TYPE_EEPROM, /**< Device Type EEPROM */
    CAMERA_DEVICE_TYPE_MAX,    /**< Device Type Invalid */
}SensorLibDeviceType_t;

/** @brief This structure defines I2C Packet info for a device */
typedef struct
{
    uint8_t                  devId;         /**< Device Id */
    uint8_t                  subdevId;      /**< Subdevice Id */
    CameraI2CReg_t*          pRegSet;       /**< I2C Payload */
    uint32_t                 size;          /**< I2C Reg count */
    SensorLibI2COperation_t  i2cOperation;  /**< Type of operation */
    SensorLibDeviceType_t    deviceType;    /**< Device Type packets to be applied */
    CameraI2CDataType_e      regAddrType;   /**< I2C device register type. */
    CameraI2CDataType_e      dataType;      /**< I2C device data type. */
} SensorLibSubmitI2CPacket_t;

/** @brief This structure defines SOF notification for sensorlib */
typedef struct
{
    uint8_t                  devId;      /**< Device Id */
    uint8_t                  subdevId;   /**< Subdevice Id */
    uint64_t                 requestId;  /**< SOF Request Id */
} SensorLibNotifySOF_t;

/** @brief This structure defines eeprom read I2C Packet info for a device */
typedef struct
{
    uint8_t                  devId;         /**< Device Id */
    uint8_t                  subdevId;      /**< Subdevice Id */
    uint32_t                 readAddr;      /**< EEPROM read address. */
    uint8_t*                 pReadData;     /**< EEPROM read data. */
    uint32_t                 size;          /**< EEPROM size. */
} SensorLibEEPROMReadPacket_t;

/**
 * @brief This structure defines the configuration for a sensor lib device
 */
typedef struct
{
    // Device Config
    uint8_t deviceId;       /**< unique device id */

    uint32_t deviceType;    /**< device type (e.g. specific model of deserializer family) */
    uint32_t deviceMode;    /**< operation mode of the device (e.g. can be used for different configuration modes) */

    uint8_t deviceI2cAddr;  /**< Default I2C address of the device */

    uint8_t numSubdevices;  /**< number of sensors connected */
    SensorLibSubdevConfig_t subdevices[SENSORLIB_MAX_SUBDEV_PER_DEVICE]; /**< sensor configuration */

    int8_t deviceInternalGpio[SENSORLIB_MAX_DEVICE_INTERNAL_GPIO]; /**< internal gpio configuration */

    SensorLibPowerSaveMode_e powerSaveMode; /**< power saving mode */

    uint32_t detectThrdId;     /**< device detection thread id */

    /** @brief Number of CSI connections */
    uint32_t numCsi;
    /** @brief CSI connection information */
    CameraCsiInfo_t csiInfo[SENSORLIB_MAX_NUM_CSI];

    /** @brief Number of I2C ports */
    uint32_t numI2cPorts;
    /** @brief Description of I2C ports */
    CameraI2CPort_t i2cPort[SENSORLIB_MAX_NUM_I2C_PER_DEVICE];

    /** @brief Number of SOC GPIOs */
    uint32_t numGpio;
    /** @brief SOC GPIO config */
    CameraGpioInfo_t gpioConfig[SENSORLIB_MAX_NUM_GPIO_PER_DEVICE];

    /** @brief Number of GPIO interrupts */
    uint32_t numGpioIntr;
    /** @brief GPIO Interrupt config*/
    CameraGpioIntrPin_t gpioIntrConfig[SENSORLIB_MAX_NUM_INTR_PER_DEVICE];

    uint32_t custom[16];   /**< custom device settings */

    CameraFsyncInfo_t fsyncInfo; /**< Stores frame sync configuration info */
} SensorLibDeviceConfig_t;

/** @brief List of commands supported */
typedef enum
{
    SENSOR_TIMER_WAIT_TRIGGER = 0,        /**< wait trigger csi/gpio event, should be the 1st command */
    SENSOR_TIMER_DELAY,                   /**< delay between toggle */
    SENSOR_TIMER_TOGGLE_GPIO,             /**< toggle high/low gpio info */
    SENSOR_TIMER_START_FSYNC_GENERATION,  /**< start fsync,  should be the last command */
    SENSOR_TIMER_MAX_NUM_COMMANDS = 32    /**< Maximum number of sensor timer commnds */
} SensorTimerCommandList_e;

/** @brief Info needed to handle command SENSOR_TIMER_WAIT_TRIGGER */
typedef struct
{
    CameraI2CSyncTrigger_e triggerType; /**< trigger type */
    union
    {
        CameraI2CSyncTriggerCsi_t csiTrigger; /**< CSI trigger for sync */
        CameraI2CSyncTriggerGpio_t gpioTrigger; /**< GPIO trigger for sync */
    };
} SensorTimerWaitTriggerCmd_t;

/** @brief Info needed to handle command SENSOR_TIMER_DELAY */
typedef struct
{
    uint64_t delayNs;        /**< Delay in nanoseconds required */
} SensorTimerDelayCmd_t;

/** @brief GPIO related info */
typedef struct
{
    CCITimerList_e cciTimerInfo;    /**< CCI timer info */
    CameraGpioValue_e info;         /**< GPIO toggle info */
} SensorTimerGpioInfo_t;

/** @brief info needed to handle command SENSOR_TIMER_TOGGLE_GPIO */
typedef struct
{
    uint32_t    numOfGpioToToggle;     /**< Number of GPIOs to toggle */
    SensorTimerGpioInfo_t  gpioInformation[CCI_TIMER_MAX]; /**< Each GPIO info */
} SensorTimerToggleGpioCmd_t;

/** @brief info needed to handle command SENSOR_TIMER_START_FSYNC_GENERATION */
typedef struct
{
    bool start;     /**< Start fsync info */
} SensorTimerStartFsyncGeneration_t;


/** @brief contains all command info */
typedef struct
{
    SensorTimerCommandList_e commandType;    /**< Command type to be executed */
    union
    {
        SensorTimerWaitTriggerCmd_t triggerInfo;  /**< Trigger info */
        SensorTimerDelayCmd_t delayInfo;          /**< Delay info */
        SensorTimerToggleGpioCmd_t toggleInfo;    /**< GPIO toggle info */
        SensorTimerStartFsyncGeneration_t startInfo; /**< fsync start info */
    };
} SensorTimerCommandInfo_t;

/** @brief structure passed to CCI driver to execute commands */
typedef struct
{
    uint32_t numCommands; /**< number of commands to be executed */
    SensorTimerCommandInfo_t commandInfo[SENSOR_TIMER_MAX_NUM_COMMANDS]; /**< list of commands */
} SensorTimerCommandSetup_t;

/** @} */ /* end_addtogroup SensorLib_Datatypes */

#endif /* SENSORSDK_H */
