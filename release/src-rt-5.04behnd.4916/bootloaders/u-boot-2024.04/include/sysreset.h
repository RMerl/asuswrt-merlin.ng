/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __SYSRESET_H
#define __SYSRESET_H

struct udevice;

/**
 * enum sysreset_t - system reset types
 */
enum sysreset_t {
	/** @SYSRESET_WARM: reset CPU, keep GPIOs active */
	SYSRESET_WARM,
	/** @SYSRESET_COLD: reset CPU and GPIOs */
	SYSRESET_COLD,
	/** @SYSRESET_POWER: reset PMIC (remove and restore power) */
	SYSRESET_POWER,
	/** @SYSRESET_POWER_OFF: turn off power */
	SYSRESET_POWER_OFF,
	/** @SYSRESET_COUNT: number of available reset types */
	SYSRESET_COUNT,
};

/**
 * struct sysreset_ops - operations of system reset drivers
 */
struct sysreset_ops {
	/**
	 * @request:	request a sysreset of the given type
	 *
	 * Note that this function may return before the reset takes effect.
	 *
	 * @dev:	Device to be used for system reset
	 * @type:	Reset type to request
	 * Return:
	 * -EINPROGRESS if the reset has been started and
	 * will complete soon, -EPROTONOSUPPORT if not supported
	 * by this device, 0 if the reset has already happened
	 * (in which case this method will not actually return)
	 */
	int (*request)(struct udevice *dev, enum sysreset_t type);
	/**
	 * @get_status:	get printable reset status information
	 *
	 * @dev:	Device to check
	 * @buf:	Buffer to receive the textual reset information
	 * @size:	Size of the passed buffer
	 * Return:	0 if OK, -ve on error
	 */
	int (*get_status)(struct udevice *dev, char *buf, int size);

	/**
	 * @get_last:	get information on the last reset
	 *
	 * @dev:	Device to check
	 * Return:	last reset state (enum :enum:`sysreset_t`) or -ve error
	 */
	int (*get_last)(struct udevice *dev);
};

#define sysreset_get_ops(dev)        ((struct sysreset_ops *)(dev)->driver->ops)

/**
 * sysreset_request() - request a sysreset
 *
 * @dev:	Device to be used for system reset
 * @type:	Reset type to request
 * Return:	0 if OK, -EPROTONOSUPPORT if not supported by this device
 */
int sysreset_request(struct udevice *dev, enum sysreset_t type);

/**
 * sysreset_get_status() - get printable reset status information
 *
 * @dev:	Device to check
 * @buf:	Buffer to receive the textual reset information
 * @size:	Size of the passed buffer
 * Return:	 0 if OK, -ve on error
 */
int sysreset_get_status(struct udevice *dev, char *buf, int size);

/**
 * sysreset_get_last() - get information on the last reset
 *
 * @dev:	Device to check
 * Return:	last reset state (enum sysreset_t) or -ve error
 */
int sysreset_get_last(struct udevice *dev);

/**
 * sysreset_walk() - cause a system reset
 *
 * This works through the available sysreset devices until it finds one that can
 * perform a reset. If the provided sysreset type is not available, the next one
 * will be tried.
 *
 * If this function fails to reset, it will display a message and halt
 *
 * @type:	Reset type to request
 * Return:	-EINPROGRESS if a reset is in progress, -ENOSYS if not available
 */
int sysreset_walk(enum sysreset_t type);

/**
 * sysreset_get_last_walk() - get information on the last reset
 *
 * This works through the available sysreset devices until it finds one that can
 * perform a reset. If the provided sysreset type is not available, the next one
 * will be tried.
 *
 * If no device prives the information, this function returns -ENOENT
 *
 * Return:	last reset state (enum sysreset_t) or -ve error
 */
int sysreset_get_last_walk(void);

/**
 * sysreset_walk_halt() - try to reset, otherwise halt
 *
 * This calls sysreset_walk(). If it returns, indicating that reset is not
 * supported, it prints a message and halts.
 *
 * @type:	Reset type to request
 */
void sysreset_walk_halt(enum sysreset_t type);

/**
 * reset_cpu() - calls sysreset_walk(SYSRESET_WARM)
 */
void reset_cpu(void);

/**
 * sysreset_register_wdt() - register a watchdog for use with sysreset
 *
 * This registers the given watchdog timer to be used to reset the system.
 *
 * @dev:	WDT device
 * @return:	0 if OK, -errno if error
 */
int sysreset_register_wdt(struct udevice *dev);

#endif
