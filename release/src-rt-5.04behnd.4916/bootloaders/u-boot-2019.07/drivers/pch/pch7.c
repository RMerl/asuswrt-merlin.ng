// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <pch.h>

#define GPIO_BASE	0x44
#define BIOS_CTRL	0xd8

static int pch7_get_spi_base(struct udevice *dev, ulong *sbasep)
{
	u32 rcba;

	dm_pci_read_config32(dev, PCH_RCBA, &rcba);
	/* Bits 31-14 are the base address, 13-1 are reserved, 0 is enable */
	rcba = rcba & 0xffffc000;
	*sbasep = rcba + 0x3020;

	return 0;
}

static int pch7_set_spi_protect(struct udevice *dev, bool protect)
{
	uint8_t bios_cntl;

	/* Adjust the BIOS write protect to dis/allow write commands */
	dm_pci_read_config8(dev, BIOS_CTRL, &bios_cntl);
	if (protect)
		bios_cntl &= ~BIOS_CTRL_BIOSWE;
	else
		bios_cntl |= BIOS_CTRL_BIOSWE;
	dm_pci_write_config8(dev, BIOS_CTRL, bios_cntl);

	return 0;
}

static int pch7_get_gpio_base(struct udevice *dev, u32 *gbasep)
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

static const struct pch_ops pch7_ops = {
	.get_spi_base	= pch7_get_spi_base,
	.set_spi_protect = pch7_set_spi_protect,
	.get_gpio_base	= pch7_get_gpio_base,
};

static const struct udevice_id pch7_ids[] = {
	{ .compatible = "intel,pch7" },
	{ }
};

U_BOOT_DRIVER(pch7_drv) = {
	.name		= "intel-pch7",
	.id		= UCLASS_PCH,
	.of_match	= pch7_ids,
	.ops		= &pch7_ops,
};
