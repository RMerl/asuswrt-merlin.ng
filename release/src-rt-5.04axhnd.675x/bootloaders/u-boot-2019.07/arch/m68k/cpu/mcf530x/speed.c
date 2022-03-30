// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014  Angelo Dureghello <angelo@sysam.it>
 *
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/immap.h>

DECLARE_GLOBAL_DATA_PTR;

/* get_clocks() fills in gd->cpu_clock and gd->bus_clk */
int get_clocks(void)
{
#if defined(CONFIG_M5307)
	gd->bus_clk = CONFIG_SYS_CLK;
	gd->cpu_clk = CONFIG_SYS_CPU_CLK;
#endif

	return 0;
}
