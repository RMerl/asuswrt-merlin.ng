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

/* SPL options */
#include "imx6_spl.h"

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(16 << 20)

#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE		MX6UL_UART7_BASE_ADDR

/* MMC Configs */
#define CONFIG_SYS_FSL_ESDHC_ADDR	USDHC2_BASE_ADDR

/* I2C configs */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_MXC_I2C4		/* enable I2C bus 4 */
#define CONFIG_SYS_I2C_SPEED		100000

/* Miscellaneous configurable options */
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 0x10000000)

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_SYS_HZ			1000

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR
#define PHYS_SDRAM_SIZE			(128 << 20)

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* Environment is in stored in the eMMC boot partition */
#define CONFIG_ENV_SIZE			(16 << 10)
#define CONFIG_ENV_OFFSET		(512 << 10)
#define CONFIG_SYS_MMC_ENV_DEV		0	/* USDHC2 */
#define CONFIG_SYS_MMC_ENV_PART		1	/* boot parition */
#define CONFIG_MMCROOT			"/dev/mmcblk0p2"  /* USDHC2 */

/* USB Configs */
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS		0
#define CONFIG_USB_MAX_CONTROLLER_COUNT	2

#define CONFIG_FEC_MXC
#define CONFIG_FEC_ENET_DEV		0
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_MXC_PHYADDR          0x0
#define CONFIG_FEC_XCV_TYPE             RMII
#define CONFIG_ETHPRIME			"FEC"
#define CONFIG_PHY_SMSC

#define CONFIG_IMX_THERMAL

#define CONFIG_SYS_MMC_IMG_LOAD_PART	1

#define CONFIG_UBOOT_SECTOR_START	0x2
#define CONFIG_UBOOT_SECTOR_COUNT	0x3fe

#define CONFIG_EXTRA_ENV_SETTINGS \
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
	"mmcpart=" __stringify(CONFIG_SYS_MMC_IMG_LOAD_PART) "\0" \
	"mmcroot=" CONFIG_MMCROOT " rootwait rw\0" \
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
	"uboot_start="__stringify(CONFIG_UBOOT_SECTOR_START)"\0"	\
	"uboot_size="__stringify(CONFIG_UBOOT_SECTOR_COUNT)"\0"		\
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
