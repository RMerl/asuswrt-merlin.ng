/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */
#ifndef _DV_AINTC_DEFS_H_
#define _DV_AINTC_DEFS_H_

struct dv_aintc_regs {
	unsigned int	fiq0;		/* 0x00 */
	unsigned int	fiq1;		/* 0x04 */
	unsigned int	irq0;		/* 0x08 */
	unsigned int	irq1;		/* 0x0c */
	unsigned int	fiqentry;	/* 0x10 */
	unsigned int	irqentry;	/* 0x14 */
	unsigned int	eint0;		/* 0x18 */
	unsigned int	eint1;		/* 0x1c */
	unsigned int	intctl;		/* 0x20 */
	unsigned int	eabase;		/* 0x24 */
	unsigned char	rsvd0[8];	/* 0x28 */
	unsigned int	intpri0;	/* 0x30 */
	unsigned int	intpri1;	/* 0x34 */
	unsigned int	intpri2;	/* 0x38 */
	unsigned int	intpri3;	/* 0x3c */
	unsigned int	intpri4;	/* 0x40 */
	unsigned int	intpri5;	/* 0x44 */
	unsigned int	intpri6;	/* 0x48 */
	unsigned int	intpri7;	/* 0x4c */
};

#define dv_aintc_regs ((struct dv_aintc_regs *)DAVINCI_ARM_INTC_BASE)

#define DV_AINTC_INTCTL_IDMODE	(1 << 2)

#endif /* _DV_AINTC_DEFS_H_ */
