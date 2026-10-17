/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * ti_am335x_common.h
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - https://www.ti.com/
 *
 * For more details, please see the technical documents listed at
 * https://www.ti.com/product/am3359#technicaldocuments
 */

#ifndef __CONFIG_TI_AM335X_COMMON_H__
#define __CONFIG_TI_AM335X_COMMON_H__

#define CFG_MAX_RAM_BANK_SIZE	(1024 << 20)	/* 1GB */
#define CFG_SYS_TIMERBASE		0x48040000	/* Use Timer2 */

#include <asm/arch/omap.h>

/* NS16550 Configuration */
#define CFG_SYS_NS16550_CLK		48000000

/*
 * SPL related defines.  The Public RAM memory map the ROM defines the
 * area between 0x402F0400 and 0x4030B800 as a download area and
 * 0x4030B800 to 0x4030CE00 as a public stack area.  The ROM also
 * supports X-MODEM loading via UART, and we leverage this and then use
 * Y-MODEM to load u-boot.img, when booted over UART.
 */

/* Enable the watchdog inside of SPL */

/*
 * Since SPL did pll and ddr initialization for us,
 * we don't need to do it twice.
 */

/*
 * When building U-Boot such that there is no previous loader
 * we need to call board_early_init_f.  This is taken care of in
 * s_init when we have SPL used.
 */

/* Now bring in the rest of the common code. */
#include <configs/ti_armv7_omap.h>

#endif	/* __CONFIG_TI_AM335X_COMMON_H__ */
