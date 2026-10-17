// SPDX-License-Identifier: GPL-2.0+
/*
 * board.c
 *
 * Board functions for Phytec phyCORE-AM335x R2 (PCL060 / PCM060) based boards
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - https://www.ti.com/
 * Copyright (C) 2013 Lars Poeschel, Lemonage Software GmbH
 * Copyright (C) 2015 Wadim Egorov, PHYTEC Messtechnik GmbH
 * Copyright (C) 2019 DENX Software Engineering GmbH
 */

#include <common.h>
#include <init.h>
#include <spl.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <power/tps65910.h>
#include <jffs2/load_kernel.h>
#include <mtd_node.h>
#include <fdt_support.h>
#include "board.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SPL_BUILD

static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

/* DDR RAM defines */
#if defined(CONFIG_TARGET_PCM051)
#define DDR_CLK_MHZ		303 /* DDR_DPLL_MULT value */
#else
#define DDR_CLK_MHZ		400 /* DDR_DPLL_MULT value */
#endif

#define OSC	(V_OSCK / 1000000)
const struct dpll_params dpll_ddr = {
		DDR_CLK_MHZ, OSC - 1, 1, -1, -1, -1, -1};

const struct dpll_params *get_dpll_ddr_params(void)
{
	return &dpll_ddr;
}

const struct ctrl_ioregs ioregs = {
	.cm0ioctl		= 0x18B,
	.cm1ioctl		= 0x18B,
	.cm2ioctl		= 0x18B,
	.dt0ioctl		= 0x18B,
	.dt1ioctl		= 0x18B,
};

static const struct cmd_control ddr3_cmd_ctrl_data = {
	.cmd0csratio = 0x80,
	.cmd0iclkout = 0x0,

	.cmd1csratio = 0x80,
	.cmd1iclkout = 0x0,

	.cmd2csratio = 0x80,
	.cmd2iclkout = 0x0,
};

enum {
	PHYCORE_R2_MT41K128M16JT_256MB,
	PHYCORE_R2_MT41K256M16TW107IT_512MB,
	PHYCORE_R2_MT41K512M16HA125IT_1024MB,
	PHYCORE_R13_MT41K256M16HA125E_256MB,
};

struct am335x_sdram_timings {
	struct emif_regs ddr3_emif_reg_data;
	struct ddr_data ddr3_data;
};

static struct am335x_sdram_timings physom_timings[] = {
	[PHYCORE_R2_MT41K128M16JT_256MB] = {
		.ddr3_emif_reg_data = {
			.sdram_config = 0x61C052B2,
			.ref_ctrl = 0x00000C30,
			.sdram_tim1 = 0x0AAAD4DB,
			.sdram_tim2 = 0x26437FDA,
			.sdram_tim3 = 0x501F83FF,
			.zq_config = 0x50074BE4,
			.emif_ddr_phy_ctlr_1 = 0x7,
			.ocp_config = 0x003d3d3d,
		},
		.ddr3_data = {
			.datardsratio0 = 0x36,
			.datawdsratio0 = 0x38,
			.datafwsratio0 = 0x99,
			.datawrsratio0 = 0x73,
		},
	},
	[PHYCORE_R2_MT41K256M16TW107IT_512MB] = {
		.ddr3_emif_reg_data = {
			.sdram_config = 0x61C05332,
			.ref_ctrl = 0x00000C30,
			.sdram_tim1 = 0x0AAAD4DB,
			.sdram_tim2 = 0x266B7FDA,
			.sdram_tim3 = 0x501F867F,
			.zq_config = 0x50074BE4,
			.emif_ddr_phy_ctlr_1 = 0x7,
			.ocp_config = 0x003d3d3d,
		},
		.ddr3_data = {
			.datardsratio0 = 0x37,
			.datawdsratio0 = 0x38,
			.datafwsratio0 = 0x92,
			.datawrsratio0 = 0x72,
		},
	},
	[PHYCORE_R2_MT41K512M16HA125IT_1024MB] = {
		.ddr3_emif_reg_data = {
			.sdram_config = 0x61C053B2,
			.ref_ctrl = 0x00000C30,
			.sdram_tim1 = 0x0AAAD4DB,
			.sdram_tim2 = 0x268F7FDA,
			.sdram_tim3 = 0x501F88BF,
			.zq_config = 0x50074BE4,
			.emif_ddr_phy_ctlr_1 = 0x7,
			.ocp_config = 0x003d3d3d,
		},
		.ddr3_data = {
			.datardsratio0 = 0x38,
			.datawdsratio0 = 0x4d,
			.datafwsratio0 = 0x9d,
			.datawrsratio0 = 0x82,
		},
	},
	[PHYCORE_R13_MT41K256M16HA125E_256MB] = {
		.ddr3_emif_reg_data = {
			.sdram_config = MT41K256M16HA125E_EMIF_SDCFG,
			.ref_ctrl = MT41K256M16HA125E_EMIF_SDREF,
			.sdram_tim1 = MT41K256M16HA125E_EMIF_TIM1,
			.sdram_tim2 = MT41K256M16HA125E_EMIF_TIM2,
			.sdram_tim3 = MT41K256M16HA125E_EMIF_TIM3,
			.zq_config = MT41K256M16HA125E_ZQ_CFG,
			.emif_ddr_phy_ctlr_1 = MT41K256M16HA125E_EMIF_READ_LATENCY | PHY_EN_DYN_PWRDN,
		},
		.ddr3_data = {
			.datardsratio0 = MT41K256M16HA125E_RD_DQS,
			.datawdsratio0 = MT41K256M16HA125E_WR_DQS,
			.datafwsratio0 = MT41K256M16HA125E_PHY_FIFO_WE,
			.datawrsratio0 = MT41K256M16HA125E_PHY_WR_DATA,
		},
	},
};

void sdram_init(void)
{
#if defined(CONFIG_TARGET_PCM051)
	int ram_type_index = PHYCORE_R13_MT41K256M16HA125E_256MB;
#else
	/* Configure memory to maximum supported size for detection */
	int ram_type_index = PHYCORE_R2_MT41K512M16HA125IT_1024MB;

	config_ddr(DDR_CLK_MHZ, &ioregs,
		   &physom_timings[ram_type_index].ddr3_data,
		   &ddr3_cmd_ctrl_data,
		   &physom_timings[ram_type_index].ddr3_emif_reg_data,
		   0);

	/* Detect memory physically present */
	gd->ram_size = get_ram_size((void *)CFG_SYS_SDRAM_BASE,
				    CFG_MAX_RAM_BANK_SIZE);

	/* Reconfigure memory for actual detected size */
	switch (gd->ram_size) {
	case SZ_1G:
		ram_type_index = PHYCORE_R2_MT41K512M16HA125IT_1024MB;
		break;
	case SZ_512M:
		ram_type_index = PHYCORE_R2_MT41K256M16TW107IT_512MB;
		break;
	case SZ_256M:
	default:
		ram_type_index = PHYCORE_R2_MT41K128M16JT_256MB;
		break;
	}
#endif
	config_ddr(DDR_CLK_MHZ, &ioregs,
		   &physom_timings[ram_type_index].ddr3_data,
		   &ddr3_cmd_ctrl_data,
		   &physom_timings[ram_type_index].ddr3_emif_reg_data,
		   0);
}

const struct dpll_params *get_dpll_mpu_params(void)
{
	int ind = get_sys_clk_index();
	int freq = am335x_get_efuse_mpu_max_freq(cdev);

	switch (freq) {
	case MPUPLL_M_1000:
		return &dpll_mpu_opp[ind][5];
	case MPUPLL_M_800:
		return &dpll_mpu_opp[ind][4];
	case MPUPLL_M_720:
		return &dpll_mpu_opp[ind][3];
	case MPUPLL_M_600:
		return &dpll_mpu_opp[ind][2];
	case MPUPLL_M_500:
		return &dpll_mpu_opp100;
	case MPUPLL_M_300:
		return &dpll_mpu_opp[ind][0];
	}

	return &dpll_mpu_opp[ind][0];
}

static void scale_vcores_generic(int freq)
{
	int sil_rev, mpu_vdd;

	/*
	 * We use a TPS65910 PMIC. For all  MPU frequencies we support we use a
	 * CORE voltage of 1.10V. For MPU voltage we need to switch based on
	 * the frequency we are running at.
	 */
	if (power_tps65910_init(0))
		return;

	/*
	 * Depending on MPU clock and PG we will need a different
	 * VDD to drive at that speed.
	 */
	sil_rev = readl(&cdev->deviceid) >> 28;
	mpu_vdd = am335x_get_tps65910_mpu_vdd(sil_rev, freq);

	/* Tell the TPS65910 to use i2c */
	tps65910_set_i2c_control();

	/* First update MPU voltage. */
	if (tps65910_voltage_update(MPU, mpu_vdd))
		return;

	/* Second, update the CORE voltage. */
	if (tps65910_voltage_update(CORE, TPS65910_OP_REG_SEL_1_1_0))
		return;
}

void scale_vcores(void)
{
	int freq;

	freq = am335x_get_efuse_mpu_max_freq(cdev);
	scale_vcores_generic(freq);
}

void set_uart_mux_conf(void)
{
	enable_uart0_pin_mux();
}

void set_mux_conf_regs(void)
{
	enable_i2c0_pin_mux();
	enable_board_pin_mux();
}
#endif

/*
 * Basic board specific setup.  Pinmux has been handled already.
 */
int board_init(void)
{
	gd->bd->bi_boot_params = CFG_SYS_SDRAM_BASE + 0x100;
	return 0;
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, struct bd_info *bd)
{
#ifdef CONFIG_FDT_FIXUP_PARTITIONS
	static const struct node_info nodes[] = {
		{ "ti,omap2-nand", MTD_DEV_TYPE_NAND, },
	};

	fdt_fixup_mtdparts(blob, nodes, ARRAY_SIZE(nodes));
#endif
	return 0;
}
#endif
