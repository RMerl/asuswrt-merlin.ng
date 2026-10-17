// SPDX-License-Identifier: GPL-2.0
/*
 * Amlogic Meson VPU Power Domain Controller driver
 *
 * Copyright (c) 2018 BayLibre, SAS.
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <power-domain-uclass.h>
#include <regmap.h>
#include <syscon.h>
#include <reset.h>
#include <clk.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>

enum {
	VPU_PWRC_COMPATIBLE_GX		= 0,
	VPU_PWRC_COMPATIBLE_G12A	= 1,
};

/* AO Offsets */

#define AO_RTI_GEN_PWR_SLEEP0		(0x3a << 2)

#define GEN_PWR_VPU_HDMI		BIT(8)
#define GEN_PWR_VPU_HDMI_ISO		BIT(9)

/* HHI Offsets */

#define HHI_MEM_PD_REG0			(0x40 << 2)
#define HHI_VPU_MEM_PD_REG0		(0x41 << 2)
#define HHI_VPU_MEM_PD_REG1		(0x42 << 2)
#define HHI_VPU_MEM_PD_REG2		(0x4d << 2)

struct meson_gx_pwrc_vpu_priv {
	struct regmap *regmap_ao;
	struct regmap *regmap_hhi;
	struct reset_ctl_bulk resets;
	struct clk_bulk clks;
};

static int meson_gx_pwrc_vpu_on(struct power_domain *power_domain)
{
	struct meson_gx_pwrc_vpu_priv *priv = dev_get_priv(power_domain->dev);
	int i, ret;

	regmap_update_bits(priv->regmap_ao, AO_RTI_GEN_PWR_SLEEP0,
			   GEN_PWR_VPU_HDMI, 0);
	udelay(20);

	/* Power Up Memories */
	for (i = 0; i < 32; i += 2) {
		regmap_update_bits(priv->regmap_hhi, HHI_VPU_MEM_PD_REG0,
				   0x3 << i, 0);
		udelay(5);
	}

	for (i = 0; i < 32; i += 2) {
		regmap_update_bits(priv->regmap_hhi, HHI_VPU_MEM_PD_REG1,
				   0x3 << i, 0);
		udelay(5);
	}

	for (i = 8; i < 16; i++) {
		regmap_update_bits(priv->regmap_hhi, HHI_MEM_PD_REG0,
				   BIT(i), 0);
		udelay(5);
	}
	udelay(20);

	ret = reset_assert_bulk(&priv->resets);
	if (ret)
		return ret;

	regmap_update_bits(priv->regmap_ao, AO_RTI_GEN_PWR_SLEEP0,
			   GEN_PWR_VPU_HDMI_ISO, 0);

	ret = reset_deassert_bulk(&priv->resets);
	if (ret)
		return ret;

	ret = clk_enable_bulk(&priv->clks);
	if (ret)
		return ret;

	return 0;
}

static int meson_g12a_pwrc_vpu_on(struct power_domain *power_domain)
{
	struct meson_gx_pwrc_vpu_priv *priv = dev_get_priv(power_domain->dev);
	int i, ret;

	regmap_update_bits(priv->regmap_ao, AO_RTI_GEN_PWR_SLEEP0,
			   GEN_PWR_VPU_HDMI, 0);
	udelay(20);

	/* Power Up Memories */
	for (i = 0; i < 32; i += 2) {
		regmap_update_bits(priv->regmap_hhi, HHI_VPU_MEM_PD_REG0,
				   0x3 << i, 0);
		udelay(5);
	}

	for (i = 0; i < 32; i += 2) {
		regmap_update_bits(priv->regmap_hhi, HHI_VPU_MEM_PD_REG1,
				   0x3 << i, 0);
		udelay(5);
	}

	for (i = 0; i < 32; i += 2) {
		regmap_update_bits(priv->regmap_hhi, HHI_VPU_MEM_PD_REG2,
				   0x3 << i, 0);
		udelay(5);
	}

	for (i = 8; i < 16; i++) {
		regmap_update_bits(priv->regmap_hhi, HHI_MEM_PD_REG0,
				   BIT(i), 0);
		udelay(5);
	}
	udelay(20);

	ret = reset_assert_bulk(&priv->resets);
	if (ret)
		return ret;

	regmap_update_bits(priv->regmap_ao, AO_RTI_GEN_PWR_SLEEP0,
			   GEN_PWR_VPU_HDMI_ISO, 0);

	ret = reset_deassert_bulk(&priv->resets);
	if (ret)
		return ret;

	ret = clk_enable_bulk(&priv->clks);
	if (ret)
		return ret;

	return 0;
}

static int meson_pwrc_vpu_on(struct power_domain *power_domain)
{
	unsigned int compat = dev_get_driver_data(power_domain->dev);

	switch (compat) {
	case VPU_PWRC_COMPATIBLE_GX:
		return meson_gx_pwrc_vpu_on(power_domain);
	case VPU_PWRC_COMPATIBLE_G12A:
		return meson_g12a_pwrc_vpu_on(power_domain);
	}

	return -EINVAL;
}

static int meson_gx_pwrc_vpu_off(struct power_domain *power_domain)
{
	struct meson_gx_pwrc_vpu_priv *priv = dev_get_priv(power_domain->dev);
	int i;

	regmap_update_bits(priv->regmap_ao, AO_RTI_GEN_PWR_SLEEP0,
			   GEN_PWR_VPU_HDMI_ISO, GEN_PWR_VPU_HDMI_ISO);
	udelay(20);

	/* Power Down Memories */
	for (i = 0; i < 32; i += 2) {
		regmap_update_bits(priv->regmap_hhi, HHI_VPU_MEM_PD_REG0,
				   0x3 << i, 0x3 << i);
		udelay(5);
	}
	for (i = 0; i < 32; i += 2) {
		regmap_update_bits(priv->regmap_hhi, HHI_VPU_MEM_PD_REG1,
				   0x3 << i, 0x3 << i);
		udelay(5);
	}
	for (i = 8; i < 16; i++) {
		regmap_update_bits(priv->regmap_hhi, HHI_MEM_PD_REG0,
				   BIT(i), BIT(i));
		udelay(5);
	}
	udelay(20);

	regmap_update_bits(priv->regmap_ao, AO_RTI_GEN_PWR_SLEEP0,
			   GEN_PWR_VPU_HDMI, GEN_PWR_VPU_HDMI);
	mdelay(20);

	clk_disable_bulk(&priv->clks);

	return 0;
}

static int meson_g12a_pwrc_vpu_off(struct power_domain *power_domain)
{
	struct meson_gx_pwrc_vpu_priv *priv = dev_get_priv(power_domain->dev);
	int i;

	regmap_update_bits(priv->regmap_ao, AO_RTI_GEN_PWR_SLEEP0,
			   GEN_PWR_VPU_HDMI_ISO, GEN_PWR_VPU_HDMI_ISO);
	udelay(20);

	/* Power Down Memories */
	for (i = 0; i < 32; i += 2) {
		regmap_update_bits(priv->regmap_hhi, HHI_VPU_MEM_PD_REG0,
				   0x3 << i, 0x3 << i);
		udelay(5);
	}
	for (i = 0; i < 32; i += 2) {
		regmap_update_bits(priv->regmap_hhi, HHI_VPU_MEM_PD_REG1,
				   0x3 << i, 0x3 << i);
		udelay(5);
	}
	for (i = 0; i < 32; i += 2) {
		regmap_update_bits(priv->regmap_hhi, HHI_VPU_MEM_PD_REG2,
				   0x3 << i, 0x3 << i);
		udelay(5);
	}
	for (i = 8; i < 16; i++) {
		regmap_update_bits(priv->regmap_hhi, HHI_MEM_PD_REG0,
				   BIT(i), BIT(i));
		udelay(5);
	}
	udelay(20);

	regmap_update_bits(priv->regmap_ao, AO_RTI_GEN_PWR_SLEEP0,
			   GEN_PWR_VPU_HDMI, GEN_PWR_VPU_HDMI);
	mdelay(20);

	clk_disable_bulk(&priv->clks);

	return 0;
}

static int meson_pwrc_vpu_off(struct power_domain *power_domain)
{
	unsigned int compat = dev_get_driver_data(power_domain->dev);

	switch (compat) {
	case VPU_PWRC_COMPATIBLE_GX:
		return meson_gx_pwrc_vpu_off(power_domain);
	case VPU_PWRC_COMPATIBLE_G12A:
		return meson_g12a_pwrc_vpu_off(power_domain);
	}

	return -EINVAL;
}

static int meson_pwrc_vpu_of_xlate(struct power_domain *power_domain,
				   struct ofnode_phandle_args *args)
{
	/* #power-domain-cells is 0 */

	if (args->args_count != 0) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	return 0;
}

struct power_domain_ops meson_gx_pwrc_vpu_ops = {
	.off = meson_pwrc_vpu_off,
	.on = meson_pwrc_vpu_on,
	.of_xlate = meson_pwrc_vpu_of_xlate,
};

static const struct udevice_id meson_gx_pwrc_vpu_ids[] = {
	{
		.compatible = "amlogic,meson-gx-pwrc-vpu",
		.data = VPU_PWRC_COMPATIBLE_GX,
	},
	{
		.compatible = "amlogic,meson-g12a-pwrc-vpu",
		.data = VPU_PWRC_COMPATIBLE_G12A,
	},
	{ }
};

static int meson_gx_pwrc_vpu_probe(struct udevice *dev)
{
	struct meson_gx_pwrc_vpu_priv *priv = dev_get_priv(dev);
	u32 hhi_phandle;
	ofnode hhi_node;
	int ret;

	priv->regmap_ao = syscon_node_to_regmap(dev_ofnode(dev_get_parent(dev)));
	if (IS_ERR(priv->regmap_ao))
		return PTR_ERR(priv->regmap_ao);

	ret = ofnode_read_u32(dev_ofnode(dev), "amlogic,hhi-sysctrl",
			      &hhi_phandle);
	if (ret)
		return ret;

	hhi_node = ofnode_get_by_phandle(hhi_phandle);
	if (!ofnode_valid(hhi_node))
		return -EINVAL;

	priv->regmap_hhi = syscon_node_to_regmap(hhi_node);
	if (IS_ERR(priv->regmap_hhi))
		return PTR_ERR(priv->regmap_hhi);

	ret = reset_get_bulk(dev, &priv->resets);
	if (ret)
		return ret;

	ret = clk_get_bulk(dev, &priv->clks);
	if (ret)
		return ret;

	return 0;
}

U_BOOT_DRIVER(meson_gx_pwrc_vpu) = {
	.name = "meson_gx_pwrc_vpu",
	.id = UCLASS_POWER_DOMAIN,
	.of_match = meson_gx_pwrc_vpu_ids,
	.probe = meson_gx_pwrc_vpu_probe,
	.ops = &meson_gx_pwrc_vpu_ops,
	.priv_auto	= sizeof(struct meson_gx_pwrc_vpu_priv),
};
