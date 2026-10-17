/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#ifndef _MSCC_OCELOT_DEVCPU_GCB_H_
#define _MSCC_OCELOT_DEVCPU_GCB_H_

#include <linux/bitops.h>
#define PERF_SOFT_RST                                     0x90

#define PERF_SOFT_RST_SOFT_SWC_RST                        BIT(1)
#define PERF_SOFT_RST_SOFT_CHIP_RST                       BIT(0)

#define GPIO_ALT(x)				(0x88 + 4 * (x))

#define CHIP_ID					(0x08)

#endif
