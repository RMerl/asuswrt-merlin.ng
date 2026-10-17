/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022-2023 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * (C) Copyright 2022 Linaro
 * Rui Miguel Silva <rui.silva@linaro.org>
 * Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
 *
 * Configuration for Corstone1000. Parts were derived from other ARM
 * configurations.
 */

#ifndef __CORSTONE1000_H
#define __CORSTONE1000_H

#include <linux/sizes.h>

#define V2M_BASE		0x80000000

#define CFG_PL011_CLOCK	50000000

/* Physical Memory Map */
#define PHYS_SDRAM_1		(V2M_BASE)
#define PHYS_SDRAM_1_SIZE	0x80000000

#define CFG_SYS_SDRAM_BASE	PHYS_SDRAM_1

#define BOOT_TARGET_DEVICES(func) \
	func(USB, usb, 0)

#include <config_distro_bootcmd.h>

#define CFG_EXTRA_ENV_SETTINGS BOOTENV

#endif
