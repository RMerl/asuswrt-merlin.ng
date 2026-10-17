// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Theobroma Systems Design und Consulting GmbH
 */

#include <common.h>
#include <clk.h>
#include <display.h>
#include <dm.h>
#include <dw_hdmi.h>
#include <edid.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/arch-rockchip/grf_rk3399.h>
#include <power/regulator.h>
#include "rk_hdmi.h"

static int rk3399_hdmi_enable(struct udevice *dev, int panel_bpp,
			      const struct display_timing *edid)
{
	struct rk_hdmi_priv *priv = dev_get_priv(dev);
	struct display_plat *uc_plat = dev_get_uclass_plat(dev);
	int vop_id = uc_plat->source_id;
	struct rk3399_grf_regs *grf = priv->grf;

	/* select the hdmi encoder input data from our source_id */
	rk_clrsetreg(&grf->soc_con20, GRF_RK3399_HDMI_VOP_SEL_MASK,
		     (vop_id == 1) ? GRF_RK3399_HDMI_VOP_SEL_L : 0);

	return dw_hdmi_enable(&priv->hdmi, edid);
}

static int rk3399_hdmi_of_to_plat(struct udevice *dev)
{
	struct rk_hdmi_priv *priv = dev_get_priv(dev);
	struct dw_hdmi *hdmi = &priv->hdmi;

	hdmi->i2c_clk_high = 0x7a;
	hdmi->i2c_clk_low = 0x8d;

	return rk_hdmi_of_to_plat(dev);
}

static const char * const rk3399_regulator_names[] = {
	"vcc1v8_hdmi",
	"vcc0v9_hdmi"
};

static int rk3399_hdmi_probe(struct udevice *dev)
{
	/* Enable regulators required for HDMI */
	rk_hdmi_probe_regulators(dev, rk3399_regulator_names,
				 ARRAY_SIZE(rk3399_regulator_names));

	return rk_hdmi_probe(dev);
}

static const struct dm_display_ops rk3399_hdmi_ops = {
	.read_edid = rk_hdmi_read_edid,
	.enable = rk3399_hdmi_enable,
};

static const struct udevice_id rk3399_hdmi_ids[] = {
	{ .compatible = "rockchip,rk3399-dw-hdmi" },
	{ }
};

U_BOOT_DRIVER(rk3399_hdmi_rockchip) = {
	.name = "rk3399_hdmi_rockchip",
	.id = UCLASS_DISPLAY,
	.of_match = rk3399_hdmi_ids,
	.ops = &rk3399_hdmi_ops,
	.of_to_plat = rk3399_hdmi_of_to_plat,
	.probe = rk3399_hdmi_probe,
	.priv_auto	= sizeof(struct rk_hdmi_priv),
};
