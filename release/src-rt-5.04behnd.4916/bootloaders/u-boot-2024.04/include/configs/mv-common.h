/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

/*
 * This file contains Marvell Board Specific common defincations.
 * This file should be included in board config header file.
 *
 * It supports common definations for Kirkwood platform
 * TBD: support for Orion5X platforms
 */

#ifndef _MV_COMMON_H
#define _MV_COMMON_H

#include <linux/stringify.h>

/*
 * High Level Configuration Options (easy to change)
 */

/*
 * Custom CONFIG_TEXT_BASE can be done in <board>.h
 */

/* additions for new ARM relocation support */
#define CFG_SYS_SDRAM_BASE	0x00000000

/*
 * NS16550 Configuration
 */
#define CFG_SYS_NS16550_CLK		CFG_SYS_TCLK
#if !CONFIG_IS_ENABLED(DM_SERIAL)
#define CFG_SYS_NS16550_COM1		MV_UART_CONSOLE_BASE
#endif

#if defined(CONFIG_ARMADA_38X) && !defined(CFG_SYS_BAUDRATE_TABLE)
#define CFG_SYS_BAUDRATE_TABLE	{ 300, 600, 1200, 1800, 2400, 4800, \
					  9600, 19200, 38400, 57600, 115200, \
					  230400, 460800, 500000, 576000, \
					  921600, 1000000, 1152000, 1500000, \
					  2000000, 2500000, 3125000, 4000000, \
					  5200000 }
#endif

/* auto boot */

/*
 * Other required minimal configurations
 */

/* ====> Include platform Common Definitions */
#include <asm/arch/config.h>

/* ====> Include driver Common Definitions */
/*
 * Common NAND configuration
 */

#endif /* _MV_COMMON_H */
