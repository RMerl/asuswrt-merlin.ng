// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2017-2022 Intel Corporation <www.intel.com>
 *
 */

#include <common.h>
#include <init.h>
#include <div64.h>
#include <asm/io.h>
#include <asm/arch/timer.h>

/*
 * Timer initialization
 */
int timer_init(void)
{
#ifdef CONFIG_SPL_BUILD
	int enable = 0x3;	/* timer enable + output signal masked */
	int loadval = ~0;

	/* enable system counter */
	writel(enable, SOCFPGA_GTIMER_SEC_ADDRESS);
	/* enable processor pysical counter */
	asm volatile("msr cntp_ctl_el0, %0" : : "r" (enable));
	asm volatile("msr cntp_tval_el0, %0" : : "r" (loadval));
#endif
	return 0;
}

__always_inline u64 __get_time_stamp(void)
{
	u64 cntpct;

	isb();
	asm volatile("mrs %0, cntpct_el0" : "=r" (cntpct));

	return cntpct;
}

__always_inline uint64_t __usec_to_tick(unsigned long usec)
{
	u64 tick = usec;
	u64 cntfrq;

	asm volatile("mrs %0, cntfrq_el0" : "=r" (cntfrq));
	tick *= cntfrq;
	do_div(tick, 1000000);

	return tick;
}

__always_inline void __udelay(unsigned long usec)
{
	/* get current timestamp */
	u64 tmp = __get_time_stamp() + __usec_to_tick(usec);

	while (__get_time_stamp() < tmp + 1)	/* loop till event */
		;
}