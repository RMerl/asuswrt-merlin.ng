/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 */

#ifndef _DM_ROOT_H_
#define _DM_ROOT_H_

struct udevice;

/**
 * dm_root() - Return pointer to the top of the driver tree
 *
 * This function returns pointer to the root node of the driver tree,
 *
 * @return pointer to root device, or NULL if not inited yet
 */
struct udevice *dm_root(void);

struct global_data;
/**
 * dm_fixup_for_gd_move() - Handle global_data moving to a new place
 *
 * The uclass list is part of global_data. Due to the way lists work, moving
 * the list will cause it to become invalid. This function fixes that up so
 * that the uclass list will work correctly.
 */
void dm_fixup_for_gd_move(struct global_data *new_gd);

/**
 * dm_scan_platdata() - Scan all platform data and bind drivers
 *
 * This scans all available platdata and creates drivers for each
 *
 * @pre_reloc_only: If true, bind only drivers with the DM_FLAG_PRE_RELOC
 * flag. If false bind all drivers.
 * @return 0 if OK, -ve on error
 */
int dm_scan_platdata(bool pre_reloc_only);

/**
 * dm_scan_fdt() - Scan the device tree and bind drivers
 *
 * This scans the device tree and creates a driver for each node. Only
 * the top-level subnodes are examined.
 *
 * @blob: Pointer to device tree blob
 * @pre_reloc_only: If true, bind only nodes with special devicetree properties,
 * or drivers with the DM_FLAG_PRE_RELOC flag. If false bind all drivers.
 * @return 0 if OK, -ve on error
 */
int dm_scan_fdt(const void *blob, bool pre_reloc_only);

/**
 * dm_extended_scan_fdt() - Scan the device tree and bind drivers
 *
 * This calls dm_scna_dft() which scans the device tree and creates a driver
 * for each node. the top-level subnodes are examined and also all sub-nodes
 * of "clocks" node.
 *
 * @blob: Pointer to device tree blob
 * @pre_reloc_only: If true, bind only nodes with special devicetree properties,
 * or drivers with the DM_FLAG_PRE_RELOC flag. If false bind all drivers.
 * @return 0 if OK, -ve on error
 */
int dm_extended_scan_fdt(const void *blob, bool pre_reloc_only);

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
 * @return 0 if OK, -ve on error
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
 * @return 0 if OK, -ve on error
 */
int dm_init_and_scan(bool pre_reloc_only);

/**
 * dm_init() - Initialise Driver Model structures
 *
 * This function will initialize roots of driver tree and class tree.
 * This needs to be called before anything uses the DM
 *
 * @of_live:	Enable live device tree
 * @return 0 if OK, -ve on error
 */
int dm_init(bool of_live);

/**
 * dm_uninit - Uninitialise Driver Model structures
 *
 * All devices will be removed and unbound
 * @return 0 if OK, -ve on error
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
 * @return 0 if OK, -ve on error
 */
int dm_remove_devices_flags(uint flags);
#else
static inline int dm_remove_devices_flags(uint flags) { return 0; }
#endif

#endif
