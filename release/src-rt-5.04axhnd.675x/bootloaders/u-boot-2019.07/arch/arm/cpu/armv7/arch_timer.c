// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <common.h>
#include <asm/io.h>
#include <div64.h>
#include <bootstage.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_SYS_HZ_CLOCK
static inline u32 read_cntfrq(void)
{
	u32 frq;

	asm volatile("mrc p15, 0, %0, c14, c0, 0" : "=r" (frq));
	return frq;
}
#endif

int timer_init(void)
{
	gd->arch.tbl = 0;
	gd->arch.tbu = 0;

#ifdef CONFIG_SYS_HZ_CLOCK
	gd->arch.timer_rate_hz = CONFIG_SYS_HZ_CLOCK;
#else
	gd->arch.timer_rate_hz = read_cntfrq();
#endif
	return 0;
}

unsigned long long get_ticks(void)
{
	ulong nowl, nowu;

	asm volatile("mrrc p15, 0, %0, %1, c14" : "=r" (nowl), "=r" (nowu));

	gd->arch.tbl = nowl;
	gd->arch.tbu = nowu;

	return (((unsigned long long)gd->arch.tbu) << 32) | gd->arch.tbl;
}


ulong timer_get_boot_us(void)
{
	if (!gd->arch.timer_rate_hz)
		timer_init();

	return lldiv(get_ticks(), gd->arch.timer_rate_hz / 1000000);
}

ulong get_tbclk(void)
{
	return gd->arch.timer_rate_hz;
}
