/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */
#ifndef _DV_PLL_DEFS_H_
#define _DV_PLL_DEFS_H_

struct dv_pll_regs {
	unsigned int	pid;		/* 0x00 */
	unsigned char	rsvd0[224];	/* 0x04 */
	unsigned int	rstype;		/* 0xe4 */
	unsigned char	rsvd1[24];	/* 0xe8 */
	unsigned int	pllctl;		/* 0x100 */
	unsigned char	rsvd2[4];	/* 0x104 */
	unsigned int	secctl;		/* 0x108 */
	unsigned int	rv;		/* 0x10c */
	unsigned int	pllm;		/* 0x110 */
	unsigned int	prediv;		/* 0x114 */
	unsigned int	plldiv1;	/* 0x118 */
	unsigned int	plldiv2;	/* 0x11c */
	unsigned int	plldiv3;	/* 0x120 */
	unsigned int	oscdiv1;	/* 0x124 */
	unsigned int	postdiv;	/* 0x128 */
	unsigned int	bpdiv;		/* 0x12c */
	unsigned char	rsvd5[8];	/* 0x130 */
	unsigned int	pllcmd;		/* 0x138 */
	unsigned int	pllstat;	/* 0x13c */
	unsigned int	alnctl;		/* 0x140 */
	unsigned int	dchange;	/* 0x144 */
	unsigned int	cken;		/* 0x148 */
	unsigned int	ckstat;		/* 0x14c */
	unsigned int	systat;		/* 0x150 */
	unsigned char	rsvd6[12];	/* 0x154 */
	unsigned int	plldiv4;	/* 0x160 */
	unsigned int	plldiv5;	/* 0x164 */
	unsigned int	plldiv6;	/* 0x168 */
	unsigned int	plldiv7;	/* 0x16C */
	unsigned int	plldiv8;	/* 0x170 */
	unsigned int	plldiv9;	/* 0x174 */
};

#define PLL_MASTER_LOCK	(1 << 4)

#define PLLCTL_CLOCK_MODE_SHIFT	8
#define PLLCTL_PLLEN	(1 << 0)
#define PLLCTL_PLLPWRDN	(1 << 1)
#define PLLCTL_PLLRST	(1 << 3)
#define PLLCTL_PLLDIS	(1 << 4)
#define PLLCTL_PLLENSRC	(1 << 5)
#define PLLCTL_RES_9	(1 << 8)
#define PLLCTL_EXTCLKSRC	(1 << 9)

#define PLL_DIVEN	(1 << 15)
#define PLL_POSTDEN	PLL_DIVEN

#define PLL_SCSCFG3_DIV45PENA	(1 << 2)
#define PLL_SCSCFG3_EMA_CLKSRC	(1 << 1)

#define PLL_RSTYPE_POR		(1 << 0)
#define PLL_RSTYPE_XWRST	(1 << 1)

#define PLLSECCTL_TINITZ	(1 << 16)
#define PLLSECCTL_TENABLE	(1 << 17)
#define PLLSECCTL_TENABLEDIV	(1 << 18)
#define PLLSECCTL_STOPMODE	(1 << 22)

#define PLLCMD_GOSET		(1 << 0)
#define PLLCMD_GOSTAT		(1 << 0)

#define PLL0_LOCK		0x07000000
#define PLL1_LOCK		0x07000000

#define dv_pll0_regs ((struct dv_pll_regs *)DAVINCI_PLL_CNTRL0_BASE)
#define dv_pll1_regs ((struct dv_pll_regs *)DAVINCI_PLL_CNTRL1_BASE)

#define ARM_PLLDIV	(offsetof(struct dv_pll_regs, plldiv2))
#define DDR_PLLDIV	(offsetof(struct dv_pll_regs, plldiv7))
#define SPI_PLLDIV	(offsetof(struct dv_pll_regs, plldiv4))

unsigned int davinci_clk_get(unsigned int div);
#endif /* _DV_PLL_DEFS_H_ */
