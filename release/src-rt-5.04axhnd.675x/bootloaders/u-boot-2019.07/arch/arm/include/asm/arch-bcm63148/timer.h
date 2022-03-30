/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _63148_TIMER_H
#define _63148_TIMER_H

#define TIMR_BASE                               0xfffe8080

#define TIMER_CLKRSTCTL                         0x2c
#define TIMER_CLKRSTCTL_FAP1_PLL_CLKEN          (1<<11)
#define TIMER_CLKRSTCTL_FAP2_PLL_CLKEN          (1<<15)
#define TIMER_WD_RESET                          0x34
#define TIMER_RESET_STATUS                      0x38
#define TIMER_RESET_STATUS_POR                  (1 << 31)

#endif
