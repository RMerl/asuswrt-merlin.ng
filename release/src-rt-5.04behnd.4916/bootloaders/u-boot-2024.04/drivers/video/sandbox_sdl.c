// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <log.h>
#include <video.h>
#include <asm/global_data.h>
#include <asm/sdl.h>
#include <asm/state.h>
#include <asm/u-boot-sandbox.h>
#include <dm/device-internal.h>
#include <dm/test.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	/* Default LCD size we support */
	LCD_MAX_WIDTH		= 1366,
	LCD_MAX_HEIGHT		= 768,
};

static int sandbox_sdl_probe(struct udevice *dev)
{
	struct video_uc_plat *uc_plat = dev_get_uclass_plat(dev);
	struct sandbox_sdl_plat *plat = dev_get_plat(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct sandbox_state *state = state_get_current();
	int ret;

	ret = sandbox_sdl_init_display(plat->xres, plat->yres, plat->bpix,
				       state->double_lcd);
	if (ret) {
		puts("LCD init failed\n");
		return ret;
	}
	uc_priv->xsize = plat->xres;
	uc_priv->ysize = plat->yres;
	uc_priv->bpix = plat->bpix;
	uc_priv->rot = plat->rot;
	uc_priv->vidconsole_drv_name = plat->vidconsole_drv_name;
	uc_priv->font_size = plat->font_size;
	if (IS_ENABLED(CONFIG_VIDEO_COPY))
		uc_plat->copy_base = uc_plat->base + uc_plat->size / 2;

	return 0;
}

static void set_bpp(struct udevice *dev, enum video_log2_bpp l2bpp)
{
	struct video_uc_plat *uc_plat = dev_get_uclass_plat(dev);
	struct sandbox_sdl_plat *plat = dev_get_plat(dev);

	plat->bpix = l2bpp;

	uc_plat->size = plat->xres * plat->yres * VNBYTES(plat->bpix);

	/*
	 * Set up to the maximum size we'll ever need. This is a strange case.
	 * The video memory is allocated by video_post_bind() called from
	 * board_init_r(). If a test changes the reoslution so it needs more
	 * memory later (with sandbox_sdl_set_bpp()), it is too late to make
	 * the frame buffer larger.
	 *
	 * So use a maximum size here.
	 */
	uc_plat->size = max(uc_plat->size, 1920U * 1080 * VNBYTES(VIDEO_BPP32));

	/* Allow space for two buffers, the lower one being the copy buffer */
	log_debug("Frame buffer size %x\n", uc_plat->size);

	/*
	 * If a copy framebuffer is used, double the size and use the last half
	 * as the copy, with the first half as the normal frame buffer.
	 */
	if (IS_ENABLED(CONFIG_VIDEO_COPY))
		uc_plat->size *= 2;
}

int sandbox_sdl_set_bpp(struct udevice *dev, enum video_log2_bpp l2bpp)
{
	struct video_uc_plat *uc_plat = dev_get_uclass_plat(dev);
	int ret;

	if (device_active(dev))
		return -EINVAL;
	sandbox_sdl_remove_display();

	uc_plat->hide_logo = true;
	set_bpp(dev, l2bpp);

	ret = device_probe(dev);
	if (ret)
		return ret;

	return 0;
}

static int sandbox_sdl_remove(struct udevice *dev)
{
	/*
	 * Removing the display it a bit annoying when running unit tests, since
	 * they remove all devices. It is nice to be able to see what the test
	 * wrote onto the display. So this comment is just here to show how to
	 * do it, if we want to make it optional one day.
	 *
	 * sandbox_sdl_remove_display();
	 */
	return 0;
}

static int sandbox_sdl_bind(struct udevice *dev)
{
	struct sandbox_sdl_plat *plat = dev_get_plat(dev);
	enum video_log2_bpp l2bpp;
	int ret = 0;

	plat->xres = dev_read_u32_default(dev, "xres", LCD_MAX_WIDTH);
	plat->yres = dev_read_u32_default(dev, "yres", LCD_MAX_HEIGHT);
	l2bpp = dev_read_u32_default(dev, "log2-depth", VIDEO_BPP16);
	plat->rot = dev_read_u32_default(dev, "rotate", 0);

	set_bpp(dev, l2bpp);

	return ret;
}

static const struct udevice_id sandbox_sdl_ids[] = {
	{ .compatible = "sandbox,lcd-sdl" },
	{ }
};

U_BOOT_DRIVER(sandbox_lcd_sdl) = {
	.name	= "sandbox_lcd_sdl",
	.id	= UCLASS_VIDEO,
	.of_match = sandbox_sdl_ids,
	.bind	= sandbox_sdl_bind,
	.probe	= sandbox_sdl_probe,
	.remove	= sandbox_sdl_remove,
	.plat_auto	= sizeof(struct sandbox_sdl_plat),
};
