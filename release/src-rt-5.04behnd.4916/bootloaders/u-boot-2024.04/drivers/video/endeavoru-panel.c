// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <common.h>
#include <backlight.h>
#include <dm.h>
#include <panel.h>
#include <log.h>
#include <misc.h>
#include <mipi_display.h>
#include <mipi_dsi.h>
#include <asm/gpio.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <power/regulator.h>

struct endeavoru_panel_priv {
	struct udevice *vdd;
	struct udevice *vddio;

	struct udevice *backlight;

	struct gpio_desc reset_gpio;
};

static struct display_timing default_timing = {
	.pixelclock.typ		= 63200000,
	.hactive.typ		= 720,
	.hfront_porch.typ	= 55,
	.hback_porch.typ	= 29,
	.hsync_len.typ		= 16,
	.vactive.typ		= 1280,
	.vfront_porch.typ	= 2,
	.vback_porch.typ	= 1,
	.vsync_len.typ		= 1,
};

static void dcs_write_one(struct mipi_dsi_device *dsi, u8 cmd, u8 data)
{
	mipi_dsi_dcs_write(dsi, cmd, &data, 1);
}

/*
 * This panel is not able to auto-increment all cmd addresses so for some of
 * them, we need to send them one by one...
 */
#define dcs_write_seq(dsi, cmd, seq...)			\
({							\
	static const u8 d[] = { seq };			\
	unsigned int i;					\
							\
	for (i = 0; i < ARRAY_SIZE(d) ; i++)		\
		dcs_write_one(dsi, cmd + i, d[i]);	\
})

static int endeavoru_panel_enable_backlight(struct udevice *dev)
{
	struct endeavoru_panel_priv *priv = dev_get_priv(dev);
	int ret;

	ret = dm_gpio_set_value(&priv->reset_gpio, 1);
	if (ret) {
		log_err("error changing reset-gpios (%d)\n", ret);
		return ret;
	}
	mdelay(5);

	ret = regulator_set_enable_if_allowed(priv->vddio, 1);
	if (ret) {
		log_err("error enabling iovcc-supply (%d)\n", ret);
		return ret;
	}
	mdelay(1);

	ret = regulator_set_enable_if_allowed(priv->vdd, 1);
	if (ret) {
		log_err("error enabling vcc-supply (%d)\n", ret);
		return ret;
	}
	mdelay(20);

	ret = dm_gpio_set_value(&priv->reset_gpio, 0);
	if (ret) {
		log_err("error changing reset-gpios (%d)\n", ret);
		return ret;
	}
	mdelay(2);

	/* Reset panel */
	ret = dm_gpio_set_value(&priv->reset_gpio, 1);
	if (ret) {
		log_err("error changing reset-gpios (%d)\n", ret);
		return ret;
	}
	mdelay(1);

	ret = dm_gpio_set_value(&priv->reset_gpio, 0);
	if (ret) {
		log_err("error changing reset-gpios (%d)\n", ret);
		return ret;
	}
	mdelay(25);

	return 0;
}

static int endeavoru_panel_set_backlight(struct udevice *dev, int percent)
{
	struct endeavoru_panel_priv *priv = dev_get_priv(dev);
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_device *dsi = plat->device;
	int ret;

	dcs_write_one(dsi, 0xc2, 0x08);

	/* color enhancement 2.2 */
	dcs_write_one(dsi, 0xff, 0x03);
	dcs_write_one(dsi, 0xfe, 0x08);
	dcs_write_one(dsi, 0x18, 0x00);
	dcs_write_one(dsi, 0x19, 0x00);
	dcs_write_one(dsi, 0x1a, 0x00);
	dcs_write_one(dsi, 0x25, 0x26);

	dcs_write_seq(dsi, 0x00, 0x00, 0x05, 0x10, 0x17,
		      0x22, 0x26, 0x29, 0x29, 0x26, 0x23,
		      0x17, 0x12, 0x06, 0x02, 0x01, 0x00);

	dcs_write_one(dsi, 0xfb, 0x01);
	dcs_write_one(dsi, 0xff, 0x00);
	dcs_write_one(dsi, 0xfe, 0x01);

	mipi_dsi_dcs_exit_sleep_mode(dsi);

	mdelay(105);

	dcs_write_one(dsi, 0x35, 0x00);

	/* PWM frequency adjust */
	dcs_write_one(dsi, 0xff, 0x04);
	dcs_write_one(dsi, 0x0a, 0x07);
	dcs_write_one(dsi, 0x09, 0x20);
	dcs_write_one(dsi, 0xff, 0x00);

	dcs_write_one(dsi, 0xff, 0xee);
	dcs_write_one(dsi, 0x12, 0x50);
	dcs_write_one(dsi, 0x13, 0x02);
	dcs_write_one(dsi, 0x6a, 0x60);
	dcs_write_one(dsi, 0xfb, 0x01);
	dcs_write_one(dsi, 0xff, 0x00);

	mipi_dsi_dcs_set_display_on(dsi);

	mdelay(42);

	dcs_write_one(dsi, 0xba, 0x01);

	dcs_write_one(dsi, 0x53, 0x24);
	dcs_write_one(dsi, 0x55, 0x80);
	dcs_write_one(dsi, 0x5e, 0x06);

	ret = backlight_enable(priv->backlight);
	if (ret)
		return ret;

	/* Set backlight */
	dcs_write_one(dsi, 0x51, 0x96);

	ret = backlight_set_brightness(priv->backlight, percent);
	if (ret)
		return ret;

	return 0;
}

static int endeavoru_panel_timings(struct udevice *dev,
				   struct display_timing *timing)
{
	memcpy(timing, &default_timing, sizeof(*timing));
	return 0;
}

static int endeavoru_panel_of_to_plat(struct udevice *dev)
{
	struct endeavoru_panel_priv *priv = dev_get_priv(dev);
	int ret;

	ret = uclass_get_device_by_phandle(UCLASS_PANEL_BACKLIGHT, dev,
					   "backlight", &priv->backlight);
	if (ret) {
		log_err("cannot get backlight: ret = %d\n", ret);
		return ret;
	}

	ret = uclass_get_device_by_phandle(UCLASS_REGULATOR, dev,
					   "vdd-supply", &priv->vdd);
	if (ret) {
		log_err("cannot get vdd-supply: ret = %d\n", ret);
		return ret;
	}

	ret = uclass_get_device_by_phandle(UCLASS_REGULATOR, dev,
					   "vddio-supply", &priv->vddio);
	if (ret) {
		log_err("cannot get vddio-supply: ret = %d\n", ret);
		return ret;
	}

	ret = gpio_request_by_name(dev, "reset-gpios", 0,
				   &priv->reset_gpio, GPIOD_IS_OUT);
	if (ret) {
		log_err("could not decode reser-gpios (%d)\n", ret);
		return ret;
	}

	return 0;
}

static int endeavoru_panel_probe(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);

	/* fill characteristics of DSI data link */
	plat->lanes = 2;
	plat->format = MIPI_DSI_FMT_RGB888;
	plat->mode_flags = MIPI_DSI_MODE_VIDEO;

	return 0;
}

static const struct panel_ops endeavoru_panel_ops = {
	.enable_backlight	= endeavoru_panel_enable_backlight,
	.set_backlight		= endeavoru_panel_set_backlight,
	.get_display_timing	= endeavoru_panel_timings,
};

static const struct udevice_id endeavoru_panel_ids[] = {
	{ .compatible = "htc,edge-panel" },
	{ }
};

U_BOOT_DRIVER(endeavoru_panel) = {
	.name		= "endeavoru_panel",
	.id		= UCLASS_PANEL,
	.of_match	= endeavoru_panel_ids,
	.ops		= &endeavoru_panel_ops,
	.of_to_plat	= endeavoru_panel_of_to_plat,
	.probe		= endeavoru_panel_probe,
	.plat_auto	= sizeof(struct mipi_dsi_panel_plat),
	.priv_auto	= sizeof(struct endeavoru_panel_priv),
};
