/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for Total Compute platform. Parts were derived from other ARM
 * configurations.
 * (C) Copyright 2020-2023 Arm Limited
 * Usama Arif <usama.arif@arm.com>
 */

#ifndef __TOTAL_COMPUTE_H
#define __TOTAL_COMPUTE_H

/* Link Definitions */

/* AP non-secure UART base address */
#define UART0_BASE		0x2A400000

/* PL011 Serial Configuration */
#define CFG_PL011_CLOCK	7372800

/* Miscellaneous configurable options */

/* Physical Memory Map */
#define PHYS_SDRAM_1		0x80000000
/* Top 48MB reserved for secure world use */
#define DRAM_SEC_SIZE		0x03000000
#define PHYS_SDRAM_1_SIZE	0x80000000 - DRAM_SEC_SIZE
#define CFG_SYS_SDRAM_BASE	PHYS_SDRAM_1

#define PHYS_SDRAM_2		0x8080000000
#define PHYS_SDRAM_2_SIZE	0x180000000

#define CFG_EXTRA_ENV_SETTINGS	\
				"bootm_size=0x20000000\0"	\
				"load_addr=0xa0000000\0"	\
				"kernel_addr_r=0x80080000\0"	\
				"initrd_addr_r=0x88000000\0"	\
				"fdt_addr_r=0x83000000\0"
/*
 * If vbmeta partition is present, boot Android with verification using AVB.
 * Else if system partition is present (no vbmeta partition), boot Android
 * without verification (for development purposes).
 * Else boot FIT image.
 */

#define CFG_SYS_FLASH_BASE		0x0C000000

#endif /* __TOTAL_COMPUTE_H */
