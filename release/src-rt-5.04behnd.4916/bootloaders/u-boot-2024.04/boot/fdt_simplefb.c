// SPDX-License-Identifier: GPL-2.0+
/*
 * Simplefb device tree support
 *
 * (C) Copyright 2015
 * Stephen Warren <swarren@wwwdotorg.org>
 */

#include <common.h>
#include <dm.h>
#include <fdt_support.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <video.h>

DECLARE_GLOBAL_DATA_PTR;

static int fdt_simplefb_configure_node(void *blob, int off)
{
	int xsize, ysize;
	int bpix; /* log2 of bits per pixel */
	const char *name;
	ulong fb_base;
	struct video_uc_plat *plat;
	struct video_priv *uc_priv;
	struct udevice *dev;
	int ret;

	ret = uclass_first_device_err(UCLASS_VIDEO, &dev);
	if (ret)
		return ret;
	uc_priv = dev_get_uclass_priv(dev);
	plat = dev_get_uclass_plat(dev);
	xsize = uc_priv->xsize;
	ysize = uc_priv->ysize;
	bpix = uc_priv->bpix;
	fb_base = plat->base;
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

int fdt_simplefb_add_node(void *blob)
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

	return fdt_simplefb_configure_node(blob, off);
}

/**
 * fdt_simplefb_enable_existing_node() - enable simple-framebuffer DT node
 *
 * @blob:	device-tree
 * Return:	0 on success, non-zero otherwise
 */
static int fdt_simplefb_enable_existing_node(void *blob)
{
	int off;

	off = fdt_node_offset_by_compatible(blob, -1, "simple-framebuffer");
	if (off < 0)
		return -1;

	return fdt_simplefb_configure_node(blob, off);
}

#if IS_ENABLED(CONFIG_VIDEO)
int fdt_simplefb_enable_and_mem_rsv(void *blob)
{
	struct fdt_memory mem;
	int ret;

	/* nothing to do when video is not active */
	if (!video_is_active())
		return 0;

	ret = fdt_simplefb_enable_existing_node(blob);
	if (ret)
		return ret;

	/* nothing to do when the frame buffer is not defined */
	if (gd->video_bottom == gd->video_top)
		return 0;

	/* reserved with no-map tag the video buffer */
	mem.start = gd->video_bottom;
	mem.end = gd->video_top - 1;

	return fdtdec_add_reserved_memory(blob, "framebuffer", &mem, NULL, 0, NULL,
					  FDTDEC_RESERVED_MEMORY_NO_MAP);
}
#endif
