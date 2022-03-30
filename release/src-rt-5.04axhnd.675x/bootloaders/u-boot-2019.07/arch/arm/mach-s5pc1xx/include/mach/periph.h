/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Samsung Electronics
 * Rajeshwari Shinde <rajeshwari.s@samsung.com>
 */

#ifndef __ASM_ARM_ARCH_PERIPH_H
#define __ASM_ARM_ARCH_PERIPH_H

/*
 * Peripherals required for pinmux configuration. List will
 * grow with support for more devices getting added.
 * Numbering based on interrupt table.
 *
 */
enum periph_id {
	PERIPH_ID_UART0 = 51,
	PERIPH_ID_UART1,
	PERIPH_ID_UART2,
	PERIPH_ID_UART3,
	PERIPH_ID_I2C0 = 56,
	PERIPH_ID_I2C1,
	PERIPH_ID_I2C2,
	PERIPH_ID_I2C3,
	PERIPH_ID_I2C4,
	PERIPH_ID_I2C5,
	PERIPH_ID_I2C6,
	PERIPH_ID_I2C7,
	PERIPH_ID_SPI0 = 68,
	PERIPH_ID_SPI1,
	PERIPH_ID_SPI2,
	PERIPH_ID_SDMMC0 = 75,
	PERIPH_ID_SDMMC1,
	PERIPH_ID_SDMMC2,
	PERIPH_ID_SDMMC3,
	PERIPH_ID_I2C8 = 87,
	PERIPH_ID_I2C9,
	PERIPH_ID_I2S0 = 98,
	PERIPH_ID_I2S1 = 99,

	/* Since following peripherals do
	 * not have shared peripheral interrupts (SPIs)
	 * they are numbered arbitiraly after the maximum
	 * SPIs Exynos has (128)
	 */
	PERIPH_ID_SROMC = 128,
	PERIPH_ID_SPI3,
	PERIPH_ID_SPI4,
	PERIPH_ID_SDMMC4,
	PERIPH_ID_PWM0,
	PERIPH_ID_PWM1,
	PERIPH_ID_PWM2,
	PERIPH_ID_PWM3,
	PERIPH_ID_PWM4,
	PERIPH_ID_I2C10 = 203,

	PERIPH_ID_NONE = -1,
};

#endif /* __ASM_ARM_ARCH_PERIPH_H */
