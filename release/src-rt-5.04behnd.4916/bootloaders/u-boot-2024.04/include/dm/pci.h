/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2019 Google, Inc
 */

#ifndef __DM_PCI_H
#define __DM_PCI_H

struct udevice;

/**
 * pci_get_devfn() - Extract the devfn from fdt_pci_addr of the device
 *
 * Get devfn from fdt_pci_addr of the specified device
 *
 * This returns an int to avoid a dependency on pci.h
 *
 * @dev:	PCI device
 * Return: devfn in bits 15...8 if found (pci_dev_t format), or -ENODEV if not
 *	found
 */
int pci_get_devfn(struct udevice *dev);

/**
 * pci_ofplat_get_devfn() - Get the PCI dev/fn from of-platdata
 *
 * This function is used to obtain a PCI device/function from of-platdata
 * register data. In this case the first cell of the 'reg' property contains
 * the required information.
 *
 * This returns an int to avoid a dependency on pci.h
 *
 * @reg: reg value from dt-plat.c array (first member). This is not a
 *	pointer type, since the caller may use fdt32_t or fdt64_t depending on
 *	the address sizes.
 * Return: device/function for that device (pci_dev_t format)
 */
static inline int pci_ofplat_get_devfn(u32 reg)
{
	return reg & 0xff00;
}

#endif
