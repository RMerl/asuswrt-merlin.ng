/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 by Vladimir Zapolskiy <vz@mleia.com>
 */

#ifndef _LPC32XX_UART_H
#define _LPC32XX_UART_H

#include <asm/types.h>

/* 14-clock UART Registers */
struct hsuart_regs {
	union {
		u32 rx;		/* Receiver FIFO		*/
		u32 tx;		/* Transmitter FIFO		*/
	};
	u32 level;		/* FIFO Level Register		*/
	u32 iir;		/* Interrupt ID Register	*/
	u32 ctrl;		/* Control Register		*/
	u32 rate;		/* Rate Control Register	*/
};

/* 14-clock UART Receiver FIFO Register bits */
#define HSUART_RX_BREAK			(1 << 10)
#define HSUART_RX_ERROR			(1 << 9)
#define HSUART_RX_EMPTY			(1 << 8)
#define HSUART_RX_DATA			(0xff << 0)

/* 14-clock UART Level Register bits */
#define HSUART_LEVEL_TX			(0xff << 8)
#define HSUART_LEVEL_RX			(0xff << 0)

/* 14-clock UART Interrupt Identification Register bits */
#define HSUART_IIR_TX_INT_SET		(1 << 6)
#define HSUART_IIR_RX_OE		(1 << 5)
#define HSUART_IIR_BRK			(1 << 4)
#define HSUART_IIR_FE			(1 << 3)
#define HSUART_IIR_RX_TIMEOUT		(1 << 2)
#define HSUART_IIR_RX_TRIG		(1 << 1)
#define HSUART_IIR_TX			(1 << 0)

/* 14-clock UART Control Register bits */
#define HSUART_CTRL_HRTS_INV		(1 << 21)
#define HSUART_CTRL_HRTS_TRIG_48	(0x3 << 19)
#define HSUART_CTRL_HRTS_TRIG_32	(0x2 << 19)
#define HSUART_CTRL_HRTS_TRIG_16	(0x1 << 19)
#define HSUART_CTRL_HRTS_TRIG_8		(0x0 << 19)
#define HSUART_CTRL_HRTS_EN		(1 << 18)
#define HSUART_CTRL_TMO_16		(0x3 << 16)
#define HSUART_CTRL_TMO_8		(0x2 << 16)
#define HSUART_CTRL_TMO_4		(0x1 << 16)
#define HSUART_CTRL_TMO_DISABLED	(0x0 << 16)
#define HSUART_CTRL_HCTS_INV		(1 << 15)
#define HSUART_CTRL_HCTS_EN		(1 << 14)
#define HSUART_CTRL_HSU_OFFSET(n)	((n) << 9)
#define HSUART_CTRL_HSU_BREAK		(1 << 8)
#define HSUART_CTRL_HSU_ERR_INT_EN	(1 << 7)
#define HSUART_CTRL_HSU_RX_INT_EN	(1 << 6)
#define HSUART_CTRL_HSU_TX_INT_EN	(1 << 5)
#define HSUART_CTRL_HSU_RX_TRIG_48	(0x5 << 2)
#define HSUART_CTRL_HSU_RX_TRIG_32	(0x4 << 2)
#define HSUART_CTRL_HSU_RX_TRIG_16	(0x3 << 2)
#define HSUART_CTRL_HSU_RX_TRIG_8	(0x2 << 2)
#define HSUART_CTRL_HSU_RX_TRIG_4	(0x1 << 2)
#define HSUART_CTRL_HSU_RX_TRIG_1	(0x0 << 2)
#define HSUART_CTRL_HSU_TX_TRIG_16	(0x3 << 0)
#define HSUART_CTRL_HSU_TX_TRIG_8	(0x2 << 0)
#define HSUART_CTRL_HSU_TX_TRIG_4	(0x1 << 0)
#define HSUART_CTRL_HSU_TX_TRIG_0	(0x0 << 0)

/* UART Control Registers */
struct uart_ctrl_regs {
	u32 ctrl;		/* Control Register		*/
	u32 clkmode;		/* Clock Mode Register		*/
	u32 loop;		/* Loopback Control Register	*/
};

/* UART Control Register bits */
#define UART_CTRL_UART3_MD_CTRL		(1 << 11)
#define UART_CTRL_HDPX_INV		(1 << 10)
#define UART_CTRL_HDPX_EN		(1 << 9)
#define UART_CTRL_UART6_IRDA		(1 << 5)
#define UART_CTRL_IR_TX6_INV		(1 << 4)
#define UART_CTRL_IR_RX6_INV		(1 << 3)
#define UART_CTRL_IR_RX_LENGTH		(1 << 2)
#define UART_CTRL_IR_TX_LENGTH		(1 << 1)
#define UART_CTRL_UART5_USB_MODE	(1 << 0)

/* UART Clock Mode Register bits */
#define UART_CLKMODE_STATX(n)		(1 << ((n) + 16))
#define UART_CLKMODE_STAT		(1 << 14)
#define UART_CLKMODE_MASK(n)		(0x3 << (2 * (n) - 2))
#define UART_CLKMODE_AUTO(n)		(0x2 << (2 * (n) - 2))
#define UART_CLKMODE_ON(n)		(0x1 << (2 * (n) - 2))
#define UART_CLKMODE_OFF(n)		(0x0 << (2 * (n) - 2))

/* UART Loopback Control Register bits */
#define UART_LOOPBACK(n)		(1 << ((n) - 1))

#endif /* _LPC32XX_UART_H */
