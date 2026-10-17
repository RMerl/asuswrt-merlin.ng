// SPDX-License-Identifier: GPL-2.0+
/*
 * OMAP panel support
 *
 * Copyright (C) 2020 Dario Binacchi <dariobin@libero.it>
 */

#include <common.h>
#include <backlight.h>
#include <clk.h>
#include <display.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <log.h>
#include <panel.h>
#include <asm/gpio.h>
#include <linux/err.h>
#include "tilcdc.h"

struct tilcdc_panel_priv {
	struct tilcdc_panel_info info;
	struct display_timing timing;
	struct udevice *backlight;
	struct gpio_desc enable;
};

static int tilcdc_panel_enable_backlight(struct udevice *dev)
{
	struct tilcdc_panel_priv *priv = dev_get_priv(dev);

	if (dm_gpio_is_valid(&priv->enable))
		dm_gpio_set_value(&priv->enable, 1);

	if (priv->backlight)
		return backlight_enable(priv->backlight);

	return 0;
}

static int tilcdc_panel_set_backlight(struct udevice *dev, int percent)
{
	struct tilcdc_panel_priv *priv = dev_get_priv(dev);

	if (dm_gpio_is_valid(&priv->enable))
		dm_gpio_set_value(&priv->enable, 1);

	if (priv->backlight)
		return backlight_set_brightness(priv->backlight, percent);

	return 0;
}

int tilcdc_panel_get_display_info(struct udevice *dev,
				  struct tilcdc_panel_info *info)
{
	struct tilcdc_panel_priv *priv = dev_get_priv(dev);

	memcpy(info, &priv->info, sizeof(*info));
	return 0;
}

static int tilcdc_panel_get_display_timing(struct udevice *dev,
					   struct display_timing *timing)
{
	struct tilcdc_panel_priv *priv = dev_get_priv(dev);

	memcpy(timing, &priv->timing, sizeof(*timing));
	return 0;
}

static int tilcdc_panel_remove(struct udevice *dev)
{
	struct tilcdc_panel_priv *priv = dev_get_priv(dev);

	if (dm_gpio_is_valid(&priv->enable))
		dm_gpio_free(dev, &priv->enable);

	return 0;
}

static int tilcdc_panel_probe(struct udevice *dev)
{
	struct tilcdc_panel_priv *priv = dev_get_priv(dev);
	int err;

	err = uclass_get_device_by_phandle(UCLASS_PANEL_BACKLIGHT, dev,
					   "backlight", &priv->backlight);
	if (err)
		dev_warn(dev, "failed to get backlight\n");

	err = gpio_request_by_name(dev, "enable-gpios", 0, &priv->enable,
				   GPIOD_IS_OUT);
	if (err) {
		dev_warn(dev, "failed to get enable GPIO\n");
		if (err != -ENOENT)
			return err;
	}

	return 0;
}

static int tilcdc_panel_of_to_plat(struct udevice *dev)
{
	struct tilcdc_panel_priv *priv = dev_get_priv(dev);
	ofnode node;
	int err;

	err = ofnode_decode_display_timing(dev_ofnode(dev), 0, &priv->timing);
	if (err) {
		dev_err(dev, "failed to get display timing\n");
		return err;
	}

	node = dev_read_subnode(dev, "panel-info");
	if (!ofnode_valid(node)) {
		dev_err(dev, "missing 'panel-info' node\n");
		return -ENXIO;
	}

	err |= ofnode_read_u32(node, "ac-bias", &priv->info.ac_bias);
	err |= ofnode_read_u32(node, "ac-bias-intrpt",
			       &priv->info.ac_bias_intrpt);
	err |= ofnode_read_u32(node, "dma-burst-sz", &priv->info.dma_burst_sz);
	err |= ofnode_read_u32(node, "bpp", &priv->info.bpp);
	err |= ofnode_read_u32(node, "fdd", &priv->info.fdd);
	err |= ofnode_read_u32(node, "sync-edge", &priv->info.sync_edge);
	err |= ofnode_read_u32(node, "sync-ctrl", &priv->info.sync_ctrl);
	err |= ofnode_read_u32(node, "raster-order", &priv->info.raster_order);
	err |= ofnode_read_u32(node, "fifo-th", &priv->info.fifo_th);
	if (err) {
		dev_err(dev, "failed to get panel info\n");
		return err;
	}

	/* optional */
	priv->info.tft_alt_mode = ofnode_read_bool(node, "tft-alt-mode");
	priv->info.invert_pxl_clk = ofnode_read_bool(node, "invert-pxl-clk");

	dev_dbg(dev, "LCD: %dx%d, bpp=%d, clk=%d Hz\n",
		priv->timing.hactive.typ, priv->timing.vactive.typ,
		priv->info.bpp, priv->timing.pixelclock.typ);
	dev_dbg(dev, "     hbp=%d, hfp=%d, hsw=%d\n",
		priv->timing.hback_porch.typ, priv->timing.hfront_porch.typ,
		priv->timing.hsync_len.typ);
	dev_dbg(dev, "     vbp=%d, vfp=%d, vsw=%d\n",
		priv->timing.vback_porch.typ, priv->timing.vfront_porch.typ,
		priv->timing.vsync_len.typ);

	return 0;
}

static const struct panel_ops tilcdc_panel_ops = {
	.enable_backlight = tilcdc_panel_enable_backlight,
	.set_backlight = tilcdc_panel_set_backlight,
	.get_display_timing = tilcdc_panel_get_display_timing,
};

static const struct udevice_id tilcdc_panel_ids[] = {
	{.compatible = "ti,tilcdc,panel"},
	{}
};

U_BOOT_DRIVER(tilcdc_panel) = {
	.name = "tilcdc_panel",
	.id = UCLASS_PANEL,
	.of_match = tilcdc_panel_ids,
	.ops = &tilcdc_panel_ops,
	.of_to_plat = tilcdc_panel_of_to_plat,
	.probe = tilcdc_panel_probe,
	.remove = tilcdc_panel_remove,
	.priv_auto = sizeof(struct tilcdc_panel_priv),
};
