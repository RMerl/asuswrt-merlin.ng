/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * SoC-specific lowlevel code for tms320dm365 and similar chips
 *
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */
#ifndef __DM365_LOWLEVEL_H
#define __DM365_LOWLEVEL_H

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/io.h>

void dm365_waitloop(unsigned long loopcnt);
int dm365_pll1_init(unsigned long pllmult, unsigned long prediv);
int dm365_pll2_init(unsigned long pllm, unsigned long prediv);
int dm365_ddr_setup(void);
void dm365_psc_init(void);
void dm365_pinmux_ctl(unsigned long offset, unsigned long mask,
	unsigned long value);
void dm36x_lowlevel_init(ulong bootflag);

#endif /* #ifndef __DM365_LOWLEVEL_H */
