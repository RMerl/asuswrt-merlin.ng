// SPDX-License-Identifier: GPL-2.0+
/*
 * Based on corenet_ds ddr code
 */

#include <common.h>
#include <i2c.h>
#include <hwconfig.h>
#include <asm/mmu.h>
#include <fsl_ddr_sdram.h>
#include <fsl_ddr_dimm_params.h>
#include <asm/fsl_law.h>

DECLARE_GLOBAL_DATA_PTR;


struct board_specific_parameters {
	u32 n_ranks;
	u32 datarate_mhz_high;
	u32 clk_adjust;
	u32 wrlvl_start;
	u32 cpo;
	u32 write_data_delay;
	u32 force_2t;
};

/*
 * This table contains all valid speeds we want to override with board
 * specific parameters. datarate_mhz_high values need to be in ascending order
 * for each n_ranks group.
 */
static const struct board_specific_parameters udimm0[] = {
	/*
	 * memory controller 0
	 *   num|  hi|  clk| wrlvl | cpo  |wrdata|2T
	 * ranks| mhz|adjst| start |      |delay |
	 */
	{4,   850,    4,     6,   0xff,    2,  0},
	{4,   950,    5,     7,   0xff,    2,  0},
	{4,  1050,    5,     8,   0xff,    2,  0},
	{4,  1250,    5,    10,   0xff,    2,  0},
	{4,  1350,    5,    11,   0xff,    2,  0},
	{4,  1666,    5,    12,   0xff,    2,  0},
	{2,   850,    5,     6,   0xff,    2,  0},
	{2,  1050,    5,     7,   0xff,    2,  0},
	{2,  1250,    4,     6,   0xff,    2,  0},
	{2,  1350,    5,     7,   0xff,    2,  0},
	{2,  1666,    5,     8,   0xff,    2,  0},
	{1,  1250,    4,     6,   0xff,    2,  0},
	{1,  1335,    4,     7,   0xff,    2,  0},
	{1,  1666,    4,     8,   0xff,    2,  0},
	{}
};

/*
 * The two slots have slightly different timing. The center values are good
 * for both slots. We use identical speed tables for them. In future use, if
 * DIMMs have fewer center values that require two separated tables, copy the
 * udimm0 table to udimm1 and make changes to clk_adjust and wrlvl_start.
 */
static const struct board_specific_parameters *udimms[] = {
	udimm0,
	udimm0,
};

static const struct board_specific_parameters rdimm0[] = {
	/*
	 * memory controller 0
	 *   num|  hi|  clk| wrlvl | cpo  |wrdata|2T
	 * ranks| mhz|adjst| start |      |delay |
	 */
	{4,   850,    4,     6,   0xff,    2,  0},
	{4,   950,    5,     7,   0xff,    2,  0},
	{4,  1050,    5,     8,   0xff,    2,  0},
	{4,  1250,    5,    10,   0xff,    2,  0},
	{4,  1350,    5,    11,   0xff,    2,  0},
	{4,  1666,    5,    12,   0xff,    2,  0},
	{2,   850,    4,     6,   0xff,    2,  0},
	{2,  1050,    4,     7,   0xff,    2,  0},
	{2,  1666,    4,     8,   0xff,    2,  0},
	{1,   850,    4,     5,   0xff,    2,  0},
	{1,   950,    4,     7,   0xff,    2,  0},
	{1,  1666,    4,     8,   0xff,    2,  0},
	{}
};

/*
 * The two slots have slightly different timing. See comments above.
 */
static const struct board_specific_parameters *rdimms[] = {
	rdimm0,
	rdimm0,
};

void fsl_ddr_board_options(memctl_options_t *popts,
				dimm_params_t *pdimm,
				unsigned int ctrl_num)
{
	const struct board_specific_parameters *pbsp, *pbsp_highest = NULL;
	ulong ddr_freq;

	if (ctrl_num > 1) {
		printf("Wrong parameter for controller number %d", ctrl_num);
		return;
	}
	if (!pdimm->n_ranks)
		return;

	if (popts->registered_dimm_en)
		pbsp = rdimms[ctrl_num];
	else
		pbsp = udimms[ctrl_num];


	/* Get clk_adjust, cpo, write_data_delay,2T, according to the board ddr
	 * freqency and n_banks specified in board_specific_parameters table.
	 */
	ddr_freq = get_ddr_freq(0) / 1000000;
	while (pbsp->datarate_mhz_high) {
		if (pbsp->n_ranks == pdimm->n_ranks) {
			if (ddr_freq <= pbsp->datarate_mhz_high) {
				popts->cpo_override = pbsp->cpo;
				popts->write_data_delay =
					pbsp->write_data_delay;
				popts->clk_adjust = pbsp->clk_adjust;
				popts->wrlvl_start = pbsp->wrlvl_start;
				popts->twot_en = pbsp->force_2t;
				goto found;
			}
			pbsp_highest = pbsp;
		}
		pbsp++;
	}

	if (pbsp_highest) {
		printf("Error: board specific timing not found for data rate %lu MT/s!\nTrying to use the highest speed (%u) parameters\n",
			ddr_freq, pbsp_highest->datarate_mhz_high);
		popts->cpo_override = pbsp_highest->cpo;
		popts->write_data_delay = pbsp_highest->write_data_delay;
		popts->clk_adjust = pbsp_highest->clk_adjust;
		popts->wrlvl_start = pbsp_highest->wrlvl_start;
		popts->twot_en = pbsp_highest->force_2t;
	} else {
		panic("DIMM is not supported by this board");
	}
found:
	/*
	 * Factors to consider for half-strength driver enable:
	 *	- number of DIMMs installed
	 */
	popts->half_strength_driver_enable = 0;
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

	/* DHC_EN =1, ODT = 60 Ohm */
	popts->ddr_cdr1 = DDR_CDR1_DHC_EN;
}

int dram_init(void)
{
	phys_size_t dram_size;

	puts("Initializing....");

	if (!fsl_use_spd())
		panic("Cyrus only supports using SPD for DRAM\n");

	puts("using SPD\n");
	dram_size = fsl_ddr_sdram();

	dram_size = setup_ddr_tlbs(dram_size / 0x100000);
	dram_size *= 0x100000;

	debug("    DDR: ");
	gd->ram_size = dram_size;

	return 0;
}
