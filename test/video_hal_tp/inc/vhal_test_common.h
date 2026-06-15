#ifndef	VHAL_TEST_COMMON_H
#define	VHAL_TEST_COMMON_H

#include <condition_variable>
#include <chrono>
#include <mutex>
#include <experimental/source_location>
#include <thread>
#include <unordered_map>

#include "ilm/ilm_common.h"
#include "ilm/ilm_control.h"

#include "sys_db_cnv_value_public.h"

#include "video_hal.h"
#include "gtest/gtest.h"

void Setup_vhal(void);
void Shutdown_vhal(void);

constexpr int32_t	MKIND_NONE{-1};	/* 未決定 */
constexpr int32_t	MKIND_QCOM{0};	/* 実機 */
constexpr int32_t	MKIND_X86{1};	/* x86_64 */

/* 環境設定 */
class VhalApiTestEnvironment : public ::testing::Environment
{
public:
	virtual void SetUp();
	virtual void TearDown();
};

/* API評価 */
class VhalApiTest:public::testing::Test
{
protected:
    void SetUp()
    {
    }

    void TearDown()
    {
    }
};

/* シーケンス評価 */
class VhalSeqTest:public::testing::Test
{
protected:
    void SetUp()
    {
    }

    void TearDown()
    {
    }
};

/* 環境異常評価 */
class VhalEnvTest:public::testing::Test
{
protected:
    void SetUp()
    {
    }

    void TearDown()
    {
    }
};

/* プロセス名 */
#define PNAME_VIDEO_HAL_SERVER      "video_hal_svc" /* VideoHAL Server */

/* プロパティ名 */
#define INVALID_PROPERTY            "invalid.property"  /* 不正なプロパティ名 */

/* ヘルパ関数戻り値 */
#define VHAL_TEST_SUCCESS (0)    /* 正常終了 */
#define VHAL_TEST_ERROR   (1)    /* 異常終了 */

#define CB_TIMEOUT_SECOUNDS                 (2)		/* コールバック試験時のタイムアウト時間 */
#define CB_SCREEN_CAPTURE_TIMEOUT_SECOUNDS (21)		/* コールバック試験時のタイムアウト時間(画面キャプチャ) */
#define CB_HDCP_TIMEOUT_SECOUNDS            (8)		/* コールバック試験時のタイムアウト時間 */

#define GETVALSIZE_MAX    (512)     /* GetValueに文字列指定時のvalsize */

/* 画像フォーマット種別 */
#define PNG_FORMAT        (1)    /* png */
#define BMP_FORMAT        (2)    /* bmp */

#define SYSDB_PATH_POWER_STATUS  "/db/sys_info/power/power_status.dbf" /* シーン別電源 */
#define SYSDB_FUNCID_VIDEO_HAL_TP (999)  /* 要求元機能ID（通信端点） */
#define VHAL_STAT_EPNAME			"video_hal_svc.sysdb"   /* video_hal_svc通信拠点名 */

/* 映像パス名 */
#define VPATH_NAME_CLEAR         ""
#define VPATH_NAME_DTV           "DTV"
#define VPATH_NAME_HDMI          "HDMI"
#define VPATH_NAME_MEDIAPLAYER   "MEDIAPLAYER"
#define VPATH_NAME_DRC           "DRC"
#define VPATH_NAME_CARPLAY       "CARPLAY"
#define VPATH_NAME_ANDROIDAUTO   "ANDROIDAUTO"
#define VPATH_NAME_MAP           "MAP"
#define VPATH_NAME_CAMERA        "CAMERA"
#define VPATH_NAME_MULTISENSORY  "MULTISENSORY"
#define VPATH_NAME_FRAGRANCE     "FRAGRANCE"
#define VPATH_NAME_REAR_HDMI     "REAR-HDMI"	
#define VPATH_NAME_REAR_HDMI2    "REAR-HDMI2"	
#define VPATH_NAME_REAR_MIRACAST "REAR-MIRACAST"	
#define VPATH_NAME_REAR_BROWSER  "REAR-BROWSER"
#define VPATH_NAME_REAR_VDSP     "REAR-VDSP"
#define VPATH_NAME_REAR_AUDIOSEL "REAR-AUDIOSEL"
#define VPATH_NAME_UNKNOWN       "UNKNOWN"

/* スクリーン名 */
#define SCREEN_NAME_FRONT        "front"
#define SCREEN_NAME_REAR         "rear"
#define SCREEN_NAME_IC           "instrumentcluster"
#define SCREEN_NAME_HUD          "hud"

/* スクリーンID */
#define SCREEN_ID_FRONT        (0)
#define SCREEN_ID_REAR         (1)
#define SCREEN_ID_IC           (2)
#define SCREEN_ID_HUD          (3)

/* レイヤ名 */
#define LAYER_NAME_MUTE_ON_CAMERA  "layer_mute_on_camera"
#define LAYER_NAME_MOVIE           "layer_movie"
#define LAYER_NAME_CAMERA          "layer_camera"
#define LAYER_NAME_MUTE_CAMERA     "layer_mute_camera"
#define LAYER_NAME_MUTE_FD         "layer_mute_fd"
#define LAYER_NAME_DEBUG           "layer_debug"
#define LAYER_NAME_FLUTTER         "layer_flutter"
#define LAYER_NAME_MUTE_TV         "layer_mute_fv"
#define LAYER_NAME_VIDEO           "front_video"
#define LAYER_NAME_MUTE_RD         "layer_mute_rd"
#define LAYER_NAME_MUTE_RV         "layer_mute_rv"
#define LAYER_NAME_REAR_VIDEO      "rear_video"
#define LAYER_NAME_IC_PROJECTION   "ic_projection"
#define LAYER_NAME_IC_MAP          "ic_map"

/* レイヤID */
#define LAYER_ID_MUTE_ON_CAMERA   (20)
#define LAYER_ID_MOVIE            (8)
#define LAYER_ID_CAMERA           (1)
#define LAYER_ID_MUTE_CAMERA      (2) 
#define LAYER_ID_MUTE_FD          (3)
#define LAYER_ID_DEBUG            (7)
#define LAYER_ID_FLUTTER          (4)
#define LAYER_ID_MUTE_TV          (5)
#define LAYER_ID_VIDEO            (6)
#define LAYER_ID_MUTE_RD          (101)
#define LAYER_ID_MUTE_RV          (102)
#define LAYER_ID_REAR_VIDEO       (103)
#define LAYER_ID_IC_PROJECTION    (203)
#define LAYER_ID_IC_MAP           (202)

/* サーフェス名 */
/* 前席 */
#define SURFACE_NAME_MUTE_ON_CAMERA          "mute_on_camera"
#define SURFACE_NAME_CAMERA                  "camera"
#define SURFACE_NAME_MUTE_FRONT_CAMERA       "mute_front_camera"
#define SURFACE_NAME_MUTE_FRONT_DISPLAY      "mute_front_display"
#define SURFACE_NAME_DEBUG_OVERLAY           "debug_overlay"
#define SURFACE_NAME_FLUTTER_BASIC           "flutter_basic"
#define SURFACE_NAME_FLUTTER_BASIC2          "flutter_basic2"
#define SURFACE_NAME_FLUTTER_BASIC3          "flutter_basic3"
#define SURFACE_NAME_FLUTTER_BASIC4          "flutter_basic4"
#define SURFACE_NAME_FLUTTER_BASIC5          "flutter_basic5"
#define SURFACE_NAME_MUTE_FRONT_VIDEO        "mute_front_video"
#define SURFACE_NAME_FRONT_DTV               "front_dtv"
#define SURFACE_NAME_FRONT_HDMI              "front_hdmi"
#define SURFACE_NAME_MEDIA_PLAYER            "media_player"
#define SURFACE_NAME_DRIVE_RECODER           "drive_recorder"
#define SURFACE_NAME_CARPLAY                 "carplay"
#define SURFACE_NAME_ANDROIDAUTO             "android_auto"
#define SURFACE_NAME_MULTISENSORY            "front_multisensory"
#define SURFACE_NAME_FRAGRANCE               "fragrance"
#define SURFACE_NAME_BACKGROUND_FRONT_VIDEO  "background_front_video"
/* 後席 */
#define SURFACE_NAME_MUTE_REAR_DISPLAY       "mute_rear_display"
#define SURFACE_NAME_MUTE_REAR_VIDEO         "mute_rear_video"
#define SURFACE_NAME_REAR_DTV                "rear_dtv"
#define SURFACE_NAME_REAR_HDMI               "rear_hdmi"
#define SURFACE_NAME_REAR_MULTISENSORY       "rear_multisensory"
#define SURFACE_NAME_BACKGROUND_REAR_VIDEO   "background_rear_video"
/* メータ */
#define SURFACE_NAME_IC_CARPLAY              "ic_carplay"
#define SURFACE_NAME_IC_ANDROIDAUTO          "ic_android_auto"
#define SURFACE_NAME_IC_MAP                  "ic_map"

/* サーフェスID */
/* 前席 */
#define SURFACE_ID_MUTE_ON_CAMERA              (11)         /* カメラ映像MUTE */
#define SURFACE_ID_CAMERA                      (10)         /* カメラ映像 */
#define SURFACE_ID_MUTE_FRONT_CAMERA           (19)         /* カメラアプリによる背面MUTE */
#define SURFACE_ID_MUTE_FRONT_DISPLAY          (71)         /* 前席ディスプレイMUTE
#define SURFACE_ID_DEBUG_OVERLAY               (99999)      /* デバッグ用 */
#define SURFACE_ID_FLUTTER_BASIC               (1)          /* flutter */
#define SURFACE_ID_FLUTTER_BASIC2              (2000000)    /* flutter */
#define SURFACE_ID_FLUTTER_BASIC3              (2000001)    /* flutter */
#define SURFACE_ID_FLUTTER_BASIC4              (20000000)   /* flutter */
#define SURFACE_ID_FLUTTER_BASIC5              (20000001)   /* flutter */
#define SURFACE_ID_MUTE_FRONT_VIDEO            (81)         /* 前席映像MUTE */
#define SURFACE_ID_FRONT_DTV                   (20)         /* DTV映像 */
#define SURFACE_ID_FRONT_HDMI                  (21)         /* HDMI映像 */
#define SURFACE_ID_MEDIA_PLAYER                (22)         /* メディアプレイヤー映像 */
#define SURFACE_ID_DRIVE_RECODER               (23)         /* ドラレコ映像 */
#define SURFACE_ID_CARPLAY                     (31)         /* CarPlayプロジェクション映像 */
#define SURFACE_ID_ANDROIDAUTO                 (32)         /* AndroidAutoプロジェクション映像 */
#define SURFACE_ID_MULTISENSORY                (34)         /* 多感覚連携映像 */
#define SURFACE_ID_FRAGRANCE                   (35)         /* フレグランス映像 */
#define SURFACE_ID_BACKGROUND_FRONT_VIDEO      (39)         /* 前席映像背景黒画 */
#define SURFACE_ID_MOVIE                       (51)         /* MOVIE映像 */
/* 後席 */
#define SURFACE_ID_MUTE_REAR_DISPLAY           (171)        /* 後席ディスプレイMUTE */
#define SURFACE_ID_MUTE_REAR_VIDEO             (181)        /* 後席映像MUTE */
#define SURFACE_ID_REAR_DTV                    (120)        /* DTV映像 */
#define SURFACE_ID_REAR_HDMI                   (121)        /* HDMI映像 */
#define SURFACE_ID_REAR_MULTISENSORY           (134)        /* 多感覚連携映像 */
#define SURFACE_ID_BACKGROUND_REAR_VIDEO       (129)        /* 後席映像背景黒画 */
/* メータ */
#define SURFACE_ID_IC_CARPLAY                  (231)        /* CarPlayプロジェクション映像 */
#define SURFACE_ID_IC_ANDROIDAUTO              (232)        /* AndroidAutoプロジェクション映像 */
#define SURFACE_ID_IC_MAP                      (210)        /* 地図 */
/* HUD */
#define SURFACE_ID_HUD_HUD                     (301)        /* HUD */


/* VideoHAL プロパティ初期値 */
#define DEFAULT_PROPERTY_OPACITY               (100)
#define DEFAULT_PROPERTY_WIDEMODE              (VIDEO_HAL_WIDE_MODE_NORMAL)
#define DEFAULT_PROPERTY_VISIBLE               (false)
#define DEFAULT_PROPERTY_OUTPUT_X              (0)
#define DEFAULT_PROPERTY_OUTPUT_Y              (0)
#define DEFAULT_PROPERTY_OUTPUT_WIDTH          (0)
#define DEFAULT_PROPERTY_OUTPUT_HEIGHT         (0)
#define DEFAULT_PROPERTY_CLIPPING_X            (0)
#define DEFAULT_PROPERTY_CLIPPING_Y            (0)
#define DEFAULT_PROPERTY_CLIPPING_WIDTH        (0)
#define DEFAULT_PROPERTY_CLIPPING_HEIGHT       (0)
#define DEFAULT_PROPERTY_CLIPPING_ENABLE       (false)
#define DEFAULT_PROPERTY_VSRC_ID               (VIDEO_HAL_VSRC_ID_OTHER)
#define DEFAULT_PROPERTY_BRIGHTNESS            (32)
#define DEFAULT_PROPERTY_CONTRAST              (32)
#define DEFAULT_PROPERTY_MUTE_DISPLAY          (true)
#define DEFAULT_PROPERTY_FORCED_HMI_IMG_ADJ    (false)
#define DEFAULT_PROPERTY_DAY_NIGHT             (VIDEO_HAL_SETTING_NIGHT)
#define DEFAULT_PROPERTY_THEME_COLOR           (VIDEO_HAL_THEME_COLOR_AUTO_DARK)

/* テストデータ設定マクロ */
#define OTHER_NUM_VALUE(num, upper_range)      ((num % upper_range) + 1)        /* 別値 */
#define OTHER_BOOL_VALUE(flg)                  (! flg)                          /* 反転 */

/* テスト繰り返し回数 */
#define TEST_REPEAT_COUNT                      (100)                    /* API単体評価(短時間の連続コール)繰り返し回数 */
#define TEST_SEQUENCE_REPEAT_COUNT             (10)                     /* シーケンス評価(連続動作)繰り返し回数 */

/* 汎用動画再生機能 */
#define TEST_MOVIE_FILE_NAME                   "/opt/dc-ivi-pf/tests/dn-cdc-lvgvm-hal-video-impl-qc/certification-program/Startup_Toyota_Global_1280x720.mp4"  /* 動画ファイル */
#define TEST_MOVIE_FILE_WIDTH                  (1280)
#define TEST_MOVIE_FILE_HEIGHT                 (720)
#define TEST_MOVIE_PLAYTIME_SECOND             (10)                  /* 動画ファイル再生時間*/
#define TEST_MOVIE_NONEXISTANT_FILE            "nonexistant"        /* 無効なファイル名 */

/* iviレイアウト構成設定機能 */
#define LAYER_CONFIG_JSON "/opt/dc-ivi-pf/share/layer_config.json"  /* レイヤ構成ファイル */
#define INVALID_LAYER_CONFIG_JSON "/tmp/invalid/layer_config.json"  /* レイヤ構成ファイル */

/* クリッピングサイズ設定機能 */
#define TEST_CLIPPING_IMAGE                   "Clipping_800x600.png"    /* クリッピングサイズ設定評価用画像 */

/* ワイド設定機能 */
#define TEST_WIDE_IMAGE                       "Wide_800x600.png"        /* ワイド設定機能評価用画像 */

/* 評価用画像 */
#define TEST_ANDROIDAUTO_IMAGE                "ANDROIDAUTO_1280x720.png"    /* ANDROIDAUTOサーフェス */
#define TEST_CARPLAY_IMAGE                    "CARPLAY_1280x720.png"        /* CARPLAYサーフェス */
#define TEST_DRC_IMAGE                        "DRC_1280x720.png"            /* DRCサーフェス */
#define TEST_DTV_IMAGE                        "DTV_1280x720.png"            /* DTVサーフェス */
#define TEST_MEDIAPLAYER_IMAGE                "MEDIAPLAYER_1280x720.png"    /* MEDIAPLAYERサーフェス */
#define TEST_MAP_IMAGE                        "MAP_1280x720.png"            /* MAPサーフェス */
#define TEST_CLIPPING_IMAGE                   "Clipping_800x600.png"        /* クリッピングサイズ設定評価 */
#define TEST_WIDE_IMAGE                       "Wide_800x600.png"            /* ワイド設定機能評価 */
#define TEST_MULTISENSORY_IMAGE               "MULTISENSORY_1280x720.png"   /* 多感覚連携サーフェス */
#define TEST_FRAGRANCE_IMAGE                  "FRAGRANCE_1280x720.png"      /* フレグランスサーフェス */
#define TEST_HUD_IMAGE                        "HUD_448x224.png"             /* HUDサーフェス */

#define TEST_RENDER_IMAGE_ST                  (2000)   /* render_image起動待ちスリープミリ秒 */

/* VideoHAL隠しプロパティパラメータ */
/* カメラ同期切替(vhal.tp.setting.control.sync.camera) */
#define CAM_SYNC_NO             (0x00)      /* 同期無し */
#define CAM_SYNC                (0x01)      /* 同期有り */
#define CAM_SYNC_UNKNOWN        (0xFF)      /* 未確定・無効 */

/* カメラ種別判別通知(vhal.tp.setting.control.size.camera, vhal.tp.setting.control.size.iccamera) */
#define CAM_SIZE_1920_1080      (0x01)      /*1920, 1080 */  
#define CAM_SIZE_1280_635       (0x07)      /*1280, 635	*/

/* カメラ接続有無 */
#define NOP_CONNECTED_CAMERA    (NOP_CONNECTED_CAMERA_VALUE)

enum InternalEventType {
  INTERNAL_EVENT_CREATE_SURFACE,
  INTERNAL_EVENT_DELETE_SURFACE,
  INTERNAL_EVENT_NOTIFY_SURFACE_SIZE,
  INTERNAL_EVENT_DETECT_CAMERA_SYNC,
  INTERNAL_EVENT_NOTIFY_CAMERA_TYPE,
  INTERNAL_EVENT_CHANGE_HDMI_VIDEO_FORMAT,
  INTERNAL_EVENT_DETECT_CONNECTED_HDMI
};

struct InternalEventParameter {
	cockpit::hal::video_hal::CtlObj obj;
    InternalEventType type;
    union {
        struct {
            uint32_t surface_id;
        } create_surface;
        struct {
            uint32_t surface_id;
        } delete_surface;
        struct {
            uint32_t surface_id;
        } notify_surface_size;        
        struct {
            uint32_t cam_sync;
        } detect_camera_sync;
        struct {
            uint32_t cam_size;
        } notify_camera_type;
        struct {
            uint32_t format;
        } change_hdmi_video_format;
        struct {
            uint32_t conn;
        } detect_connected_hdmi;
    } u;
};

/* ヘルパ関数 */
void PrintMsg(std::string msg, const std::experimental::source_location location = std::experimental::source_location::current());
void PrintMsgV(const std::experimental::source_location location, const char* fmt, ...);
int32_t GetMachine(void);
int32_t SigStop(const std::string& pname);
int32_t SigCont(const std::string& pname);
int32_t ScreenShot(std::string test_name, int file_number);
int32_t ScreenShot(void);
int32_t GetImageSize(uint32_t surface_id, uint32_t& width, uint32_t& height);
int32_t ExecuteRenderImage(uint32_t surface_id);
int32_t ExecuteRenderImage(uint32_t surface_id, std::string& filename);
int32_t ExecuteRenderImage(cockpit::hal::video_hal::CtlObj& obj, uint32_t surface_id);
int32_t ExecuteRenderImage(cockpit::hal::video_hal::CtlObj& obj, uint32_t surface_id, std::string& filename);
int32_t KillAllRenderImage(cockpit::hal::video_hal::CtlObj& obj);
int32_t KillRenderImage(int surface_id);
int32_t KillRenderImage(cockpit::hal::video_hal::CtlObj& obj, int surface_id);
void IlmInitilize(void);
void IlmFinalize(void);
void GetScreenProperties(t_ilm_uint screenId, struct ilmScreenProperties *screenProp);
const char * GetMovieFileName(void);
int32_t SwitchVpathFront(cockpit::hal::video_hal::CtlObj& obj, std::string vpath);
int32_t SwitchVpathRear(cockpit::hal::video_hal::CtlObj& obj, std::string vpath);
int32_t SwitchVpathCamera(cockpit::hal::video_hal::CtlObj& obj, std::string vpath);
int32_t SwitchVpathIc(cockpit::hal::video_hal::CtlObj& obj, std::string vpath);
int32_t SwitchVpathRse(cockpit::hal::video_hal::CtlObj& obj, std::string vpath);
int32_t SetOpacityFront(cockpit::hal::video_hal::CtlObj& obj, uint32_t opacity);
int32_t SetOpacityRear(cockpit::hal::video_hal::CtlObj& obj, uint32_t opacity);
int32_t SetOpacityIc(cockpit::hal::video_hal::CtlObj& obj, uint32_t opacity);
int32_t SetVisibleFront(cockpit::hal::video_hal::CtlObj& obj, bool visible);
int32_t SetVisibleRear(cockpit::hal::video_hal::CtlObj& obj, bool visible);
int32_t SetVisibleIc(cockpit::hal::video_hal::CtlObj& obj, bool visible);
int32_t SetFrontOutput(cockpit::hal::video_hal::CtlObj& obj, uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool on);
int32_t SetRearOutput(cockpit::hal::video_hal::CtlObj& obj, uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool on);
int32_t SetCameraOutput(cockpit::hal::video_hal::CtlObj& obj, uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool on);
int32_t SetIcOutput(cockpit::hal::video_hal::CtlObj& obj, uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool on);
int32_t SetFrontClipping(cockpit::hal::video_hal::CtlObj& obj, uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool enable, bool on);
int32_t TrueFalseSetFrontClipping(cockpit::hal::video_hal::CtlObj& obj, uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool enable, bool on);
int32_t SetIcClipping(cockpit::hal::video_hal::CtlObj& obj, uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool enable, bool on);
int32_t TrueFalseSetIcClipping(cockpit::hal::video_hal::CtlObj& obj, uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool enable, bool on);
int32_t SetWideModeFront(cockpit::hal::video_hal::CtlObj& obj, uint32_t widemode);
int32_t SetWideModeRear(cockpit::hal::video_hal::CtlObj& obj, uint32_t widemode);
int32_t SetBrightness(cockpit::hal::video_hal::CtlObj& obj, uint32_t brightness);
int32_t SetContrast(cockpit::hal::video_hal::CtlObj& obj, uint32_t contrast);
int32_t SetForcedHmiImgAdj(cockpit::hal::video_hal::CtlObj& obj, bool on);
int32_t SetForcedMultisensoryImgAdj(cockpit::hal::video_hal::CtlObj& obj, bool on);
int32_t SetVideoSourceId(cockpit::hal::video_hal::CtlObj& obj, uint32_t video_source_id);
int32_t MuteDisplayFront(cockpit::hal::video_hal::CtlObj& obj, bool mute);
int32_t MuteDisplayRear(cockpit::hal::video_hal::CtlObj& obj, bool mute);
int32_t MuteVideoFront(cockpit::hal::video_hal::CtlObj& obj, bool mute);
int32_t MuteVideoRear(cockpit::hal::video_hal::CtlObj& obj, bool mute);
int32_t MuteBacklight(cockpit::hal::video_hal::CtlObj& obj, bool on);
int32_t SetIviLayerOrder(cockpit::hal::video_hal::CtlObj& obj, int32_t ivi_id, int32_t layer_order);
int32_t SetIviLayerOrder(cockpit::hal::video_hal::CtlObj& obj, std::string ivi_name, int32_t layer_order);
int32_t SetConnectedCamera(cockpit::hal::video_hal::CtlObj& obj, int32_t connected_camera);
int32_t SetConnectedRse(cockpit::hal::video_hal::CtlObj& obj, int32_t connected_rse);
int32_t SetThemeColor(cockpit::hal::video_hal::CtlObj& obj, int32_t theme_color);
int32_t SetDayNight(cockpit::hal::video_hal::CtlObj& obj, int32_t day_night);
int32_t SetHeaconAreaSize(cockpit::hal::video_hal::CtlObj& obj, uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool on);
/* シーン別電源設定 */
typedef void (*SetPowerStatus)(void);
void SetPowerStatusNormal(void);
void SetPowerStatusPre(void);
void SetPowerStatusBackGround(void);

void DetectCameraSync(cockpit::hal::video_hal::CtlObj& obj, int32_t cam_sync);
void NotifyCameraType(cockpit::hal::video_hal::CtlObj& obj, int32_t cam_size);
void ChangeHdmiVideoFormat(int32_t format);
void ReadHdmiVideoFormat(int32_t& format);
void DetectConnectedHdmi(int32_t conn);

void OccurrInternalEvent(InternalEventParameter& param);

int32_t StartHomeScreen(void);
int32_t StopHomeScreen(void);

#define PrintDbg(...)	PrintMsgV(std::experimental::source_location::current(), __VA_ARGS__)

#endif /* VHAL_TEST_COMMON_H */
