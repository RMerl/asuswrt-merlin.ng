/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Copyright 2020-21 NXP
 * Copyright 2021 Microsoft Corporation
 */

#ifndef __NXP_I2C_MUX_H__
#define __NXP_I2C_MUX_H__

#ifdef CONFIG_FSL_USE_PCA9547_MUX
int select_i2c_ch_pca9547(u8 ch, int bus);
#endif

#endif
