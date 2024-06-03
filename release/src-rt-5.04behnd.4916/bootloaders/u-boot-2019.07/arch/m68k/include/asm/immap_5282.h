/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MCF5282 Internal Memory Map
 *
 * Copyright (c) 2003 Josef Baumgartner <josef.baumgartner@telex.de>
 */

#ifndef __IMMAP_5282__
#define __IMMAP_5282__

#define MMAP_SCM	(CONFIG_SYS_MBAR + 0x00000000)
#define MMAP_SDRAMC	(CONFIG_SYS_MBAR + 0x00000040)
#define MMAP_FBCS	(CONFIG_SYS_MBAR + 0x00000080)
#define MMAP_DMA0	(CONFIG_SYS_MBAR + 0x00000100)
#define MMAP_DMA1	(CONFIG_SYS_MBAR + 0x00000140)
#define MMAP_DMA2	(CONFIG_SYS_MBAR + 0x00000180)
#define MMAP_DMA3	(CONFIG_SYS_MBAR + 0x000001C0)
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
#define MMAP_QADC	(CONFIG_SYS_MBAR + 0x00190000)
#define MMAP_GPTMRA	(CONFIG_SYS_MBAR + 0x001A0000)
#define MMAP_GPTMRB	(CONFIG_SYS_MBAR + 0x001B0000)
#define MMAP_CAN	(CONFIG_SYS_MBAR + 0x001C0000)
#define MMAP_CFMC	(CONFIG_SYS_MBAR + 0x001D0000)
#define MMAP_CFMMEM	(CONFIG_SYS_MBAR + 0x04000000)

#include <asm/coldfire/eport.h>
#include <asm/coldfire/flexbus.h>
#include <asm/coldfire/flexcan.h>
#include <asm/coldfire/intctrl.h>
#include <asm/coldfire/qspi.h>

/* System Control Module */
typedef struct scm_ctrl {
	u32 ipsbar;
	u32 res1;
	u32 rambar;
	u32 res2;
	u8 crsr;
	u8 cwcr;
	u8 lpicr;
	u8 cwsr;
	u32 res3;
	u8 mpark;
	u8 res4[3];
	u8 pacr0;
	u8 pacr1;
	u8 pacr2;
	u8 pacr3;
	u8 pacr4;
	u8 res5;
	u8 pacr5;
	u8 pacr6;
	u8 pacr7;
	u8 res6;
	u8 pacr8;
	u8 res7;
	u8 gpacr0;
	u8 gpacr1;
	u16 res8;
} scm_t;

typedef struct canex_ctrl {
	can_msg_t msg[16];	/* 0x00 Message Buffer 0-15 */
} canex_t;

/* Clock Module registers */
typedef struct pll_ctrl {
	u16 syncr;		/* 0x00 synthesizer control register */
	u16 synsr;		/* 0x02 synthesizer status register */
} pll_t;

/* Watchdog registers */
typedef struct wdog_ctrl {
	ushort wcr;
	ushort wmr;
	ushort wcntr;
	ushort wsr;
} wdog_t;

#endif				/* __IMMAP_5282__ */
