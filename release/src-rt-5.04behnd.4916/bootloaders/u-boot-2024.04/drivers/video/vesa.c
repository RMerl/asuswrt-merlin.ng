// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <pci.h>
#include <vesa.h>
#include <video.h>
#include <asm/mtrr.h>

static int vesa_video_probe(struct udevice *dev)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);
	ulong fbbase;
	int ret;

	ret = vesa_setup_video(dev, NULL);
	if (ret)
		return log_ret(ret);

	/* Use write-combining for the graphics memory, 256MB */
	fbbase = IS_ENABLED(CONFIG_VIDEO_COPY) ? plat->copy_base : plat->base;
	mtrr_set_next_var(MTRR_TYPE_WRCOMB, fbbase, 256 << 20);

	return 0;
}

static int vesa_video_bind(struct udevice *dev)
{
	struct video_uc_plat *uc_plat = dev_get_uclass_plat(dev);

	/* Set the maximum supported resolution */
	uc_plat->size = 2560 * 1600 * 4;
	log_debug("%s: Frame buffer size %x\n", __func__, uc_plat->size);

	return 0;
}

static const struct udevice_id vesa_video_ids[] = {
	{ .compatible = "vesa-fb" },
	{ }
};

U_BOOT_DRIVER(vesa_video) = {
	.name	= "vesa_video",
	.id	= UCLASS_VIDEO,
	.of_match = vesa_video_ids,
	.bind	= vesa_video_bind,
	.probe	= vesa_video_probe,
};

static struct pci_device_id vesa_video_supported[] = {
	{ PCI_DEVICE_CLASS(PCI_CLASS_DISPLAY_VGA << 8, ~0) },
	{ },
};

U_BOOT_PCI_DEVICE(vesa_video, vesa_video_supported);
