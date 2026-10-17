/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2020 Engicam srl
 * Copyright (c) 2020 Amarula Solutions(India)
 */

#ifndef __IMX8MM_ICORE_MX8MM_H
#define __IMX8MM_ICORE_MX8MM_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

#define CFG_SYS_UBOOT_BASE \
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#ifdef CONFIG_SPL_BUILD
/* malloc f used before GD_FLG_FULL_MALLOC_INIT set */
# define CFG_MALLOC_F_ADDR		0x930000
/* For RAW image gives a error info not panic */
#endif /* CONFIG_SPL_BUILD */

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 2) \
	func(MMC, mmc, 0)
#include <config_distro_bootcmd.h>

#define ENV_MEM_LAYOUT_SETTINGS \
	"fdt_addr_r=0x44000000\0" \
	"kernel_addr_r=0x42000000\0" \
	"ramdisk_addr_r=0x46400000\0" \
	"scriptaddr=0x46000000\0"

#define CFG_EXTRA_ENV_SETTINGS \
	ENV_MEM_LAYOUT_SETTINGS \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"console=ttymxc1,115200\0" \
	BOOTENV

/* Link Definitions */

#define CFG_SYS_INIT_RAM_ADDR        0x40000000
#define CFG_SYS_INIT_RAM_SIZE        SZ_2M

#define CFG_SYS_SDRAM_BASE           0x40000000

/* SDRAM configuration */
#define PHYS_SDRAM                      0x40000000
#define PHYS_SDRAM_SIZE			SZ_2G /* 2GB DDR */

/* USDHC */
#define CFG_SYS_FSL_USDHC_NUM	2
#define CFG_SYS_FSL_ESDHC_ADDR	0

#endif /* __IMX8MM_ICORE_MX8MM_H */
