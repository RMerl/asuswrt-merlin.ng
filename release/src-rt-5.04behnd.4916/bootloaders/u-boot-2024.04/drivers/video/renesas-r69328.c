// SPDX-License-Identifier: GPL-2.0+
/*
 * Renesas R69328 panel driver
 *
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

/*
 * The datasheet is not publicly available, all values are
 * taken from the downstream. If you have access to datasheets,
 * corrections are welcome.
 */

#define R69328_MACP		0xB0 /* Manufacturer Command Access Protect */

#define R69328_GAMMA_SET_A	0xC8 /* Gamma Setting A */
#define R69328_GAMMA_SET_B	0xC9 /* Gamma Setting B */
#define R69328_GAMMA_SET_C	0xCA /* Gamma Setting C */

#define R69328_POWER_SET	0xD1

struct renesas_r69328_priv {
	struct udevice *backlight;

	struct gpio_desc enable_gpio;
	struct gpio_desc reset_gpio;
};

static const u8 address_mode[] = {
	MIPI_DCS_SET_ADDRESS_MODE
};

#define dsi_generic_write_seq(dsi, cmd, seq...) do {			\
		static const u8 b[] = { cmd, seq };			\
		int ret;						\
		ret = mipi_dsi_dcs_write_buffer(dsi, b, ARRAY_SIZE(b));	\
		if (ret < 0)						\
			return ret;					\
	} while (0)

static struct display_timing default_timing = {
	.pixelclock.typ		= 68000000,
	.hactive.typ		= 720,
	.hfront_porch.typ	= 92,
	.hback_porch.typ	= 62,
	.hsync_len.typ		= 4,
	.vactive.typ		= 1280,
	.vfront_porch.typ	= 6,
	.vback_porch.typ	= 3,
	.vsync_len.typ		= 1,
};

static int renesas_r69328_enable_backlight(struct udevice *dev)
{
	struct renesas_r69328_priv *priv = dev_get_priv(dev);
	int ret;

	ret = dm_gpio_set_value(&priv->enable_gpio, 1);
	if (ret) {
		log_err("error changing enable-gpios (%d)\n", ret);
		return ret;
	}
	mdelay(5);

	ret = dm_gpio_set_value(&priv->reset_gpio, 0);
	if (ret) {
		log_err("error changing reset-gpios (%d)\n", ret);
		return ret;
	}
	mdelay(5);

	ret = dm_gpio_set_value(&priv->reset_gpio, 1);
	if (ret) {
		log_err("error changing reset-gpios (%d)\n", ret);
		return ret;
	}

	mdelay(5);

	return 0;
}

static int renesas_r69328_set_backlight(struct udevice *dev, int percent)
{
	struct renesas_r69328_priv *priv = dev_get_priv(dev);
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_device *dsi = plat->device;
	int ret;

	mipi_dsi_dcs_write_buffer(dsi, address_mode,
				  sizeof(address_mode));

	ret = mipi_dsi_dcs_set_pixel_format(dsi, MIPI_DCS_PIXEL_FMT_24BIT << 4);
	if (ret < 0) {
		log_err("failed to set pixel format: %d\n", ret);
		return ret;
	}

	ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (ret < 0) {
		log_err("failed to exit sleep mode: %d\n", ret);
		return ret;
	}

	mdelay(100);

	/* MACP Off */
	dsi_generic_write_seq(dsi, R69328_MACP, 0x04);

	dsi_generic_write_seq(dsi, R69328_POWER_SET, 0x14,
			      0x1d, 0x21, 0x67, 0x11, 0x9a);

	dsi_generic_write_seq(dsi, R69328_GAMMA_SET_A, 0x00,
			      0x1a, 0x20, 0x28, 0x25, 0x24,
			      0x26, 0x15, 0x13, 0x11, 0x18,
			      0x1e, 0x1c, 0x00, 0x00, 0x1a,
			      0x20, 0x28, 0x25, 0x24, 0x26,
			      0x15, 0x13, 0x11, 0x18, 0x1e,
			      0x1c, 0x00);
	dsi_generic_write_seq(dsi, R69328_GAMMA_SET_B, 0x00,
			      0x1a, 0x20, 0x28, 0x25, 0x24,
			      0x26, 0x15, 0x13, 0x11, 0x18,
			      0x1e, 0x1c, 0x00, 0x00, 0x1a,
			      0x20, 0x28, 0x25, 0x24, 0x26,
			      0x15, 0x13, 0x11, 0x18, 0x1e,
			      0x1c, 0x00);
	dsi_generic_write_seq(dsi, R69328_GAMMA_SET_C, 0x00,
			      0x1a, 0x20, 0x28, 0x25, 0x24,
			      0x26, 0x15, 0x13, 0x11, 0x18,
			      0x1e, 0x1c, 0x00, 0x00, 0x1a,
			      0x20, 0x28, 0x25, 0x24, 0x26,
			      0x15, 0x13, 0x11, 0x18, 0x1e,
			      0x1c, 0x00);

	/* MACP On */
	dsi_generic_write_seq(dsi, R69328_MACP, 0x03);

	ret = mipi_dsi_dcs_set_display_on(dsi);
	if (ret < 0) {
		log_err("failed to set display on: %d\n", ret);
		return ret;
	}

	mdelay(50);

	ret = backlight_enable(priv->backlight);
	if (ret)
		return ret;

	ret = backlight_set_brightness(priv->backlight, percent);
	if (ret)
		return ret;

	return 0;
}

static int renesas_r69328_timings(struct udevice *dev,
				  struct display_timing *timing)
{
	memcpy(timing, &default_timing, sizeof(*timing));
	return 0;
}

static int renesas_r69328_of_to_plat(struct udevice *dev)
{
	struct renesas_r69328_priv *priv = dev_get_priv(dev);
	int ret;

	ret = uclass_get_device_by_phandle(UCLASS_PANEL_BACKLIGHT, dev,
					   "backlight", &priv->backlight);
	if (ret) {
		log_err("cannot get backlight: ret = %d\n", ret);
		return ret;
	}

	ret = gpio_request_by_name(dev, "enable-gpios", 0,
				   &priv->enable_gpio, GPIOD_IS_OUT);
	if (ret) {
		log_err("could not decode enable-gpios (%d)\n", ret);
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

static int renesas_r69328_probe(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);

	/* fill characteristics of DSI data link */
	plat->lanes = 4;
	plat->format = MIPI_DSI_FMT_RGB888;
	plat->mode_flags = MIPI_DSI_MODE_VIDEO;

	return 0;
}

static const struct panel_ops renesas_r69328_ops = {
	.enable_backlight	= renesas_r69328_enable_backlight,
	.set_backlight		= renesas_r69328_set_backlight,
	.get_display_timing	= renesas_r69328_timings,
};

static const struct udevice_id renesas_r69328_ids[] = {
	{ .compatible = "jdi,dx12d100vm0eaa" },
	{ }
};

U_BOOT_DRIVER(renesas_r69328) = {
	.name		= "renesas_r69328",
	.id		= UCLASS_PANEL,
	.of_match	= renesas_r69328_ids,
	.ops		= &renesas_r69328_ops,
	.of_to_plat	= renesas_r69328_of_to_plat,
	.probe		= renesas_r69328_probe,
	.plat_auto	= sizeof(struct mipi_dsi_panel_plat),
	.priv_auto	= sizeof(struct renesas_r69328_priv),
};
