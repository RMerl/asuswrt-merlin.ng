// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2017, Fuzhou Rockchip Electronics Co., Ltd
 * Author: Eric Gao <eric.gao@rock-chips.com>
 */

#include <common.h>
#include <clk.h>
#include <display.h>
#include <dm.h>
#include <log.h>
#include <panel.h>
#include <regmap.h>
#include "rk_mipi.h"
#include <syscon.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <dm/uclass-internal.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/cru.h>
#include <asm/arch-rockchip/grf_rk3399.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/arch-rockchip/rockchip_mipi_dsi.h>

/* Select mipi dsi source, big or little vop */
static int rk_mipi_dsi_source_select(struct udevice *dev)
{
	struct rk_mipi_priv *priv = dev_get_priv(dev);
	struct rk3399_grf_regs *grf = priv->grf;
	struct display_plat *disp_uc_plat = dev_get_uclass_plat(dev);

	/* Select the video source */
	switch (disp_uc_plat->source_id) {
	case VOP_B:
		rk_clrsetreg(&grf->soc_con20, GRF_DSI0_VOP_SEL_MASK,
			     GRF_DSI0_VOP_SEL_B << GRF_DSI0_VOP_SEL_SHIFT);
		break;
	case VOP_L:
		rk_clrsetreg(&grf->soc_con20, GRF_DSI0_VOP_SEL_MASK,
			     GRF_DSI0_VOP_SEL_L << GRF_DSI0_VOP_SEL_SHIFT);
		break;
	default:
		debug("%s: Invalid VOP id\n", __func__);
		return -EINVAL;
	}

	return 0;
}

/* Setup mipi dphy working mode */
static void rk_mipi_dphy_mode_set(struct udevice *dev)
{
	struct rk_mipi_priv *priv = dev_get_priv(dev);
	struct rk3399_grf_regs *grf = priv->grf;
	int val;

	/* Set Controller as TX mode */
	val = GRF_DPHY_TX0_RXMODE_DIS << GRF_DPHY_TX0_RXMODE_SHIFT;
	rk_clrsetreg(&grf->soc_con22, GRF_DPHY_TX0_RXMODE_MASK, val);

	/* Exit tx stop mode */
	val |= GRF_DPHY_TX0_TXSTOPMODE_DIS << GRF_DPHY_TX0_TXSTOPMODE_SHIFT;
	rk_clrsetreg(&grf->soc_con22, GRF_DPHY_TX0_TXSTOPMODE_MASK, val);

	/* Disable turnequest */
	val |= GRF_DPHY_TX0_TURNREQUEST_DIS << GRF_DPHY_TX0_TURNREQUEST_SHIFT;
	rk_clrsetreg(&grf->soc_con22, GRF_DPHY_TX0_TURNREQUEST_MASK, val);
}

/*
 * This function is called by rk_display_init() using rk_mipi_dsi_enable() and
 * rk_mipi_phy_enable() to initialize mipi controller and dphy. If success,
 * enable backlight.
 */
static int rk_display_enable(struct udevice *dev, int panel_bpp,
			  const struct display_timing *timing)
{
	int ret;
	struct rk_mipi_priv *priv = dev_get_priv(dev);

	/* Fill the mipi controller parameter */
	priv->ref_clk = 24 * MHz;
	priv->sys_clk = priv->ref_clk;
	priv->pix_clk = timing->pixelclock.typ;
	priv->phy_clk = priv->pix_clk * 6;
	priv->txbyte_clk = priv->phy_clk / 8;
	priv->txesc_clk = 20 * MHz;

	/* Select vop port, big or little */
	rk_mipi_dsi_source_select(dev);

	/* Set mipi dphy work mode */
	rk_mipi_dphy_mode_set(dev);

	/* Config  and enable mipi dsi according to timing */
	ret = rk_mipi_dsi_enable(dev, timing);
	if (ret) {
		debug("%s: rk_mipi_dsi_enable() failed (err=%d)\n",
		      __func__, ret);
		return ret;
	}

	/* Config and enable mipi phy */
	ret = rk_mipi_phy_enable(dev);
	if (ret) {
		debug("%s: rk_mipi_phy_enable() failed (err=%d)\n",
		      __func__, ret);
		return ret;
	}

	/* Enable backlight */
	ret = panel_enable_backlight(priv->panel);
	if (ret) {
		debug("%s: panel_enable_backlight() failed (err=%d)\n",
		      __func__, ret);
		return ret;
	}

	return 0;
}

static int rk_mipi_of_to_plat(struct udevice *dev)
{
	struct rk_mipi_priv *priv = dev_get_priv(dev);

	priv->grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	if (IS_ERR_OR_NULL(priv->grf)) {
		debug("%s: Get syscon grf failed (ret=%p)\n",
		      __func__, priv->grf);
		return  -ENXIO;
	}
	priv->regs = dev_read_addr(dev);
	if (priv->regs == FDT_ADDR_T_NONE) {
		debug("%s: Get MIPI dsi address failed\n", __func__);
		return  -ENXIO;
	}

	return 0;
}

/*
 * Probe function: check panel existence and readingit's timing. Then config
 * mipi dsi controller and enable it according to the timing parameter.
 */
static int rk_mipi_probe(struct udevice *dev)
{
	int ret;
	struct rk_mipi_priv *priv = dev_get_priv(dev);

	ret = uclass_get_device_by_phandle(UCLASS_PANEL, dev, "rockchip,panel",
					   &priv->panel);
	if (ret) {
		debug("%s: Can not find panel (err=%d)\n", __func__, ret);
		return ret;
	}

	return 0;
}

static const struct dm_display_ops rk_mipi_dsi_ops = {
	.read_timing = rk_mipi_read_timing,
	.enable = rk_display_enable,
};

static const struct udevice_id rk_mipi_dsi_ids[] = {
	{ .compatible = "rockchip,rk3399-mipi-dsi" },
	{ .compatible = "rockchip,rk3399_mipi_dsi" },
	{ }
};

U_BOOT_DRIVER(rk_mipi_dsi) = {
	.name	= "rk_mipi_dsi",
	.id	= UCLASS_DISPLAY,
	.of_match = rk_mipi_dsi_ids,
	.of_to_plat = rk_mipi_of_to_plat,
	.probe	= rk_mipi_probe,
	.ops	= &rk_mipi_dsi_ops,
	.priv_auto	  = sizeof(struct rk_mipi_priv),
};
