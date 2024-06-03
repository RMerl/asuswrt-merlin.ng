// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */

#include <bootcount.h>
#include <linux/compiler.h>
#include <i2c.h>

#define BC_MAGIC	0xbc

void bootcount_store(ulong a)
{
	unsigned char buf[3];
	int ret;

	buf[0] = BC_MAGIC;
	buf[1] = (a & 0xff);
	ret = i2c_write(CONFIG_SYS_I2C_RTC_ADDR, CONFIG_SYS_BOOTCOUNT_ADDR,
		  CONFIG_BOOTCOUNT_ALEN, buf, 2);
	if (ret != 0)
		puts("Error writing bootcount\n");
}

ulong bootcount_load(void)
{
	unsigned char buf[3];
	int ret;

	ret = i2c_read(CONFIG_SYS_I2C_RTC_ADDR, CONFIG_SYS_BOOTCOUNT_ADDR,
		       CONFIG_BOOTCOUNT_ALEN, buf, 2);
	if (ret != 0) {
		puts("Error loading bootcount\n");
		return 0;
	}
	if (buf[0] == BC_MAGIC)
		return buf[1];

	bootcount_store(0);

	return 0;
}
