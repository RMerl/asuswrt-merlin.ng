// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * I2C test
 *
 * For verifying the I2C bus, a full I2C bus scanning is performed.
 *
 * #ifdef CONFIG_SYS_POST_I2C_ADDRS
 *   The test is considered as passed if all the devices and only the devices
 *   in the list are found.
 *   #ifdef CONFIG_SYS_POST_I2C_IGNORES
 *     Ignore devices listed in CONFIG_SYS_POST_I2C_IGNORES.  These devices
 *     are optional or not vital to board functionality.
 *   #endif
 * #else [ ! CONFIG_SYS_POST_I2C_ADDRS ]
 *   The test is considered as passed if any I2C device is found.
 * #endif
 */

#include <common.h>
#include <post.h>
#include <i2c.h>

#if CONFIG_POST & CONFIG_SYS_POST_I2C

static int i2c_ignore_device(unsigned int chip)
{
#ifdef CONFIG_SYS_POST_I2C_IGNORES
	const unsigned char i2c_ignore_list[] = CONFIG_SYS_POST_I2C_IGNORES;
	int i;

	for (i = 0; i < sizeof(i2c_ignore_list); i++)
		if (i2c_ignore_list[i] == chip)
			return 1;
#endif

	return 0;
}

int i2c_post_test (int flags)
{
	unsigned int i;
#ifndef CONFIG_SYS_POST_I2C_ADDRS
	/* Start at address 1, address 0 is the general call address */
	for (i = 1; i < 128; i++) {
		if (i2c_ignore_device(i))
			continue;
		if (i2c_probe (i) == 0)
			return 0;
	}

	/* No devices found */
	return -1;
#else
	unsigned int ret  = 0;
	int j;
	unsigned char i2c_addr_list[] = CONFIG_SYS_POST_I2C_ADDRS;

	/* Start at address 1, address 0 is the general call address */
	for (i = 1; i < 128; i++) {
		if (i2c_ignore_device(i))
			continue;
		if (i2c_probe(i) != 0)
			continue;

		for (j = 0; j < sizeof(i2c_addr_list); ++j) {
			if (i == i2c_addr_list[j]) {
				i2c_addr_list[j] = 0xff;
				break;
			}
		}

		if (j == sizeof(i2c_addr_list)) {
			ret = -1;
			post_log("I2C: addr %02x not expected\n", i);
		}
	}

	for (i = 0; i < sizeof(i2c_addr_list); ++i) {
		if (i2c_addr_list[i] == 0xff)
			continue;
		if (i2c_ignore_device(i2c_addr_list[i]))
			continue;
		post_log("I2C: addr %02x did not respond\n", i2c_addr_list[i]);
		ret = -1;
	}

	return ret;
#endif
}

#endif /* CONFIG_POST & CONFIG_SYS_POST_I2C */
