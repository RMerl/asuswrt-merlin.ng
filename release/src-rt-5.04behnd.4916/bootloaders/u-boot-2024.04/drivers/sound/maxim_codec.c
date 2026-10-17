// SPDX-License-Identifier: GPL-2.0
/*
 * maxim_codec.c -- MAXIM CODEC Common driver
 *
 * Copyright 2011 Maxim Integrated Products
 */

#include <common.h>
#include <div64.h>
#include <i2c.h>
#include <i2s.h>
#include <log.h>
#include <sound.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include "maxim_codec.h"

/*
 * Writes value to a device register through i2c
 *
 * @param priv	Private data for driver
 * @param reg	reg number to be write
 * @param data	data to be writen to the above registor
 *
 * Return:	int value 1 for change, 0 for no change or negative error code.
 */
int maxim_i2c_write(struct maxim_priv *priv, unsigned int reg,
		    unsigned char data)
{
	debug("%s: Write Addr : 0x%02X, Data :  0x%02X\n",
	      __func__, reg, data);
	return dm_i2c_write(priv->dev, reg, &data, 1);
}

/*
 * Read a value from a device register through i2c
 *
 * @param priv	Private data for driver
 * @param reg	reg number to be read
 * @param data	address of read data to be stored
 *
 * Return:	int value 0 for success, -1 in case of error.
 */
unsigned int maxim_i2c_read(struct maxim_priv *priv, unsigned int reg,
			    unsigned char *data)
{
	int ret;

	return dm_i2c_read(priv->dev, reg, data, 1);
	if (ret != 0) {
		debug("%s: Error while reading register %#04x\n",
		      __func__, reg);
		return -1;
	}

	return 0;
}

/*
 * update device register bits through i2c
 *
 * @param priv	Private data for driver
 * @param reg	codec register
 * @param mask	register mask
 * @param value	new value
 *
 * Return: int value 0 for success, non-zero error code.
 */
int maxim_bic_or(struct maxim_priv *priv, unsigned int reg, unsigned char mask,
		 unsigned char value)
{
	int change, ret = 0;
	unsigned char old, new;

	if (maxim_i2c_read(priv, reg, &old) != 0)
		return -1;
	new = (old & ~mask) | (value & mask);
	change  = (old != new) ? 1 : 0;
	if (change)
		ret = maxim_i2c_write(priv, reg, new);
	if (ret < 0)
		return ret;

	return change;
}
