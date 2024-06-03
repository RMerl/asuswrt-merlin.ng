// SPDX-License-Identifier: GPL-2.0+
/*
 * OMAP USB2 PHY LAYER
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com
 * Written by Jean-Jacques Hiblot <jjhiblot@ti.com>
 */

#include <common.h>
#include <asm/io.h>
#include <dm.h>
#include <errno.h>
#include <generic-phy.h>
#include <regmap.h>
#include <syscon.h>

#define OMAP_USB2_CALIBRATE_FALSE_DISCONNECT	BIT(0)

#define OMAP_DEV_PHY_PD		BIT(0)
#define OMAP_USB2_PHY_PD	BIT(28)

#define AM437X_USB2_PHY_PD		BIT(0)
#define AM437X_USB2_OTG_PD		BIT(1)
#define AM437X_USB2_OTGVDET_EN		BIT(19)
#define AM437X_USB2_OTGSESSEND_EN	BIT(20)

#define USB2PHY_DISCON_BYP_LATCH	BIT(31)
#define USB2PHY_ANA_CONFIG1		(0x4c)

DECLARE_GLOBAL_DATA_PTR;

struct omap_usb2_phy {
	struct regmap *pwr_regmap;
	ulong flags;
	void *phy_base;
	u32 pwr_reg_offset;
};

struct usb_phy_data {
	const char *label;
	u8 flags;
	u32 mask;
	u32 power_on;
	u32 power_off;
};

static const struct usb_phy_data omap5_usb2_data = {
	.label = "omap5_usb2",
	.flags = 0,
	.mask = OMAP_DEV_PHY_PD,
	.power_off = OMAP_DEV_PHY_PD,
};

static const struct usb_phy_data dra7x_usb2_data = {
	.label = "dra7x_usb2",
	.flags = OMAP_USB2_CALIBRATE_FALSE_DISCONNECT,
	.mask = OMAP_DEV_PHY_PD,
	.power_off = OMAP_DEV_PHY_PD,
};

static const struct usb_phy_data dra7x_usb2_phy2_data = {
	.label = "dra7x_usb2_phy2",
	.flags = OMAP_USB2_CALIBRATE_FALSE_DISCONNECT,
	.mask = OMAP_USB2_PHY_PD,
	.power_off = OMAP_USB2_PHY_PD,
};

static const struct usb_phy_data am437x_usb2_data = {
	.label = "am437x_usb2",
	.flags =  0,
	.mask = AM437X_USB2_PHY_PD | AM437X_USB2_OTG_PD |
		AM437X_USB2_OTGVDET_EN | AM437X_USB2_OTGSESSEND_EN,
	.power_on = AM437X_USB2_OTGVDET_EN | AM437X_USB2_OTGSESSEND_EN,
	.power_off = AM437X_USB2_PHY_PD | AM437X_USB2_OTG_PD,
};

static const struct udevice_id omap_usb2_id_table[] = {
	{
		.compatible = "ti,omap5-usb2",
		.data = (ulong)&omap5_usb2_data,
	},
	{
		.compatible = "ti,dra7x-usb2",
		.data = (ulong)&dra7x_usb2_data,
	},
	{
		.compatible = "ti,dra7x-usb2-phy2",
		.data = (ulong)&dra7x_usb2_phy2_data,
	},
	{
		.compatible = "ti,am437x-usb2",
		.data = (ulong)&am437x_usb2_data,
	},
	{},
};

static int omap_usb_phy_power(struct phy *usb_phy, bool on)
{
	struct udevice *dev = usb_phy->dev;
	const struct usb_phy_data *data;
	const struct omap_usb2_phy *phy = dev_get_priv(dev);
	u32 val;
	int rc;

	data = (const struct usb_phy_data *)dev_get_driver_data(dev);
	if (!data)
		return -EINVAL;

	rc = regmap_read(phy->pwr_regmap, phy->pwr_reg_offset, &val);
	if (rc)
		return rc;
	val &= ~data->mask;
	if (on)
		val |= data->power_on;
	else
		val |= data->power_off;
	rc = regmap_write(phy->pwr_regmap, phy->pwr_reg_offset, val);
	if (rc)
		return rc;

	return 0;
}

static int omap_usb2_phy_init(struct phy *usb_phy)
{
	struct udevice *dev = usb_phy->dev;
	struct omap_usb2_phy *priv = dev_get_priv(dev);
	u32 val;

	if (priv->flags & OMAP_USB2_CALIBRATE_FALSE_DISCONNECT) {
		/*
		 *
		 * Reduce the sensitivity of internal PHY by enabling the
		 * DISCON_BYP_LATCH of the USB2PHY_ANA_CONFIG1 register. This
		 * resolves issues with certain devices which can otherwise
		 * be prone to false disconnects.
		 *
		 */
		val = readl(priv->phy_base + USB2PHY_ANA_CONFIG1);
		val |= USB2PHY_DISCON_BYP_LATCH;
		writel(val, priv->phy_base + USB2PHY_ANA_CONFIG1);
	}

	return 0;
}

static int omap_usb2_phy_power_on(struct phy *usb_phy)
{
	return omap_usb_phy_power(usb_phy, true);
}

static int omap_usb2_phy_power_off(struct phy *usb_phy)
{
	return omap_usb_phy_power(usb_phy, false);
}

static int omap_usb2_phy_exit(struct phy *usb_phy)
{
	return omap_usb_phy_power(usb_phy, false);
}

struct phy_ops omap_usb2_phy_ops = {
	.init = omap_usb2_phy_init,
	.power_on = omap_usb2_phy_power_on,
	.power_off = omap_usb2_phy_power_off,
	.exit = omap_usb2_phy_exit,
};

int omap_usb2_phy_probe(struct udevice *dev)
{
	int rc;
	struct regmap *regmap;
	struct omap_usb2_phy *priv = dev_get_priv(dev);
	const struct usb_phy_data *data;
	u32 tmp[2];

	data = (const struct usb_phy_data *)dev_get_driver_data(dev);
	if (!data)
		return -EINVAL;

	if (data->flags & OMAP_USB2_CALIBRATE_FALSE_DISCONNECT) {
		u32 base = dev_read_addr(dev);

		if (base == FDT_ADDR_T_NONE)
			return -EINVAL;
		priv->phy_base = (void *)base;
		priv->flags |= OMAP_USB2_CALIBRATE_FALSE_DISCONNECT;
	}

	regmap = syscon_regmap_lookup_by_phandle(dev, "syscon-phy-power");
	if (!IS_ERR(regmap)) {
		priv->pwr_regmap = regmap;
		rc =  dev_read_u32_array(dev, "syscon-phy-power", tmp, 2);
		if (rc) {
			printf("couldn't get power reg. offset (err %d)\n", rc);
			return rc;
		}
		priv->pwr_reg_offset = tmp[1];
		return 0;
	}
	regmap = syscon_regmap_lookup_by_phandle(dev, "ctrl-module");
	if (!IS_ERR(regmap)) {
		priv->pwr_regmap = regmap;
		priv->pwr_reg_offset = 0;
		return 0;
	}

	printf("can't get regmap (err %ld)\n", PTR_ERR(regmap));
	return PTR_ERR(regmap);
}

U_BOOT_DRIVER(omap_usb2_phy) = {
	.name = "omap_usb2_phy",
	.id = UCLASS_PHY,
	.of_match = omap_usb2_id_table,
	.probe = omap_usb2_phy_probe,
	.ops = &omap_usb2_phy_ops,
	.priv_auto_alloc_size = sizeof(struct omap_usb2_phy),
};
