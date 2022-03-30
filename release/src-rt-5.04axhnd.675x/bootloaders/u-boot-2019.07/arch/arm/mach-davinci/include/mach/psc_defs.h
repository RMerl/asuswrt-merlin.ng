/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */
#ifndef _DV_PSC_DEFS_H_
#define _DV_PSC_DEFS_H_

/*
 * Power/Sleep Ctrl Register structure
 * See sprufb3.pdf, Chapter 7
 */
struct dv_psc_regs {
	unsigned int	pid;		/* 0x000 */
	unsigned char	rsvd0[16];	/* 0x004 */
	unsigned char	rsvd1[4];	/* 0x014 */
	unsigned int	inteval;	/* 0x018 */
	unsigned char	rsvd2[36];	/* 0x01C */
	unsigned int	merrpr0;	/* 0x040 */
	unsigned int	merrpr1;	/* 0x044 */
	unsigned char	rsvd3[8];	/* 0x048 */
	unsigned int	merrcr0;	/* 0x050 */
	unsigned int	merrcr1;	/* 0x054 */
	unsigned char	rsvd4[8];	/* 0x058 */
	unsigned int	perrpr;		/* 0x060 */
	unsigned char	rsvd5[4];	/* 0x064 */
	unsigned int	perrcr;		/* 0x068 */
	unsigned char	rsvd6[4];	/* 0x06C */
	unsigned int	epcpr;		/* 0x070 */
	unsigned char	rsvd7[4];	/* 0x074 */
	unsigned int	epccr;		/* 0x078 */
	unsigned char	rsvd8[144];	/* 0x07C */
	unsigned char	rsvd9[20];	/* 0x10C */
	unsigned int	ptcmd;		/* 0x120 */
	unsigned char	rsvd10[4];	/* 0x124 */
	unsigned int	ptstat;		/* 0x128 */
	unsigned char	rsvd11[212];	/* 0x12C */
	unsigned int	pdstat0;	/* 0x200 */
	unsigned int	pdstat1;	/* 0x204 */
	unsigned char	rsvd12[248];	/* 0x208 */
	unsigned int	pdctl0;		/* 0x300 */
	unsigned int	pdctl1;		/* 0x304 */
	unsigned char	rsvd13[536];	/* 0x308 */
	unsigned int	mckout0;	/* 0x520 */
	unsigned int	mckout1;	/* 0x524 */
	unsigned char	rsvd14[728];	/* 0x528 */
	unsigned int	mdstat[52];	/* 0x800 */
	unsigned char	rsvd15[304];	/* 0x8D0 */
	unsigned int	mdctl[52];	/* 0xA00 */
};

/* PSC constants */
#define EMURSTIE_MASK	(0x00000200)

#define PD0		(0)

#define PSC_ENABLE		(0x3)
#define PSC_DISABLE		(0x2)
#define PSC_SYNCRESET		(0x1)
#define PSC_SWRSTDISABLE	(0x0)

#define PSC_GOSTAT		(1 << 0)
#define PSC_MD_STATE_MSK	(0x1f)

#define PSC_CMD_GO		(1 << 0)

#define dv_psc_regs ((struct dv_psc_regs *)DAVINCI_PWR_SLEEP_CNTRL_BASE)

#endif /* _DV_PSC_DEFS_H_ */
