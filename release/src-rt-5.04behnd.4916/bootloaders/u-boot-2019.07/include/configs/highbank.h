/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2010-2011 Calxeda, Inc.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_SYS_BOOTMAPSZ		(16 << 20)

#define CONFIG_SYS_TIMER_RATE		(150000000/256)
#define CONFIG_SYS_TIMER_COUNTER	(0xFFF34000 + 0x4)
#define CONFIG_SYS_TIMER_COUNTS_DOWN

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(512 * 1024)

#define CONFIG_PL011_CLOCK		150000000
#define CONFIG_PL01x_PORTS		{ (void *)(0xFFF36000) }

#define CONFIG_SYS_BOOTCOUNT_LE		/* Use little-endian accessors */

#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	5
#define CONFIG_SYS_SCSI_MAX_LUN		1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
					CONFIG_SYS_SCSI_MAX_LUN)

#define CONFIG_CALXEDA_XGMAC

/*
 * Command line configuration.
 */

#define CONFIG_BOOT_RETRY_TIME		-1
#define CONFIG_RESET_TO_RETRY

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_SYS_LOAD_ADDR		0x800000
#define CONFIG_SYS_64BIT_LBA

/*-----------------------------------------------------------------------
 * Physical Memory Map
 * The DRAM is already setup, so do not touch the DT node later.
 */
#define PHYS_SDRAM_1_SIZE		(4089 << 20)
#define CONFIG_SYS_MEMTEST_START	0x100000
#define CONFIG_SYS_MEMTEST_END		(PHYS_SDRAM_1_SIZE - 0x100000)

/* Environment data setup
*/
#define CONFIG_SYS_NVRAM_BASE_ADDR	0xfff88000	/* NVRAM base address */
#define CONFIG_SYS_NVRAM_SIZE		0x8000		/* NVRAM size */
#define CONFIG_ENV_SIZE			0x2000		/* Size of Environ */
#define CONFIG_ENV_ADDR			CONFIG_SYS_NVRAM_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_INIT_SP_ADDR		0x01000000
#define CONFIG_SKIP_LOWLEVEL_INIT

#endif
