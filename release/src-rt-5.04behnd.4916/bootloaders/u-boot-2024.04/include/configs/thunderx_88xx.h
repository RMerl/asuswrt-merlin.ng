/* SPDX-License-Identifier: GPL-2.0+ */
/**
 * (C) Copyright 2014, Cavium Inc.
**/

#ifndef __THUNDERX_88XX_H__
#define __THUNDERX_88XX_H__

#define MEM_BASE			0x00500000

#define CFG_SYS_LOWMEM_BASE		MEM_BASE

/* Link Definitions */

/* SMP Spin Table Definitions */
#define CPU_RELEASE_ADDR		(CFG_SYS_SDRAM_BASE + 0x7fff0)

/* PL011 Serial Configuration */

#define CFG_PL011_CLOCK		24000000

/* Generic Interrupt Controller Definitions */
#define GICD_BASE			(0x801000000000)
#define GICR_BASE			(0x801000002000)
#define CFG_SYS_SERIAL0		0x87e024000000
#define CFG_SYS_SERIAL1		0x87e025000000

/* Miscellaneous configurable options */

/* Physical Memory Map */
#define PHYS_SDRAM_1			(MEM_BASE)	  /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE		(0x80000000-MEM_BASE)	/* 2048 MB */
#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM_1

/* Initial environment variables */
#define UBOOT_IMG_HEAD_SIZE		0x40
/* C80000 - 0x40 */
#define CFG_EXTRA_ENV_SETTINGS	\
					"kernel_addr=08007ffc0\0"	\
					"fdt_addr=0x94C00000\0"		\
					"fdt_high=0x9fffffff\0"

/* Do not preserve environment */

#define PLL_REF_CLK			50000000	/* 50 MHz */
#define NS_PER_REF_CLK_TICK		(1000000000/PLL_REF_CLK)

#endif /* __THUNDERX_88XX_H__ */
