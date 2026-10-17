// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Copyright 2020-21 NXP
 * Copyright 2021 Microsoft Corporation
 */

#include <common.h>
#include <i2c.h>
#include "i2c_common.h"

#if CONFIG_IS_ENABLED(DM_I2C)

/* If DM is in use, retrieve the chip for the specified bus number */
int fsl_i2c_get_device(int address, int bus, DEVICE_HANDLE_T *dev)
{
	int ret = i2c_get_chip_for_busnum(bus, address, 1, dev);

	if (ret)
		printf("I2C: Bus %d has no device with address 0x%02X\n",
		       bus, address);
	return ret;
}

#else

/* Handle is passed directly */
int fsl_i2c_get_device(int address, int bus, DEVICE_HANDLE_T *dev)
{
	*dev = address;
	return 0;
}

#endif
