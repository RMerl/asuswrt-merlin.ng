// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 NXP
 */

#define LOG_CATEGORY UCLASS_ETH_PHY

#include <common.h>
#include <dm.h>
#include <log.h>
#include <net.h>
#include <asm-generic/gpio.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#include <dm/lists.h>
#include <linux/delay.h>

struct eth_phy_device_priv {
	struct mii_dev *mdio_bus;
	struct gpio_desc reset_gpio;
	u32 reset_assert_delay;
	u32 reset_deassert_delay;
};

int eth_phy_binds_nodes(struct udevice *eth_dev)
{
	ofnode mdio_node, phy_node;
	const char *node_name;
	int ret;

	/* search a subnode named "mdio.*" */
	dev_for_each_subnode(mdio_node, eth_dev) {
		node_name = ofnode_get_name(mdio_node);
		if (!strncmp(node_name, "mdio", 4))
			break;
	}
	if (!ofnode_valid(mdio_node)) {
		dev_dbg(eth_dev, "%s: %s mdio subnode not found!\n", __func__,
			eth_dev->name);
		return -ENXIO;
	}
	dev_dbg(eth_dev, "%s: %s subnode found!\n", __func__, node_name);

	ofnode_for_each_subnode(phy_node, mdio_node) {
		node_name = ofnode_get_name(phy_node);

		dev_dbg(eth_dev, "* Found child node: '%s'\n", node_name);

		ret = device_bind_driver_to_node(eth_dev,
						 "eth_phy_generic_drv",
						 node_name, phy_node, NULL);
		if (ret) {
			dev_dbg(eth_dev, "  - Eth phy binding error: %d\n", ret);
			continue;
		}

		dev_dbg(eth_dev, "  - bound phy device: '%s'\n", node_name);
	}

	return 0;
}

int eth_phy_set_mdio_bus(struct udevice *eth_dev, struct mii_dev *mdio_bus)
{
	struct udevice *dev;
	struct eth_phy_device_priv *uc_priv;

	for (uclass_first_device(UCLASS_ETH_PHY, &dev); dev;
	     uclass_next_device(&dev)) {
		if (dev->parent == eth_dev) {
			uc_priv = (struct eth_phy_device_priv *)(dev_get_uclass_priv(dev));

			if (!uc_priv->mdio_bus)
				uc_priv->mdio_bus = mdio_bus;
		}
	}

	return 0;
}

struct mii_dev *eth_phy_get_mdio_bus(struct udevice *eth_dev)
{
	int ret;
	struct udevice *phy_dev;
	struct eth_phy_device_priv *uc_priv;

	/* Will probe the parent of phy device, then phy device */
	ret = uclass_get_device_by_phandle(UCLASS_ETH_PHY, eth_dev,
					   "phy-handle", &phy_dev);
	if (!ret) {
		if (eth_dev != phy_dev->parent) {
			/*
			 * phy_dev is shared and controlled by
			 * other eth controller
			 */
			uc_priv = (struct eth_phy_device_priv *)(dev_get_uclass_priv(phy_dev));
			if (uc_priv->mdio_bus)
				log_notice("Get shared mii bus on %s\n", eth_dev->name);
			else
				log_notice("Can't get shared mii bus on %s\n", eth_dev->name);

			return uc_priv->mdio_bus;
		}
	} else {
		log_debug("Can't find phy-handle for %s\n", eth_dev->name);
	}

	return NULL;
}

int eth_phy_get_addr(struct udevice *dev)
{
	struct ofnode_phandle_args phandle_args;
	int reg;

	if (dev_read_phandle_with_args(dev, "phy-handle", NULL, 0, 0,
				       &phandle_args)) {
		dev_dbg(dev, "Failed to find phy-handle");
		return -ENODEV;
	}

	reg = ofnode_read_u32_default(phandle_args.node, "reg", 0);

	return reg;
}

/* parsing generic properties of devicetree/bindings/net/ethernet-phy.yaml */
static int eth_phy_of_to_plat(struct udevice *dev)
{
	struct eth_phy_device_priv *uc_priv = dev_get_uclass_priv(dev);
	int ret;

	if (!CONFIG_IS_ENABLED(DM_GPIO))
		return 0;

	/* search "reset-gpios" in phy node */
	ret = gpio_request_by_name(dev, "reset-gpios", 0,
				   &uc_priv->reset_gpio,
				   GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);
	if (ret && ret != -ENOENT)
		return ret;

	uc_priv->reset_assert_delay = dev_read_u32_default(dev, "reset-assert-us", 0);
	uc_priv->reset_deassert_delay = dev_read_u32_default(dev, "reset-deassert-us", 0);

	/* These are used by some DTs, try these as a fallback. */
	if (!uc_priv->reset_assert_delay && !uc_priv->reset_deassert_delay) {
		uc_priv->reset_assert_delay =
			dev_read_u32_default(dev, "reset-delay-us", 0);
		uc_priv->reset_deassert_delay =
			dev_read_u32_default(dev, "reset-post-delay-us", 0);
	}

	return 0;
}

static void eth_phy_reset(struct udevice *dev, int value)
{
	struct eth_phy_device_priv *uc_priv = dev_get_uclass_priv(dev);
	u32 delay;

	if (!CONFIG_IS_ENABLED(DM_GPIO))
		return;

	if (!dm_gpio_is_valid(&uc_priv->reset_gpio))
		return;

	dm_gpio_set_value(&uc_priv->reset_gpio, value);

	delay = value ? uc_priv->reset_assert_delay : uc_priv->reset_deassert_delay;
	if (delay)
		udelay(delay);
}

static int eth_phy_pre_probe(struct udevice *dev)
{
	/* Assert and deassert the reset signal */
	eth_phy_reset(dev, 1);
	eth_phy_reset(dev, 0);

	return 0;
}

UCLASS_DRIVER(eth_phy_generic) = {
	.id		= UCLASS_ETH_PHY,
	.name		= "eth_phy_generic",
	.per_device_auto	= sizeof(struct eth_phy_device_priv),
	.pre_probe	= eth_phy_pre_probe,
};

U_BOOT_DRIVER(eth_phy_generic_drv) = {
	.name		= "eth_phy_generic_drv",
	.id		= UCLASS_ETH_PHY,
	.of_to_plat	= eth_phy_of_to_plat,
};
