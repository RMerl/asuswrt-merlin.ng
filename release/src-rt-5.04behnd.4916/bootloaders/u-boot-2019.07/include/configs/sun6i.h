/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012-2013 Henrik Nordstrom <henrik@henriknordstrom.net>
 * (C) Copyright 2013 Luke Kenneth Casson Leighton <lkcl@lkcl.net>
 * (C) Copyright 2013 Maxime Ripard <maxime.ripard@free-electrons.com>
 *
 * Configuration settings for the Allwinner A31 (sun6i) CPU
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * A31 specific configuration
 */

#define CONFIG_ARMV7_SECURE_BASE	SUNXI_SRAM_B_BASE
#define CONFIG_ARMV7_SECURE_MAX_SIZE    (64 * 1024) /* 64 KB */

/*
 * Include common sunxi configuration where most the settings are
 */
#include <configs/sunxi-common.h>

#endif /* __CONFIG_H */
