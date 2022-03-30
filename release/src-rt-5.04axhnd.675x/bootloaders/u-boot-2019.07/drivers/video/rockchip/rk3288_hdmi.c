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
#include <asm/arch-rockchip/grf_rk3288.h>
#include <power/regulator.h>
#include "rk_hdmi.h"

static int rk3288_hdmi_enable(struct udevice *dev, int panel_bpp,
			      const struct display_timing *edid)
{
	struct rk_hdmi_priv *priv = dev_get_priv(dev);
	struct display_plat *uc_plat = dev_get_uclass_platdata(dev);
	int vop_id = uc_plat->source_id;
	struct rk3288_grf *grf = priv->grf;

	/* hdmi source select hdmi controller */
	rk_setreg(&grf->soc_con6, 1 << 15);

	/* hdmi data from vop id */
	rk_clrsetreg(&grf->soc_con6, 1 << 4, (vop_id == 1) ? (1 << 4) : 0);

	return 0;
}

static int rk3288_hdmi_ofdata_to_platdata(struct udevice *dev)
{
	struct rk_hdmi_priv *priv = dev_get_priv(dev);
	struct dw_hdmi *hdmi = &priv->hdmi;

	hdmi->i2c_clk_high = 0x7a;
	hdmi->i2c_clk_low = 0x8d;

	/*
	 * TODO(sjg@chromium.org): The above values don't work - these
	 * ones work better, but generate lots of errors in the data.
	 */
	hdmi->i2c_clk_high = 0x0d;
	hdmi->i2c_clk_low = 0x0d;

	return rk_hdmi_ofdata_to_platdata(dev);
}

static int rk3288_clk_config(struct udevice *dev)
{
	struct display_plat *uc_plat = dev_get_uclass_platdata(dev);
	struct clk clk;
	int ret;

	/*
	 * Configure the maximum clock to permit whatever resolution the
	 * monitor wants
	 */
	ret = clk_get_by_index(uc_plat->src_dev, 0, &clk);
	if (ret >= 0) {
		ret = clk_set_rate(&clk, 384000000);
		clk_free(&clk);
	}
	if (ret < 0) {
		debug("%s: Failed to set clock in source device '%s': ret=%d\n",
		      __func__, uc_plat->src_dev->name, ret);
		return ret;
	}

	return 0;
}

static const char * const rk3288_regulator_names[] = {
	"vcc50_hdmi"
};

static int rk3288_hdmi_probe(struct udevice *dev)
{
	/* Enable VOP clock for RK3288 */
	rk3288_clk_config(dev);

	/* Enable regulators required for HDMI */
	rk_hdmi_probe_regulators(dev, rk3288_regulator_names,
				 ARRAY_SIZE(rk3288_regulator_names));

	return rk_hdmi_probe(dev);
}

static const struct dm_display_ops rk3288_hdmi_ops = {
	.read_edid = rk_hdmi_read_edid,
	.enable = rk3288_hdmi_enable,
};

static const struct udevice_id rk3288_hdmi_ids[] = {
	{ .compatible = "rockchip,rk3288-dw-hdmi" },
	{ }
};

U_BOOT_DRIVER(rk3288_hdmi_rockchip) = {
	.name = "rk3288_hdmi_rockchip",
	.id = UCLASS_DISPLAY,
	.of_match = rk3288_hdmi_ids,
	.ops = &rk3288_hdmi_ops,
	.ofdata_to_platdata = rk3288_hdmi_ofdata_to_platdata,
	.probe = rk3288_hdmi_probe,
	.priv_auto_alloc_size = sizeof(struct rk_hdmi_priv),
};
