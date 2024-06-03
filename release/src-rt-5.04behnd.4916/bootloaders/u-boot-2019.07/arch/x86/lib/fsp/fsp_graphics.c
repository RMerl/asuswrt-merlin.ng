// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <vbe.h>
#include <video.h>
#include <asm/fsp/fsp_support.h>

DECLARE_GLOBAL_DATA_PTR;

struct pixel {
	u8 pos;
	u8 size;
};

static const struct fsp_framebuffer {
	struct pixel red;
	struct pixel green;
	struct pixel blue;
	struct pixel rsvd;
} fsp_framebuffer_format_map[] = {
	[pixel_rgbx_8bpc] = { {0, 8}, {8, 8}, {16, 8}, {24, 8} },
	[pixel_bgrx_8bpc] = { {16, 8}, {8, 8}, {0, 8}, {24, 8} },
};

static int save_vesa_mode(struct vesa_mode_info *vesa)
{
	const struct hob_graphics_info *ginfo;
	const struct fsp_framebuffer *fbinfo;

	ginfo = fsp_get_graphics_info(gd->arch.hob_list, NULL);

	/*
	 * If there is no graphics info structure, bail out and keep
	 * running on the serial console.
	 *
	 * Note: on some platforms (eg: Braswell), the FSP will not produce
	 * the graphics info HOB unless you plug some cables to the display
	 * interface (eg: HDMI) on the board.
	 */
	if (!ginfo) {
		debug("FSP graphics hand-off block not found\n");
		return -ENXIO;
	}

	vesa->x_resolution = ginfo->width;
	vesa->y_resolution = ginfo->height;
	vesa->bits_per_pixel = 32;
	vesa->bytes_per_scanline = ginfo->pixels_per_scanline * 4;
	vesa->phys_base_ptr = ginfo->fb_base;

	if (ginfo->pixel_format >= pixel_bitmask) {
		debug("FSP set unknown framebuffer format: %d\n",
		      ginfo->pixel_format);
		return -EINVAL;
	}
	fbinfo = &fsp_framebuffer_format_map[ginfo->pixel_format];
	vesa->red_mask_size = fbinfo->red.size;
	vesa->red_mask_pos = fbinfo->red.pos;
	vesa->green_mask_size = fbinfo->green.size;
	vesa->green_mask_pos = fbinfo->green.pos;
	vesa->blue_mask_size = fbinfo->blue.size;
	vesa->blue_mask_pos = fbinfo->blue.pos;
	vesa->reserved_mask_size = fbinfo->rsvd.size;
	vesa->reserved_mask_pos = fbinfo->rsvd.pos;

	return 0;
}

static int fsp_video_probe(struct udevice *dev)
{
	struct video_uc_platdata *plat = dev_get_uclass_platdata(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct vesa_mode_info *vesa = &mode_info.vesa;
	int ret;

	printf("Video: ");

	/* Initialize vesa_mode_info structure */
	ret = save_vesa_mode(vesa);
	if (ret)
		goto err;

	/*
	 * The framebuffer base address in the FSP graphics info HOB reflects
	 * the value assigned by the FSP. After PCI enumeration the framebuffer
	 * base address may be relocated. Let's get the updated one from device.
	 *
	 * For IGD, it seems to be always on BAR2.
	 */
	vesa->phys_base_ptr = dm_pci_read_bar32(dev, 2);

	ret = vbe_setup_video_priv(vesa, uc_priv, plat);
	if (ret)
		goto err;

	printf("%dx%dx%d\n", uc_priv->xsize, uc_priv->ysize,
	       vesa->bits_per_pixel);

	return 0;

err:
	printf("No video mode configured in FSP!\n");
	return ret;
}

static const struct udevice_id fsp_video_ids[] = {
	{ .compatible = "fsp-fb" },
	{ }
};

U_BOOT_DRIVER(fsp_video) = {
	.name	= "fsp_video",
	.id	= UCLASS_VIDEO,
	.of_match = fsp_video_ids,
	.probe	= fsp_video_probe,
};

static struct pci_device_id fsp_video_supported[] = {
	{ PCI_DEVICE_CLASS(PCI_CLASS_DISPLAY_VGA << 8, 0xffff00) },
	{ },
};

U_BOOT_PCI_DEVICE(fsp_video, fsp_video_supported);
