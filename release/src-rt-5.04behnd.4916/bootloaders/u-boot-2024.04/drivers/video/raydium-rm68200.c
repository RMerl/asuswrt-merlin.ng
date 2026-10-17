// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 STMicroelectronics - All Rights Reserved
 * Author(s): Yannick Fertre <yannick.fertre@st.com> for STMicroelectronics.
 *            Philippe Cornu <philippe.cornu@st.com> for STMicroelectronics.
 *
 * This rm68200 panel driver is inspired from the Linux Kernel driver
 * drivers/gpu/drm/panel/panel-raydium-rm68200.c.
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

/*** Manufacturer Command Set ***/
#define MCS_CMD_MODE_SW	0xFE /* CMD Mode Switch */
#define MCS_CMD1_UCS	0x00 /* User Command Set (UCS = CMD1) */
#define MCS_CMD2_P0	0x01 /* Manufacture Command Set Page0 (CMD2 P0) */
#define MCS_CMD2_P1	0x02 /* Manufacture Command Set Page1 (CMD2 P1) */
#define MCS_CMD2_P2	0x03 /* Manufacture Command Set Page2 (CMD2 P2) */
#define MCS_CMD2_P3	0x04 /* Manufacture Command Set Page3 (CMD2 P3) */

/* CMD2 P0 commands (Display Options and Power) */
#define MCS_STBCTR	0x12 /* TE1 Output Setting Zig-Zag Connection */
#define MCS_SGOPCTR	0x16 /* Source Bias Current */
#define MCS_SDCTR	0x1A /* Source Output Delay Time */
#define MCS_INVCTR	0x1B /* Inversion Type */
#define MCS_EXT_PWR_IC	0x24 /* External PWR IC Control */
#define MCS_SETAVDD	0x27 /* PFM Control for AVDD Output */
#define MCS_SETAVEE	0x29 /* PFM Control for AVEE Output */
#define MCS_BT2CTR	0x2B /* DDVDL Charge Pump Control */
#define MCS_BT3CTR	0x2F /* VGH Charge Pump Control */
#define MCS_BT4CTR	0x34 /* VGL Charge Pump Control */
#define MCS_VCMCTR	0x46 /* VCOM Output Level Control */
#define MCS_SETVGN	0x52 /* VG M/S N Control */
#define MCS_SETVGP	0x54 /* VG M/S P Control */
#define MCS_SW_CTRL	0x5F /* Interface Control for PFM and MIPI */

/* CMD2 P2 commands (GOA Timing Control) - no description in datasheet */
#define GOA_VSTV1		0x00
#define GOA_VSTV2		0x07
#define GOA_VCLK1		0x0E
#define GOA_VCLK2		0x17
#define GOA_VCLK_OPT1		0x20
#define GOA_BICLK1		0x2A
#define GOA_BICLK2		0x37
#define GOA_BICLK3		0x44
#define GOA_BICLK4		0x4F
#define GOA_BICLK_OPT1		0x5B
#define GOA_BICLK_OPT2		0x60
#define MCS_GOA_GPO1		0x6D
#define MCS_GOA_GPO2		0x71
#define MCS_GOA_EQ		0x74
#define MCS_GOA_CLK_GALLON	0x7C
#define MCS_GOA_FS_SEL0		0x7E
#define MCS_GOA_FS_SEL1		0x87
#define MCS_GOA_FS_SEL2		0x91
#define MCS_GOA_FS_SEL3		0x9B
#define MCS_GOA_BS_SEL0		0xAC
#define MCS_GOA_BS_SEL1		0xB5
#define MCS_GOA_BS_SEL2		0xBF
#define MCS_GOA_BS_SEL3		0xC9
#define MCS_GOA_BS_SEL4		0xD3

/* CMD2 P3 commands (Gamma) */
#define MCS_GAMMA_VP		0x60 /* Gamma VP1~VP16 */
#define MCS_GAMMA_VN		0x70 /* Gamma VN1~VN16 */

struct rm68200_panel_priv {
	struct udevice *reg;
	struct udevice *backlight;
	struct gpio_desc reset;
};

static const struct display_timing default_timing = {
	.pixelclock.typ		= 54000000,
	.hactive.typ		= 720,
	.hfront_porch.typ	= 48,
	.hback_porch.typ	= 48,
	.hsync_len.typ		= 9,
	.vactive.typ		= 1280,
	.vfront_porch.typ	= 12,
	.vback_porch.typ	= 12,
	.vsync_len.typ		= 5,
};

static void rm68200_dcs_write_buf(struct udevice *dev, const void *data,
				  size_t len)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_device *device = plat->device;
	int err;

	err = mipi_dsi_dcs_write_buffer(device, data, len);
	if (err < 0)
		dev_err(dev, "MIPI DSI DCS write buffer failed: %d\n", err);
}

static void rm68200_dcs_write_cmd(struct udevice *dev, u8 cmd, u8 value)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_device *device = plat->device;
	int err;

	err = mipi_dsi_dcs_write(device, cmd, &value, 1);
	if (err < 0)
		dev_err(dev, "MIPI DSI DCS write failed: %d\n", err);
}

#define dcs_write_seq(ctx, seq...)				\
({								\
	static const u8 d[] = { seq };				\
								\
	rm68200_dcs_write_buf(ctx, d, ARRAY_SIZE(d));		\
})

/*
 * This panel is not able to auto-increment all cmd addresses so for some of
 * them, we need to send them one by one...
 */
#define dcs_write_cmd_seq(ctx, cmd, seq...)			\
({								\
	static const u8 d[] = { seq };				\
	unsigned int i;						\
								\
	for (i = 0; i < ARRAY_SIZE(d) ; i++)			\
		rm68200_dcs_write_cmd(ctx, cmd + i, d[i]);	\
})

static void rm68200_init_sequence(struct udevice *dev)
{
	/* Enter CMD2 with page 0 */
	dcs_write_seq(dev, MCS_CMD_MODE_SW, MCS_CMD2_P0);
	dcs_write_cmd_seq(dev, MCS_EXT_PWR_IC, 0xC0, 0x53, 0x00);
	dcs_write_seq(dev, MCS_BT2CTR, 0xE5);
	dcs_write_seq(dev, MCS_SETAVDD, 0x0A);
	dcs_write_seq(dev, MCS_SETAVEE, 0x0A);
	dcs_write_seq(dev, MCS_SGOPCTR, 0x52);
	dcs_write_seq(dev, MCS_BT3CTR, 0x53);
	dcs_write_seq(dev, MCS_BT4CTR, 0x5A);
	dcs_write_seq(dev, MCS_INVCTR, 0x00);
	dcs_write_seq(dev, MCS_STBCTR, 0x0A);
	dcs_write_seq(dev, MCS_SDCTR, 0x06);
	dcs_write_seq(dev, MCS_VCMCTR, 0x56);
	dcs_write_seq(dev, MCS_SETVGN, 0xA0, 0x00);
	dcs_write_seq(dev, MCS_SETVGP, 0xA0, 0x00);
	dcs_write_seq(dev, MCS_SW_CTRL, 0x11); /* 2 data lanes, see doc */

	dcs_write_seq(dev, MCS_CMD_MODE_SW, MCS_CMD2_P2);
	dcs_write_seq(dev, GOA_VSTV1, 0x05);
	dcs_write_seq(dev, 0x02, 0x0B);
	dcs_write_seq(dev, 0x03, 0x0F);
	dcs_write_seq(dev, 0x04, 0x7D, 0x00, 0x50);
	dcs_write_cmd_seq(dev, GOA_VSTV2, 0x05, 0x16, 0x0D, 0x11, 0x7D, 0x00,
			  0x50);
	dcs_write_cmd_seq(dev, GOA_VCLK1, 0x07, 0x08, 0x01, 0x02, 0x00, 0x7D,
			  0x00, 0x85, 0x08);
	dcs_write_cmd_seq(dev, GOA_VCLK2, 0x03, 0x04, 0x05, 0x06, 0x00, 0x7D,
			  0x00, 0x85, 0x08);
	dcs_write_seq(dev, GOA_VCLK_OPT1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		      0x00, 0x00, 0x00, 0x00);
	dcs_write_cmd_seq(dev, GOA_BICLK1, 0x07, 0x08);
	dcs_write_seq(dev, 0x2D, 0x01);
	dcs_write_seq(dev, 0x2F, 0x02, 0x00, 0x40, 0x05, 0x08, 0x54, 0x7D,
		      0x00);
	dcs_write_cmd_seq(dev, GOA_BICLK2, 0x03, 0x04, 0x05, 0x06, 0x00);
	dcs_write_seq(dev, 0x3D, 0x40);
	dcs_write_seq(dev, 0x3F, 0x05, 0x08, 0x54, 0x7D, 0x00);
	dcs_write_seq(dev, GOA_BICLK3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		      0x00, 0x00, 0x00, 0x00, 0x00);
	dcs_write_seq(dev, GOA_BICLK4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		      0x00, 0x00);
	dcs_write_seq(dev, 0x58, 0x00, 0x00, 0x00);
	dcs_write_seq(dev, GOA_BICLK_OPT1, 0x00, 0x00, 0x00, 0x00, 0x00);
	dcs_write_seq(dev, GOA_BICLK_OPT2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
	dcs_write_seq(dev, MCS_GOA_GPO1, 0x00, 0x00, 0x00, 0x00);
	dcs_write_seq(dev, MCS_GOA_GPO2, 0x00, 0x20, 0x00);
	dcs_write_seq(dev, MCS_GOA_EQ, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
		      0x00, 0x00);
	dcs_write_seq(dev, MCS_GOA_CLK_GALLON, 0x00, 0x00);
	dcs_write_cmd_seq(dev, MCS_GOA_FS_SEL0, 0xBF, 0x02, 0x06, 0x14, 0x10,
			  0x16, 0x12, 0x08, 0x3F);
	dcs_write_cmd_seq(dev, MCS_GOA_FS_SEL1, 0x3F, 0x3F, 0x3F, 0x3F, 0x0C,
			  0x0A, 0x0E, 0x3F, 0x3F, 0x00);
	dcs_write_cmd_seq(dev, MCS_GOA_FS_SEL2, 0x04, 0x3F, 0x3F, 0x3F, 0x3F,
			  0x05, 0x01, 0x3F, 0x3F, 0x0F);
	dcs_write_cmd_seq(dev, MCS_GOA_FS_SEL3, 0x0B, 0x0D, 0x3F, 0x3F, 0x3F,
			  0x3F);
	dcs_write_cmd_seq(dev, 0xA2, 0x3F, 0x09, 0x13, 0x17, 0x11, 0x15);
	dcs_write_cmd_seq(dev, 0xA9, 0x07, 0x03, 0x3F);
	dcs_write_cmd_seq(dev, MCS_GOA_BS_SEL0, 0x3F, 0x05, 0x01, 0x17, 0x13,
			  0x15, 0x11, 0x0F, 0x3F);
	dcs_write_cmd_seq(dev, MCS_GOA_BS_SEL1, 0x3F, 0x3F, 0x3F, 0x3F, 0x0B,
			  0x0D, 0x09, 0x3F, 0x3F, 0x07);
	dcs_write_cmd_seq(dev, MCS_GOA_BS_SEL2, 0x03, 0x3F, 0x3F, 0x3F, 0x3F,
			  0x02, 0x06, 0x3F, 0x3F, 0x08);
	dcs_write_cmd_seq(dev, MCS_GOA_BS_SEL3, 0x0C, 0x0A, 0x3F, 0x3F, 0x3F,
			  0x3F, 0x3F, 0x0E, 0x10, 0x14);
	dcs_write_cmd_seq(dev, MCS_GOA_BS_SEL4, 0x12, 0x16, 0x00, 0x04, 0x3F);
	dcs_write_seq(dev, 0xDC, 0x02);
	dcs_write_seq(dev, 0xDE, 0x12);

	dcs_write_seq(dev, MCS_CMD_MODE_SW, 0x0E); /* No documentation */
	dcs_write_seq(dev, 0x01, 0x75);

	dcs_write_seq(dev, MCS_CMD_MODE_SW, MCS_CMD2_P3);
	dcs_write_cmd_seq(dev, MCS_GAMMA_VP, 0x00, 0x0C, 0x12, 0x0E, 0x06,
			  0x12, 0x0E, 0x0B, 0x15, 0x0B, 0x10, 0x07, 0x0F,
			  0x12, 0x0C, 0x00);
	dcs_write_cmd_seq(dev, MCS_GAMMA_VN, 0x00, 0x0C, 0x12, 0x0E, 0x06,
			  0x12, 0x0E, 0x0B, 0x15, 0x0B, 0x10, 0x07, 0x0F,
			  0x12, 0x0C, 0x00);

	/* Exit CMD2 */
	dcs_write_seq(dev, MCS_CMD_MODE_SW, MCS_CMD1_UCS);
}

static int rm68200_panel_enable_backlight(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_device *device = plat->device;
	struct rm68200_panel_priv *priv = dev_get_priv(dev);
	int ret;

	ret = mipi_dsi_attach(device);
	if (ret < 0)
		return ret;

	rm68200_init_sequence(dev);

	ret = mipi_dsi_dcs_exit_sleep_mode(device);
	if (ret)
		return ret;

	mdelay(125);

	ret = mipi_dsi_dcs_set_display_on(device);
	if (ret)
		return ret;

	mdelay(20);

	ret = backlight_enable(priv->backlight);
	if (ret)
		return ret;

	return 0;
}

static int rm68200_panel_get_display_timing(struct udevice *dev,
					    struct display_timing *timings)
{
	memcpy(timings, &default_timing, sizeof(*timings));

	return 0;
}

static int rm68200_panel_of_to_plat(struct udevice *dev)
{
	struct rm68200_panel_priv *priv = dev_get_priv(dev);
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

static int rm68200_panel_probe(struct udevice *dev)
{
	struct rm68200_panel_priv *priv = dev_get_priv(dev);
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	int ret;

	if (CONFIG_IS_ENABLED(DM_REGULATOR) && priv->reg) {
		ret = regulator_set_enable(priv->reg, true);
		if (ret)
			return ret;
	}

	/* reset panel */
	dm_gpio_set_value(&priv->reset, true);
	mdelay(1);
	dm_gpio_set_value(&priv->reset, false);
	mdelay(10);

	/* fill characteristics of DSI data link */
	plat->lanes = 2;
	plat->format = MIPI_DSI_FMT_RGB888;
	plat->mode_flags = MIPI_DSI_MODE_VIDEO |
			   MIPI_DSI_MODE_VIDEO_BURST |
			   MIPI_DSI_MODE_LPM;

	return 0;
}

static const struct panel_ops rm68200_panel_ops = {
	.enable_backlight = rm68200_panel_enable_backlight,
	.get_display_timing = rm68200_panel_get_display_timing,
};

static const struct udevice_id rm68200_panel_ids[] = {
	{ .compatible = "raydium,rm68200" },
	{ }
};

U_BOOT_DRIVER(rm68200_panel) = {
	.name			  = "rm68200_panel",
	.id			  = UCLASS_PANEL,
	.of_match		  = rm68200_panel_ids,
	.ops			  = &rm68200_panel_ops,
	.of_to_plat	  = rm68200_panel_of_to_plat,
	.probe			  = rm68200_panel_probe,
	.plat_auto	= sizeof(struct mipi_dsi_panel_plat),
	.priv_auto	= sizeof(struct rm68200_panel_priv),
};
