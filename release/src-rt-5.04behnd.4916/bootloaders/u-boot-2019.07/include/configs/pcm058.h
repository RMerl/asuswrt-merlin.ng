/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) Stefano Babic <sbabic@denx.de>
 */


#ifndef __PCM058_CONFIG_H
#define __PCM058_CONFIG_H

#ifdef CONFIG_SPL
#define CONFIG_SYS_SPI_U_BOOT_OFFS	(64 * 1024)
#include "imx6_spl.h"
#endif

#include "mx6_common.h"

/* Thermal */
#define CONFIG_IMX_THERMAL

/* Serial */
#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE	       UART2_BASE
#define CONSOLE_DEV		"ttymxc1"

#define PHYS_SDRAM_SIZE		(1u * 1024 * 1024 * 1024)

/* Early setup */


/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(8 * SZ_1M)

/* Ethernet */
#define CONFIG_FEC_MXC
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_ETHPRIME			"FEC"
#define CONFIG_FEC_MXC_PHYADDR		3

/* SPI Flash */

/* I2C Configs */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C3		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_SPEED		  100000

#ifndef CONFIG_SPL_BUILD
/* Enable NAND support */
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_SYS_NAND_USE_FLASH_BBT
#endif

/* DMA stuff, needed for GPMI/MXS NAND support */

/* Filesystem support */

/* Physical Memory Map */
#define PHYS_SDRAM                     MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE          PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE       IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* MMC Configs */
#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define CONFIG_SYS_FSL_USDHC_NUM	1

/* Environment organization */
#define CONFIG_ENV_SIZE                (16 * 1024)
#define CONFIG_ENV_OFFSET		(1024 * SZ_1K)
#define CONFIG_ENV_SECT_SIZE		(64 * SZ_1K)
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_OFFSET_REDUND       (CONFIG_ENV_OFFSET + \
						CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND         CONFIG_ENV_SIZE

#ifdef CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET              (0x1E0000)
#define CONFIG_ENV_SECT_SIZE           (128 * SZ_1K)
#endif

#endif
