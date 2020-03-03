/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "bochs.h"

/* ---------------------------------------------------------------------- */

static void bochs_vga_writeb(struct bochs_device *bochs, u16 ioport, u8 val)
{
	if (WARN_ON(ioport < 0x3c0 || ioport > 0x3df))
		return;

	if (bochs->mmio) {
		int offset = ioport - 0x3c0 + 0x400;
		writeb(val, bochs->mmio + offset);
	} else {
		outb(val, ioport);
	}
}

static u16 bochs_dispi_read(struct bochs_device *bochs, u16 reg)
{
	u16 ret = 0;

	if (bochs->mmio) {
		int offset = 0x500 + (reg << 1);
		ret = readw(bochs->mmio + offset);
	} else {
		outw(reg, VBE_DISPI_IOPORT_INDEX);
		ret = inw(VBE_DISPI_IOPORT_DATA);
	}
	return ret;
}

static void bochs_dispi_write(struct bochs_device *bochs, u16 reg, u16 val)
{
	if (bochs->mmio) {
		int offset = 0x500 + (reg << 1);
		writew(val, bochs->mmio + offset);
	} else {
		outw(reg, VBE_DISPI_IOPORT_INDEX);
		outw(val, VBE_DISPI_IOPORT_DATA);
	}
}

int bochs_hw_init(struct drm_device *dev, uint32_t flags)
{
	struct bochs_device *bochs = dev->dev_private;
	struct pci_dev *pdev = dev->pdev;
	unsigned long addr, size, mem, ioaddr, iosize, qext_size;
	u16 id;

	if (pdev->resource[2].flags & IORESOURCE_MEM) {
		/* mmio bar with vga and bochs registers present */
		if (pci_request_region(pdev, 2, "bochs-drm") != 0) {
			DRM_ERROR("Cannot request mmio region\n");
			return -EBUSY;
		}
		ioaddr = pci_resource_start(pdev, 2);
		iosize = pci_resource_len(pdev, 2);
		bochs->mmio = ioremap(ioaddr, iosize);
		if (bochs->mmio == NULL) {
			DRM_ERROR("Cannot map mmio region\n");
			return -ENOMEM;
		}
	} else {
		ioaddr = VBE_DISPI_IOPORT_INDEX;
		iosize = 2;
		if (!request_region(ioaddr, iosize, "bochs-drm")) {
			DRM_ERROR("Cannot request ioports\n");
			return -EBUSY;
		}
		bochs->ioports = 1;
	}

	id = bochs_dispi_read(bochs, VBE_DISPI_INDEX_ID);
	mem = bochs_dispi_read(bochs, VBE_DISPI_INDEX_VIDEO_MEMORY_64K)
		* 64 * 1024;
	if ((id & 0xfff0) != VBE_DISPI_ID0) {
		DRM_ERROR("ID mismatch\n");
		return -ENODEV;
	}

	if ((pdev->resource[0].flags & IORESOURCE_MEM) == 0)
		return -ENODEV;
	addr = pci_resource_start(pdev, 0);
	size = pci_resource_len(pdev, 0);
	if (addr == 0)
		return -ENODEV;
	if (size != mem) {
		DRM_ERROR("Size mismatch: pci=%ld, bochs=%ld\n",
			size, mem);
		size = min(size, mem);
	}

	if (pci_request_region(pdev, 0, "bochs-drm") != 0) {
		DRM_ERROR("Cannot request framebuffer\n");
		return -EBUSY;
	}

	bochs->fb_map = ioremap(addr, size);
	if (bochs->fb_map == NULL) {
		DRM_ERROR("Cannot map framebuffer\n");
		return -ENOMEM;
	}
	bochs->fb_base = addr;
	bochs->fb_size = size;

	DRM_INFO("Found bochs VGA, ID 0x%x.\n", id);
	DRM_INFO("Framebuffer size %ld kB @ 0x%lx, %s @ 0x%lx.\n",
		 size / 1024, addr,
		 bochs->ioports ? "ioports" : "mmio",
		 ioaddr);

	if (bochs->mmio && pdev->revision >= 2) {
		qext_size = readl(bochs->mmio + 0x600);
		if (qext_size < 4 || qext_size > iosize)
			goto noext;
		DRM_DEBUG("Found qemu ext regs, size %ld\n", qext_size);
		if (qext_size >= 8) {
#ifdef __BIG_ENDIAN
			writel(0xbebebebe, bochs->mmio + 0x604);
#else
			writel(0x1e1e1e1e, bochs->mmio + 0x604);
#endif
			DRM_DEBUG("  qext endian: 0x%x\n",
				  readl(bochs->mmio + 0x604));
		}
	}

noext:
	return 0;
}

void bochs_hw_fini(struct drm_device *dev)
{
	struct bochs_device *bochs = dev->dev_private;

	if (bochs->mmio)
		iounmap(bochs->mmio);
	if (bochs->ioports)
		release_region(VBE_DISPI_IOPORT_INDEX, 2);
	if (bochs->fb_map)
		iounmap(bochs->fb_map);
	pci_release_regions(dev->pdev);
}

void bochs_hw_setmode(struct bochs_device *bochs,
		      struct drm_display_mode *mode)
{
	bochs->xres = mode->hdisplay;
	bochs->yres = mode->vdisplay;
	bochs->bpp = 32;
	bochs->stride = mode->hdisplay * (bochs->bpp / 8);
	bochs->yres_virtual = bochs->fb_size / bochs->stride;

	DRM_DEBUG_DRIVER("%dx%d @ %d bpp, vy %d\n",
			 bochs->xres, bochs->yres, bochs->bpp,
			 bochs->yres_virtual);

	bochs_vga_writeb(bochs, 0x3c0, 0x20); /* unblank */

	bochs_dispi_write(bochs, VBE_DISPI_INDEX_ENABLE,      0);
	bochs_dispi_write(bochs, VBE_DISPI_INDEX_BPP,         bochs->bpp);
	bochs_dispi_write(bochs, VBE_DISPI_INDEX_XRES,        bochs->xres);
	bochs_dispi_write(bochs, VBE_DISPI_INDEX_YRES,        bochs->yres);
	bochs_dispi_write(bochs, VBE_DISPI_INDEX_BANK,        0);
	bochs_dispi_write(bochs, VBE_DISPI_INDEX_VIRT_WIDTH,  bochs->xres);
	bochs_dispi_write(bochs, VBE_DISPI_INDEX_VIRT_HEIGHT,
			  bochs->yres_virtual);
	bochs_dispi_write(bochs, VBE_DISPI_INDEX_X_OFFSET,    0);
	bochs_dispi_write(bochs, VBE_DISPI_INDEX_Y_OFFSET,    0);

	bochs_dispi_write(bochs, VBE_DISPI_INDEX_ENABLE,
			  VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);
}

void bochs_hw_setbase(struct bochs_device *bochs,
		      int x, int y, u64 addr)
{
	unsigned long offset = (unsigned long)addr +
		y * bochs->stride +
		x * (bochs->bpp / 8);
	int vy = offset / bochs->stride;
	int vx = (offset % bochs->stride) * 8 / bochs->bpp;

	DRM_DEBUG_DRIVER("x %d, y %d, addr %llx -> offset %lx, vx %d, vy %d\n",
			 x, y, addr, offset, vx, vy);
	bochs_dispi_write(bochs, VBE_DISPI_INDEX_X_OFFSET, vx);
	bochs_dispi_write(bochs, VBE_DISPI_INDEX_Y_OFFSET, vy);
}
