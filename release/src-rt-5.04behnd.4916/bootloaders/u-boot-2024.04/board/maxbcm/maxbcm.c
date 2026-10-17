// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <init.h>
#include <miiphy.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <linux/mbus.h>

#include "../drivers/ddr/marvell/axp/ddr3_hw_training.h"
#include "../arch/arm/mach-mvebu/serdes/axp/high_speed_env_spec.h"

DECLARE_GLOBAL_DATA_PTR;

/* Base addresses for the external device chip selects */
#define DEV_CS0_BASE		0xe0000000
#define DEV_CS1_BASE		0xe1000000
#define DEV_CS2_BASE		0xe2000000
#define DEV_CS3_BASE		0xe3000000

/* DDR3 static configuration */
MV_DRAM_MC_INIT ddr3_b0_maxbcm[MV_MAX_DDR3_STATIC_SIZE] = {
	{0x00001400, 0x7301CC30},	/* DDR SDRAM Configuration Register */
	{0x00001404, 0x30000820},	/* Dunit Control Low Register */
	{0x00001408, 0x5515BAAB},	/* DDR SDRAM Timing (Low) Register */
	{0x0000140C, 0x38DA3F97},	/* DDR SDRAM Timing (High) Register */
	{0x00001410, 0x20100005},	/* DDR SDRAM Address Control Register */
	{0x00001414, 0x0000F3FF},	/* DDR SDRAM Open Pages Control Reg */
	{0x00001418, 0x00000e00},	/* DDR SDRAM Operation Register */
	{0x0000141C, 0x00000672},	/* DDR SDRAM Mode Register */
	{0x00001420, 0x00000004},	/* DDR SDRAM Extended Mode Register */
	{0x00001424, 0x0000F3FF},	/* Dunit Control High Register */
	{0x00001428, 0x0011A940},	/* Dunit Control High Register */
	{0x0000142C, 0x014C5134},	/* Dunit Control High Register */
	{0x0000147C, 0x0000D771},

	{0x00001494, 0x00010000},	/* DDR SDRAM ODT Control (Low) Reg */
	{0x0000149C, 0x00000001},	/* DDR Dunit ODT Control Register */
	{0x000014A0, 0x00000001},
	{0x000014A8, 0x00000101},

	/* Recommended Settings from Marvell for 4 x 16 bit devices: */
	{0x000014C0, 0x192424C9},	/* DRAM addr and Ctrl Driving Strenght*/
	{0x000014C4, 0xAAA24C9},	/* DRAM Data and DQS Driving Strenght */

	/*
	 * DO NOT Modify - Open Mbus Window - 2G - Mbus is required for the
	 * training sequence
	 */
	{0x000200e8, 0x3FFF0E01},
	{0x00020184, 0x3FFFFFE0},	/* Close fast path Window to - 2G */

	{0x0001504, 0x3FFFFFE1},	/* CS0 Size */
	{0x000150C, 0x00000000},	/* CS1 Size */
	{0x0001514, 0x00000000},	/* CS2 Size */
	{0x000151C, 0x00000000},	/* CS3 Size */

	{0x0020220, 0x00000007},	/* Reserved */

	{0x00001538, 0x0000000B},	/* Read Data Sample Delays Register */
	{0x0000153C, 0x0000000B},	/* Read Data Ready Delay Register */

	{0x000015D0, 0x00000670},	/* MR0 */
	{0x000015D4, 0x00000044},	/* MR1 */
	{0x000015D8, 0x00000018},	/* MR2 */
	{0x000015DC, 0x00000000},	/* MR3 */
	{0x000015E0, 0x00000001},
	{0x000015E4, 0x00203c18},	/* ZQDS Configuration Register */
	{0x000015EC, 0xF800A225},	/* DDR PHY */

	{0x0, 0x0}
};

MV_DRAM_MODES maxbcm_ddr_modes[MV_DDR3_MODES_NUMBER] = {
	{"maxbcm_1600-800", 0xB, 0x5, 0x0, A0, ddr3_b0_maxbcm,  NULL},
};

extern MV_SERDES_CHANGE_M_PHY serdes_change_m_phy[];

/* MAXBCM: SERDES 0-4 PCIE, Serdes 7 = SGMII 0, all others =  unconnected */
MV_BIN_SERDES_CFG maxbcm_serdes_cfg[] = {
	{ MV_PEX_ROOT_COMPLEX, 0x20011111, 0x00000000,
	  { PEX_BUS_MODE_X1, PEX_BUS_MODE_X1, PEX_BUS_DISABLED,
	    PEX_BUS_DISABLED },
	  0x1f, serdes_change_m_phy
	}
};

MV_DRAM_MODES *ddr3_get_static_ddr_mode(void)
{
	/* Only one mode supported for this board */
	return &maxbcm_ddr_modes[0];
}

MV_BIN_SERDES_CFG *board_serdes_cfg_get(void)
{
	return &maxbcm_serdes_cfg[0];
}

int board_early_init_f(void)
{
	/*
	 * Don't configure MPP (pin multiplexing) and GPIO here,
	 * its already done in bin_hdr
	 */

	/*
	 * Setup some board specific mbus address windows
	 */
	mbus_dt_setup_win(DEV_CS0_BASE, 16 << 20,
			  CPU_TARGET_DEVICEBUS_BOOTROM_SPI, CPU_ATTR_DEV_CS0);
	mbus_dt_setup_win(DEV_CS1_BASE, 16 << 20,
			  CPU_TARGET_DEVICEBUS_BOOTROM_SPI, CPU_ATTR_DEV_CS1);
	mbus_dt_setup_win(DEV_CS2_BASE, 16 << 20,
			  CPU_TARGET_DEVICEBUS_BOOTROM_SPI, CPU_ATTR_DEV_CS2);
	mbus_dt_setup_win(DEV_CS3_BASE, 16 << 20,
			  CPU_TARGET_DEVICEBUS_BOOTROM_SPI, CPU_ATTR_DEV_CS3);

	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	return 0;
}

int checkboard(void)
{
	puts("Board: maxBCM\n");

	return 0;
}

/* Configure and enable MV88E6185 switch */
int board_phy_config(struct phy_device *phydev)
{
	/*
	 * todo:
	 * Fill this with the real setup / config code.
	 * Please see board/Marvell/db-mv784mp-gp/db-mv784mp-gp.c
	 * for details.
	 */
	printf("88E6185 Initialized\n");
	return 0;
}
