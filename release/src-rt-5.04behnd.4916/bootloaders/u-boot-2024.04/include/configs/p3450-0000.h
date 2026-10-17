/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2018-2019 NVIDIA Corporation.
 */

#ifndef _P3450_0000_H
#define _P3450_0000_H

#include <linux/sizes.h>

#include "tegra210-common.h"

/* High-level configuration options */
#define CFG_TEGRA_BOARD_STRING	"NVIDIA P3450-0000"

/* Board-specific serial config */

/* Only MMC/PXE/DHCP for now, add USB back in later when supported */
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#define BOARD_EXTRA_ENV_SETTINGS \
	"preboot=if test -e mmc 1:1 /u-boot-preboot.scr; then " \
		"load mmc 1:1 ${scriptaddr} /u-boot-preboot.scr; " \
		"source ${scriptaddr}; " \
	"fi\0"

/* General networking support */
#include "tegra-common-post.h"

#endif /* _P3450_0000_H */
