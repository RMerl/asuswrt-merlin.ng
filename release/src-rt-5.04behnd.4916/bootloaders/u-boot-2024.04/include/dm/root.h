/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 */

#ifndef _DM_ROOT_H_
#define _DM_ROOT_H_

#include <dm/tag.h>

struct udevice;

/* Head of the uclass list if CONFIG_OF_PLATDATA_INST is enabled */
extern struct list_head uclass_head;

/**
 * struct dm_stats - Information about driver model memory usage
 *
 * @total_size: All data
 * @dev_count: Number of devices
 * @dev_size: Size of all devices (just the struct udevice)
 * @dev_name_size: Bytes used by device names
 * @uc_count: Number of uclasses
 * @uc_size: Size of all uclasses (just the struct uclass)
 * @tag_count: Number of tags
 * @tag_size: Bytes used by all tags
 * @uc_attach_count: Number of uclasses with attached data (priv)
 * @uc_attach_size: Total size of that attached data
 * @attach_count_total: Total number of attached data items for all udevices and
 *	uclasses
 * @attach_size_total: Total number of bytes of attached data
 * @attach_count: Number of devices with attached, for each type
 * @attach_size: Total number of bytes of attached data, for each type
 */
struct dm_stats {
	int total_size;
	int dev_count;
	int dev_size;
	int dev_name_size;
	int uc_count;
	int uc_size;
	int tag_count;
	int tag_size;
	int uc_attach_count;
	int uc_attach_size;
	int attach_count_total;
	int attach_size_total;
	int attach_count[DM_TAG_ATTACH_COUNT];
	int attach_size[DM_TAG_ATTACH_COUNT];
};

/**
 * dm_root() - Return pointer to the top of the driver tree
 *
 * This function returns pointer to the root node of the driver tree,
 *
 * Return: pointer to root device, or NULL if not inited yet
 */
struct udevice *dm_root(void);

struct global_data;
/**
 * dm_fixup_for_gd_move() - Handle global_data moving to a new place
 *
 * @new_gd: Pointer to the new global data
 *
 * The uclass list is part of global_data. Due to the way lists work, moving
 * the list will cause it to become invalid. This function fixes that up so
 * that the uclass list will work correctly.
 */
void dm_fixup_for_gd_move(struct global_data *new_gd);

/**
 * dm_scan_plat() - Scan all platform data and bind drivers
 *
 * This scans all available plat and creates drivers for each
 *
 * @pre_reloc_only: If true, bind only drivers with the DM_FLAG_PRE_RELOC
 * flag. If false bind all drivers.
 * Return: 0 if OK, -ve on error
 */
int dm_scan_plat(bool pre_reloc_only);

/**
 * dm_scan_fdt() - Scan the device tree and bind drivers
 *
 * This scans the device tree and creates a driver for each node. Only
 * the top-level subnodes are examined.
 *
 * @pre_reloc_only: If true, bind only nodes with special devicetree properties,
 * or drivers with the DM_FLAG_PRE_RELOC flag. If false bind all drivers.
 * Return: 0 if OK, -ve on error
 */
int dm_scan_fdt(bool pre_reloc_only);

/**
 * dm_extended_scan() - Scan the device tree and bind drivers
 *
 * This calls dm_scna_dft() which scans the device tree and creates a driver
 * for each node. the top-level subnodes are examined and also all sub-nodes
 * of "clocks" node.
 *
 * @pre_reloc_only: If true, bind only nodes with special devicetree properties,
 * or drivers with the DM_FLAG_PRE_RELOC flag. If false bind all drivers.
 * Return: 0 if OK, -ve on error
 */
int dm_extended_scan(bool pre_reloc_only);

/**
 * dm_scan_other() - Scan for other devices
 *
 * Some devices may not be visible to Driver Model. This weak function can
 * be provided by boards which wish to create their own devices
 * programmaticaly. They should do this by calling device_bind() on each
 * device.
 *
 * @pre_reloc_only: If true, bind only nodes with special devicetree properties,
 * or drivers with the DM_FLAG_PRE_RELOC flag. If false bind all drivers.
 * Return: 0 if OK, -ve on error
 */
int dm_scan_other(bool pre_reloc_only);

/**
 * dm_init_and_scan() - Initialise Driver Model structures and scan for devices
 *
 * This function initialises the roots of the driver tree and uclass trees,
 * then scans and binds available devices from platform data and the FDT.
 * This calls dm_init() to set up Driver Model structures.
 *
 * @pre_reloc_only: If true, bind only nodes with special devicetree properties,
 * or drivers with the DM_FLAG_PRE_RELOC flag. If false bind all drivers.
 * Return: 0 if OK, -ve on error
 */
int dm_init_and_scan(bool pre_reloc_only);

/**
 * dm_init() - Initialise Driver Model structures
 *
 * This function will initialize roots of driver tree and class tree.
 * This needs to be called before anything uses the DM
 *
 * @of_live:	Enable live device tree
 * Return: 0 if OK, -ve on error
 */
int dm_init(bool of_live);

/**
 * dm_uninit - Uninitialise Driver Model structures
 *
 * All devices will be removed and unbound
 * Return: 0 if OK, -ve on error
 */
int dm_uninit(void);

#if CONFIG_IS_ENABLED(DM_DEVICE_REMOVE)
/**
 * dm_remove_devices_flags - Call remove function of all drivers with
 *                           specific removal flags set to selectively
 *                           remove drivers
 *
 * All devices with the matching flags set will be removed
 *
 * @flags: Flags for selective device removal
 * Return: 0 if OK, -ve on error
 */
int dm_remove_devices_flags(uint flags);
#else
static inline int dm_remove_devices_flags(uint flags) { return 0; }
#endif

/**
 * dm_get_stats() - Get some stats for driver mode
 *
 * @device_countp: Returns total number of devices that are bound
 * @uclass_countp: Returns total number of uclasses in use
 */
void dm_get_stats(int *device_countp, int *uclass_countp);

/**
 * dm_get_mem() - Get stats on memory usage in driver model
 *
 * @stats: Place to put the information
 */
void dm_get_mem(struct dm_stats *stats);

#endif
