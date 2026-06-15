/*******************************************************************************
    機能名称    ：  GPIO制御モジュール
    ファイル名称：  vhal_gpio_control.h
*******************************************************************************/
#ifndef	VHAL_GPIO_CONTROL_H
#define	VHAL_GPIO_CONTROL_H

#include <string>

extern "C"
{
	#include "hal_gpio_public.h"
}

namespace videohal
{

/*****************************************************************************
 クラス名称：CVhalGpioControl
 処理概要  ：GPIOの制御を行う。
*****************************************************************************/
class CVhalGpioControl {
public:
	CVhalGpioControl(void);
	virtual ~CVhalGpioControl(void);
	CVhalGpioControl(const CVhalGpioControl& src) = delete;
	CVhalGpioControl& operator=(const CVhalGpioControl& src) & = delete;
	CVhalGpioControl(CVhalGpioControl&& src) = delete;
	CVhalGpioControl& operator=(CVhalGpioControl&& src) & = delete;

	int32_t Initialize(void);
	void Finalize(void);

	int32_t SetValueDirectionLow(const HalGpioId gpioId, const HalGpioLevel value) const;

private:
	void*				p_hmi_{nullptr};		/* HALモジュール情報ハンドル  */
	HalGpioDeviceOps*	p_methods_{nullptr};	/* HAL GPIO API一覧構造体     */
};

} /* namespace videohal */


#endif	/* #ifndef	VHAL_GPIO_CONTROL_H */
