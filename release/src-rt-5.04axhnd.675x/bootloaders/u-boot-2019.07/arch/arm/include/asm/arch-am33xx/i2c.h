/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012
 * Texas Instruments, <www.ti.com>
 */
#ifndef _I2C_AM33XX_H_
#define _I2C_AM33XX_H_

#include <asm/omap_i2c.h>

#define  I2C_BASE1		0x44E0B000
#define  I2C_BASE2		0x4802A000
#define  I2C_BASE3		0x4819C000

#define I2C_DEFAULT_BASE		I2C_BASE1

#define I2C_IP_CLK			48000000
#define I2C_INTERNAL_SAMPLING_CLK	12000000

#endif /* _I2C_AM33XX_H_ */
