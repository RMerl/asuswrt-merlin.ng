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

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(35 * SZ_1M)

/* Total Size of Environment Sector */
#define CONFIG_ENV_SIZE			SZ_128K

/* Allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* Environment */
#ifndef CONFIG_ENV_IS_NOWHERE
/* Environment in MMC */
# if defined(CONFIG_ENV_IS_IN_MMC)
#  define CONFIG_ENV_OFFSET		0x100000
# endif
#endif

#ifndef CONFIG_SPL_BUILD
#define CONFIG_EXTRA_ENV_SETTINGS	\
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
#endif

/* UART */
#define CONFIG_MXC_UART_BASE		UART3_BASE

/* MMC */
#define CONFIG_SYS_MMC_ENV_DEV		2

/* Ethernet */
#define CONFIG_FEC_MXC_PHYADDR		1

/* USB */
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_MXC_USB_PORTSC			(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS			0
#define CONFIG_USB_MAX_CONTROLLER_COUNT		2

/* Falcon */
#define CONFIG_SPL_FS_LOAD_ARGS_NAME	"args"
#define CONFIG_SPL_FS_LOAD_KERNEL_NAME	"uImage"
#define CONFIG_CMD_SPL
#define CONFIG_SYS_SPL_ARGS_ADDR	0x13000000
#define CONFIG_CMD_SPL_WRITE_SIZE	(128 * SZ_1K)

/* MMC support: args@1MB kernel@2MB */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR		0x800   /* 1MB */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS		(CONFIG_CMD_SPL_WRITE_SIZE / 512)
#define CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR		0x1000  /* 2MB */

/* Miscellaneous configurable options */
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 0x8000000)

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_SYS_HZ			1000

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - \
					GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
					CONFIG_SYS_INIT_SP_OFFSET)

/* SPL */
#include "imx6_spl.h"

#endif /* __IMX6DL_MAMOJ_CONFIG_H */
