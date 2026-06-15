
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "hal_gpio_public.h"

/*
各デバッグ用ファイルの説明

戻り値格納ファイル：
    戻り値以上のサイズがある場合は、読み込んだ内容が戻り値となる。
    ファイルがない、またはサイズ不足の場合はデフォルト値を返す。
    一度戻り値を返すと、ファイルサイズが0になる。

以下の関数(ファイル)の戻り値を返すことが可。
・/run/arene/vehicle_fs/var/bev3/stub/hal/func_ret/SetValue
・/run/arene/vehicle_fs/var/bev3/stub/hal/func_ret/SetDirection
*/

#define STUB_PATH				"/run/arene/vehicle_fs/var/bev3/stub/hal/"
#define STUB_PATH_FUNC			STUB_PATH "func_ret/"
#define STUB_FUNC(_fnc)			STUB_PATH_FUNC #_fnc
#define STUB_CALL(_fnc)			STUB_PATH #_fnc "_call"

static int calledFirst_ = {FALSE};

/* 関数の戻り値を設定する(数値) */
static INT32 ReadFuncResult(const char* path, INT32 defValue)
{
	INT32 value = defValue;
	if (StubClientRead_bin(path, (int*)&value, sizeof(value)))
	{
		value = defValue;
	}
	return value;
}

/*---------------------------------------------------- */
/* オープン */
void* HalOpen(HalModuleId module_id)
{
	/* 初回のみ実施 */
	if (TRUE != calledFirst_)
	{
		calledFirst_ = TRUE;

		/* FileObserver用フォルダ作成 */
		system("mkdir -p " STUB_PATH_FUNC);

		/* 関数戻り値ファイル作成 */
		system("touch " STUB_FUNC(SetValue));
		system("touch " STUB_FUNC(SetDirection));
	}

	/* HalModuleIdのサイズでmallocしたアドレスを戻り値として返す */
	return malloc(sizeof(HalModuleId));
}

/*---------------------------------------------------- */
/* クローズ */
INT32 HalClose(void* hmi)
{
	/* HalOpen時にmallocしたアドレス(引数hmi)をfreeする */
	free(hmi);
	return HAL_SUCCESS;
}

/*---------------------------------------------------- */
/* 値取得 */
static INT32 GetValue(void* hmi, HalGpioId id)
{
	(void)hmi;
	(void)id;
	/* callイベント発生 */
	system("echo test > " STUB_CALL(GetValue));
	return (INT32)HAL_GPIO_LOW;
}

/*---------------------------------------------------- */
/* 値設定 */
static INT32 SetValue(void* hmi, HalGpioId id, HalGpioLevel val)
{
	(void)hmi;
	(void)id;
	(void)val;
	/* callイベント発生 */
	system("echo test > " STUB_CALL(SetValue));
	return ReadFuncResult(STUB_FUNC(SetValue), HAL_SUCCESS);
}

/*---------------------------------------------------- */
/* IN/OUT設定 */
static INT32 SetDirection(void* hmi, HalGpioId id, HalGpioDir dir)
{
	(void)hmi;
	(void)id;
	(void)dir;
	/* callイベント発生 */
	system("echo test > " STUB_CALL(SetDirection));
	return ReadFuncResult(STUB_FUNC(SetDirection), HAL_SUCCESS);
}

/*---------------------------------------------------- */
/* 関数アドレステーブル */
static HalGpioDeviceOps device =
{
	.getValue = GetValue,
	.setValue = SetValue,
	.setDirection = SetDirection,
};

/*---------------------------------------------------- */
/* 関数アドレステーブル取得 */
HalGpioDeviceOps* HalGpioGetMethods(void* hmi)
{
	(void)hmi;
	/* callイベント発生 */
	system("echo test > " STUB_CALL(HalGpioGetMethods));
	return &device;
}
/*---------------------------------------------------- */
