/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011 Andes Technology Corp
 * Macpaul Lin <macpaul@andestech.com>
 */

/*
 * DWCDDR21MCTL - Synopsys DWC DDR2/DDR1 Memory Controller
 */
#ifndef __DWCDDR21MCTL_H
#define __DWCDDR21MCTL_H

#ifndef __ASSEMBLY__
struct dwcddr21mctl {
	unsigned int	ccr;		/* Controller Configuration */
	unsigned int	dcr;		/* DRAM Configuration */
	unsigned int	iocr;		/* I/O Configuration */
	unsigned int	csr;		/* Controller Status */
	unsigned int	drr;		/* DRAM refresh */
	unsigned int	tpr0;		/* SDRAM Timing Parameters 0 */
	unsigned int	tpr1;		/* SDRAM Timing Parameters 1 */
	unsigned int	tpr2;		/* SDRAM Timing Parameters 2 */
	unsigned int	gdllcr;		/* Global DLL Control */
	unsigned int	dllcr[10];	/* DLL Control */
	unsigned int	rslr[4];	/* Rank System Lantency */
	unsigned int	rdgr[4];	/* Rank DQS Gating */
	unsigned int	dqtr[9];	/* DQ Timing */
	unsigned int	dqstr;		/* DQS Timing */
	unsigned int	dqsbtr;		/* DQS_b Timing */
	unsigned int	odtcr;		/* ODT Configuration */
	unsigned int	dtr[2];		/* Data Training */
	unsigned int	dtar;		/* Data Training Address */
	unsigned int	rsved[82];	/* Reserved */
	unsigned int	mr;		/* Mode Register */
	unsigned int	emr;		/* Extended Mode Register */
	unsigned int	emr2;		/* Extended Mode Register 2 */
	unsigned int	emr3;		/* Extended Mode Register 3 */
	unsigned int	hpcr[32];	/* Host Port Configurarion */
	unsigned int	pqcr[8];	/* Priority Queue Configuration */
	unsigned int	mmgcr;		/* Memory Manager General Config */
};
#endif /* __ASSEMBLY__ */

/*
 * Control Configuration Register
 */
#define DWCDDR21MCTL_CCR_ECCEN(x)	((x) << 0)
#define DWCDDR21MCTL_CCR_NOMRWR(x)	((x) << 1)
#define DWCDDR21MCTL_CCR_HOSTEN(x)	((x) << 2)
#define DWCDDR21MCTL_CCR_XBISC(x)	((x) << 3)
#define DWCDDR21MCTL_CCR_NOAPD(x)	((x) << 4)
#define DWCDDR21MCTL_CCR_RRB(x)		((x) << 13)
#define DWCDDR21MCTL_CCR_DQSCFG(x)	((x) << 14)
#define DWCDDR21MCTL_CCR_DFTLM(x)	(((x) & 0x3) << 15)
#define DWCDDR21MCTL_CCR_DFTCMP(x)	((x) << 17)
#define DWCDDR21MCTL_CCR_FLUSH(x)	((x) << 27)
#define DWCDDR21MCTL_CCR_ITMRST(x)	((x) << 28)
#define DWCDDR21MCTL_CCR_IB(x)		((x) << 29)
#define DWCDDR21MCTL_CCR_DTT(x)		((x) << 30)
#define DWCDDR21MCTL_CCR_IT(x)		((x) << 31)

/*
 * DRAM Configuration Register
 */
#define DWCDDR21MCTL_DCR_DDRMD(x)	((x) << 0)
#define DWCDDR21MCTL_DCR_DIO(x)		(((x) & 0x3) << 1)
#define DWCDDR21MCTL_DCR_DSIZE(x)	(((x) & 0x7) << 3)
#define DWCDDR21MCTL_DCR_SIO(x)		(((x) & 0x7) << 6)
#define DWCDDR21MCTL_DCR_PIO(x)		((x) << 9)
#define DWCDDR21MCTL_DCR_RANKS(x)	(((x) & 0x3) << 10)
#define DWCDDR21MCTL_DCR_RNKALL(x)	((x) << 12)
#define DWCDDR21MCTL_DCR_AMAP(x)	(((x) & 0x3) << 13)
#define DWCDDR21MCTL_DCR_RANK(x)	(((x) & 0x3) << 25)
#define DWCDDR21MCTL_DCR_CMD(x)		(((x) & 0xf) << 27)
#define DWCDDR21MCTL_DCR_EXE(x)		((x) << 31)

/*
 * I/O Configuration Register
 */
#define DWCDDR21MCTL_IOCR_RTT(x)	(((x) & 0xf) << 0)
#define DWCDDR21MCTL_IOCR_DS(x)		(((x) & 0xf) << 4)
#define DWCDDR21MCTL_IOCR_TESTEN(x)	((x) << 0x8)
#define DWCDDR21MCTL_IOCR_RTTOH(x)	(((x) & 0x7) << 26)
#define DWCDDR21MCTL_IOCR_RTTOE(x)	((x) << 29)
#define DWCDDR21MCTL_IOCR_DQRTT(x)	((x) << 30)
#define DWCDDR21MCTL_IOCR_DQSRTT(x)	((x) << 31)

/*
 * Controller Status Register
 */
#define DWCDDR21MCTL_CSR_DRIFT(x)	(((x) & 0x3ff) << 0)
#define DWCDDR21MCTL_CSR_DFTERR(x)	((x) << 18)
#define DWCDDR21MCTL_CSR_ECCERR(x)	((x) << 19)
#define DWCDDR21MCTL_CSR_DTERR(x)	((x) << 20)
#define DWCDDR21MCTL_CSR_DTIERR(x)	((x) << 21)
#define DWCDDR21MCTL_CSR_ECCSEC(x)	((x) << 22)

/*
 * DRAM Refresh Register
 */
#define DWCDDR21MCTL_DRR_TRFC(x)	(((x) & 0xff) << 0)
#define DWCDDR21MCTL_DRR_TRFPRD(x)	(((x) & 0xffff) << 8)
#define DWCDDR21MCTL_DRR_RFBURST(x)	(((x) & 0xf) << 24)
#define DWCDDR21MCTL_DRR_RD(x)		((x) << 31)

/*
 * SDRAM Timing Parameters Register 0
 */
#define DWCDDR21MCTL_TPR0_TMRD(x)	(((x) & 0x3) << 0)
#define DWCDDR21MCTL_TPR0_TRTP(x)	(((x) & 0x7) << 2)
#define DWCDDR21MCTL_TPR0_TWTR(x)	(((x) & 0x7) << 5)
#define DWCDDR21MCTL_TPR0_TRP(x)	(((x) & 0xf) << 8)
#define DWCDDR21MCTL_TPR0_TRCD(x)	(((x) & 0xf) << 12)
#define DWCDDR21MCTL_TPR0_TRAS(x)	(((x) & 0x1f) << 16)
#define DWCDDR21MCTL_TPR0_TRRD(x)	(((x) & 0xf) << 21)
#define DWCDDR21MCTL_TPR0_TRC(x)	(((x) & 0x3f) << 25)
#define DWCDDR21MCTL_TPR0_TCCD(x)	((x) << 31)

/*
 * SDRAM Timing Parameters Register 1
 */
#define DWCDDR21MCTL_TPR1_TAOND(x)	(((x) & 0x3) << 0)
#define DWCDDR21MCTL_TPR1_TRTW(x)	((x) << 2)
#define DWCDDR21MCTL_TPR1_TFAW(x)	(((x) & 0x3f) << 3)
#define DWCDDR21MCTL_TPR1_TRNKRTR(x)	(((x) & 0x3) << 12)
#define DWCDDR21MCTL_TPR1_TRNKWTW(x)	(((x) & 0x3) << 14)
#define DWCDDR21MCTL_TPR1_XCL(x)	(((x) & 0xf) << 23)
#define DWCDDR21MCTL_TPR1_XWR(x)	(((x) & 0xf) << 27)
#define DWCDDR21MCTL_TPR1_XTP(x)	((x) << 31)

/*
 * SDRAM Timing Parameters Register 2
 */
#define DWCDDR21MCTL_TPR2_TXS(x)	(((x) & 0x3ff) << 0)
#define DWCDDR21MCTL_TPR2_TXP(x)	(((x) & 0x1f) << 10)
#define DWCDDR21MCTL_TPR2_TCKE(x)	(((x) & 0xf) << 15)

/*
 * Global DLL Control Register
 */
#define DWCDDR21MCTL_GDLLCR_DRES(x)	(((x) & 0x3) << 0)
#define DWCDDR21MCTL_GDLLCR_IPUMP(x)	(((x) & 0x7) << 2)
#define DWCDDR21MCTL_GDLLCR_TESTEN(x)	((x) << 5)
#define DWCDDR21MCTL_GDLLCR_DTC(x)	(((x) & 0x7) << 6)
#define DWCDDR21MCTL_GDLLCR_ATC(x)	(((x) & 0x3) << 9)
#define DWCDDR21MCTL_GDLLCR_TESTSW(x)	((x) << 11)
#define DWCDDR21MCTL_GDLLCR_MBIAS(x)	(((x) & 0xff) << 12)
#define DWCDDR21MCTL_GDLLCR_SBIAS(x)	(((x) & 0xff) << 20)
#define DWCDDR21MCTL_GDLLCR_LOCKDET(x)	((x) << 29)

/*
 * DLL Control Register 0-9
 */
#define DWCDDR21MCTL_DLLCR_SFBDLY(x)	(((x) & 0x7) << 0)
#define DWCDDR21MCTL_DLLCR_SFWDLY(x)	(((x) & 0x7) << 3)
#define DWCDDR21MCTL_DLLCR_MFBDLY(x)	(((x) & 0x7) << 6)
#define DWCDDR21MCTL_DLLCR_MFWDLY(x)	(((x) & 0x7) << 9)
#define DWCDDR21MCTL_DLLCR_SSTART(x)	(((x) & 0x3) << 12)
#define DWCDDR21MCTL_DLLCR_PHASE(x)	(((x) & 0xf) << 14)
#define DWCDDR21MCTL_DLLCR_ATESTEN(x)	((x) << 18)
#define DWCDDR21MCTL_DLLCR_DRSVD(x)	((x) << 19)
#define DWCDDR21MCTL_DLLCR_DD(x)	((x) << 31)

/*
 * Rank System Lantency Register
 */
#define DWCDDR21MCTL_RSLR_SL0(x)	(((x) & 0x7) << 0)
#define DWCDDR21MCTL_RSLR_SL1(x)	(((x) & 0x7) << 3)
#define DWCDDR21MCTL_RSLR_SL2(x)	(((x) & 0x7) << 6)
#define DWCDDR21MCTL_RSLR_SL3(x)	(((x) & 0x7) << 9)
#define DWCDDR21MCTL_RSLR_SL4(x)	(((x) & 0x7) << 12)
#define DWCDDR21MCTL_RSLR_SL5(x)	(((x) & 0x7) << 15)
#define DWCDDR21MCTL_RSLR_SL6(x)	(((x) & 0x7) << 18)
#define DWCDDR21MCTL_RSLR_SL7(x)	(((x) & 0x7) << 21)
#define DWCDDR21MCTL_RSLR_SL8(x)	(((x) & 0x7) << 24)

/*
 * Rank DQS Gating Register
 */
#define DWCDDR21MCTL_RDGR_DQSSEL0(x)	(((x) & 0x3) << 0)
#define DWCDDR21MCTL_RDGR_DQSSEL1(x)	(((x) & 0x3) << 2)
#define DWCDDR21MCTL_RDGR_DQSSEL2(x)	(((x) & 0x3) << 4)
#define DWCDDR21MCTL_RDGR_DQSSEL3(x)	(((x) & 0x3) << 6)
#define DWCDDR21MCTL_RDGR_DQSSEL4(x)	(((x) & 0x3) << 8)
#define DWCDDR21MCTL_RDGR_DQSSEL5(x)	(((x) & 0x3) << 10)
#define DWCDDR21MCTL_RDGR_DQSSEL6(x)	(((x) & 0x3) << 12)
#define DWCDDR21MCTL_RDGR_DQSSEL7(x)	(((x) & 0x3) << 14)
#define DWCDDR21MCTL_RDGR_DQSSEL8(x)	(((x) & 0x3) << 16)

/*
 * DQ Timing Register
 */
#define DWCDDR21MCTL_DQTR_DQDLY0(x)	(((x) & 0xf) << 0)
#define DWCDDR21MCTL_DQTR_DQDLY1(x)	(((x) & 0xf) << 4)
#define DWCDDR21MCTL_DQTR_DQDLY2(x)	(((x) & 0xf) << 8)
#define DWCDDR21MCTL_DQTR_DQDLY3(x)	(((x) & 0xf) << 12)
#define DWCDDR21MCTL_DQTR_DQDLY4(x)	(((x) & 0xf) << 16)
#define DWCDDR21MCTL_DQTR_DQDLY5(x)	(((x) & 0xf) << 20)
#define DWCDDR21MCTL_DQTR_DQDLY6(x)	(((x) & 0xf) << 24)
#define DWCDDR21MCTL_DQTR_DQDLY7(x)	(((x) & 0xf) << 28)

/*
 * DQS Timing Register
 */
#define DWCDDR21MCTL_DQSTR_DQSDLY0(x)	(((x) & 0x7) << 0)
#define DWCDDR21MCTL_DQSTR_DQSDLY1(x)	(((x) & 0x7) << 3)
#define DWCDDR21MCTL_DQSTR_DQSDLY2(x)	(((x) & 0x7) << 6)
#define DWCDDR21MCTL_DQSTR_DQSDLY3(x)	(((x) & 0x7) << 9)
#define DWCDDR21MCTL_DQSTR_DQSDLY4(x)	(((x) & 0x7) << 12)
#define DWCDDR21MCTL_DQSTR_DQSDLY5(x)	(((x) & 0x7) << 15)
#define DWCDDR21MCTL_DQSTR_DQSDLY6(x)	(((x) & 0x7) << 18)
#define DWCDDR21MCTL_DQSTR_DQSDLY7(x)	(((x) & 0x7) << 21)
#define DWCDDR21MCTL_DQSTR_DQSDLY8(x)	(((x) & 0x7) << 24)

/*
 * DQS_b (DQSBTR) Timing Register
 */
#define DWCDDR21MCTL_DQSBTR_DQSDLY0(x)	(((x) & 0x7) << 0)
#define DWCDDR21MCTL_DQSBTR_DQSDLY1(x)	(((x) & 0x7) << 3)
#define DWCDDR21MCTL_DQSBTR_DQSDLY2(x)	(((x) & 0x7) << 6)
#define DWCDDR21MCTL_DQSBTR_DQSDLY3(x)	(((x) & 0x7) << 9)
#define DWCDDR21MCTL_DQSBTR_DQSDLY4(x)	(((x) & 0x7) << 12)
#define DWCDDR21MCTL_DQSBTR_DQSDLY5(x)	(((x) & 0x7) << 15)
#define DWCDDR21MCTL_DQSBTR_DQSDLY6(x)	(((x) & 0x7) << 18)
#define DWCDDR21MCTL_DQSBTR_DQSDLY7(x)	(((x) & 0x7) << 21)
#define DWCDDR21MCTL_DQSBTR_DQSDLY8(x)	(((x) & 0x7) << 24)

/*
 * ODT Configuration Register
 */
#define DWCDDR21MCTL_ODTCR_RDODT0(x)	(((x) & 0xf) << 0)
#define DWCDDR21MCTL_ODTCR_RDODT1(x)	(((x) & 0xf) << 4)
#define DWCDDR21MCTL_ODTCR_RDODT2(x)	(((x) & 0xf) << 8)
#define DWCDDR21MCTL_ODTCR_RDODT3(x)	(((x) & 0xf) << 12)
#define DWCDDR21MCTL_ODTCR_WDODT0(x)	(((x) & 0xf) << 16)
#define DWCDDR21MCTL_ODTCR_WDODT1(x)	(((x) & 0xf) << 20)
#define DWCDDR21MCTL_ODTCR_WDODT2(x)	(((x) & 0xf) << 24)
#define DWCDDR21MCTL_ODTCR_WDODT3(x)	(((x) & 0xf) << 28)

/*
 * Data Training Register
 */
#define DWCDDR21MCTL_DTR0_DTBYTE0(x)	(((x) & 0xff) << 0)	/* def: 0x11 */
#define DWCDDR21MCTL_DTR0_DTBYTE1(x)	(((x) & 0xff) << 8)	/* def: 0xee */
#define DWCDDR21MCTL_DTR0_DTBYTE2(x)	(((x) & 0xff) << 16)	/* def: 0x22 */
#define DWCDDR21MCTL_DTR0_DTBYTE3(x)	(((x) & 0xff) << 24)	/* def: 0xdd */

#define DWCDDR21MCTL_DTR1_DTBYTE4(x)	(((x) & 0xff) << 0)	/* def: 0x44 */
#define DWCDDR21MCTL_DTR1_DTBYTE5(x)	(((x) & 0xff) << 8)	/* def: 0xbb */
#define DWCDDR21MCTL_DTR1_DTBYTE6(x)	(((x) & 0xff) << 16)	/* def: 0x88 */
#define DWCDDR21MCTL_DTR1_DTBYTE7(x)	(((x) & 0xff) << 24)	/* def: 0x77 */

/*
 * Data Training Address Register
 */
#define DWCDDR21MCTL_DTAR_DTCOL(x)	(((x) & 0xfff) << 0)
#define DWCDDR21MCTL_DTAR_DTROW(x)	(((x) & 0xffff) << 12)
#define DWCDDR21MCTL_DTAR_DTBANK(x)	(((x) & 0x7) << 28)

/*
 * Mode Register
 */
#define DWCDDR21MCTL_MR_BL(x)		(((x) & 0x7) << 0)
#define DWCDDR21MCTL_MR_BT(x)		((x) << 3)
#define DWCDDR21MCTL_MR_CL(x)		(((x) & 0x7) << 4)
#define DWCDDR21MCTL_MR_TM(x)		((x) << 7)
#define DWCDDR21MCTL_MR_DR(x)		((x) << 8)
#define DWCDDR21MCTL_MR_WR(x)		(((x) & 0x7) << 9)
#define DWCDDR21MCTL_MR_PD(x)		((x) << 12)

/*
 * Extended Mode register
 */
#define DWCDDR21MCTL_EMR_DE(x)		((x) << 0)
#define DWCDDR21MCTL_EMR_ODS(x)		((x) << 1)
#define DWCDDR21MCTL_EMR_RTT2(x)	((x) << 2)
#define DWCDDR21MCTL_EMR_AL(x)		(((x) & 0x7) << 3)
#define DWCDDR21MCTL_EMR_RTT6(x)	((x) << 6)
#define DWCDDR21MCTL_EMR_OCD(x)		(((x) & 0x7) << 7)
#define DWCDDR21MCTL_EMR_DQS(x)		((x) << 10)
#define DWCDDR21MCTL_EMR_RDQS(x)	((x) << 11)
#define DWCDDR21MCTL_EMR_OE(x)		((x) << 12)

#define EMR_RTT2(x)			DWCDDR21MCTL_EMR_RTT2(x)
#define EMR_RTT6(x)			DWCDDR21MCTL_EMR_RTT6(x)

#define DWCDDR21MCTL_EMR_RTT_DISABLED	(EMR_RTT6(0) | EMR_RTT2(0))
#define DWCDDR21MCTL_EMR_RTT_75		(EMR_RTT6(0) | EMR_RTT2(1))
#define DWCDDR21MCTL_EMR_RTT_150	(EMR_RTT6(1) | EMR_RTT2(0))
#define DWCDDR21MCTL_EMR_RTT_50		(EMR_RTT6(1) | EMR_RTT2(1))

/*
 * Extended Mode register 2
 */
#define DWCDDR21MCTL_EMR2_PASR(x)	(((x) & 0x7) << 0)
#define DWCDDR21MCTL_EMR2_DCC(x)	((x) << 3)
#define DWCDDR21MCTL_EMR2_SRF(x)	((x) << 7)

/*
 * Extended Mode register 3: [15:0] reserved for JEDEC.
 */

/*
 * Host port Configuration register 0-31
 */
#define DWCDDR21MCTL_HPCR_HPBL(x)	(((x) & 0xf) << 0)

/*
 * Priority Queue Configuration register 0-7
 */
#define DWCDDR21MCTL_HPCR_TOUT(x)	(((x) & 0xf) << 0)
#define DWCDDR21MCTL_HPCR_TOUTX(x)	(((x) & 0x3) << 8)
#define DWCDDR21MCTL_HPCR_LPQS(x)	(((x) & 0x3) << 10)
#define DWCDDR21MCTL_HPCR_PQBL(x)	(((x) & 0xff) << 12)
#define DWCDDR21MCTL_HPCR_SWAIT(x)	(((x) & 0x1f) << 20)
#define DWCDDR21MCTL_HPCR_INTRPT(x)	(((x) & 0x7) << 25)
#define DWCDDR21MCTL_HPCR_APQS(x)	((x) << 28)

/*
 * Memory Manager General Configuration register
 */
#define DWCDDR21MCTL_MMGCR_UHPP(x)	(((x) & 0x3) << 0)

#endif	/* __DWCDDR21MCTL_H */
