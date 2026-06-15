#ifndef QCARCAM_TYPES_H
#define QCARCAM_TYPES_H

/**************************************************************************************************
@file
    qcarcam_types.h

@brief
    QCarCam API types - QTI Automotive Imaging System Proprietary API

Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
All rights reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

**************************************************************************************************/

/*=================================================================================================
** INCLUDE FILES FOR MODULE
=================================================================================================*/
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*=================================================================================================
** Constant and Macros
=================================================================================================*/

/** @addtogroup qcarcam_constants
@{ */

/** @brief Invalid QCarCam handle type. */
#define QCARCAM_HNDL_INVALID ((QCarCamHndl_t)0)

/** @brief Maximum number of planes in the buffer. */
#define QCARCAM_MAX_NUM_PLANES 3U

/** @brief Maximum number of buffers. */
#define QCARCAM_MAX_NUM_BUFFERS 20U

/** @brief Minimum number of buffers. */
#define QCARCAM_MIN_NUM_BUFFERS 3U

/** @brief Maximum input name length. */
#define QCARCAM_INPUT_NAME_MAX_LEN 80U

/** @brief Macro for a timeout of infinite time. */
#define QCARCAM_TIMEOUT_INIFINITE (-1ULL)

/** @brief Macro for a no wait timeout (i.e., not do wait). */
#define QCARCAM_TIMEOUT_NO_WAIT   (0)

/** @brief Maximum size of gamma table setting.*/
#define QCARCAM_MAX_GAMMA_TABLE 128U

/** @brief Maximum payload size in bytes. */
#define QCARCAM_MAX_PAYLOAD_SIZE 1024U

/** @brief Maximum vendor defined payload size in bytes.*/
#define QCARCAM_MAX_VENDOR_PAYLOAD_SIZE 1024U

/** @brief Maximum number of frames in a batch.*/
#define QCARCAM_MAX_BATCH_FRAMES 4U

/** @brief Maximum number of ISP instances.*/
#define QCARCAM_MAX_ISP_INSTANCES 4U

/** @brief Maximum number of input modes supported. */
#define QCARCAM_MAX_NUM_MODES   12U


/**@brief QCarCam open flags */
/** Sets session as master for the input. */
#define QCARCAM_OPEN_FLAGS_MASTER       (1U << 0U)
/** Enables self-recovery for the session. */
#define QCARCAM_OPEN_FLAGS_RECOVERY     (1U << 1U)
/** Request based mode of operation */
#define QCARCAM_OPEN_FLAGS_REQUEST_MODE (1U << 2U)
/** Enables average frame count based frame rate monitoring. */
#define QCARCAM_OPEN_FLAGS_AVG_FPS_MONITOR_MODE (1U << 3U)
/** Enables a multi-client session */
#define QCARCAM_OPEN_FLAGS_MULTI_CLIENT_SESSION (1U << 4U)
/** Enables batching of multiple frame info in a single FRAME_READY callback */
#define QCARCAM_OPEN_FLAGS_MULTI_FRAME_INFO     (1U << 5U)


/** @brief max number of input streams per session. */
#define QCARCAM_MAX_INPUT_STREAMS 4U

/** @brief max number of output streams per session. */
#define QCARCAM_MAX_OUTPUT_STREAMS 32U

/** @brief max number of sources for an input. */
#define QCARCAM_INPUT_MAX_NUM_SOURCES 16U

/** @brief Input description flags. */
#define QCARCAM_INPUT_FLAG_PAIRED     (1U << 0U)   /**< Paired input stream. */


/** QCarCam Events */
/** @brief Event type: Frame ready event.
    @desc  In free run mode, the frame can be dequeued via QCarCamGetFrame().

    Payload: #QCarCamFrameInfo_t with the available frame information
    */
#define QCARCAM_EVENT_FRAME_READY   (1U << 0U)

/** @brief Event type: Error event.
    @desc  An error event is sent describing the source and severity of the error

    Payload: #QCarCamErrorInfo_t with the error information
    */
#define QCARCAM_EVENT_ERROR         (1U << 1U)

/** @brief Event type: Vendor event.
    @desc  A vendor event is a customized message.

    Payload: #QCarCamVendorParam_t */
#define QCARCAM_EVENT_VENDOR        (1U << 2U)

/** @brief Event type: Start of Frame (SOF) event
    @desc  This event is sent at the time of QCarCam driver receiving the start of frame at the HW level.

    Payload: #QCarCamHWTimestamp_t indicating the time of the SOF

    @note This event feature may not be suppported on all platforms. */
#define QCARCAM_EVENT_FRAME_SOF     (1U << 3U)

/** @brief Input signal change event
    @desc  An event is sent to indicate a change in lock of the input with the new status in the payload.

    Payload: #QCarCamInputSignal_e */
#define QCARCAM_EVENT_INPUT_SIGNAL    (1U << 4U)

/** @brief Property change notification event.
    @desc In multi client usecases, subscriber clients can be notified of properties modified by the master
           client of an input.

    Payload: #QCarCamParamType_e

    @note This event feature may not be suppported on all platforms. */
#define QCARCAM_EVENT_PROPERTY_NOTIFY (1U << 5U)

/** @brief Recovery event
    @desc  A recovery event is sent to notify client of progression of a recovery attempt.
    Payload: #QCarCamRecovery_t*/
#define QCARCAM_EVENT_RECOVERY        (1U << 6U)

/** @brief Input mode change event
    @desc The input mode was changed externally. For instance, in HDMI input usecases, a new resolution
          may have been set externally and now requires client to switch to the new resolution.

     Paylaod: None */
#define QCARCAM_EVENT_INPUT_MODE_CHANGE (1U << 8U)

/** @brief New input detected event
    @desc Event when a new input is detected.

    Payload: None

    @note This event only sent as a system wide event. */
#define QCARCAM_EVENT_INPUT_DETECT      (1U << 9U)

/** @brief Notify an event specific to multiclient session
    @desc  For a multi-client session, a list of multiclient specific events is provided to notify of impact from other
            clients to the common input session.

    Payload: #QCarCamMCNotify_t

    @note Event sent only as per session event. */
#define QCARCAM_EVENT_MC_NOTIFY         (1U << 10U)

/** @brief Event type: Frame ready event for more than one frame in a batch.
    @desc Both QCARCAM_OPEN_FLAGS_MULTI_FRAME_INFO and QCARCAM_OPEN_FLAGS_REQUEST_MODE should be enabled.

    Payload: #QCarCamMultiFrameInfo_t

    @note Only valid for request mode. */
#define QCARCAM_EVENT_MULTI_STREAM_FRAME_READY   (1U << 11U)



/* Buffer list ID range identifiers
 The bufferlist ID is comprised of 2 parts:
 1. the bufferlist type #QCarCamBufferlistType_e which is in the TYPE bits
 2. the index within that type which is in the INDEX bits.

 The start of each bufferlist type range is defined by the bufferlist type starting offset.
 */
/** Bufferlist ID index mask */
#define QCARCAM_BUFFERLIST_ID_INDEX_MASK                0xFFU
/** Bufferlist type starting bit position */
#define QCARCAM_BUFFERLIST_ID_TYPE_SHIFT                8U
/** Bufferlist type mask */
#define QCARCAM_BUFFERLIST_ID_TYPE_MASK                 ((uint32_t)QCARCAM_BUFFERLIST_TYPE_MAX    << QCARCAM_BUFFERLIST_ID_TYPE_SHIFT)
/** Output bufferlist ID output range */
#define QCARCAM_BUFFERLIST_ID_OUTPUT_RANGE              ((uint32_t)QCARCAM_BUFFERLIST_TYPE_OUTPUT << QCARCAM_BUFFERLIST_ID_TYPE_SHIFT)
/** Input bufferlist ID output range */
#define QCARCAM_BUFFERLIST_ID_INPUT_RANGE               ((uint32_t)QCARCAM_BUFFERLIST_TYPE_INPUT  << QCARCAM_BUFFERLIST_ID_TYPE_SHIFT)
/** Input metadata bufferlist ID output range */
#define QCARCAM_BUFFERLIST_ID_INPUT_METADATA_RANGE      ((uint32_t)QCARCAM_BUFFERLIST_TYPE_INPUT_METADATA  << QCARCAM_BUFFERLIST_ID_TYPE_SHIFT)
/** Output metadata bufferlist ID output range */
#define QCARCAM_BUFFERLIST_ID_OUTPUT_METADATA_RANGE     ((uint32_t)QCARCAM_BUFFERLIST_TYPE_OUTPUT_METADATA << QCARCAM_BUFFERLIST_ID_TYPE_SHIFT)
/** Input/Output bufferlist ID output range */
#define QCARCAM_BUFFERLIST_ID_INPUT_OUTPUT_RANGE        ((uint32_t)QCARCAM_BUFFERLIST_TYPE_INPUT_OUTPUT    << QCARCAM_BUFFERLIST_ID_TYPE_SHIFT)

/** @brief Macro to retrieve bufferlist type from bufferlist id */
#define QCARCAM_GET_BUFFERLIST_TYPE(_id_) (((_id_) & QCARCAM_BUFFERLIST_ID_TYPE_MASK) >> QCARCAM_BUFFERLIST_ID_TYPE_SHIFT)

/** @brief Macro to retrieve bufferlist index within type from bufferlist id */
#define QCARCAM_GET_BUFFERLIST_INDEX(_id_) ((_id_) & QCARCAM_BUFFERLIST_ID_INDEX_MASK)


/** @brief Buffer flag bit if the buffer is cached. */
#define QCARCAM_BUFFER_FLAG_CACHE       (1U << 0U)

/** @brief Buffer flag bit if the output buffer (memHndl in #QCarCamPlane_t) refers to an OS memory
           handle. */
#define QCARCAM_BUFFER_FLAG_OS_HNDL     (1U << 1U)

/** @brief will be redefined later to extend support for security domains */
#define QCARCAM_BUFFER_FLAG_SECURE      (1U << 2U)

/** @brief Buffer flag bit to indicate lossy UBWC */
#define QCARCAM_BUFFER_FLAG_UBWC_LOSSY  (1U << 3U)



/** @brief Buffer status valid type definition. */
#define QCARCAM_BUFFER_STATUS_VALID       (0U)

/** @brief Buffer status invalid type definition in case of errors in fulfilling request */
#define QCARCAM_BUFFER_STATUS_INVALID     (1U << 0U)

/** @brief Buffer status request sync error occured definition. */
#define QCARCAM_BUFFER_STATUS_RSYNC_ERROR  (1U << 1U)


/** @brief QCarCamRequest_t flag to indicate injection request */
#define QCARCAM_REQUEST_FLAG_INJECTION  (1U << 0U)

/** #QCarCamGetParamEx_t and #QCarCamSetParamEx_t flags */
/** @brief indicates parameter is for specific input */
#define QCARCAM_PARAM_EX_FLAG_INPUT   (0U)
/** @brief indicates parameter is for specific output */
#define QCARCAM_PARAM_EX_FLAG_OUTPUT  (1U << 0U)

/** @brief Macro for creating the color format.

    @hideinitializer
*/
#define QCARCAM_COLOR_FMT(_pattern_, _bitdepth_, _pack_)    \
    ((((uint32_t)(_pack_) & 0xffU) << 24) |                 \
     (((uint32_t)(_bitdepth_) & 0xffU) << 16) |             \
      ((uint32_t)(_pattern_) & 0xffffU))

/** @brief Macro for getting the color pattern from the color format.

    @hideinitializer
*/
#define QCARCAM_COLOR_GET_PATTERN(_color_)                  \
    ((QCarCamColorPattern_e)((_color_) & 0xffff))

/** @brief Macro for getting the bit depth from the color format.

    @hideinitializer
*/
#define QCARCAM_COLOR_GET_BITDEPTH(_color_)                 \
    ((QCarCamColorBitDepth_e)(((_color_) & 0xff0000) >> 16))

/** @brief Macro for getting the color pack from the color format.

    @hideinitializer
*/
#define QCARCAM_COLOR_GET_PACK(_color_)                     \
    ((QCarCamColorPack_e)(((_color_) & 0xff000000) >> 24))

/** @} */ /* end_addtogroup qcarcam_constants */

/** @addtogroup qcarcam_datatypes
@{ */

/*=================================================================================================
** Typedef Enumerators
=================================================================================================*/

/** @brief QCarCam input handle type requested via QCarCamOpen() associated to a unique input ID. */
typedef uint64_t QCarCamHndl_t;

/** @brief Return codes. */
typedef enum
{
    QCARCAM_RET_OK = 0,            /**< API return type OK (success). */
    QCARCAM_RET_FAILED,            /**< API return type failed. */
    QCARCAM_RET_BADPARAM,          /**< API return type bad parameter(s). */
    QCARCAM_RET_INVALID_OP,        /**< API return type invalid operation. */
    QCARCAM_RET_BADSTATE,          /**< API return type bad state. */
    QCARCAM_RET_NOT_PERMITTED,     /**< API return type not permitted. */
    QCARCAM_RET_OUT_OF_BOUND,      /**< API return type out of bound. */
    QCARCAM_RET_TIMEOUT,           /**< API return type time out. */
    QCARCAM_RET_NOMEM,             /**< API return type no memory. */
    QCARCAM_RET_UNSUPPORTED,       /**< API return type not supported. */
    QCARCAM_RET_BUSY,              /**< API return type busy. */
    QCARCAM_RET_NOT_FOUND,         /**< API return type not found. */
    QCARCAM_RET_BIST_FAILURE,      /**< API return type built in self test failure. */
    QCARCAM_RET_SENSOR_NOT_FOUND,  /**< API return type sensor not found. */
    QCARCAM_RET_LAST      = 255    /**< Last or invalid QCARCAM_RET_* type. */
} QCarCamRet_e;

/** @brief Color type definition. */
typedef enum
{
    QCARCAM_RAW = 0U,                       /**< Type for raw color format. */

    /* Memory layout for YUV formats is the color component name from LSB to MSB
     * e.g. YUYV ->  Y:[0-7], U:[8-15], Y[16-23], V[24-31]
     * */
    QCARCAM_YUV_YUYV = 0x100U,              /**< Type for YUV_YUYV color format. */
    QCARCAM_YUV_YVYU,                       /**< Type for YUV_YVYU color format. */
    QCARCAM_YUV_UYVY,                       /**< Type for YUV_UYVY color format. */
    QCARCAM_YUV_VYUY,                       /**< Type for YUV_VYUY color format. */

    QCARCAM_YUV420_Y_UV  = 0x104U,          /**< Type for YUV420_Y_UV color format. */
    QCARCAM_YUV420_Y_VU  = 0x105U,          /**< Type for YUV420_Y_VV color format. */
    QCARCAM_YUV420_Y_U_V = 0x106U,          /**< Type for YUV420_Y_U_V color format. */
    QCARCAM_YUV420_Y_V_U = 0x107U,          /**< Type for YUV420_Y_V_U color format. */

    QCARCAM_YUV_NV12 = QCARCAM_YUV420_Y_UV,    /**< Type for YUV_NV12 color format. */
    QCARCAM_YUV_NV21 = QCARCAM_YUV420_Y_VU,    /**< Type for YUV_NV21 color format. */

    QCARCAM_YUV_YU12 = QCARCAM_YUV420_Y_U_V,   /**< Type for YUV_YU12 color format. */
    QCARCAM_YUV_YV12 = QCARCAM_YUV420_Y_V_U,   /**< Type for YUV_YV12 color format. */

    QCARCAM_YUV_Y,                          /**< Type for YUV_Y color format.    */

    QCARCAM_YUV_Y12_UV8,                    /**< Type for 12bit Y plane and 8bit UV plane */
    QCARCAM_YUV_Y12_UV10,                   /**< Type for 12bit Y plane and 10bit UV plane */

    QCARCAM_YUV444_Y_U_V,                   /**< Type for YUV444_Y_U_V color format. */

    QCARCAM_BAYER_GBRG = 0x200U,            /**< Type for BAYER_GBRG color format. */
    QCARCAM_BAYER_GRBG,                     /**< Type for BAYER_GRBG color format. */
    QCARCAM_BAYER_RGGB,                     /**< Type for BAYER_RGGB color format. */
    QCARCAM_BAYER_BGGR,                     /**< Type for BAYER_BGGR color format. */

    /* Memory layout for RGB formats is the color component name from MSB to LSB
     * e.g. RGB888 ->  R:[16-23], G:[8-15], B[0-7]
     * */
    QCARCAM_RGB = 0x300U,        /**< Type for RGB color R:[16-23], G:[8-15]: B[0-7] */
    QCARCAM_RGB_RGB888 = QCARCAM_RGB, /**< Type for RGB888 color R:[16-23], G:[8-15]: B[0-7] */
    QCARCAM_RGB_BGR888,         /**< Type for BGR888 color B:[16-23], G:[8-15]: R[0-7] */
    QCARCAM_RGB_RGB565,         /**< Type for RGB565 color R:[11-15], G:[5-10]: B[0-4] */
    QCARCAM_RGB_RGBX8888,       /**< Type for RGBX8888 color R:[24-31] G:[16-23], B:[8-15]: X[0-7] */
    QCARCAM_RGB_BGRX8888,       /**< Type for BGRX8888 color B:[24-31] G:[16-23], R:[8-15]: X[0-7] */
    QCARCAM_RGB_RGBX1010102,    /**< Type for RGBX1010102 color R:[22-31] G:[12-21], B:[2-11]: X[0-1] */
    QCARCAM_RGB_BGRX1010102,    /**< Type for BGRX1010102 color B:[22-31] G:[12-21], R:[2-11]: X[0-1] */
    QCARCAM_RGB_R8_G8_B8,       /**< Type for Planar R,G,B with 8bit per channel */

    QCARCAM_COLORTYPE_MAX = 0x7fffffffU     /**< Last or invalid color type. */
} QCarCamColorPattern_e;

/** @brief Bitdepth per color channel
 For RAW/YUV formats it is the bits per color channel
 For RGB formats it is the effective bits per pixel
 */
typedef enum
{
    QCARCAM_BITDEPTH_8 =  8U,           /**< Type for color bit depth of 8. */
    QCARCAM_BITDEPTH_10 = 10U,          /**< Type for color bit depth of 10. */
    QCARCAM_BITDEPTH_12 = 12U,          /**< Type for color bit depth of 12. */
    QCARCAM_BITDEPTH_14 = 14U,          /**< Type for color bit depth of 14. */
    QCARCAM_BITDEPTH_16 = 16U,          /**< Type for color bit depth of 16. */
    QCARCAM_BITDEPTH_20 = 20U,          /**< Type for color bit depth of 20. */
    QCARCAM_BITDEPTH_24 = 24U,          /**< Type for color bit depth of 24. */
    QCARCAM_BITDEPTH_32 = 32U,          /**< Type for color bit depth of 32. */
    QCARCAM_BITDEPTH_MAX = 0x7fffffffU  /**< Last or invalid color bit depth type. */
} QCarCamColorBitDepth_e;

/** @brief Color packing type. */
typedef enum
{
    /** Type for Qualcomm Technologies Inc (QTI) color pack. */
    QCARCAM_PACK_QTI = 0,

    /** Type for Mobile Industry Processor Interface (MIPI) color pack. */
    QCARCAM_PACK_MIPI,

    /** Type for Plain 8 color pack. */
    QCARCAM_PACK_PLAIN8,

    /** Type for Plain 16 color pack into the lowest significant bits */
    QCARCAM_PACK_PLAIN16,

    /** Type for Plain 32 color pack. */
    QCARCAM_PACK_PLAIN32,

    /** ype for UBWC color pack.  */
    QCARCAM_PACK_UBWC,

    /** Type for Plain 16 color pack into the most significant bits */
    QCARCAM_PACK_PLAIN16_MSB,

    /** Last or invalid color pack type. */
    QCARCAM_PACK_MAX = 0x7fffffffU
} QCarCamColorPack_e;

/** @brief Color formats. */
typedef enum
{
    /** MIPI packed RAW formats. */

    /** MIPI packed RAW 8bit */
    QCARCAM_FMT_MIPIRAW_8 = QCARCAM_COLOR_FMT(QCARCAM_RAW,
                                              QCARCAM_BITDEPTH_8,
                                              QCARCAM_PACK_MIPI),

    /** MIPI packed RAW 10bit */
    QCARCAM_FMT_MIPIRAW_10 = QCARCAM_COLOR_FMT(QCARCAM_RAW,
                                               QCARCAM_BITDEPTH_10,
                                               QCARCAM_PACK_MIPI),

    /** MIPI packed RAW 12bit */
    QCARCAM_FMT_MIPIRAW_12 = QCARCAM_COLOR_FMT(QCARCAM_RAW,
                                               QCARCAM_BITDEPTH_12,
                                               QCARCAM_PACK_MIPI),

    /** MIPI packed RAW 14bit */
    QCARCAM_FMT_MIPIRAW_14 = QCARCAM_COLOR_FMT(QCARCAM_RAW,
                                               QCARCAM_BITDEPTH_14,
                                               QCARCAM_PACK_MIPI),

    /** MIPI packed RAW 16bit */
    QCARCAM_FMT_MIPIRAW_16 = QCARCAM_COLOR_FMT(QCARCAM_RAW,
                                               QCARCAM_BITDEPTH_16,
                                               QCARCAM_PACK_MIPI),

    /** MIPI packed RAW 20bit */
    QCARCAM_FMT_MIPIRAW_20 = QCARCAM_COLOR_FMT(QCARCAM_RAW,
                                               QCARCAM_BITDEPTH_20,
                                               QCARCAM_PACK_MIPI),

    /** PLAIN16 packed RAW formats. */

    /** Plain16 packed RAW 10bit */
    QCARCAM_FMT_PLAIN16_10 = QCARCAM_COLOR_FMT(QCARCAM_RAW,
                                               QCARCAM_BITDEPTH_10,
                                               QCARCAM_PACK_PLAIN16),

    /** Plain16 packed RAW 12bit */
    QCARCAM_FMT_PLAIN16_12 = QCARCAM_COLOR_FMT(QCARCAM_RAW,
                                               QCARCAM_BITDEPTH_12,
                                               QCARCAM_PACK_PLAIN16),

    /** Plain16 packed RAW 14bit */
    QCARCAM_FMT_PLAIN16_14 = QCARCAM_COLOR_FMT(QCARCAM_RAW,
                                               QCARCAM_BITDEPTH_14,
                                               QCARCAM_PACK_PLAIN16),

    /**< Plain16 packed RAW 16bit */
    QCARCAM_FMT_PLAIN16_16 = QCARCAM_COLOR_FMT(QCARCAM_RAW,
                                               QCARCAM_BITDEPTH_16,
                                               QCARCAM_PACK_PLAIN16),

    /**< Plain32 packed RAW 20bit */
    QCARCAM_FMT_PLAIN32_20 = QCARCAM_COLOR_FMT(QCARCAM_RAW,
                                               QCARCAM_BITDEPTH_20,
                                               QCARCAM_PACK_PLAIN32),

    /**< Plain32 packed RAW 32bit */
    QCARCAM_FMT_PLAIN32_32 = QCARCAM_COLOR_FMT(QCARCAM_RAW,
                                               QCARCAM_BITDEPTH_32,
                                               QCARCAM_PACK_PLAIN32),

    /** YUV Packed Formats. */

    /** QTI packed YUV_UYVY 8bit */
    QCARCAM_FMT_UYVY_8 = QCARCAM_COLOR_FMT(QCARCAM_YUV_UYVY,
                                           QCARCAM_BITDEPTH_8,
                                           QCARCAM_PACK_QTI),

    /** QTI packed YUV_UYVY 10bit */
    QCARCAM_FMT_UYVY_10 = QCARCAM_COLOR_FMT(QCARCAM_YUV_UYVY,
                                            QCARCAM_BITDEPTH_10,
                                            QCARCAM_PACK_QTI),

    /** QTI packed YUV_UYVY 12bit */
    QCARCAM_FMT_UYVY_12 = QCARCAM_COLOR_FMT(QCARCAM_YUV_UYVY,
                                            QCARCAM_BITDEPTH_12,
                                            QCARCAM_PACK_QTI),

    /** QTI packed YUV_YUYV 8bit */
    QCARCAM_FMT_YUYV_8 = QCARCAM_COLOR_FMT(QCARCAM_YUV_YUYV,
                                           QCARCAM_BITDEPTH_8,
                                           QCARCAM_PACK_QTI),

    /** QTI packed YUV_YUYV 10bit */
    QCARCAM_FMT_YUYV_10 = QCARCAM_COLOR_FMT(QCARCAM_YUV_YUYV,
                                            QCARCAM_BITDEPTH_10,
                                            QCARCAM_PACK_QTI),

    /** QTI packed YUV_YUYV 8bit */
    QCARCAM_FMT_VYUY_8 = QCARCAM_COLOR_FMT(QCARCAM_YUV_VYUY,
                                           QCARCAM_BITDEPTH_8,
                                           QCARCAM_PACK_QTI),

    /** QTI packed YUV_YUYV 10bit */
    QCARCAM_FMT_VYUY_10 = QCARCAM_COLOR_FMT(QCARCAM_YUV_VYUY,
                                            QCARCAM_BITDEPTH_10,
                                            QCARCAM_PACK_QTI),

    /** QTI packed YUV_YUYV 8bit */
    QCARCAM_FMT_YVYU_8 = QCARCAM_COLOR_FMT(QCARCAM_YUV_YVYU,
                                           QCARCAM_BITDEPTH_8,
                                           QCARCAM_PACK_QTI),

    /** QTI packed YUV_YUYV 10bit */
    QCARCAM_FMT_YVYU_10 = QCARCAM_COLOR_FMT(QCARCAM_YUV_YVYU,
                                            QCARCAM_BITDEPTH_10,
                                            QCARCAM_PACK_QTI),

    /** QTI packed YUV_YUYV 12bit */
    QCARCAM_FMT_YUYV_12 = QCARCAM_COLOR_FMT(QCARCAM_YUV_YUYV,
                                            QCARCAM_BITDEPTH_12,
                                            QCARCAM_PACK_QTI),

    /** YUV Semi-Planar Formats. */

    /** QTI packed YUV_NV12 8bit */
    QCARCAM_FMT_NV12 = QCARCAM_COLOR_FMT(QCARCAM_YUV_NV12,
                                         QCARCAM_BITDEPTH_8,
                                         QCARCAM_PACK_QTI),

    /** QTI packed YUV_NV21 8bit */
    QCARCAM_FMT_NV21 = QCARCAM_COLOR_FMT(QCARCAM_YUV_NV21,
                                         QCARCAM_BITDEPTH_8,
                                         QCARCAM_PACK_QTI),

    /** QTI packed YUV_YU12 8bit */
    QCARCAM_FMT_YU12 = QCARCAM_COLOR_FMT(QCARCAM_YUV_YU12,
                                         QCARCAM_BITDEPTH_8,
                                         QCARCAM_PACK_QTI),

    /** QTI packed YUV_YV12 8bit */
    QCARCAM_FMT_YV12 = QCARCAM_COLOR_FMT(QCARCAM_YUV_YV12,
                                         QCARCAM_BITDEPTH_8,
                                         QCARCAM_PACK_QTI),

    /** YUV Y plane only formats. */

    /** QTI packed YUV_Y 8bit */
    QCARCAM_FMT_Y8 = QCARCAM_COLOR_FMT(QCARCAM_YUV_Y,
                                       QCARCAM_BITDEPTH_8,
                                       QCARCAM_PACK_QTI),

    /** PLAIN16_10 MSB packed YUV_Y */
    QCARCAM_FMT_Y10 = QCARCAM_COLOR_FMT(QCARCAM_YUV_Y,
                                     QCARCAM_BITDEPTH_10,
                                     QCARCAM_PACK_PLAIN16_MSB),

    /** PLAIN16_10 LSB packed YUV_Y */
    QCARCAM_FMT_Y10LSB = QCARCAM_COLOR_FMT(QCARCAM_YUV_Y,
                                     QCARCAM_BITDEPTH_10,
                                     QCARCAM_PACK_PLAIN16),

    /** YUV MIPI and PLAIN packed formats. */

    /** MIPI packed UYVY 10bit */
    QCARCAM_FMT_MIPIUYVY_10 = QCARCAM_COLOR_FMT(QCARCAM_YUV_UYVY,
                                                QCARCAM_BITDEPTH_10,
                                                QCARCAM_PACK_MIPI),

    /** Plain16 packed Y_UV 10bit */
    QCARCAM_FMT_P010 = QCARCAM_COLOR_FMT(QCARCAM_YUV420_Y_UV,
                                         QCARCAM_BITDEPTH_10,
                                         QCARCAM_PACK_PLAIN16_MSB),

    /** Plain16 packed Y_UV 10bit LSB*/
    QCARCAM_FMT_P010LSB = QCARCAM_COLOR_FMT(QCARCAM_YUV420_Y_UV,
                                            QCARCAM_BITDEPTH_10,
                                            QCARCAM_PACK_PLAIN16),

    /** P01210 Y-12bit UV-10bit plain16 */
    QCARCAM_FMT_P01210 = QCARCAM_COLOR_FMT(QCARCAM_YUV_Y12_UV10,
                                           QCARCAM_BITDEPTH_12,
                                           QCARCAM_PACK_PLAIN16_MSB),

    /** P0128 Y-12bit UV-8bit plain16 */
    QCARCAM_FMT_P01208 = QCARCAM_COLOR_FMT(QCARCAM_YUV_Y12_UV8,
                                           QCARCAM_BITDEPTH_12,
                                           QCARCAM_PACK_PLAIN16_MSB),

    /** P01210 Y-12bit UV-10bit plain16 LSB */
    QCARCAM_FMT_P01210LSB = QCARCAM_COLOR_FMT(QCARCAM_YUV_Y12_UV10,
                                           QCARCAM_BITDEPTH_12,
                                           QCARCAM_PACK_PLAIN16),

    /** P0128 Y-12bit UV-8bit plain16 LSB */
    QCARCAM_FMT_P01208LSB = QCARCAM_COLOR_FMT(QCARCAM_YUV_Y12_UV8,
                                            QCARCAM_BITDEPTH_12,
                                            QCARCAM_PACK_PLAIN16),

    /** YUV Planar Formats. */

    /** QTI planar 8bit YUV444 */
    QCARCAM_FMT_YUV444_888 = QCARCAM_COLOR_FMT(QCARCAM_YUV444_Y_U_V,
                                            QCARCAM_BITDEPTH_8,
                                            QCARCAM_PACK_QTI),


    /** RGB Formats. */

    /** QTI packed RGB888 24bit */
    QCARCAM_FMT_RGB_888 = QCARCAM_COLOR_FMT(QCARCAM_RGB,
                                            QCARCAM_BITDEPTH_24,
                                            QCARCAM_PACK_QTI),

    /** QTI packed BGR888 24bit */
    QCARCAM_FMT_BGR_888 = QCARCAM_COLOR_FMT(QCARCAM_RGB_BGR888,
                                            QCARCAM_BITDEPTH_24,
                                            QCARCAM_PACK_QTI),

    /** QTI packed RGB565 16bit */
    QCARCAM_FMT_RGB_565 = QCARCAM_COLOR_FMT(QCARCAM_RGB_RGB565,
                                            QCARCAM_BITDEPTH_16,
                                            QCARCAM_PACK_QTI),

    /** QTI packed RGBX8888 32bit */
    QCARCAM_FMT_RGBX_8888 = QCARCAM_COLOR_FMT(QCARCAM_RGB_RGBX8888,
                                              QCARCAM_BITDEPTH_32,
                                              QCARCAM_PACK_QTI),

    /** QTI packed BGRX8888 32bit */
    QCARCAM_FMT_BGRX_8888 = QCARCAM_COLOR_FMT(QCARCAM_RGB_BGRX8888,
                                              QCARCAM_BITDEPTH_32,
                                              QCARCAM_PACK_QTI),

    /** QTI packed RGBX1010102 32bit */
    QCARCAM_FMT_RGBX_1010102 = QCARCAM_COLOR_FMT(QCARCAM_RGB_RGBX1010102,
                                                 QCARCAM_BITDEPTH_32,
                                                 QCARCAM_PACK_QTI),

    /** QTI packed BGRX1010102 32bit */
    QCARCAM_FMT_BGRX_1010102 = QCARCAM_COLOR_FMT(QCARCAM_RGB_BGRX1010102,
                                                 QCARCAM_BITDEPTH_32,
                                                 QCARCAM_PACK_QTI),

    /** QTI planar 8bit RGB (R8, G8, 88) */
    QCARCAM_FMT_R8_G8_B8 = QCARCAM_COLOR_FMT(QCARCAM_RGB_R8_G8_B8,
                                            QCARCAM_BITDEPTH_8,
                                            QCARCAM_PACK_QTI),

    /** Compressed formats */

    /** UBWC packed YUV_NV12 8bit */
    QCARCAM_FMT_UBWC_NV12 = QCARCAM_COLOR_FMT(QCARCAM_YUV_NV12,
                                              QCARCAM_BITDEPTH_8,
                                              QCARCAM_PACK_UBWC),

    /** UBWC packed YUV_TP10 10bit */
    QCARCAM_FMT_UBWC_TP10 = QCARCAM_COLOR_FMT(QCARCAM_YUV420_Y_UV,
                                              QCARCAM_BITDEPTH_10,
                                              QCARCAM_PACK_UBWC),

    /** Last or invalid color format type. */
    QCARCAM_FMT_MAX = 0x7FFFFFFF
} QCarCamColorFmt_e;

/** @brief Supported color space formats */
typedef enum
{
    QCARCAM_COLOR_SPACE_UNCORRECTED = 0,
    QCARCAM_COLOR_SPACE_SRGB,
    QCARCAM_COLOR_SPACE_LRGB,
    QCARCAM_COLOR_SPACE_BT601,
    QCARCAM_COLOR_SPACE_BT601_FULL,
    QCARCAM_COLOR_SPACE_BT709,
    QCARCAM_COLOR_SPACE_BT709_FULL,
}QCarCamColorSpace_e;

/** @brief QCarCam Parameters. The parameters are broken into sections. */
typedef enum
{
    /** Stream configuration parameters section */
    /** @note Stream configuration parameter base value.
     Parameters in this section are used for session and stream specific settings.
     **/
    QCARCAM_STREAM_CONFIG_PARAM_BASE = 0x00000000,

    /**
     Description:
      This parameter is used to mask which events will trigger the registered callback method to be invoked.
      It is a bitmask of QCARCAM_EVENT_*. A set flag will indicate the event is active.

     Type :
      uint32_t

     Default :
      All events enabled

     Allowed State:
      May be performed during all states
     **/
    QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK,

    /**
     Description:
      This parameter is used to set crop region at the input of the ISP

     Type :
      #QCarCamRegion_t

     Default :
      No additional cropping.

     Allowed State:
      May be performed prior to RESERVE
     **/
    QCARCAM_STREAM_CONFIG_PARAM_SET_CROP,

    /**
     Description:
      This parameter is used to control the frame delivery latency parameters when operating in free run mode.
      On being ready, buffers are stored in a queue awaiting for client to call #QCarCamGetFrame(). The configurable
      parameters can set the maximum number of buffers allowed in the queue. In case the maximum is reached,
      a parameter is used to specify how many buffers are dropped.

     Type :
      #QCarCamLatencyControl_t

     Default :
      Implementation specific

     Allowed State:
      May be performed prior to RESERVE
     **/
    QCARCAM_STREAM_CONFIG_PARAM_LATENCY_CONTROL,

    /**
     Description:
      This parameter is to configure frame drop pattern at the input of the ISP

     Type :
      #QCarCamFrameDropConfig_t

     Default :
      No frame drop.

     Allowed State:
      May be performed prior to RESERVE
     **/
    QCARCAM_STREAM_CONFIG_PARAM_FRAME_DROP_CONTROL,

    /**
     Description:
      This parameter is to switch to a new sensor mode using the index to the input modes array
      that is filled from a call to #QCarCamQueryInputModes()

     Type :
      uint32_t

     Default :
      Input's default mode is implementation specific and is set through the #QCarCamOpen() API.

     Allowed State:
      May be performed prior to RESERVE
      May be allowed in STREAMING state only if possible.
     **/
    QCARCAM_STREAM_CONFIG_PARAM_INPUT_MODE,

    /**
     Description:
      This parameter is to set the current session as the "master" session for the opened inputs.

     Type :
      uint32_t

     Default :
      The first client session is identified as a master.

     Allowed State:
      May be performed in any state
     **/
    QCARCAM_STREAM_CONFIG_PARAM_MASTER,

    /**
     UNSUPPORTED
     Subscribe to get notified of a parameter change from a master client
     **/
    QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_SUBSCRIBE,

    /**
     UNSUPPORTED
     UnSubscribe from a parameter change done by a master client
     **/
    QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_UNSUBSCRIBE,

    /**
     Description:
      Configures mode to batch multiple frames into single output frame. The passed in buffers must be
      large enough to hold the configured number of frames.
      This mode is especially relevant for high frame rate applications where there is tolerance for
      increased frame latency to batch several frames into a single ready event.

     Type :
      #QCarCamBatchConfig_t

     Default :
      No batching of multiple frames.

     Allowed State:
      May be performed prior to RESERVE
     **/
    QCARCAM_STREAM_CONFIG_PARAM_BATCH_MODE,

    /**
     Description:
      Configures the ISP usecase to be used for the session.

     Type :
      QCarCamIspUsecaseConfig_t

     Default :
      Invalid ISP usecase

     Allowed State:
      May be performed prior to RESERVE
     **/
    QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE,

    /**
     Description:
      Configures the frame rate monitor controls.

     Type :
      #QCarCamFrameRateMonitorConfig_t

     Default :
      No frame rate monitoring.

     Allowed State:
      May be performed prior to RESERVE
     **/
    QCARCAM_STREAM_CONFIG_PARAM_FRAME_RATE_MONITOR_CONTROL,

    /**
     Description:
      Configures parameters for standalone injection case.

     Type :
      #QCarCamInjectCfg_t

     Default :
      No injection configuration.

     Allowed State:
      May be performed prior to RESERVE
     **/
    QCARCAM_STREAM_CONFIG_PARAM_STANDALONE_INJECTION_CONFIG,

    /**
     Description:
      Parameter to set the master input ID if multiple cameras are present in a single sessions.

     Type :
      uint32_t

     Allowed State:
      May be performed prior to RESERVE
     **/
    QCARCAM_STREAM_CONFIG_PARAM_MASTER_INPUT,

    /**
     Description:
      2A synchronization type for multicamera usecases

     Type :
      uint32_t

     Allowed State:
      May be performed prior to RESERVE
     **/
    QCARCAM_STREAM_CONFIG_PARAM_2A_SYNC_TYPE,

    /**
     Description:
      Configures the LDC EEPROM params for output max FOV streams to be used for the session.

     Type :
      #QCarCamLDCMaxFOVConfig_t

     Allowed State:
      May be performed prior to RESERVE
     **/
    QCARCAM_STREAM_CONFIG_PARAM_LDC_MAX_FOV,

    /**
     Description:
      Parameter to filter the set of metadata tags to be published in the output metadata for the session.

     Type :
      #QCarCamMetadataPublishConfig_t

     Allowed State:
      May be performed prior to RESERVE
     **/
    QCARCAM_STREAM_CONFIG_PARAM_METADATA_PUBLISH_CONFIG,

    /** Sensor parameters section.
     @note Parameters in this section are used to control the input sensor module directly and
     do not affect settings to the ISP.
     Their support depends on the support provided by the respective sensor module driver.
     **/
    QCARCAM_SENSOR_PARAM_BASE = 0x00000100, /**< Sensor parameter base value */

    /**
     Description:
      Enable/Disable horizontal mirroring

     Type :
      uint32_t

     Default :
      Implmentation specific.
     **/
    QCARCAM_SENSOR_PARAM_MIRROR_H,

    /**
     Description:
      Enable/Disable vertical flip (vertical mirror)

     Type :
      uint32_t

     Default :
      Implmentation specific.
     **/
    QCARCAM_SENSOR_PARAM_MIRROR_V,

    /**
     Description:
      Configured Video standard

     Type :
      uint32_t

     Default :
      Implmentation specific.
     **/
    QCARCAM_SENSOR_PARAM_VID_STD,

    /**
     Description:
      Current Detected Video standard

     Type :
      uint32_t
     **/
    QCARCAM_SENSOR_PARAM_CURRENT_VID_STD,

    /**
     Description:
      Video lock status

     Type :
      #QCarCamInputSignal_e

     Default :
      Implmentation specific.
     **/
    QCARCAM_SENSOR_PARAM_SIGNAL_STATUS,

    /**
     Description:
      Exposure configuration to the sensor

     Type :
      #QCarCamExposureConfig_t

     Default :
      Implmentation specific.
     **/
    QCARCAM_SENSOR_PARAM_EXPOSURE,

    /**
     Description:
      Gamma configuration to the sensor

     Type :
      #QCarCamGammaConfig_t

     Default :
      Implmentation specific.
     **/
    QCARCAM_SENSOR_PARAM_GAMMA,

    /**
     Description:
      Brightness configuration to the sensor.
      Exposure compensation is used to alter exposure from the value selected by the camera, for a brighter or darker image.

     Type :
      float

     Range:
      [-1.0f, 1.0f]

     Default value:
      0.0f
     **/
    QCARCAM_SENSOR_PARAM_BRIGHTNESS,

    /**
     Description:
      Contrast configuration to the sensor.
      Contrast is the separation between the darkest and brightest areas of the image.
      Increasing the contrast increases the separation between dark and bright, for darker shadows and brighter highlights.
      Decreasing the contrast brings the shadows up and the highlights down to make them closer to one another.

     Type :
      float

     Range:
      [-1.0f, 1.0f]

     Default value:
      0.0f
     **/
    QCARCAM_SENSOR_PARAM_CONTRAST,

    /**
     Description:
      Hue configuration to the sensor.

     Type :
      float

     Range:
      [-1.0f, 1.0f]

     Default value:
      0.0f
     **/
    QCARCAM_SENSOR_PARAM_HUE,

    /**
     Description:
      Hue configuration to the sensor.

     Type :
      float

     Range:
      [-1.0f, 1.0f]

     Default value:
      0.0f
     **/
    QCARCAM_SENSOR_PARAM_SATURATION,

    /**
     Description:
      Color space of the sensor image format.

     Type :
      #QCarCamColorSpace_e
     **/
    QCARCAM_SENSOR_PARAM_COLOR_SPACE,

    /**
     Description:
      Dynamically set/get the camera mount orientation

     Type :
      #QCarCamMountOrientation_t

     Allowed State:
      May be performed prior to RESERVE
     **/
    QCARCAM_SENSOR_PARAM_MOUNT_ORIENTATION,

    /** Vendor specific sensor parameter value. */
    QCARCAM_VENDOR_PARAM_BASE = 0x00000300,

    /**
     Description:
      Vendor parameter is used for vendor defined parameters that are passed to the vendor implemented
      sensor libraries. The parameter payload #QCarCamVendorParam_t is an abstract data array that can be used
      to abstract a vendor's message interface.

     Type :
      #QCarCamVendorParam_t
     **/
    QCARCAM_VENDOR_PARAM,


    /** Last or invalid parameter type. */
    QCARCAM_PARAM_MAX = 0x7FFFFFFF
} QCarCamParamType_e;

/** @brief Camera module mount orientation in degrees */
typedef enum
{
    QCARCAM_SENSOR_ORIENTATION_0  = 0,
    QCARCAM_SENSOR_ORIENTATION_90 = 90,
    QCARCAM_SENSOR_ORIENTATION_180 = 180,
    QCARCAM_SENSOR_ORIENTATION_270 = 270
} QCarCamSensorOrientation_e;

/** @brief Camera module mount orientation structure */
typedef struct
{
    QCarCamSensorOrientation_e orientation; /**< mount orientation in degrees */
    /** output sensor pattern.
     @note this is not needed for SET_PARAM and only filled by GET_PARAM */
    QCarCamColorPattern_e      pattern;
} QCarCamMountOrientation_t;

/** @brief Interlaced field description */
typedef enum
{
    QCARCAM_INTERLACE_FIELD_NONE = 0,   /**< not an interlaced frame */
    QCARCAM_INTERLACE_FIELD_UNKNOWN,    /**< interlaced but unknown field type */
    QCARCAM_INTERLACE_FIELD_ODD,        /**< interlaced odd field */
    QCARCAM_INTERLACE_FIELD_EVEN,       /**< interlaced even field */
    QCARCAM_INTERLACE_FIELD_ODD_EVEN,   /**< interlaced odd field at top, even field on bottom */
    QCARCAM_INTERLACE_FIELD_EVEN_ODD,   /**< interlaced even field at top, odd field on bottom */

    QCARCAM_INTERLACE_FIELD_MAX = 0x7FFFFFFF  /**< Last or invalid interlace type. */
} QCarCamInterlaceField_e;

/** @brief Exposure modes */
typedef enum
{
    QCARCAM_EXPOSURE_AUTO,   /**< Automatic exposure control */
    QCARCAM_EXPOSURE_MANUAL, /**< Manual exposure control */
} QCarCamExposureMode_e;

/** @brief Exposures identification.
 *    T1 exposure is a longest and each successive exposure being shorter (T1 > T2 > T3 > T4)
 */
typedef enum
{
    QCARCAM_HDR_EXPOSURE_T1,
    QCARCAM_HDR_EXPOSURE_T2,
    QCARCAM_HDR_EXPOSURE_T3,
    QCARCAM_HDR_EXPOSURE_T4,
    QCARCAM_HDR_NUM_EXPOSURES
} QCarCamExposure_e;

/** @brief Gamma parameter type. */
typedef enum
{
    QCARCAM_GAMMA_EXPONENT,         /**< set gamma exponent value in a float */
    QCARCAM_GAMMA_KNEEPOINTS,       /**< set table of gamma kneepoint values. */
} QCarCamGammaType_e;

/** @brief Multiclient event type. */
typedef enum
{
    QCARCAM_MC_STREAM_CREATE,       /**< multiclient stream created */
    QCARCAM_MC_STREAM_DESTROY,      /**< multiclient stream destroyed */
    QCARCAM_MC_STREAM_START,        /**< multiclient stream started */
    QCARCAM_MC_STREAM_STOP,         /**< multiclient stream stoped */
} QCarCamMCEvent_e;

/** @brief Input operation modes.

    The operation mode dictates the overall operating mode of the session.
    If an operation mode has an ISP component, then the QCarCamIspUsecase_e can specify
    the usecase for an ISP component instance.
  */
typedef enum
{
    /** Raw dump of data mode of operation.
     * This mode will perform a raw data dump of the incoming sensor streams
     * to memory passed by the client.
     */
    QCARCAM_OPMODE_RAW_DUMP,

    /** Mode of operation with the ISP with the sensor as the input source.
     The ISP usecase that is set for the session defines the available output streams.

      @note The parameter QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE must be set to
             specify the ISP usecase for the session.
     */
    QCARCAM_OPMODE_ISP,

    /** Mode of operation for offline use of the ISP. A memory buffer is used to input
        the image to be processed.

      @note The parameter QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE must be set to
             specify the ISP usecase for the session.
     */
    QCARCAM_OPMODE_OFFLINE_ISP,

    /** The paired operating mode is designed for bonded CSI inputs to recombine the data into a single buffer.
        The ISP recombines the two streams to the same client buffer (one stream for left portion of the output
        image and another for the right side). Checks are performed against the timestamps of the two streams to
        ensure that they belong to the same incoming frame. If the processing goes out of sync for the two streams
        an error event is sent indicating this. */
    QCARCAM_OPMODE_PAIRED_INPUT,


    /** The deinterlace operating mode utilizes implementation specific resource (SW, GPU or DSP) to perform a
        deinterlace operation on the incoming stream.
     */
    QCARCAM_OPMODE_DEINTERLACE,

    /** Gpu color conversion and scaling. */
    QCARCAM_OPMODE_TRANSFORMER,

    /** Preprocessing then ISP. */
    QCARCAM_OPMODE_PREPROCESS_ISP,

    /** Color conversion using raw data interface (yuv422 to nv12) */
    QCARCAM_OPMODE_RDI_CONVERSION,

    QCARCAM_OPMODE_MAX = 0x7FFFFFFF /**< Last or invalid parameter type. */
} QCarCamOpmode_e;

/** @brief ISP use case for operating modes with ISP */
typedef enum
{
    QCARCAM_ISP_USECASE_PREVIEW,                /**< outputs 0-preview */
    QCARCAM_ISP_USECASE_SHDR_PREPROCESS,        /**< outputs 0-preview */
    QCARCAM_ISP_USECASE_CUSTOM_START  = 0x100U, /**< starting offset for custom defined usecases */

    QCARCAM_ISP_USECASE_MAX = 0x7FFFFFFF    /**< Last or invalid parameter type. */
} QCarCamIspUsecase_e;

/** @brief Batch mode types. */
typedef enum
{
    /** #QCarCamBatchFramesInfo_t is returned in QCarCamFrameInfo_t for this mode */
    QCARCAM_BATCH_MODE_FILL_ALL_FRAME_INFO = 0,

    /** #QCarCamBatchResult_t is returned in QCarCamFrameInfo_t for this mode */
    QCARCAM_BATCH_MODE_FILL_BATCH_RESULT = 1,

    /** Last or invalid parameter type. */
    QCARCAM_BATCH_MODE_MAX = 0x7FFFFFFF
}QCarCamBatchMode_e;

/** @brief Bufferlist type */
typedef enum
{
    /** output bufferlist type may only be used for output streams */
    QCARCAM_BUFFERLIST_TYPE_OUTPUT = 0U,
    /** input bufferlist may only be used for injection streams */
    QCARCAM_BUFFERLIST_TYPE_INPUT,
    /** input metadata bufferlist may only be used for input metadata */
    QCARCAM_BUFFERLIST_TYPE_INPUT_METADATA,
    /** output metadata bufferlist may only be used for output metadata */
    QCARCAM_BUFFERLIST_TYPE_OUTPUT_METADATA,
    /** input/output bufferlist may only be used for reprocessing streams */
    QCARCAM_BUFFERLIST_TYPE_INPUT_OUTPUT,
    QCARCAM_BUFFERLIST_TYPE_MAX = 0xFU
}QCarCamBufferlistType_e;

/** @brief buffer list identification */
typedef enum
{
    /** Output buffer lists start at this index */
    QCARCAM_BUFFERLIST_ID_OUTPUT_START          = QCARCAM_BUFFERLIST_ID_OUTPUT_RANGE,
    QCARCAM_BUFFERLIST_ID_OUTPUT_0              = QCARCAM_BUFFERLIST_ID_OUTPUT_START,

    /** Input buffer lists for injection start at this index */
    QCARCAM_BUFFERLIST_ID_INPUT_START           = QCARCAM_BUFFERLIST_ID_INPUT_RANGE,
    QCARCAM_BUFFERLIST_ID_INPUT_0               = QCARCAM_BUFFERLIST_ID_INPUT_START,

    /** Input metadata bufferlists start at this index */
    QCARCAM_BUFFERLIST_ID_INPUT_METADATA_START  = QCARCAM_BUFFERLIST_ID_INPUT_METADATA_RANGE,
    QCARCAM_BUFFERLIST_ID_INPUT_METADATA        = QCARCAM_BUFFERLIST_ID_INPUT_METADATA_START,

    /** Output metadata bufferlists start at this index */
    QCARCAM_BUFFERLIST_ID_OUTPUT_METADATA_START = QCARCAM_BUFFERLIST_ID_OUTPUT_METADATA_RANGE,
    QCARCAM_BUFFERLIST_ID_OUTPUT_METADATA       = QCARCAM_BUFFERLIST_ID_OUTPUT_METADATA_START,

    /** Input/Output buffer lists start at this index. These are exclusive to ISP reprocessing */
    QCARCAM_BUFFERLIST_ID_INPUT_OUTPUT_START    = QCARCAM_BUFFERLIST_ID_INPUT_OUTPUT_RANGE,

    QCARCAM_BUFFERLIST_ID_MAX = 0x7FFFFFFF          /**< Last or invalid parameter type. */
} QCarCamBufferListId_e;

/** @brief Input Event payload definition */
typedef enum
{
    QCARCAM_INPUT_SIGNAL_VALID = 0,   /**< Signal is locked/valid */
    QCARCAM_INPUT_SIGNAL_LOST,        /**< Signal is not locked or was lost */

    QCARCAM_INPUT_SIGNAL_MAX = 0x7FFFFFFF    /**< Last or invalid parameter type. */
} QCarCamInputSignal_e;

/** @brief Error event type definition. */
typedef enum
{
    /**
    Description:
     Self recoverable error like single bit error correction.
     Following fields of QCarCamErrorInfo_t are applicable :
     errorId, errorCode, errorSource, inputId, frameId, bufferlistId and timestamp.

    Impact:
      No stream or request will be impacted.

    Recovery:
     it doesn't need any recovery.
    */
    QCARCAM_ERROR_WARNING,

    /**
    Description:
     This is fatal error and entire camera subsystem will be non functional,
     SMMU error, CAMNOC, CAM_CC etc. are the possible errors which will bring
     entire camera subsystem down.
     Following fields of QCarCamErrorInfo_t are applicable :
     errorId, errorCode, errorSource and timestamp.

    Impact:
      Complete camera session will be down and no further request will be processed.

    Recovery:
     Reset will be required to recover the stream.
    */
    QCARCAM_ERROR_SUBSYSTEM_FATAL,

    /**
    Description:
     This is fatal error for a particular stream, like parity error etc.
     Following fields of QCarCamErrorInfo_t are applicable :
     errorId, errorCode, errorSource, inputId, frameId, bufferlistId and timestamp.

    Impact:
      It will impact one or more streams.
      No further request will be processed  for the impacted streams untill recovered.

    Recovery:
     Reset will be required to recover the impacted streams.
    */
    QCARCAM_ERROR_FATAL,

    /**
    Description:
     This is transient error , like MIPI CRC, FRAME FREEZE error etc.
     Following fields of QCarCamErrorInfo_t are applicable :
     errorId, errorCode, errorSource, inputId, frameId, bufferlistId and timestamp.

    Impact:
      It will impact one or more streams.

    Recovery:
     Stream will recover automatically once source of error is recovered.
    */
    QCARCAM_ERROR_TRANSIENT,

    /**
    Description:
     Request error will be generated if request is invalid , i.e wrong param.
     Following fields of QCarCamErrorInfo_t are applicable :
     requestId.

    Impact:
      Only particular request will be impacted and next valid request can be processed.

    Recovery:
     It will recover automatically if next request is valid.
    */
    QCARCAM_ERROR_REQUEST,

    /**<
    Description:
     This is fatal error for a particular stream, like CCIF protocol violation etc.
     and qcarcam driver has initiated internal Recovery for the same. Client can send new
     request at recovery state also to minimize recovery time.
     Following fields of QCarCamErrorInfo_t are applicable :
     errorId, errorCode, errorSource, inputId, frameId, bufferlistId and timestamp.

    Impact:
      Only particular request will be impacted and next valid request can be processed.

    Recovery:
     Internal recovery triggered by qcarcam driver, on completion
     RECOVERY event will be sent to notify Success Or failure.
     On failure client is not expected to send further request to server.
    */
    QCARCAM_ERROR_RECOVERABLE,

    /**
    Description:
    Broken connection error will be generated if camera driver is killed.

    Impact:
    Client application will not get any replies from the server and subsequent ipc attempts will result in failure.

    Recovery:
    This is fatal error, relaunch the camera driver and reinitialize the client application.
    */
    QCARCAM_ERROR_BROKEN_CONN
} QCarCamErrorEvent_e;

/** @brief Recover Message Id */
typedef enum
{
   QCARCAM_RECOVERY_STARTED = 0, /**< Recovery started */
   QCARCAM_RECOVERY_SUCCESS,     /**< Recovery completed successfully */
   QCARCAM_RECOVERY_FAILED       /**< Recovery failed - handle in error state */
} QCarCamEventRecoveryMsg_e;


/** @brief Priority levels
 */
typedef enum
{
    QCARCAM_PRIO_UNDEF = 0U,
    QCARCAM_PRIO_LOW,
    QCARCAM_PRIO_HIGH,
} QCarCamPriorityLvl_e;

/** @brief Specify how the output metadata needs to be published */
typedef enum
{
    QCARCAM_METADATA_PUBLISH_OFF,               /**< Do not publish this metadata */
    QCARCAM_METADATA_PUBLISH_ONLY_COMMON,       /**< Publish only to the common metadata associated with the request */
} QCarCamMetadataPublishMode_e;

/** @brief Metadata Tag Groups to specify the group of metadata tags to be configured */
typedef enum
{
    /**
    Description:
    Undefined configuration

    Metadata Tags affected by configuring this group:
    None

    Supported Metadata Publish Modes:
    None
    */
    QCARCAM_METADATA_TAG_GROUP_UNDEF = 0U,

    /**
    Description:
    Config for One Time LDC Metadata

    Metadata Tags affected by configuring this group:
    #QCARCAM_METADATA_TAG_ICA_CAMERA_MATRIX
    #QCARCAM_METADATA_TAG_ICA_IN_GRID_OUT_2_IN_TRANSFORM

    Supported Metadata Publish Modes:
    #QCARCAM_METADATA_PUBLISH_OFF
    #QCARCAM_METADATA_PUBLISH_ONLY_COMMON

    Expected Time of Delivery:
    Alongside first frame once streaming starts.
    It is sent again with the first frame after a mode switch.
    */
    QCARCAM_METADATA_TAG_GROUP_ONE_TIME_LDC,
} QCarCamMetadataTagGroup_e;

/*=================================================================================================
** Typedef Structures
=================================================================================================*/

/** @brief QCarCam API initialization definition. */
typedef struct
{
    uint32_t flags;         /**< QCarCam initialization flags. */
    uint32_t apiVersion;    /**< QCarCam API version. */
} QCarCamInit_t;

/** @brief QcarCam API extended get parameter definition. */
typedef struct
{
    QCarCamParamType_e param; /**< Specifies the parameter to get. */
    void *pValue;         /**< Points to the data that will be retrieved. The payload type must match the expected type
                             for the parameter as documented in #QCarCamParamType_e */
    uint32_t size;  /**< Size of memory in bytes pointed to by pValue; The value of size shall match the expected size
                         for the parameter as documented in #QCarCamParamType_e */
    uint32_t flag;  /**<  bitmask of QCARCAM_PARAM_EX_FLAG_* bits */

    /** union of possible value depending on #flag */
    union
    {
        uint32_t inputId; /**< inputId if flags set to QCARCAM_PARAM_EX_FLAG_INPUT */
        uint32_t bufferlistId; /**< bufferlistId of output if flags set to QCARCAM_PARAM_EX_FLAG_OUTPUT */
    }u;
} QCarCamGetParamEx_t;

/** @brief QcarCam API extended set parameter definition. */
typedef struct
{
    QCarCamParamType_e param; /**< Specifies the parameter to set. */
    const void *pValue; /**< Points to the data that will be set. The payload type must match the expected type
                             for the parameter as documented in #QCarCamParamType_e */
    uint32_t size;  /**< Size of memory in bytes pointed to by pValue; The value of size shall match the expected size
                         for the parameter as documented in #QCarCamParamType_e */
    uint32_t flag;  /**<  bitmask of QCARCAM_PARAM_EX_FLAG_* bits */

    /** union of possible value depending on #flag */
    union
    {
        uint32_t inputId; /**< inputId if flags set to QCARCAM_PARAM_EX_FLAG_INPUT */
        uint32_t bufferlistId; /**< bufferlistId of output if flags set to QCARCAM_PARAM_EX_FLAG_OUTPUT */
    }u;
} QCarCamSetParamEx_t;

/** @brief ISP instance config parameters. */
typedef struct
{
    uint32_t  id;                  /**< ISP instance id. */
    uint32_t  cameraId;            /**< ISP camera id. */
    QCarCamIspUsecase_e usecaseId;    /**< ISP use case. */
    uint32_t  tuningMode;          /**< tuning mode */
} QCarCamIspUsecaseConfig_t;

/** @brief An input stream is defined by the source of an input in a specific mode */
typedef struct
{
    uint32_t inputId;   /**< Input identifier */
    uint32_t srcId;     /**< Input source identifier. See #QCarCamInputSrc_t */
    uint32_t inputMode; /**< The input mode id is the index into #QCarCamInputModes_t pModex*/
} QCarCamInputStream_t;

/** @brief QCarCamOpen parameter. */
typedef struct
{
    QCarCamOpmode_e opMode; /**< OPMODE to indicate usecase */
    uint32_t  priority;     /**< priority of the stream */
    uint32_t  flags;        /**< bitmask of QCARCAM_OPEN_FLAGS_* */
    QCarCamInputStream_t  inputs[QCARCAM_MAX_INPUT_STREAMS]; /**< set of input sources */
    uint32_t  numInputs;    /**< number of inputs[] */
    uint32_t  clientId;     /**< client id, used for multi client usecase, set to 0 by default for single client usecase */
} QCarCamOpen_t;

/** @brief Per stream request specifies the buffer used as well as any settings metadata */
typedef struct
{
    uint32_t bufferlistId; /**< buffer list ID */
    uint32_t bufferIdx;    /**< buffer index within the buffer list. QCARCAM_BUFFERLIST_ID_OUTPUT or
                                QCARCAM_BUFFERLIST_ID_INPUT_OUTPUT ranges */

    uint32_t metaBufferlistId; /**< metadata buffer list ID. Must be in QCARCAM_BUFFERLIST_ID_INPUT_METADATA range*/
    uint32_t metaBufferId;     /**< metadata buffer index within the meta buffer list */
} QCarCamStreamRequest_t;


/** @brief Buffer request for specifies the buffer identifier */
typedef struct
{
    uint32_t bufferlistId; /**< buffer list ID */
    uint32_t bufferIdx;    /**< buffer index within the buffer list*/
} QCarCamBufferRequest_t;

/** @brief Capture request definition
 *
 * The request is identified by the requestId and will correspond to a specific input frame.
 *
 * An input buffer may be passed for processing or reprocessing usecase. In such a usecase the flags must have the
 * QCARCAM_REQUEST_FLAG_INJECTION bit set.
 *
 * For output buffer requests, streamRequests may be filled with the output buffers and associated stream settings.
 * If for multiple output usecases, an output buffer is not set for one of the outputs, it will be dropped.
 *
 * Overall Settings to be applied to all individual inputs may be passed as inputCommonMetadata. If not passed then the last passed setting
 * would take effect.
 *
 * Settings for the individual inputs used may also be passed as inputMetadata. If not passed then the last passed setting
 * would take effect.
 *
 * Output metadata settings to be filled for the individual outputs may be passed.
 */
typedef struct
{
    uint32_t requestId;       /**< unique request Id that monotonically increases */

    QCarCamBufferRequest_t inputBuffer;  /**< input buffer for injection usecase */

    QCarCamStreamRequest_t streamRequests[QCARCAM_MAX_OUTPUT_STREAMS]; /**< output buffer indices */
    uint32_t numStreamRequests; /**< number of per streamRequests[] */

    QCarCamBufferRequest_t inputCommonMetadata; /**< common input metadata to be applied to all inputs */

    QCarCamBufferRequest_t inputMetadata[QCARCAM_MAX_INPUT_STREAMS];   /**< input metadata for each of the individual inputs.
                                                 The bufferlistId must be within QCARCAM_BUFFERLIST_ID_INPUT_METADATA range or
                                                 it will be taken as not set */

    QCarCamBufferRequest_t outputMetadata[QCARCAM_MAX_INPUT_STREAMS];  /**< output metadata for each of the individual inputs
                                                 The bufferlistId must be within QCARCAM_BUFFERLIST_ID_OUTPUT_METADATA range or
                                                 it will be taken as not set */

    uint32_t flags; /**< bitmask of QCARCAM_REQUEST_FLAG_  */

    uint32_t syncId;/**< Sync Id used for request synchronization across multiple cameras that are frame synced.
                         Camera driver shall attempt to process requests that share the same syncId to capture frame synced
                         images from cameras that are configured as part of a frame sync group.

                         A zero value indicates no request synchronization.

                         Request synchronization is only enabled by assigning a non-zero value to syncId.
                         When enabled, syncId starts from 1 and must monotonically increment with each request.
                         If one or more requests for synchronization cannot be met, #QCARCAM_EVENT_FRAME_READY event shall
                         return with the #QCARCAM_BUFFER_STATUS_RSYNC_ERROR buffer flag. */
} QCarCamRequest_t;

/** @brief Rectangle region defined by start x/y offset and width/height */
typedef struct
{
    uint32_t x;  /**< offset from start pixel in x direction */
    uint32_t y;  /**< offset from start line in y direction */
    uint32_t width; /**< width in pixels */
    uint32_t height; /**< height in pixels */
} QCarCamRegion_t;

/** @brief Standalone Injection session configuration */
typedef struct
{
    uint32_t inputId;  /**< Input identifier of the camera using which image is captured */
    uint32_t inputMode; /**< Input mode identifier #QCarCamInputModes_t pModex*/
    uint32_t inputTuningParamFeature1Mode; /**< Input Tuning Parameter Feature 1 Mode */
    uint32_t inputTuningParamFeature2Mode; /**< Input Tuning Parameter Feature 2 Mode */
    uint32_t inputSceneMode; /**< Input Scene Mode */
} QCarCamInjectCfg_t;

/** @brief A source is identified by its source id
 * It is defined by resolution (width/height), color format, frame rate, and
 * security domain.
*/
typedef struct
{
    uint32_t srcId;  /**< identifier of the source stream */

    uint32_t width;  /**< Frame width in pixels. */
    uint32_t height; /**< Frame height in pixels.*/
    QCarCamColorFmt_e   colorFmt;   /**< Color format */
    float   fps;        /**< Frame per second frequency (in hertz). */
    uint32_t securityDomain; /**< @note to be defined */
} QCarCamInputSrc_t;

/** @brief Input mode definition.
 * An input mode can comprise of 1 or more sources.
 * For instance, fused HDR mode may have 1 source, but a non-fused mode may have
 * a separate source for each exposure.
*/
typedef struct {
    QCarCamInputSrc_t sources[QCARCAM_INPUT_MAX_NUM_SOURCES];   /**< Definition of available input sources for this mode. */
    uint32_t          numSources;                               /**< Number of sources available for this mode. */
} QCarCamMode_t;

/** @brief Input mode query structure
 *  QCarCamQueryInputModes() would return QCarCamMode_t[numModes]
*/
typedef struct
{
    uint32_t currentMode;  /**< current mode set for an input */
    uint32_t numModes;     /**< number of modes to query (size of pModes[]). */
    QCarCamMode_t *pModes; /**< pointer to memory that will hold up to QCarCamMode_t[numModes] */
} QCarCamInputModes_t;

/** @brief Input definition
 *  QCarCamQueryInputs() would return QCarCamInput_t[]
 *  QCarCamQueryInputModes() would return QCarCamMode_t[numModes]
*/
typedef struct
{
    uint32_t      inputId;                                /**< Input Identifier. */
    uint32_t      devId;                                  /**< Input device Id */
    uint32_t      subdevId;                               /**< Input sub device Id */
    char          inputName[QCARCAM_INPUT_NAME_MAX_LEN];  /**< Input name. */
    uint32_t      numModes;                               /**< Number of input modes
                                                               in the modes list. */
    uint32_t      flags;                                  /**< bitmask of QCARCAM_INPUT_FLAG_ */
} QCarCamInput_t;


/** @brief Buffer plane definition. */
typedef struct
{
    uint32_t    width;      /**< Width in pixels. */
    uint32_t    height;     /**< Height in pixels. */
    uint32_t    stride;     /**< Stride in bytes. */
    uint32_t    size;       /**< Size in bytes. */
    uint64_t    memHndl;    /**< Buffer for the plane. */
    uint32_t    offset;     /**< Offset in bytes from start of buffer to the plane. */
} QCarCamPlane_t;

/** @brief Buffer definition. */
typedef struct
{
    QCarCamPlane_t  planes[QCARCAM_MAX_NUM_PLANES]; /**< List of all planes in a buffer. */
    uint32_t        numPlanes;                      /**< Number of planes in a buffer. */
} QCarCamBuffer_t;

/** @brief Buffer list definition. */
typedef struct
{
    uint32_t            id;         /**< Buffer list ID associated with the output buffer of an
                                         input. */

    QCarCamColorFmt_e   colorFmt;   /**< Color format type. */
    QCarCamBuffer_t     *pBuffers;  /**< List of buffers. */
    uint32_t            nBuffers;   /**< Number of buffers in a list. */
    uint32_t            flags;      /**< Bitmask of the QCARCAM_BUFFER_FLAG_* macro. */
} QCarCamBufferList_t;

/** @brief Hardware timestamp definition. */
typedef struct
{
    uint64_t timestamp;         /**< Hardware timestamp (in nanoseconds). */
    uint64_t timestampGPTP;     /**< Generic Precision Time Protocol (GPTP) timestamp in nanoseconds. */
} QCarCamHWTimestamp_t;

/** @brief Information for a batch frame */
typedef struct
{
    uint32_t               seqNo;  /**< frame number */
    QCarCamHWTimestamp_t   timestamp; /**< hw timestamp */
} QCarCamBatchFramesInfo_t;

/** @brief Information about the entire batch */
typedef struct
{
    uint32_t frameCount; /**< number of valid frames in current batch*/
} QCarCamBatchResult_t;

/** @brief Frame buffer ready payload definition. */
typedef struct
{
    uint32_t                id;             /**< Buffer list ID. */
    uint32_t                bufferIndex;    /**< Index of the QCarCamBuffer_t pBuffers list.  */
    uint32_t                seqNo;          /**< Sequence number (i.e., frame ID). */
    uint64_t                timestamp;      /**< Software based timestamp for frame ready. */
    QCarCamHWTimestamp_t    sofTimestamp;   /**< Hardware Start of Frame (SoF) timestamp. */
    uint32_t                flags;          /**< Refer to QCARCAM_BUFFER_STATUS_* for flag details. */
    QCarCamInterlaceField_e fieldType;      /**< Interlaced field information. */

    uint32_t                requestId;      /**< submit frame requestId. */
    uint32_t                inputMetaIdx;   /**< intput metadata consumed for the request. */
    uint32_t                outputMetaIdx;  /**< output metadata produced by the request. */

    union
    {
        QCarCamBatchFramesInfo_t batchFramesInfo[QCARCAM_MAX_BATCH_FRAMES];  /**< detailed info for each frame in batch */
        QCarCamBatchResult_t     batchResult;                                /**< detailed info about the current batch returned */
    };

} QCarCamFrameInfo_t;

/** @brief Frame buffer info for an individual frame. */
typedef struct
{
    uint32_t                id;             /**< Buffer list ID. */
    uint32_t                bufferIndex;    /**< Index of the QCarCamBuffer_t pBuffers list.  */
    uint32_t                seqNo;          /**< Sequence number (i.e., frame ID). */
    uint32_t                flags;          /**< Refer to QCARCAM_BUFFER_STATUS_* for flag details. */

} QCarCamSingleFrameInfo_t;

/** @brief Frame buffer ready payload definition for multi-frame mode(QCARCAM_OPEN_FLAGS_MULTI_FRAME_INFO flag is enabled). */
typedef struct
{
    QCarCamSingleFrameInfo_t batchFrameInfo[QCARCAM_MAX_OUTPUT_STREAMS];
    uint32_t                 numFrameInfo;   /**< number of valid frame info in batchFrameInfo[] */
    uint64_t                 timestamp;      /**< Software based timestamp for frame ready. */
    QCarCamHWTimestamp_t     sofTimestamp;   /**< Hardware Start of Frame (SoF) timestamp. */
    uint32_t                 requestId;      /**< submit frame requestId. */
    uint32_t                 inputMetaIdx;   /**< intput metadata consumed for the request. */
    uint32_t                 outputMetaIdx;  /**< output metadata produced by the request. */

} QCarCamMultiFrameInfo_t;

/** @brief Controls latency Q for the output */
typedef struct
{
    uint32_t  latencyMax; /**< Max buffer latency in frame done Q. */
    uint32_t  latencyReduceRate; /**< Number of buffers to drop when max latency reached. */
} QCarCamLatencyControl_t;

/** @brief Vendor parameter and vendor event payload definition*/
typedef struct
{
    uint8_t    data[QCARCAM_MAX_VENDOR_PAYLOAD_SIZE];  /**< Vendor data */
} QCarCamVendorParam_t;

/** @brief List of events to subscribe to */
typedef struct
{
    uint32_t numParams; /**< List of entries in params */
    uint32_t* params;   /**< List of #QCarCamParamType_e to subscribe to */
}QCarCamEventSubscribe_t;

/** @brief Error event payload definition. */
typedef struct
{
    QCarCamErrorEvent_e       errorId;      /**< Error ID type as defined by #QCarCamErrorEvent_e. */
    uint32_t                  errorCode;    /**< Error code from QTI Safety Manual. */
    uint32_t                  errorSource;  /**< Error source from Camera HW IPs listed safety manual. */
    uint32_t                  inputId;      /**< Input identifier if applicable. */
    uint32_t                  frameId;      /**< Frame Id associated with error if applicable. */
    uint32_t                  bufferlistId; /**< BufferlistId to inform erreneous buffer. */
    uint32_t                  requestId;    /**< Request id which got failed , only aplicable for QCARCAM_ERROR_REQUEST. */
    uint64_t                  timestamp;    /**< SW timestamp in microsecond at error detection. */
    /** union of possible value depending on #errorCode */
    union
    {
        uint64_t errorCount;   /**< per frame errorCount if errorCode representing CRC error */
    }u;
} QCarCamErrorInfo_t;

/** @brief Recovery event payload definition */
typedef struct
{
    QCarCamEventRecoveryMsg_e  msg;   /**< Recovery message id */
    QCarCamErrorInfo_t         error; /**< Error event information */
} QCarCamRecovery_t;

/** @brief Multiclient session specific event payload definition */
typedef struct
{
    QCarCamMCEvent_e    event;
    uint32_t            bufferListId[QCARCAM_MAX_OUTPUT_STREAMS];
    uint32_t            numStreams;
} QCarCamMCNotify_t;

/**
 * @brief Contains possible values for the event payload in QCarCam_EventCallback_t().
 *
 * @verbatim

           EVENT ID                        |       TYPE              |    NOTE
    ---------------------------------------------------------------------------
    QCARCAM_EVENT_FRAME_READY              | QCarCamFrameInfo_t      |
    QCARCAM_EVENT_INPUT_SIGNAL             | QCarCamInputSignal_e    |  u32Data
    QCARCAM_EVENT_ERROR                    | QCarCamErrorInfo_t      |
    QCARCAM_EVENT_VENDOR                   | QCarCamVendorParam_t    |
    QCARCAM_EVENT_PROPERTY_NOTIFY          | QCarCamParamType_e      |  u32Data
    QCARCAM_EVENT_FRAME_SOF                | QCarCamHWTimestamp_t    |
    QCARCAM_EVENT_RECOVERY                 | QCarCamRecovery_t       |
    QCARCAM_EVENT_MC_NOTIFY                | QCarCamMCNotify_t       |
    QCARCAM_EVENT_MULTI_STREAM_FRAME_READY | QCarCamMultiFrameInfo_t |

 * @endverbatim
*/
typedef union
{
    uint32_t                u32Data;           /**< uint32_t type */
    QCarCamErrorInfo_t      errInfo;           /**< Error info type */
    QCarCamHWTimestamp_t    hwTimestamp;       /**< HW timestamp */
    QCarCamVendorParam_t    vendorData;        /**< vendor data paylaod. */
    QCarCamFrameInfo_t      frameInfo;         /**< Frame info. */
    QCarCamRecovery_t       recovery;          /**< recovery payload. */
    QCarCamMCNotify_t       mcEventInfo;       /**< Multiclient session event payload */
    QCarCamMultiFrameInfo_t multiFrameInfo;    /**< MultiStream Frame Info */
    uint8_t array[QCARCAM_MAX_PAYLOAD_SIZE];   /**< max event payload. */
} QCarCamEventPayload_t;

/** @brief Exposure configuration parameter
 *  T1 exposure is the longest and each successive exposure being shorter (T1 > T2 > T3 > T4).
 *  The indices in exposureTime and exposureRatio arrays 0,1,2,3 correspond to T1, T2, T3, T4
*/
typedef struct
{
    QCarCamExposureMode_e mode;   /**< exposure mode. */
    uint32_t hdrMode;             /**< hdr mode. */
    uint32_t numExposures;        /**< number of hdr exposures. */
    float exposureTime[QCARCAM_HDR_NUM_EXPOSURES];  /**< time in ms. */
    float exposureRatio[QCARCAM_HDR_NUM_EXPOSURES]; /**< ratio going for successive exposures (T1/T2, T2/T3, T3/T4). */
    float gain[QCARCAM_HDR_NUM_EXPOSURES];           /**< 1.0 to Max supported in sensor. */
}QCarCamExposureConfig_t;

/** @brief Gamma Configuration parameter */
typedef struct
{
    QCarCamGammaType_e type;  /**< gamma configuration mode. */
    float fpValue;            /**< gamma exponent. */
    uint32_t table[QCARCAM_MAX_GAMMA_TABLE]; /**< gamma table. */
    uint32_t tableSize;       /**< size of gamma table. */
}QCarCamGammaConfig_t;

/** @brief Frame drop config parameters */
typedef struct
{
    uint8_t frameDropPeriod;      /**< number of bits in pattern (min value of 1. max value of 32). */
    uint32_t frameDropPattern;    /**< active bit pattern. value of 1 to keep frame, value of 0 to drop frame. */
}QCarCamFrameDropConfig_t;

/** @brief Frame monitoring config parameters */
typedef struct
{
    uint32_t flags;               /**< Enable/disable frame rate monitoring. 0: Disable 1: Enable*/
    uint32_t type;                /**< Input/output frame rate monitoring. 0: Input FPS monitoring, 1: Output FPS monitoring */
    uint32_t inputId;             /**< Input id, should be valid in case of input frame rate monitoring */
    uint32_t bufferListId;        /**< Buffer list id for enabling frame rate monitoring */
    uint32_t fps;                 /**< Required frame rate */
    uint32_t fpsTolerance;        /**< Tolerance limit for fps. Frame rate error will not
                                       occur if fps is within the range i.e. fps +/- fpsTolerance */
}QCarCamFrameRateMonitorConfig_t;

/** @brief Batch mode configuration */
typedef struct
{
    QCarCamBatchMode_e mode;   /**< batch mode. */
    uint32_t numBatchFrames;   /**< number of frames in batch.
                                    Minimum batch size supported is 1.
                                    Maximum batch size is defined by QCARCAM_MAX_BATCH_FRAMES */
    uint32_t frameIncrement;   /**< offset in bytes from frame N's first pixel to frame N+1's first pixel. */
    uint32_t detectFirstPhaseTimer; /**< For special case that requires batching to be started based on specific time difference. */
}QCarCamBatchConfig_t;


/** @brief Diagnostic structure to pass a handle to memory buffer to be filled */
typedef struct
{
    uint32_t size;    /**< buffer size in bytes */
    uint64_t memhndl; /**< memory handle of buffer */
    uint32_t flags;   /**< buffer flags */
}QCarCamDiagInfo_t;

/** @brief ICA LDC EPPROM max FOV configuration
 *
 * LDC EPPROM mode, for specified aspect ratio streams, IPE ICA support output max field of view streams base on below image size configs and
 * EEPROM intrinsic params.
 *
 * distortedWidth usually is configured to sensor output image width, if not configured, API set default value is 0, ICA module will take sensor
 * output image width as distortedWidth. Depend on this input distorted image width, ICA module generates grid.
 *
 * distortedHeight usually is configured to sensor output image height, if not configured, API set default value is 0, ICA module algo will take sensor
 * output image height as distortedHeight. Depend on this input distorted image width, ICA module generates grid.
 *
 * undistortedWidth follows max output stream image width. Depend on this expect output image size, ICA module generates corresponding grid to
 * maximize LDC output streams FOV. If it's not configured, default value is 0, will not enable ICA LDC max FOV function.
 *
 * undistortedHeight follows max output stream image height. Depend on this expect output image size, ICA module generates corresponding grid to
 * maximize LDC output streams FOV. If it's not configured, default value is 0, will not enable ICA LDC max FOV function.
*/
typedef struct
{
    uint32_t distortedWidth;     /**< LDC function input distorted image width in pixels */
    uint32_t distortedHeight;    /**< LDC function input distorted image height in pixels */
    uint32_t undistortedWidth;   /**< LDC max FOV expect output undistorted image width in pixels */
    uint32_t undistortedHeight;  /**< LDC max FOV expect output undistorted image height in pixels */
}QCarCamLDCMaxFOVConfig_t;

/** @brief Specify the required Output Metadata
 *
 * This structure is used to specify the metadata tag groups for which the configuration has to be applied.
*/
typedef struct
{
    QCarCamMetadataTagGroup_e     metadataTagGroup;     /**< Group of Metadata Tag to which this configuration belongs */
    QCarCamMetadataPublishMode_e  publishMode;          /**< Flag used to determine the mode of publising the metadata */
} QCarCamMetadataPublishConfig_t;

/** @} */ /* end_addtogroup qcarcam_datatypes */

/**
 * @cond QCarCamEventCallback_t @endcond
 *
 * @ingroup qcarcam_functions
 *
 * @brief Callback function for handling events.
 *
 * @param[out] hndl          Handle of the input.
 * @param[out] eventId       ID of the event that has resulted from the QCARCAM_EVENT_* macro.
 * @param[out] pPayload      Event payload data associated with the event ID.
 * @param[out] pPrivateData  Handle provided by the client.
 *
 * @details
 * The QCarCam client library calls using this event handler callback whenever any event occurs in
 * the QCarCam system. The client should provide this callback via QCarCamRegisterEventCallback().
 * @par
 * For the pPayload associated with each eventId, check the table provided with
 * #QCarCamEventPayload_t.
 *
 * @return #QCARCAM_RET_OK if successful.
 */
typedef QCarCamRet_e (*QCarCamEventCallback_t)(
    const QCarCamHndl_t hndl,
    const uint32_t eventId,
    const QCarCamEventPayload_t *pPayload,
    void  *pPrivateData);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* QCARCAM_TYPES_H */
