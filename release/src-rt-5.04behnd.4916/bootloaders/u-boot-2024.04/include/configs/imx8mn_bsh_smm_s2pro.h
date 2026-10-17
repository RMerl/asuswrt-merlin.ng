/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 Collabora Ltd.
 */

#ifndef __IMX8MN_BSH_SMM_S2PRO_H
#define __IMX8MN_BSH_SMM_S2PRO_H

#include <configs/imx8mn_bsh_smm_s2_common.h>

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \

#include <config_distro_bootcmd.h>

#define EMMCARGS \
	"fastboot_partition_alias_all=" \
		__stringify(CONFIG_FASTBOOT_FLASH_MMC_DEV) ".0:0\0" \
	"fastboot_partition_alias_bootloader=" \
		__stringify(CONFIG_FASTBOOT_FLASH_MMC_DEV) ".1:0\0" \
	"emmc_dev=" __stringify(CONFIG_FASTBOOT_FLASH_MMC_DEV) "\0" \
	"emmc_ack=1\0" \

/* Initial environment variables */
#define CFG_EXTRA_ENV_SETTINGS \
	MEM_LAYOUT_ENV_SETTINGS \
	EMMCARGS \
	BOOTENV

#define PHYS_SDRAM_SIZE			SZ_512M

/* USDHC */
#define CFG_SYS_FSL_ESDHC_ADDR	0

#endif /* __IMX8MN_BSH_SMM_S2PRO_H */
