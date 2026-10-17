// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 STMicroelectronics - All Rights Reserved
 * Author(s): Yannick Fertre <yannick.fertre@st.com> for STMicroelectronics.
 *            Philippe Cornu <philippe.cornu@st.com> for STMicroelectronics.
 *
 * This otm8009a panel driver is inspired from the Linux Kernel driver
 * drivers/gpu/drm/panel/panel-orisetech-otm8009a.c.
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

#define OTM8009A_BACKLIGHT_DEFAULT	240
#define OTM8009A_BACKLIGHT_MAX		255

/* Manufacturer Command Set */
#define MCS_ADRSFT	0x0000	/* Address Shift Function */
#define MCS_PANSET	0xB3A6	/* Panel Type Setting */
#define MCS_SD_CTRL	0xC0A2	/* Source Driver Timing Setting */
#define MCS_P_DRV_M	0xC0B4	/* Panel Driving Mode */
#define MCS_OSC_ADJ	0xC181	/* Oscillator Adjustment for Idle/Normal mode */
#define MCS_RGB_VID_SET	0xC1A1	/* RGB Video Mode Setting */
#define MCS_SD_PCH_CTRL	0xC480	/* Source Driver Precharge Control */
#define MCS_NO_DOC1	0xC48A	/* Command not documented */
#define MCS_PWR_CTRL1	0xC580	/* Power Control Setting 1 */
#define MCS_PWR_CTRL2	0xC590	/* Power Control Setting 2 for Normal Mode */
#define MCS_PWR_CTRL4	0xC5B0	/* Power Control Setting 4 for DC Voltage */
#define MCS_PANCTRLSET1	0xCB80	/* Panel Control Setting 1 */
#define MCS_PANCTRLSET2	0xCB90	/* Panel Control Setting 2 */
#define MCS_PANCTRLSET3	0xCBA0	/* Panel Control Setting 3 */
#define MCS_PANCTRLSET4	0xCBB0	/* Panel Control Setting 4 */
#define MCS_PANCTRLSET5	0xCBC0	/* Panel Control Setting 5 */
#define MCS_PANCTRLSET6	0xCBD0	/* Panel Control Setting 6 */
#define MCS_PANCTRLSET7	0xCBE0	/* Panel Control Setting 7 */
#define MCS_PANCTRLSET8	0xCBF0	/* Panel Control Setting 8 */
#define MCS_PANU2D1	0xCC80	/* Panel U2D Setting 1 */
#define MCS_PANU2D2	0xCC90	/* Panel U2D Setting 2 */
#define MCS_PANU2D3	0xCCA0	/* Panel U2D Setting 3 */
#define MCS_PAND2U1	0xCCB0	/* Panel D2U Setting 1 */
#define MCS_PAND2U2	0xCCC0	/* Panel D2U Setting 2 */
#define MCS_PAND2U3	0xCCD0	/* Panel D2U Setting 3 */
#define MCS_GOAVST	0xCE80	/* GOA VST Setting */
#define MCS_GOACLKA1	0xCEA0	/* GOA CLKA1 Setting */
#define MCS_GOACLKA3	0xCEB0	/* GOA CLKA3 Setting */
#define MCS_GOAECLK	0xCFC0	/* GOA ECLK Setting */
#define MCS_NO_DOC2	0xCFD0	/* Command not documented */
#define MCS_GVDDSET	0xD800	/* GVDD/NGVDD */
#define MCS_VCOMDC	0xD900	/* VCOM Voltage Setting */
#define MCS_GMCT2_2P	0xE100	/* Gamma Correction 2.2+ Setting */
#define MCS_GMCT2_2N	0xE200	/* Gamma Correction 2.2- Setting */
#define MCS_NO_DOC3	0xF5B6	/* Command not documented */
#define MCS_CMD2_ENA1	0xFF00	/* Enable Access Command2 "CMD2" */
#define MCS_CMD2_ENA2	0xFF80	/* Enable Access Orise Command2 */

struct otm8009a_panel_priv {
	struct udevice *reg;
	struct gpio_desc reset;
};

static const struct display_timing default_timing = {
	.pixelclock.typ		= 29700000,
	.hactive.typ		= 480,
	.hfront_porch.typ	= 98,
	.hback_porch.typ	= 98,
	.hsync_len.typ		= 32,
	.vactive.typ		= 800,
	.vfront_porch.typ	= 15,
	.vback_porch.typ	= 14,
	.vsync_len.typ		= 10,
};

static void otm8009a_dcs_write_buf(struct udevice *dev, const void *data,
				   size_t len)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_device *device = plat->device;

	if (mipi_dsi_dcs_write_buffer(device, data, len) < 0)
		dev_err(dev, "mipi dsi dcs write buffer failed\n");
}

static void otm8009a_dcs_write_buf_hs(struct udevice *dev, const void *data,
				      size_t len)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_device *device = plat->device;

	/* data will be sent in dsi hs mode (ie. no lpm) */
	device->mode_flags &= ~MIPI_DSI_MODE_LPM;

	if (mipi_dsi_dcs_write_buffer(device, data, len) < 0)
		dev_err(dev, "mipi dsi dcs write buffer failed\n");

	/* restore back the dsi lpm mode */
	device->mode_flags |= MIPI_DSI_MODE_LPM;
}

#define dcs_write_seq(dev, seq...)				\
({								\
	static const u8 d[] = { seq };				\
	otm8009a_dcs_write_buf(dev, d, ARRAY_SIZE(d));		\
})

#define dcs_write_seq_hs(dev, seq...)				\
({								\
	static const u8 d[] = { seq };				\
	otm8009a_dcs_write_buf_hs(dev, d, ARRAY_SIZE(d));	\
})

#define dcs_write_cmd_at(dev, cmd, seq...)		\
({							\
	static const u16 c = cmd;			\
	struct udevice *device = dev;			\
	dcs_write_seq(device, MCS_ADRSFT, (c) & 0xFF);	\
	dcs_write_seq(device, (c) >> 8, seq);		\
})

static int otm8009a_init_sequence(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_device *device = plat->device;
	int ret;

	/* Enter CMD2 */
	dcs_write_cmd_at(dev, MCS_CMD2_ENA1, 0x80, 0x09, 0x01);

	/* Enter Orise Command2 */
	dcs_write_cmd_at(dev, MCS_CMD2_ENA2, 0x80, 0x09);

	dcs_write_cmd_at(dev, MCS_SD_PCH_CTRL, 0x30);
	mdelay(10);

	dcs_write_cmd_at(dev, MCS_NO_DOC1, 0x40);
	mdelay(10);

	dcs_write_cmd_at(dev, MCS_PWR_CTRL4 + 1, 0xA9);
	dcs_write_cmd_at(dev, MCS_PWR_CTRL2 + 1, 0x34);
	dcs_write_cmd_at(dev, MCS_P_DRV_M, 0x50);
	dcs_write_cmd_at(dev, MCS_VCOMDC, 0x4E);
	dcs_write_cmd_at(dev, MCS_OSC_ADJ, 0x66); /* 65Hz */
	dcs_write_cmd_at(dev, MCS_PWR_CTRL2 + 2, 0x01);
	dcs_write_cmd_at(dev, MCS_PWR_CTRL2 + 5, 0x34);
	dcs_write_cmd_at(dev, MCS_PWR_CTRL2 + 4, 0x33);
	dcs_write_cmd_at(dev, MCS_GVDDSET, 0x79, 0x79);
	dcs_write_cmd_at(dev, MCS_SD_CTRL + 1, 0x1B);
	dcs_write_cmd_at(dev, MCS_PWR_CTRL1 + 2, 0x83);
	dcs_write_cmd_at(dev, MCS_SD_PCH_CTRL + 1, 0x83);
	dcs_write_cmd_at(dev, MCS_RGB_VID_SET, 0x0E);
	dcs_write_cmd_at(dev, MCS_PANSET, 0x00, 0x01);

	dcs_write_cmd_at(dev, MCS_GOAVST, 0x85, 0x01, 0x00, 0x84, 0x01, 0x00);
	dcs_write_cmd_at(dev, MCS_GOACLKA1, 0x18, 0x04, 0x03, 0x39, 0x00, 0x00,
			 0x00, 0x18, 0x03, 0x03, 0x3A, 0x00, 0x00, 0x00);
	dcs_write_cmd_at(dev, MCS_GOACLKA3, 0x18, 0x02, 0x03, 0x3B, 0x00, 0x00,
			 0x00, 0x18, 0x01, 0x03, 0x3C, 0x00, 0x00, 0x00);
	dcs_write_cmd_at(dev, MCS_GOAECLK, 0x01, 0x01, 0x20, 0x20, 0x00, 0x00,
			 0x01, 0x02, 0x00, 0x00);

	dcs_write_cmd_at(dev, MCS_NO_DOC2, 0x00);

	dcs_write_cmd_at(dev, MCS_PANCTRLSET1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	dcs_write_cmd_at(dev, MCS_PANCTRLSET2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0);
	dcs_write_cmd_at(dev, MCS_PANCTRLSET3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			 0, 0, 0, 0, 0);
	dcs_write_cmd_at(dev, MCS_PANCTRLSET4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	dcs_write_cmd_at(dev, MCS_PANCTRLSET5, 0, 4, 4, 4, 4, 4, 0, 0, 0, 0,
			 0, 0, 0, 0, 0);
	dcs_write_cmd_at(dev, MCS_PANCTRLSET6, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4,
			 4, 0, 0, 0, 0);
	dcs_write_cmd_at(dev, MCS_PANCTRLSET7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	dcs_write_cmd_at(dev, MCS_PANCTRLSET8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
			 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

	dcs_write_cmd_at(dev, MCS_PANU2D1, 0x00, 0x26, 0x09, 0x0B, 0x01, 0x25,
			 0x00, 0x00, 0x00, 0x00);
	dcs_write_cmd_at(dev, MCS_PANU2D2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x0A, 0x0C, 0x02);
	dcs_write_cmd_at(dev, MCS_PANU2D3, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
	dcs_write_cmd_at(dev, MCS_PAND2U1, 0x00, 0x25, 0x0C, 0x0A, 0x02, 0x26,
			 0x00, 0x00, 0x00, 0x00);
	dcs_write_cmd_at(dev, MCS_PAND2U2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x25, 0x0B, 0x09, 0x01);
	dcs_write_cmd_at(dev, MCS_PAND2U3, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

	dcs_write_cmd_at(dev, MCS_PWR_CTRL1 + 1, 0x66);

	dcs_write_cmd_at(dev, MCS_NO_DOC3, 0x06);

	dcs_write_cmd_at(dev, MCS_GMCT2_2P, 0x00, 0x09, 0x0F, 0x0E, 0x07, 0x10,
			 0x0B, 0x0A, 0x04, 0x07, 0x0B, 0x08, 0x0F, 0x10, 0x0A,
			 0x01);
	dcs_write_cmd_at(dev, MCS_GMCT2_2N, 0x00, 0x09, 0x0F, 0x0E, 0x07, 0x10,
			 0x0B, 0x0A, 0x04, 0x07, 0x0B, 0x08, 0x0F, 0x10, 0x0A,
			 0x01);

	/* Exit CMD2 */
	dcs_write_cmd_at(dev, MCS_CMD2_ENA1, 0xFF, 0xFF, 0xFF);

	ret =  mipi_dsi_dcs_nop(device);
	if (ret)
		return ret;

	ret = mipi_dsi_dcs_exit_sleep_mode(device);
	if (ret)
		return ret;

	/* Wait for sleep out exit */
	mdelay(120);

	/* Default portrait 480x800 rgb24 */
	dcs_write_seq(dev, MIPI_DCS_SET_ADDRESS_MODE, 0x00);

	ret =  mipi_dsi_dcs_set_column_address(device, 0,
					       default_timing.hactive.typ - 1);
	if (ret)
		return ret;

	ret =  mipi_dsi_dcs_set_page_address(device, 0,
					     default_timing.vactive.typ - 1);
	if (ret)
		return ret;

	/* See otm8009a driver documentation for pixel format descriptions */
	ret =  mipi_dsi_dcs_set_pixel_format(device, MIPI_DCS_PIXEL_FMT_24BIT |
					     MIPI_DCS_PIXEL_FMT_24BIT << 4);
	if (ret)
		return ret;

	/* Disable CABC feature */
	dcs_write_seq(dev, MIPI_DCS_WRITE_POWER_SAVE, 0x00);

	ret = mipi_dsi_dcs_set_display_on(device);
	if (ret)
		return ret;

	ret = mipi_dsi_dcs_nop(device);
	if (ret)
		return ret;

	/* Send Command GRAM memory write (no parameters) */
	dcs_write_seq(dev, MIPI_DCS_WRITE_MEMORY_START);

	return 0;
}

static int otm8009a_panel_enable_backlight(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_device *device = plat->device;
	int ret;

	ret = mipi_dsi_attach(device);
	if (ret < 0)
		return ret;

	ret = otm8009a_init_sequence(dev);
	if (ret)
		return ret;

	/*
	 * Power on the backlight with the requested brightness
	 * Note We can not use mipi_dsi_dcs_set_display_brightness()
	 * as otm8009a driver support only 8-bit brightness (1 param).
	 */
	dcs_write_seq(dev, MIPI_DCS_SET_DISPLAY_BRIGHTNESS,
		      OTM8009A_BACKLIGHT_DEFAULT);

	/* Update Brightness Control & Backlight */
	dcs_write_seq(dev, MIPI_DCS_WRITE_CONTROL_DISPLAY, 0x24);

	/* Update Brightness Control & Backlight */
	dcs_write_seq_hs(dev, MIPI_DCS_WRITE_CONTROL_DISPLAY);

	/* Need to wait a few time before sending the first image */
	mdelay(10);

	return 0;
}

static int otm8009a_panel_get_display_timing(struct udevice *dev,
					     struct display_timing *timings)
{
	memcpy(timings, &default_timing, sizeof(*timings));

	return 0;
}

static int otm8009a_panel_of_to_plat(struct udevice *dev)
{
	struct otm8009a_panel_priv *priv = dev_get_priv(dev);
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
		dev_err(dev, "warning: cannot get reset GPIO\n");
		if (ret != -ENOENT)
			return ret;
	}

	return 0;
}

static int otm8009a_panel_probe(struct udevice *dev)
{
	struct otm8009a_panel_priv *priv = dev_get_priv(dev);
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	int ret;

	if (CONFIG_IS_ENABLED(DM_REGULATOR) && priv->reg) {
		dev_dbg(dev, "enable regulator '%s'\n", priv->reg->name);
		ret = regulator_set_enable(priv->reg, true);
		if (ret)
			return ret;
	}

	/* reset panel */
	dm_gpio_set_value(&priv->reset, true);
	mdelay(1); /* >50us */
	dm_gpio_set_value(&priv->reset, false);
	mdelay(10); /* >5ms */

	/* fill characteristics of DSI data link */
	plat->lanes = 2;
	plat->format = MIPI_DSI_FMT_RGB888;
	plat->mode_flags = MIPI_DSI_MODE_VIDEO |
			   MIPI_DSI_MODE_VIDEO_BURST |
			   MIPI_DSI_MODE_LPM;

	return 0;
}

static const struct panel_ops otm8009a_panel_ops = {
	.enable_backlight = otm8009a_panel_enable_backlight,
	.get_display_timing = otm8009a_panel_get_display_timing,
};

static const struct udevice_id otm8009a_panel_ids[] = {
	{ .compatible = "orisetech,otm8009a" },
	{ }
};

U_BOOT_DRIVER(otm8009a_panel) = {
	.name			  = "otm8009a_panel",
	.id			  = UCLASS_PANEL,
	.of_match		  = otm8009a_panel_ids,
	.ops			  = &otm8009a_panel_ops,
	.of_to_plat	  = otm8009a_panel_of_to_plat,
	.probe			  = otm8009a_panel_probe,
	.plat_auto	= sizeof(struct mipi_dsi_panel_plat),
	.priv_auto	= sizeof(struct otm8009a_panel_priv),
};
