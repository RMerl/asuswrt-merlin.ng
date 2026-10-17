/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017-2019 A. Karas, SomLabs
 * Copyright (C) 2015-2016 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the SoMlabs VisionSOM 6ULL board.
 */
#ifndef __SOMLABS_VISIONSOM_6ULL_H
#define __SOMLABS_VISIONSOM_6ULL_H

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>
#include "mx6_common.h"
#include <asm/mach-imx/gpio.h>

/* MMC Configs */
#ifdef CONFIG_FSL_USDHC
#define CFG_SYS_FSL_ESDHC_ADDR	USDHC2_BASE_ADDR

#define CFG_SYS_FSL_USDHC_NUM	1
#endif /* CONFIG_FSL_USDHC */

#define CFG_EXTRA_ENV_SETTINGS \
	"bootm_size=0x10000000\0" \
	"console=ttymxc0\0" \
	"initrd_addr=0x86800000\0" \
	"fdt_addr=0x83000000\0" \
	"script=boot.scr\0" \
	"image=zImage\0" \
	"splashimage=0x80000000\0" \
	"splashfile=/boot/splash.bmp\0" \
	"mmcdev=1\0" \
	"mmcpart=1\0" \
	"mmcroot=/dev/mmcblk1p1 rootwait rw\0" \
	"setrootmmc=setenv rootspec root=${mmcroot}\0" \
	"setbootscriptmmc=setenv loadbootscript " \
		"load mmc ${mmcdev}:${mmcpart} " \
		"${loadaddr} /boot/${script};\0" \
	"setloadmmc=setenv loadimage load mmc ${mmcdev}:${mmcpart} " \
		"${loadaddr} /boot/${image}; " \
		"setenv loadfdt load mmc ${mmcdev}:${mmcpart} " \
		"${fdt_addr} /boot/${fdt_file};\0" \
	"setbootargs=setenv bootargs console=${console},${baudrate} " \
		"${rootspec}\0" \
	"execbootscript=echo Running bootscript...; source\0" \
	"setfdtfile=setenv fdt_file somlabs-visionsom-6ull.dtb\0" \
	"checkbootdev=run setbootscriptmmc; " \
		"run setrootmmc; " \
		"run setloadmmc; " \

/* Miscellaneous configurable options */

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* environment organization */

/* USB Configs */
#ifdef CONFIG_CMD_USB
#define CFG_MXC_USB_PORTSC  (PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS   0
#endif

#ifdef CONFIG_CMD_NET
#define CFG_FEC_MXC_PHYADDR		0x1
#endif

#endif
