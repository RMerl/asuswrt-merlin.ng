/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c), Vaisala Oyj
 */

#ifndef REBOOT_MODE_REBOOT_MODE_H__
#define REBOOT_MODE_REBOOT_MODE_H__

#include <asm/types.h>
#include <dm/device.h>

struct reboot_mode_mode {
	const char *mode_name;
	u32 mode_id;
};

struct reboot_mode_uclass_platdata {
	struct reboot_mode_mode *modes;
	u8 count;
	const char *env_variable;
};

struct reboot_mode_ops {
	/**
	 * get() - get the current reboot mode value
	 *
	 * Returns the current value from the reboot mode backing store.
	 *
	 * @dev:	Device to read from
	 * @rebootmode:	Address to save the current reboot mode value
	 */
	int (*get)(struct udevice *dev, u32 *rebootmode);

	/**
	 * set() - set a reboot mode value
	 *
	 * Sets the value in the reboot mode backing store.
	 *
	 * @dev:	Device to read from
	 * @rebootmode:	New reboot mode value to store
	 */
	int (*set)(struct udevice *dev, u32 rebootmode);
};

/* Access the operations for a reboot mode device */
#define reboot_mode_get_ops(dev) ((struct reboot_mode_ops *)(dev)->driver->ops)

/**
 * dm_reboot_mode_update() - Update the reboot mode env variable.
 *
 * @dev:	Device to read from
 * Return: 0 if OK, -ve on error
 */
int dm_reboot_mode_update(struct udevice *dev);

#endif /* REBOOT_MODE_REBOOT_MODE_H__ */
