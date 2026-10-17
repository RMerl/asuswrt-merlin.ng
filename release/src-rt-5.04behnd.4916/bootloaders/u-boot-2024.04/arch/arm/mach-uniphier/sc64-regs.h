/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * UniPhier SC (System Control) block registers for ARMv8 SoCs
 *
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#ifndef SC64_REGS_H
#define SC64_REGS_H

#ifndef __ASSEMBLY__
#include <linux/compiler.h>
extern void __iomem *sc_base;
#endif

#define SC_BASE			0x61840000

#define SC_RSTCTRL		0x2000
#define SC_RSTCTRL3		0x2008
#define SC_RSTCTRL4		0x200c
#define SC_RSTCTRL5		0x2010
#define SC_RSTCTRL6		0x2014
#define SC_RSTCTRL7		0x2018

#define SC_CLKCTRL		0x2100
#define SC_CLKCTRL3		0x2108
#define SC_CLKCTRL4		0x210c
#define SC_CLKCTRL5		0x2110
#define SC_CLKCTRL6		0x2114
#define SC_CLKCTRL7		0x2118

#define SC_CA72_GEARST		0x8000
#define SC_CA72_GEARSET		0x8004
#define SC_CA72_GEARUPD		0x8008
#define SC_CA53_GEARST		0x8080
#define SC_CA53_GEARSET		0x8084
#define SC_CA53_GEARUPD		0x8088
#define   SC_CA_GEARUPD			(1 << 0)

#endif /* SC64_REGS_H */
