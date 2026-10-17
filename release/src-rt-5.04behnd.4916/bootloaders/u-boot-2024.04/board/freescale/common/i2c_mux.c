// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Copyright 2020-21 NXP
 * Copyright 2021 Microsoft Corporation
 */

#include <common.h>
#include <i2c.h>
#include "i2c_common.h"
#include "i2c_mux.h"

/*
 * A new Kconfig option for something that used to always be built should be
 * “default y”.
 */
#ifdef CONFIG_FSL_USE_PCA9547_MUX

int select_i2c_ch_pca9547(u8 ch, int bus)
{
	int ret;
	DEVICE_HANDLE_T dev;

	/* Open device handle */
	ret = fsl_i2c_get_device(I2C_MUX_PCA_ADDR_PRI, bus, &dev);
	if (ret) {
		printf("PCA: No PCA9547 device found\n");
		return ret;
	}

	ret = I2C_WRITE(dev, 0, &ch, sizeof(ch));
	if (ret) {
		printf("PCA: Unable to select channel %d (%d)\n", (int)ch, ret);
		return ret;
	}

	return 0;
}

#endif
