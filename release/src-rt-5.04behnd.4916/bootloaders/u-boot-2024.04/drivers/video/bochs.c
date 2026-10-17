// SPDX-License-Identifier: GPL-2.0
/*
 * Modified from coreboot bochs.c
 */

#define LOG_CATEGORY	UCLASS_VIDEO

#include <common.h>
#include <dm.h>
#include <log.h>
#include <pci.h>
#include <video.h>
#include <asm/io.h>
#include <linux/sizes.h>
#include "bochs.h"

static int xsize = CONFIG_VIDEO_BOCHS_SIZE_X;
static int ysize = CONFIG_VIDEO_BOCHS_SIZE_Y;

static void bochs_write(void *mmio, int index, int val)
{
	writew(val, mmio + MMIO_BASE + index * 2);
}

static int bochs_read(void *mmio, int index)
{
	return readw(mmio + MMIO_BASE + index * 2);
}

static void bochs_vga_write(void *mmio, int index, uint8_t val)
{
	writeb(val, mmio + VGA_BASE + index);
}

static int bochs_init_fb(struct udevice *dev)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	ulong fb;
	void *mmio;
	int id, mem;

	log_debug("probing %s at PCI %x\n", dev->name, dm_pci_get_bdf(dev));
	fb = dm_pci_read_bar32(dev, 0);
	if (!fb)
		return log_msg_ret("fb", -EIO);

	/* MMIO bar supported since qemu 3.0+ */
	mmio = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_2, 0, 0, PCI_REGION_TYPE,
			      PCI_REGION_MEM);

	if (!mmio)
		return log_msg_ret("map", -EIO);

	/* bochs dispi detection */
	id = bochs_read(mmio, INDEX_ID);
	if ((id & 0xfff0) != ID0) {
		log_debug("ID mismatch\n");
		return -EPROTONOSUPPORT;
	}
	mem = bochs_read(mmio, INDEX_VIDEO_MEMORY_64K) * SZ_64K;
	log_debug("QEMU VGA: bochs @ %p: %d MiB FB at %lx\n", mmio, mem / SZ_1M,
		  fb);

	uc_priv->xsize = xsize;
	uc_priv->ysize = ysize;
	uc_priv->bpix = VIDEO_BPP32;

	/* setup video mode */
	bochs_write(mmio, INDEX_ENABLE,  0);
	bochs_write(mmio, INDEX_BANK,  0);
	bochs_write(mmio, INDEX_BPP, VNBITS(uc_priv->bpix));
	bochs_write(mmio, INDEX_XRES, xsize);
	bochs_write(mmio, INDEX_YRES, ysize);
	bochs_write(mmio, INDEX_VIRT_WIDTH, xsize);
	bochs_write(mmio, INDEX_VIRT_HEIGHT, ysize);
	bochs_write(mmio, INDEX_X_OFFSET, 0);
	bochs_write(mmio, INDEX_Y_OFFSET, 0);
	bochs_write(mmio, INDEX_ENABLE, ENABLED | LFB_ENABLED);

	/* disable blanking */
	bochs_vga_write(mmio, VGA_ATT_W - VGA_INDEX, VGA_AR_ENABLE_DISPLAY);

	plat->base = fb;

	return 0;
}

static int bochs_video_probe(struct udevice *dev)
{
	int ret;

	ret = bochs_init_fb(dev);
	if (ret)
		return log_ret(ret);

	return 0;
}

static int bochs_video_bind(struct udevice *dev)
{
	struct video_uc_plat *uc_plat = dev_get_uclass_plat(dev);

	/* Set the frame buffer size per configuration */
	uc_plat->size = xsize * ysize * 32 / 8;
	log_debug("%s: Frame buffer size %x\n", __func__, uc_plat->size);

	return 0;
}

U_BOOT_DRIVER(bochs_video) = {
	.name	= "bochs_video",
	.id	= UCLASS_VIDEO,
	.bind	= bochs_video_bind,
	.probe	= bochs_video_probe,
};

static struct pci_device_id bochs_video_supported[] = {
	{ PCI_DEVICE(0x1234, 0x1111) },
	{ },
};

U_BOOT_PCI_DEVICE(bochs_video, bochs_video_supported);
