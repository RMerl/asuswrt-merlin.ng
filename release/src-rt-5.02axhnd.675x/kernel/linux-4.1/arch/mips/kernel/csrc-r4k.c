/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2007 by Ralf Baechle
 */
#include <linux/clocksource.h>
#include <linux/init.h>
#include <linux/sched_clock.h>

#include <asm/time.h>

static cycle_t c0_hpt_read(struct clocksource *cs)
{
	return read_c0_count();
}

static struct clocksource clocksource_mips = {
	.name		= "MIPS",
	.read		= c0_hpt_read,
	.mask		= CLOCKSOURCE_MASK(32),
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
};

static u64 __maybe_unused notrace r4k_read_sched_clock(void)
{
	return read_c0_count();
}

int __init init_r4k_clocksource(void)
{
	if (!cpu_has_counter || !mips_hpt_frequency)
		return -ENXIO;

	/* Calculate a somewhat reasonable rating value */
	clocksource_mips.rating = 200 + mips_hpt_frequency / 10000000;

	clocksource_register_hz(&clocksource_mips, mips_hpt_frequency);

#ifndef CONFIG_CPU_FREQ
	sched_clock_register(r4k_read_sched_clock, 32, mips_hpt_frequency);
#endif

	return 0;
}
