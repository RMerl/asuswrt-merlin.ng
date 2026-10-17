// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2022 Google LLC
 */
#include <net.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <atsha204a-i2c.h>
#include "mercury_aa1.h"

#define MERCURY_AA1_ATSHA204A_OTP_MAC0 4
#define MERCURY_AA1_ATSHA204A_OTP_MAC1 5

int mercury_aa1_read_mac(u8 *mac)
{
	struct udevice *dev;
	u8 buf[8];
	int ret;

	ret = uclass_get_device_by_name(UCLASS_MISC, "atsha204a@64", &dev);
	if (ret)
		return ret;

	ret = atsha204a_wakeup(dev);
	if (ret)
		return ret;

	ret = atsha204a_read(dev, ATSHA204A_ZONE_OTP, false,
			     MERCURY_AA1_ATSHA204A_OTP_MAC0, buf);
	if (ret)
		goto sleep;

	ret = atsha204a_read(dev, ATSHA204A_ZONE_OTP, false,
			     MERCURY_AA1_ATSHA204A_OTP_MAC1, buf + 4);
	if (ret)
		goto sleep;

	memcpy(mac, buf, ARP_HLEN);

sleep:
	atsha204a_sleep(dev);
	return ret;
}
