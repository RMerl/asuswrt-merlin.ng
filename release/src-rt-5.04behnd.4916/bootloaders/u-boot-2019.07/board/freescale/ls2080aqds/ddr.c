// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <fsl_ddr_sdram.h>
#include <fsl_ddr_dimm_params.h>
#include <asm/arch/soc.h>
#include <asm/arch/clock.h>
#include "ddr.h"

DECLARE_GLOBAL_DATA_PTR;

void fsl_ddr_board_options(memctl_options_t *popts,
				dimm_params_t *pdimm,
				unsigned int ctrl_num)
{
#ifdef CONFIG_SYS_FSL_HAS_DP_DDR
	u8 dq_mapping_0, dq_mapping_2, dq_mapping_3;
#endif
	const struct board_specific_parameters *pbsp, *pbsp_highest = NULL;
	ulong ddr_freq;
	int slot;

	if (ctrl_num > 2) {
		printf("Not supported controller number %d\n", ctrl_num);
		return;
	}

	for (slot = 0; slot < CONFIG_DIMM_SLOTS_PER_CTLR; slot++) {
		if (pdimm[slot].n_ranks)
			break;
	}

	if (slot >= CONFIG_DIMM_SLOTS_PER_CTLR)
		return;

	/*
	 * we use identical timing for all slots. If needed, change the code
	 * to  pbsp = rdimms[ctrl_num] or pbsp = udimms[ctrl_num];
	 */
	if (popts->registered_dimm_en)
		pbsp = rdimms[ctrl_num];
	else
		pbsp = udimms[ctrl_num];


	/* Get clk_adjust, wrlvl_start, wrlvl_ctl, according to the board ddr
	 * freqency and n_banks specified in board_specific_parameters table.
	 */
	ddr_freq = get_ddr_freq(ctrl_num) / 1000000;
	while (pbsp->datarate_mhz_high) {
		if (pbsp->n_ranks == pdimm[slot].n_ranks &&
		    (pdimm[slot].rank_density >> 30) >= pbsp->rank_gb) {
			if (ddr_freq <= pbsp->datarate_mhz_high) {
				popts->clk_adjust = pbsp->clk_adjust;
				popts->wrlvl_start = pbsp->wrlvl_start;
				popts->wrlvl_ctl_2 = pbsp->wrlvl_ctl_2;
				popts->wrlvl_ctl_3 = pbsp->wrlvl_ctl_3;
				goto found;
			}
			pbsp_highest = pbsp;
		}
		pbsp++;
	}

	if (pbsp_highest) {
		printf("Error: board specific timing not found for data rate %lu MT/s\n"
			"Trying to use the highest speed (%u) parameters\n",
			ddr_freq, pbsp_highest->datarate_mhz_high);
		popts->clk_adjust = pbsp_highest->clk_adjust;
		popts->wrlvl_start = pbsp_highest->wrlvl_start;
		popts->wrlvl_ctl_2 = pbsp->wrlvl_ctl_2;
		popts->wrlvl_ctl_3 = pbsp->wrlvl_ctl_3;
	} else {
		panic("DIMM is not supported by this board");
	}
found:
	debug("Found timing match: n_ranks %d, data rate %d, rank_gb %d\n"
		"\tclk_adjust %d, wrlvl_start %d, wrlvl_ctrl_2 0x%x, wrlvl_ctrl_3 0x%x\n",
		pbsp->n_ranks, pbsp->datarate_mhz_high, pbsp->rank_gb,
		pbsp->clk_adjust, pbsp->wrlvl_start, pbsp->wrlvl_ctl_2,
		pbsp->wrlvl_ctl_3);
#ifdef CONFIG_SYS_FSL_HAS_DP_DDR
	if (ctrl_num == CONFIG_DP_DDR_CTRL) {
		/* force DDR bus width to 32 bits */
		popts->data_bus_width = 1;
		popts->otf_burst_chop_en = 0;
		popts->burst_length = DDR_BL8;
		popts->bstopre = 0;	/* enable auto precharge */
		/*
		 * Layout optimization results byte mapping
		 * Byte 0 -> Byte ECC
		 * Byte 1 -> Byte 3
		 * Byte 2 -> Byte 2
		 * Byte 3 -> Byte 1
		 * Byte ECC -> Byte 0
		 */
		dq_mapping_0 = pdimm[slot].dq_mapping[0];
		dq_mapping_2 = pdimm[slot].dq_mapping[2];
		dq_mapping_3 = pdimm[slot].dq_mapping[3];
		pdimm[slot].dq_mapping[0] = pdimm[slot].dq_mapping[8];
		pdimm[slot].dq_mapping[1] = pdimm[slot].dq_mapping[9];
		pdimm[slot].dq_mapping[2] = pdimm[slot].dq_mapping[6];
		pdimm[slot].dq_mapping[3] = pdimm[slot].dq_mapping[7];
		pdimm[slot].dq_mapping[6] = dq_mapping_2;
		pdimm[slot].dq_mapping[7] = dq_mapping_3;
		pdimm[slot].dq_mapping[8] = dq_mapping_0;
		pdimm[slot].dq_mapping[9] = 0;
		pdimm[slot].dq_mapping[10] = 0;
		pdimm[slot].dq_mapping[11] = 0;
		pdimm[slot].dq_mapping[12] = 0;
		pdimm[slot].dq_mapping[13] = 0;
		pdimm[slot].dq_mapping[14] = 0;
		pdimm[slot].dq_mapping[15] = 0;
		pdimm[slot].dq_mapping[16] = 0;
		pdimm[slot].dq_mapping[17] = 0;
	}
#endif
	/* To work at higher than 1333MT/s */
	popts->half_strength_driver_enable = 0;
	/*
	 * Write leveling override
	 */
	popts->wrlvl_override = 1;
	popts->wrlvl_sample = 0x0;	/* 32 clocks */

	/*
	 * Rtt and Rtt_WR override
	 */
	popts->rtt_override = 0;

	/* Enable ZQ calibration */
	popts->zq_en = 1;

	if (ddr_freq < 2350) {
		if (pdimm[0].n_ranks == 2 && pdimm[1].n_ranks == 2) {
			/* four chip-selects */
			popts->ddr_cdr1 = DDR_CDR1_DHC_EN |
					  DDR_CDR1_ODT(DDR_CDR_ODT_80ohm);
			popts->ddr_cdr2 = DDR_CDR2_ODT(DDR_CDR_ODT_80ohm);
			popts->twot_en = 1; /* enable 2T timing */
		} else {
			popts->ddr_cdr1 = DDR_CDR1_DHC_EN |
					  DDR_CDR1_ODT(DDR_CDR_ODT_60ohm);
			popts->ddr_cdr2 = DDR_CDR2_ODT(DDR_CDR_ODT_60ohm) |
					  DDR_CDR2_VREF_RANGE_2;
		}
	} else {
		popts->ddr_cdr1 = DDR_CDR1_DHC_EN |
				  DDR_CDR1_ODT(DDR_CDR_ODT_100ohm);
		popts->ddr_cdr2 = DDR_CDR2_ODT(DDR_CDR_ODT_100ohm) |
				  DDR_CDR2_VREF_RANGE_2;
	}
}

#ifdef CONFIG_TFABOOT
int fsl_initdram(void)
{
	gd->ram_size = tfa_get_dram_size();

	if (!gd->ram_size)
		gd->ram_size = fsl_ddr_sdram_size();

	return 0;
}
#else
int fsl_initdram(void)
{
#if defined(CONFIG_SPL) && !defined(CONFIG_SPL_BUILD)
	gd->ram_size = fsl_ddr_sdram_size();
#else
	puts("Initializing DDR....using SPD\n");

	gd->ram_size = fsl_ddr_sdram();
#endif

	return 0;
}
#endif /* CONFIG_TFABOOT */
