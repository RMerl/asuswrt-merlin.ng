// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Microchip
 *		      Wenyou Yang <wenyou.yang@microchip.com>
 */

#include <common.h>
#include <dm.h>
#include <eeprom.h>
#include <env.h>
#include <i2c_eeprom.h>
#include <net.h>
#include <netdev.h>

int at91_set_ethaddr(int offset)
{
	const int ETH_ADDR_LEN = 6;
	unsigned char ethaddr[ETH_ADDR_LEN];
	const char *ETHADDR_NAME = "ethaddr";
	struct udevice *dev;
	int ret;

	if (env_get(ETHADDR_NAME))
		return 0;

	ret = uclass_first_device_err(UCLASS_I2C_EEPROM, &dev);
	if (ret)
		return ret;

	ret = i2c_eeprom_read(dev, offset, ethaddr, 6);
	if (ret)
		return ret;

	if (is_valid_ethaddr(ethaddr))
		eth_env_set_enetaddr(ETHADDR_NAME, ethaddr);

	return 0;
}

/* this function will set eth1addr from a second eeprom, if available */
int at91_set_eth1addr(int offset)
{
	const int ETH_ADDR_LEN = 6;
	unsigned char ethaddr[ETH_ADDR_LEN];
	/* configure eth1addr for second interface */
	const char *ETHADDR_NAME = "eth1addr";
	struct udevice *dev;
	int ret;

	if (env_get(ETHADDR_NAME))
		return 0;

	/* first eeprom is retrieved, this is for the first interface */
	ret = uclass_first_device_err(UCLASS_I2C_EEPROM, &dev);
	if (ret)
		return ret;

	/* attempt to obtain a second eeprom device */
	ret = uclass_next_device_err(&dev);
	if (ret)
		return ret;

	ret = i2c_eeprom_read(dev, offset, ethaddr, 6);
	if (ret)
		return ret;

	if (is_valid_ethaddr(ethaddr))
		eth_env_set_enetaddr(ETHADDR_NAME, ethaddr);

	return 0;
}
