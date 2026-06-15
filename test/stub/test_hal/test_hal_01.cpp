
#include <unistd.h>
#include "test_main.h"
#include "file_observer.h"
#include "hal_gpio_public.h"
#include "stub_common.h"
#include <condition_variable>

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

#define STUB_PATH					"/run/arene/vehicle_fs/var/bev3/stub/hal/"
#define STUB_PATH_FUNC				STUB_PATH "func_ret/"
#define STUB_FUNC(_fnc)				STUB_PATH_FUNC #_fnc
#define STUB_CALL(_fnc)				STUB_PATH #_fnc "_call"

enum {
	eHalGVal ,
	eHalSVal ,
	eHalSDir ,
	eHalGMeth ,
	eHalMAX
};

static std::mutex mtx_sync[eHalMAX];
static std::condition_variable cond_sync[eHalMAX];
static bool detect_ntfy = false;
static bool first_run = true;

class HalTest_Api: public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();
};

void HalTest_Api::SetUp()
{
	if (true == first_run)
	{
		first_run = false;
		system("rm -fr " STUB_PATH_FUNC);
	}
}

void HalTest_Api::TearDown()
{
}

/*---------------------------------------------------- */
static void StubWrite(const char* path, const INT32 val)
{
	cockpit::bs::CFileObserver::GetInstance()->Write(path, &val, sizeof(val));
}

/*---------------------------------------------------- */
static void StubWrite(const char* path, const void* ptr)
{
	cockpit::bs::CFileObserver::GetInstance()->Write(path, (void*)&ptr, sizeof(ptr));
}

/*---------------------------------------------------- */
static void StubWrite(const char* path, const void* ptr, size_t size)
{
	cockpit::bs::CFileObserver::GetInstance()->Write(path, ptr, size);
}

/*---------------------------------------------------- */
/* GetValueのcall時に実施するCB */
static void GetValueListener(const char* p_path)
{
	if (true == detect_ntfy)
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync[eHalGVal]);
		cond_sync[eHalGVal].notify_one();
	}
}

/*---------------------------------------------------- */
/* SetValueのcall時に実施するCB */
static void SetValueListener(const char* p_path)
{
	if (true == detect_ntfy)
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync[eHalSVal]);
		cond_sync[eHalSVal].notify_one();
	}
}

/*---------------------------------------------------- */
/* SetDirectionのcall時に実施するCB */
static void SetDirectionListener(const char* p_path)
{
	if (true == detect_ntfy)
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync[eHalSDir]);
		cond_sync[eHalSDir].notify_one();
	}
}

/*---------------------------------------------------- */
/* HalGpioGetMethodsのcall時に実施するCB */
static void HalGpioGetMethodsListener(const char* p_path)
{
	if (true == detect_ntfy)
	{
		std::lock_guard<std::mutex> lock_sync(mtx_sync[eHalGMeth]);
		cond_sync[eHalGMeth].notify_one();
	}
}



//
// Googletest TestCase
//
/*---------------------------------------------------- */
TEST_F(HalTest_Api, HalOpen)
{
	void* hmi = nullptr;

	/* 戻り値に何も指定せずに実施 */
	hmi = HalOpen(HAL_ID_GPIO);		/* hmiにはポインタ値が返るはず */
	EXPECT_NE(nullptr, hmi);		/* 0以外であること */
	free(hmi);
}

TEST_F(HalTest_Api, HalClose)
{
	void*	hmi;
	INT32	ret;

	/* 戻り値に何も指定せずに実施 */
	hmi = HalOpen(HAL_ID_GPIO);
	ret = HalClose(hmi);
	EXPECT_EQ(HAL_SUCCESS, ret);
}

/*---------------------------------------------------- */
TEST_F(HalTest_Api, HalGpioGetMethods)
{
	void*	hmi = NULL;
	HalGpioDeviceOps*	ret;

	/* 戻り値に何も指定せずに実施 */
	ret = HalGpioGetMethods(hmi);
	EXPECT_NE(nullptr, ret);		/* 0以外であること */

	/* _call調査(成功) */
	StubClientRegisterListerner(STUB_CALL(HalGpioGetMethods), HalGpioGetMethodsListener);
	{
		detect_ntfy = true;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eHalGMeth]);
		ret = HalGpioGetMethods(hmi);
		EXPECT_NE(nullptr, ret);		/* 0以外であること */
		std::cv_status intime = cond_sync[eHalGMeth].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::no_timeout, intime);
	}

	/* _call調査(タイムアウト) */
	{
		detect_ntfy = false;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eHalGMeth]);
		ret = HalGpioGetMethods(hmi);
		EXPECT_NE(nullptr, ret);		/* 0以外であること */
		std::cv_status intime = cond_sync[eHalGMeth].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::timeout, intime);
	}

}

/*---------------------------------------------------- */
TEST_F(HalTest_Api, GetValue)
{
	void*	hmi = NULL;
	INT32	ret;
	HalGpioDeviceOps*	ops = HalGpioGetMethods(hmi);

	/* 戻り値に何も指定せずに実施 */
	ret = ops->getValue(hmi, HAL_GPIO_O_USB_HUB_RST);
	EXPECT_EQ(HAL_GPIO_LOW, ret);

	/* _call調査(成功) */
	StubClientRegisterListerner(STUB_CALL(GetValue), GetValueListener);
	{
		detect_ntfy = true;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eHalGVal]);
		ret = ops->getValue(hmi, HAL_GPIO_O_USB_HUB_RST);
		EXPECT_EQ(HAL_GPIO_LOW, ret);
		std::cv_status intime = cond_sync[eHalGVal].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::no_timeout, intime);
	}

	/* _call調査(タイムアウト) */
	{
		detect_ntfy = false;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eHalGVal]);
		ret = ops->getValue(hmi, HAL_GPIO_O_USB_HUB_RST);
		EXPECT_EQ(HAL_GPIO_LOW, ret);
		std::cv_status intime = cond_sync[eHalGVal].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::timeout, intime);
	}
}

/*---------------------------------------------------- */
TEST_F(HalTest_Api, SetValue)
{
	void*	hmi = NULL;
	INT32	ret;
	HalGpioDeviceOps*	ops = HalGpioGetMethods(hmi);

	/* 戻り値に何も指定せずに実施 */
	ret =  ops->setValue(hmi, HAL_GPIO_O_GVIF_TX_METER_RST, HAL_GPIO_LOW);
	EXPECT_EQ(HAL_SUCCESS, ret);

	/* 戻り値にHAL_ERR_ARGを指定 */
	StubWrite(STUB_FUNC(SetValue), HAL_ERR_ARG);
	ret = ops->setValue(hmi, HAL_GPIO_O_GVIF_TX_METER_RST, HAL_GPIO_LOW);
	EXPECT_EQ(HAL_ERR_ARG, ret);

	/* 戻り値にHAL_SUCCESSを指定 */
	StubWrite(STUB_FUNC(SetValue), HAL_SUCCESS);
	ret = ops->setValue(hmi, HAL_GPIO_O_GVIF_TX_METER_RST, HAL_GPIO_LOW);
	EXPECT_EQ(HAL_SUCCESS, ret);

	/* _call調査(成功) */
	StubClientRegisterListerner(STUB_CALL(SetValue), SetValueListener);
	{
		detect_ntfy = true;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eHalSVal]);
		ret =  ops->setValue(hmi, HAL_GPIO_O_GVIF_TX_METER_RST, HAL_GPIO_LOW);
		EXPECT_EQ(HAL_SUCCESS, ret);
		std::cv_status intime = cond_sync[eHalSVal].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::no_timeout, intime);
	}

	/* _call調査(タイムアウト) */
	{
		detect_ntfy = false;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eHalSVal]);
		ret =  ops->setValue(hmi, HAL_GPIO_O_GVIF_TX_METER_RST, HAL_GPIO_LOW);
		EXPECT_EQ(HAL_SUCCESS, ret);
		std::cv_status intime = cond_sync[eHalSVal].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::timeout, intime);
	}
}

/*---------------------------------------------------- */
TEST_F(HalTest_Api, SetDirection)
{
	void*	hmi = NULL;
	INT32	ret;
	HalGpioDeviceOps*	ops = HalGpioGetMethods(hmi);

	/* 戻り値に何も指定せずに実施 */
	ret = ops->setDirection(hmi, HAL_GPIO_O_GVIF_TX_METER_RST, HAL_GPIO_OUT);
	EXPECT_EQ(HAL_SUCCESS, ret);

	/* 戻り値にHAL_ERR_ARGを指定 */
	StubWrite(STUB_FUNC(SetDirection), HAL_ERR_ARG);
	ret = ops->setDirection(hmi, HAL_GPIO_O_GVIF_TX_METER_RST, HAL_GPIO_OUT);
	EXPECT_EQ(HAL_ERR_ARG, ret);

	/* 戻り値にHAL_SUCCESSを指定 */
	StubWrite(STUB_FUNC(SetDirection), HAL_SUCCESS);
	ret = ops->setDirection(hmi, HAL_GPIO_O_GVIF_TX_METER_RST, HAL_GPIO_OUT);
	EXPECT_EQ(HAL_SUCCESS, ret);

	/* _call調査(成功) */
	StubClientRegisterListerner(STUB_CALL(SetDirection), SetDirectionListener);
	{
		detect_ntfy = true;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eHalSDir]);
		ret = ops->setDirection(hmi, HAL_GPIO_O_GVIF_TX_METER_RST, HAL_GPIO_OUT);
		EXPECT_EQ(HAL_SUCCESS, ret);
		std::cv_status intime = cond_sync[eHalSDir].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::no_timeout, intime);
	}

	/* _call調査(タイムアウト) */
	{
		detect_ntfy = false;
		std::unique_lock<std::mutex> lock_sync(mtx_sync[eHalSDir]);
		ret = ops->setDirection(hmi, HAL_GPIO_O_GVIF_TX_METER_RST, HAL_GPIO_OUT);
		EXPECT_EQ(HAL_SUCCESS, ret);
		std::cv_status intime = cond_sync[eHalSDir].wait_for(lock_sync, std::chrono::milliseconds(200));
		EXPECT_EQ(std::cv_status::timeout, intime);
	}
}

/*---------------------------------------------------- */
