/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 */

#ifndef _DM_LISTS_H_
#define _DM_LISTS_H_

#include <dm/ofnode.h>
#include <dm/uclass-id.h>

/**
 * lists_driver_lookup_name() - Return u_boot_driver corresponding to name
 *
 * This function returns a pointer to a driver given its name. This is used
 * for binding a driver given its name and platdata.
 *
 * @name: Name of driver to look up
 * @return pointer to driver, or NULL if not found
 */
struct driver *lists_driver_lookup_name(const char *name);

/**
 * lists_uclass_lookup() - Return uclass_driver based on ID of the class
 * id:		ID of the class
 *
 * This function returns the pointer to uclass_driver, which is the class's
 * base structure based on the ID of the class. Returns NULL on error.
 */
struct uclass_driver *lists_uclass_lookup(enum uclass_id id);

/**
 * lists_bind_drivers() - search for and bind all drivers to parent
 *
 * This searches the U_BOOT_DEVICE() structures and creates new devices for
 * each one. The devices will have @parent as their parent.
 *
 * @parent: parent device (root)
 * @pre_reloc_only: If true, bind only drivers with the DM_FLAG_PRE_RELOC flag.
 * If false bind all drivers.
 */
int lists_bind_drivers(struct udevice *parent, bool pre_reloc_only);

/**
 * lists_bind_fdt() - bind a device tree node
 *
 * This creates a new device bound to the given device tree node, with
 * @parent as its parent.
 *
 * @parent: parent device (root)
 * @node: device tree node to bind
 * @devp: if non-NULL, returns a pointer to the bound device
 * @pre_reloc_only: If true, bind only nodes with special devicetree properties,
 * or drivers with the DM_FLAG_PRE_RELOC flag. If false bind all drivers.
 * @return 0 if device was bound, -EINVAL if the device tree is invalid,
 * other -ve value on error
 */
int lists_bind_fdt(struct udevice *parent, ofnode node, struct udevice **devp,
		   bool pre_reloc_only);

/**
 * device_bind_driver() - bind a device to a driver
 *
 * This binds a new device to a driver.
 *
 * @parent:	Parent device
 * @drv_name:	Name of driver to attach to this parent
 * @dev_name:	Name of the new device thus created
 * @devp:	If non-NULL, returns the newly bound device
 */
int device_bind_driver(struct udevice *parent, const char *drv_name,
		       const char *dev_name, struct udevice **devp);

/**
 * device_bind_driver_to_node() - bind a device to a driver for a node
 *
 * This binds a new device to a driver for a given device tree node. This
 * should only be needed if the node lacks a compatible strings.
 *
 * @parent:	Parent device
 * @drv_name:	Name of driver to attach to this parent
 * @dev_name:	Name of the new device thus created
 * @node:	Device tree node
 * @devp:	If non-NULL, returns the newly bound device
 */
int device_bind_driver_to_node(struct udevice *parent, const char *drv_name,
			       const char *dev_name, ofnode node,
			       struct udevice **devp);

#endif
