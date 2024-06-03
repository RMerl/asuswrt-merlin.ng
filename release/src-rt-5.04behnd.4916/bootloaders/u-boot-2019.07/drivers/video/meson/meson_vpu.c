// SPDX-License-Identifier: GPL-2.0
/*
 * Amlogic Meson Video Processing Unit driver
 *
 * Copyright (c) 2018 BayLibre, SAS.
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#include "meson_vpu.h"
#include <power-domain.h>
#include <efi_loader.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#include <fdt_support.h>
#include <linux/sizes.h>
#include <asm/arch/mem.h>
#include "meson_registers.h"
#include "simplefb_common.h"

#define MESON_VPU_OVERSCAN SZ_64K

/* Static variable for use in meson_vpu_rsv_fb() */
static struct meson_framebuffer {
	u64 base;
	u64 fb_size;
	unsigned int xsize;
	unsigned int ysize;
	bool is_cvbs;
} meson_fb = { 0 };

static int meson_vpu_setup_mode(struct udevice *dev, struct udevice *disp)
{
	struct video_uc_platdata *uc_plat = dev_get_uclass_platdata(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct display_timing timing;
	bool is_cvbs = false;
	int ret = 0;

	if (disp) {
		ret = display_read_timing(disp, &timing);
		if (ret) {
			debug("%s: Failed to read timings\n", __func__);
			goto cvbs;
		}

		uc_priv->xsize = timing.hactive.typ;
		uc_priv->ysize = timing.vactive.typ;

		ret = display_enable(disp, 0, &timing);
		if (ret)
			goto cvbs;
	} else {
cvbs:
		/* CVBS has a fixed 720x480i (NTSC) and 720x576i (PAL) */
		is_cvbs = true;
		timing.flags = DISPLAY_FLAGS_INTERLACED;
		uc_priv->xsize = 720;
		uc_priv->ysize = 576;
	}

	uc_priv->bpix = VPU_MAX_LOG2_BPP;

	meson_fb.is_cvbs = is_cvbs;
	meson_fb.xsize = uc_priv->xsize;
	meson_fb.ysize = uc_priv->ysize;

	/* Move the framebuffer to the end of addressable ram */
	meson_fb.fb_size = ALIGN(meson_fb.xsize * meson_fb.ysize *
				 ((1 << VPU_MAX_LOG2_BPP) / 8) +
				 MESON_VPU_OVERSCAN, EFI_PAGE_SIZE);
	meson_fb.base = gd->bd->bi_dram[0].start +
			gd->bd->bi_dram[0].size - meson_fb.fb_size;

	/* Override the framebuffer address */
	uc_plat->base = meson_fb.base;

	meson_vpu_setup_plane(dev, timing.flags & DISPLAY_FLAGS_INTERLACED);
	meson_vpu_setup_venc(dev, &timing, is_cvbs);
	meson_vpu_setup_vclk(dev, &timing, is_cvbs);

	video_set_flush_dcache(dev, 1);

	return 0;
}

static const struct udevice_id meson_vpu_ids[] = {
	{ .compatible = "amlogic,meson-gxbb-vpu", .data = VPU_COMPATIBLE_GXBB },
	{ .compatible = "amlogic,meson-gxl-vpu", .data = VPU_COMPATIBLE_GXL },
	{ .compatible = "amlogic,meson-gxm-vpu", .data = VPU_COMPATIBLE_GXM },
	{ }
};

static int meson_vpu_probe(struct udevice *dev)
{
	struct meson_vpu_priv *priv = dev_get_priv(dev);
	struct power_domain pd;
	struct udevice *disp;
	int ret;

	/* Before relocation we don't need to do anything */
	if (!(gd->flags & GD_FLG_RELOC))
		return 0;

	priv->dev = dev;

	priv->io_base = dev_remap_addr_index(dev, 0);
	if (!priv->io_base)
		return -EINVAL;

	priv->hhi_base = dev_remap_addr_index(dev, 1);
	if (!priv->hhi_base)
		return -EINVAL;

	priv->dmc_base = dev_remap_addr_index(dev, 2);
	if (!priv->dmc_base)
		return -EINVAL;

	ret = power_domain_get(dev, &pd);
	if (ret)
		return ret;

	ret = power_domain_on(&pd);
	if (ret)
		return ret;

	meson_vpu_init(dev);

	/* probe the display */
	ret = uclass_get_device(UCLASS_DISPLAY, 0, &disp);

	return meson_vpu_setup_mode(dev, ret ? NULL : disp);
}

static int meson_vpu_bind(struct udevice *dev)
{
	struct video_uc_platdata *plat = dev_get_uclass_platdata(dev);

	plat->size = VPU_MAX_WIDTH * VPU_MAX_HEIGHT *
		(1 << VPU_MAX_LOG2_BPP) / 8;

	return 0;
}

#if defined(CONFIG_VIDEO_DT_SIMPLEFB)
static void meson_vpu_setup_simplefb(void *fdt)
{
	const char *pipeline = NULL;
	u64 mem_start, mem_size;
	int offset, ret;

	if (meson_fb.is_cvbs)
		pipeline = "vpu-cvbs";
	else
		pipeline = "vpu-hdmi";

	offset = meson_simplefb_fdt_match(fdt, pipeline);
	if (offset < 0) {
		eprintf("Cannot setup simplefb: node not found\n");

		/* If simplefb is missing, add it as reserved memory */
		meson_board_add_reserved_memory(fdt, meson_fb.base,
						meson_fb.fb_size);

		return;
	}

	/*
	 * SimpleFB will try to iomap the framebuffer, so we can't use
	 * fdt_add_mem_rsv on the memory area. Instead, the FB is stored
	 * at the end of the RAM and we strip this portion from the kernel
	 * allowed region
	 */
	mem_start = gd->bd->bi_dram[0].start;
	mem_size = gd->bd->bi_dram[0].size - meson_fb.fb_size;
	ret = fdt_fixup_memory_banks(fdt, &mem_start, &mem_size, 1);
	if (ret) {
		eprintf("Cannot setup simplefb: Error reserving memory\n");
		return;
	}

	ret = fdt_setup_simplefb_node(fdt, offset, meson_fb.base,
				      meson_fb.xsize, meson_fb.ysize,
				      meson_fb.xsize * 4, "x8r8g8b8");
	if (ret)
		eprintf("Cannot setup simplefb: Error setting properties\n");
}
#endif

void meson_vpu_rsv_fb(void *fdt)
{
	if (!meson_fb.base || !meson_fb.xsize || !meson_fb.ysize)
		return;

#if defined(CONFIG_EFI_LOADER)
	efi_add_memory_map(meson_fb.base, meson_fb.fb_size >> EFI_PAGE_SHIFT,
			   EFI_RESERVED_MEMORY_TYPE, false);
#endif
#if defined(CONFIG_VIDEO_DT_SIMPLEFB)
	meson_vpu_setup_simplefb(fdt);
#endif
}

U_BOOT_DRIVER(meson_vpu) = {
	.name = "meson_vpu",
	.id = UCLASS_VIDEO,
	.of_match = meson_vpu_ids,
	.probe = meson_vpu_probe,
	.bind = meson_vpu_bind,
	.priv_auto_alloc_size = sizeof(struct meson_vpu_priv),
	.flags  = DM_FLAG_PRE_RELOC,
};
