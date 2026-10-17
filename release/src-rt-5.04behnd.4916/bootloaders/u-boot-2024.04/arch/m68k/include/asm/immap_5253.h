/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MCF5253 Internal Memory Map
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef __IMMAP_5253__
#define __IMMAP_5253__

#define MMAP_INTC		(CFG_SYS_MBAR + 0x00000040)
#define MMAP_FBCS		(CFG_SYS_MBAR + 0x00000080)
#define MMAP_DTMR0		(CFG_SYS_MBAR + 0x00000140)
#define MMAP_DTMR1		(CFG_SYS_MBAR + 0x00000180)
#define MMAP_UART0		(CFG_SYS_MBAR + 0x000001C0)
#define MMAP_UART1		(CFG_SYS_MBAR + 0x00000200)
#define MMAP_I2C0		(CFG_SYS_MBAR + 0x00000280)
#define MMAP_QSPI		(CFG_SYS_MBAR + 0x00000400)
#define MMAP_CAN0		(CFG_SYS_MBAR + 0x00010000)
#define MMAP_CAN1		(CFG_SYS_MBAR + 0x00011000)

#define MMAP_PAR		(CFG_SYS_MBAR2 + 0x0000019C)
#define MMAP_I2C1		(CFG_SYS_MBAR2 + 0x00000440)
#define MMAP_UART2		(CFG_SYS_MBAR2 + 0x00000C00)

#include <asm/coldfire/ata.h>
#include <asm/coldfire/flexbus.h>
#include <asm/coldfire/flexcan.h>
#include <asm/coldfire/qspi.h>

typedef struct canex_ctrl {
	can_msg_t msg[32];	/* 0x80 Message Buffer 0-31 */
} canex_t;

#endif				/* __IMMAP_5253__ */
