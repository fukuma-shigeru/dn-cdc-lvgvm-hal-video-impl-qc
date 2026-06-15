/*******************************************************************************
    Function name : VideoHAL public define module
    File name     : vhal_ctl_define_public.h
*******************************************************************************/
#ifndef	VHAL_CTL_DEFINE_PUBLIC_H
#define	VHAL_CTL_DEFINE_PUBLIC_H

#include <cstdint>
#include <string>

namespace videohal
{

static constexpr uint32_t VHAL_ENV_VAL_MAX		{64U};
static constexpr int32_t  VHAL_EPOLL_SIZE_MAX	{ 32};

/* VideoHAL property name */
static const std::string VHAL_PROP_VPATH_FRONT_CTL_CURRENT			{"vpath.front.control.current"};
static const std::string VHAL_PROP_VPATH_FRONT_STS_CURRENT			{"vpath.front.status.current"};
static const std::string VHAL_PROP_VPATH_FRONT_STS_AVAILABLE		{"vpath.front.status.available"};
static const std::string VHAL_PROP_VPATH_REAR_CTL_CURRENT			{"vpath.rear.control.current"};
static const std::string VHAL_PROP_VPATH_REAR_STS_CURRENT			{"vpath.rear.status.current"};
static const std::string VHAL_PROP_VPATH_REAR_STS_AVAILABLE			{"vpath.rear.status.available"};
static const std::string VHAL_PROP_VPATH_CAMERA_CTL_CURRENT			{"vpath.camera.control.current"};
static const std::string VHAL_PROP_VPATH_CAMERA_STS_CURRENT			{"vpath.camera.status.current"};
static const std::string VHAL_PROP_VPATH_CAMERA_STS_AVAILABLE		{"vpath.camera.status.available"};
static const std::string VHAL_PROP_VPATH_ICLUSTER_CTL_CURRENT		{"vpath.instrumentcluster.control.current"};
static const std::string VHAL_PROP_VPATH_ICLUSTER_STS_CURRENT		{"vpath.instrumentcluster.status.current"};
static const std::string VHAL_PROP_VPATH_ICLUSTER_STS_AVAILABLE		{"vpath.instrumentcluster.status.available"};
static const std::string VHAL_PROP_VPATH_FRONT_CTL_OUTPUT_ON		{"vpath.front.control.output.on"};
static const std::string VHAL_PROP_VPATH_FRONT_CTL_OUTPUT_X			{"vpath.front.control.output.x"};
static const std::string VHAL_PROP_VPATH_FRONT_CTL_OUTPUT_Y			{"vpath.front.control.output.y"};
static const std::string VHAL_PROP_VPATH_FRONT_CTL_OUTPUT_W			{"vpath.front.control.output.width"};
static const std::string VHAL_PROP_VPATH_FRONT_CTL_OUTPUT_H			{"vpath.front.control.output.height"};
static const std::string VHAL_PROP_VPATH_FRONT_STS_OUTPUT_X			{"vpath.front.status.output.x"};
static const std::string VHAL_PROP_VPATH_FRONT_STS_OUTPUT_Y			{"vpath.front.status.output.y"};
static const std::string VHAL_PROP_VPATH_FRONT_STS_OUTPUT_W			{"vpath.front.status.output.width"};
static const std::string VHAL_PROP_VPATH_FRONT_STS_OUTPUT_H			{"vpath.front.status.output.height"};
static const std::string VHAL_PROP_VPATH_REAR_CTL_OUTPUT_ON			{"vpath.rear.control.output.on"};
static const std::string VHAL_PROP_VPATH_REAR_CTL_OUTPUT_X			{"vpath.rear.control.output.x"};
static const std::string VHAL_PROP_VPATH_REAR_CTL_OUTPUT_Y			{"vpath.rear.control.output.y"};
static const std::string VHAL_PROP_VPATH_REAR_CTL_OUTPUT_W			{"vpath.rear.control.output.width"};
static const std::string VHAL_PROP_VPATH_REAR_CTL_OUTPUT_H			{"vpath.rear.control.output.height"};
static const std::string VHAL_PROP_VPATH_REAR_STS_OUTPUT_X			{"vpath.rear.status.output.x"};
static const std::string VHAL_PROP_VPATH_REAR_STS_OUTPUT_Y			{"vpath.rear.status.output.y"};
static const std::string VHAL_PROP_VPATH_REAR_STS_OUTPUT_W			{"vpath.rear.status.output.width"};
static const std::string VHAL_PROP_VPATH_REAR_STS_OUTPUT_H			{"vpath.rear.status.output.height"};
static const std::string VHAL_PROP_VPATH_CAMERA_CTL_OUTPUT_ON		{"vpath.camera.control.output.on"};
static const std::string VHAL_PROP_VPATH_CAMERA_CTL_OUTPUT_X		{"vpath.camera.control.output.x"};
static const std::string VHAL_PROP_VPATH_CAMERA_CTL_OUTPUT_Y		{"vpath.camera.control.output.y"};
static const std::string VHAL_PROP_VPATH_CAMERA_CTL_OUTPUT_W		{"vpath.camera.control.output.width"};
static const std::string VHAL_PROP_VPATH_CAMERA_CTL_OUTPUT_H		{"vpath.camera.control.output.height"};
static const std::string VHAL_PROP_VPATH_CAMERA_STS_OUTPUT_X		{"vpath.camera.status.output.x"};
static const std::string VHAL_PROP_VPATH_CAMERA_STS_OUTPUT_Y		{"vpath.camera.status.output.y"};
static const std::string VHAL_PROP_VPATH_CAMERA_STS_OUTPUT_W		{"vpath.camera.status.output.width"};
static const std::string VHAL_PROP_VPATH_CAMERA_STS_OUTPUT_H		{"vpath.camera.status.output.height"};
static const std::string VHAL_PROP_VPATH_ICLUSTER_CTL_OUTPUT_ON		{"vpath.instrumentcluster.control.output.on"};
static const std::string VHAL_PROP_VPATH_ICLUSTER_CTL_OUTPUT_X		{"vpath.instrumentcluster.control.output.x"};
static const std::string VHAL_PROP_VPATH_ICLUSTER_CTL_OUTPUT_Y		{"vpath.instrumentcluster.control.output.y"};
static const std::string VHAL_PROP_VPATH_ICLUSTER_CTL_OUTPUT_W		{"vpath.instrumentcluster.control.output.width"};
static const std::string VHAL_PROP_VPATH_ICLUSTER_CTL_OUTPUT_H		{"vpath.instrumentcluster.control.output.height"};
static const std::string VHAL_PROP_VPATH_ICLUSTER_STS_OUTPUT_X		{"vpath.instrumentcluster.status.output.x"};
static const std::string VHAL_PROP_VPATH_ICLUSTER_STS_OUTPUT_Y		{"vpath.instrumentcluster.status.output.y"};
static const std::string VHAL_PROP_VPATH_ICLUSTER_STS_OUTPUT_W		{"vpath.instrumentcluster.status.output.width"};
static const std::string VHAL_PROP_VPATH_ICLUSTER_STS_OUTPUT_H		{"vpath.instrumentcluster.status.output.height"};
static const std::string VHAL_PROP_VPATH_REAR_CTL_RSE_DISP			{"vpath.rear.control.rse.display"};
static const std::string VHAL_PROP_VPATH_REAR_CTL_RSE_NOTIFY		{"vpath.rear.control.rse.notification"};
static const std::string VHAL_PROP_VPATH_REAR_STS_RSE_DISP			{"vpath.rear.status.rse.display"};
static const std::string VHAL_PROP_VPATH_REAR_STS_RSE_NOTIFY		{"vpath.rear.status.rse.notification"};
static const std::string VHAL_PROP_VPATH_FRONT_CTL_CLIP_ON			{"vpath.front.control.clipping.on"};
static const std::string VHAL_PROP_VPATH_FRONT_CTL_CLIP_ENABLE		{"vpath.front.control.clipping.enable"};
static const std::string VHAL_PROP_VPATH_FRONT_CTL_CLIP_X			{"vpath.front.control.clipping.x"};
static const std::string VHAL_PROP_VPATH_FRONT_CTL_CLIP_Y			{"vpath.front.control.clipping.y"};
static const std::string VHAL_PROP_VPATH_FRONT_CTL_CLIP_W			{"vpath.front.control.clipping.width"};
static const std::string VHAL_PROP_VPATH_FRONT_CTL_CLIP_H			{"vpath.front.control.clipping.height"};
static const std::string VHAL_PROP_VPATH_FRONT_STS_CLIP_ENABLE		{"vpath.front.status.clipping.enable"};
static const std::string VHAL_PROP_VPATH_FRONT_STS_CLIP_X			{"vpath.front.status.clipping.x"};
static const std::string VHAL_PROP_VPATH_FRONT_STS_CLIP_Y			{"vpath.front.status.clipping.y"};
static const std::string VHAL_PROP_VPATH_FRONT_STS_CLIP_W			{"vpath.front.status.clipping.width"};
static const std::string VHAL_PROP_VPATH_FRONT_STS_CLIP_H			{"vpath.front.status.clipping.height"};
static const std::string VHAL_PROP_VPATH_ICLUSTER_CTL_CLIP_ON		{"vpath.instrumentcluster.control.clipping.on"};
static const std::string VHAL_PROP_VPATH_ICLUSTER_CTL_CLIP_ENABLE	{"vpath.instrumentcluster.control.clipping.enable"};
static const std::string VHAL_PROP_VPATH_ICLUSTER_CTL_CLIP_X		{"vpath.instrumentcluster.control.clipping.x"};
static const std::string VHAL_PROP_VPATH_ICLUSTER_CTL_CLIP_Y		{"vpath.instrumentcluster.control.clipping.y"};
static const std::string VHAL_PROP_VPATH_ICLUSTER_CTL_CLIP_W		{"vpath.instrumentcluster.control.clipping.width"};
static const std::string VHAL_PROP_VPATH_ICLUSTER_CTL_CLIP_H		{"vpath.instrumentcluster.control.clipping.height"};
static const std::string VHAL_PROP_VPATH_ICLUSTER_STS_CLIP_ENABLE	{"vpath.instrumentcluster.status.clipping.enable"};
static const std::string VHAL_PROP_VPATH_ICLUSTER_STS_CLIP_X		{"vpath.instrumentcluster.status.clipping.x"};
static const std::string VHAL_PROP_VPATH_ICLUSTER_STS_CLIP_Y		{"vpath.instrumentcluster.status.clipping.y"};
static const std::string VHAL_PROP_VPATH_ICLUSTER_STS_CLIP_W		{"vpath.instrumentcluster.status.clipping.width"};
static const std::string VHAL_PROP_VPATH_ICLUSTER_STS_CLIP_H		{"vpath.instrumentcluster.status.clipping.height"};
static const std::string VHAL_PROP_VPATH_FRONT_CTL_WIDE				{"vpath.front.control.widemode"};
static const std::string VHAL_PROP_VPATH_FRONT_STS_WIDE				{"vpath.front.status.widemode"};
static const std::string VHAL_PROP_VPATH_REAR_CTL_WIDE				{"vpath.rear.control.widemode"};
static const std::string VHAL_PROP_VPATH_REAR_STS_WIDE				{"vpath.rear.status.widemode"};
static const std::string VHAL_PROP_VPATH_FRONT_CTL_VISIBLE			{"vpath.front.control.visible"};
static const std::string VHAL_PROP_VPATH_FRONT_STS_VISIBLE			{"vpath.front.status.visible"};
static const std::string VHAL_PROP_VPATH_REAR_CTL_VISIBLE			{"vpath.rear.control.visible"};
static const std::string VHAL_PROP_VPATH_REAR_STS_VISIBLE			{"vpath.rear.status.visible"};
static const std::string VHAL_PROP_VPATH_ICLUSTER_CTL_VISIBLE		{"vpath.instrumentcluster.control.visible"};
static const std::string VHAL_PROP_VPATH_ICLUSTER_STS_VISIBLE		{"vpath.instrumentcluster.status.visible"};
static const std::string VHAL_PROP_VPATH_FRONT_CTL_OPACITY			{"vpath.front.control.opacity"};
static const std::string VHAL_PROP_VPATH_FRONT_STS_OPACITY			{"vpath.front.status.opacity"};
static const std::string VHAL_PROP_VPATH_REAR_CTL_OPACITY			{"vpath.rear.control.opacity"};
static const std::string VHAL_PROP_VPATH_REAR_STS_OPACITY			{"vpath.rear.status.opacity"};
static const std::string VHAL_PROP_VPATH_ICLUSTER_CTL_OPACITY		{"vpath.instrumentcluster.control.opacity"};
static const std::string VHAL_PROP_VPATH_ICLUSTER_STS_OPACITY		{"vpath.instrumentcluster.status.opacity"};
static const std::string VHAL_PROP_VSRC_FRONT_CTL_ID				{"vsrc.front.control.id"};
static const std::string VHAL_PROP_VSRC_FRONT_STS_ID				{"vsrc.front.status.id"};
static const std::string VHAL_PROP_VSRC_FRONT_CTL_BRIGHT			{"vsrc.front.control.brightness.step"};
static const std::string VHAL_PROP_VSRC_FRONT_CTL_CONTRAST			{"vsrc.front.control.contrast.step"};
static const std::string VHAL_PROP_VSRC_FRONT_STS_BRIGHT			{"vsrc.front.status.brightness.step"};
static const std::string VHAL_PROP_VSRC_FRONT_STS_CONTRAST			{"vsrc.front.status.contrast.step"};
static const std::string VHAL_PROP_VSRC_FRONT_CTL_IMG_ADJ_UPD		{"vsrc.front.control.img_adj_upd.on"};
static const std::string VHAL_PROP_VSRC_FRONT_CTL_FORCE_MULTISENSORY_IMG_ADJ	{"vsrc.front.control.forced_multisensory_img_adj"};
static const std::string VHAL_PROP_VSRC_FRONT_STS_FORCE_MULTISENSORY_IMG_ADJ	{"vsrc.front.status.forced_multisensory_img_adj"};
static const std::string VHAL_PROP_VSRC_FRONT_CTL_FORCE_HMI_IMG_ADJ	{"vsrc.front.control.forced_hmi_img_adj"};
static const std::string VHAL_PROP_VSRC_FRONT_STS_FORCE_HMI_IMG_ADJ	{"vsrc.front.status.forced_hmi_img_adj"};
static const std::string VHAL_PROP_MUTE_FRONT_CTL_DISP				{"mute.front.control.display"};
static const std::string VHAL_PROP_MUTE_FRONT_CTL_VIDEO				{"mute.front.control.video"};
static const std::string VHAL_PROP_MUTE_FRONT_CTL_BACK				{"mute.front.control.backlight"};
static const std::string VHAL_PROP_MUTE_REAR_CTL_DISP				{"mute.rear.control.display"};
static const std::string VHAL_PROP_MUTE_REAR_CTL_VIDEO				{"mute.rear.control.video"};
static const std::string VHAL_PROP_MUTE_FRONT_STS_DISP				{"mute.front.status.display"};
static const std::string VHAL_PROP_MUTE_FRONT_STS_VIDEO				{"mute.front.status.video"};
static const std::string VHAL_PROP_MUTE_FRONT_STS_BACK				{"mute.front.status.backlight"};
static const std::string VHAL_PROP_MUTE_REAR_STS_DISP				{"mute.rear.status.display"};
static const std::string VHAL_PROP_MUTE_REAR_STS_VIDEO				{"mute.rear.status.video"};
static const std::string VHAL_PROP_FAUTH_CTL_RSE					{"hdcp.first_auth.control.rse"};
static const std::string VHAL_PROP_FAUTH_CTL_SPRT					{"hdcp.first_auth.control.separate_disp"};
static const std::string VHAL_PROP_FAUTH_CTL_ICLUSTER				{"hdcp.first_auth.control.instrumentcluster"};
static const std::string VHAL_PROP_FAUTH_STS_RSE_RSLT				{"hdcp.first_auth.status.rse.result"};
static const std::string VHAL_PROP_FAUTH_STS_RSE_DEV_CNT			{"hdcp.first_auth.status.rse.deviceCount"};
static const std::string VHAL_PROP_FAUTH_STS_RSE_RCV_ID0			{"hdcp.first_auth.status.rse.receiverID0"};
static const std::string VHAL_PROP_FAUTH_STS_RSE_RCV_ID1			{"hdcp.first_auth.status.rse.receiverID1"};
static const std::string VHAL_PROP_FAUTH_STS_RSE_RCV_ID2			{"hdcp.first_auth.status.rse.receiverID2"};
static const std::string VHAL_PROP_FAUTH_STS_SPRT_RSLT				{"hdcp.first_auth.status.separate_disp.result"};
static const std::string VHAL_PROP_FAUTH_STS_SPRT_DEV_CNT			{"hdcp.first_auth.status.separate_disp.deviceCount"};
static const std::string VHAL_PROP_FAUTH_STS_SPRT_RCV_ID0			{"hdcp.first_auth.status.separate_disp.receiverID0"};
static const std::string VHAL_PROP_FAUTH_STS_ICLUSTER_RSLT			{"hdcp.first_auth.status.instrumentcluster.result"};
static const std::string VHAL_PROP_FAUTH_STS_ICLUSTER_DEV_CNT		{"hdcp.first_auth.status.instrumentcluster.deviceCount"};
static const std::string VHAL_PROP_FAUTH_STS_ICLUSTER_RCV_ID0		{"hdcp.first_auth.status.instrumentcluster.receiverID0"};
static const std::string VHAL_PROP_IVI_LAYOUT_CTL_CONF_FILE			{"ivi.layout.control.configuration.file"};
static const std::string VHAL_PROP_IVI_LAYOUT_STS_CONF_FILE			{"ivi.layout.status.configuration.file"};
static const std::string VHAL_PROP_IVI_SCRN_STS_PROPS				{"ivi.screen.status.properties"};
static const std::string VHAL_PROP_IVI_LAYER_STS_PROPS				{"ivi.layer.status.properties"};
static const std::string VHAL_PROP_IVI_SURF_STS_PROPS				{"ivi.surface.status.properties"};
static const std::string VHAL_PROP_IVI_LAYER_CTL_ORDER				{"ivi.layer.control.order"};
static const std::string VHAL_PROP_IVI_LAYER_STS_ORDER				{"ivi.layer.status.order"};
static const std::string VHAL_PROP_CAP_SCRN_CTL_BITMAP				{"capture.screen.control.bitmap"};
static const std::string VHAL_PROP_CAP_SCRN_STS_BITMAP				{"capture.screen.status.bitmap"};
static const std::string VHAL_PROP_CAP_SURF_CTL_BITMAP				{"capture.surface.control.bitmap"};
static const std::string VHAL_PROP_CAP_SURF_STS_BITMAP				{"capture.surface.status.bitmap"};
static const std::string VHAL_PROP_MOVIE_FRONT_CTL_PREPARE			{"movie.front.control.prepare"};
static const std::string VHAL_PROP_MOVIE_FRONT_CTL_FILE_PATH		{"movie.front.control.file.path"};
static const std::string VHAL_PROP_MOVIE_FRONT_CTL_OUTPUT_X			{"movie.front.control.output.x"};
static const std::string VHAL_PROP_MOVIE_FRONT_CTL_OUTPUT_Y			{"movie.front.control.output.y"};
static const std::string VHAL_PROP_MOVIE_FRONT_CTL_OUTPUT_W			{"movie.front.control.output.width"};
static const std::string VHAL_PROP_MOVIE_FRONT_CTL_OUTPUT_H			{"movie.front.control.output.height"};
static const std::string VHAL_PROP_MOVIE_FRONT_CTL_REPEAT			{"movie.front.control.repeat"};
static const std::string VHAL_PROP_MOVIE_FRONT_CTL_CLEAR			{"movie.front.control.clear"};
static const std::string VHAL_PROP_MOVIE_FRONT_CTL_START			{"movie.front.control.start"};
static const std::string VHAL_PROP_MOVIE_FRONT_CTL_CANCEL			{"movie.front.control.cancel"};
static const std::string VHAL_PROP_MOVIE_FRONT_STS_RESULT			{"movie.front.status.result"};
static const std::string VHAL_PROP_SETTING_CTL_DISP					{"vhal.setting.control.display_type"};
static const std::string VHAL_PROP_SETTING_STS_DISP					{"vhal.setting.status.display_type"};
static const std::string VHAL_PROP_SETTING_CTL_CNCT_CAMERA			{"vhal.setting.control.connected.camera"};
static const std::string VHAL_PROP_SETTING_STS_CNCT_CAMERA			{"vhal.setting.status.connected.camera"};
static const std::string VHAL_PROP_SETTING_CTL_CNCT_RSE				{"vhal.setting.control.connected.rse"};
static const std::string VHAL_PROP_SETTING_STS_CNCT_RSE				{"vhal.setting.status.connected.rse"};
static const std::string VHAL_PROP_SETTING_CTL_CNCT_SPRT			{"vhal.setting.control.connected.separate_disp"};
static const std::string VHAL_PROP_SETTING_STS_CNCT_SPRT			{"vhal.setting.status.connected.separate_disp"};
static const std::string VHAL_PROP_SETTING_CTL_DAY_NIGHT			{"vhal.setting.control.day_night"};
static const std::string VHAL_PROP_SETTING_STS_DAY_NIGHT			{"vhal.setting.status.day_night"};
static const std::string VHAL_PROP_SETTING_CTL_THEME_COLOR			{"vhal.setting.control.theme_color"};
static const std::string VHAL_PROP_SETTING_STS_THEME_COLOR			{"vhal.setting.status.theme_color"};
static const std::string VHAL_PROP_SETTING_STS_CNCT_HDMI			{"vhal.setting.status.connected.hdmi"};
static const std::string VHAL_PROP_SETTING_STS_DISP_FRONT			{"vhal.setting.status.display.front"};
static const std::string VHAL_PROP_SETTING_STS_DISP_REAR			{"vhal.setting.status.display.rear"};
static const std::string VHAL_PROP_SETTING_STS_DISP_ICLUSTER		{"vhal.setting.status.display.instrumentcluster"};
static const std::string VHAL_PROP_SETTING_STS_DISP_FRONT_WIDTH		{"vhal.setting.status.display.front.width"};
static const std::string VHAL_PROP_SETTING_STS_DISP_FRONT_HEIGHT	{"vhal.setting.status.display.front.height"};
static const std::string VHAL_PROP_SETTING_STS_DISP_REAR_WIDTH		{"vhal.setting.status.display.rear.width"};
static const std::string VHAL_PROP_SETTING_STS_DISP_REAR_HEIGHT		{"vhal.setting.status.display.rear.height"};
static const std::string VHAL_PROP_SETTING_STS_DISP_ICLUSTER_WIDTH	{"vhal.setting.status.display.instrumentcluster.width"};
static const std::string VHAL_PROP_SETTING_STS_DISP_ICLUSTER_HEIGHT	{"vhal.setting.status.display.instrumentcluster.height"};
static const std::string VHAL_PROP_HEACON_FRONT_CTL_OUTPUT_ON		{"heacon.front.control.output.on"};
static const std::string VHAL_PROP_HEACON_FRONT_CTL_OUTPUT_X		{"heacon.front.control.output.x"};
static const std::string VHAL_PROP_HEACON_FRONT_CTL_OUTPUT_Y		{"heacon.front.control.output.y"};
static const std::string VHAL_PROP_HEACON_FRONT_CTL_OUTPUT_W		{"heacon.front.control.output.width"};
static const std::string VHAL_PROP_HEACON_FRONT_CTL_OUTPUT_H		{"heacon.front.control.output.height"};
static const std::string VHAL_PROP_HEACON_FRONT_STS_OUTPUT_X		{"heacon.front.status.output.x"};
static const std::string VHAL_PROP_HEACON_FRONT_STS_OUTPUT_Y		{"heacon.front.status.output.y"};
static const std::string VHAL_PROP_HEACON_FRONT_STS_OUTPUT_W		{"heacon.front.status.output.width"};
static const std::string VHAL_PROP_HEACON_FRONT_STS_OUTPUT_H		{"heacon.front.status.output.height"};

/* API result code */
static constexpr int32_t VHAL_API_SUCCESS				{ 0};	/* success */
static constexpr int32_t VHAL_API_ERR					{-1};	/* general failure */
static constexpr int32_t VHAL_API_ERR_PARAM				{-2}; 	/* invalid parameter */
/* MISRA C++-2008 Rule 0-1-4 static constexpr int32_t VHAL_API_ERR_PROP_INIT			{-3}; */	/* Vhal client initialize error */
/* MISRA C++-2008 Rule 0-1-4 static constexpr int32_t VHAL_API_ERR_PROP_DEINIT		{-4}; */	/* Vhal client terminate error */
/* MISRA C++-2008 Rule 0-1-4 static constexpr int32_t VHAL_API_ERR_PROP_UPDATE		{-5}; */	/* Vhal property update error */
/* MISRA C++-2008 Rule 0-1-4 static constexpr int32_t VHAL_API_ERR_PROP_GET_VAL		{-6}; */	/* Vhal property get error */
/* MISRA C++-2008 Rule 0-1-4 static constexpr int32_t VHAL_API_ERR_PROP_SET_VAL		{-7}; */	/* Vhal property set error */
/* MISRA C++-2008 Rule 0-1-4 static constexpr int32_t VHAL_API_ERR_PROP_REG_CB		{-8}; */	/* Vhal property notification register error */
/* MISRA C++-2008 Rule 0-1-4 static constexpr int32_t VHAL_API_ERR_PROP_CLR_CB		{-9}; */	/* Vhal property notification clear error */

/* data type */
static constexpr uint32_t VHAL_DATA_TYPE_NUM			{1U};	/* number (int32_t, uint32_t, uint64_t) */
static constexpr uint32_t VHAL_DATA_TYPE_BOOL			{2U};	/* boolean */
static constexpr uint32_t VHAL_DATA_TYPE_STR			{3U};	/* string */


/* video source ID */
/* static constexpr uint32_t VHAL_VSRC_ID_BUILTIN_BACK_CAM			{0x42U}; */     /* (unused) */
static constexpr uint32_t VHAL_VSRC_ID_DTV							{0x49U};
/* static constexpr uint32_t VHAL_VSRC_ID_OTV						{0x4AU}; */     /* (unused) */
/* static constexpr uint32_t VHAL_VSRC_ID_BLURAY					{0x4BU}; */     /* (unused) */
static constexpr uint32_t VHAL_VSRC_ID_OTHER						{0x50U};	    /* hmi screen */
/* static constexpr uint32_t VHAL_VSRC_ID_EXTERNAL_BACK_CAM_GVIF	{0x5CU}; */     /* (unused) */
/* static constexpr uint32_t VHAL_VSRC_ID_EXTERNAL_BACK_CAM_NTSC	{0xB3U}; */     /* (unused) */
/* static constexpr uint32_t VHAL_VSRC_ID_CAM_DIAG_GVIF_BGM			{0xBAU}; */     /* (unused) */
/* static constexpr uint32_t VHAL_VSRC_ID_CAM_DIAG_GVIF_PVM			{0xBBU}; */     /* (unused) */
/* static constexpr uint32_t VHAL_VSRC_ID_CAM_DIAG_NTSC				{0xBEU}; */     /* (unused) */
static constexpr uint32_t VHAL_VSRC_ID_IMAGE_ADJUST_CAM				{0xBFU};
static constexpr uint32_t VHAL_VSRC_ID_HDMI							{0xD3U};
/* static constexpr uint32_t VHAL_VSRC_ID_DRC						{0xE1U}; */     /* (unused) */
static constexpr uint32_t VHAL_VSRC_ID_MULTISENSORY					{0xF1U};		/* 多感覚連携 */

/* wide mode value */
static constexpr uint32_t VHAL_WIDE_MODE_NORMAL			{0U};
static constexpr uint32_t VHAL_WIDE_MODE_STRETCHED		{1U};
static constexpr uint32_t VHAL_WIDE_MODE_ZOOMED			{2U};
static constexpr uint32_t VHAL_WIDE_MODE_INVALID		{3U};

/* HDCPfirst authentication result */
static constexpr int32_t VHAL_HDCP_FIRST_AUTH_STS_SUCCESS	{ 1};
static constexpr int32_t VHAL_HDCP_FIRST_AUTH_STS_FAILED	{ 0};
static constexpr int32_t VHAL_HDCP_FIRST_AUTH_STS_REAUTH	{ 2};
static constexpr int32_t VHAL_HDCP_FIRST_AUTH_STS_NONE		{-1};

/* vpath change result */
static constexpr uint32_t VHAL_VPATH_STS_SUCCESS		{1U};
static constexpr uint32_t VHAL_VPATH_STS_FAILED			{0U};

/* ivi layout result */
static constexpr uint32_t VHAL_IVI_LAYOUT_STS_SUCCESS	{1U};
// MISRA C++-2008 Rule 0-1-4  static constexpr uint32_t VHAL_IVI_LAYOUT_STS_FAILED	{0U};

/* screen capture result */
static constexpr int32_t VHAL_CAPTURE_STS_SUCCESS		{1};
static constexpr int32_t VHAL_CAPTURE_STS_FAILED		{0};

/* vehicle parameters display type (unused) */
/* static constexpr int32_t VHAL_VEHICLE_DISPLAY_INVALID					{-1}; */		/* property not set */
/* static constexpr int32_t VHAL_VEHICLE_DISPLAY_NONE						{ 0}; */		/* display not specified (unknown) */
/* static constexpr int32_t VHAL_VEHICLE_DISPLAY_BLTIN_STD_8IN_HD16_9		{ 1}; */		/* built-in, (standard, 8inch, HD, aspect ratio 16:9) */
/* static constexpr int32_t VHAL_VEHICLE_DISPLAY_BLTIN_HCP_9_8IN_QVGA4_3	{ 2}; */		/* built-in, (heater control panel, 9.8inch, QVGA, aspect ratio 4:3) */
/* static constexpr int32_t VHAL_VEHICLE_DISPLAY_BLTIN_STD_10_5IN_HD16_9	{ 3}; */		/* built-in, (standard, 10.5inch, HD, aspect ratio 16:9) */
/* static constexpr int32_t VHAL_VEHICLE_DISPLAY_BLTIN_HCP_14IN_FLHD16_9	{ 4}; */		/* built-in, (heater control panel, 14inch, FullHD, aspect ratio 16:9) */
/* static constexpr int32_t VHAL_VEHICLE_DISPLAY_BLTIN_STD_14IN_FLHD16_9	{ 5}; */		/* built-in, (standard, 14inch, FullHD, aspect ratio 16:9) */
/* static constexpr int32_t VHAL_VEHICLE_DISPLAY_SEPRT_STD_8IN_HD16_9		{ 6}; */		/* separate, (standard, 8inch, HD, aspect ratio 16:9) */
/* static constexpr int32_t VHAL_VEHICLE_DISPLAY_SEPRT_STD_10_5IN_HD16_9	{ 7}; */		/* separate, (standard, 10.5inch, HD, aspect ratio 16:9) */
/* static constexpr int32_t VHAL_VEHICLE_DISPLAY_SEPRT_WID_12_3IN_WHD24_9	{ 8}; */		/* separate, (wide, 12.3inch, WideHD, aspect ratio 24:9) */
/* static constexpr int32_t VHAL_VEHICLE_DISPLAY_BLTIN_WID_12_3IN_WHD24_9	{ 9}; */		/* built-in, (wide, 12.3inch, WideHD, aspect ratio 24:9) */

/* connected camera type */
static constexpr int32_t VHAL_CONNECTED_CAMERA_INVALID		{-1};		/* Property is not set */
static constexpr int32_t VHAL_CONNECTED_CAMERA_NONE			{ 0};		/* Camera not connected */
static constexpr int32_t VHAL_CONNECTED_CAMERA_ABGM			{ 1};		/* Analog back guide camera */
static constexpr int32_t VHAL_CONNECTED_CAMERA_DBGM			{ 3};		/* Digital back guide camera */
static constexpr int32_t VHAL_CONNECTED_CAMERA_DPVM_NORMAL	{ 4};		/* Digital panorama view camera (Normal) */
static constexpr int32_t VHAL_CONNECTED_CAMERA_DPVM_WIDE	{ 5};		/* Digital panorama view camera (WideHD) */
static constexpr int32_t VHAL_CONNECTED_CAMERA_DPVM_FULLHD	{ 6};		/* Digital panorama view camera (FullHD) */
static constexpr int32_t VHAL_CONNECTED_CAMERA_SBGM_CAA		{ 7};		/* Simple back guide camera (CAA) */
static constexpr int32_t VHAL_CONNECTED_CAMERA_BGM_ADU		{ 8};		/* Back guide camera (ADU) */
static constexpr int32_t VHAL_CONNECTED_CAMERA_SIM			{ 9};		/* Derivative of panorama view camera */
static constexpr int32_t VHAL_CONNECTED_CAMERA_PVM			{10};		/* Panorama view camera */
static constexpr int32_t VHAL_CONNECTED_CAMERA_MTM			{11};		/* Multi terrain camera */
static constexpr int32_t VHAL_CONNECTED_CAMERA_PVM_METER	{12};		/* Panorama view camera (Meter) */
static constexpr int32_t VHAL_CONNECTED_CAMERA_MTM_METER	{13};		/* Multi terrain camera (Meter) */

/* connected RSE type */
static constexpr int32_t VHAL_CONNECTED_RSE_INVALID			{-1};
// MISRA C++-2008 Rule 0-1-4 static constexpr int32_t VHAL_CONNECTED_RSE_NONE			{ 0};
static constexpr int32_t VHAL_CONNECTED_RSE_DOP				{ 1};
static constexpr int32_t VHAL_CONNECTED_RSE_FULL			{ 2};

/* separate display connection (unused) */
/* static constexpr int32_t VHAL_CONNECTED_SEPARATE_DISP_INVALID	{-1}; */
/* static constexpr int32_t VHAL_CONNECTED_SEPARATE_DISP_NONE		{ 0}; */
/* static constexpr int32_t VHAL_CONNECTED_SEPARATE_DISP_READY		{ 1}; */

/* day or night mode */
static constexpr uint32_t VHAL_SETTING_DAY					{0U};
static constexpr uint32_t VHAL_SETTING_NIGHT				{1U};
static constexpr uint32_t VHAL_SETTING_FORCED_DAY			{2U};

/* theme color type */
static constexpr int32_t VHAL_THEME_COLOR_AUTO_LIGHT		{0};
static constexpr int32_t VHAL_THEME_COLOR_AUTO_DARK			{1};
static constexpr int32_t VHAL_THEME_COLOR_SELECT_LIGHT		{2};
static constexpr int32_t VHAL_THEME_COLOR_SELECT_DARK		{3};
static constexpr int32_t VHAL_THEME_COLOR_FORCED_LIGHT		{4};

/* HDMI connection */
static constexpr int32_t VHAL_CONNECTED_HDMI_NONE			{0};
static constexpr int32_t VHAL_CONNECTED_HDMI_READY			{1};

/* Movie play result */
static constexpr int32_t VHAL_MOVIE_STS_NONE				{ 0};
static constexpr int32_t VHAL_MOVIE_STS_READY				{ 1};
static constexpr int32_t VHAL_MOVIE_STS_PLAYING				{ 2};
static constexpr int32_t VHAL_MOVIE_STS_FINISHED			{ 3};
static constexpr int32_t VHAL_MOVIE_STS_CANCELED			{ 4};
static constexpr int32_t VHAL_MOVIE_STS_FAILED				{-1};

/* opacity value */
static constexpr uint32_t VHAL_OPACITY_VALUE_MAX			{100U};

/* tier1 build target */
static constexpr int32_t VHAL_TIER1_TARGET_X86_64			{ 0};
static constexpr int32_t VHAL_TIER1_TARGET_ADP				{ 1};
static constexpr int32_t VHAL_TIER1_TARGET_1S				{ 2};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_CTL_DEFINE_PUBLIC_H */

