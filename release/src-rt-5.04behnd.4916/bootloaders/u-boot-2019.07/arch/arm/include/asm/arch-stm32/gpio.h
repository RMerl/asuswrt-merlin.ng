/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#ifndef _GPIO_H_
#define _GPIO_H_

#define STM32_GPIOS_PER_BANK		16

enum stm32_gpio_port {
	STM32_GPIO_PORT_A = 0,
	STM32_GPIO_PORT_B,
	STM32_GPIO_PORT_C,
	STM32_GPIO_PORT_D,
	STM32_GPIO_PORT_E,
	STM32_GPIO_PORT_F,
	STM32_GPIO_PORT_G,
	STM32_GPIO_PORT_H,
	STM32_GPIO_PORT_I
};

enum stm32_gpio_pin {
	STM32_GPIO_PIN_0 = 0,
	STM32_GPIO_PIN_1,
	STM32_GPIO_PIN_2,
	STM32_GPIO_PIN_3,
	STM32_GPIO_PIN_4,
	STM32_GPIO_PIN_5,
	STM32_GPIO_PIN_6,
	STM32_GPIO_PIN_7,
	STM32_GPIO_PIN_8,
	STM32_GPIO_PIN_9,
	STM32_GPIO_PIN_10,
	STM32_GPIO_PIN_11,
	STM32_GPIO_PIN_12,
	STM32_GPIO_PIN_13,
	STM32_GPIO_PIN_14,
	STM32_GPIO_PIN_15
};

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
	enum stm32_gpio_port	port;
	enum stm32_gpio_pin	pin;
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

int stm32_offset_to_index(struct udevice *dev, unsigned int offset);

#endif /* _GPIO_H_ */
