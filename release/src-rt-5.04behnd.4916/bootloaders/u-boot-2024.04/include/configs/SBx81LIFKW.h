/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016 Allied Telesis <www.alliedtelesis.co.nz>
 */

#ifndef _CONFIG_SBX81LIFKW_H
#define _CONFIG_SBX81LIFKW_H

/* additions for new ARM relocation support */
#define CFG_SYS_SDRAM_BASE	0x00000000

/*
 * NS16550 Configuration
 */
#define CFG_SYS_NS16550_CLK		CFG_SYS_TCLK
#define CFG_SYS_NS16550_COM1		KW_UART0_BASE

/*
 * Serial Port configuration
 * The following definitions let you select what serial you want to use
 * for your console driver.
 */

#define MTDPARTS_MTDOOPS "errlog"

/*
 *  Environment variables configurations
 */

/*
 * U-Boot bootcode configuration
 */

#define CFG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Mem map for Linux*/

/* size in bytes reserved for initial data */

#include <asm/arch/config.h>

#endif /* _CONFIG_SBX81LIFKW_H */
