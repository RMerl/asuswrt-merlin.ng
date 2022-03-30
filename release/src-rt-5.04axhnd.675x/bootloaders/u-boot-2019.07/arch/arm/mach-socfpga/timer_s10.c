// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2017-2018 Intel Corporation <www.intel.com>
 *
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/timer.h>

/*
 * Timer initialization
 */
int timer_init(void)
{
	int enable = 0x3;	/* timer enable + output signal masked */
	int loadval = ~0;

	/* enable system counter */
	writel(enable, SOCFPGA_GTIMER_SEC_ADDRESS);
	/* enable processor pysical counter */
	asm volatile("msr cntp_ctl_el0, %0" : : "r" (enable));
	asm volatile("msr cntp_tval_el0, %0" : : "r" (loadval));

	return 0;
}
