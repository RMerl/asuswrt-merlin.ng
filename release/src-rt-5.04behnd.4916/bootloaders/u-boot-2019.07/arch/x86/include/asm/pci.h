/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se
 */

#ifndef _PCI_I386_H_
#define _PCI_I386_H_

#include <pci.h>

/* bus mapping constants (used for PCI core initialization) */
#define PCI_REG_ADDR	0xcf8
#define PCI_REG_DATA	0xcfc

#define PCI_CFG_EN	0x80000000

#ifndef __ASSEMBLY__

int pci_x86_read_config(struct udevice *bus, pci_dev_t bdf, uint offset,
			ulong *valuep, enum pci_size_t size);

int pci_x86_write_config(struct udevice *bus, pci_dev_t bdf, uint offset,
			 ulong value, enum pci_size_t size);

/**
 * Assign IRQ number to a PCI device
 *
 * This function assigns IRQ for a PCI device. If the device does not exist
 * or does not require interrupts then this function has no effect.
 *
 * @bus:	PCI bus number
 * @device:	PCI device number
 * @irq:	An array of IRQ numbers that are assigned to INTA through
 *		INTD of this PCI device.
 */
void pci_assign_irqs(int bus, int device, u8 irq[4]);

#endif /* __ASSEMBLY__ */

#endif /* _PCI_I386_H_ */
