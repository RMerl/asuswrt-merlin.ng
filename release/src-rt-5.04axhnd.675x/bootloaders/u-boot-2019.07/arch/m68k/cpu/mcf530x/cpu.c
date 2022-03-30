// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014  Angelo Dureghello <angelo@sysam.it>
 *
 */

#include <common.h>
#include <asm/immap.h>
#include <asm/io.h>

#ifdef CONFIG_M5307
int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	sim_t *sim = (sim_t *)(MMAP_SIM);

	/* enable watchdog/reset, set timeout to 0 and wait */
	out_8(&sim->sypcr, SYPCR_SWE | SYPCR_SWRI);

	/* wait for watchdog reset */
	for (;;)
		;

	/* we don't return! */
	return 0;
}

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	char buf[32];

	printf("CPU:   Freescale Coldfire MCF5307 at %s MHz\n",
	       strmhz(buf, CONFIG_SYS_CPU_CLK));
	return 0;
}
#endif /* CONFIG_DISPLAY_CPUINFO */
#endif
