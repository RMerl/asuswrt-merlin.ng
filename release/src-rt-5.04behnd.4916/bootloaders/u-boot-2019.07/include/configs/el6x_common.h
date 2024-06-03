/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) Stefano Babic <sbabic@denx.de>
 *
 * Configuration settings for the E+L i.MX6Q DO82 board.
 */

#ifndef __EL6Q_COMMON_CONFIG_H
#define __EL6Q_COMMON_CONFIG_H

#define CONFIG_BOARD_NAME		EL6Q

#include "mx6_common.h"

#define CONFIG_IMX_THERMAL

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(10 * SZ_1M)

#define CONFIG_MXC_UART

#ifdef CONFIG_SPL
#define CONFIG_SYS_SPI_U_BOOT_OFFS	(64 * 1024)
#include "imx6_spl.h"
#endif

/* MMC Configs */
#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define CONFIG_SYS_FSL_USDHC_NUM	2

/* I2C config */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_MXC_I2C3		/* enable I2C bus 3 */
#define CONFIG_SYS_I2C_SPEED			100000

/* PMIC */
#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_PFUZE100
#define CONFIG_POWER_PFUZE100_I2C_ADDR	0x08

/* Commands */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_MXC_UART_BASE	UART2_BASE

#define CONFIG_BOARD_NAME	EL6Q

#define CONFIG_EXTRA_ENV_SETTINGS                                               \
	"board="__stringify(CONFIG_BOARD_NAME)"\0"                              \
	"cma_size="__stringify(EL6Q_CMA_SIZE)"\0"                               \
	"chp_size="__stringify(EL6Q_COHERENT_POOL_SIZE)"\0"                     \
	"console=" CONSOLE_DEV "\0"					\
	"fdtfile=undefined\0" \
	"fdt_high=0xffffffff\0" \
	"fdt_addr_r=0x18000000\0" \
	"fdt_addr=0x18000000\0" \
	"findfdt=setenv fdtfile " CONFIG_DEFAULT_FDT_FILE "\0"                  \
	"kernel_addr_r=" __stringify(CONFIG_LOADADDR) "\0" \
	"scriptaddr=" __stringify(CONFIG_LOADADDR) "\0" \
	"pxefile_addr_r=" __stringify(CONFIG_LOADADDR) "\0" \
	BOOTENV

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1) \
	func(PXE, PXE, na) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

#define CONFIG_ARP_TIMEOUT     200UL

#define CONFIG_SYS_MEMTEST_START       0x10000000
#define CONFIG_SYS_MEMTEST_END         0x10800000
#define CONFIG_SYS_MEMTEST_SCRATCH     0x10800000

/* Physical Memory Map */
#define PHYS_SDRAM                     MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE          PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE       IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* environment organization */

#define CONFIG_ENV_SIZE			(8 * 1024)

#if defined(CONFIG_ENV_IS_IN_MMC)
#define CONFIG_SYS_MMC_ENV_DEV		1
#define CONFIG_SYS_MMC_ENV_PART		2
#define CONFIG_ENV_OFFSET		0x0
#endif

#endif                         /* __EL6Q_COMMON_CONFIG_H */
