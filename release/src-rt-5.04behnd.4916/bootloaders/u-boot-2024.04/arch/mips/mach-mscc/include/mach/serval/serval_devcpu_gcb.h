/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#ifndef _MSCC_SERVAL_DEVCPU_GCB_H_
#define _MSCC_SERVAL_DEVCPU_GCB_H_

#include <linux/bitops.h>
#define CHIP_ID                                           0x0

#define PERF_GPR                                          0x4

#define PERF_SOFT_RST                                     0x8

#define PERF_SOFT_RST_SOFT_NON_CFG_RST                    BIT(2)
#define PERF_SOFT_RST_SOFT_SWC_RST                        BIT(1)
#define PERF_SOFT_RST_SOFT_CHIP_RST                       BIT(0)

#define GPIO_ALT(x)                                       (0x54 + 4 * (x))

#endif
