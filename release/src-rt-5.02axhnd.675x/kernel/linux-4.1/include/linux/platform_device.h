/*
 * platform_device.h - generic, centralized driver model
 *
 * Copyright (c) 2001-2003 Patrick Mochel <mochel@osdl.org>
 *
 * This file is released under the GPLv2
 *
 * See Documentation/driver-model/ for more information.
 */

#ifndef _PLATFORM_DEVICE_H_
#define _PLATFORM_DEVICE_H_

#include <linux/device.h>
#include <linux/mod_devicetable.h>

#define PLATFORM_DEVID_NONE	(-1)
#define PLATFORM_DEVID_AUTO	(-2)

struct mfd_cell;

struct platform_device {
	const char	*name;
	int		id;
	bool		id_auto;
	struct device	dev;
	u32		num_resources;
	struct resource	*resource;

	const struct platform_device_id	*id_entry;
	char *driver_override; /* Driver name to force a match */

	/* MFD cell pointer */
	struct mfd_cell *mfd_cell;

	/* arch specific additions */
	struct pdev_archdata	archdata;
};

#define platform_get_device_id(pdev)	((pdev)->id_entry)

#define to_platform_device(x) container_of((x), struct platform_device, dev)

extern int platform_device_register(struct platform_device *);
extern void platform_device_unregister(struct platform_device *);

extern struct bus_type platform_bus_type;
extern struct device platform_bus;

extern void arch_setup_pdev_archdata(struct platform_device *);
extern struct resource *platform_get_resource(struct platform_device *,
					      unsigned int, unsigned int);
extern int platform_get_irq(struct platform_device *, unsigned int);
extern struct resource *platform_get_resource_byname(struct platform_device *,
						     unsigned int,
						     const char *);
extern int platform_get_irq_byname(struct platform_device *, const char *);
extern int platform_add_devices(struct platform_device **, int);

struct platform_device_info {
		struct device *parent;
		struct fwnode_handle *fwnode;

		const char *name;
		int id;

		const struct resource *res;
		unsigned int num_res;

		const void *data;
		size_t size_data;
		u64 dma_mask;
};
extern struct platform_device *platform_device_register_full(
		const struct platform_device_info *pdevinfo);

/**
 * platform_device_register_resndata - add a platform-level device with
 * resources and platform-specific data
 *
 * @parent: parent device for the device we're adding
 * @name: base name of the device we're adding
 * @id: instance id
 * @res: set of resources that needs to be allocated for the device
 * @num: number of resources
 * @data: platform specific data for this platform device
 * @size: size of platform specific data
 *
 * Returns &struct platform_device pointer on success, or ERR_PTR() on error.
 */
static inline struct platform_device *platform_device_register_resndata(
		struct device *parent, const char *name, int id,
		const struct resource *res, unsigned int num,
		const void *data, size_t size) {

	struct platform_device_info pdevinfo = {
		.parent = parent,
		.name = name,
		.id = id,
		.res = res,
		.num_res = num,
		.data = data,
		.size_data = size,
		.dma_mask = 0,
	};

	return platform_device_register_full(&pdevinfo);
}

/**
 * platform_device_register_simple - add a platform-level device and its resources
 * @name: base name of the device we're adding
 * @id: instance id
 * @res: set of resources that needs to be allocated for the device
 * @num: number of resources
 *
 * This function creates a simple platform device that requires minimal
 * resource and memory management. Canned release function freeing memory
 * allocated for the device allows drivers using such devices to be
 * unloaded without waiting for the last reference to the device to be
 * dropped.
 *
 * This interface is primarily intended for use with legacy drivers which
 * probe hardware directly.  Because such drivers create sysfs device nodes
 * themselves, rather than letting system infrastructure handle such device
 * enumeration tasks, they don't fully conform to the Linux driver model.
 * In particular, when such drivers are built as modules, they can't be
 * "hotplugged".
 *
 * Returns &struct platform_device pointer on success, or ERR_PTR() on error.
 */
static inline struct platform_device *platform_device_register_simple(
		const char *name, int id,
		const struct resource *res, unsigned int num)
{
	return platform_device_register_resndata(NULL, name, id,
			res, num, NULL, 0);
}

/**
 * platform_device_register_data - add a platform-level device with platform-specific data
 * @parent: parent device for the device we're adding
 * @name: base name of the device we're adding
 * @id: instance id
 * @data: platform specific data for this platform device
 * @size: size of platform specific data
 *
 * This function creates a simple platform device that requires minimal
 * resource and memory management. Canned release function freeing memory
 * allocated for the device allows drivers using such devices to be
 * unloaded without waiting for the last reference to the device to be
 * dropped.
 *
 * Returns &struct platform_device pointer on success, or ERR_PTR() on error.
 */
static inline struct platform_device *platform_device_register_data(
		struct device *parent, const char *name, int id,
		const void *data, size_t size)
{
	return platform_device_register_resndata(parent, name, id,
			NULL, 0, data, size);
}

extern struct platform_device *platform_device_alloc(const char *name, int id);
extern int platform_device_add_resources(struct platform_device *pdev,
					 const struct resource *res,
					 unsigned int num);
extern int platform_device_add_data(struct platform_device *pdev,
				    const void *data, size_t size);
extern int platform_device_add(struct platform_device *pdev);
extern void platform_device_del(struct platform_device *pdev);
extern void platform_device_put(struct platform_device *pdev);

struct platform_driver {
	int (*probe)(struct platform_device *);
	int (*remove)(struct platform_device *);
	void (*shutdown)(struct platform_device *);
	int (*suspend)(struct platform_device *, pm_message_t state);
	int (*resume)(struct platform_device *);
	struct device_driver driver;
	const struct platform_device_id *id_table;
	bool prevent_deferred_probe;
};

#define to_platform_driver(drv)	(container_of((drv), struct platform_driver, \
				 driver))

/*
 * use a macro to avoid include chaining to get THIS_MODULE
 */
#define platform_driver_register(drv) \
	__platform_driver_register(drv, THIS_MODULE)
extern int __platform_driver_register(struct platform_driver *,
					struct module *);
extern void platform_driver_unregister(struct platform_driver *);

/* non-hotpluggable platform devices may use this so that probe() and
 * its support may live in __init sections, conserving runtime memory.
 */
#define platform_driver_probe(drv, probe) \
	__platform_driver_probe(drv, probe, THIS_MODULE)
extern int __platform_driver_probe(struct platform_driver *driver,
		int (*probe)(struct platform_device *), struct module *module);

static inline void *platform_get_drvdata(const struct platform_device *pdev)
{
	return dev_get_drvdata(&pdev->dev);
}

static inline void platform_set_drvdata(struct platform_device *pdev,
					void *data)
{
	dev_set_drvdata(&pdev->dev, data);
}

/* module_platform_driver() - Helper macro for drivers that don't do
 * anything special in module init/exit.  This eliminates a lot of
 * boilerplate.  Each module may only use this macro once, and
 * calling it replaces module_init() and module_exit()
 */
#define module_platform_driver(__platform_driver) \
	module_driver(__platform_driver, platform_driver_register, \
			platform_driver_unregister)

/* module_platform_driver_probe() - Helper macro for drivers that don't do
 * anything special in module init/exit.  This eliminates a lot of
 * boilerplate.  Each module may only use this macro once, and
 * calling it replaces module_init() and module_exit()
 */
#define module_platform_driver_probe(__platform_driver, __platform_probe) \
static int __init __platform_driver##_init(void) \
{ \
	return platform_driver_probe(&(__platform_driver), \
				     __platform_probe);    \
} \
module_init(__platform_driver##_init); \
static void __exit __platform_driver##_exit(void) \
{ \
	platform_driver_unregister(&(__platform_driver)); \
} \
module_exit(__platform_driver##_exit);

#define platform_create_bundle(driver, probe, res, n_res, data, size) \
	__platform_create_bundle(driver, probe, res, n_res, data, size, THIS_MODULE)
extern struct platform_device *__platform_create_bundle(
	struct platform_driver *driver, int (*probe)(struct platform_device *),
	struct resource *res, unsigned int n_res,
	const void *data, size_t size, struct module *module);

/* early platform driver interface */
struct early_platform_driver {
	const char *class_str;
	struct platform_driver *pdrv;
	struct list_head list;
	int requested_id;
	char *buffer;
	int bufsize;
};

#define EARLY_PLATFORM_ID_UNSET -2
#define EARLY_PLATFORM_ID_ERROR -3

extern int early_platform_driver_register(struct early_platform_driver *epdrv,
					  char *buf);
extern void early_platform_add_devices(struct platform_device **devs, int num);

static inline int is_early_platform_device(struct platform_device *pdev)
{
	return !pdev->dev.driver;
}

extern void early_platform_driver_register_all(char *class_str);
extern int early_platform_driver_probe(char *class_str,
				       int nr_probe, int user_only);
extern void early_platform_cleanup(void);

#define early_platform_init(class_string, platdrv)		\
	early_platform_init_buffer(class_string, platdrv, NULL, 0)

#ifndef MODULE
#define early_platform_init_buffer(class_string, platdrv, buf, bufsiz)	\
static __initdata struct early_platform_driver early_driver = {		\
	.class_str = class_string,					\
	.buffer = buf,							\
	.bufsize = bufsiz,						\
	.pdrv = platdrv,						\
	.requested_id = EARLY_PLATFORM_ID_UNSET,			\
};									\
static int __init early_platform_driver_setup_func(char *buffer)	\
{									\
	return early_platform_driver_register(&early_driver, buffer);	\
}									\
early_param(class_string, early_platform_driver_setup_func)
#else /* MODULE */
#define early_platform_init_buffer(class_string, platdrv, buf, bufsiz)	\
static inline char *early_platform_driver_setup_func(void)		\
{									\
	return bufsiz ? buf : NULL;					\
}
#endif /* MODULE */

#ifdef CONFIG_SUSPEND
extern int platform_pm_suspend(struct device *dev);
extern int platform_pm_resume(struct device *dev);
#else
#define platform_pm_suspend		NULL
#define platform_pm_resume		NULL
#endif

#ifdef CONFIG_HIBERNATE_CALLBACKS
extern int platform_pm_freeze(struct device *dev);
extern int platform_pm_thaw(struct device *dev);
extern int platform_pm_poweroff(struct device *dev);
extern int platform_pm_restore(struct device *dev);
#else
#define platform_pm_freeze		NULL
#define platform_pm_thaw		NULL
#define platform_pm_poweroff		NULL
#define platform_pm_restore		NULL
#endif

#ifdef CONFIG_PM_SLEEP
#define USE_PLATFORM_PM_SLEEP_OPS \
	.suspend = platform_pm_suspend, \
	.resume = platform_pm_resume, \
	.freeze = platform_pm_freeze, \
	.thaw = platform_pm_thaw, \
	.poweroff = platform_pm_poweroff, \
	.restore = platform_pm_restore,
#else
#define USE_PLATFORM_PM_SLEEP_OPS
#endif

#endif /* _PLATFORM_DEVICE_H_ */
