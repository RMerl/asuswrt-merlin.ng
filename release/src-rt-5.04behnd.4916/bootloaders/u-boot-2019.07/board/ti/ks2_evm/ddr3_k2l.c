// SPDX-License-Identifier: GPL-2.0+
/*
 * Keystone2: DDR3 initialization
 *
 * (C) Copyright 2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <common.h>
#include "ddr3_cfg.h"
#include <asm/arch/ddr3.h>

static struct pll_init_data ddr3_400 = DDR3_PLL_400;

u32 ddr3_init(void)
{
	init_pll(&ddr3_400);

	/* No SO-DIMM, 2GB discreet DDR */
	printf("DRAM: 2 GiB\n");

	/* Reset DDR3 PHY after PLL enabled */
	ddr3_reset_ddrphy();

	ddr3_init_ddrphy(KS2_DDR3A_DDRPHYC, &ddr3phy_1600_2g);
	ddr3_init_ddremif(KS2_DDR3A_EMIF_CTRL_BASE, &ddr3_1600_2g);

	return 2;
}
