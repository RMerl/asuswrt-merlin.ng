/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * UniPhier SBC (System Bus Controller) registers
 *
 * Copyright (C) 2011-2014 Panasonic Corporation
 * Copyright (C) 2015-2016 Socionext Inc.
 */

#ifndef ARCH_SBC_REGS_H
#define ARCH_SBC_REGS_H

#define	SBBASE_BASE		0x58c00100
#define	SBBASE(x)		(SBBASE_BASE + (x) * 0x10)

#define	SBBASE0			(SBBASE(0))
#define	SBBASE1			(SBBASE(1))
#define	SBBASE2			(SBBASE(2))
#define	SBBASE3			(SBBASE(3))
#define	SBBASE4			(SBBASE(4))
#define	SBBASE5			(SBBASE(5))
#define	SBBASE6			(SBBASE(6))
#define	SBBASE7			(SBBASE(7))

#define	SBBASE_BANK_ENABLE	(0x00000001)

#define	SBCTRL_BASE		0x58c00200
#define	SBCTRL(x, y)		(SBCTRL_BASE + (x) * 0x10 + (y) * 4)

#define	SBCTRL00		SBCTRL(0, 0)
#define	SBCTRL01		SBCTRL(0, 1)
#define	SBCTRL02		SBCTRL(0, 2)
#define	SBCTRL03		SBCTRL(0, 3)
#define	SBCTRL04		(SBCTRL_BASE + 0x100)

#define	SBCTRL10		SBCTRL(1, 0)
#define	SBCTRL11		SBCTRL(1, 1)
#define	SBCTRL12		SBCTRL(1, 2)
#define	SBCTRL13		SBCTRL(1, 3)
#define	SBCTRL14		(SBCTRL_BASE + 0x110)

#define	SBCTRL20		SBCTRL(2, 0)
#define	SBCTRL21		SBCTRL(2, 1)
#define	SBCTRL22		SBCTRL(2, 2)
#define	SBCTRL23		SBCTRL(2, 3)
#define	SBCTRL24		(SBCTRL_BASE + 0x120)

#define	SBCTRL30		SBCTRL(3, 0)
#define	SBCTRL31		SBCTRL(3, 1)
#define	SBCTRL32		SBCTRL(3, 2)
#define	SBCTRL33		SBCTRL(3, 3)
#define	SBCTRL34		(SBCTRL_BASE + 0x130)

#define	SBCTRL40		SBCTRL(4, 0)
#define	SBCTRL41		SBCTRL(4, 1)
#define	SBCTRL42		SBCTRL(4, 2)
#define	SBCTRL43		SBCTRL(4, 3)
#define	SBCTRL44		(SBCTRL_BASE + 0x140)

#define	SBCTRL50		SBCTRL(5, 0)
#define	SBCTRL51		SBCTRL(5, 1)
#define	SBCTRL52		SBCTRL(5, 2)
#define	SBCTRL53		SBCTRL(5, 3)
#define	SBCTRL54		(SBCTRL_BASE + 0x150)

#define	SBCTRL60		SBCTRL(6, 0)
#define	SBCTRL61		SBCTRL(6, 1)
#define	SBCTRL62		SBCTRL(6, 2)
#define	SBCTRL63		SBCTRL(6, 3)
#define	SBCTRL64		(SBCTRL_BASE + 0x160)

#define	SBCTRL70		SBCTRL(7, 0)
#define	SBCTRL71		SBCTRL(7, 1)
#define	SBCTRL72		SBCTRL(7, 2)
#define	SBCTRL73		SBCTRL(7, 3)
#define	SBCTRL74		(SBCTRL_BASE + 0x170)

#define PC0CTRL				0x598000c0

#ifndef __ASSEMBLY__
#include <linux/io.h>
static inline int boot_is_swapped(void)
{
	return !(readl(SBBASE0) & SBBASE_BANK_ENABLE);
}
#endif

#endif	/* ARCH_SBC_REGS_H */
