// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009
 * Jean-Christophe PLAGNIOL-VILLARD <plagnioj@jcrosoft.com>
 *
 * (C) Copyright 2007-2012
 * Nobobuhiro Iwamatsu <iwamatsu@nigauri.org>
 *
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/io.h>

#if defined(CONFIG_CPU_SH4) || defined(CONFIG_ARCH_RMOBILE)
#define TSTR	0x4
#define TCR0	0x10
#endif /* CONFIG_CPU_SH4 */

#define TCR_TPSC	0x07
#define TSTR_STR0	BIT(0)

int timer_init(void)
{
	writew(readw(TMU_BASE + TCR0) & ~TCR_TPSC, TMU_BASE + TCR0);
	writeb(readb(TMU_BASE + TSTR) & ~TSTR_STR0, TMU_BASE + TSTR);
	writeb(readb(TMU_BASE + TSTR) | TSTR_STR0, TMU_BASE + TSTR);

	return 0;
}

