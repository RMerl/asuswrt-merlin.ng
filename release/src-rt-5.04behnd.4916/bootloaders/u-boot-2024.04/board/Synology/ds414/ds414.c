// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * Copyright (C) 2015 Phil Sutter <phil@nwl.cc>
 */

#include <common.h>
#include <init.h>
#include <miiphy.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <linux/bitops.h>
#include <linux/mbus.h>

#include "../drivers/ddr/marvell/axp/ddr3_hw_training.h"
#include "../arch/arm/mach-mvebu/serdes/axp/high_speed_env_spec.h"
#include "../arch/arm/mach-mvebu/serdes/axp/board_env_spec.h"

#include "cmd_syno.h"

DECLARE_GLOBAL_DATA_PTR;

/* GPP and MPP settings as found in mvBoardEnvSpec.c of Synology's U-Boot */

#define DS414_GPP_OUT_VAL_LOW		(BIT(25) | BIT(30))
#define DS414_GPP_OUT_VAL_MID		(BIT(10) | BIT(15))
#define DS414_GPP_OUT_VAL_HIGH		(0)

#define DS414_GPP_OUT_POL_LOW		(0)
#define DS414_GPP_OUT_POL_MID		(0)
#define DS414_GPP_OUT_POL_HIGH		(0)

#define DS414_GPP_OUT_ENA_LOW		(~(BIT(25) | BIT(30)))
#define DS414_GPP_OUT_ENA_MID		(~(BIT(10) | BIT(12) | \
					   BIT(13) | BIT(14) | BIT(15)))
#define DS414_GPP_OUT_ENA_HIGH		(~0)

static const u32 ds414_mpp_control[] = {
	0x11111111,
	0x22221111,
	0x22222222,
	0x00000000,
	0x11110000,
	0x00004000,
	0x00000000,
	0x00000000,
	0x00000000
};

/* DDR3 static MC configuration */

/* 1G_v1 (4x2Gbits) adapted by DS414 */
MV_DRAM_MC_INIT syno_ddr3_b0_667_1g_v1[MV_MAX_DDR3_STATIC_SIZE] = {
	{0x00001400, 0x73014A28},	/*DDR SDRAM Configuration Register */
	{0x00001404, 0x30000800},	/*Dunit Control Low Register */
	{0x00001408, 0x44148887},	/*DDR SDRAM Timing (Low) Register */
	{0x0000140C, 0x3AD83FEA},	/*DDR SDRAM Timing (High) Register */

	{0x00001410, 0x14000000},	/*DDR SDRAM Address Control Register */

	{0x00001414, 0x00000000},	/*DDR SDRAM Open Pages Control Register */
	{0x00001418, 0x00000e00},	/*DDR SDRAM Operation Register */
	{0x00001420, 0x00000004},	/*DDR SDRAM Extended Mode Register */
	{0x00001424, 0x0000F3FF},	/*Dunit Control High Register */
	{0x00001428, 0x000F8830},	/*Dunit Control High Register */
	{0x0000142C, 0x054C36F4},	/*Dunit Control High Register */
	{0x0000147C, 0x0000C671},

	{0x000014a0, 0x00000001},
	{0x000014a8, 0x00000100},	/*2:1 */
	{0x00020220, 0x00000006},

	{0x00001494, 0x00010000},	/*DDR SDRAM ODT Control (Low) Register */
	{0x00001498, 0x00000000},	/*DDR SDRAM ODT Control (High) Register */
	{0x0000149C, 0x00000001},	/*DDR Dunit ODT Control Register */

	{0x000014C0, 0x192424C9},	/* DRAM address and Control Driving Strenght  */
	{0x000014C4, 0x0AAA24C9},	/* DRAM Data and DQS Driving Strenght  */

	{0x000200e8, 0x3FFF0E01},	/* DO NOT Modify - Open Mbus Window - 2G - Mbus is required for the training sequence*/
	{0x00020184, 0x3FFFFFE0},	/* DO NOT Modify - Close fast path Window to - 2G */

	{0x0001504, 0x3FFFFFE1},	/* CS0 Size */
	{0x000150C, 0x00000000},	/* CS1 Size */
	{0x0001514, 0x00000000},	/* CS2 Size */
	{0x000151C, 0x00000000},	/* CS3 Size */

	{0x00001538, 0x00000009},	/*Read Data Sample Delays Register */
	{0x0000153C, 0x00000009},	/*Read Data Ready Delay Register */

	{0x000015D0, 0x00000650},	/*MR0 */
	{0x000015D4, 0x00000044},	/*MR1 */
	{0x000015D8, 0x00000010},	/*MR2 */
	{0x000015DC, 0x00000000},	/*MR3 */

	{0x000015E4, 0x00203c18},	/*ZQC Configuration Register */
	{0x000015EC, 0xF800A225},	/*DDR PHY */

	{0x0, 0x0}
};

MV_DRAM_MODES ds414_ddr_modes[MV_DDR3_MODES_NUMBER] = {
	{"ds414_1333-667",   0x3, 0x5, 0x0, A0, syno_ddr3_b0_667_1g_v1,  NULL},
};

extern MV_SERDES_CHANGE_M_PHY serdes_change_m_phy[];

MV_BIN_SERDES_CFG ds414_serdes_cfg[] = {
	{ MV_PEX_ROOT_COMPLEX, 0x02011111, 0x00000000,
	  { PEX_BUS_MODE_X4, PEX_BUS_MODE_X1, PEX_BUS_DISABLED,
	    PEX_BUS_DISABLED },
	  0x0040, serdes_change_m_phy
	}
};

MV_DRAM_MODES *ddr3_get_static_ddr_mode(void)
{
	return &ds414_ddr_modes[0];
}

MV_BIN_SERDES_CFG *board_serdes_cfg_get(void)
{
	return &ds414_serdes_cfg[0];
}

u8 board_sat_r_get(u8 dev_num, u8 reg)
{
	return 0xf;	/* All PEX ports support PCIe Gen2 */
}

int board_early_init_f(void)
{
	int i;

	/* Set GPP Out value */
	reg_write(GPP_DATA_OUT_REG(0), DS414_GPP_OUT_VAL_LOW);
	reg_write(GPP_DATA_OUT_REG(1), DS414_GPP_OUT_VAL_MID);
	reg_write(GPP_DATA_OUT_REG(2), DS414_GPP_OUT_VAL_HIGH);

	/* set GPP polarity */
	reg_write(GPP_DATA_IN_POL_REG(0), DS414_GPP_OUT_POL_LOW);
	reg_write(GPP_DATA_IN_POL_REG(1), DS414_GPP_OUT_POL_MID);
	reg_write(GPP_DATA_IN_POL_REG(2), DS414_GPP_OUT_POL_HIGH);

	/* Set GPP Out Enable */
	reg_write(GPP_DATA_OUT_EN_REG(0), DS414_GPP_OUT_ENA_LOW);
	reg_write(GPP_DATA_OUT_EN_REG(1), DS414_GPP_OUT_ENA_MID);
	reg_write(GPP_DATA_OUT_EN_REG(2), DS414_GPP_OUT_ENA_HIGH);

	for (i = 0; i < ARRAY_SIZE(ds414_mpp_control); i++)
		reg_write(MPP_CONTROL_REG(i), ds414_mpp_control[i]);

	return 0;
}

int board_init(void)
{
	u32 pwr_mng_ctrl_reg;

	/* Adress of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	/* Gate unused clocks
	 *
	 * Note: Disabling unused PCIe lanes will hang PCI bus scan.
	 *       Once this is resolved, bits 10-12, 26 and 27 can be
	 *       unset here as well.
	 */
	pwr_mng_ctrl_reg = reg_read(POWER_MNG_CTRL_REG);
	pwr_mng_ctrl_reg &= ~(BIT(0));				/* Audio */
	pwr_mng_ctrl_reg &= ~(BIT(1) | BIT(2));			/* GE3, GE2 */
	pwr_mng_ctrl_reg &= ~(BIT(14) | BIT(15));		/* SATA0 link and core */
	pwr_mng_ctrl_reg &= ~(BIT(16));				/* LCD */
	pwr_mng_ctrl_reg &= ~(BIT(17));				/* SDIO */
	pwr_mng_ctrl_reg &= ~(BIT(19) | BIT(20));		/* USB1 and USB2 */
	pwr_mng_ctrl_reg &= ~(BIT(29) | BIT(30));		/* SATA1 link and core */
	reg_write(POWER_MNG_CTRL_REG, pwr_mng_ctrl_reg);

	return 0;
}

int misc_init_r(void)
{
	if (!env_get("ethaddr")) {
		puts("Incomplete environment, populating from SPI flash\n");
		do_syno_populate(0, NULL);
	}
	return 0;
}

int checkboard(void)
{
	puts("Board: DS414\n");

	return 0;
}
