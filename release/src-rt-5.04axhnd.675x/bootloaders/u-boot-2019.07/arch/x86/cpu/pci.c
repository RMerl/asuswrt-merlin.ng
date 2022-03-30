// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2008,2009
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <pci.h>
#include <asm/io.h>
#include <asm/pci.h>

int pci_x86_read_config(struct udevice *bus, pci_dev_t bdf, uint offset,
			ulong *valuep, enum pci_size_t size)
{
	outl(bdf | (offset & 0xfc) | PCI_CFG_EN, PCI_REG_ADDR);
	switch (size) {
	case PCI_SIZE_8:
		*valuep = inb(PCI_REG_DATA + (offset & 3));
		break;
	case PCI_SIZE_16:
		*valuep = inw(PCI_REG_DATA + (offset & 2));
		break;
	case PCI_SIZE_32:
		*valuep = inl(PCI_REG_DATA);
		break;
	}

	return 0;
}

int pci_x86_write_config(struct udevice *bus, pci_dev_t bdf, uint offset,
			 ulong value, enum pci_size_t size)
{
	outl(bdf | (offset & 0xfc) | PCI_CFG_EN, PCI_REG_ADDR);
	switch (size) {
	case PCI_SIZE_8:
		outb(value, PCI_REG_DATA + (offset & 3));
		break;
	case PCI_SIZE_16:
		outw(value, PCI_REG_DATA + (offset & 2));
		break;
	case PCI_SIZE_32:
		outl(value, PCI_REG_DATA);
		break;
	}

	return 0;
}

void pci_assign_irqs(int bus, int device, u8 irq[4])
{
	pci_dev_t bdf;
	int func;
	u16 vendor;
	u8 pin, line;

	for (func = 0; func < 8; func++) {
		bdf = PCI_BDF(bus, device, func);
		pci_read_config16(bdf, PCI_VENDOR_ID, &vendor);
		if (vendor == 0xffff || vendor == 0x0000)
			continue;

		pci_read_config8(bdf, PCI_INTERRUPT_PIN, &pin);

		/* PCI spec says all values except 1..4 are reserved */
		if ((pin < 1) || (pin > 4))
			continue;

		line = irq[pin - 1];
		if (!line)
			continue;

		debug("Assigning IRQ %d to PCI device %d.%x.%d (INT%c)\n",
		      line, bus, device, func, 'A' + pin - 1);

		pci_write_config8(bdf, PCI_INTERRUPT_LINE, line);
	}
}
