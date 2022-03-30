/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015
 * (C) Copyright 2014
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Freescale i.MX6Q SabreSD board.
 */
#ifndef __ARISTAINETOS_COMMON_CONFIG_H
#define __ARISTAINETOS_COMMON_CONFIG_H

#include "mx6_common.h"

#define CONFIG_MACH_TYPE	4501
#define CONFIG_MMCROOT		"/dev/mmcblk0p1"

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(64 * SZ_1M)

#define CONFIG_MXC_UART

/* MMC Configs */
#define CONFIG_SYS_FSL_ESDHC_ADDR      0

#define CONFIG_FEC_MXC
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_ETHPRIME			"FEC"
#define CONFIG_FEC_MXC_PHYADDR		0

#define CONFIG_SPI_FLASH_MTD
#define CONFIG_SYS_SPI_ST_ENABLE_WP_PIN

#define CONFIG_EXTRA_ENV_SETTINGS \
	"script=u-boot.scr\0" \
	"fit_file=/boot/system.itb\0" \
	"loadaddr=0x12000000\0" \
	"fit_addr_r=0x14000000\0" \
	"uboot=/boot/u-boot.imx\0" \
	"uboot_sz=d0000\0" \
	"rescue_sys_addr=f0000\0" \
	"rescue_sys_length=f10000\0" \
	"panel=lb07wv8\0" \
	"splashpos=m,m\0" \
	"console=" CONSOLE_DEV "\0" \
	"fdt_high=0xffffffff\0"	  \
	"initrd_high=0xffffffff\0" \
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0" \
	"set_fit_default=fdt addr ${fit_addr_r};fdt set /configurations " \
		"default ${board_type}\0" \
	"get_env=mw ${loadaddr} 0 0x20000;" \
		"mmc rescan;" \
		"ext2load mmc ${mmcdev}:${mmcpart} ${loadaddr} env.txt;" \
		"env import -t ${loadaddr}\0" \
	"default_env=mw ${loadaddr} 0 0x20000;" \
		"env export -t ${loadaddr} serial# ethaddr eth1addr " \
		"board_type panel;" \
		"env default -a;" \
		"env import -t ${loadaddr}\0" \
	"loadbootscript=" \
		"ext2load mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"mmcpart=1\0" \
	"mmcdev=0\0" \
	"mmcroot=" CONFIG_MMCROOT " rootwait rw\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " \
		"root=${mmcroot}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs addmtd addmisc set_fit_default;" \
		"bootm ${fit_addr_r}\0" \
	"mmc_load_fit=ext2load mmc ${mmcdev}:${mmcpart} ${fit_addr_r} " \
		"${fit_file}\0" \
	"mmc_load_uboot=ext2load mmc ${mmcdev}:${mmcpart} ${loadaddr} " \
		"${uboot}\0" \
	"mmc_upd_uboot=mw.b ${loadaddr} 0xff ${uboot_sz};" \
		"setexpr cmp_buf ${loadaddr} + ${uboot_sz};" \
		"setexpr uboot_maxsize ${uboot_sz} - 400;" \
		"mw.b ${cmp_buf} 0x00 ${uboot_sz};" \
		"run mmc_load_uboot;sf probe;sf erase 0 ${uboot_sz};" \
		"sf write ${loadaddr} 400 ${filesize};" \
		"sf read ${cmp_buf} 400 ${uboot_sz};" \
		"cmp.b ${loadaddr} ${cmp_buf} ${uboot_maxsize}\0" \
	"ubiboot=echo Booting from ubi ...; " \
		"run ubiargs addmtd addmisc set_fit_default;" \
		"bootm ${fit_addr_r}\0" \
	"rescueargs=setenv bootargs console=${console},${baudrate} " \
		"root=/dev/ram rw\0 " \
	"rescueboot=echo Booting rescue system from NOR ...; " \
		"run rescueargs addmtd addmisc set_fit_default;" \
		"bootm ${fit_addr_r}\0" \
	"rescue_load_fit=sf probe;sf read ${fit_addr_r} ${rescue_sys_addr} " \
		"${rescue_sys_length}; imi ${fit_addr_r}\0" \
	CONFIG_EXTRA_ENV_BOARD_SETTINGS

#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev};" \
	"if mmc rescan; then " \
		"if run loadbootscript; then " \
			"run bootscript; " \
		"else " \
			"if run mmc_load_fit; then " \
				"run mmcboot; " \
			"else " \
				"if run ubifs_load_fit; then " \
					"run ubiboot; " \
				"else " \
					"if run rescue_load_fit; then " \
						"run rescueboot; " \
					"else " \
						"echo RESCUE SYSTEM BOOT " \
							"FAILURE;" \
					"fi; " \
				"fi; " \
			"fi; " \
		"fi; " \
	"else " \
		"if run ubifs_load_fit; then " \
			"run ubiboot; " \
		"else " \
			"if run rescue_load_fit; then " \
				"run rescueboot; " \
			"else " \
				"echo RESCUE SYSTEM BOOT FAILURE;" \
			"fi; " \
		"fi; " \
	"fi"

#define CONFIG_ARP_TIMEOUT		200UL

#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 0x100000)
#define CONFIG_SYS_MEMTEST_SCRATCH	0x10800000

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* Environment organization */
#define CONFIG_ENV_SIZE			(12 * 1024)
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_SECT_SIZE		(0x010000)
#define CONFIG_ENV_OFFSET		(0x0d0000)
#define CONFIG_ENV_OFFSET_REDUND	(0x0e0000)

#define CONFIG_SYS_FSL_USDHC_NUM	2

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_MXC_I2C3		/* enable I2C bus 3 */
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_I2C_SLAVE		0x7f
#define CONFIG_SYS_I2C_NOPROBES		{ {0, 0x00} }

/* NAND stuff */
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_ONFI_DETECTION

/* DMA stuff, needed for GPMI/MXS NAND support */

/* RTC */
#define CONFIG_SYS_I2C_RTC_ADDR	0x68
#define CONFIG_SYS_RTC_BUS_NUM	2
#define CONFIG_RTC_M41T11

/* USB Configs */
#define CONFIG_USB_MAX_CONTROLLER_COUNT 2
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET	/* For OTG port */
#define CONFIG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS	0

/* UBI support */

/* Framebuffer */
/* check this console not needed, after test remove it */
#define CONFIG_VIDEO_BMP_RLE8
#define CONFIG_SPLASH_SCREEN
#define CONFIG_SPLASH_SCREEN_ALIGN
#define CONFIG_BMP_16BPP
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_LOGO
#define CONFIG_IMX_VIDEO_SKIP

#define CONFIG_PWM_IMX
#define CONFIG_IMX6_PWM_PER_CLK	66000000

#endif /* __ARISTAINETOS_COMMON_CONFIG_H */
