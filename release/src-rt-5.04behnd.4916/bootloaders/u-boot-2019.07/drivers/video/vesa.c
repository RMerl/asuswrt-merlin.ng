// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <pci.h>
#include <vbe.h>

static int vesa_video_probe(struct udevice *dev)
{
	return vbe_setup_video(dev, NULL);
}

static const struct udevice_id vesa_video_ids[] = {
	{ .compatible = "vesa-fb" },
	{ }
};

U_BOOT_DRIVER(vesa_video) = {
	.name	= "vesa_video",
	.id	= UCLASS_VIDEO,
	.of_match = vesa_video_ids,
	.probe	= vesa_video_probe,
};

static struct pci_device_id vesa_video_supported[] = {
	{ PCI_DEVICE_CLASS(PCI_CLASS_DISPLAY_VGA << 8, ~0) },
	{ },
};

U_BOOT_PCI_DEVICE(vesa_video, vesa_video_supported);
