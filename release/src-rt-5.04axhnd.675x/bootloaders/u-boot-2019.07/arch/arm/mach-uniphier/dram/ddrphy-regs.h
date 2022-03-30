/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * UniPhier DDR PHY registers
 *
 * Copyright (C) 2014      Panasonic Corporation
 * Copyright (C) 2015-2016 Socionext Inc.
 */

#ifndef ARCH_DDRPHY_REGS_H
#define ARCH_DDRPHY_REGS_H

#define PHY_REG_SHIFT		2

#define PHY_RIDR		(0x000 << PHY_REG_SHIFT)
#define PHY_PIR			(0x001 << PHY_REG_SHIFT)
#define   PHY_PIR_INIT			BIT(0)	/* Initialization Trigger */
#define   PHY_PIR_ZCAL			BIT(1)	/* Impedance Calibration */
#define   PHY_PIR_PLLINIT		BIT(4)	/* PLL Initialization */
#define   PHY_PIR_DCAL			BIT(5)	/* DDL Calibration */
#define   PHY_PIR_PHYRST		BIT(6)	/* PHY Reset */
#define   PHY_PIR_DRAMRST		BIT(7)	/* DRAM Reset */
#define   PHY_PIR_DRAMINIT		BIT(8)	/* DRAM Initialization */
#define   PHY_PIR_WL			BIT(9)	/* Write Leveling */
#define   PHY_PIR_QSGATE		BIT(10)	/* Read DQS Gate Training */
#define   PHY_PIR_WLADJ			BIT(11)	/* Write Leveling Adjust */
#define   PHY_PIR_RDDSKW		BIT(12)	/* Read Data Bit Deskew */
#define   PHY_PIR_WRDSKW		BIT(13)	/* Write Data Bit Deskew */
#define   PHY_PIR_RDEYE			BIT(14)	/* Read Data Eye Training */
#define   PHY_PIR_WREYE			BIT(15)	/* Write Data Eye Training */
#define   PHY_PIR_LOCKBYP		BIT(28)	/* PLL Lock Bypass */
#define   PHY_PIR_DCALBYP		BIT(29)	/* DDL Calibration Bypass */
#define   PHY_PIR_ZCALBYP		BIT(30)	/* Impedance Calib Bypass */
#define   PHY_PIR_INITBYP		BIT(31)	/* Initialization Bypass */
#define PHY_PGCR0		(0x002 << PHY_REG_SHIFT)
#define PHY_PGCR1		(0x003 << PHY_REG_SHIFT)
#define   PHY_PGCR1_INHVT		BIT(26)	/* VT Calculation Inhibit */
#define PHY_PGSR0		(0x004 << PHY_REG_SHIFT)
#define   PHY_PGSR0_IDONE		BIT(0)	/* Initialization Done */
#define   PHY_PGSR0_PLDONE		BIT(1)	/* PLL Lock Done */
#define   PHY_PGSR0_DCDONE		BIT(2)	/* DDL Calibration Done */
#define   PHY_PGSR0_ZCDONE		BIT(3)	/* Impedance Calibration Done */
#define   PHY_PGSR0_DIDONE		BIT(4)	/* DRAM Initialization Done */
#define   PHY_PGSR0_WLDONE		BIT(5)	/* Write Leveling Done */
#define   PHY_PGSR0_QSGDONE		BIT(6)	/* DQS Gate Training Done */
#define   PHY_PGSR0_WLADONE		BIT(7)	/* Write Leveling Adjust Done */
#define   PHY_PGSR0_RDDONE		BIT(8)	/* Read Bit Deskew Done */
#define   PHY_PGSR0_WDDONE		BIT(9)	/* Write Bit Deskew Done */
#define   PHY_PGSR0_REDONE		BIT(10)	/* Read Eye Training Done */
#define   PHY_PGSR0_WEDONE		BIT(11)	/* Write Eye Training Done */
#define   PHY_PGSR0_DIERR		BIT(20)	/* DRAM Initialization Error */
#define   PHY_PGSR0_WLERR		BIT(21)	/* Write Leveling Error */
#define   PHY_PGSR0_QSGERR		BIT(22)	/* DQS Gate Training Error */
#define   PHY_PGSR0_WLAERR		BIT(23)	/* Write Leveling Adj Error */
#define   PHY_PGSR0_RDERR		BIT(24)	/* Read Bit Deskew Error */
#define   PHY_PGSR0_WDERR		BIT(25)	/* Write Bit Deskew Error */
#define   PHY_PGSR0_REERR		BIT(26)	/* Read Eye Training Error */
#define   PHY_PGSR0_WEERR		BIT(27)	/* Write Eye Training Error */
#define   PHY_PGSR0_DTERR_SHIFT		28	/* Data Training Error Status*/
#define   PHY_PGSR0_DTERR		(7 << (PHY_PGSR0_DTERR_SHIFT))
#define PHY_PGSR1		(0x005 << PHY_REG_SHIFT)
#define   PHY_PGSR1_VTSTOP		BIT(30)	/* VT Stop (v3-) */
#define PHY_PLLCR		(0x006 << PHY_REG_SHIFT)
#define PHY_PTR0		(0x007 << PHY_REG_SHIFT)
#define PHY_PTR1		(0x008 << PHY_REG_SHIFT)
#define PHY_PTR2		(0x009 << PHY_REG_SHIFT)
#define PHY_PTR3		(0x00A << PHY_REG_SHIFT)
#define PHY_PTR4		(0x00B << PHY_REG_SHIFT)
#define PHY_ACMDLR		(0x00C << PHY_REG_SHIFT)
#define PHY_ACBDLR		(0x00D << PHY_REG_SHIFT)
#define PHY_ACIOCR		(0x00E << PHY_REG_SHIFT)
#define PHY_DXCCR		(0x00F << PHY_REG_SHIFT)
#define   PHY_DXCCR_DQSRES_OPEN		(0 << 5)
#define   PHY_DXCCR_DQSRES_688_OHM	(1 << 5)
#define   PHY_DXCCR_DQSRES_611_OHM	(2 << 5)
#define   PHY_DXCCR_DQSRES_550_OHM	(3 << 5)
#define   PHY_DXCCR_DQSRES_500_OHM	(4 << 5)
#define   PHY_DXCCR_DQSRES_458_OHM	(5 << 5)
#define   PHY_DXCCR_DQSRES_393_OHM	(6 << 5)
#define   PHY_DXCCR_DQSRES_344_OHM	(7 << 5)
#define   PHY_DXCCR_DQSNRES_OPEN	(0 << 9)
#define   PHY_DXCCR_DQSNRES_688_OHM	(1 << 9)
#define   PHY_DXCCR_DQSNRES_611_OHM	(2 << 9)
#define   PHY_DXCCR_DQSNRES_550_OHM	(3 << 9)
#define   PHY_DXCCR_DQSNRES_500_OHM	(4 << 9)
#define   PHY_DXCCR_DQSNRES_458_OHM	(5 << 9)
#define   PHY_DXCCR_DQSNRES_393_OHM	(6 << 9)
#define   PHY_DXCCR_DQSNRES_344_OHM	(7 << 9)
#define PHY_DSGCR		(0x010 << PHY_REG_SHIFT)
#define PHY_DCR			(0x011 << PHY_REG_SHIFT)
#define PHY_DTPR0		(0x012 << PHY_REG_SHIFT)
#define PHY_DTPR1		(0x013 << PHY_REG_SHIFT)
#define PHY_DTPR2		(0x014 << PHY_REG_SHIFT)
#define PHY_MR0			(0x015 << PHY_REG_SHIFT)
#define PHY_MR1			(0x016 << PHY_REG_SHIFT)
#define PHY_MR2			(0x017 << PHY_REG_SHIFT)
#define PHY_MR3			(0x018 << PHY_REG_SHIFT)
#define PHY_ODTCR		(0x019 << PHY_REG_SHIFT)
#define PHY_DTCR		(0x01A << PHY_REG_SHIFT)
#define   PHY_DTCR_DTRANK_SHIFT		4	/* Data Training Rank */
#define   PHY_DTCR_DTRANK_MASK		(0x3 << (PHY_DTCR_DTRANK_SHIFT))
#define   PHY_DTCR_DTMPR		BIT(6)	/* Data Training using MPR */
#define   PHY_DTCR_RANKEN_SHIFT		24	/* Rank Enable */
#define   PHY_DTCR_RANKEN_MASK		(0xf << (PHY_DTCR_RANKEN_SHIFT))
#define PHY_DTAR0		(0x01B << PHY_REG_SHIFT)
#define PHY_DTAR1		(0x01C << PHY_REG_SHIFT)
#define PHY_DTAR2		(0x01D << PHY_REG_SHIFT)
#define PHY_DTAR3		(0x01E << PHY_REG_SHIFT)
#define PHY_DTDR0		(0x01F << PHY_REG_SHIFT)
#define PHY_DTDR1		(0x020 << PHY_REG_SHIFT)
#define PHY_DTEDR0		(0x021 << PHY_REG_SHIFT)
#define PHY_DTEDR1		(0x022 << PHY_REG_SHIFT)
#define PHY_PGCR2		(0x023 << PHY_REG_SHIFT)
#define PHY_GPR0		(0x05E << PHY_REG_SHIFT)
#define PHY_GPR1		(0x05F << PHY_REG_SHIFT)
/* ZQ */
#define PHY_ZQ_BASE		(0x060 << PHY_REG_SHIFT)
#define PHY_ZQ_STRIDE		(0x004 << PHY_REG_SHIFT)
#define PHY_ZQ_CR0		(0x000 << PHY_REG_SHIFT)
#define PHY_ZQ_CR1		(0x001 << PHY_REG_SHIFT)
#define PHY_ZQ_SR0		(0x002 << PHY_REG_SHIFT)
#define PHY_ZQ_SR1		(0x003 << PHY_REG_SHIFT)
/* DATX8 */
#define PHY_DX_BASE		(0x070 << PHY_REG_SHIFT)
#define PHY_DX_STRIDE		(0x010 << PHY_REG_SHIFT)
#define PHY_DX_GCR		(0x000 << PHY_REG_SHIFT)
#define   PHY_DX_GCR_WLRKEN_SHIFT	26		/* Write Level Rank Enable */
#define   PHY_DX_GCR_WLRKEN_MASK	(0xf << (PHY_DX_GCR_WLRKEN_SHIFT))
#define PHY_DX_GSR0		(0x001 << PHY_REG_SHIFT)
#define PHY_DX_GSR1		(0x002 << PHY_REG_SHIFT)
#define PHY_DX_BDLR0		(0x003 << PHY_REG_SHIFT)
#define PHY_DX_BDLR1		(0x004 << PHY_REG_SHIFT)
#define PHY_DX_BDLR2		(0x005 << PHY_REG_SHIFT)
#define PHY_DX_BDLR3		(0x006 << PHY_REG_SHIFT)
#define PHY_DX_BDLR4		(0x007 << PHY_REG_SHIFT)
#define PHY_DX_LCDLR0		(0x008 << PHY_REG_SHIFT)
#define PHY_DX_LCDLR1		(0x009 << PHY_REG_SHIFT)
#define PHY_DX_LCDLR2		(0x00A << PHY_REG_SHIFT)
#define PHY_DX_MDLR		(0x00B << PHY_REG_SHIFT)
#define PHY_DX_GTR		(0x00C << PHY_REG_SHIFT)
#define PHY_DX_GSR2		(0x00D << PHY_REG_SHIFT)

#endif /* ARCH_DDRPHY_REGS_H */
