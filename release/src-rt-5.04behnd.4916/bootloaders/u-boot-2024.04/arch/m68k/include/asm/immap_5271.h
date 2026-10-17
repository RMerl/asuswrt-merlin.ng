/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MCF5272 Internal Memory Map
 *
 * Copyright (c) 2003 Josef Baumgartner <josef.baumgartner@telex.de>
 *               2006 Zachary P. Landau <zachary.landau@labxtechnologies.com>
 */

#ifndef __IMMAP_5271__
#define __IMMAP_5271__

#define MMAP_SCM	(CFG_SYS_MBAR + 0x00000000)
#define MMAP_SDRAM	(CFG_SYS_MBAR + 0x00000040)
#define MMAP_FBCS	(CFG_SYS_MBAR + 0x00000080)
#define MMAP_DMA0	(CFG_SYS_MBAR + 0x00000100)
#define MMAP_DMA1	(CFG_SYS_MBAR + 0x00000110)
#define MMAP_DMA2	(CFG_SYS_MBAR + 0x00000120)
#define MMAP_DMA3	(CFG_SYS_MBAR + 0x00000130)
#define MMAP_UART0	(CFG_SYS_MBAR + 0x00000200)
#define MMAP_UART1	(CFG_SYS_MBAR + 0x00000240)
#define MMAP_UART2	(CFG_SYS_MBAR + 0x00000280)
#define MMAP_I2C	(CFG_SYS_MBAR + 0x00000300)
#define MMAP_QSPI	(CFG_SYS_MBAR + 0x00000340)
#define MMAP_DTMR0	(CFG_SYS_MBAR + 0x00000400)
#define MMAP_DTMR1	(CFG_SYS_MBAR + 0x00000440)
#define MMAP_DTMR2	(CFG_SYS_MBAR + 0x00000480)
#define MMAP_DTMR3	(CFG_SYS_MBAR + 0x000004C0)
#define MMAP_INTC0	(CFG_SYS_MBAR + 0x00000C00)
#define MMAP_INTC1	(CFG_SYS_MBAR + 0x00000D00)
#define MMAP_INTCACK	(CFG_SYS_MBAR + 0x00000F00)
#define MMAP_FEC	(CFG_SYS_MBAR + 0x00001000)
#define MMAP_FECFIFO	(CFG_SYS_MBAR + 0x00001400)
#define MMAP_GPIO	(CFG_SYS_MBAR + 0x00100000)
#define MMAP_CCM	(CFG_SYS_MBAR + 0x00110000)
#define MMAP_PLL	(CFG_SYS_MBAR + 0x00120000)
#define MMAP_EPORT	(CFG_SYS_MBAR + 0x00130000)
#define MMAP_WDOG	(CFG_SYS_MBAR + 0x00140000)
#define MMAP_PIT0	(CFG_SYS_MBAR + 0x00150000)
#define MMAP_PIT1	(CFG_SYS_MBAR + 0x00160000)
#define MMAP_PIT2	(CFG_SYS_MBAR + 0x00170000)
#define MMAP_PIT3	(CFG_SYS_MBAR + 0x00180000)
#define MMAP_MDHA	(CFG_SYS_MBAR + 0x00190000)
#define MMAP_RNG	(CFG_SYS_MBAR + 0x001A0000)
#define MMAP_SKHA	(CFG_SYS_MBAR + 0x001B0000)
#define MMAP_CAN1	(CFG_SYS_MBAR + 0x001C0000)
#define MMAP_ETPU	(CFG_SYS_MBAR + 0x001D0000)
#define MMAP_CAN2	(CFG_SYS_MBAR + 0x001F0000)

#include <asm/coldfire/eport.h>
#include <asm/coldfire/flexbus.h>
#include <asm/coldfire/intctrl.h>
#include <asm/coldfire/mdha.h>
#include <asm/coldfire/qspi.h>
#include <asm/coldfire/rng.h>
#include <asm/coldfire/skha.h>


#endif				/* __IMMAP_5271__ */
