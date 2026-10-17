/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2013, NVIDIA CORPORATION.  All rights reserved.
 */

#ifndef __MX6_COMMON_H
#define __MX6_COMMON_H

#include <linux/stringify.h>

#if (defined(CONFIG_MX6UL) || defined(CONFIG_MX6ULL))
#define CFG_SC_TIMER_CLK 8000000 /* 8Mhz */
#else
#ifndef CONFIG_SYS_L2CACHE_OFF
#define CFG_SYS_PL310_BASE	L2_PL310_BASE
#endif

#endif

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>
#include <asm/mach-imx/gpio.h>

#endif
