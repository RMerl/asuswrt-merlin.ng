/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 * Marek Vasut <marex@denx.de>
 */

#ifndef _DM_DEVICE_INTERNAL_H
#define _DM_DEVICE_INTERNAL_H

#include <dm/ofnode.h>

struct device_node;
struct udevice;

/**
 * device_bind() - Create a device and bind it to a driver
 *
 * Called to set up a new device attached to a driver. The device will either
 * have platdata, or a device tree node which can be used to create the
 * platdata.
 *
 * Once bound a device exists but is not yet active until device_probe() is
 * called.
 *
 * @parent: Pointer to device's parent, under which this driver will exist
 * @drv: Device's driver
 * @name: Name of device (e.g. device tree node name)
 * @platdata: Pointer to data for this device - the structure is device-
 * specific but may include the device's I/O address, etc.. This is NULL for
 * devices which use device tree.
 * @of_offset: Offset of device tree node for this device. This is -1 for
 * devices which don't use device tree.
 * @devp: if non-NULL, returns a pointer to the bound device
 * @return 0 if OK, -ve on error
 */
int device_bind(struct udevice *parent, const struct driver *drv,
		const char *name, void *platdata, int of_offset,
		struct udevice **devp);

int device_bind_ofnode(struct udevice *parent, const struct driver *drv,
		       const char *name, void *platdata, ofnode node,
		       struct udevice **devp);

/**
 * device_bind_with_driver_data() - Create a device and bind it to a driver
 *
 * Called to set up a new device attached to a driver, in the case where the
 * driver was matched to the device by means of a match table that provides
 * driver_data.
 *
 * Once bound a device exists but is not yet active until device_probe() is
 * called.
 *
 * @parent: Pointer to device's parent, under which this driver will exist
 * @drv: Device's driver
 * @name: Name of device (e.g. device tree node name)
 * @driver_data: The driver_data field from the driver's match table.
 * @node: Device tree node for this device. This is invalid for devices which
 * don't use device tree.
 * @devp: if non-NULL, returns a pointer to the bound device
 * @return 0 if OK, -ve on error
 */
int device_bind_with_driver_data(struct udevice *parent,
				 const struct driver *drv, const char *name,
				 ulong driver_data, ofnode node,
				 struct udevice **devp);
/**
 * device_bind_by_name: Create a device and bind it to a driver
 *
 * This is a helper function used to bind devices which do not use device
 * tree.
 *
 * @parent: Pointer to device's parent
 * @pre_reloc_only: If true, bind the driver only if its DM_FLAG_PRE_RELOC flag
 * is set. If false bind the driver always.
 * @info: Name and platdata for this device
 * @devp: if non-NULL, returns a pointer to the bound device
 * @return 0 if OK, -ve on error
 */
int device_bind_by_name(struct udevice *parent, bool pre_reloc_only,
			const struct driver_info *info, struct udevice **devp);

/**
 * device_probe() - Probe a device, activating it
 *
 * Activate a device so that it is ready for use. All its parents are probed
 * first.
 *
 * @dev: Pointer to device to probe
 * @return 0 if OK, -ve on error
 */
int device_probe(struct udevice *dev);

/**
 * device_remove() - Remove a device, de-activating it
 *
 * De-activate a device so that it is no longer ready for use. All its
 * children are deactivated first.
 *
 * @dev: Pointer to device to remove
 * @flags: Flags for selective device removal (DM_REMOVE_...)
 * @return 0 if OK, -ve on error (an error here is normally a very bad thing)
 */
#if CONFIG_IS_ENABLED(DM_DEVICE_REMOVE)
int device_remove(struct udevice *dev, uint flags);
#else
static inline int device_remove(struct udevice *dev, uint flags) { return 0; }
#endif

/**
 * device_unbind() - Unbind a device, destroying it
 *
 * Unbind a device and remove all memory used by it
 *
 * @dev: Pointer to device to unbind
 * @return 0 if OK, -ve on error
 */
#if CONFIG_IS_ENABLED(DM_DEVICE_REMOVE)
int device_unbind(struct udevice *dev);
#else
static inline int device_unbind(struct udevice *dev) { return 0; }
#endif

#if CONFIG_IS_ENABLED(DM_DEVICE_REMOVE)
void device_free(struct udevice *dev);
#else
static inline void device_free(struct udevice *dev) {}
#endif

/**
 * device_chld_unbind() - Unbind all device's children from the device if bound
 *			  to drv
 *
 * On error, the function continues to unbind all children, and reports the
 * first error.
 *
 * @dev:	The device that is to be stripped of its children
 * @drv:	The targeted driver
 * @return 0 on success, -ve on error
 */
#if CONFIG_IS_ENABLED(DM_DEVICE_REMOVE)
int device_chld_unbind(struct udevice *dev, struct driver *drv);
#else
static inline int device_chld_unbind(struct udevice *dev, struct driver *drv)
{
	return 0;
}
#endif

/**
 * device_chld_remove() - Stop all device's children
 * @dev:	The device whose children are to be removed
 * @drv:	The targeted driver
 * @flags:	Flag, if this functions is called in the pre-OS stage
 * @return 0 on success, -ve on error
 */
#if CONFIG_IS_ENABLED(DM_DEVICE_REMOVE)
int device_chld_remove(struct udevice *dev, struct driver *drv,
		       uint flags);
#else
static inline int device_chld_remove(struct udevice *dev, struct driver *drv,
				     uint flags)
{
	return 0;
}
#endif

/**
 * simple_bus_translate() - translate a bus address to a system address
 *
 * This handles the 'ranges' property in a simple bus. It translates the
 * device address @addr to a system address using this property.
 *
 * @dev:	Simple bus device (parent of target device)
 * @addr:	Address to translate
 * @return new address
 */
fdt_addr_t simple_bus_translate(struct udevice *dev, fdt_addr_t addr);

/* Cast away any volatile pointer */
#define DM_ROOT_NON_CONST		(((gd_t *)gd)->dm_root)
#define DM_UCLASS_ROOT_NON_CONST	(((gd_t *)gd)->uclass_root)

/* device resource management */
#ifdef CONFIG_DEVRES

/**
 * devres_release_probe - Release managed resources allocated after probing
 * @dev: Device to release resources for
 *
 * Release all resources allocated for @dev when it was probed or later.
 * This function is called on driver removal.
 */
void devres_release_probe(struct udevice *dev);

/**
 * devres_release_all - Release all managed resources
 * @dev: Device to release resources for
 *
 * Release all resources associated with @dev.  This function is
 * called on driver unbinding.
 */
void devres_release_all(struct udevice *dev);

#else /* ! CONFIG_DEVRES */

static inline void devres_release_probe(struct udevice *dev)
{
}

static inline void devres_release_all(struct udevice *dev)
{
}

#endif /* ! CONFIG_DEVRES */
#endif
