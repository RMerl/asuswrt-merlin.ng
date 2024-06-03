// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <dm.h>
#include <environment.h>
#include <i2c_eeprom.h>
#include <netdev.h>

static int get_ethaddr_from_eeprom(u8 *addr)
{
	int ret;
	struct udevice *dev;

	ret = uclass_first_device_err(UCLASS_I2C_EEPROM, &dev);
	if (ret)
		return ret;

	return i2c_eeprom_read(dev, 0, addr, 6);
}

int rk_board_late_init(void)
{
	u8 ethaddr[6];

	if (get_ethaddr_from_eeprom(ethaddr))
		return 0;

	if (is_valid_ethaddr(ethaddr))
		eth_env_set_enetaddr("ethaddr", ethaddr);

	return 0;
}
