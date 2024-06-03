/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013 Linaro
 * Andre Przywara, <andre.przywara@linaro.org>
 *
 * Configuration for Versatile Express. Parts were derived from other ARM
 *   configurations.
 */

#ifndef __VEXPRESS_CA15X2_TC2_h
#define __VEXPRESS_CA15X2_TC2_h

#define CONFIG_VEXPRESS_EXTENDED_MEMORY_MAP
#include "vexpress_common.h"

#define CONFIG_SYSFLAGS_ADDR	0x1c010030
#define CONFIG_SMP_PEN_ADDR	CONFIG_SYSFLAGS_ADDR

#endif
