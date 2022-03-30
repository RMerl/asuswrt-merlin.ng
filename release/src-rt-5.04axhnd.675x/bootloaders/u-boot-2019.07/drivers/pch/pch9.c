// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <pch.h>

#define GPIO_BASE	0x48
#define IO_BASE		0x4c
#define SBASE_ADDR	0x54

static int pch9_get_spi_base(struct udevice *dev, ulong *sbasep)
{
	uint32_t sbase_addr;

	dm_pci_read_config32(dev, SBASE_ADDR, &sbase_addr);
	*sbasep = sbase_addr & 0xfffffe00;

	return 0;
}

static int pch9_get_gpio_base(struct udevice *dev, u32 *gbasep)
{
	u32 base;

	/*
	 * GPIO_BASE moved to its current offset with ICH6, but prior to
	 * that it was unused (or undocumented). Check that it looks
	 * okay: not all ones or zeros.
	 *
	 * Note we don't need check bit0 here, because the Tunnel Creek
	 * GPIO base address register bit0 is reserved (read returns 0),
	 * while on the Ivybridge the bit0 is used to indicate it is an
	 * I/O space.
	 */
	dm_pci_read_config32(dev, GPIO_BASE, &base);
	if (base == 0x00000000 || base == 0xffffffff) {
		debug("%s: unexpected BASE value\n", __func__);
		return -ENODEV;
	}

	/*
	 * Okay, I guess we're looking at the right device. The actual
	 * GPIO registers are in the PCI device's I/O space, starting
	 * at the offset that we just read. Bit 0 indicates that it's
	 * an I/O address, not a memory address, so mask that off.
	 */
	*gbasep = base & 1 ? base & ~3 : base & ~15;

	return 0;
}

static int pch9_get_io_base(struct udevice *dev, u32 *iobasep)
{
	u32 base;

	dm_pci_read_config32(dev, IO_BASE, &base);
	if (base == 0x00000000 || base == 0xffffffff) {
		debug("%s: unexpected BASE value\n", __func__);
		return -ENODEV;
	}

	*iobasep = base & 1 ? base & ~3 : base & ~15;

	return 0;
}

static const struct pch_ops pch9_ops = {
	.get_spi_base	= pch9_get_spi_base,
	.get_gpio_base	= pch9_get_gpio_base,
	.get_io_base	= pch9_get_io_base,
};

static const struct udevice_id pch9_ids[] = {
	{ .compatible = "intel,pch9" },
	{ }
};

U_BOOT_DRIVER(pch9_drv) = {
	.name		= "intel-pch9",
	.id		= UCLASS_PCH,
	.of_match	= pch9_ids,
	.ops		= &pch9_ops,
};
