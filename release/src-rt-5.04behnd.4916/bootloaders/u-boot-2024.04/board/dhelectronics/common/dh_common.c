// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Marek Vasut <marex@denx.de>
 * Copyright 2022 DENX Software Engineering GmbH, Philip Oberfichtner <pro@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <i2c_eeprom.h>
#include <net.h>

#include "dh_common.h"

bool dh_mac_is_in_env(const char *env)
{
	unsigned char enetaddr[6];

	return eth_env_get_enetaddr(env, enetaddr);
}

int dh_get_mac_from_eeprom(unsigned char *enetaddr, const char *alias)
{
	struct udevice *dev;
	int ret;
	ofnode node;

	node = ofnode_path(alias);
	if (!ofnode_valid(node)) {
		printf("%s: ofnode for %s not found!", __func__, alias);
		return -ENOENT;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_I2C_EEPROM, node, &dev);
	if (ret) {
		printf("%s: Cannot find EEPROM! ret = %d\n", __func__, ret);
		return ret;
	}

	ret = i2c_eeprom_read(dev, 0xfa, enetaddr, 0x6);
	if (ret) {
		printf("%s: Error reading EEPROM! ret = %d\n", __func__, ret);
		return ret;
	}

	if (!is_valid_ethaddr(enetaddr)) {
		printf("%s: Address read from EEPROM is invalid!\n", __func__);
		return -EINVAL;
	}

	return 0;
}

__weak int dh_setup_mac_address(void)
{
	unsigned char enetaddr[6];

	if (dh_mac_is_in_env("ethaddr"))
		return 0;

	if (!dh_get_mac_from_eeprom(enetaddr, "eeprom0"))
		return eth_env_set_enetaddr("ethaddr", enetaddr);

	printf("%s: Unable to set mac address!\n", __func__);
	return -ENXIO;
}
