/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 NXP
 */

#ifndef __IMX8ULP_EVK_H
#define __IMX8ULP_EVK_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

#define CFG_SYS_UBOOT_BASE	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#ifdef CONFIG_SPL_BUILD
#define CFG_MALLOC_F_ADDR		0x22040000


#endif

/* ENET Config */
#if defined(CONFIG_FEC_MXC)
#define PHY_ANEG_TIMEOUT		20000

#define CFG_FEC_MXC_PHYADDR		1
#endif

#ifdef CONFIG_DISTRO_DEFAULTS
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0)

#include <config_distro_bootcmd.h>
#else
#define BOOTENV
#endif

/* Initial environment variables */
#define CFG_EXTRA_ENV_SETTINGS		\
	BOOTENV \
	"scriptaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"image=Image\0" \
	"console=ttyLP1,115200 earlycon\0" \
	"fdt_addr_r=0x83000000\0"			\
	"boot_fit=no\0" \
	"fdtfile=imx8ulp-evk.dtb\0" \
	"initrd_addr=0x83800000\0"		\
	"bootm_size=0x10000000\0" \
	"mmcpart=1\0" \
	"mmcroot=/dev/mmcblk2p2 rootwait rw\0" \

/* Link Definitions */

#define CFG_SYS_INIT_RAM_ADDR	0x80000000
#define CFG_SYS_INIT_RAM_SIZE	0x80000


#define CFG_SYS_SDRAM_BASE		0x80000000
#define PHYS_SDRAM			0x80000000
#define PHYS_SDRAM_SIZE			0x80000000 /* 2GB DDR */

/* Using ULP WDOG for reset */
#define WDOG_BASE_ADDR			WDG3_RBASE
#endif
