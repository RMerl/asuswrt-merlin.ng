// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Copyright 2020 Hitachi Power Grids. All rights reserved.
 */

#include <common.h>
#include <fsl_ddr_sdram.h>
#include <fsl_ddr_dimm_params.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/global_data.h>
#include <asm/arch/ls102xa_soc.h>

DECLARE_GLOBAL_DATA_PTR;

void fsl_ddr_board_options(memctl_options_t *popts,
			   dimm_params_t *pdimm,
			   unsigned int ctrl_num)
{
	if (ctrl_num > 1) {
		printf("Not supported controller number %d\n", ctrl_num);
		return;
	}

	// 1/2 DRAM cycle (should be increased in case of ADDR/CMD heavily loaded than the clock)
	popts->clk_adjust = 0x4;
	popts->write_data_delay = 0x4;
	// wr leveling start value for lane 0
	popts->wrlvl_start = 0x5;
	// wr leveling start values for lanes 1-3 (lane 4 not there)
	popts->wrlvl_ctl_2 = 0x05050500;
	// 32-bit DRAM, no need to set start values for lanes we do not have (5-8)
	popts->wrlvl_ctl_3 = 0x0;
	popts->cpo_override = 0x1f;

	/* force DDR bus width to 32 bits */
	popts->data_bus_width = 1;
	popts->otf_burst_chop_en = 0;
	popts->burst_length = DDR_BL8;

	/*
	 * Factors to consider for half-strength driver enable:
	 *	- number of DIMMs installed
	 */
	popts->half_strength_driver_enable = 1;
	/*
	 * Write leveling override
	 */
	popts->wrlvl_override = 1;
	popts->wrlvl_sample = 0xf;

	/*
	 * Rtt and Rtt_WR override
	 */
	popts->rtt_override = 0;

	/* Enable ZQ calibration */
	popts->zq_en = 1;

	popts->cswl_override = DDR_CSWL_CS0;

	/* optimize cpo for erratum A-009942 */
	popts->cpo_sample = 0x58;

	/* DHC_EN =1, ODT = 75 Ohm */
	popts->ddr_cdr1 = DDR_CDR1_DHC_EN | DDR_CDR1_ODT(DDR_CDR_ODT_75ohm);
	popts->ddr_cdr2 = DDR_CDR2_ODT(DDR_CDR_ODT_75ohm);
}

int fsl_initdram(void)
{
	phys_size_t dram_size;

	puts("Initializing DDR....using SPD\n");
	dram_size = fsl_ddr_sdram();

	erratum_a008850_post();

	gd->ram_size = dram_size;

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = gd->ram_size;

	return 0;
}
