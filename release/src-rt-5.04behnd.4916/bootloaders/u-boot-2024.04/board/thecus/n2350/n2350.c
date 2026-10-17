// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Tony Dinh <mibodhi@gmail.com>
 *
 */

#include <i2c.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <linux/bitops.h>

#include "../drivers/ddr/marvell/a38x/ddr3_init.h"
#include <../serdes/a38x/high_speed_env_spec.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Those N2350_GPP_xx values and defines in board_serdes_map, and board_topology_map
 * are taken from the Marvell U-Boot version "u-boot-a38x-2015T1_p18_Thecus"
 */

#define N2350_GPP_OUT_ENA_LOW	(~(BIT(20) | BIT(21) | BIT(24)))
#define N2350_GPP_OUT_ENA_MID	(~(BIT(12) | BIT(13) | BIT(16) | BIT(19) | BIT(22)))
#define N2350_GPP_OUT_VAL_LOW	(BIT(21) | BIT(24))
#define N2350_GPP_OUT_VAL_MID	(BIT(0) | BIT(12) | BIT(13) | BIT(16))
#define N2350_GPP_POL_LOW	0x0
#define N2350_GPP_POL_MID	0x0

static struct serdes_map board_serdes_map[] = {
	{ SGMII0, SERDES_SPEED_1_25_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{ SATA0,  SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{ SATA1,  SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{ DEFAULT_SERDES, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{ USB3_HOST0, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{ USB3_HOST1, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
};

int hws_board_topology_load(struct serdes_map **serdes_map_array, u8 *count)
{
	*serdes_map_array = board_serdes_map;
	*count = ARRAY_SIZE(board_serdes_map);
	return 0;
}

/*
 * Define the DDR layout / topology here in the board file. This will
 * be used by the DDR4 init code in the SPL U-Boot version to configure
 * the DDR4 controller.
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
	    SPEED_BIN_DDR_1866L,        /* speed_bin */
	    MV_DDR_DEV_WIDTH_16BIT,     /* memory_width - 16 bits */
	    MV_DDR_DIE_CAP_4GBIT,       /* mem_size - N2350 board has 2x512MB DRAM banks */
	    MV_DDR_FREQ_800,            /* frequency */
	    0, 0,			/* cas_wl cas_l */
	    MV_DDR_TEMP_LOW,		/* temperature */
	    MV_DDR_TIM_DEFAULT} },	/* timing */
	BUS_MASK_32BIT,			/* Busses mask */
	MV_DDR_CFG_DEFAULT,		/* ddr configuration data source */
	NOT_COMBINED,			/* ddr twin-die combined */
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
	/* Those MPP values are taken from the Marvell U-Boot version
	 * "u-boot-a38x-2015T1_p18_Thecus"
	 */

	/* Configure MPP */
	writel(0x50111111, MVEBU_MPP_BASE + 0x00);	/* MPP0_7 */
	writel(0x00555555, MVEBU_MPP_BASE + 0x04);	/* MPP8_15 */
	writel(0x55000000, MVEBU_MPP_BASE + 0x08);	/* MPP16_23 */
	writel(0x05050050, MVEBU_MPP_BASE + 0x0c);	/* MPP24_31 */
	writel(0x05555555, MVEBU_MPP_BASE + 0x10);	/* MPP32_39 */
	writel(0x00000565, MVEBU_MPP_BASE + 0x14);	/* MPP40_47 */
	writel(0x00000000, MVEBU_MPP_BASE + 0x18);	/* MPP48_55 */
	writel(0x00004444, MVEBU_MPP_BASE + 0x1c);	/* MPP56_63 */

	/* Set GPP Out value */
	writel(N2350_GPP_OUT_VAL_LOW, MVEBU_GPIO0_BASE + 0x00);
	writel(N2350_GPP_OUT_VAL_MID, MVEBU_GPIO1_BASE + 0x00);

	/* Set GPP Polarity */
	writel(N2350_GPP_POL_LOW, MVEBU_GPIO0_BASE + 0x0c);
	writel(N2350_GPP_POL_MID, MVEBU_GPIO1_BASE + 0x0c);

	/* Set GPP Out Enable */
	writel(N2350_GPP_OUT_ENA_LOW, MVEBU_GPIO0_BASE + 0x04);
	writel(N2350_GPP_OUT_ENA_MID, MVEBU_GPIO1_BASE + 0x04);

	return 0;
}

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	return 0;
}

int board_eth_init(struct bd_info *bis)
{
	cpu_eth_init(bis); /* Built in controller(s) come first */
	return pci_eth_init(bis);
}
