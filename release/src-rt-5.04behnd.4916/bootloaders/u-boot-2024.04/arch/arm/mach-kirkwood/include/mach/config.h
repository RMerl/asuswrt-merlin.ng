/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Lei Wen <leiwen@marvell.com>
 */

/*
 * This file should be included in board config header file.
 *
 * It supports common definitions for Kirkwood platform
 */

#ifndef _KW_CONFIG_H
#define _KW_CONFIG_H

#if defined (CONFIG_KW88F6281)
#include <asm/arch/kw88f6281.h>
#elif defined (CONFIG_KW88F6192)
#include <asm/arch/kw88f6192.h>
#else
#error "SOC Name not defined"
#endif /* CONFIG_KW88F6281 */

#include <asm/arch/soc.h>

#define CFG_I2C_MVTWSI_BASE0	KW_TWSI_BASE
#define MV_UART_CONSOLE_BASE	KW_UART0_BASE
#define MV_SATA_BASE		KW_SATA_BASE
#define MV_SATA_PORT0_OFFSET	KW_SATA_PORT0_OFFSET
#define MV_SATA_PORT1_OFFSET	KW_SATA_PORT1_OFFSET

/*
 * NAND configuration
 */
#ifdef CONFIG_CMD_NAND
#define CFG_SYS_NAND_BASE		0xD8000000	/* MV_DEFADR_NANDF */
#define NAND_ALLOW_ERASE_ALL		1
#endif

/*
 * IDE Support on SATA ports
 */
#ifdef CONFIG_IDE
#define __io
/* Data, registers and alternate blocks are at the same offset */
/* Each 8-bit ATA register is aligned to a 4-bytes address */
/* CONFIG_IDE requires some #defines for ATA registers */
/* ATA registers base is at SATA controller base */
#endif /* CONFIG_IDE */

/* Use common timer */
#ifndef CONFIG_TIMER
#define CFG_SYS_TIMER_COUNTER	(MVEBU_TIMER_BASE + 0x14)
#define CFG_SYS_TIMER_RATE		CFG_SYS_TCLK
#endif

#endif /* _KW_CONFIG_H */
