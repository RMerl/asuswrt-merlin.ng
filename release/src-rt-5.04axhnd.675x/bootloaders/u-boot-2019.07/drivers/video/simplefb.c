// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017 Rob Clark
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <fdt_support.h>
#include <video.h>

static int simple_video_probe(struct udevice *dev)
{
	struct video_uc_platdata *plat = dev_get_uclass_platdata(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	const void *blob = gd->fdt_blob;
	const int node = dev_of_offset(dev);
	const char *format;
	fdt_addr_t base;
	fdt_size_t size;

	base = fdtdec_get_addr_size_auto_parent(blob, dev_of_offset(dev->parent),
			node, "reg", 0, &size, false);
	if (base == FDT_ADDR_T_NONE) {
		debug("%s: Failed to decode memory region\n", __func__);
		return -EINVAL;
	}

	debug("%s: base=%llx, size=%llu\n", __func__, base, size);

	/*
	 * TODO is there some way to reserve the framebuffer
	 * region so it isn't clobbered?
	 */
	plat->base = base;
	plat->size = size;

	video_set_flush_dcache(dev, true);

	debug("%s: Query resolution...\n", __func__);

	uc_priv->xsize = fdtdec_get_uint(blob, node, "width", 0);
	uc_priv->ysize = fdtdec_get_uint(blob, node, "height", 0);
	uc_priv->rot = 0;

	format = fdt_getprop(blob, node, "format", NULL);
	debug("%s: %dx%d@%s\n", __func__, uc_priv->xsize, uc_priv->ysize, format);

	if (strcmp(format, "r5g6b5") == 0) {
		uc_priv->bpix = VIDEO_BPP16;
	} else if (strcmp(format, "a8b8g8r8") == 0) {
		uc_priv->bpix = VIDEO_BPP32;
	} else {
		printf("%s: invalid format: %s\n", __func__, format);
		return -EINVAL;
	}

	return 0;
}

static const struct udevice_id simple_video_ids[] = {
	{ .compatible = "simple-framebuffer" },
	{ }
};

U_BOOT_DRIVER(simple_video) = {
	.name	= "simple_video",
	.id	= UCLASS_VIDEO,
	.of_match = simple_video_ids,
	.probe	= simple_video_probe,
};
