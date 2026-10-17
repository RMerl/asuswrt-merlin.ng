/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013-2016 Stefan Agner
 *
 * Configuration settings for the Toradex Colibri T30 modules.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

#include "tegra30-common.h"

/* High-level configuration options */

/*
 * Board-specific serial config
 *
 * Colibri UART-A: NVIDIA UARTA
 * Colibri UART-B: NVIDIA UARTD
 * Colibri UART-C: NVIDIA UARTB
 */
#define CFG_SYS_NS16550_COM1		NV_PA_APB_UARTA_BASE

#define UBOOT_UPDATE \
	"uboot_hwpart=1\0" \
	"uboot_blk=0\0" \
	"set_blkcnt=setexpr blkcnt ${filesize} + 0x1ff && " \
		"setexpr blkcnt ${blkcnt} / 0x200\0" \
	"update_uboot=run set_blkcnt && mmc dev 0 ${uboot_hwpart} && " \
		"mmc write ${loadaddr} ${uboot_blk} ${blkcnt}\0" \

#define BOARD_EXTRA_ENV_SETTINGS \
	UBOOT_UPDATE \
	"boot_script_dhcp=boot.scr\0"

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
