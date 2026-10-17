/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Embedded Artists i.MX7ULP COM board.
 */

#ifndef __MX7ULP_COM_CONFIG_H
#define __MX7ULP_COM_CONFIG_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

#ifdef CONFIG_SPL
#include "imx7ulp_spl.h"
#endif

/* Using ULP WDOG for reset */
#define WDOG_BASE_ADDR			WDG1_RBASE

#define CFG_SYS_HZ_CLOCK		1000000 /* Fixed at 1MHz from TSTMR */

/* UART */
#define LPUART_BASE			LPUART4_RBASE

/* Physical Memory Map */

#define PHYS_SDRAM			0x60000000
#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM

#define CFG_EXTRA_ENV_SETTINGS \
	"image=zImage\0" \
	"console=ttyLP0\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdt_file=imx7ulp-com.dtb\0" \
	"fdt_addr=0x63000000\0" \
	"mmcdev="__stringify(CONFIG_SYS_MMC_ENV_DEV)"\0" \
	"mmcpart=1\0" \
	"mmcroot=/dev/mmcblk0p2 rootwait rw\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " \
		"root=${mmcroot}\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"if run loadfdt; then " \
			"bootz ${loadaddr} - ${fdt_addr}; " \
		"fi;\0" \

#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	SZ_256K

#define CFG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#endif	/* __CONFIG_H */
