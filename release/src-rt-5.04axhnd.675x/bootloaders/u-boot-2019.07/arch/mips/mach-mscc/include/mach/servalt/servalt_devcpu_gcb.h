/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#ifndef _MSCC_SERVALT_DEVCPU_GCB_H_
#define _MSCC_SERVALT_DEVCPU_GCB_H_

#define PERF_GPR                                          0x4

#define PERF_SOFT_RST                                     0x8

#define PERF_SOFT_RST_SOFT_NON_CFG_RST                    BIT(2)
#define PERF_SOFT_RST_SOFT_SWC_RST                        BIT(1)
#define PERF_SOFT_RST_SOFT_CHIP_RST                       BIT(0)

#define GPIO_GPIO_ALT(x)                                  (0x74 + 4 * (x))
#define GPIO_GPIO_ALT1(x)                                 (0x7c + 4 * (x))

#define GCB_PHY_CFG                                       0x118

#endif
