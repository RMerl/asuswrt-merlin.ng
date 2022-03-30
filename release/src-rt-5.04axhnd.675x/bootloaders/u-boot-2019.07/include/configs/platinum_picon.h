/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014, Barco (www.barco.com)
 */

#ifndef __PLATINUM_PICON_CONFIG_H__
#define __PLATINUM_PICON_CONFIG_H__

#define CONFIG_PLATINUM_BOARD			"Barco Picon"
#define CONFIG_PLATINUM_PROJECT			"picon"
#define CONFIG_PLATINUM_CPU			"imx6dl"

#include <configs/platinum.h>

#define CONFIG_FEC_XCV_TYPE			RMII
#define CONFIG_FEC_MXC_PHYADDR			0

#define CONFIG_HOSTNAME				"picon"

#define CONFIG_PLATFORM_ENV_SETTINGS		"\0"

#define CONFIG_EXTRA_ENV_SETTINGS		CONFIG_COMMON_ENV_SETTINGS \
						CONFIG_PLATFORM_ENV_SETTINGS

#endif /* __PLATINUM_PICON_CONFIG_H__ */
