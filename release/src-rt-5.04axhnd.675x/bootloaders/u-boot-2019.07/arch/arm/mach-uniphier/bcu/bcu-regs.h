/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * UniPhier BCU (Bus Control Unit) registers
 *
 * Copyright (C) 2011-2014 Panasonic Corporation
 */

#ifndef ARCH_BCU_REGS_H
#define ARCH_BCU_REGS_H

#define	BCU_BASE		0x50080000

#define	BCSCR(x)                (BCU_BASE + 0x180 + (x) * 4)
#define	BCSCR0			(BCSCR(0))
#define	BCSCR1			(BCSCR(1))
#define	BCSCR2			(BCSCR(2))
#define	BCSCR3			(BCSCR(3))
#define	BCSCR4			(BCSCR(4))
#define	BCSCR5			(BCSCR(5))

#define	BCIPPCCHR(x)		(BCU_BASE + 0x0280 + (x) * 4)
#define	BCIPPCCHR0		(BCIPPCCHR(0))
#define	BCIPPCCHR1		(BCIPPCCHR(1))
#define	BCIPPCCHR2		(BCIPPCCHR(2))
#define	BCIPPCCHR3		(BCIPPCCHR(3))
#define	BCIPPCCHR4		(BCIPPCCHR(4))
#define	BCIPPCCHR5		(BCIPPCCHR(5))

#endif  /* ARCH_BCU_REGS_H */
