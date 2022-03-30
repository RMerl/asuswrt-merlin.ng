/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Synopsys, Inc. All rights reserved.
 * Author: Eugeniy Paltsev <Eugeniy.Paltsev@synopsys.com>
 */

#ifndef __BOARD_CLK_LIB_H
#define __BOARD_CLK_LIB_H

#include <common.h>

enum clk_ctl_ops {
	CLK_SET		= BIT(0), /* set frequency */
	CLK_GET		= BIT(1), /* get frequency */
	CLK_ON		= BIT(2), /* enable clock */
	CLK_OFF		= BIT(3), /* disable clock */
	CLK_PRINT	= BIT(4), /* print frequency */
	CLK_MHZ		= BIT(5)  /* all values in MHZ instead of HZ */
};

/*
 * Depending on the clk_ctl_ops enable / disable /
 * set clock rate from 'rate' argument / read clock to 'rate' argument /
 * print clock rate. If CLK_MHZ flag set in clk_ctl_ops 'rate' is in MHz,
 * otherwise - in Hz.
 *
 * This function expects "clk-fmeas" node in device tree:
 * / {
 *	clk-fmeas {
 *		clocks = <&cpu_pll>, <&sys_pll>;
 *		clock-names = "cpu-pll", "sys-pll";
 *	};
 * };
 */
int soc_clk_ctl(const char *name, ulong *rate, enum clk_ctl_ops ctl);

#endif /* __BOARD_CLK_LIB_H */
