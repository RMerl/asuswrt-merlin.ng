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
 * Return: Address, or FDT_ADDR_T_NONE if there is no such property
 */
fdt_addr_t devfdt_get_addr(const struct udevice *dev);

/**
 * devfdt_get_addr_ptr() - Return pointer to the address of the reg property
 *                      of a device
 *
 * @dev: Pointer to a device
 *
 * Return: Pointer to addr, or NULL if there is no such property
 */
void *devfdt_get_addr_ptr(const struct udevice *dev);

/**
 * devfdt_remap_addr() - Return pointer to the memory-mapped I/O address
 *                           of the reg property of a device
 *
 * @dev: Pointer to a device
 *
 * Return: Pointer to addr, or NULL if there is no such property
 */
void *devfdt_remap_addr(const struct udevice *dev);

/**
 * devfdt_remap_addr_index() - Return indexed pointer to the memory-mapped
 *                                 I/O address of the reg property of a device
 * @index: the 'reg' property can hold a list of <addr, size> pairs
 *	   and @index is used to select which one is required
 *
 * @dev: Pointer to a device
 *
 * Return: Pointer to addr, or NULL if there is no such property
 */
void *devfdt_remap_addr_index(const struct udevice *dev, int index);

/**
 * devfdt_remap_addr_name() - Get the reg property of a device, indexed by
 *                            name, as a memory-mapped I/O pointer
 * @name: the 'reg' property can hold a list of <addr, size> pairs, with the
 *	  'reg-names' property providing named-based identification. @name
 *	  indicates the value to search for in 'reg-names'.
 *
 * @dev: Pointer to a device
 *
 * Return: Pointer to addr, or NULL if there is no such property
 */
void *devfdt_remap_addr_name(const struct udevice *dev, const char *name);

/**
 * devfdt_map_physmem() - Read device address from reg property of the
 *                     device node and map the address into CPU address
 *                     space.
 *
 * @dev: Pointer to device
 * @size: size of the memory to map
 *
 * Return: mapped address, or NULL if the device does not have reg property.
 */
void *devfdt_map_physmem(const struct udevice *dev, unsigned long size);

/**
 * devfdt_get_addr_index() - Get the indexed reg property of a device
 *
 * @dev: Pointer to a device
 * @index: the 'reg' property can hold a list of <addr, size> pairs
 *	   and @index is used to select which one is required
 *
 * Return: Address, or FDT_ADDR_T_NONE if there is no such property
 */
fdt_addr_t devfdt_get_addr_index(const struct udevice *dev, int index);

/**
 * devfdt_get_addr_index_ptr() - Return indexed pointer to the address of the
 *                               reg property of a device
 *
 * @dev: Pointer to a device
 * @index: the 'reg' property can hold a list of <addr, size> pairs
 *	   and @index is used to select which one is required
 *
 * Return: Pointer to addr, or NULL if there is no such property
 */
void *devfdt_get_addr_index_ptr(const struct udevice *dev, int index);

/**
 * devfdt_get_addr_size_index() - Get the indexed reg property of a device
 *
 * Returns the address and size specified in the 'reg' property of a device.
 *
 * @dev: Pointer to a device
 * @index: the 'reg' property can hold a list of <addr, size> pairs
 *	   and @index is used to select which one is required
 * @size: Pointer to size variable - this function returns the size
 *        specified in the 'reg' property here
 *
 * Return: Address, or FDT_ADDR_T_NONE if there is no such property
 */
fdt_addr_t devfdt_get_addr_size_index(const struct udevice *dev, int index,
				      fdt_size_t *size);

/**
 * devfdt_get_addr_size_index_ptr() - Return indexed pointer to the address of the
 *                                    reg property of a device
 *
 * @dev: Pointer to a device
 * @index: the 'reg' property can hold a list of <addr, size> pairs
 *	   and @index is used to select which one is required
 * @size: Pointer to size variable - this function returns the size
 *        specified in the 'reg' property here
 *
 * Return: Pointer to addr, or NULL if there is no such property
 */
void *devfdt_get_addr_size_index_ptr(const struct udevice *dev, int index,
				     fdt_size_t *size);

/**
 * devfdt_get_addr_name() - Get the reg property of a device, indexed by name
 *
 * @dev: Pointer to a device
 * @name: the 'reg' property can hold a list of <addr, size> pairs, with the
 *	  'reg-names' property providing named-based identification. @name
 *	  indicates the value to search for in 'reg-names'.
 *
 * Return: Address, or FDT_ADDR_T_NONE if there is no such property
 */
fdt_addr_t devfdt_get_addr_name(const struct udevice *dev, const char *name);

/**
 * devfdt_get_addr_name_ptr() - Get the reg property of a device as a pointer,
 *                              indexed by name
 *
 * @dev: Pointer to a device
 * @name: the 'reg' property can hold a list of <addr, size> pairs, with the
 *	  'reg-names' property providing named-based identification. @name
 *	  indicates the value to search for in 'reg-names'.
 *
 * Return: Pointer to addr, or NULL if there is no such property
 */
void *devfdt_get_addr_name_ptr(const struct udevice *dev, const char *name);

/**
 * devfdt_get_addr_size_name() - Get the reg property and its size for a device,
 *				 indexed by name
 *
 * Returns the address and size specified in the 'reg' property of a device.
 *
 * @dev: Pointer to a device
 * @name: the 'reg' property can hold a list of <addr, size> pairs, with the
 *	  'reg-names' property providing named-based identification. @name
 *	  indicates the value to search for in 'reg-names'.
 * @size: Pointer to size variable - this function returns the size
 *        specified in the 'reg' property here
 *
 * Return: Address, or FDT_ADDR_T_NONE if there is no such property
 */
fdt_addr_t devfdt_get_addr_size_name(const struct udevice *dev,
				     const char *name, fdt_size_t *size);

/**
 * devfdt_get_addr_size_name_ptr() - Get the reg property for a device as a
 *                                   pointer, indexed by name
 *
 * Returns the address and size specified in the 'reg' property of a device.
 *
 * @dev: Pointer to a device
 * @name: the 'reg' property can hold a list of <addr, size> pairs, with the
 *	  'reg-names' property providing named-based identification. @name
 *	  indicates the value to search for in 'reg-names'.
 * @size: Pointer to size variable - this function returns the size
 *        specified in the 'reg' property here
 *
 * Return: Pointer to addr, or NULL if there is no such property
 */
void *devfdt_get_addr_size_name_ptr(const struct udevice *dev,
				    const char *name, fdt_size_t *size);

/**
 * devfdt_get_addr_pci() - Read an address and handle PCI address translation
 *
 * @dev: Device to read from
 * @sizep: If non-NULL, returns size of address space
 * Return: address or FDT_ADDR_T_NONE if not found
 */
fdt_addr_t devfdt_get_addr_pci(const struct udevice *dev, fdt_size_t *sizep);

#endif
