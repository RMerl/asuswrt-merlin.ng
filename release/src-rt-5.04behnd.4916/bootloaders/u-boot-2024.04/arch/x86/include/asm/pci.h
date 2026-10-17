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

/**
 * pci_x86_read_config() - Read a configuration value from a device
 *
 * This function can be called before PCI is set up in driver model.
 *
 * @bdf:	PCI device address: bus, device and function -see PCI_BDF()
 * @offset:	Register offset to read
 * @valuep:	Place to put the returned value
 * @size:	Access size
 * Return: 0 if OK, -ve on error
 */
int pci_x86_read_config(pci_dev_t bdf, uint offset, ulong *valuep,
			enum pci_size_t size);

/**
 * pci_bus_write_config() - Write a configuration value to a device
 *
 * This function can be called before PCI is set up in driver model.
 *
 * @bdf:	PCI device address: bus, device and function -see PCI_BDF()
 * @offset:	Register offset to write
 * @value:	Value to write
 * @size:	Access size
 * Return: 0 if OK, -ve on error
 */
int pci_x86_write_config(pci_dev_t bdf, uint offset, ulong value,
			 enum pci_size_t size);

/**
 * pci_bus_clrset_config32() - Update a configuration value for a device
 *
 * The register at @offset is updated to (oldvalue & ~clr) | set. This function
 * can be called before PCI is set up in driver model.
 *
 * @bdf:	PCI device address: bus, device and function -see PCI_BDF()
 * @offset:	Register offset to update
 * @clr:	Bits to clear
 * @set:	Bits to set
 * Return: 0 if OK, -ve on error
 */
int pci_x86_clrset_config(pci_dev_t bdf, uint offset, ulong clr, ulong set,
			  enum pci_size_t size);

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
