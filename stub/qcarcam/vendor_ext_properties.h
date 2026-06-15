/**
 * @file vendor_ext_properties.h
 *
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef VENDOR_EXT_PROPERTIES
#define VENDOR_EXT_PROPERTIES

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * THE FOLLOWING FILE IS A SAMPLE OF HOW THE QCARCAM VENDOR PARAM COULD BE USED TO ABSTRACT A
 * VENDOR SPECIFIC MESSAGE INTERFACE TO THE SENSOR LIBRARY
 */

#include "sensorsdk.h"

#define VENDOR_EXT_EEPROM_MAX_NUM_DATA      4000U
#define VENDOR_EXT_TEST_MAGIC_VALUE         0xABDC0123U

//HDMI
#define HDMI_KSV_LENGTH 5
#define HDMI_SER_DEV_NUM 25
#define HDMI_IN_RES_LENGTH 4

/// @brief ISP settings
typedef enum
{
    /** @brief tests param interface

    SetParam:
     uint_val will be expected to be set to VENDOR_EXT_TEST_MAGIC_VALUE and checked in driver
     QCarCam VENDOR_EVENT will be triggered

    GetParam:
     uint_val will be expected to be set to VENDOR_EXT_TEST_MAGIC_VALUE and checked in driver
     uint_val will be set to VENDOR_EXT_TEST_MAGIC_VALUE for the reply
    */
    VENDOR_EXT_PROP_TEST,

    VENDOR_EXT_PROP_RESET,
    VENDOR_EXT_PROP_CLEAR_OP_STATUS,
    VENDOR_EXT_PROP_PIXEL_FORMAT,
    VENDOR_EXT_PROP_AUTO_HUE,
    VENDOR_EXT_PROP_AUTO_WHITE_BALANCE,
    VENDOR_EXT_PROP_WHITE_BALANCE_TEMP,
    VENDOR_EXT_PROP_EXPOSURE_COMPENSATION,
    VENDOR_EXT_PROP_BACKLIGHT_COMPENSATION,
    VENDOR_EXT_PROP_SHARPNESS,
    VENDOR_EXT_PROP_HDR_ENABLE,
    VENDOR_EXT_PROP_NOISE_REDUCTION,
    VENDOR_EXT_PROP_FRAME_RATE,
    VENDOR_EXT_PROP_FOCAL_LENGTH_H,
    VENDOR_EXT_PROP_FOCAL_LENGTH_v,
    VENDOR_EXT_PROP_NUM_EMBEDDED_LINES_TOP,
    VENDOR_EXT_PROP_NUM_EMBEDDED_LINES_BOTTOM,
    VENDOR_EXT_PROP_ROI_STATISTICS,
    VENDOR_EXT_PROP_METADATA_MODES,
    VENDOR_EXT_PROP_TEMP_SENSOR_0,
    VENDOR_EXT_PROP_TEMP_SENSOR_1,

    /** @brief resets CSI

     SetParam:
      Issues CSI TX and RX reset sequence. This is applicable for some deserializers to workaround an error when
      enabling both CSI ports. The sequence will stop all CSI RXs, reset the TXs and sensors and start them,
      then finally start the CSI RXs.

     GetParam:
      N/A
     */
    VENDOR_EXT_PROP_CSI_RESET,

    /** @brief Get and override EEPROM data

      Use mem_handle to pass memory buffer handle that will be used by the driver.

      SetParam:
       content from mem_handle will overwrite the cached EEPROM data for the sensor

      GetParam:
       content from mem_handle will be filled with the raw EEPROM data for the sensor
     */
    VENDOR_EXT_PROP_EEPROM,

    /** @brief Start CCI frame sync via sensor library

      Sensor library calls sensor platform api to start frame sync

      SetParam:
      Passes CameraCCIFrameSyncSetup_t info to hwdriver to start frame sync

      GetParam:
       N/A
    */
    VENDOR_EXT_START_CCI_FRAME_SYNC,

    /** @brief Stops CCI frame sync via sensor library

      Sensor library calls sensor platform api to stop frame sync

      SetParam:
      Passes devId and gpio info to hwdriver to stop frame sync

      GetParam:
       N/A
    */
    VENDOR_EXT_STOP_CCI_FRAME_SYNC,

    /** @brief Gets frame sync info via sensor library

      Sensor library populates CameraCCIFrameSyncSetup_t with sensor info

      SetParam:
      Returns populated CameraCCIFrameSyncSetup_t

      GetParam:
       N/A
    */
    VENDOR_EXT_GET_CCI_FRAME_SYNC_INFO,

    /** @brief Starts CCI timers via sensor libraries

      Sensor library configures timer commands set in cameraconfig,
      then starts CCI timers in hwdriver

      SetParam:
      No data needed. Starts CCI timers based on camera config

      GetParam:
       N/A
    */
    VENDOR_EXT_START_CCI_TIMERS,

    /** @brief set i2c to bypass mode */
    VENDOR_EXT_PROP_I2C_CONFIG,

    /** @brief Configure the sensor image region offsets

      SetParam:
       Use sensorRoiOffset to set the x/y origin point offset for the image region.

      @note This command may require the sensor stream to be stopped first
      @note This may result in tuning parameters to be updated for the new image region due to different area of lens.

      GetParam:
       N/A
     */
    VENDOR_EXT_PROP_ROI_OFFSET,

    /** @brief Query sensor stream status information

      SetParam:
       N/A

      GetParam:
       Use sensorStreamInfo value to read back the sensor status and frame counter.
     */
    VENDOR_EXT_PROP_SENSOR_STREAM_INFO,

    /** @brief Error injection testing

      SetParam:
       Use enumerator VENDOR_EXT_INJECTION_TEST

      GetParam:
       N/A
     */
    VENDOR_EXT_ERROR_INJECTION_TEST,

    /** @brief Parse Frame

      SetParam:
       N/A

      GetParam:
       #VendorExtParseFrame_t parseFrame
     */
    VENDOR_EXT_PARSE_FRAME,

    //HDMI
    VENDOR_EXT_PROP_KSV_LIST = 50,
    VENDOR_EXT_PROP_IN_RES,
    VENDOR_EXT_PROP_AUDIO_SAMPLING,
    VENDOR_EXT_PROP_SIGNAL_STATUS,
    VENDOR_EXT_ERROR_IN_RES_CHANGE = 60,
    VENDOR_EXT_ERROR_HDCP_2ND,
    VENDOR_EXT_ERROR_HDMI_I2C,
    VENDOR_EXT_ERROR_HDMI_FAILURE,
    //TEST
    VENDOR_EXT_PROP_TEST_SIGNAL_STATUS = 100,
    VENDOR_EXT_PROP_TEST_VIDEO_FORMAT,
    VENDOR_EXT_PROP_TEST_AUDIO_FORMAT,
    VENDOR_EXT_PROP_TEST_HDCP,


    /** @brief Start sensor stream using vendor param as trigger

      SetParam:
       No data

      GetParam:
       N/A
     */
    VENDOR_EXT_PROP_START_SENSOR_STREAM = 1000,

    VENDOR_EXT_PROP_MAX = 0x7FFFFFFF
} vendor_ext_property_type_t;

/** @brief error injection types enum */
typedef enum
{
    VENDOR_ERROR_INJECTION_I2C_INTEGRITY = 1, /**< inject I2C integrity error for testing */
    VENDOR_ERROR_INJECTION_MAX = 0x7FFFFFFF
} VENDOR_EXT_INJECTION_TEST;

/** @brief This structure defines ROI Offset for sensorlib */
typedef struct
{
    uint32_t                  context;     /**< context that will have offset shifted */
    uint16_t                  x_offset;    /**< ROI Width Offset */
    uint16_t                  y_offset;    /**< ROI Height Offset */
} SensorLibRoiOffset_t;

/** @brief Device get sensor status for dump. */
typedef enum
{
    VENDOR_SENSOR_STATUS_INVALID,
    VENDOR_SENSOR_STATUS_STANDBY,
    VENDOR_SENSOR_STATUS_STREAMING,
} vendor_sensor_status_type_e;

/** @brief Device get sensor streaming param. */
typedef struct
{
    vendor_sensor_status_type_e sensor_status; /**< sensor status */
    uint32_t frame_cnt; /**< sensor frame count */
    uint32_t des_pkt_cnt; /**< packet count of deserializer transmitter*/
} SensorStreamInfo_t;

/** @brief parsed data structure
@note this is initial version and more data will be added
*/
typedef struct
{
    uint32_t errors;  /**< bit mask of errors */
}VendorExtParseData_t;

/** @brief parse frame payload */
typedef struct
{
    uint8_t         srcId;     /**< source stream id */
    uint64_t        memhndl;   /**< memory handle */
    VendorExtParseData_t parsedData; /**< parsed data */
    QCarCamColorFmt_e colorFmt;  /**< source stream color format */
}VendorExtParseFrame_t;

//HDMI
typedef struct
{
    //uint8_t ksv[HDMI_KSV_LENGTH];
    uint8_t max_devs_exceeded;
    uint8_t device_count;
    uint8_t max_cascade_exceeded;
    uint8_t depth;
    uint8_t receiver_id[HDMI_SER_DEV_NUM][HDMI_KSV_LENGTH];
} VenderHdmiKeyList_t;

/// @brief Union to hold the values of different properties.
typedef union
{
    int int_val;
    unsigned int uint_val;
    float float_val;
    CameraCCIFrameSyncSetup_t cciFrameSync;
    SensorLibNotifyCsiInfoWithVcDt_t updateIfeInfo;
    SensorLibRoiOffset_t sensorRoiOffset;
    SensorStreamInfo_t sensorStreamInfo;
    VendorExtParseFrame_t parseFrame;

    //HDMI
    VenderHdmiKeyList_t keyList;
    uint8_t hdmi_in_res[HDMI_IN_RES_LENGTH];
    uint8_t audio_smpl;
} vendor_ext_property_val_t;

/// @brief Structure to hold the property type and its value
typedef struct
{
    vendor_ext_property_type_t type;
    vendor_ext_property_val_t value;
    void* mem_handle;
} vendor_ext_property_t;

#ifdef __cplusplus
}
#endif

#endif /* __VENDOR_EXT_PROPERTIES__ */
