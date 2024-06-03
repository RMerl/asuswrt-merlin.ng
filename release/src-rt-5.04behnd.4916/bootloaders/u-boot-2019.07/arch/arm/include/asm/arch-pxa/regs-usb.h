/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * PXA25x UDC definitions
 *
 * Copyright (C) 2012 Łukasz Dałek <luk0104@gmail.com>
 */

#ifndef __REGS_USB_H__
#define __REGS_USB_H__

struct pxa25x_udc_regs {
	/* UDC Control Register */
	uint32_t	udccr; /* 0x000 */
	uint32_t	reserved1;

	/* UDC Control Function Register */
	uint32_t	udccfr; /* 0x008 */
	uint32_t	reserved2;

	/* UDC Endpoint Control/Status Registers */
	uint32_t	udccs[16]; /* 0x010 - 0x04c */

	/* UDC Interrupt Control/Status Registers */
	uint32_t	uicr0; /* 0x050 */
	uint32_t	uicr1; /* 0x054 */
	uint32_t	usir0; /* 0x058 */
	uint32_t	usir1; /* 0x05c */

	/* UDC Frame Number/Byte Count Registers */
	uint32_t	ufnrh;  /* 0x060 */
	uint32_t	ufnrl;  /* 0x064 */
	uint32_t	ubcr2;  /* 0x068 */
	uint32_t	ubcr4;  /* 0x06c */
	uint32_t	ubcr7;  /* 0x070 */
	uint32_t	ubcr9;  /* 0x074 */
	uint32_t	ubcr12; /* 0x078 */
	uint32_t	ubcr14; /* 0x07c */

	/* UDC Endpoint Data Registers */
	uint32_t	uddr0;  /* 0x080 */
	uint32_t	reserved3[7];
	uint32_t	uddr5;  /* 0x0a0 */
	uint32_t	reserved4[7];
	uint32_t	uddr10; /* 0x0c0 */
	uint32_t	reserved5[7];
	uint32_t	uddr15; /* 0x0e0 */
	uint32_t	reserved6[7];
	uint32_t	uddr1;  /* 0x100 */
	uint32_t	reserved7[31];
	uint32_t	uddr2;  /* 0x180 */
	uint32_t	reserved8[31];
	uint32_t	uddr3;  /* 0x200 */
	uint32_t	reserved9[127];
	uint32_t	uddr4;  /* 0x400 */
	uint32_t	reserved10[127];
	uint32_t	uddr6;  /* 0x600 */
	uint32_t	reserved11[31];
	uint32_t	uddr7;  /* 0x680 */
	uint32_t	reserved12[31];
	uint32_t	uddr8;  /* 0x700 */
	uint32_t	reserved13[127];
	uint32_t	uddr9;  /* 0x900 */
	uint32_t	reserved14[127];
	uint32_t	uddr11; /* 0xb00 */
	uint32_t	reserved15[31];
	uint32_t	uddr12; /* 0xb80 */
	uint32_t	reserved16[31];
	uint32_t	uddr13; /* 0xc00 */
	uint32_t	reserved17[127];
	uint32_t	uddr14; /* 0xe00 */

};

#define PXA25X_UDC_BASE		0x40600000

#define UDCCR_UDE		(1 << 0)
#define UDCCR_UDA		(1 << 1)
#define UDCCR_RSM		(1 << 2)
#define UDCCR_RESIR		(1 << 3)
#define UDCCR_SUSIR		(1 << 4)
#define UDCCR_SRM		(1 << 5)
#define UDCCR_RSTIR		(1 << 6)
#define UDCCR_REM		(1 << 7)

/* Bulk IN endpoint 1/6/11 */
#define UDCCS_BI_TSP		(1 << 7)
#define UDCCS_BI_FST		(1 << 5)
#define UDCCS_BI_SST		(1 << 4)
#define UDCCS_BI_TUR		(1 << 3)
#define UDCCS_BI_FTF		(1 << 2)
#define UDCCS_BI_TPC		(1 << 1)
#define UDCCS_BI_TFS		(1 << 0)

/* Bulk OUT endpoint 2/7/12 */
#define UDCCS_BO_RSP		(1 << 7)
#define UDCCS_BO_RNE		(1 << 6)
#define UDCCS_BO_FST		(1 << 5)
#define UDCCS_BO_SST		(1 << 4)
#define UDCCS_BO_DME		(1 << 3)
#define UDCCS_BO_RPC		(1 << 1)
#define UDCCS_BO_RFS		(1 << 0)

/* Isochronous OUT endpoint 4/9/14 */
#define UDCCS_IO_RSP		(1 << 7)
#define UDCCS_IO_RNE		(1 << 6)
#define UDCCS_IO_DME		(1 << 3)
#define UDCCS_IO_ROF		(1 << 2)
#define UDCCS_IO_RPC		(1 << 1)
#define UDCCS_IO_RFS		(1 << 0)

/* Control endpoint 0 */
#define UDCCS0_OPR		(1 << 0)
#define UDCCS0_IPR		(1 << 1)
#define UDCCS0_FTF		(1 << 2)
#define UDCCS0_DRWF		(1 << 3)
#define UDCCS0_SST		(1 << 4)
#define UDCCS0_FST		(1 << 5)
#define UDCCS0_RNE		(1 << 6)
#define UDCCS0_SA		(1 << 7)

#define UICR0_IM0		(1 << 0)

#define USIR0_IR0		(1 << 0)
#define USIR0_IR1		(1 << 1)
#define USIR0_IR2		(1 << 2)
#define USIR0_IR3		(1 << 3)
#define USIR0_IR4		(1 << 4)
#define USIR0_IR5		(1 << 5)
#define USIR0_IR6		(1 << 6)
#define USIR0_IR7		(1 << 7)

#define UDCCFR_AREN		(1 << 7) /* ACK response enable (now) */
#define UDCCFR_ACM		(1 << 2) /* ACK control mode (wait for AREN) */
/*
 * Intel(R) PXA255 Processor Specification, September 2003 (page 31)
 * define new "must be one" bits in UDCCFR (see Table 12-13.)
 */
#define UDCCFR_MB1		(0xff & ~(UDCCFR_AREN | UDCCFR_ACM))

#define UFNRH_SIR		(1 << 7)	/* SOF interrupt request */
#define UFNRH_SIM		(1 << 6)	/* SOF interrupt mask */
#define UFNRH_IPE14		(1 << 5)	/* ISO packet error, ep14 */
#define UFNRH_IPE9		(1 << 4)	/* ISO packet error, ep9 */
#define UFNRH_IPE4		(1 << 3)	/* ISO packet error, ep4 */

#endif /* __REGS_USB_H__ */
