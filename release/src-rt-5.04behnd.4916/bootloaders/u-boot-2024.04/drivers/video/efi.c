// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 *
 * EFI framebuffer driver based on GOP
 */

#define LOG_CATEGORY LOGC_EFI

#include <common.h>
#include <dm.h>
#include <efi_api.h>
#include <log.h>
#include <vesa.h>
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

/**
 * get_mode_info() - Ask EFI for the mode information
 *
 * Gets info from the graphics-output protocol
 *
 * @vesa: Place to put the mode information
 * @fbp: Returns the address of the frame buffer
 * @infop: Returns a pointer to the mode info
 * Returns: 0 if OK, -ENOSYS if boot services are not available, -ENOTSUPP if
 * the protocol is not supported by EFI
 */
static int get_mode_info(struct vesa_mode_info *vesa, u64 *fbp,
			 struct efi_gop_mode_info **infop)
{
	efi_guid_t efi_gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	struct efi_boot_services *boot = efi_get_boot();
	struct efi_gop_mode *mode;
	struct efi_gop *gop;
	int ret;

	if (!boot)
		return log_msg_ret("sys", -ENOSYS);
	ret = boot->locate_protocol(&efi_gop_guid, NULL, (void **)&gop);
	if (ret)
		return log_msg_ret("prot", -ENOTSUPP);
	mode = gop->mode;
	log_debug("maxmode %u, mode %u, info %p, size %lx, fb %lx, fb_size %lx\n",
		  mode->max_mode, mode->mode, mode->info, mode->info_size,
		  (ulong)mode->fb_base, (ulong)mode->fb_size);

	vesa->phys_base_ptr = mode->fb_base;
	*fbp = mode->fb_base;
	vesa->x_resolution = mode->info->width;
	vesa->y_resolution = mode->info->height;
	*infop = mode->info;

	return 0;
}

/**
 * get_mode_from_entry() - Obtain fb info from the EFIET_GOP_MODE payload entry
 *
 * This gets the mode information provided by the stub to the payload and puts
 * it into a vesa structure. It also returns the mode information.
 *
 * @vesa: Place to put the mode information
 * @fbp: Returns the address of the frame buffer
 * @infop: Returns a pointer to the mode info
 * Returns: 0 if OK, -ve on error
 */
static int get_mode_from_entry(struct vesa_mode_info *vesa, u64 *fbp,
			       struct efi_gop_mode_info **infop)
{
	struct efi_gop_mode *mode;
	int size;
	int ret;

	ret = efi_info_get(EFIET_GOP_MODE, (void **)&mode, &size);
	if (ret) {
		printf("EFI graphics output entry not found\n");
		return ret;
	}
	vesa->phys_base_ptr = mode->fb_base;
	*fbp = mode->fb_base;
	vesa->x_resolution = mode->info->width;
	vesa->y_resolution = mode->info->height;
	*infop = mode->info;

	return 0;
}

static int save_vesa_mode(struct vesa_mode_info *vesa, u64 *fbp)
{
	const struct efi_framebuffer *fbinfo;
	struct efi_gop_mode_info *info;
	int ret;

	if (IS_ENABLED(CONFIG_EFI_APP))
		ret = get_mode_info(vesa, fbp, &info);
	else
		ret = get_mode_from_entry(vesa, fbp, &info);
	if (ret) {
		printf("EFI graphics output protocol not found (err=%dE)\n",
		       ret);
		return ret;
	}

	if (info->pixel_format < EFI_GOT_BITMASK) {
		fbinfo = &efi_framebuffer_format_map[info->pixel_format];
		vesa->red_mask_size = fbinfo->red.size;
		vesa->red_mask_pos = fbinfo->red.pos;
		vesa->green_mask_size = fbinfo->green.size;
		vesa->green_mask_pos = fbinfo->green.pos;
		vesa->blue_mask_size = fbinfo->blue.size;
		vesa->blue_mask_pos = fbinfo->blue.pos;
		vesa->reserved_mask_size = fbinfo->rsvd.size;
		vesa->reserved_mask_pos = fbinfo->rsvd.pos;

		vesa->bits_per_pixel = 32;
		vesa->bytes_per_scanline = info->pixels_per_scanline * 4;
	} else if (info->pixel_format == EFI_GOT_BITMASK) {
		efi_find_pixel_bits(info->pixel_bitmask[0],
				    &vesa->red_mask_pos,
				    &vesa->red_mask_size);
		efi_find_pixel_bits(info->pixel_bitmask[1],
				    &vesa->green_mask_pos,
				    &vesa->green_mask_size);
		efi_find_pixel_bits(info->pixel_bitmask[2],
				    &vesa->blue_mask_pos,
				    &vesa->blue_mask_size);
		efi_find_pixel_bits(info->pixel_bitmask[3],
				    &vesa->reserved_mask_pos,
				    &vesa->reserved_mask_size);
		vesa->bits_per_pixel = vesa->red_mask_size +
				       vesa->green_mask_size +
				       vesa->blue_mask_size +
				       vesa->reserved_mask_size;
		vesa->bytes_per_scanline = (info->pixels_per_scanline *
					    vesa->bits_per_pixel) / 8;
	} else {
		log_err("Unknown framebuffer format: %d\n", info->pixel_format);
		return -EINVAL;
	}

	return 0;
}

static int efi_video_probe(struct udevice *dev)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct vesa_mode_info *vesa = &mode_info.vesa;
	u64 fb;
	int ret;

	/* Initialize vesa_mode_info structure */
	ret = save_vesa_mode(vesa, &fb);
	if (ret)
		goto err;

	ret = vesa_setup_video_priv(vesa, fb, uc_priv, plat);
	if (ret)
		goto err;

	printf("Video: %dx%dx%d @ %lx\n", uc_priv->xsize, uc_priv->ysize,
	       vesa->bits_per_pixel, (ulong)fb);

	return 0;

err:
	printf("No video mode configured in EFI!\n");
	return ret;
}

static int efi_video_bind(struct udevice *dev)
{
	if (IS_ENABLED(CONFIG_VIDEO_COPY)) {
		struct video_uc_plat *plat = dev_get_uclass_plat(dev);
		struct vesa_mode_info vesa;
		int ret;
		u64 fb;

		/*
		 * Initialise vesa_mode_info structure so we can figure out the
		 * required framebuffer size. If something goes wrong, just do
		 * without a copy framebuffer
		 */
		ret = save_vesa_mode(&vesa, &fb);
		if (!ret) {
			/* this is not reached if the EFI call failed */
			plat->copy_size = vesa.bytes_per_scanline *
				vesa.y_resolution;
		}
	}

	return 0;
}

static const struct udevice_id efi_video_ids[] = {
	{ .compatible = "efi-fb" },
	{ }
};

U_BOOT_DRIVER(efi_video) = {
	.name	= "efi_video",
	.id	= UCLASS_VIDEO,
	.of_match = efi_video_ids,
	.bind	= efi_video_bind,
	.probe	= efi_video_probe,
};
