/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2014 Google, Inc
 */

#ifndef __I2C_EEPROM
#define __I2C_EEPROM

#include <linux/errno.h>

struct udevice;

struct i2c_eeprom_ops {
	int (*read)(struct udevice *dev, int offset, uint8_t *buf, int size);
	int (*write)(struct udevice *dev, int offset, const uint8_t *buf,
		     int size);
	int (*size)(struct udevice *dev);
};

struct i2c_eeprom {
	/* The EEPROM's page size in byte */
	unsigned long pagesize;
	/* The EEPROM's capacity in bytes */
	unsigned long size;
};

#if CONFIG_IS_ENABLED(I2C_EEPROM)
/*
 * i2c_eeprom_read() - read bytes from an I2C EEPROM chip
 *
 * @dev:	Chip to read from
 * @offset:	Offset within chip to start reading
 * @buf:	Place to put data
 * @size:	Number of bytes to read
 *
 * Return: 0 on success, -ve on failure
 */
int i2c_eeprom_read(struct udevice *dev, int offset, uint8_t *buf, int size);

/*
 * i2c_eeprom_write() - write bytes to an I2C EEPROM chip
 *
 * @dev:	Chip to write to
 * @offset:	Offset within chip to start writing
 * @buf:	Buffer containing data to write
 * @size:	Number of bytes to write
 *
 * Return: 0 on success, -ve on failure
 */
int i2c_eeprom_write(struct udevice *dev, int offset, const uint8_t *buf,
		     int size);

/*
 * i2c_eeprom_size() - get size of I2C EEPROM chip
 *
 * @dev:	Chip to query
 *
 * Return: +ve size in bytes on success, -ve on failure
 */
int i2c_eeprom_size(struct udevice *dev);

#else /* !I2C_EEPROM */

static inline int i2c_eeprom_read(struct udevice *dev, int offset, uint8_t *buf,
				  int size)
{
	return -ENOSYS;
}

static inline int i2c_eeprom_write(struct udevice *dev, int offset,
				   const uint8_t *buf, int size)
{
	return -ENOSYS;
}

static inline int i2c_eeprom_size(struct udevice *dev)
{
	return -ENOSYS;
}

#endif /* I2C_EEPROM */

#endif
