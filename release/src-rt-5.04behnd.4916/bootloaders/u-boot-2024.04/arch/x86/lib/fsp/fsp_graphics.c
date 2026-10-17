// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, Bin Meng <bmeng.cn@gmail.com>
 */

#define LOG_CATEGORY UCLASS_VIDEO

#include <common.h>
#include <dm.h>
#include <init.h>
#include <log.h>
#include <vesa.h>
#include <video.h>
#include <acpi/acpi_table.h>
#include <asm/fsp/fsp_support.h>
#include <asm/global_data.h>
#include <asm/intel_opregion.h>
#include <asm/mtrr.h>
#include <dm/acpi.h>

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
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct vesa_mode_info *vesa = &mode_info.vesa;
	int ret;

	if (!ll_boot_init())
		return -ENODEV;

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
	gd->fb_base = vesa->phys_base_ptr;

	ret = vesa_setup_video_priv(vesa, vesa->phys_base_ptr, uc_priv, plat);
	if (ret)
		goto err;

	mtrr_set_next_var(MTRR_TYPE_WRCOMB, vesa->phys_base_ptr, 256 << 20);

	printf("%dx%dx%d @ %x\n", uc_priv->xsize, uc_priv->ysize,
	       vesa->bits_per_pixel, vesa->phys_base_ptr);

	return 0;

err:
	printf("No video mode configured in FSP!\n");
	return ret;
}

static int fsp_video_bind(struct udevice *dev)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);

	/* Set the maximum supported resolution */
	plat->size = 2560 * 1600 * 4;

	return 0;
}

#ifdef CONFIG_INTEL_GMA_ACPI
static int fsp_video_acpi_write_tables(const struct udevice *dev,
				       struct acpi_ctx *ctx)
{
	struct igd_opregion *opregion;
	int ret;

	log_debug("ACPI:    * IGD OpRegion\n");
	opregion = (struct igd_opregion *)ctx->current;

	ret = intel_gma_init_igd_opregion((struct udevice *)dev, opregion);
	if (ret)
		return ret;

	acpi_inc_align(ctx, sizeof(struct igd_opregion));

	return 0;
}
#endif

struct acpi_ops fsp_video_acpi_ops = {
#ifdef CONFIG_INTEL_GMA_ACPI
	.write_tables	= fsp_video_acpi_write_tables,
#endif
};

static const struct udevice_id fsp_video_ids[] = {
	{ .compatible = "fsp-fb" },
	{ }
};

U_BOOT_DRIVER(fsp_video) = {
	.name	= "fsp_video",
	.id	= UCLASS_VIDEO,
	.of_match = fsp_video_ids,
	.bind	= fsp_video_bind,
	.probe	= fsp_video_probe,
	.flags	= DM_FLAG_PRE_RELOC,
	ACPI_OPS_PTR(&fsp_video_acpi_ops)
};

static struct pci_device_id fsp_video_supported[] = {
	{ PCI_DEVICE_CLASS(PCI_CLASS_DISPLAY_VGA << 8, 0xffff00) },
	{ },
};

U_BOOT_PCI_DEVICE(fsp_video, fsp_video_supported);
