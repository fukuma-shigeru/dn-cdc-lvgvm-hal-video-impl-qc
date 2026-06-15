/*******************************************************************************
	機能名称	：	HAL GPIO公開ヘッダ
	ファイル名称：	hal_gpio_public.h
	作成者		：	AES) m.saito
	バージョン	：	2015年02月18日 版
*******************************************************************************/
/******************************************************************************
変更履歴 ：2017.10.13 20PFハード仕様対応
*******************************************************************************/
#ifndef _HAL_GPIO_PUBLIC_H_
#define _HAL_GPIO_PUBLIC_H_

/*******************************************************************************
	インクルード
*******************************************************************************/
#include "hal_public.h"

/*******************************************************************************
	define定義
*******************************************************************************/
/*	端子状態 -----------------------------------------------------------------*/
typedef enum _HalGpioLevel {
	HAL_GPIO_LOW,						/* LOWレベル */
	HAL_GPIO_HIGH,						/* HIGHレベル */
	HAL_GPIO_LEVEL_NUM
} HalGpioLevel;

/*	端子入出力方向 -----------------------------------------------------------*/
typedef enum _HalGpioDir {
	HAL_GPIO_IN,						/* 入力 */
	HAL_GPIO_OUT,						/* 出力 */
	HAL_GPIO_OUT_LOW,					/* 出力(LOW) */
	HAL_GPIO_OUT_HIGH,					/* 出力(HIGH) */
	HAL_GPIO_DIR_NUM
} HalGpioDir;

/*	端子ID -------------------------------------------------------------------*/
typedef enum _HalGpioId {
	HAL_GPIO_O_USB_HUB_RST,		/* /USB-HUB-RST */
	HAL_GPIO_I_WAKE_UP_STAT1,	/* WAKE-UP-STAT1 */
	HAL_GPIO_O_USB_USER_PWEN,	/* USB(USER)-PWEN */
	HAL_GPIO_O_SATURN_CS,		/* /Saturn-CS */
	HAL_GPIO_O_MM_STBY,		/* /MM-STBY */
	HAL_GPIO_O_CORE_WD,		/* CORE-WD */
	HAL_GPIO_O_BBI2S_SEL,		/* BBI2S-SEL */
	HAL_GPIO_I_USB_USER_OVC,		/* /USB(USER)-OVC */
	HAL_GPIO_O_WIFI_RST,		/* /WIFI-RST */
	HAL_GPIO_O_PCIE_RST,		/* /PCIE-RST */
	HAL_GPIO_O_M_DIAG_O,		/* M-DIAG-O */
	HAL_GPIO_I_M_DIAG_I,		/* M-DIAG-I */
	HAL_GPIO_O_BT_RST,		/* /BT-RST */
	HAL_GPIO_I_WAKE_UP_STAT2,	/* WAKE-UP-STAT2 */
	HAL_GPIO_O_INIC_RST,		/* /INIC-RST */
	HAL_GPIO_O_BBI2S_OE,		/* BBI2S-OE */
	HAL_GPIO_O_SATURN_RST,		/* /Saturn-RST */
	HAL_GPIO_O_MM_SYS_MUTE,		/* MM-SYS-MUTE */
	HAL_GPIO_O_XM_RST,		/* /XM-RST */
	HAL_GPIO_O_HDMI_RX_RST,		/* /HDMI-RX-RST */
	HAL_GPIO_O_MM_RSE_MUTE,		/* MM-RSE-MUTE */
	HAL_GPIO_O_GVIF_TX_RSE_RST,	/* /GVIF-TX(RSE)-RST */
	HAL_GPIO_I_WAKE_UP_STAT3,	/* WAKE-UP-STAT3 */
	HAL_GPIO_O_MM_WDTINH,		/* MM-WDTINH */
	HAL_GPIO_O_SEC_RST,		/* /SEC-RST */
	HAL_GPIO_I_TSNS_STATUS,		/* /TSNS-STATUS */
	HAL_GPIO_I_MODEL_JUDG1,		/* MODEL-JUDG1 */
	HAL_GPIO_I_MODEL_JUDG2,		/* MODEL-JUDG2 */
	HAL_GPIO_O_AUTO_USB_DET_REQ,	/* AUTO-USB-DET-REQ */
	HAL_GPIO_I_CAMERA_MODE2,		/* CAMERA-MODE2 */
	HAL_GPIO_O_CAMERA_CAP_STBY,	/* /CAMERA-CAP-STBY */
	HAL_GPIO_O_FLEX_CMD,		/* FLEX-CMD */
	HAL_GPIO_I_PCIE_CLK_REQ,		/* /PCIE-CLK-REQ */
	HAL_GPIO_I_PCIE_WAKE,		/* /PCIE-WAKE */
	HAL_GPIO_I_SOC_WAKE_UP,		/* /SOC-WAKE-UP */
	HAL_GPIO_I_HST_SW_CTRL,		/* HST-SW-CTRL */
	HAL_GPIO_I_L_MICON_INT,		/* /L-MICON-INT */
	HAL_GPIO_I_MM_STBY_REQ,		/* /MM-STBY-REQ */
	HAL_GPIO_I_BU_DET_H,		/* /BU-DET-H */
	HAL_GPIO_O_DTV_RST,		/* /DTV-RST */
	HAL_GPIO_O_ETHER_RST,		/* /Ether-RST */
	HAL_GPIO_I_CAMERA_MODE1,		/* CAMERA-MODE1 */
	HAL_GPIO_O_INIC_TDM_DIR,		/* INIC-TDM-DIR */
	HAL_GPIO_I_HDMI_RX_INT,		/* /HDMI-RX-INT */
	HAL_GPIO_I_SEC_INT,		/* /SEC-INT */
	HAL_GPIO_I_CAMERA_MODE3,		/* CAMERA-MODE3 */
	HAL_GPIO_O_LM_STBY_REQ,		/* /LM-STBY-REQ(Reserved) */
	HAL_GPIO_I_MOST_SPI_INT,		/* /MOST-SPI-INT */
	HAL_GPIO_I_MODEL_JUDG3,		/* MODEL-JUDG3 */
	HAL_GPIO_I_SATURN_INT,		/* /Saturn-INT */
	HAL_GPIO_O_DSRC_DET_REQ,		/* DSRC-DET-REQ */
	HAL_GPIO_I_INIC_INT,		/* /INIC-INT */
	HAL_GPIO_I_E_STBY_REQ,		/* E-STBY-REQ */
	HAL_GPIO_O_INIC_BOOT,		/* INIC-BOOT */
	HAL_GPIO_I_BU_DET,		/* /BU-DET */
	HAL_GPIO_I_GVIF_RX_EIM_GPIO0,	/* GVIF-RX(EIM)-GPIO0 */
	HAL_GPIO_I_USB_BOOT,		/* USB-Boot */
	HAL_GPIO_I_GVIF_RX_EIM_GPIO1,	/* GVIF-RX(EIM)-GPIO1 */
	HAL_GPIO_I_GVIF_RX_EIM_GPIO2,	/* GVIF-RX(EIM)-GPIO2 */
	HAL_GPIO_I_USB_REP_RST,		/* /USB-Rep-RST */
	HAL_GPIO_O_GPS_VUP,		/* GPS-VUP */
	HAL_GPIO_I_GVIF_RX_EIM_GPIO3,	/* GVIF-RX(EIM)-GPIO3 */
	HAL_GPIO_O_GVIF_RX_EIM_RST,	/* /GVIF-RX(EIM)-RST */
	HAL_GPIO_I_GVIF_RX_EIM_INT,	/* GVIF-RX(EIM)-INT */
	HAL_GPIO_O_GVIF_TX_METER_RST,	/* /GVIF-TX(Meter)-RST */
	HAL_GPIO_I_MODEL_JUDG4,		/* MODEL-JUDG4 */

	/* 21MM依存部分 */
	HAL_GPIO_O_DCM_DET,				/* DCM-DET */
	HAL_GPIO_O_DREC_DET,			/* 	DREC-DET	*/
	HAL_GPIO_O_DECK_DET,			/* 	DECK-DET	*/
	HAL_GPIO_O_AUTO_DET,			/*	AUTO-DET	*/
	HAL_GPIO_O_DSRC_DET,			/*	DSRC-DET	*/

	/* 2S追加端子 */
	HAL_GPIO_O_CAMERA_CAP_STBY2,	/* /CAMERA-CAP-STBY2 */
	HAL_GPIO_O_CAMERA_CAP_STBY3,	/* /CAMERA-CAP-STBY3 */

	HAL_GPIO_ID_NUM
} HalGpioId;

/*******************************************************************************
	型宣言
*******************************************************************************/
/*	HAL GPIO API -------------------------------------------------------------*/
typedef struct _HalGpioDeviceOps {
	INT32	(*getValue)		(void* hmi, HalGpioId id);
	INT32	(*setValue)		(void* hmi, HalGpioId id, HalGpioLevel val);
	INT32	(*setDirection)	(void* hmi, HalGpioId id, HalGpioDir dir);
} HalGpioDeviceOps;

/*	HAL GPIO API一覧取得マクロ -----------------------------------------------*/
#define HAL_GET_GPIO_METHODS(h)		HalGpioGetMethods(h)

/*******************************************************************************
	グローバル変数宣言
*******************************************************************************/

/*******************************************************************************
	プロトタイプ宣言
*******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

HalGpioDeviceOps* HalGpioGetMethods(void* hmi);

#if defined(__cplusplus)
}
#endif

#endif /* _HAL_GPIO_PUBLIC_H_ */
