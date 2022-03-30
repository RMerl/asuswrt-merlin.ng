/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MCF5249 Internal Memory Map
 *
 * Copyright (c) 2003 Josef Baumgartner <josef.baumgartner@telex.de>
 */

#ifndef __IMMAP_5249__
#define __IMMAP_5249__

#define MMAP_INTC		(CONFIG_SYS_MBAR + 0x00000040)
#define MMAP_FBCS		(CONFIG_SYS_MBAR + 0x00000080)
#define MMAP_DTMR0		(CONFIG_SYS_MBAR + 0x00000140)
#define MMAP_DTMR1		(CONFIG_SYS_MBAR + 0x00000180)
#define MMAP_UART0		(CONFIG_SYS_MBAR + 0x000001C0)
#define MMAP_UART1		(CONFIG_SYS_MBAR + 0x00000200)
#define MMAP_QSPI		(CONFIG_SYS_MBAR + 0x00000400)

#include <asm/coldfire/flexbus.h>
#include <asm/coldfire/qspi.h>

#endif				/* __IMMAP_5249__ */
