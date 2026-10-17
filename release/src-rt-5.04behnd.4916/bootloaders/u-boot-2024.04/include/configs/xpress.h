/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015-2016 Stefan Roese <sr@denx.de>
 *
 * Configuration settings for the CCV xPress board
 */
#ifndef __XPRESS_CONFIG_H
#define __XPRESS_CONFIG_H

#include "mx6_common.h"
#include <asm/mach-imx/gpio.h>

#define CFG_MXC_UART_BASE		MX6UL_UART7_BASE_ADDR

/* MMC Configs */
#define CFG_SYS_FSL_ESDHC_ADDR	USDHC2_BASE_ADDR

/* Miscellaneous configurable options */

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR
#define PHYS_SDRAM_SIZE			(128 << 20)

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* Environment is in stored in the eMMC boot partition */

/* USB Configs */
#define CFG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS		0

#define CFG_FEC_ENET_DEV		0
#define CFG_FEC_MXC_PHYADDR          0x0

#define CFG_EXTRA_ENV_SETTINGS \
	"script=boot.scr\0" \
	"image=zImage\0" \
	"console=ttymxc6\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdt_file=undefined\0" \
	"fdt_addr=0x83000000\0" \
	"boot_fdt=try\0" \
	"ip_dyn=yes\0" \
	"mmcdev="__stringify(CONFIG_SYS_MMC_ENV_DEV)"\0" \
	"mmcpart=1\0" \
	"mmcroot=/dev/mmcblk0p2 rootwait rw\0" \
	"mmcautodetect=yes\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " \
		"root=${mmcroot}\0" \
	"loadbootscript=" \
		"fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if run loadfdt; then " \
				"bootz ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"bootz; " \
				"else " \
					"echo WARN: Cannot load the DT; " \
				"fi; " \
			"fi; " \
		"else " \
			"bootz; " \
		"fi;\0" \
	"uboot=ccv/u-boot.imx\0"					\
	"uboot_start=0x2\0"						\
	"uboot_size=0x3fe\0"						\
	"update_uboot=if tftp ${uboot}; then "				\
		"if itest ${filesize} > 0; then "			\
			"mmc dev 0 1;"					\
			"setexpr blkc ${filesize} / 0x200;"		\
			"setexpr blkc ${blkc} + 1;"			\
			"if itest ${blkc} <= ${uboot_size}; then "	\
				"mmc write ${loadaddr} ${uboot_start} "	\
					"${blkc};"			\
			"fi;"						\
		"fi; fi;"						\
		"setenv filesize; setenv blkc\0"			\
	"update_bootpart=mmc bootbus 0 2 1 2;mmc partconf 0 1 1 0\0"

#endif /* __XPRESS_CONFIG_H */
