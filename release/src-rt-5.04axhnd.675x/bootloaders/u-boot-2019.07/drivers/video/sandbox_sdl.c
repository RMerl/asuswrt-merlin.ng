// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <video.h>
#include <asm/sdl.h>
#include <asm/u-boot-sandbox.h>
#include <dm/test.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	/* Default LCD size we support */
	LCD_MAX_WIDTH		= 1366,
	LCD_MAX_HEIGHT		= 768,
};

static int sandbox_sdl_probe(struct udevice *dev)
{
	struct sandbox_sdl_plat *plat = dev_get_platdata(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	int ret;

	ret = sandbox_sdl_init_display(plat->xres, plat->yres, plat->bpix);
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

	return 0;
}

static int sandbox_sdl_bind(struct udevice *dev)
{
	struct video_uc_platdata *uc_plat = dev_get_uclass_platdata(dev);
	struct sandbox_sdl_plat *plat = dev_get_platdata(dev);
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(dev);
	int ret = 0;

	plat->xres = fdtdec_get_int(blob, node, "xres", LCD_MAX_WIDTH);
	plat->yres = fdtdec_get_int(blob, node, "yres", LCD_MAX_HEIGHT);
	plat->bpix = VIDEO_BPP16;
	uc_plat->size = plat->xres * plat->yres * (1 << plat->bpix) / 8;
	debug("%s: Frame buffer size %x\n", __func__, uc_plat->size);

	return ret;
}

static const struct udevice_id sandbox_sdl_ids[] = {
	{ .compatible = "sandbox,lcd-sdl" },
	{ }
};

U_BOOT_DRIVER(sdl_sandbox) = {
	.name	= "sdl_sandbox",
	.id	= UCLASS_VIDEO,
	.of_match = sandbox_sdl_ids,
	.bind	= sandbox_sdl_bind,
	.probe	= sandbox_sdl_probe,
	.platdata_auto_alloc_size	= sizeof(struct sandbox_sdl_plat),
};
