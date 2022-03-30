/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Board configuration file for Variscite DART-6UL Evaluation Kit
 * Copyright (C) 2019 Parthiban Nallathambi <parthitce@gmail.com>
 */
#ifndef __DART_6UL_H
#define __DART_6UL_H

#include <linux/sizes.h>
#include "mx6_common.h"

/* SPL options */
#include "imx6_spl.h"

/* NAND pin conflicts with usdhc2 */
#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_FSL_USDHC_NUM        1
#else
#define CONFIG_SYS_FSL_USDHC_NUM        2
#endif

#ifdef CONFIG_CMD_NET
#define CONFIG_FEC_ENET_DEV		0

#if (CONFIG_FEC_ENET_DEV == 0)
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_MXC_PHYADDR		0x1
#define CONFIG_FEC_XCV_TYPE		RMII
#define CONFIG_ETHPRIME			"eth0"
#elif (CONFIG_FEC_ENET_DEV == 1)
#define IMX_FEC_BASE			ENET2_BASE_ADDR
#define CONFIG_FEC_MXC_PHYADDR		0x3
#define CONFIG_FEC_XCV_TYPE		RMII
#define CONFIG_ETHPRIME			"eth1"
#endif
#endif

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(16 * SZ_1M)

/* Environment settings */
#define CONFIG_ENV_SIZE			SZ_8K
#define CONFIG_ENV_OFFSET		(14 * SZ_64K)
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_OFFSET_REDUND	\
	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)

/* Environment in SD */
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_SYS_MMC_ENV_PART		0
#define MMC_ROOTFS_DEV			0
#define MMC_ROOTFS_PART			2

/* Console configs */
#define CONFIG_MXC_UART_BASE		UART1_BASE

/* MMC Configs */
#define CONFIG_FSL_USDHC

#define CONFIG_SYS_FSL_ESDHC_ADDR	USDHC2_BASE_ADDR
#define CONFIG_SUPPORT_EMMC_BOOT

/* I2C configs */
#ifdef CONFIG_CMD_I2C
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_SPEED		100000
#endif

/* Miscellaneous configurable options */
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 0x8000000)

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_SYS_HZ			1000

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR
#define PHYS_SDRAM_SIZE			SZ_512M

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* USB Configs */
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS		0
#define CONFIG_USB_MAX_CONTROLLER_COUNT	2

#define CONFIG_IMX_THERMAL

#define ENV_MMC \
	"mmcdev=" __stringify(MMC_ROOTFS_DEV) "\0" \
	"mmcpart=" __stringify(MMC_ROOTFS_PART) "\0" \
	"fitpart=1\0" \
	"bootdelay=3\0" \
	"silent=1\0" \
	"optargs=rw rootwait\0" \
	"mmcautodetect=yes\0" \
	"mmcrootfstype=ext4\0" \
	"mmcfit_name=fitImage\0" \
	"mmcloadfit=fatload mmc ${mmcdev}:${fitpart} ${fit_addr} " \
		    "${mmcfit_name}\0" \
	"mmcargs=setenv bootargs " \
		"root=/dev/mmcblk${mmcdev}p${mmcpart} ${optargs} " \
		"console=${console} rootfstype=${mmcrootfstype}\0" \
	"mmc_mmc_fit=run mmcloadfit;run mmcargs addcon; bootm ${fit_addr}\0" \

/* Default environment */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"fdt_high=0xffffffff\0" \
	"console=ttymxc0,115200n8\0" \
	"addcon=setenv bootargs ${bootargs} console=${console},${baudrate}\0" \
	"fit_addr=0x82000000\0" \
	ENV_MMC

#define CONFIG_BOOTCOMMAND		"run mmc_mmc_fit"

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>
#endif /* __DART_6UL_H */
