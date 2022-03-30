/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Linaro
 * Peter Griffin <peter.griffin@linaro.org>
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
	PERIPH_ID_UART0 = 36,
	PERIPH_ID_UART1,
	PERIPH_ID_UART2,
	PERIPH_ID_UART3,
	PERIPH_ID_UART4,
	PERIPH_ID_UART5,
	PERIPH_ID_SDMMC0 = 72,
	PERIPH_ID_SDMMC1,

	PERIPH_ID_NONE = -1,
};

#endif /* __ASM_ARM_ARCH_PERIPH_H */
