// SPDX-License-Identifier: GPL-2.0+
/*
 * Renesas R61307 panel driver
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

#define R61307_MACP		0xB0 /* Manufacturer CMD Protect */

#define R61307_INVERSION	0xC1
#define R61307_GAMMA_SET_A	0xC8 /* Gamma Setting A */
#define R61307_GAMMA_SET_B	0xC9 /* Gamma Setting B */
#define R61307_GAMMA_SET_C	0xCA /* Gamma Setting C */
#define R61307_CONTRAST_SET	0xCC

struct renesas_r61307_priv {
	struct udevice *vcc;
	struct udevice *iovcc;

	struct udevice *backlight;

	struct gpio_desc reset_gpio;

	bool dig_cont_adj;
	bool inversion;
	u32 gamma;
};

static const u8 macp_on[] = {
	R61307_MACP, 0x03
};

static const u8 macp_off[] = {
	R61307_MACP, 0x04
};

static const u8 address_mode[] = {
	MIPI_DCS_SET_ADDRESS_MODE
};

static const u8 contrast_setting[] = {
	R61307_CONTRAST_SET,
	0xdc, 0xb4, 0xff
};

static const u8 column_inversion[] = {
	R61307_INVERSION,
	0x00, 0x50, 0x03, 0x22,
	0x16, 0x06, 0x60, 0x11
};

static const u8 line_inversion[] = {
	R61307_INVERSION,
	0x00, 0x10, 0x03, 0x22,
	0x16, 0x06, 0x60, 0x01
};

static const u8 gamma_setting[][25] = {
	{},
	{
		R61307_GAMMA_SET_A,
		0x00, 0x06, 0x0a, 0x0f,
		0x14, 0x1f, 0x1f, 0x17,
		0x12, 0x0c, 0x09, 0x06,
		0x00, 0x06, 0x0a, 0x0f,
		0x14, 0x1f, 0x1f, 0x17,
		0x12, 0x0c, 0x09, 0x06
	},
	{
		R61307_GAMMA_SET_A,
		0x00, 0x05, 0x0b, 0x0f,
		0x11, 0x1d, 0x20, 0x18,
		0x18, 0x09, 0x07, 0x06,
		0x00, 0x05, 0x0b, 0x0f,
		0x11, 0x1d, 0x20, 0x18,
		0x18, 0x09, 0x07, 0x06
	},
	{
		R61307_GAMMA_SET_A,
		0x0b, 0x0d, 0x10, 0x14,
		0x13, 0x1d, 0x20, 0x18,
		0x12, 0x09, 0x07, 0x06,
		0x0a, 0x0c, 0x10, 0x14,
		0x13, 0x1d, 0x20, 0x18,
		0x12, 0x09, 0x07, 0x06
	},
};

static struct display_timing default_timing = {
	.pixelclock.typ		= 62000000,
	.hactive.typ		= 768,
	.hfront_porch.typ	= 116,
	.hback_porch.typ	= 81,
	.hsync_len.typ		= 5,
	.vactive.typ		= 1024,
	.vfront_porch.typ	= 24,
	.vback_porch.typ	= 8,
	.vsync_len.typ		= 2,
};

static int renesas_r61307_enable_backlight(struct udevice *dev)
{
	struct renesas_r61307_priv *priv = dev_get_priv(dev);
	int ret;

	ret = regulator_set_enable_if_allowed(priv->vcc, 1);
	if (ret) {
		log_err("enabling vcc-supply failed (%d)\n", ret);
		return ret;
	}
	mdelay(5);

	ret = regulator_set_enable_if_allowed(priv->iovcc, 1);
	if (ret) {
		log_err("enabling iovcc-supply failed (%d)\n", ret);
		return ret;
	}

	ret = dm_gpio_set_value(&priv->reset_gpio, 0);
	if (ret) {
		log_err("changing reset-gpio failed (%d)\n", ret);
		return ret;
	}
	mdelay(5);

	ret = dm_gpio_set_value(&priv->reset_gpio, 1);
	if (ret) {
		log_err("changing reset-gpio failed (%d)\n", ret);
		return ret;
	}

	mdelay(5);

	return 0;
}

static int renesas_r61307_set_backlight(struct udevice *dev, int percent)
{
	struct renesas_r61307_priv *priv = dev_get_priv(dev);
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_device *dsi = plat->device;
	int ret;

	ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (ret < 0) {
		log_err("failed to exit sleep mode: %d\n", ret);
		return ret;
	}

	mdelay(80);

	mipi_dsi_dcs_write_buffer(dsi, address_mode,
				  sizeof(address_mode));

	mdelay(20);

	ret = mipi_dsi_dcs_set_pixel_format(dsi, MIPI_DCS_PIXEL_FMT_24BIT << 4);
	if (ret < 0) {
		log_err("failed to set pixel format: %d\n", ret);
		return ret;
	}

	/* MACP Off */
	mipi_dsi_generic_write(dsi, macp_off, sizeof(macp_off));

	if (priv->dig_cont_adj)
		mipi_dsi_generic_write(dsi, contrast_setting,
				       sizeof(contrast_setting));

	if (priv->gamma)
		mipi_dsi_generic_write(dsi, gamma_setting[priv->gamma],
				       sizeof(gamma_setting[priv->gamma]));

	if (priv->inversion)
		mipi_dsi_generic_write(dsi, column_inversion,
				       sizeof(column_inversion));
	else
		mipi_dsi_generic_write(dsi, line_inversion,
				       sizeof(line_inversion));

	/* MACP On */
	mipi_dsi_generic_write(dsi, macp_on, sizeof(macp_on));

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

static int renesas_r61307_timings(struct udevice *dev,
				  struct display_timing *timing)
{
	memcpy(timing, &default_timing, sizeof(*timing));
	return 0;
}

static int renesas_r61307_of_to_plat(struct udevice *dev)
{
	struct renesas_r61307_priv *priv = dev_get_priv(dev);
	int ret;

	ret = uclass_get_device_by_phandle(UCLASS_PANEL_BACKLIGHT, dev,
					   "backlight", &priv->backlight);
	if (ret) {
		log_err("Cannot get backlight: ret = %d\n", ret);
		return ret;
	}

	ret = uclass_get_device_by_phandle(UCLASS_REGULATOR, dev,
					   "vcc-supply", &priv->vcc);
	if (ret) {
		log_err("Cannot get vcc-supply: ret = %d\n", ret);
		return ret;
	}

	ret = uclass_get_device_by_phandle(UCLASS_REGULATOR, dev,
					   "iovcc-supply", &priv->iovcc);
	if (ret) {
		log_err("Cannot get iovcc-supply: ret = %d\n", ret);
		return ret;
	}

	ret = gpio_request_by_name(dev, "reset-gpios", 0,
				   &priv->reset_gpio, GPIOD_IS_OUT);
	if (ret) {
		log_err("Could not decode reser-gpios (%d)\n", ret);
		return ret;
	}

	priv->dig_cont_adj = dev_read_bool(dev, "renesas,contrast");
	priv->inversion = dev_read_bool(dev, "renesas,inversion");
	priv->gamma = dev_read_u32_default(dev, "renesas,gamma", 0);

	return 0;
}

static int renesas_r61307_probe(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);

	/* fill characteristics of DSI data link */
	plat->lanes = 4;
	plat->format = MIPI_DSI_FMT_RGB888;
	plat->mode_flags = MIPI_DSI_MODE_VIDEO;

	return 0;
}

static const struct panel_ops renesas_r61307_ops = {
	.enable_backlight	= renesas_r61307_enable_backlight,
	.set_backlight		= renesas_r61307_set_backlight,
	.get_display_timing	= renesas_r61307_timings,
};

static const struct udevice_id renesas_r61307_ids[] = {
	{ .compatible = "koe,tx13d100vm0eaa" },
	{ .compatible = "hitachi,tx13d100vm0eaa" },
	{ }
};

U_BOOT_DRIVER(renesas_r61307) = {
	.name		= "renesas_r61307",
	.id		= UCLASS_PANEL,
	.of_match	= renesas_r61307_ids,
	.ops		= &renesas_r61307_ops,
	.of_to_plat	= renesas_r61307_of_to_plat,
	.probe		= renesas_r61307_probe,
	.plat_auto	= sizeof(struct mipi_dsi_panel_plat),
	.priv_auto	= sizeof(struct renesas_r61307_priv),
};
