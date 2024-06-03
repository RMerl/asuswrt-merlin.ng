/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012
 * Holger Brunck, Keymile GmbH Hannover, <holger.brunck@keymile.com>
 * Christian Herzig, Keymile AG Switzerland, <christian.herzig@keymile.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_HOSTNAME		"kmeter1"
#define CONFIG_KM_BOARD_NAME   "kmeter1"
#define CONFIG_KM_DEF_NETDEV	"netdev=eth2\0"

/* include common defines/options for all Keymile boards */
#include "km/keymile-common.h"
#include "km/km-powerpc.h"
#include "km/km-mpc83xx.h"
#include "km/km-mpc8360.h"

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX	1

#define CONFIG_SYS_DDR_CS0_CONFIG	(CSCONFIG_EN | CSCONFIG_AP | \
					 CSCONFIG_ROW_BIT_13 | \
					 CSCONFIG_COL_BIT_10 | \
					 CSCONFIG_ODT_WR_ONLY_CURRENT)
#endif /* CONFIG */
