// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Ondrej Jirman <megi@xff.cz>
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

struct hx8394_panel_priv {
	struct udevice *reg_vcc;
	struct udevice *reg_iovcc;
	struct gpio_desc reset;
	struct udevice *backlight;
};

static const struct display_timing default_timing = {
	.pixelclock.typ		= 74250000,
	.hactive.typ		= 720,
	.hfront_porch.typ	= 40,
	.hback_porch.typ	= 40,
	.hsync_len.typ		= 46,
	.vactive.typ		= 1440,
	.vfront_porch.typ	= 7,
	.vback_porch.typ	= 9,
	.vsync_len.typ		= 7,
	.flags			= DISPLAY_FLAGS_VSYNC_LOW | DISPLAY_FLAGS_HSYNC_LOW,
};

#define dsi_dcs_write_seq(device, seq...) do {					\
		static const u8 d[] = { seq };					\
		int ret;							\
		ret = mipi_dsi_dcs_write_buffer(device, d, ARRAY_SIZE(d));	\
		if (ret < 0)							\
			return ret;						\
	} while (0)

static int hx8394_init_sequence(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_device *device = plat->device;
	int ret;

	dsi_dcs_write_seq(device, 0xb9, 0xff, 0x83, 0x94);
	dsi_dcs_write_seq(device, 0xb1, 0x48, 0x11, 0x71, 0x09, 0x32, 0x24,
			  0x71, 0x31, 0x55, 0x30);
	dsi_dcs_write_seq(device, 0xba, 0x63, 0x03, 0x68, 0x6b, 0xb2, 0xc0);
	dsi_dcs_write_seq(device, 0xb2, 0x00, 0x80, 0x78, 0x0c, 0x07);
	dsi_dcs_write_seq(device, 0xb4, 0x12, 0x63, 0x12, 0x63, 0x12, 0x63,
			  0x01, 0x0c, 0x7c, 0x55, 0x00, 0x3f, 0x12, 0x6b, 0x12,
			  0x6b, 0x12, 0x6b, 0x01, 0x0c, 0x7c);
	dsi_dcs_write_seq(device, 0xd3, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x1c,
			  0x00, 0x00, 0x32, 0x10, 0x09, 0x00, 0x09, 0x32, 0x15,
			  0xad, 0x05, 0xad, 0x32, 0x00, 0x00, 0x00, 0x00, 0x37,
			  0x03, 0x0b, 0x0b, 0x37, 0x00, 0x00, 0x00, 0x0c, 0x40);
	dsi_dcs_write_seq(device, 0xd5, 0x19, 0x19, 0x18, 0x18, 0x1b, 0x1b,
			  0x1a, 0x1a, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
			  0x07, 0x20, 0x21, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
			  0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x24, 0x25, 0x18,
			  0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
			  0x18, 0x18);
	dsi_dcs_write_seq(device, 0xd6, 0x18, 0x18, 0x19, 0x19, 0x1b, 0x1b,
			  0x1a, 0x1a, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
			  0x00, 0x25, 0x24, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
			  0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x21, 0x20, 0x18,
			  0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
			  0x18, 0x18);
	dsi_dcs_write_seq(device, 0xe0, 0x00, 0x04, 0x0c, 0x12, 0x14, 0x18,
			  0x1a, 0x18, 0x31, 0x3f, 0x4d, 0x4c, 0x54, 0x65, 0x6b,
			  0x70, 0x7f, 0x82, 0x7e, 0x8a, 0x99, 0x4a, 0x48, 0x49,
			  0x4b, 0x4a, 0x4c, 0x4b, 0x7f, 0x00, 0x04, 0x0c, 0x11,
			  0x13, 0x17, 0x1a, 0x18, 0x31, 0x3f, 0x4d, 0x4c, 0x54,
			  0x65, 0x6b, 0x70, 0x7f, 0x82, 0x7e, 0x8a, 0x99, 0x4a,
			  0x48, 0x49, 0x4b, 0x4a, 0x4c, 0x4b, 0x7f);
	dsi_dcs_write_seq(device, 0xcc, 0x0b);
	dsi_dcs_write_seq(device, 0xc0, 0x1f, 0x31);
	dsi_dcs_write_seq(device, 0xb6, 0x7d, 0x7d);
	dsi_dcs_write_seq(device, 0xd4, 0x02);
	dsi_dcs_write_seq(device, 0xbd, 0x01);
	dsi_dcs_write_seq(device, 0xb1, 0x00);
	dsi_dcs_write_seq(device, 0xbd, 0x00);
	dsi_dcs_write_seq(device, 0xc6, 0xed);

	ret = mipi_dsi_dcs_exit_sleep_mode(device);
	if (ret)
		return ret;

	/* Panel is operational 120 msec after reset */
	mdelay(120);

	ret = mipi_dsi_dcs_set_display_on(device);
	if (ret)
		return ret;

	return 0;
}

static int hx8394_panel_enable_backlight(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_device *device = plat->device;
	struct hx8394_panel_priv *priv = dev_get_priv(dev);
	int ret;

	ret = mipi_dsi_attach(device);
	if (ret < 0) {
		printf("mipi_dsi_attach failed %d\n", ret);
		return ret;
	}

	ret = hx8394_init_sequence(dev);
	if (ret) {
		printf("hx8394_init_sequence failed %d\n", ret);
		return ret;
	}

	if (priv->backlight) {
		ret = backlight_enable(priv->backlight);
		if (ret) {
			printf("backlight enabled failed %d\n", ret);
			return ret;
		}

		backlight_set_brightness(priv->backlight, 60);
	}

	mdelay(10);

	return 0;
}

static int hx8394_panel_get_display_timing(struct udevice *dev,
					   struct display_timing *timings)
{
	memcpy(timings, &default_timing, sizeof(*timings));

	return 0;
}

static int hx8394_panel_of_to_plat(struct udevice *dev)
{
	struct hx8394_panel_priv *priv = dev_get_priv(dev);
	int ret;

	if (CONFIG_IS_ENABLED(DM_REGULATOR)) {
		ret =  device_get_supply_regulator(dev, "vcc-supply",
						   &priv->reg_vcc);
		if (ret && ret != -ENOENT) {
			dev_err(dev, "Warning: cannot get vcc supply\n");
			return ret;
		}

		ret =  device_get_supply_regulator(dev, "iovcc-supply",
						   &priv->reg_iovcc);
		if (ret && ret != -ENOENT) {
			dev_err(dev, "Warning: cannot get iovcc supply\n");
			return ret;
		}
	}

	ret = uclass_get_device_by_phandle(UCLASS_PANEL_BACKLIGHT, dev,
					   "backlight", &priv->backlight);
	if (ret)
		dev_warn(dev, "failed to get backlight\n");

	ret = gpio_request_by_name(dev, "reset-gpios", 0, &priv->reset,
				   GPIOD_IS_OUT);
	if (ret) {
		dev_err(dev, "warning: cannot get reset GPIO (%d)\n", ret);
		if (ret != -ENOENT)
			return ret;
	}

	return 0;
}

static int hx8394_panel_probe(struct udevice *dev)
{
	struct hx8394_panel_priv *priv = dev_get_priv(dev);
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	int ret;

	dm_gpio_set_value(&priv->reset, true);

	if (CONFIG_IS_ENABLED(DM_REGULATOR)) {
		dev_dbg(dev, "enable vcc '%s'\n", priv->reg_vcc->name);
		ret = regulator_set_enable(priv->reg_vcc, true);
		if (ret)
			return ret;

		dev_dbg(dev, "enable iovcc '%s'\n", priv->reg_iovcc->name);
		ret = regulator_set_enable(priv->reg_iovcc, true);
		if (ret) {
			regulator_set_enable(priv->reg_vcc, false);
			return ret;
		}
	}

	mdelay(5);
	dm_gpio_set_value(&priv->reset, false);

	mdelay(180);

	/* fill characteristics of DSI data link */
	plat->lanes = 4;
	plat->format = MIPI_DSI_FMT_RGB888;
	plat->mode_flags = MIPI_DSI_MODE_VIDEO |
			   MIPI_DSI_MODE_VIDEO_BURST;

	return 0;
}

static const struct panel_ops hx8394_panel_ops = {
	.enable_backlight = hx8394_panel_enable_backlight,
	.get_display_timing = hx8394_panel_get_display_timing,
};

static const struct udevice_id hx8394_panel_ids[] = {
	{ .compatible = "hannstar,hsd060bhw4" },
	{ }
};

U_BOOT_DRIVER(hx8394_panel) = {
	.name		= "hx8394_panel",
	.id		= UCLASS_PANEL,
	.of_match	= hx8394_panel_ids,
	.ops		= &hx8394_panel_ops,
	.of_to_plat	= hx8394_panel_of_to_plat,
	.probe		= hx8394_panel_probe,
	.plat_auto	= sizeof(struct mipi_dsi_panel_plat),
	.priv_auto	= sizeof(struct hx8394_panel_priv),
};
