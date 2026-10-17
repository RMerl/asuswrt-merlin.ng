/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Amarula Solutions
 *
 * Configuration settings for Amarula Vyasa RK3288.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define ROCKCHIP_DEVICE_SETTINGS \
		"stdin=serial,usbkbd\0" \
		"stdout=serial,vidconsole\0" \
		"stderr=serial,vidconsole\0"

#include <configs/rk3288_common.h>

#undef BOOT_TARGET_DEVICES

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \

#endif
