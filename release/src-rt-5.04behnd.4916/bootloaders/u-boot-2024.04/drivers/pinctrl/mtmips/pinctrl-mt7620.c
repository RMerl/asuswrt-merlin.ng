// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <dm.h>
#include <dm/pinctrl.h>
#include <linux/bitops.h>
#include <linux/io.h>

#include "pinctrl-mtmips-common.h"

#define SUTIF_SHIFT			30
#define WDT_RST_SHIFT			21
#define PA_G_SHIFT			20
#define NAND_SD_SHIFT			18
#define PERST_SHIFT			16
#define EPHY_LED_SHIFT			15
#define WLED_SHIFT			13
#define SPI_CS1_SHIFT			12
#define SPI_SHIFT			11
#define RGMII2_SHIFT			10
#define RGMII1_SHIFT			9
#define MDIO_SHIFT			7
#define UARTL_SHIFT			5
#define UARTF_SHIFT			2
#define I2C_SHIFT			0

#define GM4_MASK			3
#define GM8_MASK			7

#if CONFIG_IS_ENABLED(PINMUX)
static const struct mtmips_pmx_func sutif_grp[] = {
	FUNC("i2c", 2),
	FUNC("uartl", 1),
	FUNC("none", 0),
};

static const struct mtmips_pmx_func wdt_rst_grp[] = {
	FUNC("gpio", 2),
	FUNC("refclk", 1),
	FUNC("wdt rst", 0),
};

static const struct mtmips_pmx_func pa_g_grp[] = {
	FUNC("gpio", 1),
	FUNC("pa", 0),
};

static const struct mtmips_pmx_func nand_sd_grp[] = {
	FUNC("gpio", 2),
	FUNC("sd", 1),
	FUNC("nand", 0),
};

static const struct mtmips_pmx_func perst_grp[] = {
	FUNC("gpio", 2),
	FUNC("refclk", 1),
	FUNC("perst", 0),
};

static const struct mtmips_pmx_func ephy_led_grp[] = {
	FUNC("gpio", 1),
	FUNC("led", 0),
};

static const struct mtmips_pmx_func wled_grp[] = {
	FUNC("gpio", 1),
	FUNC("led", 0),
};

static const struct mtmips_pmx_func spi_cs1_grp[] = {
	FUNC("refclk", 1),
	FUNC("spi cs1", 0),
};

static const struct mtmips_pmx_func spi_grp[] = {
	FUNC("gpio", 1),
	FUNC("spi", 0),
};

static const struct mtmips_pmx_func rgmii2_grp[] = {
	FUNC("gpio", 1),
	FUNC("rgmii2", 0),
};

static const struct mtmips_pmx_func rgmii1_grp[] = {
	FUNC("gpio", 1),
	FUNC("rgmii1", 0),
};

static const struct mtmips_pmx_func mdio_grp[] = {
	FUNC("gpio", 2),
	FUNC("refclk", 1),
	FUNC("mdio", 0),
};

static const struct mtmips_pmx_func uartl_grp[] = {
	FUNC("gpio", 1),
	FUNC("uartl", 0),
};

static const struct mtmips_pmx_func uartf_grp[] = {
	FUNC("gpio", 7),
	FUNC("i2s gpio", 6),
	FUNC("uartf gpio", 5),
	FUNC("gpio pcm", 4),
	FUNC("i2s uartf", 3),
	FUNC("i2s pcm", 2),
	FUNC("uartf pcm", 1),
	FUNC("uartf", 0),
};

static const struct mtmips_pmx_func i2c_grp[] = {
	FUNC("gpio", 1),
	FUNC("i2c", 0),
};

static const struct mtmips_pmx_group mt7620_pinmux_data[] = {
	GRP("sutif", sutif_grp, 0, SUTIF_SHIFT, GM4_MASK),
	GRP("wdt rst", wdt_rst_grp, 0, WDT_RST_SHIFT, GM4_MASK),
	GRP("pa", pa_g_grp, 0, PA_G_SHIFT, 1),
	GRP("nand", nand_sd_grp, 0, NAND_SD_SHIFT, GM4_MASK),
	GRP("perst", perst_grp, 0, PERST_SHIFT, GM4_MASK),
	GRP("ephy led", ephy_led_grp, 0, EPHY_LED_SHIFT, 1),
	GRP("wled", wled_grp, 0, WLED_SHIFT, 1),
	GRP("spi cs1", spi_cs1_grp, 0, SPI_CS1_SHIFT, 1),
	GRP("spi", spi_grp, 0, SPI_SHIFT, 1),
	GRP("rgmii2", rgmii2_grp, 0, RGMII2_SHIFT, 1),
	GRP("rgmii1", rgmii1_grp, 0, RGMII1_SHIFT, 1),
	GRP("mdio", mdio_grp, 0, MDIO_SHIFT, GM4_MASK),
	GRP("uartl", uartl_grp, 0, UARTL_SHIFT, 1),
	GRP("uartf", uartf_grp, 0, UARTF_SHIFT, GM8_MASK),
	GRP("i2c", i2c_grp, 0, I2C_SHIFT, 1),
};

static int mt7620_get_groups_count(struct udevice *dev)
{
	return ARRAY_SIZE(mt7620_pinmux_data);
}

static const char *mt7620_get_group_name(struct udevice *dev,
					 unsigned int selector)
{
	return mt7620_pinmux_data[selector].name;
}
#endif /* CONFIG_IS_ENABLED(PINMUX) */

static int mt7620_pinctrl_probe(struct udevice *dev)
{
	struct mtmips_pinctrl_priv *priv = dev_get_priv(dev);
	int ret = 0;

#if CONFIG_IS_ENABLED(PINMUX)
	ret = mtmips_pinctrl_probe(priv, ARRAY_SIZE(mt7620_pinmux_data),
				   mt7620_pinmux_data);
#endif /* CONFIG_IS_ENABLED(PINMUX) */

	return ret;
}

static int mt7620_pinctrl_of_to_plat(struct udevice *dev)
{
	struct mtmips_pinctrl_priv *priv = dev_get_priv(dev);

	priv->base = dev_remap_addr_index(dev, 0);
	if (!priv->base)
		return -EINVAL;

	return 0;
}

static const struct pinctrl_ops mt7620_pinctrl_ops = {
#if CONFIG_IS_ENABLED(PINMUX)
	.get_groups_count = mt7620_get_groups_count,
	.get_group_name = mt7620_get_group_name,
	.get_functions_count = mtmips_get_functions_count,
	.get_function_name = mtmips_get_function_name,
	.pinmux_group_set = mtmips_pinmux_group_set,
#endif /* CONFIG_IS_ENABLED(PINMUX) */
	.set_state = pinctrl_generic_set_state,
};

static const struct udevice_id mt7620_pinctrl_ids[] = {
	{ .compatible = "mediatek,mt7620-pinctrl" },
	{ }
};

U_BOOT_DRIVER(mt7620_pinctrl) = {
	.name = "mt7620-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = mt7620_pinctrl_ids,
	.of_to_plat = mt7620_pinctrl_of_to_plat,
	.ops = &mt7620_pinctrl_ops,
	.probe = mt7620_pinctrl_probe,
	.priv_auto = sizeof(struct mtmips_pinctrl_priv),
	.flags = DM_FLAG_PRE_RELOC,
};
