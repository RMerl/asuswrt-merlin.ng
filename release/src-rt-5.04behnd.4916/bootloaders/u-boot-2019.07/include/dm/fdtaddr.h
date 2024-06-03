/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 * Marek Vasut <marex@denx.de>
 */

#ifndef _DM_FDTADDR_H
#define _DM_FDTADDR_H

#include <fdtdec.h>

struct udevice;

/**
 * devfdt_get_addr() - Get the reg property of a device
 *
 * @dev: Pointer to a device
 *
 * @return addr
 */
fdt_addr_t devfdt_get_addr(struct udevice *dev);

/**
 * devfdt_get_addr_ptr() - Return pointer to the address of the reg property
 *                      of a device
 *
 * @dev: Pointer to a device
 *
 * @return Pointer to addr, or NULL if there is no such property
 */
void *devfdt_get_addr_ptr(struct udevice *dev);

/**
 * devfdt_remap_addr() - Return pointer to the memory-mapped I/O address
 *                           of the reg property of a device
 *
 * @dev: Pointer to a device
 *
 * @return Pointer to addr, or NULL if there is no such property
 */
void *devfdt_remap_addr(struct udevice *dev);

/**
 * devfdt_remap_addr_index() - Return indexed pointer to the memory-mapped
 *                                 I/O address of the reg property of a device
 * @index: the 'reg' property can hold a list of <addr, size> pairs
 *	   and @index is used to select which one is required
 *
 * @dev: Pointer to a device
 *
 * @return Pointer to addr, or NULL if there is no such property
 */
void *devfdt_remap_addr_index(struct udevice *dev, int index);

/**
 * devfdt_remap_addr_name() - Get the reg property of a device, indexed by
 *                            name, as a memory-mapped I/O pointer
 * @name: the 'reg' property can hold a list of <addr, size> pairs, with the
 *	  'reg-names' property providing named-based identification. @index
 *	  indicates the value to search for in 'reg-names'.
 *
 * @dev: Pointer to a device
 *
 * @return Pointer to addr, or NULL if there is no such property
 */
void *devfdt_remap_addr_name(struct udevice *dev, const char *name);

/**
 * devfdt_map_physmem() - Read device address from reg property of the
 *                     device node and map the address into CPU address
 *                     space.
 *
 * @dev: Pointer to device
 * @size: size of the memory to map
 *
 * @return  mapped address, or NULL if the device does not have reg
 *          property.
 */
void *devfdt_map_physmem(struct udevice *dev, unsigned long size);

/**
 * devfdt_get_addr_index() - Get the indexed reg property of a device
 *
 * @dev: Pointer to a device
 * @index: the 'reg' property can hold a list of <addr, size> pairs
 *	   and @index is used to select which one is required
 *
 * @return addr
 */
fdt_addr_t devfdt_get_addr_index(struct udevice *dev, int index);

/**
 * devfdt_get_addr_size_index() - Get the indexed reg property of a device
 *
 * Returns the address and size specified in the 'reg' property of a device.
 *
 * @dev: Pointer to a device
 * @index: the 'reg' property can hold a list of <addr, size> pairs
 *	   and @index is used to select which one is required
 * @size: Pointer to size varible - this function returns the size
 *        specified in the 'reg' property here
 *
 * @return addr
 */
fdt_addr_t devfdt_get_addr_size_index(struct udevice *dev, int index,
				   fdt_size_t *size);

/**
 * devfdt_get_addr_name() - Get the reg property of a device, indexed by name
 *
 * @dev: Pointer to a device
 * @name: the 'reg' property can hold a list of <addr, size> pairs, with the
 *	  'reg-names' property providing named-based identification. @index
 *	  indicates the value to search for in 'reg-names'.
 *
 * @return addr
 */
fdt_addr_t devfdt_get_addr_name(struct udevice *dev, const char *name);

#endif
