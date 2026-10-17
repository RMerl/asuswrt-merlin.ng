// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 */

#include <common.h>
#include <dm.h>
#include <fdt_support.h>
#include <init.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/sys_proto.h>
#include <env.h>
#include <errno.h>
#include <asm/gpio.h>
#include <malloc.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/boot_mode.h>
#include <miiphy.h>
#include <netdev.h>
#include <i2c.h>
#include <linux/delay.h>

#include <dm/platform_data/serial_mxc.h>
#include <dm/platdata.h>

#include "common.h"

DECLARE_GLOBAL_DATA_PTR;

static bool hw_ids_valid;
static bool sw_ids_valid;
static u32 cpu_id;
static u32 unit_id;

const char *gpio_table_sw_names[] = {
	"GPIO2_4", "GPIO2_5", "GPIO2_6", "GPIO2_7"
};

const char *gpio_table_sw_ids_names[] = {
	"sw0", "sw1", "sw2", "sw3"
};

const char *gpio_table_hw_names[] = {
	"GPIO6_7", "GPIO6_9", "GPIO6_10", "GPIO6_11",
	"GPIO4_7", "GPIO4_11", "GPIO4_13", "GPIO4_15"
};

const char *gpio_table_hw_ids_names[] = {
	"hw0", "hw1", "hw2", "hw3", "hw4", "hw5", "hw6", "hw7"
};

static int get_board_id(const char **pin_names, const char **ids_names,
			int size, bool *valid, u32 *id)
{
	struct gpio_desc desc;
	int i, ret, val;

	*valid = false;

	for (i = 0; i < size; i++) {
		memset(&desc, 0, sizeof(desc));

		ret = dm_gpio_lookup_name(pin_names[i], &desc);
		if (ret) {
			printf("Can't lookup request SWx gpios\n");
			return ret;
		}

		ret = dm_gpio_request(&desc, ids_names[i]);
		if (ret) {
			printf("Can't lookup request SWx gpios\n");
			return ret;
		}

		dm_gpio_set_dir_flags(&desc, GPIOD_IS_IN);

		val = dm_gpio_get_value(&desc);
		if (val < 0) {
			printf("Can't get SW%d ID\n", i);
			*id = 0;
			return val;
		}
		*id |= val << i;
	}
	*valid = true;

	return 0;
}

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

iomux_v3_cfg_t const misc_pads[] = {
	/* Prod ID GPIO pins */
	MX6_PAD_NANDF_D4__GPIO2_IO04    | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D5__GPIO2_IO05    | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D6__GPIO2_IO06    | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D7__GPIO2_IO07    | MUX_PAD_CTRL(NO_PAD_CTRL),

	/* HW revision GPIO pins */
	MX6_PAD_NANDF_CLE__GPIO6_IO07   | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_WP_B__GPIO6_IO09  | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_RB0__GPIO6_IO10   | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_CS0__GPIO6_IO11   | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_KEY_ROW0__GPIO4_IO07 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_KEY_ROW2__GPIO4_IO11 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_KEY_ROW3__GPIO4_IO13 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_KEY_ROW4__GPIO4_IO15 | MUX_PAD_CTRL(NO_PAD_CTRL),

	/* XTALOSC */
	MX6_PAD_GPIO_3__XTALOSC_REF_CLK_24M | MUX_PAD_CTRL(NO_PAD_CTRL),

	/* Emergency recovery pin */
	MX6_PAD_EIM_D29__GPIO3_IO29 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

/*
 * Do not overwrite the console
 * Always use serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	fdt_fixup_ethernet(blob);
	return 0;
}
#endif

int board_phy_config(struct phy_device *phydev)
{
	/* display5 due to PCB routing can only work with 100 Mbps */
	phydev->advertising &= ~(ADVERTISED_1000baseX_Half |
				 ADVERTISED_1000baseX_Full |
				 SUPPORTED_1000baseT_Half |
				 SUPPORTED_1000baseT_Full);

	if (phydev->drv->config)
		return phydev->drv->config(phydev);

	return 0;
}

int board_init(void)
{
	struct gpio_desc phy_int_gbe, spi2_wp;
	int ret;

	debug("board init\n");
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	/* Setup misc (application specific) stuff */
	SETUP_IOMUX_PADS(misc_pads);

	get_board_id(gpio_table_sw_names, &gpio_table_sw_ids_names[0],
		     ARRAY_SIZE(gpio_table_sw_names), &sw_ids_valid, &unit_id);
	debug("SWx unit_id 0x%x\n", unit_id);

	get_board_id(gpio_table_hw_names, &gpio_table_hw_ids_names[0],
		     ARRAY_SIZE(gpio_table_hw_names), &hw_ids_valid, &cpu_id);
	debug("HWx cpu_id 0x%x\n", cpu_id);

	if (hw_ids_valid && sw_ids_valid)
		printf("ID:    unit type 0x%x rev 0x%x\n", unit_id, cpu_id);

	udelay(25);

	/* Setup low level FEC (ETH) */
	ret = dm_gpio_lookup_name("GPIO1_28", &phy_int_gbe);
	if (ret) {
		printf("Cannot get GPIO1_28\n");
	} else {
		ret = dm_gpio_request(&phy_int_gbe, "INT_GBE");
		if (!ret)
			dm_gpio_set_dir_flags(&phy_int_gbe, GPIOD_IS_IN);
	}

	iomuxc_set_rgmii_io_voltage(DDR_SEL_1P5V_IO);
	enable_fec_anatop_clock(0, ENET_125MHZ);

	/* Setup #WP for SPI-NOR memory */
	ret = dm_gpio_lookup_name("GPIO7_0", &spi2_wp);
	if (ret) {
		printf("Cannot get GPIO7_0\n");
	} else {
		ret = dm_gpio_request(&spi2_wp, "spi2_#wp");
		if (!ret)
			dm_gpio_set_dir_flags(&spi2_wp, GPIOD_IS_OUT |
					      GPIOD_IS_OUT_ACTIVE);
	}

	return 0;
}

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	/* eMMC, USDHC-4, 8-bit bus width */
	/* SPI-NOR, ECSPI-2 SS0, 3-bytes addressing */
	{"emmc",    MAKE_CFGVAL(0x60, 0x58, 0x00, 0x00)},
	{"spinor",  MAKE_CFGVAL(0x30, 0x00, 0x00, 0x09)},
	{NULL,	0},
};

static void setup_boot_modes(void)
{
	add_board_boot_modes(board_boot_modes);
}
#else
static inline void setup_boot_modes(void) {}
#endif

int misc_init_r(void)
{
	struct gpio_desc em_pad;
	int ret;

	setup_boot_modes();

	ret = dm_gpio_lookup_name("GPIO3_29", &em_pad);
	if (ret) {
		printf("Can't find emergency PAD gpio\n");
		return ret;
	}

	ret = dm_gpio_request(&em_pad, "Emergency_PAD");
	if (ret) {
		printf("Can't request emergency PAD gpio\n");
		return ret;
	}

	dm_gpio_set_dir_flags(&em_pad, GPIOD_IS_IN);

	return 0;
}
