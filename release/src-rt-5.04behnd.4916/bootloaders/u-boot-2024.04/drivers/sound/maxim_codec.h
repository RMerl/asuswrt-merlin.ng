/* SPDX-License-Identifier: GPL-2.0 */
/*
 * maxim_codec.h -- MAXIM codec common interface file
 *
 * Copyright (C) 2013 Samsung Electronics
 * D Krishna Mohan <krishna.md@samsung.com>
 */

#ifndef __MAXIM_COMMON_H__
#define __MAXIM_COMMON_H__

enum maxim_codec_type {
	MAX98095,
	MAX98090,
};

struct maxim_priv {
	enum maxim_codec_type devtype;
	unsigned int sysclk;
	unsigned int rate;
	unsigned int fmt;
	struct udevice *dev;
};

#define MAXIM_AUDIO_I2C_BUS		7
#define MAXIM_AUDIO_I2C_REG_98095	0x22

#define MAXIM_AUDIO_I2C_REG		MAXIM_AUDIO_I2C_REG_98095

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
		    unsigned char data);

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
			    unsigned char *data);

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
		 unsigned char value);

#endif /* __MAXIM_COMMON_H__ */
