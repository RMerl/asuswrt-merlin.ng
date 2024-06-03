/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * UniPhier SC (System Control) block registers for ARMv8 SoCs
 *
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#ifndef SC64_REGS_H
#define SC64_REGS_H

#define SC_BASE_ADDR		0x61840000

#define SC_RSTCTRL		(SC_BASE_ADDR | 0x2000)
#define SC_RSTCTRL3		(SC_BASE_ADDR | 0x2008)
#define SC_RSTCTRL4		(SC_BASE_ADDR | 0x200c)
#define SC_RSTCTRL5		(SC_BASE_ADDR | 0x2010)
#define SC_RSTCTRL6		(SC_BASE_ADDR | 0x2014)
#define SC_RSTCTRL7		(SC_BASE_ADDR | 0x2018)

#define SC_CLKCTRL		(SC_BASE_ADDR | 0x2100)
#define SC_CLKCTRL3		(SC_BASE_ADDR | 0x2108)
#define SC_CLKCTRL4		(SC_BASE_ADDR | 0x210c)
#define SC_CLKCTRL5		(SC_BASE_ADDR | 0x2110)
#define SC_CLKCTRL6		(SC_BASE_ADDR | 0x2114)
#define SC_CLKCTRL7		(SC_BASE_ADDR | 0x2118)

#define SC_CA72_GEARST		(SC_BASE_ADDR | 0x8000)
#define SC_CA72_GEARSET		(SC_BASE_ADDR | 0x8004)
#define SC_CA72_GEARUPD		(SC_BASE_ADDR | 0x8008)
#define SC_CA53_GEARST		(SC_BASE_ADDR | 0x8080)
#define SC_CA53_GEARSET		(SC_BASE_ADDR | 0x8084)
#define SC_CA53_GEARUPD		(SC_BASE_ADDR | 0x8088)
#define   SC_CA_GEARUPD			(1 << 0)

#endif /* SC64_REGS_H */
