// SPDX-License-Identifier: GPL-2.0+
/*
 * Simplefb device tree support
 *
 * (C) Copyright 2015
 * Stephen Warren <swarren@wwwdotorg.org>
 */

#include <common.h>
#include <dm.h>
#include <lcd.h>
#include <fdt_support.h>
#include <linux/libfdt.h>
#include <video.h>

DECLARE_GLOBAL_DATA_PTR;

static int lcd_dt_simplefb_configure_node(void *blob, int off)
{
	int xsize, ysize;
	int bpix; /* log2 of bits per pixel */
	const char *name;
	ulong fb_base;
#ifdef CONFIG_DM_VIDEO
	struct video_uc_platdata *plat;
	struct video_priv *uc_priv;
	struct udevice *dev;
	int ret;

	ret = uclass_first_device_err(UCLASS_VIDEO, &dev);
	if (ret)
		return ret;
	uc_priv = dev_get_uclass_priv(dev);
	plat = dev_get_uclass_platdata(dev);
	xsize = uc_priv->xsize;
	ysize = uc_priv->ysize;
	bpix = uc_priv->bpix;
	fb_base = plat->base;
#else
	xsize = lcd_get_pixel_width();
	ysize = lcd_get_pixel_height();
	bpix = LCD_BPP;
	fb_base = gd->fb_base;
#endif
	switch (bpix) {
	case 4: /* VIDEO_BPP16 */
		name = "r5g6b5";
		break;
	case 5: /* VIDEO_BPP32 */
		name = "a8r8g8b8";
		break;
	default:
		return -EINVAL;
	}

	return fdt_setup_simplefb_node(blob, off, fb_base, xsize, ysize,
				       xsize * (1 << bpix) / 8, name);
}

int lcd_dt_simplefb_add_node(void *blob)
{
	static const char compat[] = "simple-framebuffer";
	static const char disabled[] = "disabled";
	int off, ret;

	off = fdt_add_subnode(blob, 0, "framebuffer");
	if (off < 0)
		return -1;

	ret = fdt_setprop(blob, off, "status", disabled, sizeof(disabled));
	if (ret < 0)
		return -1;

	ret = fdt_setprop(blob, off, "compatible", compat, sizeof(compat));
	if (ret < 0)
		return -1;

	return lcd_dt_simplefb_configure_node(blob, off);
}

int lcd_dt_simplefb_enable_existing_node(void *blob)
{
	int off;

	off = fdt_node_offset_by_compatible(blob, -1, "simple-framebuffer");
	if (off < 0)
		return -1;

	return lcd_dt_simplefb_configure_node(blob, off);
}
