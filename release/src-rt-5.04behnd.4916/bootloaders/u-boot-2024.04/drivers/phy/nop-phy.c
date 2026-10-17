// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Texas Instruments Incorporated - https://www.ti.com/
 * Written by Jean-Jacques Hiblot  <jjhiblot@ti.com>
 */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <generic-phy.h>
#include <asm-generic/gpio.h>

struct nop_phy_priv {
	struct clk_bulk bulk;
#if CONFIG_IS_ENABLED(DM_GPIO)
	struct gpio_desc reset_gpio;
#endif
};

#if CONFIG_IS_ENABLED(DM_GPIO)
static int nop_phy_reset(struct phy *phy)
{
	struct nop_phy_priv *priv = dev_get_priv(phy->dev);

	/* Return if there is no gpio since it's optional */
	if (!dm_gpio_is_valid(&priv->reset_gpio))
		return 0;

	return dm_gpio_set_value(&priv->reset_gpio, true);
}
#endif

static int nop_phy_init(struct phy *phy)
{
	struct nop_phy_priv *priv = dev_get_priv(phy->dev);
	int ret = 0;

	if (CONFIG_IS_ENABLED(CLK)) {
		ret = clk_enable_bulk(&priv->bulk);
		if (ret)
			return ret;
	}

#if CONFIG_IS_ENABLED(DM_GPIO)
	/* Take phy out of reset */
	if (dm_gpio_is_valid(&priv->reset_gpio)) {
		ret = dm_gpio_set_value(&priv->reset_gpio, false);
		if (ret) {
			if (CONFIG_IS_ENABLED(CLK))
				clk_disable_bulk(&priv->bulk);
			return ret;
		}
	}
#endif
	return 0;
}

static int nop_phy_probe(struct udevice *dev)
{
	struct nop_phy_priv *priv = dev_get_priv(dev);
	int ret = 0;

	if (CONFIG_IS_ENABLED(CLK)) {
		ret = clk_get_bulk(dev, &priv->bulk);
		if (ret < 0) {
			dev_err(dev, "Failed to get clk: %d\n", ret);
			return ret;
		}
	}
#if CONFIG_IS_ENABLED(DM_GPIO)
	ret = gpio_request_by_name(dev, "reset-gpios", 0,
				   &priv->reset_gpio,
				   GPIOD_IS_OUT);
#endif
	if (ret != -ENOENT)
		return ret;

	return 0;
}

static const struct udevice_id nop_phy_ids[] = {
	{ .compatible = "nop-phy" },
	{ .compatible = "usb-nop-xceiv" },
	{ }
};

static struct phy_ops nop_phy_ops = {
	.init = nop_phy_init,
#if CONFIG_IS_ENABLED(DM_GPIO)
	.reset = nop_phy_reset,
#endif
};

U_BOOT_DRIVER(nop_phy) = {
	.name	= "nop_phy",
	.id	= UCLASS_PHY,
	.of_match = nop_phy_ids,
	.ops = &nop_phy_ops,
	.probe = nop_phy_probe,
	.priv_auto	= sizeof(struct nop_phy_priv),
};
