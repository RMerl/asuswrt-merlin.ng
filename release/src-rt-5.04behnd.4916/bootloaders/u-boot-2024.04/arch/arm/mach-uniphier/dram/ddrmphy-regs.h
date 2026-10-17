/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * UniPhier DDR MultiPHY registers
 *
 * Copyright (C) 2015-2017 Socionext Inc.
 */

#ifndef UNIPHIER_DDRMPHY_REGS_H
#define UNIPHIER_DDRMPHY_REGS_H

#include <linux/bitops.h>

#define MPHY_SHIFT			2

#define MPHY_RIDR		(0x000 << MPHY_SHIFT)
#define MPHY_PIR		(0x001 << MPHY_SHIFT)
#define   MPHY_PIR_INIT			BIT(0)	/* Initialization Trigger */
#define   MPHY_PIR_ZCAL			BIT(1)	/* Impedance Calibration */
#define   MPHY_PIR_PLLINIT		BIT(4)	/* PLL Initialization */
#define   MPHY_PIR_DCAL			BIT(5)	/* DDL Calibration */
#define   MPHY_PIR_PHYRST		BIT(6)	/* PHY Reset */
#define   MPHY_PIR_DRAMRST		BIT(7)	/* DRAM Reset */
#define   MPHY_PIR_DRAMINIT		BIT(8)	/* DRAM Initialization */
#define   MPHY_PIR_WL			BIT(9)	/* Write Leveling */
#define   MPHY_PIR_QSGATE		BIT(10)	/* Read DQS Gate Training */
#define   MPHY_PIR_WLADJ		BIT(11)	/* Write Leveling Adjust */
#define   MPHY_PIR_RDDSKW		BIT(12)	/* Read Data Bit Deskew */
#define   MPHY_PIR_WRDSKW		BIT(13)	/* Write Data Bit Deskew */
#define   MPHY_PIR_RDEYE		BIT(14)	/* Read Data Eye Training */
#define   MPHY_PIR_WREYE		BIT(15)	/* Write Data Eye Training */
#define   MPHY_PIR_ZCALBYP		BIT(30)	/* Impedance Calib Bypass */
#define   MPHY_PIR_INITBYP		BIT(31)	/* Initialization Bypass */
#define MPHY_PGCR0		(0x002 << MPHY_SHIFT)
#define   MPHY_PGCR0_PHYFRST		BIT(26)	/* PHY FIFO Reset */
#define MPHY_PGCR1		(0x003 << MPHY_SHIFT)
#define   MPHY_PGCR1_INHVT		BIT(26)	/* VT Calculation Inhibit */
#define MPHY_PGCR2		(0x004 << MPHY_SHIFT)
#define   MPHY_PGCR2_DUALCHN		BIT(28)	/* Dual Channel Configuration*/
#define   MPHY_PGCR2_ACPDDC		BIT(29)	/* AC Power-Down with Dual Ch*/
#define MPHY_PGCR3		(0x005 << MPHY_SHIFT)
#define MPHY_PGSR0		(0x006 << MPHY_SHIFT)
#define   MPHY_PGSR0_IDONE		BIT(0)	/* Initialization Done */
#define   MPHY_PGSR0_PLDONE		BIT(1)	/* PLL Lock Done */
#define   MPHY_PGSR0_DCDONE		BIT(2)	/* DDL Calibration Done */
#define   MPHY_PGSR0_ZCDONE		BIT(3)	/* Impedance Calibration Done */
#define   MPHY_PGSR0_DIDONE		BIT(4)	/* DRAM Initialization Done */
#define   MPHY_PGSR0_WLDONE		BIT(5)	/* Write Leveling Done */
#define   MPHY_PGSR0_QSGDONE		BIT(6)	/* DQS Gate Training Done */
#define   MPHY_PGSR0_WLADONE		BIT(7)	/* Write Leveling Adjust Done */
#define   MPHY_PGSR0_RDDONE		BIT(8)	/* Read Bit Deskew Done */
#define   MPHY_PGSR0_WDDONE		BIT(9)	/* Write Bit Deskew Done */
#define   MPHY_PGSR0_REDONE		BIT(10)	/* Read Eye Training Done */
#define   MPHY_PGSR0_WEDONE		BIT(11)	/* Write Eye Training Done */
#define   MPHY_PGSR0_ZCERR		BIT(20)	/* Impedance Calib Error */
#define   MPHY_PGSR0_WLERR		BIT(21)	/* Write Leveling Error */
#define   MPHY_PGSR0_QSGERR		BIT(22)	/* DQS Gate Training Error */
#define   MPHY_PGSR0_WLAERR		BIT(23)	/* Write Leveling Adj Error */
#define   MPHY_PGSR0_RDERR		BIT(24)	/* Read Bit Deskew Error */
#define   MPHY_PGSR0_WDERR		BIT(25)	/* Write Bit Deskew Error */
#define   MPHY_PGSR0_REERR		BIT(26)	/* Read Eye Training Error */
#define   MPHY_PGSR0_WEERR		BIT(27)	/* Write Eye Training Error */
#define MPHY_PGSR1		(0x007 << MPHY_SHIFT)
#define   MPHY_PGSR1_VTSTOP		BIT(30)	/* VT Stop */
#define MPHY_PLLCR		(0x008 << MPHY_SHIFT)
#define MPHY_PTR0		(0x009 << MPHY_SHIFT)
#define MPHY_PTR1		(0x00A << MPHY_SHIFT)
#define MPHY_PTR2		(0x00B << MPHY_SHIFT)
#define MPHY_PTR3		(0x00C << MPHY_SHIFT)
#define MPHY_PTR4		(0x00D << MPHY_SHIFT)
#define MPHY_ACMDLR		(0x00E << MPHY_SHIFT)
#define MPHY_ACLCDLR		(0x00F << MPHY_SHIFT)
#define MPHY_ACBDLR0		(0x010 << MPHY_SHIFT)
#define MPHY_ACBDLR1		(0x011 << MPHY_SHIFT)
#define MPHY_ACBDLR2		(0x012 << MPHY_SHIFT)
#define MPHY_ACBDLR3		(0x013 << MPHY_SHIFT)
#define MPHY_ACBDLR4		(0x014 << MPHY_SHIFT)
#define MPHY_ACBDLR5		(0x015 << MPHY_SHIFT)
#define MPHY_ACBDLR6		(0x016 << MPHY_SHIFT)
#define MPHY_ACBDLR7		(0x017 << MPHY_SHIFT)
#define MPHY_ACBDLR8		(0x018 << MPHY_SHIFT)
#define MPHY_ACBDLR9		(0x019 << MPHY_SHIFT)
#define MPHY_ACIOCR0		(0x01A << MPHY_SHIFT)
#define MPHY_ACIOCR1		(0x01B << MPHY_SHIFT)
#define MPHY_ACIOCR2		(0x01C << MPHY_SHIFT)
#define MPHY_ACIOCR3		(0x01D << MPHY_SHIFT)
#define MPHY_ACIOCR4		(0x01E << MPHY_SHIFT)
#define MPHY_ACIOCR5		(0x01F << MPHY_SHIFT)
#define MPHY_DXCCR		(0x020 << MPHY_SHIFT)
#define MPHY_DSGCR		(0x021 << MPHY_SHIFT)
#define MPHY_DCR		(0x022 << MPHY_SHIFT)
#define MPHY_DTPR0		(0x023 << MPHY_SHIFT)
#define MPHY_DTPR1		(0x024 << MPHY_SHIFT)
#define MPHY_DTPR2		(0x025 << MPHY_SHIFT)
#define MPHY_DTPR3		(0x026 << MPHY_SHIFT)
#define MPHY_MR0		(0x027 << MPHY_SHIFT)
#define MPHY_MR1		(0x028 << MPHY_SHIFT)
#define MPHY_MR2		(0x029 << MPHY_SHIFT)
#define MPHY_MR3		(0x02A << MPHY_SHIFT)
#define MPHY_ODTCR		(0x02B << MPHY_SHIFT)
#define MPHY_DTCR		(0x02C << MPHY_SHIFT)
#define   MPHY_DTCR_RANKEN_SHIFT	24	/* Rank Enable */
#define   MPHY_DTCR_RANKEN_MASK		(0xf << (MPHY_DTCR_RANKEN_SHIFT))
#define MPHY_DTAR0		(0x02D << MPHY_SHIFT)
#define MPHY_DTAR1		(0x02E << MPHY_SHIFT)
#define MPHY_DTAR2		(0x02F << MPHY_SHIFT)
#define MPHY_DTAR3		(0x030 << MPHY_SHIFT)
#define MPHY_DTDR0		(0x031 << MPHY_SHIFT)
#define MPHY_DTDR1		(0x032 << MPHY_SHIFT)
#define MPHY_DTEDR0		(0x033 << MPHY_SHIFT)
#define MPHY_DTEDR1		(0x034 << MPHY_SHIFT)
#define MPHY_ZQCR		(0x090 << MPHY_SHIFT)
#define   MPHY_ZQCR_AVGEN			BIT(16)	/* Average Algorithm */
#define   MPHY_ZQCR_FORCE_ZCAL_VT_UPDATE	BIT(27)	/* force VT update */
/* ZQ */
#define MPHY_ZQ_BASE		(0x091 << MPHY_SHIFT)
#define MPHY_ZQ_STRIDE		(0x004 << MPHY_SHIFT)
#define MPHY_ZQ_PR		(0x000 << MPHY_SHIFT)
#define MPHY_ZQ_DR		(0x001 << MPHY_SHIFT)
#define MPHY_ZQ_SR		(0x002 << MPHY_SHIFT)
/* DATX8 */
#define MPHY_DX_BASE		(0x0A0 << MPHY_SHIFT)
#define MPHY_DX_STRIDE		(0x020 << MPHY_SHIFT)
#define MPHY_DX_GCR0		(0x000 << MPHY_SHIFT)
#define   MPHY_DX_GCR0_WLRKEN_SHIFT	26	/* Write Level Rank Enable */
#define   MPHY_DX_GCR0_WLRKEN_MASK	(0xf << (MPHY_DX_GCR0_WLRKEN_SHIFT))
#define MPHY_DX_GCR1		(0x001 << MPHY_SHIFT)
#define MPHY_DX_GCR2		(0x002 << MPHY_SHIFT)
#define MPHY_DX_GCR3		(0x003 << MPHY_SHIFT)
#define MPHY_DX_GSR0		(0x004 << MPHY_SHIFT)
#define MPHY_DX_GSR1		(0x005 << MPHY_SHIFT)
#define MPHY_DX_GSR2		(0x006 << MPHY_SHIFT)
#define MPHY_DX_BDLR0		(0x007 << MPHY_SHIFT)
#define MPHY_DX_BDLR1		(0x008 << MPHY_SHIFT)
#define MPHY_DX_BDLR2		(0x009 << MPHY_SHIFT)
#define MPHY_DX_BDLR3		(0x00A << MPHY_SHIFT)
#define MPHY_DX_BDLR4		(0x00B << MPHY_SHIFT)
#define MPHY_DX_BDLR5		(0x00C << MPHY_SHIFT)
#define MPHY_DX_BDLR6		(0x00D << MPHY_SHIFT)
#define MPHY_DX_LCDLR0		(0x00E << MPHY_SHIFT)
#define MPHY_DX_LCDLR1		(0x00F << MPHY_SHIFT)
#define MPHY_DX_LCDLR2		(0x010 << MPHY_SHIFT)
#define MPHY_DX_MDLR		(0x011 << MPHY_SHIFT)
#define MPHY_DX_GTR		(0x012 << MPHY_SHIFT)

#endif /* UNIPHIER_DDRMPHY_REGS_H */
