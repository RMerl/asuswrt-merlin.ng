/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2011-2014 Freescale Semiconductor, Inc.
 */

/*
 * Corenet DS style board configuration file
 */
#ifndef __QEMU_PPCE500_H
#define __QEMU_PPCE500_H

/* Needed to fill the ccsrbar pointer */

/* Virtual address to CCSRBAR */
#define CFG_SYS_CCSRBAR		0xe0000000
/* Physical address should be a function call */
#ifndef __ASSEMBLY__
extern unsigned long long get_phys_ccsrbar_addr_early(void);
#define CFG_SYS_CCSRBAR_PHYS_HIGH (get_phys_ccsrbar_addr_early() >> 32)
#define CFG_SYS_CCSRBAR_PHYS_LOW get_phys_ccsrbar_addr_early()
#else
#define CFG_SYS_CCSRBAR_PHYS_HIGH 0x0
#define CFG_SYS_CCSRBAR_PHYS_LOW CFG_SYS_CCSRBAR
#endif

/* Virtual address to a temporary map if we need it (max 128MB) */
#define CFG_SYS_TMPVIRT		0xe8000000

/*
 * DDR Setup
 */
#define CFG_SYS_DDR_SDRAM_BASE	0x00000000
#define CFG_SYS_SDRAM_BASE		CFG_SYS_DDR_SDRAM_BASE

#define CFG_SYS_INIT_RAM_ADDR		0x00100000
#define CFG_SYS_INIT_RAM_ADDR_PHYS_HIGH	0x0
#define CFG_SYS_INIT_RAM_ADDR_PHYS_LOW	0x00100000
/* The assembler doesn't like typecast */
#define CFG_SYS_INIT_RAM_ADDR_PHYS \
	((CFG_SYS_INIT_RAM_ADDR_PHYS_HIGH * 1ull << 32) | \
	  CFG_SYS_INIT_RAM_ADDR_PHYS_LOW)
#define CFG_SYS_INIT_RAM_SIZE		0x00004000

#define CFG_SYS_INIT_SP_OFFSET	(CFG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)

/*
 * Miscellaneous configurable options
 */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 64 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_SYS_BOOTMAPSZ	(64 << 20)	/* Initial map for Linux*/

/*
 * Environment Configuration
 */

#endif	/* __QEMU_PPCE500_H */
