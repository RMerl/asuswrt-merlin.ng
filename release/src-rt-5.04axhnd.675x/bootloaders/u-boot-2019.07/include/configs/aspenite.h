/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 * Contributor: Mahavir Jain <mjain@marvell.com>
 */

#ifndef __CONFIG_ASPENITE_H
#define __CONFIG_ASPENITE_H

/*
 * High Level Configuration Options
 */
#define CONFIG_SHEEVA_88SV331xV5	1	/* CPU Core subversion */
#define CONFIG_ARMADA100		1	/* SOC Family Name */
#define CONFIG_ARMADA168		1	/* SOC Used on this Board */
#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */

/*
 * There is no internal RAM in ARMADA100, using DRAM
 * TBD: dcache to be used for this
 */
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_TEXT_BASE - 0x00200000)

/*
 * Commands configuration
 */
/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

/*
 * Environment variables configurations
 */
#define CONFIG_ENV_SIZE	0x20000	/* 64k */

#endif	/* __CONFIG_ASPENITE_H */
