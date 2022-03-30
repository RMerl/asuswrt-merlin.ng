/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  Copyright (C) 2012 Altera Corporation <www.altera.com>
 */

#ifndef _SOCFPGA_TIMER_H_
#define _SOCFPGA_TIMER_H_

struct socfpga_timer {
	u32	load_val;
	u32	curr_val;
	u32	ctrl;
	u32	eoi;
	u32	int_stat;
};

#endif
