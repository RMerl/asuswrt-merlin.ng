/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  (C) Copyright 2010-2012
 *  NVIDIA Corporation <www.nvidia.com>
 */

#ifndef _TEGRA_COMMON_H_
#define _TEGRA_COMMON_H_
#include <linux/sizes.h>
#include <linux/stringify.h>

/*
 * High Level Configuration Options
 */

#include <asm/arch/tegra.h>		/* get chip and board defs */

/* Environment */

/*
 * NS16550 Configuration
 */
#define CFG_SYS_NS16550_CLK		V_NS16550_CLK

#ifdef CONFIG_ARM64
#define FDTFILE "nvidia/" CONFIG_DEFAULT_DEVICE_TREE ".dtb"
#else
#define FDTFILE CONFIG_DEFAULT_DEVICE_TREE ".dtb"
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define PHYS_SDRAM_1		NV_PA_SDRC_CS0
#define PHYS_SDRAM_1_SIZE	0x20000000	/* 512M */

#define CFG_SYS_SDRAM_BASE	PHYS_SDRAM_1

#define CFG_SYS_BOOTMAPSZ	(256 << 20)	/* 256M */

#ifndef CONFIG_ARM64
#define CFG_SYS_INIT_RAM_ADDR	CFG_STACKBASE
#define CFG_SYS_INIT_RAM_SIZE	CONFIG_SYS_MALLOC_LEN

/* Defines for SPL */
#endif

#endif /* _TEGRA_COMMON_H_ */
