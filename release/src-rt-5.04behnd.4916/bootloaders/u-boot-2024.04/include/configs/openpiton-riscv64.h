/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 * Copyright (c) 2021 Tianrui Wei
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 *   Tianrui Wei <tianrui-wei@outlook.com>
 */

#ifndef __OPENPITON_RISCV64_CONFIG_H
#define __OPENPITON_RISCV64_CONFIG_H

#include <linux/sizes.h>

/* Environment options */
#define CFG_SYS_SDRAM_BASE 0x80000000

/* ---------------------------------------------------------------------
 * Board boot configuration
 */

#define CFG_EXTRA_ENV_SETTINGS \
	"fdt_addr_r=0x86000000\0" \
	"kernel_addr_r=0x80200000\0" \
	"image=boot/Image\0" \
	"mmcdev=0\0" \
	"mmcpart=1\0"

#endif/* __CONFIG_H */
