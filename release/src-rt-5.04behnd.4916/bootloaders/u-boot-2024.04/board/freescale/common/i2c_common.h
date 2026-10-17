/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Copyright 2020-21 NXP
 * Copyright 2021 Microsoft Corporation
 */

#ifndef __NXP_I2C_COMMON_H__
#define __NXP_I2C_COMMON_H__

/* Common functionality shared by the I2C drivers for VID and the mux. */
#if CONFIG_IS_ENABLED(DM_I2C)
#define DEVICE_HANDLE_T struct udevice *

#define I2C_READ(dev, register, data, length) \
	dm_i2c_read(dev, register, data, length)
#define I2C_WRITE(dev, register, data, length) \
	dm_i2c_write(dev, register, data, length)
#else
#define DEVICE_HANDLE_T int

#define I2C_READ(dev, register, data, length) \
	i2c_read(dev, register, 1, data, length)
#define I2C_WRITE(dev, register, data, length) \
	i2c_write(dev, register, 1, data, length)
#endif

int fsl_i2c_get_device(int address, int bus, DEVICE_HANDLE_T *dev);

#endif
