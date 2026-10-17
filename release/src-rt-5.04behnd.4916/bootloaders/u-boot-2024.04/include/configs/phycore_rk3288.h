/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 PHYTEC Messtechnik GmbH
 * Author: Wadim Egorov <w.egorov@phytec.de>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define ROCKCHIP_DEVICE_SETTINGS
#include <configs/rk3288_common.h>

#undef BOOT_TARGET_DEVICES

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1)

#endif
