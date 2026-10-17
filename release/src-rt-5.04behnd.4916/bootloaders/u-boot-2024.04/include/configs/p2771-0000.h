/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2013-2016, NVIDIA CORPORATION.
 */

#ifndef _P2771_0000_H
#define _P2771_0000_H

#include <linux/sizes.h>

#include "tegra186-common.h"

/* High-level configuration options */
#define CFG_TEGRA_BOARD_STRING	"NVIDIA P2771-0000"

/* Environment in eMMC, at the end of 2nd "boot sector" */

#define BOARD_EXTRA_ENV_SETTINGS \
	"calculated_vars=kernel_addr_r fdt_addr_r scriptaddr pxefile_addr_r " \
		"ramdisk_addr_r\0" \
	"kernel_addr_r_align=00200000\0" \
	"kernel_addr_r_offset=00080000\0" \
	"kernel_addr_r_size=02000000\0" \
	"kernel_addr_r_aliases=loadaddr\0" \
	"fdt_addr_r_align=00200000\0" \
	"fdt_addr_r_offset=00000000\0" \
	"fdt_addr_r_size=00200000\0" \
	"scriptaddr_align=00200000\0" \
	"scriptaddr_offset=00000000\0" \
	"scriptaddr_size=00200000\0" \
	"pxefile_addr_r_align=00200000\0" \
	"pxefile_addr_r_offset=00000000\0" \
	"pxefile_addr_r_size=00200000\0" \
	"ramdisk_addr_r_align=00200000\0" \
	"ramdisk_addr_r_offset=00000000\0" \
	"ramdisk_addr_r_size=02000000\0"

#include "tegra-common-post.h"

#endif
