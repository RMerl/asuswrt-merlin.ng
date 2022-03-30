// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <vbe.h>
#include <video.h>
#include <asm/arch/sysinfo.h>

static int save_vesa_mode(struct cb_framebuffer *fb,
			  struct vesa_mode_info *vesa)
{
	/*
	 * If there is no framebuffer structure, bail out and keep
	 * running on the serial console.
	 */
	if (!fb)
		return -ENXIO;

	vesa->x_resolution = fb->x_resolution;
	vesa->y_resolution = fb->y_resolution;
	vesa->bits_per_pixel = fb->bits_per_pixel;
	vesa->bytes_per_scanline = fb->bytes_per_line;
	vesa->phys_base_ptr = fb->physical_address;
	vesa->red_mask_size = fb->red_mask_size;
	vesa->red_mask_pos = fb->red_mask_pos;
	vesa->green_mask_size = fb->green_mask_size;
	vesa->green_mask_pos = fb->green_mask_pos;
	vesa->blue_mask_size = fb->blue_mask_size;
	vesa->blue_mask_pos = fb->blue_mask_pos;
	vesa->reserved_mask_size = fb->reserved_mask_size;
	vesa->reserved_mask_pos = fb->reserved_mask_pos;

	return 0;
}

static int coreboot_video_probe(struct udevice *dev)
{
	struct video_uc_platdata *plat = dev_get_uclass_platdata(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct cb_framebuffer *fb = lib_sysinfo.framebuffer;
	struct vesa_mode_info *vesa = &mode_info.vesa;
	int ret;

	printf("Video: ");

	/* Initialize vesa_mode_info structure */
	ret = save_vesa_mode(fb, vesa);
	if (ret)
		goto err;

	ret = vbe_setup_video_priv(vesa, uc_priv, plat);
	if (ret)
		goto err;

	printf("%dx%dx%d\n", uc_priv->xsize, uc_priv->ysize,
	       vesa->bits_per_pixel);

	return 0;

err:
	printf("No video mode configured in coreboot!\n");
	return ret;
}

static const struct udevice_id coreboot_video_ids[] = {
	{ .compatible = "coreboot-fb" },
	{ }
};

U_BOOT_DRIVER(coreboot_video) = {
	.name	= "coreboot_video",
	.id	= UCLASS_VIDEO,
	.of_match = coreboot_video_ids,
	.probe	= coreboot_video_probe,
};
