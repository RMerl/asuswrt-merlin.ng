/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Simone CIANNI <simone.cianni@bticino.it>
 * Copyright (C) 2018 Raffaele RECALCATI <raffaele.recalcati@bticino.it>
 * Copyright (C) 2018 Jagan Teki <jagan@amarulasolutions.com>
 *
 * Configuration settings for the BTicion i.MX6DL Mamoj board.
 */

#ifndef __IMX6DL_MAMOJ_CONFIG_H
#define __IMX6DL_MAMOJ_CONFIG_H

#include <linux/sizes.h>
#include "mx6_common.h"

/* Total Size of Environment Sector */

/* Environment */
#ifndef CONFIG_ENV_IS_NOWHERE
/* Environment in MMC */
#endif

#define CFG_EXTRA_ENV_SETTINGS	\
	"scriptaddr=0x14000000\0"	\
	"fdt_addr_r=0x13000000\0"	\
	"kernel_addr_r=0x10008000\0"	\
	"fdt_high=0xffffffff\0"		\
	"dfu_alt_info_spl=spl raw 0x2 0x400\0" \
	"dfu_alt_info_uboot=u-boot raw 0x8a 0x11400\0" \
	BOOTENV

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 2)

#include <config_distro_bootcmd.h>

/* UART */
#define CFG_MXC_UART_BASE		UART3_BASE

/* MMC */

/* Ethernet */
#define CFG_FEC_MXC_PHYADDR		1

/* USB */
#define CFG_MXC_USB_PORTSC			(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS			0

/* Falcon */

/* MMC support: args@1MB kernel@2MB */

/* Miscellaneous configurable options */

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#endif /* __IMX6DL_MAMOJ_CONFIG_H */
