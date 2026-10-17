// SPDX-License-Identifier: GPL-2.0+
/*
 * OMAP USB2 PHY LAYER
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com
 * Written by Jean-Jacques Hiblot <jjhiblot@ti.com>
 */

#include <common.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <dm.h>
#include <errno.h>
#include <generic-phy.h>
#include <regmap.h>
#include <soc.h>
#include <syscon.h>
#include <linux/bitops.h>
#include <linux/err.h>

#define OMAP_USB2_CALIBRATE_FALSE_DISCONNECT	BIT(0)
#define OMAP_USB2_DISABLE_CHG_DET		BIT(1)

#define OMAP_DEV_PHY_PD		BIT(0)
#define OMAP_USB2_PHY_PD	BIT(28)

#define AM437X_USB2_PHY_PD		BIT(0)
#define AM437X_USB2_OTG_PD		BIT(1)
#define AM437X_USB2_OTGVDET_EN		BIT(19)
#define AM437X_USB2_OTGSESSEND_EN	BIT(20)

#define USB2PHY_DISCON_BYP_LATCH	BIT(31)
#define USB2PHY_ANA_CONFIG1		(0x4c)

#define AM654_USB2_OTG_PD		BIT(8)
#define AM654_USB2_VBUS_DET_EN		BIT(5)
#define AM654_USB2_VBUSVALID_DET_EN	BIT(4)

#define USB2PHY_CHRG_DET		 0x14
#define USB2PHY_USE_CHG_DET_REG		BIT(29)
#define USB2PHY_DIS_CHG_DET		BIT(28)

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

static const struct usb_phy_data am654_usb2_data = {
	.label = "am654_usb2",
	.flags = OMAP_USB2_CALIBRATE_FALSE_DISCONNECT,
	.mask = AM654_USB2_OTG_PD | AM654_USB2_VBUS_DET_EN |
		AM654_USB2_VBUSVALID_DET_EN,
	.power_on = AM654_USB2_VBUS_DET_EN | AM654_USB2_VBUSVALID_DET_EN,
	.power_off = AM654_USB2_OTG_PD,
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
	{
		.compatible = "ti,am654-usb2",
		.data = (ulong)&am654_usb2_data,
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

	if (priv->flags & OMAP_USB2_DISABLE_CHG_DET) {
		val = readl(priv->phy_base + USB2PHY_CHRG_DET);
		val |= USB2PHY_USE_CHG_DET_REG | USB2PHY_DIS_CHG_DET;
		writel(val, priv->phy_base + USB2PHY_CHRG_DET);
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

static const struct soc_attr am65x_sr10_soc_devices[] = {
	{ .family = "AM65X", .revision = "SR1.0" },
	{ /* sentinel */ }
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

	priv->phy_base = dev_read_addr_ptr(dev);

	if (!priv->phy_base)
		return -EINVAL;

	if (data->flags & OMAP_USB2_CALIBRATE_FALSE_DISCONNECT)
		priv->flags |= OMAP_USB2_CALIBRATE_FALSE_DISCONNECT;

	/*
	 * AM654x PG1.0 has a silicon bug that D+ is pulled high after
	 * POR, which could cause enumeration failure with some USB hubs.
	 * Disabling the USB2_PHY Charger Detect function will put D+
	 * into the normal state.
	 *
	 * Enable this workaround for AM654x PG1.0.
	 */
	if (soc_device_match(am65x_sr10_soc_devices))
		priv->flags |= OMAP_USB2_DISABLE_CHG_DET;

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
	.priv_auto	= sizeof(struct omap_usb2_phy),
};
