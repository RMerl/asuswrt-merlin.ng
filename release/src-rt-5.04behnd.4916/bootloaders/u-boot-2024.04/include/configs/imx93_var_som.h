/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022 NXP
 * Copyright 2023 Variscite Ltd.
 */

#ifndef __IMX93_VAR_SOM_H
#define __IMX93_VAR_SOM_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include <asm/arch/imx-regs.h>

#define CFG_SYS_SDRAM_BASE           0x80000000
#define CFG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1)

#include <config_distro_bootcmd.h>

/* Initial environment variables */
#define CFG_EXTRA_ENV_SETTINGS BOOTENV

#define CFG_SYS_INIT_RAM_ADDR        0x80000000
#define CFG_SYS_INIT_RAM_SIZE        0x200000

#define CFG_SYS_SDRAM_BASE           0x80000000
#define PHYS_SDRAM                      0x80000000
#define PHYS_SDRAM_SIZE			0x80000000 /* 2GB DDR */

#define DEFAULT_SDRAM_SIZE		(512 * SZ_1M) /* 512MB Minimum DDR4, see get_dram_size */
#define VAR_EEPROM_DRAM_START           (PHYS_SDRAM + (DEFAULT_SDRAM_SIZE >> 1))
#define VAR_SOM_EEPROM_I2C_NAME "i2c@42530000"
#define VAR_CARRIER_EEPROM_I2C_NAME "i2c@44340000"

#define CFG_SYS_FSL_USDHC_NUM 2

/* Using ULP WDOG for reset */
#define WDOG_BASE_ADDR          WDG3_BASE_ADDR

#if defined(CONFIG_CMD_NET)
#define PHY_ANEG_TIMEOUT 20000
#endif

#endif
