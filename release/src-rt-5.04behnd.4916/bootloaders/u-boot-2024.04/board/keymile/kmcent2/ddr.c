// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Keymile AG
 * Rainer Boschung <rainer.boschung@keymile.com>
 *
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <asm/fsl_law.h>
#include <asm/mmu.h>
#include <asm/mpc85xx_gpio.h>
#include <fsl_ddr_sdram.h>
#include <fsl_ddr_dimm_params.h>
#include <hwconfig.h>
#include <i2c.h>
#include <init.h>

DECLARE_GLOBAL_DATA_PTR;

#define DQSn_POS(n)		(3 - (((n) - 1) % 4)) * 8
#define DQSn_START(n, start)	((start) << DQSn_POS(n))

void fsl_ddr_board_options(memctl_options_t *popts, dimm_params_t *pdimm,
			   unsigned int ctrl_num)
{
	if (ctrl_num > 1) {
		printf("Not supported controller number %d\n", ctrl_num);
		return;
	}

	/* 1/2 clk delay between wr command and data strobe */
	popts->write_data_delay = 4;
	/* clk lauched 1/2 applied cylcle after address command */
	popts->clk_adjust = 4;
	/* 1T timing: command/address held for only 1 cycle */
	popts->twot_en = 0;
	popts->threet_en = 0;

	/* optimize cpo for erratum A-009942 */
	popts->cpo_sample = 0x3b;

	/* we have only one module, half str should be OK */
	popts->half_strength_driver_enable = 1;
	/*
	 * Write leveling override
	 */
	/* set for DDR3-1600 */
	popts->wrlvl_override = 1;
	popts->wrlvl_sample = 0xf;
	popts->wrlvl_start = 0x7;
	/* DQS write leveling start time according layout */
	popts->wrlvl_ctl_2 = (DQSn_START(1, 0x06) |
			      DQSn_START(2, 0x06) |
			      DQSn_START(3, 0x07) |
			      DQSn_START(4, 0x07));
	popts->wrlvl_ctl_3 = (DQSn_START(5, 0x07) |
			      DQSn_START(6, 0x08) |
			      DQSn_START(7, 0x08) |
			      DQSn_START(8, 0x08));

	/*
	 * rtt and wtt_wr override
	 */
	popts->rtt_override = 0;

	/* Enable ZQ calibration */
	popts->zq_en = 1;

	/* DHC_EN =1, ODT = 75 Ohm */
	popts->ddr_cdr1 = DDR_CDR1_DHC_EN | DDR_CDR1_ODT(DDR_CDR_ODT_75ohm);
	popts->ddr_cdr2 = DDR_CDR2_ODT(DDR_CDR_ODT_75ohm);
}

int dram_init(void)
{
	phys_size_t dram_size;

	puts("Initializing....using SPD\n");

	dram_size = fsl_ddr_sdram();

	dram_size = setup_ddr_tlbs(dram_size / 0x100000);
	dram_size *= 0x100000;

	gd->ram_size = dram_size;

	return 0;
}
