// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Allied Telesis Labs
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <i2c.h>
#include <wdt.h>
#include <asm/gpio.h>
#include <linux/mbus.h>
#include <linux/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include "../common/gpio_hog.h"

#include "../drivers/ddr/marvell/a38x/ddr3_init.h"
#include <../serdes/a38x/high_speed_env_spec.h>

DECLARE_GLOBAL_DATA_PTR;

#define MVEBU_DEV_BUS_BASE		(MVEBU_REGISTER(0x10400))

#define CONFIG_NVS_LOCATION		0xf4800000
#define CONFIG_NVS_SIZE			(512 << 10)

static struct serdes_map board_serdes_map[] = {
	{PEX0, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{DEFAULT_SERDES, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{PEX1, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{DEFAULT_SERDES, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{DEFAULT_SERDES, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{DEFAULT_SERDES, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0}
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
	    SPEED_BIN_DDR_1866M,	/* speed_bin */
	    MV_DDR_DEV_WIDTH_16BIT,	/* sdram device width */
	    MV_DDR_DIE_CAP_4GBIT,	/* die capacity */
	    MV_DDR_FREQ_SAR,		/* frequency */
	    0, 0,			/* cas_l cas_wl */
	    MV_DDR_TEMP_LOW,		/* temperature */
	    MV_DDR_TIM_2T} },		/* timing */
	BUS_MASK_32BIT_ECC,		/* subphys mask */
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
	writel(0x00001111, MVEBU_MPP_BASE + 0x00);
	writel(0x00000000, MVEBU_MPP_BASE + 0x04);
	writel(0x55000000, MVEBU_MPP_BASE + 0x08);
	writel(0x55550550, MVEBU_MPP_BASE + 0x0c);
	writel(0x55555555, MVEBU_MPP_BASE + 0x10);
	writel(0x00100565, MVEBU_MPP_BASE + 0x14);
	writel(0x40000000, MVEBU_MPP_BASE + 0x18);
	writel(0x00004444, MVEBU_MPP_BASE + 0x1c);

	return 0;
}

void spl_board_init(void)
{
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	/* window for NVS */
	mbus_dt_setup_win(&mbus_state, CONFIG_NVS_LOCATION, CONFIG_NVS_SIZE,
			  CPU_TARGET_DEVICEBUS_BOOTROM_SPI, CPU_ATTR_DEV_CS1);

	/* DEV_READYn is not needed for NVS, ignore it when accessing CS1 */
	writel(0x00004001, MVEBU_DEV_BUS_BASE + 0xc8);

	spl_board_init();

	return 0;
}

void arch_preboot_os(void)
{
#ifdef CONFIG_WATCHDOG
	wdt_stop(gd->watchdog_dev);
#endif
}

static int led_7seg_init(unsigned int segments)
{
	int node;
	int ret;
	int i;
	struct gpio_desc desc[8];

	node = fdt_node_offset_by_compatible(gd->fdt_blob, 0,
					     "atl,of-led-7seg");
	if (node < 0)
		return -ENODEV;

	ret = gpio_request_list_by_name_nodev(offset_to_ofnode(node),
					      "segment-gpios", desc,
					      ARRAY_SIZE(desc), GPIOD_IS_OUT);
	if (ret < 0)
		return ret;

	for (i = 0; i < ARRAY_SIZE(desc); i++) {
		ret = dm_gpio_set_value(&desc[i], !(segments & BIT(i)));
		if (ret)
			return ret;
	}

	return 0;
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	static struct gpio_desc usb_en = {}, nand_wp = {}, phy_reset[2] = {},
				led_en = {};

	gpio_hog(&usb_en, "atl,usb-enable", "enable-gpio", 1);
	gpio_hog(&nand_wp, "atl,nand-protect", "protect-gpio", 1);
	gpio_hog_list(phy_reset, ARRAY_SIZE(phy_reset), "atl,phy-reset", "reset-gpio", 0);
	gpio_hog(&led_en, "atl,led-enable", "enable-gpio", 1);

#ifdef MTDPARTS_MTDOOPS
	env_set("mtdoops", MTDPARTS_MTDOOPS);
#endif

	led_7seg_init(0xff);

	return 0;
}
#endif

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	puts("Board: " CONFIG_SYS_BOARD "\n");

	return 0;
}
#endif
