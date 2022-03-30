/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 */

#ifndef _DM_UCLASS_H
#define _DM_UCLASS_H

#include <dm/ofnode.h>
#include <dm/uclass-id.h>
#include <linker_lists.h>
#include <linux/list.h>

/**
 * struct uclass - a U-Boot drive class, collecting together similar drivers
 *
 * A uclass provides an interface to a particular function, which is
 * implemented by one or more drivers. Every driver belongs to a uclass even
 * if it is the only driver in that uclass. An example uclass is GPIO, which
 * provides the ability to change read inputs, set and clear outputs, etc.
 * There may be drivers for on-chip SoC GPIO banks, I2C GPIO expanders and
 * PMIC IO lines, all made available in a unified way through the uclass.
 *
 * @priv: Private data for this uclass
 * @uc_drv: The driver for the uclass itself, not to be confused with a
 * 'struct driver'
 * @dev_head: List of devices in this uclass (devices are attached to their
 * uclass when their bind method is called)
 * @sibling_node: Next uclass in the linked list of uclasses
 */
struct uclass {
	void *priv;
	struct uclass_driver *uc_drv;
	struct list_head dev_head;
	struct list_head sibling_node;
};

struct driver;
struct udevice;

/* Members of this uclass sequence themselves with aliases */
#define DM_UC_FLAG_SEQ_ALIAS			(1 << 0)

/* Same as DM_FLAG_ALLOC_PRIV_DMA */
#define DM_UC_FLAG_ALLOC_PRIV_DMA		(1 << 5)

/**
 * struct uclass_driver - Driver for the uclass
 *
 * A uclass_driver provides a consistent interface to a set of related
 * drivers.
 *
 * @name: Name of uclass driver
 * @id: ID number of this uclass
 * @post_bind: Called after a new device is bound to this uclass
 * @pre_unbind: Called before a device is unbound from this uclass
 * @pre_probe: Called before a new device is probed
 * @post_probe: Called after a new device is probed
 * @pre_remove: Called before a device is removed
 * @child_post_bind: Called after a child is bound to a device in this uclass
 * @child_pre_probe: Called before a child in this uclass is probed
 * @child_post_probe: Called after a child in this uclass is probed
 * @init: Called to set up the uclass
 * @destroy: Called to destroy the uclass
 * @priv_auto_alloc_size: If non-zero this is the size of the private data
 * to be allocated in the uclass's ->priv pointer. If zero, then the uclass
 * driver is responsible for allocating any data required.
 * @per_device_auto_alloc_size: Each device can hold private data owned
 * by the uclass. If required this will be automatically allocated if this
 * value is non-zero.
 * @per_device_platdata_auto_alloc_size: Each device can hold platform data
 * owned by the uclass as 'dev->uclass_platdata'. If the value is non-zero,
 * then this will be automatically allocated.
 * @per_child_auto_alloc_size: Each child device (of a parent in this
 * uclass) can hold parent data for the device/uclass. This value is only
 * used as a fallback if this member is 0 in the driver.
 * @per_child_platdata_auto_alloc_size: A bus likes to store information about
 * its children. If non-zero this is the size of this data, to be allocated
 * in the child device's parent_platdata pointer. This value is only used as
 * a fallback if this member is 0 in the driver.
 * @ops: Uclass operations, providing the consistent interface to devices
 * within the uclass.
 * @flags: Flags for this uclass (DM_UC_...)
 */
struct uclass_driver {
	const char *name;
	enum uclass_id id;
	int (*post_bind)(struct udevice *dev);
	int (*pre_unbind)(struct udevice *dev);
	int (*pre_probe)(struct udevice *dev);
	int (*post_probe)(struct udevice *dev);
	int (*pre_remove)(struct udevice *dev);
	int (*child_post_bind)(struct udevice *dev);
	int (*child_pre_probe)(struct udevice *dev);
	int (*child_post_probe)(struct udevice *dev);
	int (*init)(struct uclass *class);
	int (*destroy)(struct uclass *class);
	int priv_auto_alloc_size;
	int per_device_auto_alloc_size;
	int per_device_platdata_auto_alloc_size;
	int per_child_auto_alloc_size;
	int per_child_platdata_auto_alloc_size;
	const void *ops;
	uint32_t flags;
};

/* Declare a new uclass_driver */
#define UCLASS_DRIVER(__name)						\
	ll_entry_declare(struct uclass_driver, __name, uclass)

/**
 * uclass_get() - Get a uclass based on an ID, creating it if needed
 *
 * Every uclass is identified by an ID, a number from 0 to n-1 where n is
 * the number of uclasses. This function allows looking up a uclass by its
 * ID.
 *
 * @key: ID to look up
 * @ucp: Returns pointer to uclass (there is only one per ID)
 * @return 0 if OK, -ve on error
 */
int uclass_get(enum uclass_id key, struct uclass **ucp);

/**
 * uclass_get_name() - Get the name of a uclass driver
 *
 * @id: ID to look up
 * @returns the name of the uclass driver for that ID, or NULL if none
 */
const char *uclass_get_name(enum uclass_id id);

/**
 * uclass_get_by_name() - Look up a uclass by its driver name
 *
 * @name: Name to look up
 * @returns the associated uclass ID, or UCLASS_INVALID if not found
 */
enum uclass_id uclass_get_by_name(const char *name);

/**
 * uclass_get_device() - Get a uclass device based on an ID and index
 *
 * The device is probed to activate it ready for use.
 *
 * @id: ID to look up
 * @index: Device number within that uclass (0=first)
 * @devp: Returns pointer to device (there is only one per for each ID)
 * @return 0 if OK, -ve on error
 */
int uclass_get_device(enum uclass_id id, int index, struct udevice **devp);

/**
 * uclass_get_device_by_name() - Get a uclass device by its name
 *
 * This searches the devices in the uclass for one with the exactly given name.
 *
 * The device is probed to activate it ready for use.
 *
 * @id: ID to look up
 * @name: name of a device to get
 * @devp: Returns pointer to device (the first one with the name)
 * @return 0 if OK, -ve on error
 */
int uclass_get_device_by_name(enum uclass_id id, const char *name,
			      struct udevice **devp);

/**
 * uclass_get_device_by_seq() - Get a uclass device based on an ID and sequence
 *
 * If an active device has this sequence it will be returned. If there is no
 * such device then this will check for a device that is requesting this
 * sequence.
 *
 * The device is probed to activate it ready for use.
 *
 * @id: ID to look up
 * @seq: Sequence number to find (0=first)
 * @devp: Returns pointer to device (there is only one for each seq)
 * @return 0 if OK, -ve on error
 */
int uclass_get_device_by_seq(enum uclass_id id, int seq, struct udevice **devp);

/**
 * uclass_get_device_by_of_offset() - Get a uclass device by device tree node
 *
 * This searches the devices in the uclass for one attached to the given
 * device tree node.
 *
 * The device is probed to activate it ready for use.
 *
 * @id: ID to look up
 * @node: Device tree offset to search for (if -ve then -ENODEV is returned)
 * @devp: Returns pointer to device (there is only one for each node)
 * @return 0 if OK, -ve on error
 */
int uclass_get_device_by_of_offset(enum uclass_id id, int node,
				   struct udevice **devp);

/**
 * uclass_get_device_by_ofnode() - Get a uclass device by device tree node
 *
 * This searches the devices in the uclass for one attached to the given
 * device tree node.
 *
 * The device is probed to activate it ready for use.
 *
 * @id: ID to look up
 * @np: Device tree node to search for (if NULL then -ENODEV is returned)
 * @devp: Returns pointer to device (there is only one for each node)
 * @return 0 if OK, -ve on error
 */
int uclass_get_device_by_ofnode(enum uclass_id id, ofnode node,
				struct udevice **devp);

/**
 * uclass_get_device_by_phandle_id() - Get a uclass device by phandle id
 *
 * This searches the devices in the uclass for one with the given phandle id.
 *
 * The device is probed to activate it ready for use.
 *
 * @id: uclass ID to look up
 * @phandle_id: the phandle id to look up
 * @devp: Returns pointer to device (there is only one for each node)
 * @return 0 if OK, -ENODEV if there is no device match the phandle, other
 *	-ve on error
 */
int uclass_get_device_by_phandle_id(enum uclass_id id, uint phandle_id,
				    struct udevice **devp);

/**
 * uclass_get_device_by_phandle() - Get a uclass device by phandle
 *
 * This searches the devices in the uclass for one with the given phandle.
 *
 * The device is probed to activate it ready for use.
 *
 * @id: uclass ID to look up
 * @parent: Parent device containing the phandle pointer
 * @name: Name of property in the parent device node
 * @devp: Returns pointer to device (there is only one for each node)
 * @return 0 if OK, -ENOENT if there is no @name present in the node, other
 *	-ve on error
 */
int uclass_get_device_by_phandle(enum uclass_id id, struct udevice *parent,
				 const char *name, struct udevice **devp);

/**
 * uclass_get_device_by_driver() - Get a uclass device for a driver
 *
 * This searches the devices in the uclass for one that uses the given
 * driver. Use DM_GET_DRIVER(name) for the @drv argument, where 'name' is
 * the driver name - as used in U_BOOT_DRIVER(name).
 *
 * The device is probed to activate it ready for use.
 *
 * @id: ID to look up
 * @drv: Driver to look for
 * @devp: Returns pointer to the first device with that driver
 * @return 0 if OK, -ve on error
 */
int uclass_get_device_by_driver(enum uclass_id id, const struct driver *drv,
				struct udevice **devp);

/**
 * uclass_first_device() - Get the first device in a uclass
 *
 * The device returned is probed if necessary, and ready for use
 *
 * This function is useful to start iterating through a list of devices which
 * are functioning correctly and can be probed.
 *
 * @id: Uclass ID to look up
 * @devp: Returns pointer to the first device in that uclass if no error
 * occurred, or NULL if there is no first device, or an error occurred with
 * that device.
 * @return 0 if OK (found or not found), other -ve on error
 */
int uclass_first_device(enum uclass_id id, struct udevice **devp);

/**
 * uclass_first_device_err() - Get the first device in a uclass
 *
 * The device returned is probed if necessary, and ready for use
 *
 * @id: Uclass ID to look up
 * @devp: Returns pointer to the first device in that uclass, or NULL if none
 * @return 0 if found, -ENODEV if not found, other -ve on error
 */
int uclass_first_device_err(enum uclass_id id, struct udevice **devp);

/**
 * uclass_next_device() - Get the next device in a uclass
 *
 * The device returned is probed if necessary, and ready for use
 *
 * This function is useful to start iterating through a list of devices which
 * are functioning correctly and can be probed.
 *
 * @devp: On entry, pointer to device to lookup. On exit, returns pointer
 * to the next device in the uclass if no error occurred, or NULL if there is
 * no next device, or an error occurred with that next device.
 * @return 0 if OK (found or not found), other -ve on error
 */
int uclass_next_device(struct udevice **devp);

/**
 * uclass_next_device_err() - Get the next device in a uclass
 *
 * The device returned is probed if necessary, and ready for use
 *
 * @devp: On entry, pointer to device to lookup. On exit, returns pointer
 * to the next device in the uclass if no error occurred, or -ENODEV if
 * there is no next device.
 * @return 0 if found, -ENODEV if not found, other -ve on error
 */
int uclass_next_device_err(struct udevice **devp);

/**
 * uclass_first_device_check() - Get the first device in a uclass
 *
 * The device returned is probed if necessary, and ready for use
 *
 * This function is useful to start iterating through a list of devices which
 * are functioning correctly and can be probed.
 *
 * @id: Uclass ID to look up
 * @devp: Returns pointer to the first device in that uclass, or NULL if there
 * is no first device
 * @return 0 if OK (found or not found), other -ve on error. If an error occurs
 * it is still possible to move to the next device.
 */
int uclass_first_device_check(enum uclass_id id, struct udevice **devp);

/**
 * uclass_next_device_check() - Get the next device in a uclass
 *
 * The device returned is probed if necessary, and ready for use
 *
 * This function is useful to start iterating through a list of devices which
 * are functioning correctly and can be probed.
 *
 * @devp: On entry, pointer to device to lookup. On exit, returns pointer
 * to the next device in the uclass if any
 * @return 0 if OK (found or not found), other -ve on error. If an error occurs
 * it is still possible to move to the next device.
 */
int uclass_next_device_check(struct udevice **devp);

/**
 * uclass_resolve_seq() - Resolve a device's sequence number
 *
 * On entry dev->seq is -1, and dev->req_seq may be -1 (to allocate a
 * sequence number automatically, or >= 0 to select a particular number.
 * If the requested sequence number is in use, then this device will
 * be allocated another one.
 *
 * Note that the device's seq value is not changed by this function.
 *
 * @dev: Device for which to allocate sequence number
 * @return sequence number allocated, or -ve on error
 */
int uclass_resolve_seq(struct udevice *dev);

/**
 * uclass_foreach_dev() - Helper function to iteration through devices
 *
 * This creates a for() loop which works through the available devices in
 * a uclass in order from start to end.
 *
 * @pos: struct udevice * to hold the current device. Set to NULL when there
 * are no more devices.
 * @uc: uclass to scan
 */
#define uclass_foreach_dev(pos, uc)	\
	list_for_each_entry(pos, &uc->dev_head, uclass_node)

/**
 * uclass_foreach_dev_safe() - Helper function to safely iteration through devs
 *
 * This creates a for() loop which works through the available devices in
 * a uclass in order from start to end. Inside the loop, it is safe to remove
 * @pos if required.
 *
 * @pos: struct udevice * to hold the current device. Set to NULL when there
 * are no more devices.
 * @next: struct udevice * to hold the next next
 * @uc: uclass to scan
 */
#define uclass_foreach_dev_safe(pos, next, uc)	\
	list_for_each_entry_safe(pos, next, &uc->dev_head, uclass_node)

/**
 * uclass_foreach_dev_probe() - Helper function to iteration through devices
 * of given uclass
 *
 * This creates a for() loop which works through the available devices in
 * a uclass in order from start to end. Devices are probed if necessary,
 * and ready for use.
 *
 * @id: Uclass ID
 * @dev: struct udevice * to hold the current device. Set to NULL when there
 * are no more devices.
 */
#define uclass_foreach_dev_probe(id, dev)	\
	for (int _ret = uclass_first_device_err(id, &dev); !_ret && dev; \
	     _ret = uclass_next_device_err(&dev))

#endif
