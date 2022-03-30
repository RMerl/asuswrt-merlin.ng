/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __SYSRESET_H
#define __SYSRESET_H

enum sysreset_t {
	SYSRESET_WARM,	/* Reset CPU, keep GPIOs active */
	SYSRESET_COLD,	/* Reset CPU and GPIOs */
	SYSRESET_POWER,	/* Reset PMIC (remove and restore power) */
	SYSRESET_POWER_OFF,	/* Turn off power */

	SYSRESET_COUNT,
};

struct sysreset_ops {
	/**
	 * request() - request a sysreset of the given type
	 *
	 * Note that this function may return before the reset takes effect.
	 *
	 * @type:	Reset type to request
	 * @return -EINPROGRESS if the reset has been started and
	 *		will complete soon, -EPROTONOSUPPORT if not supported
	 *		by this device, 0 if the reset has already happened
	 *		(in which case this method will not actually return)
	 */
	int (*request)(struct udevice *dev, enum sysreset_t type);
	/**
	 * get_status() - get printable reset status information
	 *
	 * @dev:	Device to check
	 * @buf:	Buffer to receive the textual reset information
	 * @size:	Size of the passed buffer
	 * @return 0 if OK, -ve on error
	 */
	int (*get_status)(struct udevice *dev, char *buf, int size);

	/**
	 * get_last() - get information on the last reset
	 *
	 * @dev:	Device to check
	 * @return last reset state (enum sysreset_t) or -ve error
	 */
	int (*get_last)(struct udevice *dev);
};

#define sysreset_get_ops(dev)        ((struct sysreset_ops *)(dev)->driver->ops)

/**
 * sysreset_request() - request a sysreset
 *
 * @type:	Reset type to request
 * @return 0 if OK, -EPROTONOSUPPORT if not supported by this device
 */
int sysreset_request(struct udevice *dev, enum sysreset_t type);

/**
 * sysreset_get_status() - get printable reset status information
 *
 * @dev:	Device to check
 * @buf:	Buffer to receive the textual reset information
 * @size:	Size of the passed buffer
 * @return 0 if OK, -ve on error
 */
int sysreset_get_status(struct udevice *dev, char *buf, int size);

/**
 * sysreset_get_last() - get information on the last reset
 *
 * @dev:	Device to check
 * @return last reset state (enum sysreset_t) or -ve error
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
 * @return -EINPROGRESS if a reset is in progress, -ENOSYS if not available
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
 * @return last reset state (enum sysreset_t) or -ve error
 */
int sysreset_get_last_walk(void);

/**
 * sysreset_walk_halt() - try to reset, otherwise halt
 *
 * This calls sysreset_walk(). If it returns, indicating that reset is not
 * supported, it prints a message and halts.
 */
void sysreset_walk_halt(enum sysreset_t type);

/**
 * reset_cpu() - calls sysreset_walk(SYSRESET_WARM)
 */
void reset_cpu(ulong addr);

#endif
