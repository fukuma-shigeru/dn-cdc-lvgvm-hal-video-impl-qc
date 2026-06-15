#ifndef QCARCAM_H
#define QCARCAM_H

/**************************************************************************************************
@file
    qcarcam.h

@brief
    QCarCam API - QTI Automotive Imaging System Proprietary API.

Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
All rights reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

**************************************************************************************************/

/*=================================================================================================
** INCLUDE FILES FOR MODULE
=================================================================================================*/

#include "qcarcam_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*=================================================================================================
** Constant and Macros
=================================================================================================*/

/** @addtogroup qcarcam_constants
@{ */

/** @brief QCarCam major version.
 * The API major version is incremented for major architectural changes and new call flow requirements.
 */
#define QCARCAM_VERSION_MAJOR              6U

/** @brief QCarCam minor version.
 * API compatible; Requires compilation with new minor version and may require small changes to the client application.
 * For example:
 *   - new field to a structure that changes its size
 *   - renamed variable or enum
*/
#define QCARCAM_VERSION_MINOR              4U

/** @brief QCarCam patch version.
 * Binary compatible; Does not require any change or update to the client application.
 * For example:
 *   - new API (function, structure, enum)
 *   - new value to enumerator that does not impact existing values
 *   - new field to a union that does not change its size
 */
#define QCARCAM_VERSION_PATCH              1U


/** @brief QCarCam version type.

    @hideinitializer
*/
#define QCARCAM_VERSION \
    ((QCARCAM_VERSION_PATCH << 12U) | (QCARCAM_VERSION_MAJOR << 8U) | QCARCAM_VERSION_MINOR)


/** @} */ /* end_addtogroup qcarcam_constants */

/*=================================================================================================
** API Functions
=================================================================================================*/

/** @addtogroup qcarcam_functions
@{ */


/**
 * @cond QCarCamInitialize @endcond
 *
 * @brief Initializes the QCarCam interface. Must be the first call to the library.
 *
 * @param[in] pInitParams   Structure required for initialization.
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_UNSUPPORTED If QCarCam API version mismatch is detected; \n
 * #QCARCAM_RET_NOMEM       If not enough system memory available; \n
 * #QCARCAM_RET_NOT_FOUND   If communication to server cannot be established.
 *
 * @note If communication fails to be established, this typically suggests server is either not
 *       running or not fully initialized to accept clients.
 *
 * @see QCarCamUninitialize()
 */
QCarCamRet_e QCarCamInitialize(const QCarCamInit_t *pInitParams);

/**
 * @cond QCarCamUninitialize @endcond
 *
 * @brief Uninitializes the QCarCam interface. This is the last call to the library.
 *
 * @pre QCarCamInitialize() was called.
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADSTATE    If QCarCam interface is not initialized yet; \n
 * #QCARCAM_RET_TIMEOUT     If server does not respond within a certain time.
 *
 * @see QCarCamInitialize()
 */
QCarCamRet_e QCarCamUninitialize(void);

/**
 * @cond QCarCamQueryInputs @endcond
 *
 * @brief Queries the available input information.
 *
 * @pre QCarCamInitialize() was called.
 *
 * @param[out] pInputs  Pointer to the array inputs. If NULL (along with size = 0), *pRetSize
 *                      returns the number of available inputs.
 * @param[in]  size     Number of elements pointed to by pInputs.
 * @param[out] pRetSize If pInputs is set, it is the number of elements in an array that were
 *                      filled.
 *                      If pInputs is NULL, it is the number of available inputs to query.
 *
 * @details
 * It is recommended to first query the number of inputs available, followed by their actual
 * information query. E.g.,:
 * @par
 * -# Upon successful return of QCarCamQueryInputs() (pInputs = NULL, size = 0, &retSize),
 *    retSize shall contain the number of available inputs.
 * -# Call QCarCamQueryInputs() again with the pInput/size at least large enough to hold
 *    the number of inputs returned in Step 1. Upon a successful return, pInput will be
 *    filled up to the *pRetSize entries.
 *
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If QCarCam interface is not initialized yet; \n
 * #QCARCAM_RET_FAILED      If memory operations failed when populating input information. \n
 * #QCARCAM_RET_BUSY        If detection of all available inputs is not yet complete. Will only return available \n
 *                          inputs up to this point in time.
 * @see QCarCamInitialize()
 */
QCarCamRet_e QCarCamQueryInputs(
    QCarCamInput_t *pInputs,
    const uint32_t size,
    uint32_t *pRetSize);

/**
 * @cond QCarCamOpen @endcond
 *
 * @brief Creates a #QCarCamHndl_t session.
 *
 * @pre QCarCamInitialize() was called.
 *
 * @param[in]  pOpenParams  Open parameters for the session.
 * @param[out]  pHndl  Handle to opened session.
 *
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK on success with #QCarCamHndl_t handle filled with valid handle. \n
 * #QCARCAM_RET_BADPARAM  If invalid open parameters are provided \n
 * #QCARCAM_RET_BADSTATE  If QCarCam interface is not initialized. \n
 * #QCARCAM_RET_FAILED    If failure due to other errors such as out of memory
 *
 * @see QCarCamInitialize()
 * @see QCarCamQueryInputs()
 * @see QCarCamClose()
 */
QCarCamRet_e QCarCamOpen(const QCarCamOpen_t* pOpenParams, QCarCamHndl_t* pHndl);

/**
 * @cond QCarCamClose @endcond
 *
 * @brief Closes the #QCarCamHndl_t session.
 *
 * @pre Successful QCarCamOpen() call returning the same handle.
 *
 * @param[in]  hndl    QCarCam handle type to close.
 *
 * @details
 * The call releases resources associated with the handle returned by a successful QCarCamOpen()
 * call.
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If session is not opened.
 *
 * @see QCarCamOpen()
 */
QCarCamRet_e QCarCamClose(const QCarCamHndl_t hndl);

/**
 * @cond QCarCamGetParam @endcond
 *
 * @brief Gets the parameter value from a session represented by the #QCarCamHndl_t.
 *
 * @pre Successful QCarCamOpen() call returning the same handle.
 *
 * @param[in]     hndl    Session handle from which to query the parameter.
 * @param[in]     param   Specifies the parameter to get.
 * @param[in,out] pValue  Points to the data that will be retrieved. The payload type must match the expected type
 *                        for the parameter as documented in #QCarCamParamType_e.
 *                        For certain parameters (e.g. QCARCAM_STREAM_CONFIG_PARAM_FRAME_RATE_MONITOR_CONTROL), the input
 *                        payload is read as it provides additional information on what is to be retrieved.
 * @param[in]     size    Size of memory in bytes pointed to by pValue; The value of size shall match the expected size
 *                        of the parameter as documented in #QCarCamParamType_e
 *
 * @details
 * For the pValue and size parameters, check the table provided with #QCarCamParamType_e.
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If QCarCam interface is not initialized yet; \n
 * #QCARCAM_RET_INVALID_OP  If get parameter request type is invalid; \n
 * #QCARCAM_RET_UNSUPPORTED If the parameter is unsupported; \n
 * #QCARCAM_RET_FAILED      If memory operations failed when populating parameter information.
 *
 * @see QCarCamOpen()
 * @see QCarCamSetParam()
 */
QCarCamRet_e QCarCamGetParam(
    const QCarCamHndl_t hndl,
    const QCarCamParamType_e param,
    void *pValue,
    const uint32_t size);

/**
 * @cond QCarCamGetParamEx @endcond
 *
 * @brief Gets the parameter value from a session represented by the #QCarCamHndl_t.
 *
 * @pre Successful QCarCamOpen() call returning the same handle.
 *
 * @param[in]  hndl    Session handle from which to query the parameter.
 *                     This parameter may be passed as QCARCAM_INVALID_HANDLE with parameter QCARCAM_VENDOR_PARAM
 *                     to get a vendor parameter from a specific input without an active session.
 *
 * @param[in]  pParam  Pointer to externded get param structure
 *
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If QCarCam interface is not initialized yet; \n
 * #QCARCAM_RET_INVALID_OP  If get parameter request type is invalid; \n
 * #QCARCAM_RET_UNSUPPORTED If the parameter is unsupported; \n
 * #QCARCAM_RET_FAILED      If memory operations failed when populating parameter information.
 *
 * @see QCarCamOpen()
 * @see QCarCamGetParam()
 * @see QCarCamSetParam()
 * @see QCarCamSetParamEx()
 */
QCarCamRet_e QCarCamGetParamEx(
    const QCarCamHndl_t hndl,
    const QCarCamGetParamEx_t *pParam);

/**
 * @cond QCarCamSetParam @endcond
 *
 * @brief Sets the parameter value to a session represented by the #QCarCamHndl_t.
 *
 * @pre Successful QCarCamOpen() call returning the same handle.
 *
 * @param[in] hndl    session handle to which to apply the parameter.
 * @param[in] param   Specifies the parameter to set.
 * @param[in] pValue  Points to the data that will be set. The payload type must match the expected type
 *                    for the parameter as documented in #QCarCamParamType_e.
 * @param[in] size    Size of memory in bytes pointed to by pValue; The value of size shall match the expected size
 *                    of the parameter as documented in #QCarCamParamType_e
 *
 * @details
 * For the pValue and size parameters, check the table provided with #QCarCamParamType_e.
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If QCarCam interface is not initialized yet; \n
 * #QCARCAM_RET_INVALID_OP  If set parameter request type is invalid.
 * #QCARCAM_RET_UNSUPPORTED If the parameter is unsupported; \n
 * #QCARCAM_RET_FAILED      If memory operations failed when setting parameter information.
 *
 * @see QCarCamOpen()
 * @see QCarCamGetParam()
 */
QCarCamRet_e QCarCamSetParam(
    const QCarCamHndl_t hndl,
    const QCarCamParamType_e param,
    const void *pValue,
    const uint32_t size);


/**
 * @cond QCarCamSetParamEx @endcond
 *
 * @brief Sets the parameter value to a session represented by the #QCarCamHndl_t.
 *
 * @pre Successful QCarCamOpen() call returning the same handle.
 *
 * @param[in] hndl    Session handle to which to apply the parameter.
 *                    This parameter may be passed as QCARCAM_INVALID_HANDLE with parameter QCARCAM_VENDOR_PARAM
 *                     to set a vendor parameter to a specific input without an active session.
 * @param[in] pParam  Pointer to extended set param structure
 *
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If QCarCam interface is not initialized yet; \n
 * #QCARCAM_RET_INVALID_OP  If set parameter request type is invalid.
 * #QCARCAM_RET_UNSUPPORTED If the parameter is unsupported; \n
 * #QCARCAM_RET_FAILED      If memory operations failed when setting parameter information.
 *
 * @see QCarCamOpen()
 * @see QCarCamGetParam()
 * @see QCarCamGetParamEx()
 */
QCarCamRet_e QCarCamSetParamEx(
    const QCarCamHndl_t hndl,
    const QCarCamSetParamEx_t *pParam);

/**
 * @cond QCarCamSetBuffers @endcond
 *
 * @brief Registers buffers of particular bufferlist for the session.
 *
 * @pre Successful QCarCamOpen() call returning the same handle.
 *
 * @param[in] hndl         Handle of the session.
 * @param[in] pBuffers     Pointer to set the buffer structure.
 *
 * @details
 * This API is used to map client buffers to a specific bufferlist. The available bufferlists are
 * dependent on the operation mode and usecase picked for the session.
 * If a session has more than one output buffer based on its operating mode, individual output
 * buffers will be required to be set for each of the session's selected mode to work.
 * @par
 * This requires multiple calls to QCarCamSetBuffers() per session.
 * @par
 * The maximum number of buffers per session per output mode is defined by #QCARCAM_MAX_NUM_BUFFERS.
 * For ASIL-B operations, the minimum number of buffers per session per output mode is defined
 * by #QCARCAM_MIN_NUM_BUFFERS.
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If QCarCam interface is not initialized yet; \n
 * #QCARCAM_RET_NOMEM       If not enough system memory available; \n
 * #QCARCAM_RET_FAILED      If memory operations failed when reading buffer list provided.
 *
 * @see QCarCamOpen()
 */
QCarCamRet_e QCarCamSetBuffers(const QCarCamHndl_t hndl, const QCarCamBufferList_t *pBuffers);

/**
 * @cond QCarCamGetBuffers @endcond
 *
 * @brief Get the buffers of a particular bufferlist for a multi-client session.
 *
 * @pre Successful QCarCamOpen() call returning the same handle.
 *
 * @param[in] hndl          Handle of the session.
 * @param[in,out] pBuffers  Pointer to get the buffer structure.
 *
 * @details
 * This API is used by the client application to acquire the buffer handles for a specific bufferlist.
 * The available bufferlists are dependent on the operation mode and usecase picked for the session.
 * If a client is to request buffers for more than one bufferlist, the client is required to perform
 * multiple calls of QCarCamGetBuffers().
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If QCarCam interface is not initialized yet; \n
 * #QCARCAM_RET_NOMEM       If not enough system memory available; \n
 * #QCARCAM_RET_FAILED      If memory operations failed when reading buffer list provided.
 *
 * @see QCarCamOpen()
 */
QCarCamRet_e QCarCamGetBuffers(const QCarCamHndl_t hndl, QCarCamBufferList_t *pBuffers);

/**
 * @cond QCarCamReleaseBuffers @endcond
 *
 * @brief Release the buffers of a particular bufferlist for a multi-client session.
 *
 * @pre Successful QCarCamGetBuffers() call for the bufferlist
 *
 * @param[in] hndl          Handle of the session.
 * @param[in] bufferlistId  Bufferlist ID associated with the session handle obtained from
 *                          QCarCamGetBuffers() via the #QCarCamFrameInfo_t structure.
 *
 * @details
 * This API is used by the client application to release the buffer handles for a specific bufferlist.
 * If a client has requested buffers via QCarCamGetBuffers for more than one bufferlist, the client is
 * required to perform multiple calls of QCarCamReleaseBuffers().
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If QCarCam interface is not initialized yet; \n
 *
 * @see QCarCamGetBuffers()
 */
QCarCamRet_e QCarCamReleaseBuffers(const QCarCamHndl_t hndl, const uint32_t bufferlistId);

/**
 * @cond QCarCamStart @endcond
 *
 * @brief Starts streaming on the session represented by the #QCarCamHndl_t.
 *
 * @pre Populate necessary buffers for the session using QCarCamSetBuffers().
 *
 * @param[in]  hndl    Handle of the session.
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If start was requested in the wrong state or QCarCam interface is not
 *                          initialized yet.; \n
 * #QCARCAM_RET_NOMEM       If no buffers set for the specified session.
 *
 * @see QCarCamSetBuffers()
 */
QCarCamRet_e QCarCamStart(const QCarCamHndl_t hndl);

/**
 * @cond QCarCamStop @endcond
 *
 * @brief Stops the streaming of the #QCarCamHndl_t session
 *
 * @pre Successful QCarCamStart() call on the same handle.
 *
 * @param[in]  hndl    Handle of the session.
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If session is not started or paused or QCarCam interface is not
 *                          initialized yet.
 *
 * @see QCarCamStart()
 */
QCarCamRet_e QCarCamStop(const QCarCamHndl_t hndl);

/**
 * @cond QCarCamPause @endcond
 *
 * @brief Pauses the session that was started without relinquishing resources.
 *
 * @pre Successful QCarCamStart() call on the same handle.
 *
 * @param[in]  hndl    Handle of the session.
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If session is not started or QCarCam interface is not initialized yet.
 *
 * @see QCarCamStart()
 * @see QCarCamResume()
 */
QCarCamRet_e QCarCamPause(const QCarCamHndl_t hndl);

/**
 * @cond QCarCamResume @endcond
 *
 * @brief Resumes the session that was paused.
 *
 * @pre Successful QCarCamPause() call on the same handle.
 *
 * @param[in]  hndl    Handle of the session.
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If session is not paused or QCarCam interface is not initialized yet.
 *
 * @see QCarCamPause()
 */
QCarCamRet_e QCarCamResume(const QCarCamHndl_t hndl);

/**
 * @cond QCarCamGetFrame @endcond
 *
 * @brief Gets the available frames for the session represented by the #QCarCamHndl_t.
 *
 * @pre Successful QCarCamStart() call on the same handle.
 *
 * @param[in]  hndl        Handle of the session.
 * @param[in,out] pFrameInfo  Pointer to the frame information that will be filled. \n
 *                           The bufferlist id will indicate which bufferlist will be dequeued from.
 * @param[in]  timeout     Maximum wait time in nanoseconds for the frame to be available
 *                         before timeout.
 * @param[in]  flags       Flags for getting the frame (reserved for future use).
 *
 * @details
 * Input frames can only be fetched after a successful QCarCamStart() call - required after either
 * QCarCamOpen() or QCarCamStop() of the same session. Similarly, call to QCarCamResume() is required
 * to fetch frames if session was paused via QCarCamPause().
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If session is not started or QCarCam interface is not initialized yet; \n
 * #QCARCAM_RET_TIMEOUT     If next frame not available within the specified timeout requested.
 *
 * @see QCarCamStart()
 * @see QCarCamReleaseFrame()
 */
QCarCamRet_e QCarCamGetFrame(
    const QCarCamHndl_t hndl,
    QCarCamFrameInfo_t *pFrameInfo,
    const uint64_t timeout,
    const uint32_t flags);

/**
 * @cond QCarCamReleaseFrame @endcond
 *
 * @brief Signals that the released bufferIndex can be reused for any future frames for session
 *        represented by the #QCarCamHndl_t.
 *
 * @pre Successful QCarCamGetFrame() call on the same handle.
 *
 * @param[in] hndl         Handle of the session.
 * @param[in] id           Buffer list ID associated with the session handle obtained from
 *                         QCarCamGetFrame() via the #QCarCamFrameInfo_t structure.
 * @param[in] bufferIndex  Index into the #QCarCamBufferList_t buffers list to re-enqueue buffer
 *                         for the specified buffer list ID.
 *
 * @details
 * Upon a successful return of QCarCamGetFrame(), and the buffer identified by <id, bufferIndex>
 * has been processed, the buffer should be released back to the QCarcam library using
 * QCarCamReleaseFrame() as soon as possible so that new input frames can be fetched.
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If requested buffer is not expected to be released yet.
 *
 * @see QCarCamGetFrame()
 */
QCarCamRet_e QCarCamReleaseFrame(
    const QCarCamHndl_t hndl,
    const uint32_t id,
    const uint32_t bufferIndex);


/**
 * @cond QCarCamRegisterEventCallback @endcond
 *
 * @brief Register a QCarCam event callback for system wide or per session events.
 *
 * @pre Successful QCarCamOpen() call returning the same handle.
 *
 * @param[in]  hndl            Handle of the session. Set to NULL if registering a system wide event
 *                             callback.
 * @param[in]  callbackFunc    Callback function provided by the client.
 * @param[in]  pPrivateData    Pointer to the private data provided by the client. Will be returned
 *                             with each system or session event.
 *
 * @details
 * The QCarCam API supports one system wide event callback to be registered after initializing the
 * QCarCam API using QCarCamInitialize(). The QCarCam API uninitializing using
 * QCarCamUninitialize() will automatically unregister all event callbacks registered by the
 * client.
 * @par
 * The QCarCam API supports one session event callback to be registered per session after a session has
 * been opened successfully using QCarCamOpen().
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If QCarCam interface is not initialized yet;
 *
 * @see QCarCamUnregisterEventCallback()
 */
QCarCamRet_e QCarCamRegisterEventCallback(
    const QCarCamHndl_t hndl,
    const QCarCamEventCallback_t callbackFunc,
    void  *pPrivateData);

/**
 * @cond QCarCamUnregisterEventCallback @endcond
 *
 * @brief Unregister a QCarCam event callback for system wide or per session events.
 *
 * @pre Successful QCarCamRegisterEventCallback() call on the same handle.
 *
 * @param[in]  hndl    Handle of the session. Set to NULL if unregistering a system wide event
 *                     callback.
 *
 * @details
 * A system wide event callback can be unregistered only after all sessions have been closed using
 * QCarCamClose().
 * @par
 * A session event callback can be unregistered only after that session has been closed using
 * QCarCamClose().
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If QCarCam interface is not initialized yet.
 *
 * @see QCarCamRegisterEventCallback()
 */
QCarCamRet_e QCarCamUnregisterEventCallback(const QCarCamHndl_t hndl);

/**
 * @cond QCarCamReserve @endcond
 *
 * @brief Reserves resources required for the usecase for the session represented by the #QCarCamHndl_t.
 *
 * @pre Populate necessary buffers for the session using QCarCamSetBuffers() and populated all other parameters
 *      required to define the usecase.
 *
 * @param[in] hndl         Handle of the session.
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If in invalid state when calling this API
 *
 * @see QCarCamRelease()
 */
QCarCamRet_e QCarCamReserve(const QCarCamHndl_t hndl);

/**
 * @cond QCarCamRelease @endcond
 *
 * @brief Releases resources previously reserved for the session represented by the #QCarCamHndl_t.
 *
 * @pre Successful QCarCamReserve() on the same handle.
 *
 * @param[in] hndl         Handle of the session.
 *
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If requested buffer is not expected to be released yet.
 *
 * @see QCarCamReserve()
 */
QCarCamRet_e QCarCamRelease(const QCarCamHndl_t hndl);

/**
 * @cond QCarCamQueryInputModes @endcond
 *
 * @brief Queries available modes for the input represented by inputId.
 *
 * @pre Successful QCarCamInitialize()
 *
 * @param[in] inputId         Input identifier for which to enumerate the available modes.
                              Available inputs can be queried using QCarCamQueryInputs().
 * @param[out] pInputModes    Pointer to structure that will hold the enumerated modes. Number of modes
                              available can be queried using QCarCamQueryInputs().
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 *
 * @see QCarCamInitialize() and QCarCamQueryInputs()
 */
QCarCamRet_e QCarCamQueryInputModes(
    const uint32_t inputId,
    QCarCamInputModes_t* pInputModes);

/**
 * @cond QCarCamSubmitRequest @endcond
 *
 * @brief Submits a frame request for the input represented by the #QCarCamHndl_t. The frame request
 *         includes metadata settings to be applied for the frame as well as list of input/output buffers
 *
 * @pre Successful QCarCamStart()
 *
 * @param[in] hndl         Handle of the session.
 * @param[in] pRequest     Pointer to request structure
 *
 * @details
 * The request is identified by a unique requestId. The frame will be delivered asynchronously through the QCARCAM_EVENT_FRAME_READY
 * and the requestId will be set in the QCarCamFrameInfo_t
 *
 * @return An appropriate error as defined in #QCarCamRet_e; more specifically, \n
 * #QCARCAM_RET_OK          Only upon success; \n
 * #QCARCAM_RET_BADPARAM    If invalid parameters are provided; \n
 * #QCARCAM_RET_BADSTATE    If handle is not in correct state
 *
 * @see
 */
QCarCamRet_e QCarCamSubmitRequest(const QCarCamHndl_t hndl, const QCarCamRequest_t* pRequest);

/**
 * @cond QCarCamQueryDiagnostics @endcond
 *
 * @brief Queries System Status and Diagnostics
 *
 * Able to query system status anytime after initializing QCarCam API using QCarCamInitialize().
 *
 * @param      pDiag  structure to pass memory handle of buffer to be filled by QCarCam driver.

 * @return QCARCAM_RET_OK only if successful; check QCarCamRet_e otherwise.
 */
QCarCamRet_e QCarCamQueryDiagnostics(const QCarCamDiagInfo_t* pDiag);

/**
 * @cond QCarCamDiagnosticDump @endcond
 *
 * @brief Dumps System Diagnostic Information
 *
 * Able to dump system diagnostic anytime after initializing QCarCam API using QCarCamInitialize().
 *
 * @return QCARCAM_RET_OK only if successful; check QCarCamRet_e otherwise.
 */
QCarCamRet_e QCarCamDiagnosticDump(void);

/**
 * @cond QCarCamInitiateRecovery @endcond
 *
 * @brief Initiates recovery of streams/resources in error/degraded state.
 *
 * @note Yet to be fully defined
 */
QCarCamRet_e QCarCamInitiateRecovery(void);

/** @} */ /* end_addtogroup qcarcam_functions */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* QCARCAM_H */
