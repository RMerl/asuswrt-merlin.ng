/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 */

#ifndef	__REGS_UART_H__
#define	__REGS_UART_H__

#define	FFUART_BASE		0x40100000
#define	BTUART_BASE		0x40200000
#define	STUART_BASE		0x40700000
#define	HWUART_BASE		0x41600000

struct pxa_uart_regs {
	union {
		uint32_t	thr;
		uint32_t	rbr;
		uint32_t	dll;
	};
	union {
		uint32_t	ier;
		uint32_t	dlh;
	};
	union {
		uint32_t	fcr;
		uint32_t	iir;
	};
	uint32_t	lcr;
	uint32_t	mcr;
	uint32_t	lsr;
	uint32_t	msr;
	uint32_t	spr;
	uint32_t	isr;
};

#define	IER_DMAE	(1 << 7)
#define	IER_UUE		(1 << 6)
#define	IER_NRZE	(1 << 5)
#define	IER_RTIOE	(1 << 4)
#define	IER_MIE		(1 << 3)
#define	IER_RLSE	(1 << 2)
#define	IER_TIE		(1 << 1)
#define	IER_RAVIE	(1 << 0)

#define	IIR_FIFOES1	(1 << 7)
#define	IIR_FIFOES0	(1 << 6)
#define	IIR_TOD		(1 << 3)
#define	IIR_IID2	(1 << 2)
#define	IIR_IID1	(1 << 1)
#define	IIR_IP		(1 << 0)

#define	FCR_ITL2	(1 << 7)
#define	FCR_ITL1	(1 << 6)
#define	FCR_RESETTF	(1 << 2)
#define	FCR_RESETRF	(1 << 1)
#define	FCR_TRFIFOE	(1 << 0)
#define	FCR_ITL_1	0
#define	FCR_ITL_8	(FCR_ITL1)
#define	FCR_ITL_16	(FCR_ITL2)
#define	FCR_ITL_32	(FCR_ITL2|FCR_ITL1)

#define	LCR_DLAB	(1 << 7)
#define	LCR_SB		(1 << 6)
#define	LCR_STKYP	(1 << 5)
#define	LCR_EPS		(1 << 4)
#define	LCR_PEN		(1 << 3)
#define	LCR_STB		(1 << 2)
#define	LCR_WLS1	(1 << 1)
#define	LCR_WLS0	(1 << 0)

#define	LSR_FIFOE	(1 << 7)
#define	LSR_TEMT	(1 << 6)
#define	LSR_TDRQ	(1 << 5)
#define	LSR_BI		(1 << 4)
#define	LSR_FE		(1 << 3)
#define	LSR_PE		(1 << 2)
#define	LSR_OE		(1 << 1)
#define	LSR_DR		(1 << 0)

#define	MCR_LOOP	(1 << 4)
#define	MCR_OUT2	(1 << 3)
#define	MCR_OUT1	(1 << 2)
#define	MCR_RTS		(1 << 1)
#define	MCR_DTR		(1 << 0)

#define	MSR_DCD		(1 << 7)
#define	MSR_RI		(1 << 6)
#define	MSR_DSR		(1 << 5)
#define	MSR_CTS		(1 << 4)
#define	MSR_DDCD	(1 << 3)
#define	MSR_TERI	(1 << 2)
#define	MSR_DDSR	(1 << 1)
#define	MSR_DCTS	(1 << 0)

#endif	/* __REGS_UART_H__ */
