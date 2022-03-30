/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration settings for the Allwinner A64 (sun50i) CPU
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * A64 specific configuration
 */

#ifndef CONFIG_MACH_SUN50I_H6
#define GICD_BASE		0x1c81000
#define GICC_BASE		0x1c82000
#else
#define GICD_BASE		0x3021000
#define GICC_BASE		0x3022000
#endif

/*
 * Include common sunxi configuration where most the settings are
 */
#include <configs/sunxi-common.h>

#endif /* __CONFIG_H */
