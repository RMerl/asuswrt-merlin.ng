// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#include <common.h>
#include "emc.h"
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/emc.h>
#include <asm/arch/tegra.h>
#include <asm/arch-tegra/ap.h>
#include <asm/arch-tegra/clk_rst.h>
#include <asm/arch-tegra/pmu.h>
#include <asm/arch-tegra/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

/* These rates are hard-coded for now, until fdt provides them */
#define EMC_SDRAM_RATE_T20	(333000 * 2 * 1000)
#define EMC_SDRAM_RATE_T25	(380000 * 2 * 1000)

int board_emc_init(void)
{
	unsigned rate;

	switch (tegra_get_chip_sku()) {
	default:
	case TEGRA_SOC_T20:
		rate  = EMC_SDRAM_RATE_T20;
		break;
	case TEGRA_SOC_T25:
		rate  = EMC_SDRAM_RATE_T25;
		break;
	}
	return tegra_set_emc(gd->fdt_blob, rate);
}
