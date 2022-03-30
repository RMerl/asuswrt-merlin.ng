// SPDX-License-Identifier: GPL-2.0+
/*
 * Keystone2: DDR3 initialization
 *
 * (C) Copyright 2014-2015
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <common.h>
#include "ddr3_cfg.h"
#include <asm/arch/ddr3.h>

static struct pll_init_data ddr3_400 = DDR3_PLL_400;
static struct pll_init_data ddr3_333 = DDR3_PLL_333;

u32 ddr3_init(void)
{
	struct ddr3_spd_cb spd_cb;

	if (ddr3_get_dimm_params_from_spd(&spd_cb)) {
		printf("Sorry, I don't know how to configure DDR3A.\n"
		       "Bye :(\n");
		for (;;)
			;
	}

	printf("Detected SO-DIMM [%s]\n", spd_cb.dimm_name);

	printf("DDR3 speed %d\n", spd_cb.ddrspdclock);
	if (spd_cb.ddrspdclock == 1600)
		init_pll(&ddr3_400);
	else
		init_pll(&ddr3_333);

	/* Reset DDR3 PHY after PLL enabled */
	ddr3_reset_ddrphy();

	spd_cb.phy_cfg.zq0cr1 |= 0x10000;
	spd_cb.phy_cfg.zq1cr1 |= 0x10000;
	spd_cb.phy_cfg.zq2cr1 |= 0x10000;
	ddr3_init_ddrphy(KS2_DDR3A_DDRPHYC, &spd_cb.phy_cfg);
	ddr3_init_ddremif(KS2_DDR3A_EMIF_CTRL_BASE, &spd_cb.emif_cfg);

	printf("DRAM: %d GiB\n", spd_cb.ddr_size_gbyte);

	return (u32)spd_cb.ddr_size_gbyte;
}
