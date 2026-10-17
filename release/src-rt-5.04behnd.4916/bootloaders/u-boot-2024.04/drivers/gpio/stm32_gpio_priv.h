/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#ifndef _STM32_GPIO_PRIV_H_
#define _STM32_GPIO_PRIV_H_

enum stm32_gpio_mode {
	STM32_GPIO_MODE_IN = 0,
	STM32_GPIO_MODE_OUT,
	STM32_GPIO_MODE_AF,
	STM32_GPIO_MODE_AN
};

enum stm32_gpio_otype {
	STM32_GPIO_OTYPE_PP = 0,
	STM32_GPIO_OTYPE_OD
};

enum stm32_gpio_speed {
	STM32_GPIO_SPEED_2M = 0,
	STM32_GPIO_SPEED_25M,
	STM32_GPIO_SPEED_50M,
	STM32_GPIO_SPEED_100M
};

enum stm32_gpio_pupd {
	STM32_GPIO_PUPD_NO = 0,
	STM32_GPIO_PUPD_UP,
	STM32_GPIO_PUPD_DOWN
};

enum stm32_gpio_af {
	STM32_GPIO_AF0 = 0,
	STM32_GPIO_AF1,
	STM32_GPIO_AF2,
	STM32_GPIO_AF3,
	STM32_GPIO_AF4,
	STM32_GPIO_AF5,
	STM32_GPIO_AF6,
	STM32_GPIO_AF7,
	STM32_GPIO_AF8,
	STM32_GPIO_AF9,
	STM32_GPIO_AF10,
	STM32_GPIO_AF11,
	STM32_GPIO_AF12,
	STM32_GPIO_AF13,
	STM32_GPIO_AF14,
	STM32_GPIO_AF15
};

struct stm32_gpio_dsc {
	u8	port;
	u8	pin;
};

struct stm32_gpio_ctl {
	enum stm32_gpio_mode	mode;
	enum stm32_gpio_otype	otype;
	enum stm32_gpio_speed	speed;
	enum stm32_gpio_pupd	pupd;
	enum stm32_gpio_af	af;
};

struct stm32_gpio_regs {
	u32 moder;	/* GPIO port mode */
	u32 otyper;	/* GPIO port output type */
	u32 ospeedr;	/* GPIO port output speed */
	u32 pupdr;	/* GPIO port pull-up/pull-down */
	u32 idr;	/* GPIO port input data */
	u32 odr;	/* GPIO port output data */
	u32 bsrr;	/* GPIO port bit set/reset */
	u32 lckr;	/* GPIO port configuration lock */
	u32 afr[2];	/* GPIO alternate function */
};

struct stm32_gpio_priv {
	struct stm32_gpio_regs *regs;
	unsigned int gpio_range;
};

#endif /* _STM32_GPIO_PRIV_H_ */
