/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2006-2023  CS GROUP France
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

/*
 * System IO Config
 */
#define CFG_SYS_SICRL		0x00000000

#define CFG_SYS_DDR_SDRAM_BASE	0x00000000

#define CFG_SYS_DDRCDR		0x73000002	/* DDR II voltage is 1.8V */

/*
 * Manually set up DDR parameters
 */

/* DDR 512 M */
#define CFG_SYS_DDR_CS0_CONFIG	(CSCONFIG_EN | CSCONFIG_ODT_WR_CFG | CSCONFIG_BANK_BIT_3 | \
				 CSCONFIG_ROW_BIT_14 | CSCONFIG_COL_BIT_10)
/* 0x80840102 */
#define CFG_SYS_DDR_TIMING_0	((0 << TIMING_CFG0_RWT_SHIFT) | \
				 (0 << TIMING_CFG0_WRT_SHIFT) | \
				 (0 << TIMING_CFG0_RRT_SHIFT) | \
				 (0 << TIMING_CFG0_WWT_SHIFT) | \
				 (2 << TIMING_CFG0_ACT_PD_EXIT_SHIFT) | \
				 (2 << TIMING_CFG0_PRE_PD_EXIT_SHIFT) | \
				 (8 << TIMING_CFG0_ODT_PD_EXIT_SHIFT) | \
				 (2 << TIMING_CFG0_MRS_CYC_SHIFT))
/* 0x00220802 */
#define CFG_SYS_DDR_TIMING_1	((2 << TIMING_CFG1_PRETOACT_SHIFT) | \
				 (6 << TIMING_CFG1_ACTTOPRE_SHIFT) | \
				 (2 << TIMING_CFG1_ACTTORW_SHIFT) | \
				 (5 << TIMING_CFG1_CASLAT_SHIFT) | \
				 (27 << TIMING_CFG1_REFREC_SHIFT) | \
				 (2 << TIMING_CFG1_WRREC_SHIFT) | \
				 (2 << TIMING_CFG1_ACTTOACT_SHIFT) | \
				 (2 << TIMING_CFG1_WRTORD_SHIFT))
/* 0x3935D322 */
#define CFG_SYS_DDR_TIMING_2	((0 << TIMING_CFG2_ADD_LAT_SHIFT) | \
				 (31 << TIMING_CFG2_CPO_SHIFT) | \
				 (2 << TIMING_CFG2_WR_LAT_DELAY_SHIFT) | \
				 (2 << TIMING_CFG2_RD_TO_PRE_SHIFT) | \
				 (2 << TIMING_CFG2_WR_DATA_DELAY_SHIFT) | \
				 (3 << TIMING_CFG2_CKE_PLS_SHIFT) | \
				 (7 << TIMING_CFG2_FOUR_ACT_SHIFT))
/* 0x0F9048CA */
#define CFG_SYS_DDR_TIMING_3	0x00000000
#define CFG_SYS_DDR_CLK_CNTL	DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05
/* 0x02000000 */
#define CFG_SYS_DDR_MODE	((0x4440 << SDRAM_MODE_ESD_SHIFT) | (0x0232 << SDRAM_MODE_SD_SHIFT))
/* 0x44400232 */
#define CFG_SYS_DDR_MODE2	0x8000c000
#define CFG_SYS_DDR_INTERVAL	((800 << SDRAM_INTERVAL_REFINT_SHIFT) | \
				 (100 << SDRAM_INTERVAL_BSTOPRE_SHIFT))
#define CFG_SYS_DDR_CS0_BNDS	(CFG_SYS_DDR_SDRAM_BASE >> 8 | 0x0000001F)

#define CFG_SYS_DDR_SDRAM_CFG	(SDRAM_CFG_SREN | SDRAM_CFG_SDRAM_TYPE_DDR2 | SDRAM_CFG_32_BE)
/* 0x43080000 */
#define CFG_SYS_DDR_SDRAM_CFG2	0x00401000

/*
 * Initial RAM Base Address Setup
 */
#define CFG_SYS_INIT_RAM_ADDR	(CONFIG_SYS_IMMR + 0x110000)
#define CFG_SYS_INIT_RAM_SIZE	0x4000

/*
 * FLASH on the Local Bus
 */
#define CFG_SYS_FLASH_BASE	0x40000000	/* FLASH base address */
#define CFG_SYS_FLASH_SIZE	64		/* FLASH size is 64M */

/*
 * NAND
 */
#define CFG_SYS_NAND_BASE	0xa0000000

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 256 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
					/* Initial Memory map for Linux */
#define CFG_SYS_BOOTMAPSZ	SZ_256M

/* Board names */
#define CFG_BOARD_CMPCXXX	"cmpcpro"
#define CFG_BOARD_MCR3000_2G	"mcrpro"
#define CFG_BOARD_VGOIP		"vgoippro"
#define CFG_BOARD_MIAE		"miaepro"

#endif	/* __CONFIG_H */
