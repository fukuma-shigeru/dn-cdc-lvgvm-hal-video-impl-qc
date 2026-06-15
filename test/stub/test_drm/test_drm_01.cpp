
#include <unistd.h>
#include "test_main.h"
#include "file_observer.h"
#include "com_stddef.h"

extern "C" {
#include "xf86drm.h"
#include "xf86drmMode.h"
}

#define STUB_PATH				"/run/arene/vehicle_fs/var/bev3/stub/drm/"
#define STUB_PATH_PROP			STUB_PATH "drmModeObjectProperties"
#define STUB_PROP_STS(_no)		STUB_PATH_PROP #_no "_status"
#define STUB_PROP_CTL			STUB_PATH "drmModeObjectProperties_control"
#define STUB_RESET_CTL			STUB_PATH "drmModeReset_control"

/*#define STUB_SYSFS_CTL_PATH		"/sys/devices/platform/soc/c88000.i2c/i2c-2/2-0021/" */
/*#define STUB_GVIF_LINK_PATH		"/sys/devices/platform/soc/c88000.i2c/i2c-2/2-0020/" */
/* 本来は↑こちら↑を使うが、/sysが書込み不可のため現状はコメントにしておく */

/* ↓暫定対応 */
#define STUB_SYSFS_CTL_PATH		"/tmp/sys/devices/platform/soc/c88000.i2c/i2c-2/2-0021/"
#define STUB_GVIF_LINK_PATH		"/tmp/sys/devices/platform/soc/c88000.i2c/i2c-2/2-0020/"

#define STUB_SYSFS_CTL			STUB_SYSFS_CTL_PATH "auth_status"
#define STUB_GVIF_LINK			STUB_GVIF_LINK_PATH "meter_connection"

#define CONNECT_ID	(1)
#define TXSPEC_VAL	(0)


static bool first_run = true;
extern volatile uint64_t drmHdcpSts_;

#if 0
namespace
{

void PrintMsg(std::string msg, const std::experimental::source_location location = std::experimental::source_location::current())
{
	std::string absolute_path = location.file_name();
	std::size_t dir_pos = absolute_path.find_last_of("/");
	std::string file_name = absolute_path.substr(dir_pos + 1);
	std::cout << "[DEBUG][" << file_name << ":" << location.line() << 
		"][" << location.function_name() << "] " << msg << std::endl;
}

}
#endif

class DrmTest_Api: public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();

};

void DrmTest_Api::SetUp()
{
	if (true == first_run)
	{
		first_run = false;
		system("rm -fr " STUB_PATH_PROP "*_status");
	}
}

void DrmTest_Api::TearDown()
{
}

static void StubWrite(const char* path, const INT32 val)
{
	cockpit::bs::CFileObserver::GetInstance()->Write(path, &val, sizeof(val));
}

static void StubWrite(const char* path, const void* ptr)
{
	cockpit::bs::CFileObserver::GetInstance()->Write(path, (void*)&ptr, sizeof(ptr));
}

static void StubWrite(const char* path, const void* ptr, size_t size)
{
	cockpit::bs::CFileObserver::GetInstance()->Write(path, ptr, size);
}

static void WaitStatus(void)
{
	drmModeObjectPropertiesPtr ptr = drmModeObjectGetProperties(0,0,0);
	volatile uint64_t*	p_hdcpSts = (volatile uint64_t*)&(ptr->prop_values[1]);

	for ( ;; )
	{
		/* 安定状態になるまで待つ */
		if((0x01 == *p_hdcpSts) || (0x04 == *p_hdcpSts))
		{
			break;
		}
		usleep(5000);
	}
}

//
// Googletest TestCase
//

/*---------------------------------------------------- */
/* drmOpen関数の調査 */
TEST_F(DrmTest_Api, drmOpen)
{
	int	ret;

	/* 正常系 */
	ret = drmOpen(nullptr, nullptr);
	EXPECT_LT(2, ret);	/* 標準エラー出力(=2)より大きいはず */
	close(ret);			/* クローズはdrmCloseではなくcloseを用いる */
}

/*---------------------------------------------------- */
/* drmModeGetResources関数の調査 */
TEST_F(DrmTest_Api, drmModeGetResources)
{
	int	fd = 3;
	drmModeResPtr	ret;
	drmModeRes		dat = {};

	/* 正常系 */
	ret = drmModeGetResources(fd);
	ASSERT_NE(nullptr, ret);
	EXPECT_EQ(1, ret->count_connectors);
	ASSERT_NE(nullptr, ret->connectors);
	EXPECT_EQ(1, ret->connectors[0]);
}

/*---------------------------------------------------- */
/* drmModeGetConnector関数の調査 */
TEST_F(DrmTest_Api, drmModeGetConnector)
{
	int	fd = 3;
	uint32_t connectorId = 1;
	drmModeConnectorPtr	ret;

	/* 正常系 */
	ret = drmModeGetConnector(fd, connectorId);
	ASSERT_NE(nullptr, ret);
	EXPECT_EQ(DRM_MODE_CONNECTOR_DSI, ret->connector_type);
	EXPECT_EQ(1, ret->count_props);
	ASSERT_NE(nullptr, ret->props);
	EXPECT_EQ(0, ret->props[0]);
	ASSERT_NE(nullptr, ret->prop_values);
	EXPECT_EQ(TXSPEC_VAL, ret->prop_values[0]);
	EXPECT_EQ(CONNECT_ID, ret->connector_id);
}

/*---------------------------------------------------- */
/* drmModeGetProperty関数の調査 */
TEST_F(DrmTest_Api, drmModeGetProperty)
{
	int	fd = 3;
	drmModePropertyPtr	ret;

	/* 正常系(propertyId=0) */
	ret = drmModeGetProperty(fd, 0);
	ASSERT_NE(nullptr, ret);
	EXPECT_EQ(0, ret->prop_id);
	EXPECT_STREQ("gvif-tx-spec", ret->name);
	EXPECT_EQ(1, ret->count_values);
	ASSERT_NE(nullptr, ret->values);
	EXPECT_EQ(0, ret->values[0]);
	EXPECT_EQ(1, ret->count_enums);
	ASSERT_NE(nullptr, ret->enums);
	EXPECT_EQ(TXSPEC_VAL, ret->enums[0].value);
	EXPECT_STREQ("RSE", ret->enums[0].name);

	/* 正常系(propertyId=1) */
	ret = drmModeGetProperty(fd, 1);
	ASSERT_NE(nullptr, ret);
	EXPECT_EQ(1, ret->prop_id);
	EXPECT_STREQ("HDCP Auth Status", ret->name);
	EXPECT_EQ(1, ret->count_values);
	ASSERT_NE(nullptr, ret->values);
	EXPECT_EQ(0, ret->values[0]);

	/* 正常系(propertyId=2) */
	ret = drmModeGetProperty(fd, 2);
	ASSERT_NE(nullptr, ret);
	EXPECT_EQ(2, ret->prop_id);
	EXPECT_STREQ("HDCP Device Count", ret->name);
	EXPECT_EQ(1, ret->count_values);
	ASSERT_NE(nullptr, ret->values);
	EXPECT_EQ(0, ret->values[0]);

	/* 正常系(propertyId=3) */
	ret = drmModeGetProperty(fd, 3);
	ASSERT_NE(nullptr, ret);
	EXPECT_EQ(3, ret->prop_id);
	EXPECT_STREQ("HDCP Repeater Cascade Depth", ret->name);
	EXPECT_EQ(1, ret->count_values);
	ASSERT_NE(nullptr, ret->values);
	EXPECT_EQ(0, ret->values[0]);

	/* 正常系(propertyId=4) */
	ret = drmModeGetProperty(fd, 4);
	ASSERT_NE(nullptr, ret);
	EXPECT_EQ(4, ret->prop_id);
	EXPECT_STREQ("HDCP Receiver ID 0", ret->name);
	EXPECT_EQ(1, ret->count_values);
	ASSERT_NE(nullptr, ret->values);
	EXPECT_EQ(0, ret->values[0]);

	/* 正常系(propertyId=5) */
	ret = drmModeGetProperty(fd, 5);
	ASSERT_NE(nullptr, ret);
	EXPECT_EQ(5, ret->prop_id);
	EXPECT_STREQ("HDCP Receiver ID 1", ret->name);
	EXPECT_EQ(1, ret->count_values);
	ASSERT_NE(nullptr, ret->values);
	EXPECT_EQ(0, ret->values[0]);

	/* 正常系(propertyId=6) */
	ret = drmModeGetProperty(fd, 6);
	ASSERT_NE(nullptr, ret);
	EXPECT_EQ(6, ret->prop_id);
	EXPECT_STREQ("HDCP Receiver ID 2", ret->name);
	EXPECT_EQ(1, ret->count_values);
	ASSERT_NE(nullptr, ret->values);
	EXPECT_EQ(0, ret->values[0]);

	/* 異常系実施(propertyId=7) */
	ret = drmModeGetProperty(fd, 7);
	EXPECT_EQ(nullptr, ret);

}

/*---------------------------------------------------- */
/* drmModeObjectGetProperties関数の調査 */
TEST_F(DrmTest_Api, drmModeObjectGetProperties)
{
	int	fd = 3;
	drmModeObjectPropertiesPtr	ret;

	/* 正常系 */
	ret = drmModeObjectGetProperties(fd, 0, 0);
	ASSERT_NE(nullptr, ret);
	EXPECT_EQ(7, ret->count_props);
	ASSERT_NE(nullptr, ret->props);
	EXPECT_EQ(0, ret->props[0]);
	EXPECT_EQ(1, ret->props[1]);
	EXPECT_EQ(2, ret->props[2]);
	EXPECT_EQ(3, ret->props[3]);
	EXPECT_EQ(4, ret->props[4]);
	EXPECT_EQ(5, ret->props[5]);
	EXPECT_EQ(6, ret->props[6]);
	ASSERT_NE(nullptr, ret->prop_values);
	EXPECT_EQ(0, ret->prop_values[0]);
	EXPECT_EQ(0, ret->prop_values[1]);
	EXPECT_EQ(0, ret->prop_values[2]);
	EXPECT_EQ(0, ret->prop_values[3]);
	EXPECT_EQ(0, ret->prop_values[4]);
	EXPECT_EQ(0, ret->prop_values[5]);
	EXPECT_EQ(0, ret->prop_values[6]);
}

/*---------------------------------------------------- */
/* Free系関数の調査 */
TEST_F(DrmTest_Api, drmModeFreeXXX)
{
	/* 各関数をコールし、ダウンしないことを確認 */
	drmModeFreeResources(nullptr);
	drmModeFreeConnector(nullptr);
	drmModeFreeProperty(nullptr);
	drmModeFreeObjectProperties(nullptr);
	drmModeFreeResources((drmModeResPtr)1000);
	drmModeFreeConnector((drmModeConnectorPtr)1001);
	drmModeFreeProperty((drmModePropertyPtr)1002);
	drmModeFreeObjectProperties((drmModeObjectPropertiesPtr)1003);
}

/*---------------------------------------------------- */

static constexpr int32_t VHAL_SUCCESS					{  0};	/* 正常終了 */
static constexpr int32_t VHAL_ERR_PARAM					{ -2};	/* パラメータ不正 */
static constexpr int32_t VHAL_ERR_DRM_API				{-23};	/* Drm APIエラー */

/*---------------------------------------------------- */
/* videoHALから流用(一部改造) */
static int32_t GetValueDrmProperty(const std::string &propStr, uint64_t &value)
{
	int drmfd_ = 3;
	int32_t cnctid_rse_ = CONNECT_ID;
	int32_t	result{VHAL_ERR_DRM_API};

	do
	{
		if (-1 == drmfd_ || 0U == cnctid_rse_)
		{
			printf("parameter error. drmfd=%d, connector_id=%d", drmfd_, cnctid_rse_);
			result = VHAL_ERR_PARAM;
			break;
		}

		const drmModeObjectPropertiesPtr	p_props{drmModeObjectGetProperties(drmfd_, cnctid_rse_, DRM_MODE_OBJECT_CONNECTOR)};
		if (nullptr == p_props)
		{
			printf("drmModeObjectGetProperties() error. drmfd=%d, connector_id=%d", drmfd_, cnctid_rse_);
			break;
		}

		/* 取得できたプロパティ分のループ */
		for (uint32_t propCnt{0U}; p_props->count_props > propCnt; ++propCnt)
		{
			/* 個々のプロパティ情報を取得する */
			const drmModePropertyPtr	p_prop{drmModeGetProperty(drmfd_, p_props->props[propCnt])};
			if (nullptr != p_prop)
			{
				if (0 == propStr.compare(p_prop->name))
				{
					value = p_props->prop_values[propCnt];
					drmModeFreeProperty(p_prop);
					result = VHAL_SUCCESS;
					break;
				}
				drmModeFreeProperty(p_prop);
			}
		}
		drmModeFreeObjectProperties(p_props);

		if (VHAL_SUCCESS != result)
		{
			printf("DrmProperty[%s] not found() error.", propStr.c_str());
		}
	} while (false);

	/*	VHAL_LOGV_OUT(); */
	return result;
}

/*---------------------------------------------------- */
/* プロパティの値変更と取得(HDCP正常：デフォルト操作) */
TEST_F(DrmTest_Api, drmModeObjectGetProperties_Change1)
{
	int	fd = 3;
	drmModeObjectPropertiesPtr	ret;
	const struct {
		std::string	str;
	} tbl[7] = {
		{ "gvif-tx-spec"				} ,
		{ "HDCP Auth Status"			} ,
		{ "HDCP Device Count"			} ,
		{ "HDCP Repeater Cascade Depth"	} ,
		{ "HDCP Receiver ID 0"			} ,
		{ "HDCP Receiver ID 1"			} ,
		{ "HDCP Receiver ID 2"			} ,
	};
	uint64_t	value;
	int32_t		result;

	system("touch " STUB_RESET_CTL);		/* 値のクリア */
	usleep(100000);		/* 一定時間待つ */

	/* value値がクリアしていること */
	ret = drmModeObjectGetProperties(fd, 0, 0);
	ASSERT_NE(nullptr, ret);
	EXPECT_EQ(7, ret->count_props);
	ASSERT_NE(nullptr, ret->props);
	EXPECT_EQ(0, ret->props[0]);
	EXPECT_EQ(1, ret->props[1]);
	EXPECT_EQ(2, ret->props[2]);
	EXPECT_EQ(3, ret->props[3]);
	EXPECT_EQ(4, ret->props[4]);
	EXPECT_EQ(5, ret->props[5]);
	EXPECT_EQ(6, ret->props[6]);
	ASSERT_NE(nullptr, ret->prop_values);
	EXPECT_EQ(0, ret->prop_values[0]);
	EXPECT_EQ(0, ret->prop_values[1]);
	EXPECT_EQ(0, ret->prop_values[2]);
	EXPECT_EQ(0, ret->prop_values[3]);
	EXPECT_EQ(0, ret->prop_values[4]);
	EXPECT_EQ(0, ret->prop_values[5]);
	EXPECT_EQ(0, ret->prop_values[6]);

	system("touch " STUB_PROP_CTL);			/* HDCP動作開始 */
	WaitStatus();
	result = GetValueDrmProperty(tbl[1].str, value);
	EXPECT_EQ(VHAL_SUCCESS, result);
	EXPECT_EQ(0x01, value);

	system("touch " STUB_SYSFS_CTL);		/* sysfsファイルを更新し、HDCP動作継続 */
	/* 内部の遷移チェック */
	usleep(100000);
	EXPECT_EQ(0x02, ret->prop_values[1]);
	usleep(200000);
	EXPECT_EQ(0x03, ret->prop_values[1]);
	WaitStatus();
	/* 正常完了の遷移完了後のチェック */
	result = GetValueDrmProperty(tbl[1].str, value);
	EXPECT_EQ(VHAL_SUCCESS, result);
	EXPECT_EQ(0x04, value);
	EXPECT_EQ(0x04, ret->prop_values[1]);
	result = GetValueDrmProperty(tbl[2].str, value);
	EXPECT_EQ(VHAL_SUCCESS, result);
	EXPECT_EQ(3, value);
	EXPECT_EQ(3, ret->prop_values[2]);
	result = GetValueDrmProperty(tbl[3].str, value);
	EXPECT_EQ(VHAL_SUCCESS, result);
	EXPECT_EQ(0x01, value);
	EXPECT_EQ(0x01, ret->prop_values[3]);
	result = GetValueDrmProperty(tbl[4].str, value);
	EXPECT_EQ(VHAL_SUCCESS, result);
	EXPECT_EQ(0xaaaaaaaaaa, value);
	EXPECT_EQ(0xaaaaaaaaaa, ret->prop_values[4]);
	result = GetValueDrmProperty(tbl[5].str, value);
	EXPECT_EQ(VHAL_SUCCESS, result);
	EXPECT_EQ(0xbbbbbbbbbb, value);
	EXPECT_EQ(0xbbbbbbbbbb, ret->prop_values[5]);
	result = GetValueDrmProperty(tbl[6].str, value);
	EXPECT_EQ(VHAL_SUCCESS, result);
	EXPECT_EQ(0xcccccccccc, value);
	EXPECT_EQ(0xcccccccccc, ret->prop_values[6]);

	system("touch " STUB_RESET_CTL);		/* 値のクリア */
	usleep(100000);		/* 一定時間待つ */
}

/*---------------------------------------------------- */
/* プロパティの値変更と取得(HDCP異常) */
TEST_F(DrmTest_Api, drmModeObjectGetProperties_Change2)
{
	int	fd = 3;
	drmModeObjectPropertiesPtr	ret;
	const struct {
		std::string	str;
		uint64_t		value;
	} tbl[7] = {
		{ "gvif-tx-spec"				, 0  } ,
		{ "HDCP Auth Status"			, 0x0f } ,
		{ "HDCP Device Count"			, 0x1234567890123456 } ,
		{ "HDCP Repeater Cascade Depth"	, 0x9876543210987654 } ,
		{ "HDCP Receiver ID 0"			, 0x0123456789abcdef } ,
		{ "HDCP Receiver ID 1"			, 0xfedcba9876543210 } ,
		{ "HDCP Receiver ID 2"			, 0xffffffffffffffff } ,
	};
	uint64_t	value;
	int32_t		result;

	system("touch " STUB_RESET_CTL);		/* 値のクリア */
	usleep(100000);		/* 一定時間待つ */

	/* value値がクリアしていること */
	ret = drmModeObjectGetProperties(fd, 0, 0);
	ASSERT_NE(nullptr, ret);
	EXPECT_EQ(7, ret->count_props);
	ASSERT_NE(nullptr, ret->props);
	EXPECT_EQ(0, ret->props[0]);
	EXPECT_EQ(1, ret->props[1]);
	EXPECT_EQ(2, ret->props[2]);
	EXPECT_EQ(3, ret->props[3]);
	EXPECT_EQ(4, ret->props[4]);
	EXPECT_EQ(5, ret->props[5]);
	EXPECT_EQ(6, ret->props[6]);
	ASSERT_NE(nullptr, ret->prop_values);
	EXPECT_EQ(0, ret->prop_values[0]);
	EXPECT_EQ(0, ret->prop_values[1]);
	EXPECT_EQ(0, ret->prop_values[2]);
	EXPECT_EQ(0, ret->prop_values[3]);
	EXPECT_EQ(0, ret->prop_values[4]);
	EXPECT_EQ(0, ret->prop_values[5]);
	EXPECT_EQ(0, ret->prop_values[6]);

	system("touch " STUB_PROP_CTL);			/* HDCP動作開始 */
	WaitStatus();
	result = GetValueDrmProperty(tbl[1].str, value);
	EXPECT_EQ(VHAL_SUCCESS, result);
	EXPECT_EQ(0x01, value);

	/* ファイルの値書換え */
	StubWrite(STUB_PROP_STS(1), &tbl[1].value, sizeof(tbl[1].value));
	StubWrite(STUB_PROP_STS(2), &tbl[2].value, sizeof(tbl[2].value));
	StubWrite(STUB_PROP_STS(3), &tbl[3].value, sizeof(tbl[3].value));
	StubWrite(STUB_PROP_STS(4), &tbl[4].value, sizeof(tbl[4].value));
	StubWrite(STUB_PROP_STS(5), &tbl[5].value, sizeof(tbl[5].value));
	StubWrite(STUB_PROP_STS(6), &tbl[6].value, sizeof(tbl[6].value));
	
	system("touch " STUB_SYSFS_CTL);		/* sysfsファイルを更新し、HDCP動作継続 */
	/* 内部の遷移チェック */
	usleep(100000);
	EXPECT_EQ(0x02, ret->prop_values[1]);
	usleep(200000);
	EXPECT_EQ(0x03, ret->prop_values[1]);
	WaitStatus();
	/* 正常完了の遷移完了後のチェック */
	result = GetValueDrmProperty(tbl[1].str, value);
	EXPECT_EQ(VHAL_SUCCESS, result);
	EXPECT_EQ(0x01, value);
	EXPECT_EQ(0x01, ret->prop_values[1]);
	result = GetValueDrmProperty(tbl[2].str, value);
	EXPECT_EQ(VHAL_SUCCESS, result);
	EXPECT_EQ(0x1234567890123456, value);
	EXPECT_EQ(0x1234567890123456, ret->prop_values[2]);
	result = GetValueDrmProperty(tbl[3].str, value);
	EXPECT_EQ(VHAL_SUCCESS, result);
	EXPECT_EQ(0x9876543210987654, value);
	EXPECT_EQ(0x9876543210987654, ret->prop_values[3]);
	result = GetValueDrmProperty(tbl[4].str, value);
	EXPECT_EQ(VHAL_SUCCESS, result);
	EXPECT_EQ(0x0123456789abcdef, value);
	EXPECT_EQ(0x0123456789abcdef, ret->prop_values[4]);
	result = GetValueDrmProperty(tbl[5].str, value);
	EXPECT_EQ(VHAL_SUCCESS, result);
	EXPECT_EQ(0xfedcba9876543210, value);
	EXPECT_EQ(0xfedcba9876543210, ret->prop_values[5]);
	result = GetValueDrmProperty(tbl[6].str, value);
	EXPECT_EQ(VHAL_SUCCESS, result);
	EXPECT_EQ(0xffffffffffffffff, value);
	EXPECT_EQ(0xffffffffffffffff, ret->prop_values[6]);

	system("touch " STUB_RESET_CTL);		/* 値のクリア */
	usleep(100000);		/* 一定時間待つ */
}

/*---------------------------------------------------- */
