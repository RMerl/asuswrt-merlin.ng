// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <backlight.h>
#include <dm.h>
#include <log.h>
#include <mipi_dsi.h>
#include <panel.h>
#include <asm/gpio.h>
#include <power/regulator.h>

struct simple_panel_priv {
	struct udevice *reg;
	struct udevice *backlight;
	struct gpio_desc enable;
};

/* List of supported DSI panels */
enum {
	PANEL_NON_DSI,
	PANASONIC_VVX10F004B00,
};

static const struct mipi_dsi_panel_plat panasonic_vvx10f004b00 = {
	.mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_SYNC_PULSE |
		      MIPI_DSI_CLOCK_NON_CONTINUOUS,
	.format = MIPI_DSI_FMT_RGB888,
	.lanes = 4,
};

static int simple_panel_enable_backlight(struct udevice *dev)
{
	struct simple_panel_priv *priv = dev_get_priv(dev);
	int ret;

	debug("%s: start, backlight = '%s'\n", __func__, priv->backlight->name);
	dm_gpio_set_value(&priv->enable, 1);
	ret = backlight_enable(priv->backlight);
	debug("%s: done, ret = %d\n", __func__, ret);
	if (ret)
		return ret;

	return 0;
}

static int simple_panel_set_backlight(struct udevice *dev, int percent)
{
	struct simple_panel_priv *priv = dev_get_priv(dev);
	int ret;

	debug("%s: start, backlight = '%s'\n", __func__, priv->backlight->name);
	dm_gpio_set_value(&priv->enable, 1);
	ret = backlight_set_brightness(priv->backlight, percent);
	debug("%s: done, ret = %d\n", __func__, ret);
	if (ret)
		return ret;

	return 0;
}

static int simple_panel_get_display_timing(struct udevice *dev,
					   struct display_timing *timings)
{
	const void *blob = gd->fdt_blob;

	return fdtdec_decode_display_timing(blob, dev_of_offset(dev),
					    0, timings);
}

static int simple_panel_of_to_plat(struct udevice *dev)
{
	struct simple_panel_priv *priv = dev_get_priv(dev);
	int ret;

	if (CONFIG_IS_ENABLED(DM_REGULATOR)) {
		ret = uclass_get_device_by_phandle(UCLASS_REGULATOR, dev,
						   "power-supply", &priv->reg);
		if (ret) {
			debug("%s: Warning: cannot get power supply: ret=%d\n",
			      __func__, ret);
			if (ret != -ENOENT)
				return ret;
		}
	}

	ret = uclass_get_device_by_phandle(UCLASS_PANEL_BACKLIGHT, dev,
						   "backlight", &priv->backlight);
	if (ret) {
		debug("%s: Cannot get backlight: ret=%d\n", __func__, ret);
		if (ret != -ENOENT)
			return log_ret(ret);
	}

	ret = gpio_request_by_name(dev, "enable-gpios", 0, &priv->enable,
				   GPIOD_IS_OUT);
	if (ret) {
		debug("%s: Warning: cannot get enable GPIO: ret=%d\n",
		      __func__, ret);
		if (ret != -ENOENT)
			return log_ret(ret);
	}

	return 0;
}

static int simple_panel_probe(struct udevice *dev)
{
	struct simple_panel_priv *priv = dev_get_priv(dev);
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	const u32 dsi_data = dev_get_driver_data(dev);
	int ret;

	ret = regulator_set_enable_if_allowed(priv->reg, true);
	if (ret && ret != -ENOSYS) {
		debug("%s: failed to enable regulator '%s' %d\n",
		      __func__, priv->reg->name, ret);
		return ret;
	}

	switch (dsi_data) {
	case PANASONIC_VVX10F004B00:
		memcpy(plat, &panasonic_vvx10f004b00,
		       sizeof(panasonic_vvx10f004b00));
		break;
	case PANEL_NON_DSI:
	default:
		break;
	}

	return 0;
}

static const struct panel_ops simple_panel_ops = {
	.enable_backlight	= simple_panel_enable_backlight,
	.set_backlight		= simple_panel_set_backlight,
	.get_display_timing	= simple_panel_get_display_timing,
};

static const struct udevice_id simple_panel_ids[] = {
	{ .compatible = "simple-panel" },
	{ .compatible = "auo,b133xtn01" },
	{ .compatible = "auo,b116xw03" },
	{ .compatible = "auo,b133htn01" },
	{ .compatible = "boe,nv140fhmn49" },
	{ .compatible = "lg,lb070wv8" },
	{ .compatible = "sharp,lq123p1jx31" },
	{ .compatible = "boe,nv101wxmn51" },
	{ .compatible = "panasonic,vvx10f004b00",
	  .data = PANASONIC_VVX10F004B00 },
	{ }
};

U_BOOT_DRIVER(simple_panel) = {
	.name		= "simple_panel",
	.id		= UCLASS_PANEL,
	.of_match	= simple_panel_ids,
	.ops		= &simple_panel_ops,
	.of_to_plat	= simple_panel_of_to_plat,
	.probe		= simple_panel_probe,
	.priv_auto	= sizeof(struct simple_panel_priv),
	.plat_auto	= sizeof(struct mipi_dsi_panel_plat),
};
