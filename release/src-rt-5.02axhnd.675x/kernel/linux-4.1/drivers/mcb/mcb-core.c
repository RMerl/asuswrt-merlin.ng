/*
 * MEN Chameleon Bus.
 *
 * Copyright (C) 2013 MEN Mikroelektronik GmbH (www.men.de)
 * Author: Johannes Thumshirn <johannes.thumshirn@men.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; version 2 of the License.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/idr.h>
#include <linux/mcb.h>

static DEFINE_IDA(mcb_ida);

static const struct mcb_device_id *mcb_match_id(const struct mcb_device_id *ids,
						struct mcb_device *dev)
{
	if (ids) {
		while (ids->device) {
			if (ids->device == dev->id)
				return ids;
			ids++;
		}
	}

	return NULL;
}

static int mcb_match(struct device *dev, struct device_driver *drv)
{
	struct mcb_driver *mdrv = to_mcb_driver(drv);
	struct mcb_device *mdev = to_mcb_device(dev);
	const struct mcb_device_id *found_id;

	found_id = mcb_match_id(mdrv->id_table, mdev);
	if (found_id)
		return 1;

	return 0;
}

static int mcb_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	struct mcb_device *mdev = to_mcb_device(dev);
	int ret;

	ret = add_uevent_var(env, "MODALIAS=mcb:16z%03d", mdev->id);
	if (ret)
		return -ENOMEM;

	return 0;
}

static int mcb_probe(struct device *dev)
{
	struct mcb_driver *mdrv = to_mcb_driver(dev->driver);
	struct mcb_device *mdev = to_mcb_device(dev);
	const struct mcb_device_id *found_id;

	found_id = mcb_match_id(mdrv->id_table, mdev);
	if (!found_id)
		return -ENODEV;

	return mdrv->probe(mdev, found_id);
}

static int mcb_remove(struct device *dev)
{
	struct mcb_driver *mdrv = to_mcb_driver(dev->driver);
	struct mcb_device *mdev = to_mcb_device(dev);

	mdrv->remove(mdev);

	put_device(&mdev->dev);

	return 0;
}

static void mcb_shutdown(struct device *dev)
{
	struct mcb_device *mdev = to_mcb_device(dev);
	struct mcb_driver *mdrv = mdev->driver;

	if (mdrv && mdrv->shutdown)
		mdrv->shutdown(mdev);
}

static struct bus_type mcb_bus_type = {
	.name = "mcb",
	.match = mcb_match,
	.uevent = mcb_uevent,
	.probe = mcb_probe,
	.remove = mcb_remove,
	.shutdown = mcb_shutdown,
};

/**
 * __mcb_register_driver() - Register a @mcb_driver at the system
 * @drv: The @mcb_driver
 * @owner: The @mcb_driver's module
 * @mod_name: The name of the @mcb_driver's module
 *
 * Register a @mcb_driver at the system. Perform some sanity checks, if
 * the .probe and .remove methods are provided by the driver.
 */
int __mcb_register_driver(struct mcb_driver *drv, struct module *owner,
			const char *mod_name)
{
	if (!drv->probe || !drv->remove)
		return -EINVAL;

	drv->driver.owner = owner;
	drv->driver.bus = &mcb_bus_type;
	drv->driver.mod_name = mod_name;

	return driver_register(&drv->driver);
}
EXPORT_SYMBOL_GPL(__mcb_register_driver);

/**
 * mcb_unregister_driver() - Unregister a @mcb_driver from the system
 * @drv: The @mcb_driver
 *
 * Unregister a @mcb_driver from the system.
 */
void mcb_unregister_driver(struct mcb_driver *drv)
{
	driver_unregister(&drv->driver);
}
EXPORT_SYMBOL_GPL(mcb_unregister_driver);

static void mcb_release_dev(struct device *dev)
{
	struct mcb_device *mdev = to_mcb_device(dev);

	mcb_bus_put(mdev->bus);
	kfree(mdev);
}

/**
 * mcb_device_register() - Register a mcb_device
 * @bus: The @mcb_bus of the device
 * @dev: The @mcb_device
 *
 * Register a specific @mcb_device at a @mcb_bus and the system itself.
 */
int mcb_device_register(struct mcb_bus *bus, struct mcb_device *dev)
{
	int ret;
	int device_id;

	device_initialize(&dev->dev);
	dev->dev.bus = &mcb_bus_type;
	dev->dev.parent = bus->dev.parent;
	dev->dev.release = mcb_release_dev;

	device_id = dev->id;
	dev_set_name(&dev->dev, "mcb%d-16z%03d-%d:%d:%d",
		bus->bus_nr, device_id, dev->inst, dev->group, dev->var);

	ret = device_add(&dev->dev);
	if (ret < 0) {
		pr_err("Failed registering device 16z%03d on bus mcb%d (%d)\n",
			device_id, bus->bus_nr, ret);
		goto out;
	}

	return 0;

out:

	return ret;
}
EXPORT_SYMBOL_GPL(mcb_device_register);

/**
 * mcb_alloc_bus() - Allocate a new @mcb_bus
 *
 * Allocate a new @mcb_bus.
 */
struct mcb_bus *mcb_alloc_bus(struct device *carrier)
{
	struct mcb_bus *bus;
	int bus_nr;

	bus = kzalloc(sizeof(struct mcb_bus), GFP_KERNEL);
	if (!bus)
		return ERR_PTR(-ENOMEM);

	bus_nr = ida_simple_get(&mcb_ida, 0, 0, GFP_KERNEL);
	if (bus_nr < 0) {
		kfree(bus);
		return ERR_PTR(bus_nr);
	}

	INIT_LIST_HEAD(&bus->children);
	bus->bus_nr = bus_nr;
	bus->carrier = carrier;
	return bus;
}
EXPORT_SYMBOL_GPL(mcb_alloc_bus);

static int __mcb_devices_unregister(struct device *dev, void *data)
{
	device_unregister(dev);
	return 0;
}

static void mcb_devices_unregister(struct mcb_bus *bus)
{
	bus_for_each_dev(&mcb_bus_type, NULL, NULL, __mcb_devices_unregister);
}
/**
 * mcb_release_bus() - Free a @mcb_bus
 * @bus: The @mcb_bus to release
 *
 * Release an allocated @mcb_bus from the system.
 */
void mcb_release_bus(struct mcb_bus *bus)
{
	mcb_devices_unregister(bus);

	ida_simple_remove(&mcb_ida, bus->bus_nr);

	kfree(bus);
}
EXPORT_SYMBOL_GPL(mcb_release_bus);

/**
 * mcb_bus_put() - Increment refcnt
 * @bus: The @mcb_bus
 *
 * Get a @mcb_bus' ref
 */
struct mcb_bus *mcb_bus_get(struct mcb_bus *bus)
{
	if (bus)
		get_device(&bus->dev);

	return bus;
}
EXPORT_SYMBOL_GPL(mcb_bus_get);

/**
 * mcb_bus_put() - Decrement refcnt
 * @bus: The @mcb_bus
 *
 * Release a @mcb_bus' ref
 */
void mcb_bus_put(struct mcb_bus *bus)
{
	if (bus)
		put_device(&bus->dev);
}
EXPORT_SYMBOL_GPL(mcb_bus_put);

/**
 * mcb_alloc_dev() - Allocate a device
 * @bus: The @mcb_bus the device is part of
 *
 * Allocate a @mcb_device and add bus.
 */
struct mcb_device *mcb_alloc_dev(struct mcb_bus *bus)
{
	struct mcb_device *dev;

	dev = kzalloc(sizeof(struct mcb_device), GFP_KERNEL);
	if (!dev)
		return NULL;

	INIT_LIST_HEAD(&dev->bus_list);
	dev->bus = bus;

	return dev;
}
EXPORT_SYMBOL_GPL(mcb_alloc_dev);

/**
 * mcb_free_dev() - Free @mcb_device
 * @dev: The device to free
 *
 * Free a @mcb_device
 */
void mcb_free_dev(struct mcb_device *dev)
{
	kfree(dev);
}
EXPORT_SYMBOL_GPL(mcb_free_dev);

static int __mcb_bus_add_devices(struct device *dev, void *data)
{
	struct mcb_device *mdev = to_mcb_device(dev);
	int retval;

	if (mdev->is_added)
		return 0;

	retval = device_attach(dev);
	if (retval < 0)
		dev_err(dev, "Error adding device (%d)\n", retval);

	mdev->is_added = true;

	return 0;
}

static int __mcb_bus_add_child(struct device *dev, void *data)
{
	struct mcb_device *mdev = to_mcb_device(dev);
	struct mcb_bus *child;

	BUG_ON(!mdev->is_added);
	child = mdev->subordinate;

	if (child)
		mcb_bus_add_devices(child);

	return 0;
}

/**
 * mcb_bus_add_devices() - Add devices in the bus' internal device list
 * @bus: The @mcb_bus we add the devices
 *
 * Add devices in the bus' internal device list to the system.
 */
void mcb_bus_add_devices(const struct mcb_bus *bus)
{
	bus_for_each_dev(&mcb_bus_type, NULL, NULL, __mcb_bus_add_devices);
	bus_for_each_dev(&mcb_bus_type, NULL, NULL, __mcb_bus_add_child);

}
EXPORT_SYMBOL_GPL(mcb_bus_add_devices);

/**
 * mcb_request_mem() - Request memory
 * @dev: The @mcb_device the memory is for
 * @name: The name for the memory reference.
 *
 * Request memory for a @mcb_device. If @name is NULL the driver name will
 * be used.
 */
struct resource *mcb_request_mem(struct mcb_device *dev, const char *name)
{
	struct resource *mem;
	u32 size;

	if (!name)
		name = dev->dev.driver->name;

	size = resource_size(&dev->mem);

	mem = request_mem_region(dev->mem.start, size, name);
	if (!mem)
		return ERR_PTR(-EBUSY);

	return mem;
}
EXPORT_SYMBOL_GPL(mcb_request_mem);

/**
 * mcb_release_mem() - Release memory requested by device
 * @dev: The @mcb_device that requested the memory
 *
 * Release memory that was prior requested via @mcb_request_mem().
 */
void mcb_release_mem(struct resource *mem)
{
	u32 size;

	size = resource_size(mem);
	release_mem_region(mem->start, size);
}
EXPORT_SYMBOL_GPL(mcb_release_mem);

static int __mcb_get_irq(struct mcb_device *dev)
{
	struct resource *irq = &dev->irq;

	return irq->start;
}

/**
 * mcb_get_irq() - Get device's IRQ number
 * @dev: The @mcb_device the IRQ is for
 *
 * Get the IRQ number of a given @mcb_device.
 */
int mcb_get_irq(struct mcb_device *dev)
{
	struct mcb_bus *bus = dev->bus;

	if (bus->get_irq)
		return bus->get_irq(dev);

	return __mcb_get_irq(dev);
}
EXPORT_SYMBOL_GPL(mcb_get_irq);

static int mcb_init(void)
{
	return bus_register(&mcb_bus_type);
}

static void mcb_exit(void)
{
	bus_unregister(&mcb_bus_type);
}

/* mcb must be initialized after PCI but before the chameleon drivers.
 * That means we must use some initcall between subsys_initcall and
 * device_initcall.
 */
fs_initcall(mcb_init);
module_exit(mcb_exit);

MODULE_DESCRIPTION("MEN Chameleon Bus Driver");
MODULE_AUTHOR("Johannes Thumshirn <johannes.thumshirn@men.de>");
MODULE_LICENSE("GPL v2");
