/*******************************************************************************
    機能名称    ：  GPIO制御モジュール
    ファイル名称：  vhal_gpio_control.cpp
*******************************************************************************/
#include "vhal_gpio_control.h"

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_debug_system.h"

/*****************************************************************************
*	define																	 *
*****************************************************************************/

/*****************************************************************************
*	globals																	 *
*****************************************************************************/

/*****************************************************************************
*	statics																	 *
*****************************************************************************/

namespace videohal
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalGpioControl::CVhalGpioControl(void)
{
 	VHAL_LOGV("CVhalGpioControl is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalGpioControl::~CVhalGpioControl(void)
{
	VHAL_LOGV("CVhalGpioControl is deleted. this=%p", this);
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ：	
 戻り値  ：	処理結果
           		VHAL_ERR_****		エラー
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-C-155
                     	F-VHAL-C-156
*****************************************************************************/
int32_t CVhalGpioControl::Initialize(void)
{
	p_hmi_ = HalOpen(HAL_ID_GPIO);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-C-155",fail_ret)};
//	if(true == fail)
//	{
//		p_hmi_ = nullptr;
//	}
//#endif
	if (nullptr != p_hmi_)
	{
		p_methods_ = HAL_GET_GPIO_METHODS(p_hmi_);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret{0};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-C-156",fail_ret)};
//		if(true == fail)
//		{
//			p_methods_ = nullptr;
//		}
//#endif
	}
	if (nullptr == p_methods_)
	{
		VHAL_LOGE("HalOpen() error. (HalOpen()=%p)", p_hmi_);
		return VHAL_ERR_GPIO_API;
	}
	VHAL_LOGI("HalOpen() hmi:%p, methods:%p", p_hmi_, p_methods_);

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	終了処理（内部リソースの解放）
 引数    ：	なし
 戻り値  ：	なし
 フェールセーフNo  ：	F-VHAL-N-159
*****************************************************************************/
void CVhalGpioControl::Finalize(void)
{
	if (nullptr != p_hmi_)
	{
		int32_t ret{HAL_SUCCESS};
		ret = HalClose(p_hmi_);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret{0};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-159",fail_ret)};
//		if(true == fail)
//		{
//			ret = fail_ret;
//		}
//#endif
		if (HAL_SUCCESS != ret)
		{
			VHAL_LOGE("HalClose error. (ret=%d)", ret);
		}
	}
}

/*****************************************************************************
 処理概要：	GPIOのWrite処理(出力(LOW))
 引数    ：	HalGpioId gpioId		(i)GPIO ID
           	HalGpioLevel value		(i)HAL_GPIO_LOW/HAL_GPIO_HIGH
 戻り値  ：	処理結果
           		VHAL_ERR_****		エラー
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-N-157
                     	F-VHAL-N-158
*****************************************************************************/
int32_t CVhalGpioControl::SetValueDirectionLow(const HalGpioId gpioId, const HalGpioLevel value) const
{
	int32_t result{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	VHAL_LOGI("HalGpioId=%d, value=%d", gpioId, value);

	/*
	 * p_hmi_ is the return value of HalOpen and HAL_ERR_INVALID_HMI_HANDLE should be used
	 * to check if it's invalid. but it's also coding violation for AUTOSAR C++14 A4-10-1
	 * since HAL_ERR_INVALID_HMI_HANDLE is defined as (NULL). 
	 * so use nullptr instead of HAL_ERR_INVALID_HMI_HANDLE
	 */
	if ((nullptr == p_hmi_) || (nullptr == p_methods_))
	{
		VHAL_LOGE("parameter error. p_hmi=%p, p_methods_=%p", p_hmi_, p_methods_);
		result = VHAL_ERR_PARAM;
	}
	else
	{
		/* 端子入出力初期設定 */
		int32_t ret{HAL_SUCCESS};
		ret = p_methods_->setDirection(p_hmi_, gpioId, HAL_GPIO_OUT_LOW);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret{0};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-158",fail_ret)};
//		if(true == fail)
//		{
//			ret = fail_ret;
//		}
//#endif
		if (HAL_SUCCESS != ret)
		{
			VHAL_LOGE("HAL setDirection error=%d", ret);
			result = VHAL_ERR_GPIO_API;
		}
		else
		{
			/* 端子操作 */
			ret = p_methods_->setValue(p_hmi_, gpioId, value);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//			int32_t fail_ret{0};
//			bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-157",fail_ret)};
//			if(true == fail)
//			{
//				ret = fail_ret;
//			}
//#endif
			if (HAL_SUCCESS != ret)
			{
				VHAL_LOGE("HAL setValue error=%d", ret);
				result = VHAL_ERR_GPIO_API;
			}
		}
	}

	VHAL_LOGV_OUT();
	return result;
}

} /* namespace videohal */

