/*
 * @copyright Copyright (c) 2022 Woven Alpha, Inc
 */
/*******************************************************************************
 Function: video_hal test driver common header
 File name: video_hal_ct_common.h
*******************************************************************************/
#ifndef VIDEO_HAL_CT_COMMON_H
#define VIDEO_HAL_CT_COMMON_H

/*--------------------------------------------------------------------------*/
/* includes                                                                 */
/*--------------------------------------------------------------------------*/
#include "halcp_public.h"
#include "ilm/ilm_common.h"
#include "ilm/ilm_control.h"

#define VHAL_CT_IVIID_CARPLAY				(31)
#define VHAL_CT_IVIID_MAP					(210)
#define VHAL_CT_IVIID_FRONT_DTV				(20)	// front DTV surface ID
#define VHAL_CT_IVIID_REAR_DTV				(120)	// rear DTV surface ID
#define VHAL_CT_IVIID_CAMERA_IMG_ADJ		(33)	// front camera_img_adj surface ID

#define VHAL_CT_TIMEOUT_SECONDS				(5)

#define VHAL_CT_FRONT_ID					(0)
#define VHAL_CT_REAR_ID						(1)
#define VHAL_CT_IC_ID						(2)
#define VHAL_CT_IVIID_HMI_LAYER				(4)		//HMI which should be invisible to check video surfaces.

#define VHAL_EXEC_PATH						"/opt/dc-ivi-pf/tests/dn-cdc-lvgvm-hal-video-impl-qc/certification-program/"

/*--------------------------------------------------------------------------*/
/* macro                                                                   */
/*--------------------------------------------------------------------------*/
void wait_msec(int32_t msec);

inline int32_t HALCP_INDICATION(const char *msg) {
//	if(nullptr == getenv("COCKPIT_RUNTIME_ENABLE_CERTIFICATION") ||
//	    0 != strcmp(getenv("COCKPIT_RUNTIME_ENABLE_CERTIFICATION"),"1")) {
//		return 0;
//	} else {
        return halcp::HalcpIndication(msg);
//    }
}

inline int32_t HALCP_WAITINPUTANYKEY(const char *prompt) {
	if(nullptr == getenv("COCKPIT_RUNTIME_ENABLE_CERTIFICATION") ||
	    0 != strcmp(getenv("COCKPIT_RUNTIME_ENABLE_CERTIFICATION"),"1")) {
	    wait_msec(500);
		return 0;
	} else {
        return halcp::HalcpWaitInputAnyKey(prompt);
    }
}

inline int32_t HALCP_WAITINPUTRESULT(const char *prompt, const char *ok_char) {
	if(nullptr == getenv("COCKPIT_RUNTIME_ENABLE_CERTIFICATION") ||
	    0 != strcmp(getenv("COCKPIT_RUNTIME_ENABLE_CERTIFICATION"),"1")) {
	    wait_msec(500);
		return 0;
	} else {
        return halcp::HalcpWaitInputResult(prompt, ok_char);
    }
}

/*--------------------------------------------------------------------------*/
/* typedef                                                                  */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* struct                                                                   */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* prototype                                                                */
/*--------------------------------------------------------------------------*/
void exec_test_video_client(int32_t ivi_id, const char *filename);
void kill_test_video_client();
void set_hmi_visibility(bool visible);
void get_screen_properties(t_ilm_uint screenId, struct ilmScreenProperties *screenProp);

bool get_exec_hdmi(void);
bool get_exec_rear(void);
bool get_exec_ic(void);
bool get_exec_camera(void);
bool get_exec_heacon(void);
bool get_enable_key(void);

#endif /* VIDEO_HAL_CT_COMMON_H */
