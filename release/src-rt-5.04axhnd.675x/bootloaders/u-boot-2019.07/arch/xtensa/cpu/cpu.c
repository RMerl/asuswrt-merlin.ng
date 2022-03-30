// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 - 2013 Tensilica Inc.
 * (C) Copyright 2014 - 2016 Cadence Design Systems Inc.
 */

/*
 * CPU specific code
 */

#include <common.h>
#include <command.h>
#include <linux/stringify.h>
#include <asm/global_data.h>
#include <asm/cache.h>
#include <asm/string.h>
#include <asm/misc.h>

DECLARE_GLOBAL_DATA_PTR;

gd_t *gd __attribute__((section(".data")));

#if defined(CONFIG_DISPLAY_CPUINFO)
/*
 * Print information about the CPU.
 */

int print_cpuinfo(void)
{
	char buf[120], mhz[8];
	uint32_t id0, id1;

	asm volatile ("rsr %0, 176\n"
		      "rsr %1, 208\n"
		      : "=r"(id0), "=r"(id1));

	sprintf(buf, "CPU:   Xtensa %s (id: %08x:%08x) at %s MHz\n",
		XCHAL_CORE_ID, id0, id1, strmhz(mhz, gd->cpu_clk));
	puts(buf);
	return 0;
}
#endif

int arch_cpu_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;
	return 0;
}

int dram_init(void)
{
	return 0;
}
