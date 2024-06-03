/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * ti_omap3_common.h
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 *
 * For more details, please see the technical documents listed at
 *   http://www.ti.com/product/omap3530
 *   http://www.ti.com/product/omap3630
 *   http://www.ti.com/product/dm3730
 */

#ifndef __CONFIG_TI_OMAP3_COMMON_H__
#define __CONFIG_TI_OMAP3_COMMON_H__

/*
 * High Level Configuration Options
 */

#include <asm/arch/cpu.h>
#include <asm/arch/omap.h>

/* Clock Defines */
#define V_OSCK			26000000	/* Clock output from T2 */
#define V_SCLK			(V_OSCK >> 1)

/* NS16550 Configuration */
#define V_NS16550_CLK			48000000	/* 48MHz (APLL96/2) */
#define CONFIG_SYS_NS16550_CLK		V_NS16550_CLK
#if !defined(CONFIG_DM_SERIAL)
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#endif /* !CONFIG_DM_SERIAL */
#define CONFIG_SYS_BAUDRATE_TABLE	{4800, 9600, 19200, 38400, 57600, \
					115200}

/* Select serial console configuration */
#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_NS16550_COM1		OMAP34XX_UART1
#define CONFIG_SYS_NS16550_COM2		OMAP34XX_UART2
#define CONFIG_SYS_NS16550_COM3		OMAP34XX_UART3
#endif

/* Physical Memory Map */
#define PHYS_SDRAM_1		OMAP34XX_SDRC_CS0
#define PHYS_SDRAM_2		OMAP34XX_SDRC_CS1

/*
 * OMAP3 has 12 GP timers, they can be driven by the system clock
 * (12/13/16.8/19.2/38.4MHz) or by 32KHz clock. We use 13MHz (V_SCLK).
 * This rate is divided by a local divisor.
 */
#define CONFIG_SYS_TIMERBASE		(OMAP34XX_GPT2)

#define CONFIG_SYS_MONITOR_LEN		(256 << 10)

/* SPL */
#define CONFIG_SYS_SPL_ARGS_ADDR	(CONFIG_SYS_SDRAM_BASE + \
					 (64 << 20))

#ifdef CONFIG_NAND
#define CONFIG_SYS_NAND_BASE		0x30000000
#endif

/* Now bring in the rest of the common code. */
#include <configs/ti_armv7_omap.h>

#endif	/* __CONFIG_TI_OMAP3_COMMON_H__ */
