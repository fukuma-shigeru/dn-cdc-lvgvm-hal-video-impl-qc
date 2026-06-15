#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/eventfd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "com_stddef.h"
#include "xf86drm.h"
#include "xf86drmMode.h"

/*
===== ファイル構成
パス：/run/arene/vehicle_fs/var/bev3/stub/drm
├ func_ret    関数戻り値設定フォルダ
│　└ drmOpen                      関数の戻り値を設定する
├ drmModeObjectProperties1_status  "HDCP Auth Status"の結果(default:0x04)
├ drmModeObjectProperties2_status  "HDCP Device Count"の結果(default:0x03)
├ drmModeObjectProperties3_status  "HDCP Repeater Cascade Depth"の結果(default:0x01)
├ drmModeObjectProperties4_status  "HDCP Receiver ID 0"の結果(default:0xaaaaaaaaaa)
├ drmModeObjectProperties5_status  "HDCP Receiver ID 1"の結果(default:0xbbbbbbbbbb)
├ drmModeObjectProperties6_status  "HDCP Receiver ID 2"の結果(default:0xcccccccccc)
├ drmModeObjectProperties_control  HDCP認証開始トリガ
└ drmModeReset_control             内部プロパティ値を全て0にするトリガ(デバッグ用)

パス：/tmp/sys/devices/platform/soc/c88000.i2c/i2c-2/
├ 2-0020/meter_connection         メータGVIFリンク
└ 2-0021/auth_status              HDCP認証継続トリガ
*/


#define STUB_PATH				"/run/arene/vehicle_fs/var/bev3/stub/drm/"
#define STUB_PATH_FUNC			STUB_PATH "func_ret/"
#define STUB_FUNC(_fnc)			STUB_PATH_FUNC #_fnc
#define STUB_PATH_PROP			STUB_PATH "drmModeObjectProperties"
#define STUB_PROP_STS(_no)		STUB_PATH_PROP #_no "_status"
#define STUB_PROP_CTL			STUB_PATH "drmModeObjectProperties_control"
#define STUB_RESET_CTL			STUB_PATH "drmModeReset_control"

//#define STUB_SYSFS_CTL_PATH		"/sys/devices/platform/soc/c88000.i2c/i2c-2/2-0021/"
//#define STUB_GVIF_LINK_PATH		"/sys/devices/platform/soc/c88000.i2c/i2c-2/2-0020/"
// 本来は↑こちら↑を使うが、/sysが書込み不可のため現状はコメントにしておく

// ↓暫定対応
#define STUB_SYSFS_CTL_PATH		"/tmp/sys/devices/platform/soc/c88000.i2c/i2c-2/2-0021/"
#define STUB_GVIF_LINK_PATH		"/tmp/sys/devices/platform/soc/c88000.i2c/i2c-2/2-0020/"

#define STUB_SYSFS_CTL			STUB_SYSFS_CTL_PATH "auth_status"
#define STUB_GVIF_LINK			STUB_GVIF_LINK_PATH "meter_connection"

#define CONNECT_ID	(1)
#define TXSPEC_VAL	(0)

#define HDCP_WAIT_TIME	(200000)		/* HDCP自動遷移待ち時間(us) */

static int calledFirst_ = {FALSE};
static uint32_t connectorId_[1] = {CONNECT_ID};
static uint32_t propertyId_[1] = {0};		/* drmModePropertyRes_0_に紐づける */
static uint64_t tx_spec_[1] = {TXSPEC_VAL};

#define SetHdcpSts(_val) do {drmPropValues_[1] = (_val); drmHdcpSts_ = (_val);} while(FALSE)

#define DRM_COUNT_PROPS		(7)
static uint32_t drmProps_[DRM_COUNT_PROPS] = { 0, 1, 2, 3, 4, 5, 6 };
static uint64_t drmPropValues_[DRM_COUNT_PROPS] = { 0, 0, 0, 0, 0, 0, 0 };
static drmModeObjectProperties drmModeObjectProperties_ = {
	.count_props = DRM_COUNT_PROPS ,
	.props       = drmProps_ ,
	.prop_values = drmPropValues_ ,
};
static uint64_t drmDefValues_[DRM_COUNT_PROPS] = { 0, 0x04, 0x03, 0x01, 0xaaaaaaaaaa, 0xbbbbbbbbbb, 0xcccccccccc };
volatile uint64_t drmHdcpSts_ = 0x00;

static drmModeRes drmModeRes_ = {
	.count_connectors = 1 ,
	.connectors = connectorId_ ,	/* 1を格納 */
};

static drmModeConnector drmModeConnector_ = {
	.connector_type = DRM_MODE_CONNECTOR_DSI ,
	.count_props = 1 ,
	.props = propertyId_ ,			/* 0を格納 */
	.prop_values = tx_spec_ ,		/* 0を格納 */
	.connector_id = CONNECT_ID ,
};

static struct drm_mode_property_enum drm_mode_property_enum_ = {
	.value = TXSPEC_VAL ,
	.name = {"RSE"} ,
};

/* kDrmPropGvifTxSpec */
static drmModePropertyRes drmModePropertyRes_0_ = {
	.prop_id = 0 ,
	.name = {"gvif-tx-spec"} ,
	.flags = 0x0f ,
	.count_values = 1 ,
	.values = &drmPropValues_[0] ,
	.count_enums = 1 ,
	.enums = &drm_mode_property_enum_ ,
};

/* kDrmPropGvifTxStatus */
static drmModePropertyRes drmModePropertyRes_1_ = {
	.prop_id = 1 ,
	.name = {"HDCP Auth Status"} ,
	.count_values = 1 ,
	.values = &drmPropValues_[1] ,
};

/* HDCP Device Count */
static drmModePropertyRes drmModePropertyRes_2_ = {
	.prop_id = 2 ,
	.name = {"HDCP Device Count"} ,
	.count_values = 1 ,
	.values = &drmPropValues_[2] ,
};

/* HDCP Repeater Cascade Depth */
static drmModePropertyRes drmModePropertyRes_3_ = {
	.prop_id = 3 ,
	.name = {"HDCP Repeater Cascade Depth"} ,
	.count_values = 1 ,
	.values = &drmPropValues_[3] ,
};

/* HDCP Receiver ID 0 */
static drmModePropertyRes drmModePropertyRes_4_ = {
	.prop_id = 4 ,
	.name = {"HDCP Receiver ID 0"} ,
	.count_values = 1 ,
	.values = &drmPropValues_[4] ,
};

/* HDCP Receiver ID 1 */
static drmModePropertyRes drmModePropertyRes_5_ = {
	.prop_id = 5 ,
	.name = {"HDCP Receiver ID 1"} ,
	.count_values = 1 ,
	.values = &drmPropValues_[5] ,
};

/* HDCP Receiver ID 2 */
static drmModePropertyRes drmModePropertyRes_6_ = {
	.prop_id = 6 ,
	.name = {"HDCP Receiver ID 2"} ,
	.count_values = 1 ,
	.values = &drmPropValues_[6] ,
};

/*---------------------------------------------------- */
/* 関数の戻り値を設定する */
static int ReadFuncResult(const char* path, int defValue)
{
	int value = defValue;
	if (StubClientRead_bin(path, (int*)&value, sizeof(value)))
	{
		value = defValue;
	}
	return value;
}

/*---------------------------------------------------- */
/* パラメータ初期値設定 */
static void SetInitParam(const char* path, uint64_t value)
{
	/* ファイルがない場合は生成し、値を書き込む */
	/* 既にファイルが存在する場合はエラーになり無処理とする */
	int fd = open(path, O_CREAT|O_EXCL, 0666);
	if (fd >= 0)
	{
		close(fd);
		(void)StubClientWrite_bin(path, (void*)&value, sizeof(value));
	}
}


/*---------------------------------------------------- */
/* HDCP認証開始時の動作 */
static void drmModeObjectPropertiesListener(const char* p_path)
{
	(void*)p_path;
	volatile uint64_t	hdcpSts = drmHdcpSts_;

	/* ステータスが対象外の場合は無効 */
	if ((0x00 != hdcpSts) && (0x01 != hdcpSts) && (0x04 != hdcpSts))
	{
		return;
	}

	SetHdcpSts(0x00);					/* HDCP認証初期化状態 */
	usleep(HDCP_WAIT_TIME);
	SetHdcpSts(0x01);					/* HDCP認証開始待ち */
}

/*---------------------------------------------------- */
/* HDCP認証sysfsに書き込み発生時の動作 */
static void drmModeWrittenSysfsListener(const char* p_path)
{
	(void*)p_path;
	volatile uint64_t	hdcpSts = drmHdcpSts_;
	/* ステータスが対象外の場合は無効 */
	if (0x01 != hdcpSts)
	{
		return;
	}
	SetHdcpSts(0x02);				/* HDCP認証開始命令 */
	usleep(HDCP_WAIT_TIME);
	SetHdcpSts(0x03);				/* HDCP認証中 */
	uint64_t hdcp_result = 0x04;
	(void)StubClientRead_bin(STUB_PROP_STS(1), (int*)&hdcp_result, sizeof(hdcp_result));
	if (0x0f != hdcp_result)		/* 0x04か0x0fのみ受け付ける */
	{
		hdcp_result = drmDefValues_[1];	/* 読み込み失敗などの場合はデフォルト値 */
	}
	usleep(HDCP_WAIT_TIME);
	int		ret[7-2];
	uint64_t value[7-2] ={};
	ret[0] = StubClientRead_bin(STUB_PROP_STS(2), (int*)&value[0], sizeof(value[0]));
	ret[1] = StubClientRead_bin(STUB_PROP_STS(3), (int*)&value[1], sizeof(value[1]));
	ret[2] = StubClientRead_bin(STUB_PROP_STS(4), (int*)&value[2], sizeof(value[2]));
	ret[3] = StubClientRead_bin(STUB_PROP_STS(5), (int*)&value[3], sizeof(value[3]));
	ret[4] = StubClientRead_bin(STUB_PROP_STS(6), (int*)&value[4], sizeof(value[4]));

	for (int i=0; i<(7-2); i++)
	{
		if (0 != ret[i])
		{
			value[i] = drmDefValues_[i+2];		/* 読み込み失敗時はデフォルト値 */
		}
		drmPropValues_[i+2] = value[i];
	}
	SetHdcpSts(hdcp_result);			/* HDCP認証完了(0x04) または HDCP認証異常発生(0x0f) */
	if (0x0f == hdcp_result)
	{
		usleep(HDCP_WAIT_TIME);
		SetHdcpSts(0x01);				/* HDCP認証開始待ち */
	}
}

/*---------------------------------------------------- */
/* 値リセット時の動作 */
static void drmModeResetListener(const char* p_path)
{
	(void*)p_path;
	volatile uint64_t	hdcpSts = drmPropValues_[1];
	/* ステータスが対象外の場合は無効 */
	if ((0x00 != hdcpSts) && (0x01 != hdcpSts) && (0x04 != hdcpSts))
	{
		return;
	}
	drmPropValues_[6] = 0x00;
	drmPropValues_[5] = 0x00;
	drmPropValues_[4] = 0x00;
	drmPropValues_[3] = 0x00;
	drmPropValues_[2] = 0x00;
	drmPropValues_[1] = 0x00;
}

/*---------------------------------------------------- */
/* DRMのオープン */
int drmOpen(const char *name, const char *busid)
{
	/* 引数チェックは実施しない */
	(void)name;
	(void)busid;

	/* 初回のみ実施 */
	if (TRUE != calledFirst_)
	{
		calledFirst_ = TRUE;

		/* FileObserver用フォルダ作成 */
		system("mkdir -p " STUB_PATH);
		system("mkdir -p " STUB_SYSFS_CTL_PATH);
		system("mkdir -p " STUB_GVIF_LINK_PATH);

		/* controlファイル作成 */
		system("echo test > " STUB_PROP_CTL);
		system("echo test > " STUB_SYSFS_CTL);
		system("echo test > " STUB_RESET_CTL);
		system("echo test > " STUB_GVIF_LINK);
		StubClientRegisterListerner(STUB_PROP_CTL, drmModeObjectPropertiesListener);
		StubClientRegisterListerner(STUB_SYSFS_CTL, drmModeWrittenSysfsListener);
		StubClientRegisterListerner(STUB_RESET_CTL, drmModeResetListener);

		/* パラメータファイル作成(存在しない場合は初期値設定) */
		SetInitParam(STUB_PROP_STS(1), drmDefValues_[1]);
		SetInitParam(STUB_PROP_STS(2), drmDefValues_[2]);
		SetInitParam(STUB_PROP_STS(3), drmDefValues_[3]);
		SetInitParam(STUB_PROP_STS(4), drmDefValues_[4]);
		SetInitParam(STUB_PROP_STS(5), drmDefValues_[5]);
		SetInitParam(STUB_PROP_STS(6), drmDefValues_[6]);
	}

	/* VideoHALでのDRMのクローズ時では、drmClose()ではなくclose()を用いているため、 */
	/* 0固定ではなく、本物のファイルディスクリプタが必要。適当なディスクリプタを生成しておく。 */
	const int NG = -1;
	return (0 == ReadFuncResult(STUB_FUNC(drmOpen), 0) ? eventfd(0,0) : NG);
 }

/*---------------------------------------------------- */
/* DRMリソースアドレスの取得 */
drmModeResPtr drmModeGetResources(int fd)
{
	/* 引数チェックは実施しない */
	(void)fd;
	return &drmModeRes_;
}

/*---------------------------------------------------- */
/* DRMコネクタアドレスの取得 */
drmModeConnectorPtr drmModeGetConnector(int fd, uint32_t connectorId)
{
	/* 引数のチェックは行わない */
	(void)fd;
	(void)connectorId;
	return &drmModeConnector_;
}

/*---------------------------------------------------- */
/* DRMプロパティアドレスの取得 */
drmModePropertyPtr drmModeGetProperty(int fd, uint32_t propertyId)
{
	/* 引数チェックは行わない */
	(void)fd;

	drmModePropertyPtr	ret = NULL;
	switch (propertyId)
	{
	case 0 :
		ret = &drmModePropertyRes_0_;
		break;
	case 1 :
		ret = &drmModePropertyRes_1_;
		break;
	case 2 :
		ret = &drmModePropertyRes_2_;
		break;
	case 3 :
		ret = &drmModePropertyRes_3_;
		break;
	case 4 :
		ret = &drmModePropertyRes_4_;
		break;
	case 5 :
		ret = &drmModePropertyRes_5_;
		break;
	case 6 :
		ret = &drmModePropertyRes_6_;
		break;
	default :
		break;
	}
	return ret;
}

/*---------------------------------------------------- */
/* DRMプロパティアドレスの開放 */
void drmModeFreeProperty(drmModePropertyPtr ptr)
{
	/* 引数チェックは実施しない */
	(void)ptr;
	return;
}

/*---------------------------------------------------- */
/* DRMコネクタアドレスの開放 */
void drmModeFreeConnector(drmModeConnectorPtr ptr)
{
	/* 引数チェックは実施しない */
	(void)ptr;
	return;
}

/*---------------------------------------------------- */
/* DRMリソースアドレスの開放 */
void drmModeFreeResources(drmModeResPtr ptr)
{
	/* 引数チェックは実施しない */
	(void)ptr;
	return;
}

/*---------------------------------------------------- */
/* DRMプロパティ一覧アドレスの取得 */
drmModeObjectPropertiesPtr drmModeObjectGetProperties(int fd, uint32_t object_id, uint32_t object_type)
{
	/* 引数チェックは実施しない */
	(void)fd;
	(void)object_id;
	(void)object_type;
	return &drmModeObjectProperties_;
}

/*---------------------------------------------------- */
/* DRMプロパティ一覧アドレスの開放 */
void drmModeFreeObjectProperties(drmModeObjectPropertiesPtr ptr)
{
	/* 引数チェックは実施しない */
	(void)ptr;
	return;
}

/*---------------------------------------------------- */
