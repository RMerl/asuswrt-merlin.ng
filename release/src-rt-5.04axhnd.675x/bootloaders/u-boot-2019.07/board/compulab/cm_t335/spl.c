// SPDX-License-Identifier: GPL-2.0+
/*
 * SPL specific code for Compulab CM-T335 board
 *
 * Board functions for Compulab CM-T335 board
 *
 * Copyright (C) 2013, Compulab Ltd - http://compulab.co.il/
 *
 * Author: Ilya Ledvich <ilya@compulab.co.il>
 */

#include <common.h>
#include <errno.h>

#include <asm/arch/ddr_defs.h>
#include <asm/arch/clock.h>
#include <asm/arch/clocks_am33xx.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/hardware_am33xx.h>
#include <linux/sizes.h>

const struct ctrl_ioregs ioregs = {
	.cm0ioctl		= MT41J128MJT125_IOCTRL_VALUE,
	.cm1ioctl		= MT41J128MJT125_IOCTRL_VALUE,
	.cm2ioctl		= MT41J128MJT125_IOCTRL_VALUE,
	.dt0ioctl		= MT41J128MJT125_IOCTRL_VALUE,
	.dt1ioctl		= MT41J128MJT125_IOCTRL_VALUE,
};

static const struct ddr_data ddr3_data = {
	.datardsratio0		= MT41J128MJT125_RD_DQS,
	.datawdsratio0		= MT41J128MJT125_WR_DQS,
	.datafwsratio0		= MT41J128MJT125_PHY_FIFO_WE,
	.datawrsratio0		= MT41J128MJT125_PHY_WR_DATA,
};

static const struct cmd_control ddr3_cmd_ctrl_data = {
	.cmd0csratio		= MT41J128MJT125_RATIO,
	.cmd0iclkout		= MT41J128MJT125_INVERT_CLKOUT,

	.cmd1csratio		= MT41J128MJT125_RATIO,
	.cmd1iclkout		= MT41J128MJT125_INVERT_CLKOUT,

	.cmd2csratio		= MT41J128MJT125_RATIO,
	.cmd2iclkout		= MT41J128MJT125_INVERT_CLKOUT,
};

static struct emif_regs ddr3_emif_reg_data = {
	.sdram_config		= MT41J128MJT125_EMIF_SDCFG,
	.ref_ctrl		= MT41J128MJT125_EMIF_SDREF,
	.sdram_tim1		= MT41J128MJT125_EMIF_TIM1,
	.sdram_tim2		= MT41J128MJT125_EMIF_TIM2,
	.sdram_tim3		= MT41J128MJT125_EMIF_TIM3,
	.zq_config		= MT41J128MJT125_ZQ_CFG,
	.emif_ddr_phy_ctlr_1	= MT41J128MJT125_EMIF_READ_LATENCY |
					PHY_EN_DYN_PWRDN,
};

const struct dpll_params dpll_ddr = {
/*       M           N            M2  M3  M4  M5  M6 */
	303, (V_OSCK/1000000) - 1, 1, -1, -1, -1, -1};

void am33xx_spl_board_init(void)
{
	struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

	/* Get the frequency */
	dpll_mpu_opp100.m = am335x_get_efuse_mpu_max_freq(cdev);

	/* Set CORE Frequencies to OPP100 */
	do_setup_dpll(&dpll_core_regs, &dpll_core_opp100);

	/* Set MPU Frequency to what we detected now that voltages are set */
	do_setup_dpll(&dpll_mpu_regs, &dpll_mpu_opp100);
}

const struct dpll_params *get_dpll_ddr_params(void)
{
	return &dpll_ddr;
}

static void probe_sdram_size(long size)
{
	switch (size) {
	case SZ_512M:
		ddr3_emif_reg_data.sdram_config = MT41J256MJT125_EMIF_SDCFG;
		break;
	case SZ_256M:
		ddr3_emif_reg_data.sdram_config = MT41J128MJT125_EMIF_SDCFG;
		break;
	case SZ_128M:
		ddr3_emif_reg_data.sdram_config = MT41J64MJT125_EMIF_SDCFG;
		break;
	default:
		puts("Failed configuring DRAM, resetting...\n\n");
		reset_cpu(0);
	}
	debug("%s: setting DRAM size to %ldM\n", __func__, size >> 20);
	config_ddr(303, &ioregs, &ddr3_data,
		   &ddr3_cmd_ctrl_data, &ddr3_emif_reg_data, 0);
}

void sdram_init(void)
{
	long size = SZ_1G;

	do {
		size = size / 2;
		probe_sdram_size(size);
	} while (get_ram_size((void *)CONFIG_SYS_SDRAM_BASE, size) < size);

	return;
}
