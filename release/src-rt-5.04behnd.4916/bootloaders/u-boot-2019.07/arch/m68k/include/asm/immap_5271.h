/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MCF5272 Internal Memory Map
 *
 * Copyright (c) 2003 Josef Baumgartner <josef.baumgartner@telex.de>
 *               2006 Zachary P. Landau <zachary.landau@labxtechnologies.com>
 */

#ifndef __IMMAP_5271__
#define __IMMAP_5271__

#define MMAP_SCM	(CONFIG_SYS_MBAR + 0x00000000)
#define MMAP_SDRAM	(CONFIG_SYS_MBAR + 0x00000040)
#define MMAP_FBCS	(CONFIG_SYS_MBAR + 0x00000080)
#define MMAP_DMA0	(CONFIG_SYS_MBAR + 0x00000100)
#define MMAP_DMA1	(CONFIG_SYS_MBAR + 0x00000110)
#define MMAP_DMA2	(CONFIG_SYS_MBAR + 0x00000120)
#define MMAP_DMA3	(CONFIG_SYS_MBAR + 0x00000130)
#define MMAP_UART0	(CONFIG_SYS_MBAR + 0x00000200)
#define MMAP_UART1	(CONFIG_SYS_MBAR + 0x00000240)
#define MMAP_UART2	(CONFIG_SYS_MBAR + 0x00000280)
#define MMAP_I2C	(CONFIG_SYS_MBAR + 0x00000300)
#define MMAP_QSPI	(CONFIG_SYS_MBAR + 0x00000340)
#define MMAP_DTMR0	(CONFIG_SYS_MBAR + 0x00000400)
#define MMAP_DTMR1	(CONFIG_SYS_MBAR + 0x00000440)
#define MMAP_DTMR2	(CONFIG_SYS_MBAR + 0x00000480)
#define MMAP_DTMR3	(CONFIG_SYS_MBAR + 0x000004C0)
#define MMAP_INTC0	(CONFIG_SYS_MBAR + 0x00000C00)
#define MMAP_INTC1	(CONFIG_SYS_MBAR + 0x00000D00)
#define MMAP_INTCACK	(CONFIG_SYS_MBAR + 0x00000F00)
#define MMAP_FEC	(CONFIG_SYS_MBAR + 0x00001000)
#define MMAP_FECFIFO	(CONFIG_SYS_MBAR + 0x00001400)
#define MMAP_GPIO	(CONFIG_SYS_MBAR + 0x00100000)
#define MMAP_CCM	(CONFIG_SYS_MBAR + 0x00110000)
#define MMAP_PLL	(CONFIG_SYS_MBAR + 0x00120000)
#define MMAP_EPORT	(CONFIG_SYS_MBAR + 0x00130000)
#define MMAP_WDOG	(CONFIG_SYS_MBAR + 0x00140000)
#define MMAP_PIT0	(CONFIG_SYS_MBAR + 0x00150000)
#define MMAP_PIT1	(CONFIG_SYS_MBAR + 0x00160000)
#define MMAP_PIT2	(CONFIG_SYS_MBAR + 0x00170000)
#define MMAP_PIT3	(CONFIG_SYS_MBAR + 0x00180000)
#define MMAP_MDHA	(CONFIG_SYS_MBAR + 0x00190000)
#define MMAP_RNG	(CONFIG_SYS_MBAR + 0x001A0000)
#define MMAP_SKHA	(CONFIG_SYS_MBAR + 0x001B0000)
#define MMAP_CAN1	(CONFIG_SYS_MBAR + 0x001C0000)
#define MMAP_ETPU	(CONFIG_SYS_MBAR + 0x001D0000)
#define MMAP_CAN2	(CONFIG_SYS_MBAR + 0x001F0000)

#include <asm/coldfire/eport.h>
#include <asm/coldfire/flexbus.h>
#include <asm/coldfire/intctrl.h>
#include <asm/coldfire/mdha.h>
#include <asm/coldfire/qspi.h>
#include <asm/coldfire/rng.h>
#include <asm/coldfire/skha.h>


#endif				/* __IMMAP_5271__ */
