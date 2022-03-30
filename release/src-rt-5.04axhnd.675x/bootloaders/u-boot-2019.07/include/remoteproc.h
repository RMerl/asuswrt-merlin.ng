/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015
 * Texas Instruments Incorporated - http://www.ti.com/
 */

#ifndef _RPROC_H_
#define _RPROC_H_

/*
 * Note: The platform data support is not meant for use with newer
 * platforms. This is meant only for legacy devices. This mode of
 * initialization *will* be eventually removed once all necessary
 * platforms have moved to dm/fdt.
 */
#include <dm/platdata.h>	/* For platform data support - non dt world */

/**
 * enum rproc_mem_type - What type of memory model does the rproc use
 * @RPROC_INTERNAL_MEMORY_MAPPED: Remote processor uses own memory and is memory
 *	mapped to the host processor over an address range.
 *
 * Please note that this is an enumeration of memory model of different types
 * of remote processors. Few of the remote processors do have own internal
 * memories, while others use external memory for instruction and data.
 */
enum rproc_mem_type {
	RPROC_INTERNAL_MEMORY_MAPPED	= 0,
};

/**
 * struct dm_rproc_uclass_pdata - platform data for a CPU
 * @name: Platform-specific way of naming the Remote proc
 * @mem_type: one of 'enum rproc_mem_type'
 * @driver_plat_data: driver specific platform data that may be needed.
 *
 * This can be accessed with dev_get_uclass_platdata() for any UCLASS_REMOTEPROC
 * device.
 *
 */
struct dm_rproc_uclass_pdata {
	const char *name;
	enum rproc_mem_type mem_type;
	void *driver_plat_data;
};

/**
 * struct dm_rproc_ops - Operations that are provided by remote proc driver
 * @init:	Initialize the remoteproc device invoked after probe (optional)
 *		Return 0 on success, -ve error on fail
 * @load:	Load the remoteproc device using data provided(mandatory)
 *		This takes the following additional arguments.
 *			addr- Address of the binary image to be loaded
 *			size- Size of the binary image to be loaded
 *		Return 0 on success, -ve error on fail
 * @start:	Start the remoteproc device (mandatory)
 *		Return 0 on success, -ve error on fail
 * @stop:	Stop the remoteproc device (optional)
 *		Return 0 on success, -ve error on fail
 * @reset:	Reset the remote proc device (optional)
 *		Return 0 on success, -ve error on fail
 * @is_running:	Check if the remote processor is running(optional)
 *		Return 0 on success, 1 if not running, -ve on others errors
 * @ping:	Ping the remote device for basic communication check(optional)
 *		Return 0 on success, 1 if not responding, -ve on other errors
 */
struct dm_rproc_ops {
	int (*init)(struct udevice *dev);
	int (*load)(struct udevice *dev, ulong addr, ulong size);
	int (*start)(struct udevice *dev);
	int (*stop)(struct udevice *dev);
	int (*reset)(struct udevice *dev);
	int (*is_running)(struct udevice *dev);
	int (*ping)(struct udevice *dev);
};

/* Accessor */
#define rproc_get_ops(dev) ((struct dm_rproc_ops *)(dev)->driver->ops)

#ifdef CONFIG_REMOTEPROC
/**
 * rproc_init() - Initialize all bound remote proc devices
 *
 * Return: 0 if all ok, else appropriate error value.
 */
int rproc_init(void);

/**
 * rproc_dev_init() - Initialize a remote proc device based on id
 * @id:		id of the remote processor
 *
 * Return: 0 if all ok, else appropriate error value.
 */
int rproc_dev_init(int id);

/**
 * rproc_is_initialized() - check to see if remoteproc devices are initialized
 *
 * Return: 0 if all devices are initialized, else appropriate error value.
 */
bool rproc_is_initialized(void);

/**
 * rproc_load() - load binary to a remote processor
 * @id:		id of the remote processor
 * @addr:	address in memory where the binary image is located
 * @size:	size of the binary image
 *
 * Return: 0 if all ok, else appropriate error value.
 */
int rproc_load(int id, ulong addr, ulong size);

/**
 * rproc_start() - Start a remote processor
 * @id:		id of the remote processor
 *
 * Return: 0 if all ok, else appropriate error value.
 */
int rproc_start(int id);

/**
 * rproc_stop() - Stop a remote processor
 * @id:		id of the remote processor
 *
 * Return: 0 if all ok, else appropriate error value.
 */
int rproc_stop(int id);

/**
 * rproc_reset() - reset a remote processor
 * @id:		id of the remote processor
 *
 * Return: 0 if all ok, else appropriate error value.
 */
int rproc_reset(int id);

/**
 * rproc_ping() - ping a remote processor to check if it can communicate
 * @id:		id of the remote processor
 *
 * NOTE: this might need communication path available, which is not implemented
 * as part of remoteproc framework - hook on to appropriate bus architecture to
 * do the same
 *
 * Return: 0 if all ok, else appropriate error value.
 */
int rproc_ping(int id);

/**
 * rproc_is_running() - check to see if remote processor is running
 * @id:		id of the remote processor
 *
 * NOTE: this may not involve actual communication capability of the remote
 * processor, but just ensures that it is out of reset and executing code.
 *
 * Return: 0 if all ok, else appropriate error value.
 */
int rproc_is_running(int id);
#else
static inline int rproc_init(void) { return -ENOSYS; }
static inline int rproc_dev_init(int id) { return -ENOSYS; }
static inline bool rproc_is_initialized(void) { return false; }
static inline int rproc_load(int id, ulong addr, ulong size) { return -ENOSYS; }
static inline int rproc_start(int id) { return -ENOSYS; }
static inline int rproc_stop(int id) { return -ENOSYS; }
static inline int rproc_reset(int id) { return -ENOSYS; }
static inline int rproc_ping(int id) { return -ENOSYS; }
static inline int rproc_is_running(int id) { return -ENOSYS; }
#endif

#endif	/* _RPROC_H_ */
