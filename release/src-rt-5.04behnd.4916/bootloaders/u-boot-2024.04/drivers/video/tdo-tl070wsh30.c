// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */
#include <common.h>
#include <backlight.h>
#include <dm.h>
#include <mipi_dsi.h>
#include <panel.h>
#include <asm/gpio.h>
#include <dm/device_compat.h>
#include <linux/delay.h>
#include <power/regulator.h>

struct tl070wsh30_panel_priv {
	struct udevice *reg;
	struct udevice *backlight;
	struct gpio_desc reset;
};

static const struct display_timing default_timing = {
	.pixelclock.typ		= 47250000,
	.hactive.typ		= 1024,
	.hfront_porch.typ	= 46,
	.hback_porch.typ	= 100,
	.hsync_len.typ		= 80,
	.vactive.typ		= 600,
	.vfront_porch.typ	= 5,
	.vback_porch.typ	= 20,
	.vsync_len.typ		= 5,
	.flags			= DISPLAY_FLAGS_HSYNC_HIGH | DISPLAY_FLAGS_VSYNC_HIGH,
};

static int tl070wsh30_panel_enable_backlight(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_device *device = plat->device;
	struct tl070wsh30_panel_priv *priv = dev_get_priv(dev);
	int ret;

	ret = mipi_dsi_attach(device);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_exit_sleep_mode(device);
	if (ret)
		return ret;

	mdelay(200);

	ret = mipi_dsi_dcs_set_display_on(device);
	if (ret)
		return ret;

	mdelay(20);

	ret = backlight_enable(priv->backlight);
	if (ret)
		return ret;

	return 0;
}

static int tl070wsh30_panel_get_display_timing(struct udevice *dev,
					    struct display_timing *timings)
{
	memcpy(timings, &default_timing, sizeof(*timings));

	return 0;
}

static int tl070wsh30_panel_of_to_plat(struct udevice *dev)
{
	struct tl070wsh30_panel_priv *priv = dev_get_priv(dev);
	int ret;

	if (CONFIG_IS_ENABLED(DM_REGULATOR)) {
		ret =  device_get_supply_regulator(dev, "power-supply",
						   &priv->reg);
		if (ret && ret != -ENOENT) {
			dev_err(dev, "Warning: cannot get power supply\n");
			return ret;
		}
	}

	ret = gpio_request_by_name(dev, "reset-gpios", 0, &priv->reset,
				   GPIOD_IS_OUT);
	if (ret) {
		dev_err(dev, "Warning: cannot get reset GPIO\n");
		if (ret != -ENOENT)
			return ret;
	}

	ret = uclass_get_device_by_phandle(UCLASS_PANEL_BACKLIGHT, dev,
					   "backlight", &priv->backlight);
	if (ret) {
		dev_err(dev, "Cannot get backlight: ret=%d\n", ret);
		return ret;
	}

	return 0;
}

static int tl070wsh30_panel_probe(struct udevice *dev)
{
	struct tl070wsh30_panel_priv *priv = dev_get_priv(dev);
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	int ret;

	if (CONFIG_IS_ENABLED(DM_REGULATOR) && priv->reg) {
		ret = regulator_set_enable(priv->reg, true);
		if (ret)
			return ret;
	}

	mdelay(10);

	/* reset panel */
	dm_gpio_set_value(&priv->reset, true);

	mdelay(10);

	dm_gpio_set_value(&priv->reset, false);

	/* fill characteristics of DSI data link */
	plat->lanes = 4;
	plat->format = MIPI_DSI_FMT_RGB888;
	plat->mode_flags = MIPI_DSI_MODE_VIDEO |
			   MIPI_DSI_MODE_VIDEO_BURST |
			   MIPI_DSI_MODE_LPM;

	return 0;
}

static const struct panel_ops tl070wsh30_panel_ops = {
	.enable_backlight = tl070wsh30_panel_enable_backlight,
	.get_display_timing = tl070wsh30_panel_get_display_timing,
};

static const struct udevice_id tl070wsh30_panel_ids[] = {
	{ .compatible = "tdo,tl070wsh30" },
	{ }
};

U_BOOT_DRIVER(tl070wsh30_panel) = {
	.name			  = "tl070wsh30_panel",
	.id			  = UCLASS_PANEL,
	.of_match		  = tl070wsh30_panel_ids,
	.ops			  = &tl070wsh30_panel_ops,
	.of_to_plat		  = tl070wsh30_panel_of_to_plat,
	.probe			  = tl070wsh30_panel_probe,
	.plat_auto		  = sizeof(struct mipi_dsi_panel_plat),
	.priv_auto		  = sizeof(struct tl070wsh30_panel_priv),
};
