// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016 Rockchip Inc.
 */

#include <common.h>
#include <display.h>
#include <dm.h>
#include <edid.h>
#include <panel.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/lvds_rk3288.h>
#include <asm/arch-rockchip/grf_rk3288.h>
#include <dt-bindings/clock/rk3288-cru.h>
#include <dt-bindings/video/rk3288.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * struct rk_lvds_priv - private rockchip lvds display driver info
 *
 * @reg: LVDS register address
 * @grf: GRF register
 * @panel: Panel device that is used in driver
 *
 * @output: Output mode, decided single or double channel,
 *		LVDS or LVTLL
 * @format: Data format that RGB data will packing as
 */
struct rk_lvds_priv {
	void __iomem *regs;
	struct rk3288_grf *grf;
	struct udevice *panel;

	int output;
	int format;
};

static inline void lvds_writel(struct rk_lvds_priv *lvds, u32 offset, u32 val)
{
	writel(val, lvds->regs + offset);

	writel(val, lvds->regs + offset + 0x100);
}

int rk_lvds_enable(struct udevice *dev, int panel_bpp,
		   const struct display_timing *edid)
{
	struct rk_lvds_priv *priv = dev_get_priv(dev);
	struct display_plat *uc_plat = dev_get_uclass_platdata(dev);
	int ret = 0;
	unsigned int val = 0;

	ret = panel_enable_backlight(priv->panel);
	if (ret) {
		debug("%s: backlight error: %d\n", __func__, ret);
		return ret;
	}

	/* Select the video source */
	if (uc_plat->source_id)
		val = RK3288_LVDS_SOC_CON6_SEL_VOP_LIT |
		    (RK3288_LVDS_SOC_CON6_SEL_VOP_LIT << 16);
	else
		val = RK3288_LVDS_SOC_CON6_SEL_VOP_LIT << 16;
	rk_setreg(&priv->grf->soc_con6, val);

	/* Select data transfer format */
	val = priv->format;
	if (priv->output == LVDS_OUTPUT_DUAL)
		val |= LVDS_DUAL | LVDS_CH0_EN | LVDS_CH1_EN;
	else if (priv->output == LVDS_OUTPUT_SINGLE)
		val |= LVDS_CH0_EN;
	else if (priv->output == LVDS_OUTPUT_RGB)
		val |= LVDS_TTL_EN | LVDS_CH0_EN | LVDS_CH1_EN;
	val |= (0xffff << 16);
	rk_setreg(&priv->grf->soc_con7, val);

	/* Enable LVDS PHY */
	if (priv->output == LVDS_OUTPUT_RGB) {
		lvds_writel(priv, RK3288_LVDS_CH0_REG0,
			    RK3288_LVDS_CH0_REG0_TTL_EN |
			    RK3288_LVDS_CH0_REG0_LANECK_EN |
			    RK3288_LVDS_CH0_REG0_LANE4_EN |
			    RK3288_LVDS_CH0_REG0_LANE3_EN |
			    RK3288_LVDS_CH0_REG0_LANE2_EN |
			    RK3288_LVDS_CH0_REG0_LANE1_EN |
			    RK3288_LVDS_CH0_REG0_LANE0_EN);
		lvds_writel(priv, RK3288_LVDS_CH0_REG2,
			    RK3288_LVDS_PLL_FBDIV_REG2(0x46));

		lvds_writel(priv, RK3288_LVDS_CH0_REG3,
			    RK3288_LVDS_PLL_FBDIV_REG3(0x46));
		lvds_writel(priv, RK3288_LVDS_CH0_REG4,
			    RK3288_LVDS_CH0_REG4_LANECK_TTL_MODE |
			    RK3288_LVDS_CH0_REG4_LANE4_TTL_MODE |
			    RK3288_LVDS_CH0_REG4_LANE3_TTL_MODE |
			    RK3288_LVDS_CH0_REG4_LANE2_TTL_MODE |
			    RK3288_LVDS_CH0_REG4_LANE1_TTL_MODE |
			    RK3288_LVDS_CH0_REG4_LANE0_TTL_MODE);
		lvds_writel(priv, RK3288_LVDS_CH0_REG5,
			    RK3288_LVDS_CH0_REG5_LANECK_TTL_DATA |
			    RK3288_LVDS_CH0_REG5_LANE4_TTL_DATA |
			    RK3288_LVDS_CH0_REG5_LANE3_TTL_DATA |
			    RK3288_LVDS_CH0_REG5_LANE2_TTL_DATA |
			    RK3288_LVDS_CH0_REG5_LANE1_TTL_DATA |
			    RK3288_LVDS_CH0_REG5_LANE0_TTL_DATA);
		lvds_writel(priv, RK3288_LVDS_CH0_REGD,
			    RK3288_LVDS_PLL_PREDIV_REGD(0x0a));
		lvds_writel(priv, RK3288_LVDS_CH0_REG20,
			    RK3288_LVDS_CH0_REG20_LSB);
	} else {
		lvds_writel(priv, RK3288_LVDS_CH0_REG0,
			    RK3288_LVDS_CH0_REG0_LVDS_EN |
			    RK3288_LVDS_CH0_REG0_LANECK_EN |
			    RK3288_LVDS_CH0_REG0_LANE4_EN |
			    RK3288_LVDS_CH0_REG0_LANE3_EN |
			    RK3288_LVDS_CH0_REG0_LANE2_EN |
			    RK3288_LVDS_CH0_REG0_LANE1_EN |
			    RK3288_LVDS_CH0_REG0_LANE0_EN);
		lvds_writel(priv, RK3288_LVDS_CH0_REG1,
			    RK3288_LVDS_CH0_REG1_LANECK_BIAS |
			    RK3288_LVDS_CH0_REG1_LANE4_BIAS |
			    RK3288_LVDS_CH0_REG1_LANE3_BIAS |
			    RK3288_LVDS_CH0_REG1_LANE2_BIAS |
			    RK3288_LVDS_CH0_REG1_LANE1_BIAS |
			    RK3288_LVDS_CH0_REG1_LANE0_BIAS);
		lvds_writel(priv, RK3288_LVDS_CH0_REG2,
			    RK3288_LVDS_CH0_REG2_RESERVE_ON |
			    RK3288_LVDS_CH0_REG2_LANECK_LVDS_MODE |
			    RK3288_LVDS_CH0_REG2_LANE4_LVDS_MODE |
			    RK3288_LVDS_CH0_REG2_LANE3_LVDS_MODE |
			    RK3288_LVDS_CH0_REG2_LANE2_LVDS_MODE |
			    RK3288_LVDS_CH0_REG2_LANE1_LVDS_MODE |
			    RK3288_LVDS_CH0_REG2_LANE0_LVDS_MODE |
			    RK3288_LVDS_PLL_FBDIV_REG2(0x46));
		lvds_writel(priv, RK3288_LVDS_CH0_REG3,
			    RK3288_LVDS_PLL_FBDIV_REG3(0x46));
		lvds_writel(priv, RK3288_LVDS_CH0_REG4, 0x00);
		lvds_writel(priv, RK3288_LVDS_CH0_REG5, 0x00);
		lvds_writel(priv, RK3288_LVDS_CH0_REGD,
			    RK3288_LVDS_PLL_PREDIV_REGD(0x0a));
		lvds_writel(priv, RK3288_LVDS_CH0_REG20,
			    RK3288_LVDS_CH0_REG20_LSB);
	}

	/* Power on */
	writel(RK3288_LVDS_CFG_REGC_PLL_ENABLE,
	       priv->regs + RK3288_LVDS_CFG_REGC);

	writel(RK3288_LVDS_CFG_REG21_TX_ENABLE,
	       priv->regs + RK3288_LVDS_CFG_REG21);

	return 0;
}

int rk_lvds_read_timing(struct udevice *dev, struct display_timing *timing)
{
	if (fdtdec_decode_display_timing
	    (gd->fdt_blob, dev_of_offset(dev), 0, timing)) {
		debug("%s: Failed to decode display timing\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static int rk_lvds_ofdata_to_platdata(struct udevice *dev)
{
	struct rk_lvds_priv *priv = dev_get_priv(dev);
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(dev);
	int ret;
	priv->regs = (void *)devfdt_get_addr(dev);
	priv->grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);

	ret = fdtdec_get_int(blob, node, "rockchip,output", -1);
	if (ret != -1) {
		priv->output = ret;
		debug("LVDS output : %d\n", ret);
	} else {
		/* default set it as output rgb */
		priv->output = LVDS_OUTPUT_RGB;
	}

	ret = fdtdec_get_int(blob, node, "rockchip,data-mapping", -1);
	if (ret != -1) {
		priv->format = ret;
		debug("LVDS data-mapping : %d\n", ret);
	} else {
		/* default set it as format jeida */
		priv->format = LVDS_FORMAT_JEIDA;
	}

	ret = fdtdec_get_int(blob, node, "rockchip,data-width", -1);
	if (ret != -1) {
		debug("LVDS data-width : %d\n", ret);
		if (ret == 24) {
			priv->format |= LVDS_24BIT;
		} else if (ret == 18) {
			priv->format |= LVDS_18BIT;
		} else {
			debug("rockchip-lvds unsupport data-width[%d]\n", ret);
			ret = -EINVAL;
			return ret;
		}
	} else {
		priv->format |= LVDS_24BIT;
	}

	return 0;
}

int rk_lvds_probe(struct udevice *dev)
{
	struct rk_lvds_priv *priv = dev_get_priv(dev);
	int ret;

	ret = uclass_get_device_by_phandle(UCLASS_PANEL, dev, "rockchip,panel",
					   &priv->panel);
	if (ret) {
		debug("%s: Cannot find panel for '%s' (ret=%d)\n", __func__,
		      dev->name, ret);
		return ret;
	}

	return 0;
}

static const struct dm_display_ops lvds_rockchip_ops = {
	.read_timing = rk_lvds_read_timing,
	.enable = rk_lvds_enable,
};

static const struct udevice_id rockchip_lvds_ids[] = {
	{.compatible = "rockchip,rk3288-lvds"},
	{}
};

U_BOOT_DRIVER(lvds_rockchip) = {
	.name	= "lvds_rockchip",
	.id	= UCLASS_DISPLAY,
	.of_match = rockchip_lvds_ids,
	.ops	= &lvds_rockchip_ops,
	.ofdata_to_platdata	= rk_lvds_ofdata_to_platdata,
	.probe	= rk_lvds_probe,
	.priv_auto_alloc_size	= sizeof(struct rk_lvds_priv),
};
