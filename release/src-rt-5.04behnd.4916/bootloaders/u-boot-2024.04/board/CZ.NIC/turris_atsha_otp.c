// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Marek Behún <kabel@kernel.org>
 * Copyright (C) 2016 Tomas Hlavacek <tomas.hlavacek@nic.cz>
 */

#include <env.h>
#include <net.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <atsha204a-i2c.h>

#include "turris_atsha_otp.h"

#define TURRIS_ATSHA_OTP_VERSION	0
#define TURRIS_ATSHA_OTP_SERIAL		1
#define TURRIS_ATSHA_OTP_MAC0		3
#define TURRIS_ATSHA_OTP_MAC1		4

extern U_BOOT_DRIVER(atsha204);

static struct udevice *get_atsha204a_dev(void)
{
	/* Cannot be static because BSS does not have to be ready at this early stage */
	struct udevice *dev;

	if (uclass_get_device_by_driver(UCLASS_MISC, DM_DRIVER_GET(atsha204), &dev)) {
		puts("Cannot find ATSHA204A on I2C bus!\n");
		dev = NULL;
	}

	return dev;
}

static void increment_mac(u8 *mac)
{
	int i;

	for (i = 5; i >= 3; i--) {
		mac[i] += 1;
		if (mac[i])
			break;
	}
}

static void set_mac_if_invalid(int i, u8 *mac)
{
	u8 oldmac[6];

	if (is_valid_ethaddr(mac) &&
	    !eth_env_get_enetaddr_by_index("eth", i, oldmac))
		eth_env_set_enetaddr_by_index("eth", i, mac);
}

int turris_atsha_otp_init_mac_addresses(int first_idx)
{
	struct udevice *dev = get_atsha204a_dev();
	u8 mac0[4], mac1[4], mac[6];
	int ret;

	if (!dev)
		return -1;

	ret = atsha204a_wakeup(dev);
	if (ret)
		return ret;

	ret = atsha204a_read(dev, ATSHA204A_ZONE_OTP, false,
			     TURRIS_ATSHA_OTP_MAC0, mac0);
	if (ret)
		return ret;

	ret = atsha204a_read(dev, ATSHA204A_ZONE_OTP, false,
			     TURRIS_ATSHA_OTP_MAC1, mac1);
	if (ret)
		return ret;

	atsha204a_sleep(dev);

	mac[0] = mac0[1];
	mac[1] = mac0[2];
	mac[2] = mac0[3];
	mac[3] = mac1[1];
	mac[4] = mac1[2];
	mac[5] = mac1[3];

	set_mac_if_invalid((first_idx + 0) % 3, mac);
	increment_mac(mac);
	set_mac_if_invalid((first_idx + 1) % 3, mac);
	increment_mac(mac);
	set_mac_if_invalid((first_idx + 2) % 3, mac);

	return 0;
}

int turris_atsha_otp_init_serial_number(void)
{
	char serial[17];
	int ret;

	ret = turris_atsha_otp_get_serial_number(serial);
	if (ret)
		return ret;

	if (!env_get("serial#"))
		return -1;

	return 0;
}

int turris_atsha_otp_get_serial_number(char serial[17])
{
	struct udevice *dev = get_atsha204a_dev();
	u32 version_num, serial_num;
	const char *serial_env;
	int ret;

	if (!dev)
		return -1;

	serial_env = env_get("serial#");
	if (serial_env && strlen(serial_env) == 16) {
		memcpy(serial, serial_env, 17);
		return 0;
	}

	ret = atsha204a_wakeup(dev);
	if (ret)
		return ret;

	ret = atsha204a_read(dev, ATSHA204A_ZONE_OTP, false,
			     TURRIS_ATSHA_OTP_VERSION,
			     (u8 *)&version_num);
	if (ret)
		return ret;

	ret = atsha204a_read(dev, ATSHA204A_ZONE_OTP, false,
			     TURRIS_ATSHA_OTP_SERIAL,
			     (u8 *)&serial_num);
	if (ret)
		return ret;

	atsha204a_sleep(dev);

	sprintf(serial, "%08X%08X", be32_to_cpu(version_num), be32_to_cpu(serial_num));
	env_set("serial#", serial);

	return 0;
}
