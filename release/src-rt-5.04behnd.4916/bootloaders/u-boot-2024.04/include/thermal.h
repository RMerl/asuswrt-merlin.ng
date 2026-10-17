/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *
 * (C) Copyright 2014 Freescale Semiconductor, Inc
 */

#ifndef _THERMAL_H_
#define _THERMAL_H_

struct udevice;

int thermal_get_temp(struct udevice *dev, int *temp);

/**
 * struct dm_thermal_ops - Driver model Thermal operations
 *
 * The uclass interface is implemented by all Thermal devices which use
 * driver model.
 */
struct dm_thermal_ops {
	/**
	 * Get the current temperature
	 *
	 * This must be called before doing any transfers with a Thermal device.
	 * It will enable and initialize any Thermal hardware as necessary.
	 *
	 * @dev:	The Thermal device
	 * @temp:	pointer that returns the measured temperature
	 */
	int (*get_temp)(struct udevice *dev, int *temp);
};

#endif	/* _THERMAL_H_ */
