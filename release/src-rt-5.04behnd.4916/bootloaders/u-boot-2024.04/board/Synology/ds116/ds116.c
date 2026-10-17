// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Tony Dinh <mibodhi@gmail.com>
 *
 */

#include <i2c.h>
#include <init.h>
#include <miiphy.h>
#include <net.h>
#include <netdev.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <linux/bitops.h>

#include "../drivers/ddr/marvell/a38x/ddr3_init.h"
#include <../serdes/a38x/high_speed_env_spec.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Those DS116_GPP_xx values and defines in board_serdes_map, and board_topology_map
 * are taken from Marvell U-Boot version
 * U-Boot 2013.01-g6cc0a6d (Marvell version: 2015_T1.0p16)
 */
#define DS116_GPP_OUT_ENA_LOW						\
	(~(BIT(1)  | BIT(4)  | BIT(6)  | BIT(7)  | BIT(8)  | BIT(9)  |	\
	   BIT(10) | BIT(11) | BIT(15) | BIT(19) | BIT(22) | BIT(23) |	\
	   BIT(25) | BIT(26) | BIT(27) | BIT(29) | BIT(30) | BIT(31)))
#define DS116_GPP_OUT_ENA_MID						\
	(~(BIT(0) | BIT(1) | BIT(2) | BIT(3) | BIT(4) | BIT(15) |	\
	   BIT(16) | BIT(17) | BIT(18) | BIT(26) | BIT(27)))

#define DS116_GPP_OUT_VAL_LOW	BIT(15)
#define DS116_GPP_OUT_VAL_MID	(BIT(26) | BIT(27))
#define DS116_GPP_POL_LOW	0x0
#define DS116_GPP_POL_MID	0x0

static struct serdes_map board_serdes_map[] = {
	{SGMII0, SERDES_SPEED_1_25_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{SATA0, SERDES_SPEED_6_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{DEFAULT_SERDES, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{DEFAULT_SERDES, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{USB3_HOST0, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{USB3_HOST1, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
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
	    SPEED_BIN_DDR_1866L,        /* speed_bin */
	    MV_DDR_DEV_WIDTH_16BIT,     /* memory_width - 16 bits */
	    MV_DDR_DIE_CAP_4GBIT,       /* mem_size - DS116 board has 2x512MB DRAM banks */
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
	/*
	 * Those MPP values are taken from the Marvell U-Boot version
	 * U-Boot 2013.01-g6cc0a6d (Marvell version: 2015_T1.0p16)
	 */

	/* Configure MPP */
	writel(0x00111111, MVEBU_MPP_BASE + 0x00);
	writel(0x00000000, MVEBU_MPP_BASE + 0x04);
	writel(0x11040330, MVEBU_MPP_BASE + 0x08);
	writel(0x00000011, MVEBU_MPP_BASE + 0x0c);
	writel(0x00000000, MVEBU_MPP_BASE + 0x10);
	writel(0x00000000, MVEBU_MPP_BASE + 0x14);
	writel(0x00000000, MVEBU_MPP_BASE + 0x18);
	writel(0x00000000, MVEBU_MPP_BASE + 0x1c);

	/* Set GPP Out value */
	writel(DS116_GPP_OUT_VAL_LOW, MVEBU_GPIO0_BASE + 0x00);
	writel(DS116_GPP_OUT_VAL_MID, MVEBU_GPIO1_BASE + 0x00);

	/* Set GPP Polarity */
	writel(DS116_GPP_POL_LOW, MVEBU_GPIO0_BASE + 0x0c);
	writel(DS116_GPP_POL_MID, MVEBU_GPIO1_BASE + 0x0c);

	/* Set GPP Out Enable */
	writel(DS116_GPP_OUT_ENA_LOW, MVEBU_GPIO0_BASE + 0x04);
	writel(DS116_GPP_OUT_ENA_MID, MVEBU_GPIO1_BASE + 0x04);

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	return 0;
}

int board_eth_init(struct bd_info *bis)
{
	cpu_eth_init(bis); /* Built in controller(s) come first */
	return pci_eth_init(bis);
}
