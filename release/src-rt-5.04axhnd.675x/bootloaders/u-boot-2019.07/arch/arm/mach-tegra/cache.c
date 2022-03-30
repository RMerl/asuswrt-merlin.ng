// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2013-2014, NVIDIA CORPORATION.  All rights reserved.
 */

/* Tegra cache routines */

#include <common.h>
#include <asm/io.h>
#include <asm/arch-tegra/ap.h>
#if IS_ENABLED(CONFIG_TEGRA_GP_PADCTRL)
#include <asm/arch/gp_padctrl.h>
#endif

#ifndef CONFIG_ARM64
void config_cache(void)
{
	u32 reg = 0;

	/* enable SMP mode and FW for CPU0, by writing to Auxiliary Ctl reg */
	asm volatile(
		"mrc p15, 0, r0, c1, c0, 1\n"
		"orr r0, r0, #0x41\n"
		"mcr p15, 0, r0, c1, c0, 1\n");

	/* Currently, only Tegra114+ needs this L2 cache change to boot Linux */
	if (tegra_get_chip() < CHIPID_TEGRA114)
		return;

	/*
	 * Systems with an architectural L2 cache must not use the PL310.
	 * Config L2CTLR here for a data RAM latency of 3 cycles.
	 */
	asm("mrc p15, 1, %0, c9, c0, 2" : : "r" (reg));
	reg &= ~7;
	reg |= 2;
	asm("mcr p15, 1, %0, c9, c0, 2" : : "r" (reg));
}
#endif
