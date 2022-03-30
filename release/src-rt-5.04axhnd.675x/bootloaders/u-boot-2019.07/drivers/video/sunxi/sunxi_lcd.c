// SPDX-License-Identifier: GPL-2.0+
/*
 * Allwinner LCD driver
 *
 * (C) Copyright 2017 Vasily Khoruzhick <anarsoul@gmail.com>
 */

#include <common.h>
#include <display.h>
#include <video_bridge.h>
#include <backlight.h>
#include <dm.h>
#include <edid.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/lcdc.h>
#include <asm/arch/gpio.h>
#include <asm/gpio.h>

struct sunxi_lcd_priv {
	struct display_timing timing;
	int panel_bpp;
};

static void sunxi_lcdc_config_pinmux(void)
{
#ifdef CONFIG_MACH_SUN50I
	int pin;

	for (pin = SUNXI_GPD(0); pin <= SUNXI_GPD(21); pin++) {
		sunxi_gpio_set_cfgpin(pin, SUNXI_GPD_LCD0);
		sunxi_gpio_set_drv(pin, 3);
	}
#endif
}

static int sunxi_lcd_enable(struct udevice *dev, int bpp,
			    const struct display_timing *edid)
{
	struct sunxi_ccm_reg * const ccm =
	       (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct sunxi_lcdc_reg * const lcdc =
	       (struct sunxi_lcdc_reg *)SUNXI_LCD0_BASE;
	struct sunxi_lcd_priv *priv = dev_get_priv(dev);
	struct udevice *backlight;
	int clk_div, clk_double, ret;

	/* Reset off */
	setbits_le32(&ccm->ahb_reset1_cfg, 1 << AHB_RESET_OFFSET_LCD0);
	/* Clock on */
	setbits_le32(&ccm->ahb_gate1, 1 << AHB_GATE_OFFSET_LCD0);

	lcdc_init(lcdc);
	sunxi_lcdc_config_pinmux();
	lcdc_pll_set(ccm, 0, edid->pixelclock.typ / 1000,
		     &clk_div, &clk_double, false);
	lcdc_tcon0_mode_set(lcdc, edid, clk_div, false,
			    priv->panel_bpp, CONFIG_VIDEO_LCD_DCLK_PHASE);
	lcdc_enable(lcdc, priv->panel_bpp);

	ret = uclass_get_device(UCLASS_PANEL_BACKLIGHT, 0, &backlight);
	if (!ret)
		backlight_enable(backlight);

	return 0;
}

static int sunxi_lcd_read_timing(struct udevice *dev,
				 struct display_timing *timing)
{
	struct sunxi_lcd_priv *priv = dev_get_priv(dev);

	memcpy(timing, &priv->timing, sizeof(struct display_timing));

	return 0;
}

static int sunxi_lcd_probe(struct udevice *dev)
{
	struct udevice *cdev;
	struct sunxi_lcd_priv *priv = dev_get_priv(dev);
	int ret;
	int node, timing_node, val;

#ifdef CONFIG_VIDEO_BRIDGE
	/* Try to get timings from bridge first */
	ret = uclass_get_device(UCLASS_VIDEO_BRIDGE, 0, &cdev);
	if (!ret) {
		u8 edid[EDID_SIZE];
		int channel_bpp;

		ret = video_bridge_attach(cdev);
		if (ret) {
			debug("video bridge attach failed: %d\n", ret);
			return ret;
		}
		ret = video_bridge_read_edid(cdev, edid, EDID_SIZE);
		if (ret > 0) {
			ret = edid_get_timing(edid, ret,
					      &priv->timing, &channel_bpp);
			priv->panel_bpp = channel_bpp * 3;
			if (!ret)
				return ret;
		}
	}
#endif

	/* Fallback to timings from DT if there's no bridge or
	 * if reading EDID failed
	 */
	ret = uclass_get_device(UCLASS_PANEL, 0, &cdev);
	if (ret) {
		debug("video panel not found: %d\n", ret);
		return ret;
	}

	if (fdtdec_decode_display_timing(gd->fdt_blob, dev_of_offset(cdev),
					 0, &priv->timing)) {
		debug("%s: Failed to decode display timing\n", __func__);
		return -EINVAL;
	}
	timing_node = fdt_subnode_offset(gd->fdt_blob, dev_of_offset(cdev),
					 "display-timings");
	node = fdt_first_subnode(gd->fdt_blob, timing_node);
	val = fdtdec_get_int(gd->fdt_blob, node, "bits-per-pixel", -1);
	if (val != -1)
		priv->panel_bpp = val;
	else
		priv->panel_bpp = 18;

	return 0;
}

static const struct dm_display_ops sunxi_lcd_ops = {
	.read_timing = sunxi_lcd_read_timing,
	.enable = sunxi_lcd_enable,
};

U_BOOT_DRIVER(sunxi_lcd) = {
	.name   = "sunxi_lcd",
	.id     = UCLASS_DISPLAY,
	.ops    = &sunxi_lcd_ops,
	.probe  = sunxi_lcd_probe,
	.priv_auto_alloc_size = sizeof(struct sunxi_lcd_priv),
};

#ifdef CONFIG_MACH_SUN50I
U_BOOT_DEVICE(sunxi_lcd) = {
	.name = "sunxi_lcd"
};
#endif
