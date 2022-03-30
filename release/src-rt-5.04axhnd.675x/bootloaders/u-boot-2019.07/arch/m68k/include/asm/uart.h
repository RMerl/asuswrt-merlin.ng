/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * uart.h -- ColdFire internal UART support defines.
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

/****************************************************************************/
#ifndef	uart_h
#define	uart_h
/****************************************************************************/

/* UART module registers */
/* Register read/write struct */
typedef struct uart {
	u8 umr;			/* 0x00 Mode Register */
	u8 resv0[0x3];
	union {
		u8 usr;		/* 0x04 Status Register */
		u8 ucsr;	/* 0x04 Clock Select Register */
	};
	u8 resv1[0x3];
	u8 ucr;			/* 0x08 Command Register */
	u8 resv2[0x3];
	union {
		u8 utb;		/* 0x0c Transmit Buffer */
		u8 urb;		/* 0x0c Receive Buffer */
	};
	u8 resv3[0x3];
	union {
		u8 uipcr;	/* 0x10 Input Port Change Register */
		u8 uacr;	/* 0x10 Auxiliary Control reg */
	};
	u8 resv4[0x3];
	union {
		u8 uimr;	/* 0x14 Interrupt Mask reg */
		u8 uisr;	/* 0x14 Interrupt Status reg */
	};
	u8 resv5[0x3];
	u8 ubg1;		/* 0x18 Counter Timer Upper Register */
	u8 resv6[0x3];
	u8 ubg2;		/* 0x1c Counter Timer Lower Register */
	u8 resv7[0x17];
	u8 uip;			/* 0x34 Input Port Register */
	u8 resv8[0x3];
	u8 uop1;		/* 0x38 Output Port Set Register */
	u8 resv9[0x3];
	u8 uop0;		/* 0x3c Output Port Reset Register */
} uart_t;

/*********************************************************************
* Universal Asynchronous Receiver Transmitter (UART)
*********************************************************************/
/* Bit definitions and macros for UMR */
#define UART_UMR_BC(x)			(((x)&0x03))
#define UART_UMR_PT			(0x04)
#define UART_UMR_PM(x)			(((x)&0x03)<<3)
#define UART_UMR_ERR			(0x20)
#define UART_UMR_RXIRQ			(0x40)
#define UART_UMR_RXRTS			(0x80)
#define UART_UMR_SB(x)			(((x)&0x0F))
#define UART_UMR_TXCTS			(0x10)	/* Trsnsmit CTS */
#define UART_UMR_TXRTS			(0x20)	/* Transmit RTS */
#define UART_UMR_CM(x)			(((x)&0x03)<<6)	/* CM bits */
#define UART_UMR_PM_MULTI_ADDR		(0x1C)
#define UART_UMR_PM_MULTI_DATA		(0x18)
#define UART_UMR_PM_NONE		(0x10)
#define UART_UMR_PM_FORCE_HI		(0x0C)
#define UART_UMR_PM_FORCE_LO		(0x08)
#define UART_UMR_PM_ODD			(0x04)
#define UART_UMR_PM_EVEN		(0x00)
#define UART_UMR_BC_5			(0x00)
#define UART_UMR_BC_6			(0x01)
#define UART_UMR_BC_7			(0x02)
#define UART_UMR_BC_8			(0x03)
#define UART_UMR_CM_NORMAL		(0x00)
#define UART_UMR_CM_ECH			(0x40)
#define UART_UMR_CM_LOCAL_LOOP		(0x80)
#define UART_UMR_CM_REMOTE_LOOP		(0xC0)
#define UART_UMR_SB_STOP_BITS_1		(0x07)
#define UART_UMR_SB_STOP_BITS_15	(0x08)
#define UART_UMR_SB_STOP_BITS_2		(0x0F)

/* Bit definitions and macros for USR */
#define UART_USR_RXRDY			(0x01)
#define UART_USR_FFULL			(0x02)
#define UART_USR_TXRDY			(0x04)
#define UART_USR_TXEMP			(0x08)
#define UART_USR_OE			(0x10)
#define UART_USR_PE			(0x20)
#define UART_USR_FE			(0x40)
#define UART_USR_RB			(0x80)

/* Bit definitions and macros for UCSR */
#define UART_UCSR_TCS(x)		(((x)&0x0F))
#define UART_UCSR_RCS(x)		(((x)&0x0F)<<4)
#define UART_UCSR_RCS_SYS_CLK		(0xD0)
#define UART_UCSR_RCS_CTM16		(0xE0)
#define UART_UCSR_RCS_CTM		(0xF0)
#define UART_UCSR_TCS_SYS_CLK		(0x0D)
#define UART_UCSR_TCS_CTM16		(0x0E)
#define UART_UCSR_TCS_CTM		(0x0F)

/* Bit definitions and macros for UCR */
#define UART_UCR_RXC(x)			(((x)&0x03))
#define UART_UCR_TXC(x)			(((x)&0x03)<<2)
#define UART_UCR_MISC(x)		(((x)&0x07)<<4)
#define UART_UCR_NONE			(0x00)
#define UART_UCR_STOP_BREAK		(0x70)
#define UART_UCR_START_BREAK		(0x60)
#define UART_UCR_BKCHGINT		(0x50)
#define UART_UCR_RESET_ERROR		(0x40)
#define UART_UCR_RESET_TX		(0x30)
#define UART_UCR_RESET_RX		(0x20)
#define UART_UCR_RESET_MR		(0x10)
#define UART_UCR_TX_DISABLED		(0x08)
#define UART_UCR_TX_ENABLED		(0x04)
#define UART_UCR_RX_DISABLED		(0x02)
#define UART_UCR_RX_ENABLED		(0x01)

/* Bit definitions and macros for UIPCR */
#define UART_UIPCR_CTS			(0x01)
#define UART_UIPCR_COS			(0x10)

/* Bit definitions and macros for UACR */
#define UART_UACR_IEC			(0x01)

/* Bit definitions and macros for UIMR */
#define UART_UIMR_TXRDY			(0x01)
#define UART_UIMR_RXRDY_FU		(0x02)
#define UART_UIMR_DB			(0x04)
#define UART_UIMR_COS			(0x80)

/* Bit definitions and macros for UISR */
#define UART_UISR_TXRDY			(0x01)
#define UART_UISR_RXRDY_FU		(0x02)
#define UART_UISR_DB			(0x04)
#define UART_UISR_RXFTO			(0x08)
#define UART_UISR_TXFIFO		(0x10)
#define UART_UISR_RXFIFO		(0x20)
#define UART_UISR_COS			(0x80)

/* Bit definitions and macros for UIP */
#define UART_UIP_CTS			(0x01)

/* Bit definitions and macros for UOP1 */
#define UART_UOP1_RTS			(0x01)

/* Bit definitions and macros for UOP0 */
#define UART_UOP0_RTS			(0x01)

/****************************************************************************/
#endif				/* mcfuart_h */
