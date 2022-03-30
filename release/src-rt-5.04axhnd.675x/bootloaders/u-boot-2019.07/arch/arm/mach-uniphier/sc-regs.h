/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * UniPhier SC (System Control) block registers
 *
 * Copyright (C) 2011-2015 Panasonic Corporation
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#ifndef ARCH_SC_REGS_H
#define ARCH_SC_REGS_H

#define SC_BASE_ADDR			0x61840000

#define SC_DPLLCTRL			(SC_BASE_ADDR | 0x1200)
#define SC_DPLLCTRL_SSC_EN		(0x1 << 31)
#define SC_DPLLCTRL_FOUTMODE_MASK        (0xf << 16)
#define SC_DPLLCTRL_SSC_RATE		(0x1 << 15)

#define SC_DPLLCTRL2			(SC_BASE_ADDR | 0x1204)
#define SC_DPLLCTRL2_NRSTDS		(0x1 << 28)

#define SC_DPLLCTRL3			(SC_BASE_ADDR | 0x1208)
#define SC_DPLLCTRL3_LPFSEL_COEF2	(0x0 << 31)
#define SC_DPLLCTRL3_LPFSEL_COEF3	(0x1 << 31)

#define SC_UPLLCTRL			(SC_BASE_ADDR | 0x1210)

#define SC_VPLL27ACTRL			(SC_BASE_ADDR | 0x1270)
#define SC_VPLL27ACTRL2			(SC_BASE_ADDR | 0x1274)
#define SC_VPLL27ACTRL3			(SC_BASE_ADDR | 0x1278)

#define SC_VPLL27BCTRL			(SC_BASE_ADDR | 0x1290)
#define SC_VPLL27BCTRL2			(SC_BASE_ADDR | 0x1294)
#define SC_VPLL27BCTRL3			(SC_BASE_ADDR | 0x1298)

#define SC_RSTCTRL			(SC_BASE_ADDR | 0x2000)
#define SC_RSTCTRL_NRST_USB3B0		(0x1 << 17)	/* USB3 #0 bus */
#define SC_RSTCTRL_NRST_USB3C0		(0x1 << 16)	/* USB3 #0 core */
#define SC_RSTCTRL_NRST_ETHER		(0x1 << 12)
#define SC_RSTCTRL_NRST_GIO		(0x1 <<  6)
/* Pro4 or older */
#define SC_RSTCTRL_NRST_UMC1		(0x1 <<  5)
#define SC_RSTCTRL_NRST_UMC0		(0x1 <<  4)
#define SC_RSTCTRL_NRST_NAND		(0x1 <<  2)

#define SC_RSTCTRL2			(SC_BASE_ADDR | 0x2004)
#define SC_RSTCTRL2_NRST_USB3B1		(0x1 << 17)	/* USB3 #1 bus */
#define SC_RSTCTRL2_NRST_USB3C1		(0x1 << 16)	/* USB3 #1 core */

#define SC_RSTCTRL3			(SC_BASE_ADDR | 0x2008)

/* Pro5 or newer */
#define SC_RSTCTRL4			(SC_BASE_ADDR | 0x200c)
#define SC_RSTCTRL4_NRST_UMCSB		(0x1 << 12)	/* UMC system bus */
#define SC_RSTCTRL4_NRST_UMCA2		(0x1 << 10)	/* UMC ch2 standby */
#define SC_RSTCTRL4_NRST_UMCA1		(0x1 <<  9)	/* UMC ch1 standby */
#define SC_RSTCTRL4_NRST_UMCA0		(0x1 <<  8)	/* UMC ch0 standby */
#define SC_RSTCTRL4_NRST_UMC32		(0x1 <<  6)	/* UMC ch2 */
#define SC_RSTCTRL4_NRST_UMC31		(0x1 <<  5)	/* UMC ch1 */
#define SC_RSTCTRL4_NRST_UMC30		(0x1 <<  4)	/* UMC ch0 */

#define SC_RSTCTRL5			(SC_BASE_ADDR | 0x2010)

#define SC_RSTCTRL6			(SC_BASE_ADDR | 0x2014)

#define SC_CLKCTRL			(SC_BASE_ADDR | 0x2104)
#define SC_CLKCTRL_CEN_USB31		(0x1 << 17)	/* USB3 #1 */
#define SC_CLKCTRL_CEN_USB30		(0x1 << 16)	/* USB3 #0 */
#define SC_CLKCTRL_CEN_ETHER		(0x1 << 12)
#define SC_CLKCTRL_CEN_GIO		(0x1 <<  6)
/* Pro4 or older */
#define SC_CLKCTRL_CEN_UMC		(0x1 <<  4)
#define SC_CLKCTRL_CEN_NAND		(0x1 <<  2)
#define SC_CLKCTRL_CEN_SBC		(0x1 <<  1)
#define SC_CLKCTRL_CEN_PERI		(0x1 <<  0)

/* Pro5 or newer */
#define SC_CLKCTRL4			(SC_BASE_ADDR | 0x210c)
#define SC_CLKCTRL4_CEN_UMCSB		(0x1 << 12)	/* UMC system bus */
#define SC_CLKCTRL4_CEN_UMC2		(0x1 <<  2)	/* UMC ch2 */
#define SC_CLKCTRL4_CEN_UMC1		(0x1 <<  1)	/* UMC ch1 */
#define SC_CLKCTRL4_CEN_UMC0		(0x1 <<  0)	/* UMC ch0 */

/* System reset control register */
#define SC_IRQTIMSET			(SC_BASE_ADDR | 0x3000)
#define SC_SLFRSTSEL			(SC_BASE_ADDR | 0x3010)
#define SC_SLFRSTCTL			(SC_BASE_ADDR | 0x3014)

#endif /* ARCH_SC_REGS_H */
