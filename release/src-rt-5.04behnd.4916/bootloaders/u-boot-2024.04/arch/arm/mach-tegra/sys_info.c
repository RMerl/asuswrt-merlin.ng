// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010,2011
 * NVIDIA Corporation <www.nvidia.com>
 */

#include <common.h>
#include <init.h>
#include <linux/ctype.h>
#if defined(CONFIG_TEGRA124) || defined(CONFIG_TEGRA30)
#include <asm/arch-tegra/pmc.h>

static char *get_reset_cause(void)
{
	struct pmc_ctlr *pmc = (struct pmc_ctlr *)NV_PA_PMC_BASE;

	switch (pmc->pmc_reset_status) {
	case 0x00:
		return "POR";
	case 0x01:
		return "WATCHDOG";
	case 0x02:
		return "SENSOR";
	case 0x03:
		return "SW_MAIN";
	case 0x04:
		return "LP0";
	}
	return "UNKNOWN";
}
#endif

/* Print CPU information */
int print_cpuinfo(void)
{
	printf("SoC: %s\n", CONFIG_SYS_SOC);
#if defined(CONFIG_TEGRA124) || defined(CONFIG_TEGRA30)
	printf("Reset cause: %s\n", get_reset_cause());
#endif

	/* TBD: Add printf of major/minor rev info, stepping, etc. */
	return 0;
}
