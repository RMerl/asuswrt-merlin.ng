// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 *
 * EFI framebuffer driver based on GOP
 */

#include <common.h>
#include <dm.h>
#include <efi_api.h>
#include <vbe.h>
#include <video.h>

struct pixel {
	u8 pos;
	u8 size;
};

static const struct efi_framebuffer {
	struct pixel red;
	struct pixel green;
	struct pixel blue;
	struct pixel rsvd;
} efi_framebuffer_format_map[] = {
	[EFI_GOT_RGBA8] = { {0, 8}, {8, 8}, {16, 8}, {24, 8} },
	[EFI_GOT_BGRA8] = { {16, 8}, {8, 8}, {0, 8}, {24, 8} },
};

static void efi_find_pixel_bits(u32 mask, u8 *pos, u8 *size)
{
	u8 first, len;

	first = 0;
	len = 0;

	if (mask) {
		while (!(mask & 0x1)) {
			mask = mask >> 1;
			first++;
		}

		while (mask & 0x1) {
			mask = mask >> 1;
			len++;
		}
	}

	*pos = first;
	*size = len;
}

static int save_vesa_mode(struct vesa_mode_info *vesa)
{
	struct efi_entry_gopmode *mode;
	const struct efi_framebuffer *fbinfo;
	int size;
	int ret;

	ret = efi_info_get(EFIET_GOP_MODE, (void **)&mode, &size);
	if (ret == -ENOENT) {
		debug("efi graphics output protocol mode not found\n");
		return -ENXIO;
	}

	vesa->phys_base_ptr = mode->fb_base;
	vesa->x_resolution = mode->info->width;
	vesa->y_resolution = mode->info->height;

	if (mode->info->pixel_format < EFI_GOT_BITMASK) {
		fbinfo = &efi_framebuffer_format_map[mode->info->pixel_format];
		vesa->red_mask_size = fbinfo->red.size;
		vesa->red_mask_pos = fbinfo->red.pos;
		vesa->green_mask_size = fbinfo->green.size;
		vesa->green_mask_pos = fbinfo->green.pos;
		vesa->blue_mask_size = fbinfo->blue.size;
		vesa->blue_mask_pos = fbinfo->blue.pos;
		vesa->reserved_mask_size = fbinfo->rsvd.size;
		vesa->reserved_mask_pos = fbinfo->rsvd.pos;

		vesa->bits_per_pixel = 32;
		vesa->bytes_per_scanline = mode->info->pixels_per_scanline * 4;
	} else if (mode->info->pixel_format == EFI_GOT_BITMASK) {
		efi_find_pixel_bits(mode->info->pixel_bitmask[0],
				    &vesa->red_mask_pos,
				    &vesa->red_mask_size);
		efi_find_pixel_bits(mode->info->pixel_bitmask[1],
				    &vesa->green_mask_pos,
				    &vesa->green_mask_size);
		efi_find_pixel_bits(mode->info->pixel_bitmask[2],
				    &vesa->blue_mask_pos,
				    &vesa->blue_mask_size);
		efi_find_pixel_bits(mode->info->pixel_bitmask[3],
				    &vesa->reserved_mask_pos,
				    &vesa->reserved_mask_size);
		vesa->bits_per_pixel = vesa->red_mask_size +
				       vesa->green_mask_size +
				       vesa->blue_mask_size +
				       vesa->reserved_mask_size;
		vesa->bytes_per_scanline = (mode->info->pixels_per_scanline *
					    vesa->bits_per_pixel) / 8;
	} else {
		debug("efi set unknown framebuffer format: %d\n",
		      mode->info->pixel_format);
		return -EINVAL;
	}

	return 0;
}

static int efi_video_probe(struct udevice *dev)
{
	struct video_uc_platdata *plat = dev_get_uclass_platdata(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct vesa_mode_info *vesa = &mode_info.vesa;
	int ret;

	/* Initialize vesa_mode_info structure */
	ret = save_vesa_mode(vesa);
	if (ret)
		goto err;

	ret = vbe_setup_video_priv(vesa, uc_priv, plat);
	if (ret)
		goto err;

	printf("Video: %dx%dx%d\n", uc_priv->xsize, uc_priv->ysize,
	       vesa->bits_per_pixel);

	return 0;

err:
	printf("No video mode configured in EFI!\n");
	return ret;
}

static const struct udevice_id efi_video_ids[] = {
	{ .compatible = "efi-fb" },
	{ }
};

U_BOOT_DRIVER(efi_video) = {
	.name	= "efi_video",
	.id	= UCLASS_VIDEO,
	.of_match = efi_video_ids,
	.probe	= efi_video_probe,
};
