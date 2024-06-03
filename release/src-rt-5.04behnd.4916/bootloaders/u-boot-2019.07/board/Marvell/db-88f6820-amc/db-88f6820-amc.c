// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <i2c.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

#include "../drivers/ddr/marvell/a38x/ddr3_init.h"
#include <../serdes/a38x/high_speed_env_spec.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Those values and defines are taken from the Marvell U-Boot version
 * "u-boot-2013.01-2016_T1.0.eng_drop_v10"
 */
#define DB_AMC_88F68XX_GPP_OUT_ENA_LOW					\
	(~(BIT(29)))
#define DB_AMC_88F68XX_GPP_OUT_ENA_MID					\
	(~(BIT(12) | BIT(17) | BIT(18) | BIT(20) | BIT(21)))
#define DB_AMC_88F68XX_GPP_OUT_VAL_LOW	(BIT(29))
#define DB_AMC_88F68XX_GPP_OUT_VAL_MID	0x0
#define DB_AMC_88F68XX_GPP_OUT_VAL_HIGH	0x0
#define DB_AMC_88F68XX_GPP_POL_LOW	0x0
#define DB_AMC_88F68XX_GPP_POL_MID	0x0

static struct serdes_map board_serdes_map[] = {
	{PEX0, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{DEFAULT_SERDES, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{DEFAULT_SERDES, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{DEFAULT_SERDES, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{SGMII1, SERDES_SPEED_1_25_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{SGMII2, SERDES_SPEED_1_25_GBPS, SERDES_DEFAULT_MODE, 0, 0}
};

int hws_board_topology_load(struct serdes_map **serdes_map_array, u8 *count)
{
	*serdes_map_array = board_serdes_map;
	*count = ARRAY_SIZE(board_serdes_map);
	return 0;
}

/*
 * Define the DDR layout / topology here in the board file. This will
 * be used by the DDR3 init code in the SPL U-Boot version to configure
 * the DDR3 controller.
 */
static struct mv_ddr_topology_map board_topology_map = {
	DEBUG_LEVEL_ERROR,
	0x1, /* active interfaces */
	/* cs_mask, mirror, dqs_swap, ck_swap X PUPs */
	{ { { {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0} },
	    SPEED_BIN_DDR_1866L,	/* speed_bin */
	    MV_DDR_DEV_WIDTH_8BIT,	/* memory_width */
	    MV_DDR_DIE_CAP_2GBIT,	/* mem_size */
	    MV_DDR_FREQ_800,		/* frequency */
	    0, 0,			/* cas_wl cas_l */
	    MV_DDR_TEMP_LOW,		/* temperature */
	    MV_DDR_TIM_DEFAULT} },	/* timing */
	BUS_MASK_32BIT,			/* Busses mask */
	MV_DDR_CFG_DEFAULT,		/* ddr configuration data source */
	{ {0} },			/* raw spd data */
	{0}				/* timing parameters */
};

struct mv_ddr_topology_map *mv_ddr_topology_map_get(void)
{
	/* Return the board topology as defined in the board code */
	return &board_topology_map;
}

int board_early_init_f(void)
{
	/* Configure MPP */
	writel(0x11111111, MVEBU_MPP_BASE + 0x00);
	writel(0x11111111, MVEBU_MPP_BASE + 0x04);
	writel(0x55066011, MVEBU_MPP_BASE + 0x08);
	writel(0x05055550, MVEBU_MPP_BASE + 0x0c);
	writel(0x05055555, MVEBU_MPP_BASE + 0x10);
	writel(0x01106565, MVEBU_MPP_BASE + 0x14);
	writel(0x40000000, MVEBU_MPP_BASE + 0x18);
	writel(0x00004444, MVEBU_MPP_BASE + 0x1c);

	/* Set GPP Out value */
	writel(DB_AMC_88F68XX_GPP_OUT_VAL_LOW, MVEBU_GPIO0_BASE + 0x00);
	writel(DB_AMC_88F68XX_GPP_OUT_VAL_MID, MVEBU_GPIO1_BASE + 0x00);

	/* Set GPP Polarity */
	writel(DB_AMC_88F68XX_GPP_POL_LOW, MVEBU_GPIO0_BASE + 0x0c);
	writel(DB_AMC_88F68XX_GPP_POL_MID, MVEBU_GPIO1_BASE + 0x0c);

	/* Set GPP Out Enable */
	writel(DB_AMC_88F68XX_GPP_OUT_ENA_LOW, MVEBU_GPIO0_BASE + 0x04);
	writel(DB_AMC_88F68XX_GPP_OUT_ENA_MID, MVEBU_GPIO1_BASE + 0x04);

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
	puts("Board: Marvell DB-88F6820-AMC\n");

	return 0;
}

int board_eth_init(bd_t *bis)
{
	cpu_eth_init(bis); /* Built in controller(s) come first */
	return pci_eth_init(bis);
}
