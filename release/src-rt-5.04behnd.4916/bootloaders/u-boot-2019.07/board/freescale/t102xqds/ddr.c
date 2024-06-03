// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <i2c.h>
#include <hwconfig.h>
#include <asm/mmu.h>
#include <fsl_ddr_sdram.h>
#include <fsl_ddr_dimm_params.h>
#include <asm/fsl_law.h>
#include <asm/mpc85xx_gpio.h>

DECLARE_GLOBAL_DATA_PTR;

struct board_specific_parameters {
	u32 n_ranks;
	u32 datarate_mhz_high;
	u32 rank_gb;
	u32 clk_adjust;
	u32 wrlvl_start;
	u32 wrlvl_ctl_2;
	u32 wrlvl_ctl_3;
};

/*
 * datarate_mhz_high values need to be in ascending order
 */
static const struct board_specific_parameters udimm0[] = {
	/*
	 * memory controller 0
	 *   num|  hi| rank|  clk| wrlvl |   wrlvl   |  wrlvl |
	 * ranks| mhz| GB  |adjst| start |   ctl2    |  ctl3  |
	 */
#if defined(CONFIG_SYS_FSL_DDR4)
	{2,  1666,  0,  8,  7,  0x0808090B,  0x0C0D0E0A,},
	{2,  1900,  0,  8,  6,  0x08080A0C,  0x0D0E0F0A,},
	{1,  1666,  0,  8,  6,  0x0708090B,  0x0C0D0E09,},
	{1,  1900,  0,  8,  6,  0x08080A0C,  0x0D0E0F0A,},
	{1,  2200,  0,  8,  7,  0x08090A0D,  0x0F0F100C,},
#elif defined(CONFIG_SYS_FSL_DDR3)
	{2,  833,   0,  8,  6,  0x06060607,  0x08080807,},
	{2,  1350,  0,  8,  7,  0x0708080A,  0x0A0B0C09,},
	{2,  1666,  0,  8,  7,  0x0808090B,  0x0C0D0E0A,},
	{1,  833,   0,  8,  6,  0x06060607,  0x08080807,},
	{1,  1350,  0,  8,  7,  0x0708080A,  0x0A0B0C09,},
	{1,  1666,  0,  8,  7,  0x0808090B,  0x0C0D0E0A,},
#else
#error DDR type not defined
#endif
	{}
};

static const struct board_specific_parameters *udimms[] = {
	udimm0,
};

void fsl_ddr_board_options(memctl_options_t *popts,
			   dimm_params_t *pdimm,
			   unsigned int ctrl_num)
{
	const struct board_specific_parameters *pbsp, *pbsp_highest = NULL;
	ulong ddr_freq;
	struct cpu_type *cpu = gd->arch.cpu;

	if (ctrl_num > 2) {
		printf("Not supported controller number %d\n", ctrl_num);
		return;
	}
	if (!pdimm->n_ranks)
		return;

	pbsp = udimms[0];

	/* Get clk_adjust according to the board ddr freqency and n_banks
	 * specified in board_specific_parameters table.
	 */
	ddr_freq = get_ddr_freq(0) / 1000000;
	while (pbsp->datarate_mhz_high) {
		if (pbsp->n_ranks == pdimm->n_ranks &&
		    (pdimm->rank_density >> 30) >= pbsp->rank_gb) {
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
		printf("Error: board specific timing not found\n");
		printf("for data rate %lu MT/s\n", ddr_freq);
		printf("Trying to use the highest speed (%u) parameters\n",
		       pbsp_highest->datarate_mhz_high);
		popts->clk_adjust = pbsp_highest->clk_adjust;
		popts->wrlvl_start = pbsp_highest->wrlvl_start;
		popts->wrlvl_ctl_2 = pbsp->wrlvl_ctl_2;
		popts->wrlvl_ctl_3 = pbsp->wrlvl_ctl_3;
	} else {
		panic("DIMM is not supported by this board");
	}
found:
	debug("Found timing match: n_ranks %d, data rate %d, rank_gb %d\n",
	      pbsp->n_ranks, pbsp->datarate_mhz_high, pbsp->rank_gb);
	debug("\tclk_adjust %d, wrlvl_start %d, wrlvl_ctrl_2 0x%x, ",
	      pbsp->clk_adjust, pbsp->wrlvl_start, pbsp->wrlvl_ctl_2);
	debug("wrlvl_ctrl_3 0x%x\n", pbsp->wrlvl_ctl_3);

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
	 * rtt and rtt_wr override
	 */
	popts->rtt_override = 0;

	/* Enable ZQ calibration */
	popts->zq_en = 1;

	/* DHC_EN =1, ODT = 75 Ohm */
#ifdef CONFIG_SYS_FSL_DDR4
	popts->ddr_cdr1 = DDR_CDR1_DHC_EN | DDR_CDR1_ODT(DDR_CDR_ODT_80ohm);
	popts->ddr_cdr2 = DDR_CDR2_ODT(DDR_CDR_ODT_80ohm) |
			  DDR_CDR2_VREF_OVRD(70);	/* Vref = 70% */
#else
	popts->ddr_cdr1 = DDR_CDR1_DHC_EN | DDR_CDR1_ODT(DDR_CDR_ODT_75ohm);
	popts->ddr_cdr2 = DDR_CDR2_ODT(DDR_CDR_ODT_75ohm);

	/* optimize cpo for erratum A-009942 */
	popts->cpo_sample = 0x5f;
#endif

	/* T1023 supports max DDR bus 32bit width, T1024 supports DDR 64bit,
	 * set DDR bus width to 32bit for T1023
	 */
	if (cpu->soc_ver == SVR_T1023)
		popts->data_bus_width = DDR_DATA_BUS_WIDTH_32;

#ifdef CONFIG_FORCE_DDR_DATA_BUS_WIDTH_32
	/* for DDR bus 32bit test on T1024 */
	popts->data_bus_width = DDR_DATA_BUS_WIDTH_32;
#endif
}

#if defined(CONFIG_DEEP_SLEEP)
void board_mem_sleep_setup(void)
{
	void __iomem *qixis_base = (void *)QIXIS_BASE;

	/* does not provide HW signals for power management */
	clrbits_8(qixis_base + 0x21, 0x2);
	/* Disable MCKE isolation */
	gpio_set_value(2, 0);
	udelay(1);
}
#endif

int dram_init(void)
{
	phys_size_t dram_size;

#if defined(CONFIG_SPL_BUILD) || !defined(CONFIG_RAMBOOT_PBL)
	puts("Initializing....using SPD\n");
	dram_size = fsl_ddr_sdram();
#else
	/* DDR has been initialised by first stage boot loader */
	dram_size =  fsl_ddr_sdram_size();
#endif
	dram_size = setup_ddr_tlbs(dram_size / 0x100000);
	dram_size *= 0x100000;

#if defined(CONFIG_DEEP_SLEEP) && !defined(CONFIG_SPL_BUILD)
	fsl_dp_resume();
#endif

	gd->ram_size = dram_size;

	return 0;
}
