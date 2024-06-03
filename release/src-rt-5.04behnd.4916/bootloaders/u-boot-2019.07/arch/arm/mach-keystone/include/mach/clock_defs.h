/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * keystone2: common pll clock definitions
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#ifndef _CLOCK_DEFS_H_
#define _CLOCK_DEFS_H_

#include <asm/arch/hardware.h>

/* PLL Control Registers */
struct pllctl_regs {
	u32	ctl;		/* 00 */
	u32	ocsel;		/* 04 */
	u32	secctl;		/* 08 */
	u32	resv0;
	u32	mult;		/* 10 */
	u32	prediv;		/* 14 */
	u32	div1;		/* 18 */
	u32	div2;		/* 1c */
	u32	div3;		/* 20 */
	u32	oscdiv1;	/* 24 */
	u32	resv1;		/* 28 */
	u32	bpdiv;		/* 2c */
	u32	wakeup;		/* 30 */
	u32	resv2;
	u32	cmd;		/* 38 */
	u32	stat;		/* 3c */
	u32	alnctl;		/* 40 */
	u32	dchange;	/* 44 */
	u32	cken;		/* 48 */
	u32	ckstat;		/* 4c */
	u32	systat;		/* 50 */
	u32	ckctl;		/* 54 */
	u32	resv3[2];
	u32	div4;		/* 60 */
	u32	div5;		/* 64 */
	u32	div6;		/* 68 */
	u32	div7;		/* 6c */
	u32	div8;		/* 70 */
	u32	div9;		/* 74 */
	u32	div10;		/* 78 */
	u32	div11;		/* 7c */
	u32	div12;		/* 80 */
};

static struct pllctl_regs *pllctl_regs[] = {
	(struct pllctl_regs *)(KS2_CLOCK_BASE + 0x100)
};

#define pllctl_reg(pll, reg)            (&(pllctl_regs[pll]->reg))
#define pllctl_reg_read(pll, reg)       __raw_readl(pllctl_reg(pll, reg))
#define pllctl_reg_write(pll, reg, val) __raw_writel(val, pllctl_reg(pll, reg))

#define pllctl_reg_rmw(pll, reg, mask, val) \
	pllctl_reg_write(pll, reg, \
		(pllctl_reg_read(pll, reg) & ~(mask)) | val)

#define pllctl_reg_setbits(pll, reg, mask) \
	pllctl_reg_rmw(pll, reg, 0, mask)

#define pllctl_reg_clrbits(pll, reg, mask) \
	pllctl_reg_rmw(pll, reg, mask, 0)

#define pll0div_read(N) ((pllctl_reg_read(CORE_PLL, div##N) & 0xff) + 1)

/* PLLCTL Bits */
#define PLLCTL_PLLENSRC_SHIF	5
#define PLLCTL_PLLENSRC_MASK	BIT(5)
#define PLLCTL_PLLRST_SHIFT	3
#define PLLCTL_PLLRST_MASK	BIT(3)
#define PLLCTL_PLLPWRDN_SHIFT	1
#define PLLCTL_PLLPWRDN_MASK	BIT(1)
#define PLLCTL_PLLEN_SHIFT	0
#define PLLCTL_PLLEN_MASK	BIT(0)

/* SECCTL Bits */
#define SECCTL_BYPASS_SHIFT	23
#define SECCTL_BYPASS_MASK	BIT(23)
#define SECCTL_OP_DIV_SHIFT	19
#define SECCTL_OP_DIV_MASK	(0xf << 19)

/* PLLM Bits */
#define PLLM_MULT_LO_SHIFT	0
#define PLLM_MULT_LO_MASK	0x3f
#define PLLM_MULT_LO_BITS	6

/* PLLDIVn Bits */
#define PLLDIV_ENABLE_SHIFT	15
#define PLLDIV_ENABLE_MASK	BIT(15)
#define PLLDIV_RATIO_SHIFT	0x0
#define PLLDIV_RATIO_MASK	0xff
#define PLLDIV_MAX		16

/* PLLCMD Bits */
#define PLLCMD_GOSET_SHIFT	0
#define PLLCMD_GOSET_MASK	BIT(0)

/* PLLSTAT Bits */
#define PLLSTAT_GOSTAT_SHIFT	0
#define PLLSTAT_GOSTAT_MASK	BIT(0)

/* Device Config PLLCTL0 */
#define CFG_PLLCTL0_BWADJ_SHIFT		24
#define CFG_PLLCTL0_BWADJ_MASK		(0xff << 24)
#define CFG_PLLCTL0_BWADJ_BITS		8
#define CFG_PLLCTL0_BYPASS_SHIFT	23
#define CFG_PLLCTL0_BYPASS_MASK		BIT(23)
#define CFG_PLLCTL0_CLKOD_SHIFT		19
#define CFG_PLLCTL0_CLKOD_MASK		(0xf << 19)
#define CFG_PLLCTL0_PLLM_HI_SHIFT	12
#define CFG_PLLCTL0_PLLM_HI_MASK	(0x7f << 12)
#define CFG_PLLCTL0_PLLM_SHIFT		6
#define CFG_PLLCTL0_PLLM_MASK		(0x1fff << 6)
#define CFG_PLLCTL0_PLLD_SHIFT		0
#define CFG_PLLCTL0_PLLD_MASK		0x3f

/* Device Config PLLCTL1 */
#define CFG_PLLCTL1_RST_SHIFT	14
#define CFG_PLLCTL1_RST_MASK	BIT(14)
#define CFG_PLLCTL1_PAPLL_SHIFT	13
#define CFG_PLLCTL1_PAPLL_MASK	BIT(13)
#define CFG_PLLCTL1_ENSAT_SHIFT	6
#define CFG_PLLCTL1_ENSAT_MASK	BIT(6)
#define CFG_PLLCTL1_BWADJ_SHIFT	0
#define CFG_PLLCTL1_BWADJ_MASK	0xf

#define MISC_CTL1_ARM_PLL_EN	BIT(13)

#endif  /* _CLOCK_DEFS_H_ */
