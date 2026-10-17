/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2008
 * Sergei Poselenov, Emcraft Systems, sposelenov@emcraft.com.
 *
 * Wolfgang Denk <wd@denx.de>
 * Copyright 2004 Freescale Semiconductor.
 * (C) Copyright 2002,2003 Motorola,Inc.
 * Xianghua Xiao <X.Xiao@motorola.com>
 */

/*
 * Socrates
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * Only possible on E500 Version 2 or newer cores.
 */

/*
 * sysclk for MPC85xx
 *
 * Two valid values are:
 *    33000000
 *    66000000
 *
 * Most PCI cards are still 33Mhz, so in the presence of PCI, 33MHz
 * is likely the desired value here, so that is now the default.
 * The board, however, can run at 66MHz.  In any event, this value
 * must match the settings of some switches.  Details can be found
 * in the README.mpc85xxads.
 */

#define CFG_SYS_INIT_DBCR DBCR_IDM		/* Enable Debug Exceptions	*/

#undef	CFG_SYS_DRAM_TEST			/* memory test, takes time	*/

#define CFG_SYS_CCSRBAR		0xE0000000
#define CFG_SYS_CCSRBAR_PHYS_LOW	CFG_SYS_CCSRBAR

/* DDR Setup */
#define CFG_SYS_DDR_SDRAM_BASE	0x00000000
#define CFG_SYS_SDRAM_BASE		CFG_SYS_DDR_SDRAM_BASE

/* I2C addresses of SPD EEPROMs */
#define SPD_EEPROM_ADDRESS	0x50	/* CTLR 0 DIMM 0 */


/* Hardcoded values, to use instead of SPD */
#define CFG_SYS_DDR_CS0_BNDS		0x0000000f
#define CFG_SYS_DDR_CS0_CONFIG		0x80010102
#define CFG_SYS_DDR_TIMING_0		0x00260802
#define CFG_SYS_DDR_TIMING_1		0x3935D322
#define CFG_SYS_DDR_TIMING_2		0x14904CC8
#define CFG_SYS_DDR_MODE			0x00480432
#define CFG_SYS_DDR_INTERVAL		0x030C0100
#define CFG_SYS_DDR_CONFIG_2		0x04400000
#define CFG_SYS_DDR_CONFIG			0xC3008000
#define CFG_SYS_DDR_CLK_CONTROL		0x03800000
#define CFG_SYS_SDRAM_SIZE			256 /* in Megs */

/*
 * Flash on the LocalBus
 */
#define CFG_SYS_FLASH0		0xFE000000
#define CFG_SYS_FLASH_BANKS_LIST	{ CFG_SYS_FLASH0 }

#define CFG_SYS_LBC_FLASH_BASE	CFG_SYS_FLASH0	/* Localbus flash start	*/
#define CFG_SYS_FLASH_BASE		CFG_SYS_LBC_FLASH_BASE /* start of FLASH	*/

#define CFG_SYS_LBC_LCRR		0x00030004    /* LB clock ratio reg	*/
#define CFG_SYS_LBC_LBCR		0x00000000    /* LB config reg		*/
#define CFG_SYS_LBC_LSRT		0x20000000    /* LB sdram refresh timer	*/
#define CFG_SYS_LBC_MRTPR		0x20000000    /* LB refresh timer presc.*/

#define CFG_SYS_INIT_RAM_ADDR	0xe4010000	/* Initial RAM address	*/
#define CFG_SYS_INIT_RAM_SIZE	0x4000		/* Size used area in RAM*/

#define CFG_SYS_INIT_SP_OFFSET	(CFG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)

/* FPGA and NAND */
#define CFG_SYS_FPGA_BASE		0xc0000000
#define CFG_SYS_FPGA_SIZE		0x00100000	/* 1 MB		*/

#define CFG_SYS_NAND_BASE		(CFG_SYS_FPGA_BASE + 0x70)

/* LIME GDC */
#define CFG_SYS_LIME_BASE		0xc8000000

/*
 * General PCI
 * Memory space is mapped 1-1.
 */

#define CFG_SYS_PCI1_MEM_PHYS	0x80000000
#define CFG_SYS_PCI1_IO_PHYS	0xE2000000

/*
 * Miscellaneous configurable options
 */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_SYS_BOOTMAPSZ	(8 << 20)	/* Initial Memory map for Linux	*/

#define CFG_ENV_FLAGS_LIST_STATIC "ethaddr:mw,eth1addr:mw,system1_addr:xw,serial#:sw,ethact:sw,ethprime:sw"

/* pass open firmware flat tree */

#endif	/* __CONFIG_H */
