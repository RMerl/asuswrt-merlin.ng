/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014, Barco (www.barco.com)
 */

#ifndef __PLATINUM_TITANIUM_CONFIG_H__
#define __PLATINUM_TITANIUM_CONFIG_H__

#define CONFIG_PLATINUM_BOARD			"Barco Titanium"
#define CONFIG_PLATINUM_PROJECT			"titanium"
#define CONFIG_PLATINUM_CPU			"imx6q"

#define PHYS_SDRAM_SIZE				(512 << 20)
#define CONFIG_SYS_NAND_MAX_CHIPS		1

#include <configs/platinum.h>

#define CONFIG_FEC_XCV_TYPE			RGMII
#define CONFIG_FEC_MXC_PHYADDR			4

#define CONFIG_PHY_RESET_DELAY			1000

#define CONFIG_HOSTNAME				"titanium"

#define CONFIG_PLATFORM_ENV_SETTINGS		"\0"

#define CONFIG_EXTRA_ENV_SETTINGS		CONFIG_COMMON_ENV_SETTINGS \
						CONFIG_PLATFORM_ENV_SETTINGS

#endif /* __PLATINUM_TITANIUM_CONFIG_H__ */
