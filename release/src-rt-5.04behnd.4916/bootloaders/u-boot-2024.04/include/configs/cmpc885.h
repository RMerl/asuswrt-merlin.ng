/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022 CS Group
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* Definitions for initial stack pointer and data area (in DPRAM) */
#define CFG_SYS_INIT_RAM_ADDR		(CONFIG_SYS_IMMR + 0x2800)
#define CFG_SYS_INIT_RAM_SIZE		(0x2e00 - 0x2800)
#define CFG_SYS_INIT_SP			(CONFIG_SYS_IMMR + 0x3c00)

/* RAM configuration (note that CFG_SYS_SDRAM_BASE must be zero) */
#define CFG_SYS_SDRAM_BASE		0x00000000

/* FLASH Configuration */
#define CFG_SYS_FLASH_BASE		0x40000000

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 24 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_SYS_BOOTMAPSZ		(32 << 20)

/* NAND configuration part */
#define CFG_SYS_NAND_BASE		0xC0000000

/* Board names */
#define CFG_BOARD_CMPCXXX		"cmpc885"
#define CFG_BOARD_MCR3000_2G		"mcr3k_2g"
#define CFG_BOARD_VGOIP			"vgoip"
#define CFG_BOARD_MIAE			"miae"

#endif /* __CONFIG_H */
