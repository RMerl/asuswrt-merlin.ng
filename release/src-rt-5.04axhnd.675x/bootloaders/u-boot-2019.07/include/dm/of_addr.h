/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Taken from Linux v4.9 drivers/of/address.c
 *
 * Modified for U-Boot
 * Copyright (c) 2017 Google, Inc
 */

#ifndef _DM_OF_ADDR_H
#define _DM_OF_ADDR_H

/**
 * of_translate_address() - translate a device-tree address to a CPU address
 *
 * Translate an address from the device-tree into a CPU physical address,
 * this walks up the tree and applies the various bus mappings on the  way.
 *
 * Note: We consider that crossing any level with #size-cells == 0 to mean
 * that translation is impossible (that is we are not dealing with a value
 * that can be mapped to a cpu physical address). This is not really specified
 * that way, but this is traditionally the way IBM at least do things
 *
 * @np: node to check
 * @in_addr: pointer to input address
 * @return translated address or OF_BAD_ADDR on error
 */
u64 of_translate_address(const struct device_node *no, const __be32 *in_addr);

/**
 * of_get_address() - obtain an address from a node
 *
 * Extract an address from a node, returns the region size and the address
 * space flags too. The PCI version uses a BAR number instead of an absolute
 * index.
 *
 * @np: Node to check
 * @index: Index of address to read (0 = first)
 * @size: place to put size on success
 * @flags: place to put flags on success
 * @return pointer to address which can be read
 */
const __be32 *of_get_address(const struct device_node *no, int index,
			     u64 *size, unsigned int *flags);

struct resource;

/**
 * of_address_to_resource() - translate device tree address to resource
 *
 * Note that if your address is a PIO address, the conversion will fail if
 * the physical address can't be internally converted to an IO token with
 * pci_address_to_pio(), that is because it's either called to early or it
 * can't be matched to any host bridge IO space
 *
 * @np: node to check
 * @index: index of address to read (0 = first)
 * @r: place to put resource information
 * @return 0 if OK, -ve on error
 */
int of_address_to_resource(const struct device_node *no, int index,
			   struct resource *r);

#endif
