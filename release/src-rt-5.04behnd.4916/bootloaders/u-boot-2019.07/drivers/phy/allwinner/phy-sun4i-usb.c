/*
 * Allwinner sun4i USB PHY driver
 *
 * Copyright (C) 2017 Jagan Teki <jagan@amarulasolutions.com>
 * Copyright (C) 2015 Hans de Goede <hdegoede@redhat.com>
 * Copyright (C) 2014 Roman Byshko <rbyshko@gmail.com>
 *
 * Modelled arch/arm/mach-sunxi/usb_phy.c to compatible with generic-phy.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dm/device.h>
#include <generic-phy.h>
#include <phy-sun4i-usb.h>
#include <reset.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/cpu.h>

#define REG_ISCR			0x00
#define REG_PHYCTL_A10			0x04
#define REG_PHYBIST			0x08
#define REG_PHYTUNE			0x0c
#define REG_PHYCTL_A33			0x10
#define REG_PHY_OTGCTL			0x20
#define REG_PMU_UNK1			0x10

/* Common Control Bits for Both PHYs */
#define PHY_PLL_BW			0x03
#define PHY_RES45_CAL_EN		0x0c

/* Private Control Bits for Each PHY */
#define PHY_TX_AMPLITUDE_TUNE		0x20
#define PHY_TX_SLEWRATE_TUNE		0x22
#define PHY_DISCON_TH_SEL		0x2a
#define PHY_SQUELCH_DETECT		0x3c

#define PHYCTL_DATA			BIT(7)
#define OTGCTL_ROUTE_MUSB		BIT(0)

#define PHY_TX_RATE			BIT(4)
#define PHY_TX_MAGNITUDE		BIT(2)
#define PHY_TX_AMPLITUDE_LEN		5

#define PHY_RES45_CAL_DATA		BIT(0)
#define PHY_RES45_CAL_LEN		1
#define PHY_DISCON_TH_LEN		2

#define SUNXI_AHB_ICHR8_EN		BIT(10)
#define SUNXI_AHB_INCR4_BURST_EN	BIT(9)
#define SUNXI_AHB_INCRX_ALIGN_EN	BIT(8)
#define SUNXI_ULPI_BYPASS_EN		BIT(0)

/* A83T specific control bits for PHY0 */
#define PHY_CTL_VBUSVLDEXT		BIT(5)
#define PHY_CTL_SIDDQ			BIT(3)

/* A83T specific control bits for PHY2 HSIC */
#define SUNXI_EHCI_HS_FORCE		BIT(20)
#define SUNXI_HSIC_CONNECT_INT		BIT(16)
#define SUNXI_HSIC			BIT(1)

#define MAX_PHYS			4

enum sun4i_usb_phy_type {
	sun4i_a10_phy,
	sun6i_a31_phy,
	sun8i_a33_phy,
	sun8i_a83t_phy,
	sun8i_h3_phy,
	sun8i_v3s_phy,
	sun50i_a64_phy,
};

struct sun4i_usb_phy_cfg {
	int num_phys;
	enum sun4i_usb_phy_type type;
	u32 disc_thresh;
	u8 phyctl_offset;
	bool dedicated_clocks;
	bool enable_pmu_unk1;
	bool phy0_dual_route;
};

struct sun4i_usb_phy_info {
	const char *gpio_vbus;
	const char *gpio_vbus_det;
	const char *gpio_id_det;
} phy_info[] = {
	{
		.gpio_vbus = CONFIG_USB0_VBUS_PIN,
		.gpio_vbus_det = CONFIG_USB0_VBUS_DET,
		.gpio_id_det = CONFIG_USB0_ID_DET,
	},
	{
		.gpio_vbus = CONFIG_USB1_VBUS_PIN,
		.gpio_vbus_det = NULL,
		.gpio_id_det = NULL,
	},
	{
		.gpio_vbus = CONFIG_USB2_VBUS_PIN,
		.gpio_vbus_det = NULL,
		.gpio_id_det = NULL,
	},
	{
		.gpio_vbus = CONFIG_USB3_VBUS_PIN,
		.gpio_vbus_det = NULL,
		.gpio_id_det = NULL,
	},
};

struct sun4i_usb_phy_plat {
	void __iomem *pmu;
	int power_on_count;
	int gpio_vbus;
	int gpio_vbus_det;
	int gpio_id_det;
	struct clk clocks;
	struct reset_ctl resets;
	int id;
};

struct sun4i_usb_phy_data {
	void __iomem *base;
	const struct sun4i_usb_phy_cfg *cfg;
	struct sun4i_usb_phy_plat *usb_phy;
};

static int initial_usb_scan_delay = CONFIG_INITIAL_USB_SCAN_DELAY;

static void sun4i_usb_phy_write(struct phy *phy, u32 addr, u32 data, int len)
{
	struct sun4i_usb_phy_data *phy_data = dev_get_priv(phy->dev);
	struct sun4i_usb_phy_plat *usb_phy = &phy_data->usb_phy[phy->id];
	u32 temp, usbc_bit = BIT(usb_phy->id * 2);
	void __iomem *phyctl = phy_data->base + phy_data->cfg->phyctl_offset;
	int i;

	if (phy_data->cfg->phyctl_offset == REG_PHYCTL_A33) {
		/* SoCs newer than A33 need us to set phyctl to 0 explicitly */
		writel(0, phyctl);
	}

	for (i = 0; i < len; i++) {
		temp = readl(phyctl);

		/* clear the address portion */
		temp &= ~(0xff << 8);

		/* set the address */
		temp |= ((addr + i) << 8);
		writel(temp, phyctl);

		/* set the data bit and clear usbc bit*/
		temp = readb(phyctl);
		if (data & 0x1)
			temp |= PHYCTL_DATA;
		else
			temp &= ~PHYCTL_DATA;
		temp &= ~usbc_bit;
		writeb(temp, phyctl);

		/* pulse usbc_bit */
		temp = readb(phyctl);
		temp |= usbc_bit;
		writeb(temp, phyctl);

		temp = readb(phyctl);
		temp &= ~usbc_bit;
		writeb(temp, phyctl);

		data >>= 1;
	}
}

static void sun4i_usb_phy_passby(struct phy *phy, bool enable)
{
	struct sun4i_usb_phy_data *data = dev_get_priv(phy->dev);
	struct sun4i_usb_phy_plat *usb_phy = &data->usb_phy[phy->id];
	u32 bits, reg_value;

	if (!usb_phy->pmu)
		return;

	bits = SUNXI_AHB_ICHR8_EN | SUNXI_AHB_INCR4_BURST_EN |
		SUNXI_AHB_INCRX_ALIGN_EN | SUNXI_ULPI_BYPASS_EN;

	/* A83T USB2 is HSIC */
	if (data->cfg->type == sun8i_a83t_phy && usb_phy->id == 2)
		bits |= SUNXI_EHCI_HS_FORCE | SUNXI_HSIC_CONNECT_INT |
			SUNXI_HSIC;

	reg_value = readl(usb_phy->pmu);

	if (enable)
		reg_value |= bits;
	else
		reg_value &= ~bits;

	writel(reg_value, usb_phy->pmu);
}

static int sun4i_usb_phy_power_on(struct phy *phy)
{
	struct sun4i_usb_phy_data *data = dev_get_priv(phy->dev);
	struct sun4i_usb_phy_plat *usb_phy = &data->usb_phy[phy->id];

	if (initial_usb_scan_delay) {
		mdelay(initial_usb_scan_delay);
		initial_usb_scan_delay = 0;
	}

	usb_phy->power_on_count++;
	if (usb_phy->power_on_count != 1)
		return 0;

	if (usb_phy->gpio_vbus >= 0)
		gpio_set_value(usb_phy->gpio_vbus, SUNXI_GPIO_PULL_UP);

	return 0;
}

static int sun4i_usb_phy_power_off(struct phy *phy)
{
	struct sun4i_usb_phy_data *data = dev_get_priv(phy->dev);
	struct sun4i_usb_phy_plat *usb_phy = &data->usb_phy[phy->id];

	usb_phy->power_on_count--;
	if (usb_phy->power_on_count != 0)
		return 0;

	if (usb_phy->gpio_vbus >= 0)
		gpio_set_value(usb_phy->gpio_vbus, SUNXI_GPIO_PULL_DISABLE);

	return 0;
}

static void sun4i_usb_phy0_reroute(struct sun4i_usb_phy_data *data, bool id_det)
{
	u32 regval;

	regval = readl(data->base + REG_PHY_OTGCTL);
	if (!id_det) {
		/* Host mode. Route phy0 to EHCI/OHCI */
		regval &= ~OTGCTL_ROUTE_MUSB;
	} else {
		/* Peripheral mode. Route phy0 to MUSB */
		regval |= OTGCTL_ROUTE_MUSB;
	}
	writel(regval, data->base + REG_PHY_OTGCTL);
}

static int sun4i_usb_phy_init(struct phy *phy)
{
	struct sun4i_usb_phy_data *data = dev_get_priv(phy->dev);
	struct sun4i_usb_phy_plat *usb_phy = &data->usb_phy[phy->id];
	u32 val;
	int ret;

	ret = clk_enable(&usb_phy->clocks);
	if (ret) {
		dev_err(dev, "failed to enable usb_%ldphy clock\n", phy->id);
		return ret;
	}

	ret = reset_deassert(&usb_phy->resets);
	if (ret) {
		dev_err(dev, "failed to deassert usb_%ldreset reset\n", phy->id);
		return ret;
	}

	if (data->cfg->type == sun8i_a83t_phy) {
		if (phy->id == 0) {
			val = readl(data->base + data->cfg->phyctl_offset);
			val |= PHY_CTL_VBUSVLDEXT;
			val &= ~PHY_CTL_SIDDQ;
			writel(val, data->base + data->cfg->phyctl_offset);
		}
	} else {
		if (usb_phy->pmu && data->cfg->enable_pmu_unk1) {
			val = readl(usb_phy->pmu + REG_PMU_UNK1);
			writel(val & ~2, usb_phy->pmu + REG_PMU_UNK1);
		}

		if (usb_phy->id == 0)
			sun4i_usb_phy_write(phy, PHY_RES45_CAL_EN,
					    PHY_RES45_CAL_DATA,
					    PHY_RES45_CAL_LEN);

		/* Adjust PHY's magnitude and rate */
		sun4i_usb_phy_write(phy, PHY_TX_AMPLITUDE_TUNE,
				    PHY_TX_MAGNITUDE | PHY_TX_RATE,
				    PHY_TX_AMPLITUDE_LEN);

		/* Disconnect threshold adjustment */
		sun4i_usb_phy_write(phy, PHY_DISCON_TH_SEL,
				    data->cfg->disc_thresh, PHY_DISCON_TH_LEN);
	}

	sun4i_usb_phy_passby(phy, true);

	sun4i_usb_phy0_reroute(data, true);

	return 0;
}

static int sun4i_usb_phy_exit(struct phy *phy)
{
	struct sun4i_usb_phy_data *data = dev_get_priv(phy->dev);
	struct sun4i_usb_phy_plat *usb_phy = &data->usb_phy[phy->id];
	int ret;

	if (phy->id == 0) {
		if (data->cfg->type == sun8i_a83t_phy) {
			void __iomem *phyctl = data->base +
				data->cfg->phyctl_offset;

			writel(readl(phyctl) | PHY_CTL_SIDDQ, phyctl);
		}
	}

	sun4i_usb_phy_passby(phy, false);

	ret = clk_disable(&usb_phy->clocks);
	if (ret) {
		dev_err(dev, "failed to disable usb_%ldphy clock\n", phy->id);
		return ret;
	}

	ret = reset_assert(&usb_phy->resets);
	if (ret) {
		dev_err(dev, "failed to assert usb_%ldreset reset\n", phy->id);
		return ret;
	}

	return 0;
}

static int sun4i_usb_phy_xlate(struct phy *phy,
			       struct ofnode_phandle_args *args)
{
	struct sun4i_usb_phy_data *data = dev_get_priv(phy->dev);

	if (args->args_count >= data->cfg->num_phys)
		return -EINVAL;

	if (args->args_count)
		phy->id = args->args[0];
	else
		phy->id = 0;

	debug("%s: phy_id = %ld\n", __func__, phy->id);
	return 0;
}

int sun4i_usb_phy_vbus_detect(struct phy *phy)
{
	struct sun4i_usb_phy_data *data = dev_get_priv(phy->dev);
	struct sun4i_usb_phy_plat *usb_phy = &data->usb_phy[phy->id];
	int err, retries = 3;

	debug("%s: id_det = %d\n", __func__, usb_phy->gpio_id_det);

	if (usb_phy->gpio_vbus_det < 0)
		return usb_phy->gpio_vbus_det;

	err = gpio_get_value(usb_phy->gpio_vbus_det);
	/*
	 * Vbus may have been provided by the board and just been turned of
	 * some milliseconds ago on reset, what we're measuring then is a
	 * residual charge on Vbus, sleep a bit and try again.
	 */
	while (err > 0 && retries--) {
		mdelay(100);
		err = gpio_get_value(usb_phy->gpio_vbus_det);
	}

	return err;
}

int sun4i_usb_phy_id_detect(struct phy *phy)
{
	struct sun4i_usb_phy_data *data = dev_get_priv(phy->dev);
	struct sun4i_usb_phy_plat *usb_phy = &data->usb_phy[phy->id];

	debug("%s: id_det = %d\n", __func__, usb_phy->gpio_id_det);

	if (usb_phy->gpio_id_det < 0)
		return usb_phy->gpio_id_det;

	return gpio_get_value(usb_phy->gpio_id_det);
}

void sun4i_usb_phy_set_squelch_detect(struct phy *phy, bool enabled)
{
	sun4i_usb_phy_write(phy, PHY_SQUELCH_DETECT, enabled ? 0 : 2, 2);
}

static struct phy_ops sun4i_usb_phy_ops = {
	.of_xlate = sun4i_usb_phy_xlate,
	.init = sun4i_usb_phy_init,
	.power_on = sun4i_usb_phy_power_on,
	.power_off = sun4i_usb_phy_power_off,
	.exit = sun4i_usb_phy_exit,
};

static int sun4i_usb_phy_probe(struct udevice *dev)
{
	struct sun4i_usb_phy_plat *plat = dev_get_platdata(dev);
	struct sun4i_usb_phy_data *data = dev_get_priv(dev);
	int i, ret;

	data->cfg = (const struct sun4i_usb_phy_cfg *)dev_get_driver_data(dev);
	if (!data->cfg)
		return -EINVAL;

	data->base = (void __iomem *)devfdt_get_addr_name(dev, "phy_ctrl");
	if (IS_ERR(data->base))
		return PTR_ERR(data->base);

	data->usb_phy = plat;
	for (i = 0; i < data->cfg->num_phys; i++) {
		struct sun4i_usb_phy_plat *phy = &plat[i];
		struct sun4i_usb_phy_info *info = &phy_info[i];
		char name[16];

		phy->gpio_vbus = sunxi_name_to_gpio(info->gpio_vbus);
		if (phy->gpio_vbus >= 0) {
			ret = gpio_request(phy->gpio_vbus, "usb_vbus");
			if (ret)
				return ret;
			ret = gpio_direction_output(phy->gpio_vbus, 0);
			if (ret)
				return ret;
		}

		phy->gpio_vbus_det = sunxi_name_to_gpio(info->gpio_vbus_det);
		if (phy->gpio_vbus_det >= 0) {
			ret = gpio_request(phy->gpio_vbus_det, "usb_vbus_det");
			if (ret)
				return ret;
			ret = gpio_direction_input(phy->gpio_vbus_det);
			if (ret)
				return ret;
		}

		phy->gpio_id_det = sunxi_name_to_gpio(info->gpio_id_det);
		if (phy->gpio_id_det >= 0) {
			ret = gpio_request(phy->gpio_id_det, "usb_id_det");
			if (ret)
				return ret;
			ret = gpio_direction_input(phy->gpio_id_det);
			if (ret)
				return ret;
			sunxi_gpio_set_pull(phy->gpio_id_det, SUNXI_GPIO_PULL_UP);
		}

		if (data->cfg->dedicated_clocks)
			snprintf(name, sizeof(name), "usb%d_phy", i);
		else
			strlcpy(name, "usb_phy", sizeof(name));

		ret = clk_get_by_name(dev, name, &phy->clocks);
		if (ret) {
			dev_err(dev, "failed to get usb%d_phy clock phandle\n", i);
			return ret;
		}

		snprintf(name, sizeof(name), "usb%d_reset", i);
		ret = reset_get_by_name(dev, name, &phy->resets);
		if (ret) {
			dev_err(dev, "failed to get usb%d_reset reset phandle\n", i);
			return ret;
		}

		if (i || data->cfg->phy0_dual_route) {
			snprintf(name, sizeof(name), "pmu%d", i);
			phy->pmu = (void __iomem *)devfdt_get_addr_name(dev, name);
			if (IS_ERR(phy->pmu))
				return PTR_ERR(phy->pmu);
		}

		phy->id = i;
	};

	debug("Allwinner Sun4I USB PHY driver loaded\n");
	return 0;
}

static const struct sun4i_usb_phy_cfg sun4i_a10_cfg = {
	.num_phys = 3,
	.type = sun4i_a10_phy,
	.disc_thresh = 3,
	.phyctl_offset = REG_PHYCTL_A10,
	.dedicated_clocks = false,
	.enable_pmu_unk1 = false,
};

static const struct sun4i_usb_phy_cfg sun5i_a13_cfg = {
	.num_phys = 2,
	.type = sun4i_a10_phy,
	.disc_thresh = 2,
	.phyctl_offset = REG_PHYCTL_A10,
	.dedicated_clocks = false,
	.enable_pmu_unk1 = false,
};

static const struct sun4i_usb_phy_cfg sun6i_a31_cfg = {
	.num_phys = 3,
	.type = sun6i_a31_phy,
	.disc_thresh = 3,
	.phyctl_offset = REG_PHYCTL_A10,
	.dedicated_clocks = true,
	.enable_pmu_unk1 = false,
};

static const struct sun4i_usb_phy_cfg sun7i_a20_cfg = {
	.num_phys = 3,
	.type = sun4i_a10_phy,
	.disc_thresh = 2,
	.phyctl_offset = REG_PHYCTL_A10,
	.dedicated_clocks = false,
	.enable_pmu_unk1 = false,
};

static const struct sun4i_usb_phy_cfg sun8i_a23_cfg = {
	.num_phys = 2,
	.type = sun4i_a10_phy,
	.disc_thresh = 3,
	.phyctl_offset = REG_PHYCTL_A10,
	.dedicated_clocks = true,
	.enable_pmu_unk1 = false,
};

static const struct sun4i_usb_phy_cfg sun8i_a33_cfg = {
	.num_phys = 2,
	.type = sun8i_a33_phy,
	.disc_thresh = 3,
	.phyctl_offset = REG_PHYCTL_A33,
	.dedicated_clocks = true,
	.enable_pmu_unk1 = false,
};

static const struct sun4i_usb_phy_cfg sun8i_a83t_cfg = {
	.num_phys = 3,
	.type = sun8i_a83t_phy,
	.phyctl_offset = REG_PHYCTL_A33,
	.dedicated_clocks = true,
};

static const struct sun4i_usb_phy_cfg sun8i_h3_cfg = {
	.num_phys = 4,
	.type = sun8i_h3_phy,
	.disc_thresh = 3,
	.phyctl_offset = REG_PHYCTL_A33,
	.dedicated_clocks = true,
	.enable_pmu_unk1 = true,
	.phy0_dual_route = true,
};

static const struct sun4i_usb_phy_cfg sun8i_v3s_cfg = {
	.num_phys = 1,
	.type = sun8i_v3s_phy,
	.disc_thresh = 3,
	.phyctl_offset = REG_PHYCTL_A33,
	.dedicated_clocks = true,
	.enable_pmu_unk1 = true,
	.phy0_dual_route = true,
};

static const struct sun4i_usb_phy_cfg sun50i_a64_cfg = {
	.num_phys = 2,
	.type = sun50i_a64_phy,
	.disc_thresh = 3,
	.phyctl_offset = REG_PHYCTL_A33,
	.dedicated_clocks = true,
	.enable_pmu_unk1 = true,
	.phy0_dual_route = true,
};

static const struct udevice_id sun4i_usb_phy_ids[] = {
	{ .compatible = "allwinner,sun4i-a10-usb-phy", .data = (ulong)&sun4i_a10_cfg },
	{ .compatible = "allwinner,sun5i-a13-usb-phy", .data = (ulong)&sun5i_a13_cfg },
	{ .compatible = "allwinner,sun6i-a31-usb-phy", .data = (ulong)&sun6i_a31_cfg },
	{ .compatible = "allwinner,sun7i-a20-usb-phy", .data = (ulong)&sun7i_a20_cfg },
	{ .compatible = "allwinner,sun8i-a23-usb-phy", .data = (ulong)&sun8i_a23_cfg },
	{ .compatible = "allwinner,sun8i-a33-usb-phy", .data = (ulong)&sun8i_a33_cfg },
	{ .compatible = "allwinner,sun8i-a83t-usb-phy", .data = (ulong)&sun8i_a83t_cfg },
	{ .compatible = "allwinner,sun8i-h3-usb-phy", .data = (ulong)&sun8i_h3_cfg },
	{ .compatible = "allwinner,sun8i-v3s-usb-phy", .data = (ulong)&sun8i_v3s_cfg },
	{ .compatible = "allwinner,sun50i-a64-usb-phy", .data = (ulong)&sun50i_a64_cfg},
	{ }
};

U_BOOT_DRIVER(sun4i_usb_phy) = {
	.name	= "sun4i_usb_phy",
	.id	= UCLASS_PHY,
	.of_match = sun4i_usb_phy_ids,
	.ops = &sun4i_usb_phy_ops,
	.probe = sun4i_usb_phy_probe,
	.platdata_auto_alloc_size = sizeof(struct sun4i_usb_phy_plat[MAX_PHYS]),
	.priv_auto_alloc_size = sizeof(struct sun4i_usb_phy_data),
};
