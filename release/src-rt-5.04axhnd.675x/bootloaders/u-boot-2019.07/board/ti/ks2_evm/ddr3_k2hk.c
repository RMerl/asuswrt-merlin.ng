// SPDX-License-Identifier: GPL-2.0+
/*
 * Keystone2: DDR3 initialization
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <common.h>
#include "ddr3_cfg.h"
#include <asm/arch/ddr3.h>
#include <asm/arch/hardware.h>

struct pll_init_data ddr3a_333 = DDR3_PLL_333(A);
struct pll_init_data ddr3a_400 = DDR3_PLL_400(A);

u32 ddr3_init(void)
{
	u32 ddr3_size;
	struct ddr3_spd_cb spd_cb;

	if (ddr3_get_dimm_params_from_spd(&spd_cb)) {
		printf("Sorry, I don't know how to configure DDR3A.\n"
		       "Bye :(\n");
		for (;;)
			;
	}

	printf("Detected SO-DIMM [%s]\n", spd_cb.dimm_name);

	if ((cpu_revision() > 1) ||
	    (__raw_readl(KS2_RSTCTRL_RSTYPE) & 0x1)) {
		printf("DDR3 speed %d\n", spd_cb.ddrspdclock);
		if (spd_cb.ddrspdclock == 1600)
			init_pll(&ddr3a_400);
		else
			init_pll(&ddr3a_333);
	}

	if (cpu_revision() > 0) {
		if (cpu_revision() > 1) {
			/* PG 2.0 */
			/* Reset DDR3A PHY after PLL enabled */
			ddr3_reset_ddrphy();
			spd_cb.phy_cfg.zq0cr1 |= 0x10000;
			spd_cb.phy_cfg.zq1cr1 |= 0x10000;
			spd_cb.phy_cfg.zq2cr1 |= 0x10000;
		}
		ddr3_init_ddrphy(KS2_DDR3A_DDRPHYC, &spd_cb.phy_cfg);

		ddr3_init_ddremif(KS2_DDR3A_EMIF_CTRL_BASE, &spd_cb.emif_cfg);

		ddr3_size = spd_cb.ddr_size_gbyte;
	} else {
		ddr3_init_ddrphy(KS2_DDR3A_DDRPHYC, &spd_cb.phy_cfg);
		spd_cb.emif_cfg.sdcfg |= 0x1000;
		ddr3_init_ddremif(KS2_DDR3A_EMIF_CTRL_BASE, &spd_cb.emif_cfg);
		ddr3_size = spd_cb.ddr_size_gbyte / 2;
	}
	printf("DRAM: %d GiB (includes reported below)\n", ddr3_size);

	/* Apply the workaround for PG 1.0 and 1.1 Silicons */
	if (cpu_revision() <= 1)
		ddr3_err_reset_workaround();

	return ddr3_size;
}
