/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014-2015 Freescale Semiconductor, Inc.
 * Copyright Jasbir Matharu
 * Copyright 2015 UDOO Team
 *
 * Configuration settings for the UDOO NEO board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "mx6_common.h"

#include "imx6_spl.h"

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(3 * SZ_1M)
#define CONFIG_MXC_UART

/* MMC Configuration */
#define CONFIG_SYS_FSL_ESDHC_ADDR	USDHC2_BASE_ADDR

/* Command definition */
#define CONFIG_MXC_UART_BASE		UART1_BASE
#define CONFIG_SYS_MMC_ENV_DEV		0  /*USDHC2*/

/* Linux only */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"console=ttymxc0,115200\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdtfile=undefined\0" \
	"fdt_addr=0x83000000\0" \
	"fdt_addr_r=0x83000000\0" \
	"ip_dyn=yes\0" \
	"mmcdev=0\0" \
	"mmcrootfstype=ext4\0" \
	"findfdt="\
		"if test $board_name = BASIC; then " \
			"setenv fdtfile imx6sx-udoo-neo-basic.dtb; fi; " \
		"if test $board_name = BASICKS; then " \
			"setenv fdtfile imx6sx-udoo-neo-basic.dtb; fi; " \
		"if test $board_name = FULL; then " \
			"setenv fdtfile imx6sx-udoo-neo-full.dtb; fi; " \
		"if test $board_name = EXTENDED; then " \
			"setenv fdtfile imx6sx-udoo-neo-extended.dtb; fi; " \
		"if test $fdtfile = UNDEFINED; then " \
			"echo WARNING: Could not determine dtb to use; fi\0" \
	"kernel_addr_r=" __stringify(CONFIG_LOADADDR) "\0" \
	"pxefile_addr_r=" __stringify(CONFIG_LOADADDR) "\0" \
	"ramdisk_addr_r=0x84000000\0" \
	"scriptaddr=" __stringify(CONFIG_LOADADDR) "\0" \
	BOOTENV

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

/* Miscellaneous configurable options */
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 0x10000)

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
#define CONFIG_ENV_OFFSET		(8 * SZ_64K)
#define CONFIG_ENV_SIZE			SZ_8K

#define CONFIG_IMX_THERMAL

/* I2C configs */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1
#define CONFIG_SYS_I2C_SPEED		100000

/* PMIC */
#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_PFUZE3000
#define CONFIG_POWER_PFUZE3000_I2C_ADDR	0x08
#define PFUZE3000_I2C_BUS	0

/* Network */
#define CONFIG_FEC_MXC

#define CONFIG_FEC_ENET_DEV 0
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_MXC_PHYADDR          0x0

#define CONFIG_FEC_XCV_TYPE             RMII
#define CONFIG_ETHPRIME                 "FEC0"

#endif				/* __CONFIG_H */
