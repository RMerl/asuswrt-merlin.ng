/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
 *
 * (C) Copyright 2011 Andes Technology Corp
 * Macpaul Lin <macpaul@andestech.com>
 */

/*
 * FTSDMC021 - SDRAM Controller
 */
#ifndef __FTSDMC021_H
#define __FTSDMC021_H

#ifndef __ASSEMBLY__
struct ftsdmc021 {
	unsigned int	tp1;		/* 0x00 - SDRAM Timing Parameter 1 */
	unsigned int	tp2;		/* 0x04 - SDRAM Timing Parameter 2 */
	unsigned int	cr1;		/* 0x08 - SDRAM Configuration Reg 1 */
	unsigned int	cr2;		/* 0x0c - SDRAM Configuration Reg 2 */
	unsigned int	bank0_bsr;	/* 0x10 - Ext. Bank Base/Size Reg 0 */
	unsigned int	bank1_bsr;	/* 0x14 - Ext. Bank Base/Size Reg 1 */
	unsigned int	bank2_bsr;	/* 0x18 - Ext. Bank Base/Size Reg 2 */
	unsigned int	bank3_bsr;	/* 0x1c - Ext. Bank Base/Size Reg 3 */
	unsigned int	bank4_bsr;	/* 0x20 - Ext. Bank Base/Size Reg 4 */
	unsigned int	bank5_bsr;	/* 0x24 - Ext. Bank Base/Size Reg 5 */
	unsigned int	bank6_bsr;	/* 0x28 - Ext. Bank Base/Size Reg 6 */
	unsigned int	bank7_bsr;	/* 0x2c - Ext. Bank Base/Size Reg 7 */
	unsigned int	ragr;		/* 0x30 - Read Arbitration Group Reg */
	unsigned int	frr;		/* 0x34 - Flush Request Register */
	unsigned int	ebisr;		/* 0x38 - EBI Support Register	*/
	unsigned int	rsved[25];	/* 0x3c-0x9c - Reserved		*/
	unsigned int	crr;		/* 0x100 - Controller Revision Reg */
	unsigned int	cfr;		/* 0x104 - Controller Feature Reg */
};
#endif /* __ASSEMBLY__ */

/*
 * Timing Parameter 1 Register
 */
#define FTSDMC021_TP1_TCL(x)	((x) & 0x3)		/* CAS Latency */
#define FTSDMC021_TP1_TWR(x)	(((x) & 0x3) << 4)	/* W-Recovery Time */
#define FTSDMC021_TP1_TRF(x)	(((x) & 0xf) << 8)	/* Auto-Refresh Cycle */
#define FTSDMC021_TP1_TRCD(x)	(((x) & 0x7) << 12)	/* RAS-to-CAS Delay */
#define FTSDMC021_TP1_TRP(x)	(((x) & 0xf) << 16)	/* Precharge Cycle */
#define FTSDMC021_TP1_TRAS(x)	(((x) & 0xf) << 20)

/*
 * Timing Parameter 2 Register
 */
#define FTSDMC021_TP2_REF_INTV(x)	((x) & 0xffff)	/* Refresh interval */
/* b(16:19) - Initial Refresh Times */
#define FTSDMC021_TP2_INI_REFT(x)	(((x) & 0xf) << 16)
/* b(20:23) - Initial Pre-Charge Times */
#define FTSDMC021_TP2_INI_PREC(x)	(((x) & 0xf) << 20)

/*
 * SDRAM Configuration Register 1
 */
#define FTSDMC021_CR1_BNKSIZE(x)	((x) & 0xf)		/* Bank Size  */
#define FTSDMC021_CR1_MBW(x)		(((x) & 0x3) << 4)	/* Bus Width  */
#define FTSDMC021_CR1_DSZ(x)		(((x) & 0x7) << 8)	/* SDRAM Size */
#define FTSDMC021_CR1_DDW(x)		(((x) & 0x3) << 12)	/* Data Width */
/* b(16) MA2T: Double Memory Address Cycle Enable */
#define FTSDMC021_CR1_MA2T(x)		(1 << 16)
/* The value of b(0:3)CR1: 1M-512M, must be power of 2 */
#define FTSDMC021_BANK_SIZE(x)		(ffs(x) - 1)

/*
 * Configuration Register 2
 */
#define FTSDMC021_CR2_SREF	(1 << 0)	/* Self-Refresh Mode */
#define FTSDMC021_CR2_PWDN	(1 << 1)	/* Power Down Operation Mode */
#define FTSDMC021_CR2_ISMR	(1 << 2)	/* Start Set-Mode-Register */
#define FTSDMC021_CR2_IREF	(1 << 3)	/* Init Refresh Start Flag */
#define FTSDMC021_CR2_IPREC	(1 << 4)	/* Init Pre-Charge Start Flag */
#define FTSDMC021_CR2_REFTYPE	(1 << 5)

/*
 * SDRAM External Bank Base/Size Register
 */
#define FTSDMC021_BANK_ENABLE		(1 << 12)

/* 12-bit base address of external bank.
 * Default value is 0x800.
 * The 12-bit equals to the haddr[31:20] of AHB address bus. */
#define FTSDMC021_BANK_BASE(x)		((x) & 0xfff)

/*
 * Read Arbitration Grant Window Register
 */
#define FTSDMC021_RAGR_CH1GW(x)		(((x) & 0xff) << 0)
#define FTSDMC021_RAGR_CH2GW(x)		(((x) & 0xff) << 4)
#define FTSDMC021_RAGR_CH3GW(x)		(((x) & 0xff) << 8)
#define FTSDMC021_RAGR_CH4GW(x)		(((x) & 0xff) << 12)
#define FTSDMC021_RAGR_CH5GW(x)		(((x) & 0xff) << 16)
#define FTSDMC021_RAGR_CH6GW(x)		(((x) & 0xff) << 20)
#define FTSDMC021_RAGR_CH7GW(x)		(((x) & 0xff) << 24)
#define FTSDMC021_RAGR_CH8GW(x)		(((x) & 0xff) << 28)

/*
 * Flush Request Register
 */
#define FTSDMC021_FRR_FLUSHCHN(x)	(((x) & 0x7) << 0)
#define FTSDMC021_FRR_FLUSHCMPLT	(1 << 3)	/* Flush Req Flag */

/*
 * External Bus Interface Support Register (EBISR)
 */
#define FTSDMC021_EBISR_MR(x)		((x) & 0xfff)	/* Far-end mode	*/
#define FTSDMC021_EBISR_PRSMR		(1 << 12)	/* Pre-SMR	*/
#define FTSDMC021_EBISR_POPREC		(1 << 13)
#define FTSDMC021_EBISR_POSMR		(1 << 14)	/* Post-SMR	*/

/*
 * Controller Revision Register (CRR, Read Only)
 */
#define FTSDMC021_CRR_REV_VER		(((x) >> 0) & 0xff)
#define FTSDMC021_CRR_MINOR_VER		(((x) >> 8) & 0xff)
#define FTSDMC021_CRR_MAJOR_VER		(((x) >> 16) & 0xff)

/*
 * Controller Feature Register (CFR, Read Only)
 */
#define FTSDMC021_CFR_EBNK		(((x) >> 0) & 0xf)
#define FTSDMC021_CFR_CHN		(((x) >> 8) & 0xf)
#define FTSDMC021_CFR_EBI		(((x) >> 16) & 0x1)
#define FTSDMC021_CFR_CH1_FDEPTH	(((x) >> 24) & 0x1)
#define FTSDMC021_CFR_CH2_FDEPTH	(((x) >> 25) & 0x1)
#define FTSDMC021_CFR_CH3_FDEPTH	(((x) >> 26) & 0x1)
#define FTSDMC021_CFR_CH4_FDEPTH	(((x) >> 27) & 0x1)
#define FTSDMC021_CFR_CH5_FDEPTH	(((x) >> 28) & 0x1)
#define FTSDMC021_CFR_CH6_FDEPTH	(((x) >> 29) & 0x1)
#define FTSDMC021_CFR_CH7_FDEPTH	(((x) >> 30) & 0x1)
#define FTSDMC021_CFR_CH8_FDEPTH	(((x) >> 31) & 0x1)

#endif	/* __FTSDMC021_H */
