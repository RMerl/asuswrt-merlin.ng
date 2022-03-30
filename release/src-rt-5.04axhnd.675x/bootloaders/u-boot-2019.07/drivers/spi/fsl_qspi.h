/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2013-2014 Freescale Semiconductor, Inc.
 *
 * Register definitions for Freescale QSPI
 */

#ifndef _FSL_QSPI_H_
#define _FSL_QSPI_H_

struct fsl_qspi_regs {
	u32 mcr;
	u32 rsvd0[1];
	u32 ipcr;
	u32 flshcr;
	u32 buf0cr;
	u32 buf1cr;
	u32 buf2cr;
	u32 buf3cr;
	u32 bfgencr;
	u32 soccr;
	u32 rsvd1[2];
	u32 buf0ind;
	u32 buf1ind;
	u32 buf2ind;
	u32 rsvd2[49];
	u32 sfar;
	u32 rsvd3[1];
	u32 smpr;
	u32 rbsr;
	u32 rbct;
	u32 rsvd4[15];
	u32 tbsr;
	u32 tbdr;
	u32 rsvd5[1];
	u32 sr;
	u32 fr;
	u32 rser;
	u32 spndst;
	u32 sptrclr;
	u32 rsvd6[4];
	u32 sfa1ad;
	u32 sfa2ad;
	u32 sfb1ad;
	u32 sfb2ad;
	u32 rsvd7[28];
	u32 rbdr[32];
	u32 rsvd8[32];
	u32 lutkey;
	u32 lckcr;
	u32 rsvd9[2];
	u32 lut[64];
};

#define QSPI_IPCR_SEQID_SHIFT		24
#define QSPI_IPCR_SEQID_MASK		(0xf << QSPI_IPCR_SEQID_SHIFT)

#define QSPI_MCR_END_CFD_SHIFT		2
#define QSPI_MCR_END_CFD_MASK		(3 << QSPI_MCR_END_CFD_SHIFT)
#ifdef CONFIG_SYS_FSL_QSPI_AHB
/* AHB needs 64bit operation */
#define QSPI_MCR_END_CFD_LE		(3 << QSPI_MCR_END_CFD_SHIFT)
#else
#define QSPI_MCR_END_CFD_LE		(1 << QSPI_MCR_END_CFD_SHIFT)
#endif
#define QSPI_MCR_DDR_EN_SHIFT		7
#define QSPI_MCR_DDR_EN_MASK		(1 << QSPI_MCR_DDR_EN_SHIFT)
#define QSPI_MCR_CLR_RXF_SHIFT		10
#define QSPI_MCR_CLR_RXF_MASK		(1 << QSPI_MCR_CLR_RXF_SHIFT)
#define QSPI_MCR_CLR_TXF_SHIFT		11
#define QSPI_MCR_CLR_TXF_MASK		(1 << QSPI_MCR_CLR_TXF_SHIFT)
#define QSPI_MCR_MDIS_SHIFT		14
#define QSPI_MCR_MDIS_MASK		(1 << QSPI_MCR_MDIS_SHIFT)
#define QSPI_MCR_RESERVED_SHIFT		16
#define QSPI_MCR_RESERVED_MASK		(0xf << QSPI_MCR_RESERVED_SHIFT)
#define QSPI_MCR_SWRSTHD_SHIFT		1
#define QSPI_MCR_SWRSTHD_MASK		(1 << QSPI_MCR_SWRSTHD_SHIFT)
#define QSPI_MCR_SWRSTSD_SHIFT		0
#define QSPI_MCR_SWRSTSD_MASK		(1 << QSPI_MCR_SWRSTSD_SHIFT)

#define QSPI_SMPR_HSENA_SHIFT		0
#define QSPI_SMPR_HSENA_MASK		(1 << QSPI_SMPR_HSENA_SHIFT)
#define QSPI_SMPR_FSPHS_SHIFT		5
#define QSPI_SMPR_FSPHS_MASK		(1 << QSPI_SMPR_FSPHS_SHIFT)
#define QSPI_SMPR_FSDLY_SHIFT		6
#define QSPI_SMPR_FSDLY_MASK		(1 << QSPI_SMPR_FSDLY_SHIFT)
#define QSPI_SMPR_DDRSMP_SHIFT		16
#define QSPI_SMPR_DDRSMP_MASK		(7 << QSPI_SMPR_DDRSMP_SHIFT)

#define QSPI_BUFXCR_INVALID_MSTRID	0xe
#define QSPI_BUF3CR_ALLMST_SHIFT	31
#define QSPI_BUF3CR_ALLMST_MASK		(1 << QSPI_BUF3CR_ALLMST_SHIFT)
#define QSPI_BUF3CR_ADATSZ_SHIFT	8
#define QSPI_BUF3CR_ADATSZ_MASK		(0xFF << QSPI_BUF3CR_ADATSZ_SHIFT)

#define QSPI_BFGENCR_SEQID_SHIFT	12
#define QSPI_BFGENCR_SEQID_MASK		(0xf << QSPI_BFGENCR_SEQID_SHIFT)
#define QSPI_BFGENCR_PAR_EN_SHIFT	16
#define QSPI_BFGENCR_PAR_EN_MASK	(1 << QSPI_BFGENCR_PAR_EN_SHIFT)

#define QSPI_RBSR_RDBFL_SHIFT		8
#define QSPI_RBSR_RDBFL_MASK		(0x3f << QSPI_RBSR_RDBFL_SHIFT)

#define QSPI_RBCT_RXBRD_SHIFT		8
#define QSPI_RBCT_RXBRD_USEIPS		(1 << QSPI_RBCT_RXBRD_SHIFT)

#define QSPI_SR_AHB_ACC_SHIFT		2
#define QSPI_SR_AHB_ACC_MASK		(1 << QSPI_SR_AHB_ACC_SHIFT)
#define QSPI_SR_IP_ACC_SHIFT		1
#define QSPI_SR_IP_ACC_MASK		(1 << QSPI_SR_IP_ACC_SHIFT)
#define QSPI_SR_BUSY_SHIFT		0
#define QSPI_SR_BUSY_MASK		(1 << QSPI_SR_BUSY_SHIFT)

#define QSPI_LCKCR_LOCK			0x1
#define QSPI_LCKCR_UNLOCK		0x2

#define LUT_KEY_VALUE			0x5af05af0

#define OPRND0_SHIFT			0
#define OPRND0(x)			((x) << OPRND0_SHIFT)
#define PAD0_SHIFT			8
#define PAD0(x)				((x) << PAD0_SHIFT)
#define INSTR0_SHIFT			10
#define INSTR0(x)			((x) << INSTR0_SHIFT)
#define OPRND1_SHIFT			16
#define OPRND1(x)			((x) << OPRND1_SHIFT)
#define PAD1_SHIFT			24
#define PAD1(x)				((x) << PAD1_SHIFT)
#define INSTR1_SHIFT			26
#define INSTR1(x)			((x) << INSTR1_SHIFT)

#define LUT_CMD				1
#define LUT_ADDR			2
#define LUT_DUMMY			3
#define LUT_READ			7
#define LUT_WRITE			8

#define LUT_PAD1			0
#define LUT_PAD2			1
#define LUT_PAD4			2

#define ADDR24BIT			0x18
#define ADDR32BIT			0x20

#endif /* _FSL_QSPI_H_ */
