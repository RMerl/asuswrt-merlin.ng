// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011
 * Dirk Eibach,  Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 */

/*
 * Driver for NXP's pca9698 40 bit I2C gpio expander
 */

#include <common.h>
#include <i2c.h>
#include <linux/errno.h>
#include <pca9698.h>

/*
 * The pca9698 registers
 */

#define PCA9698_REG_INPUT		0x00
#define PCA9698_REG_OUTPUT		0x08
#define PCA9698_REG_POLARITY		0x10
#define PCA9698_REG_CONFIG		0x18

#define PCA9698_BUFFER_SIZE		5
#define PCA9698_GPIO_COUNT		40

static int pca9698_read40(u8 addr, u8 offset, u8 *buffer)
{
	u8 command = offset | 0x80;  /* autoincrement */

	return i2c_read(addr, command, 1, buffer, PCA9698_BUFFER_SIZE);
}

static int pca9698_write40(u8 addr, u8 offset, u8 *buffer)
{
	u8 command = offset | 0x80;  /* autoincrement */

	return i2c_write(addr, command, 1, buffer, PCA9698_BUFFER_SIZE);
}

static void pca9698_set_bit(unsigned gpio, u8 *buffer, unsigned value)
{
	unsigned byte = gpio / 8;
	unsigned bit = gpio % 8;

	if (value)
		buffer[byte] |= (1 << bit);
	else
		buffer[byte] &= ~(1 << bit);
}

int pca9698_request(unsigned gpio, const char *label)
{
	if (gpio >= PCA9698_GPIO_COUNT)
		return -EINVAL;

	return 0;
}

void pca9698_free(unsigned gpio)
{
}

int pca9698_direction_input(u8 addr, unsigned gpio)
{
	u8 data[PCA9698_BUFFER_SIZE];
	int res;

	res = pca9698_read40(addr, PCA9698_REG_CONFIG, data);
	if (res)
		return res;

	pca9698_set_bit(gpio, data, 1);

	return pca9698_write40(addr, PCA9698_REG_CONFIG, data);
}

int pca9698_direction_output(u8 addr, unsigned gpio, int value)
{
	u8 data[PCA9698_BUFFER_SIZE];
	int res;

	res = pca9698_set_value(addr, gpio, value);
	if (res)
		return res;

	res = pca9698_read40(addr, PCA9698_REG_CONFIG, data);
	if (res)
		return res;

	pca9698_set_bit(gpio, data, 0);

	return pca9698_write40(addr, PCA9698_REG_CONFIG, data);
}

int pca9698_get_value(u8 addr, unsigned gpio)
{
	unsigned config_byte = gpio / 8;
	unsigned config_bit = gpio % 8;
	unsigned value;
	u8 data[PCA9698_BUFFER_SIZE];
	int res;

	res = pca9698_read40(addr, PCA9698_REG_INPUT, data);
	if (res)
		return -1;

	value = data[config_byte] & (1 << config_bit);

	return !!value;
}

int pca9698_set_value(u8 addr, unsigned gpio, int value)
{
	u8 data[PCA9698_BUFFER_SIZE];
	int res;

	res = pca9698_read40(addr, PCA9698_REG_OUTPUT, data);
	if (res)
		return res;

	pca9698_set_bit(gpio, data, value);

	return pca9698_write40(addr, PCA9698_REG_OUTPUT, data);
}
