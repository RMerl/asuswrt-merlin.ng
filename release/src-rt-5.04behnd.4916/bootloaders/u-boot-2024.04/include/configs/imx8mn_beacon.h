/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Compass Electronics Group, LLC
 */

#ifndef __IMX8MN_BEACON_H
#define __IMX8MN_BEACON_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

#define CFG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

/* Link Definitions */

#define CFG_SYS_INIT_RAM_ADDR        0x40000000
#define CFG_SYS_INIT_RAM_SIZE        0x200000

#define CFG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM			0x40000000
#if IS_ENABLED(CONFIG_IMX8MN_BEACON_2GB_LPDDR)
#define PHYS_SDRAM_SIZE		0x80000000 /* 2GB DDR */
#else
#define PHYS_SDRAM_SIZE		0x40000000 /* 1GB DDR */
#endif

#endif
