/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010-2013, NVIDIA CORPORATION.  All rights reserved.
 */

/* Tegra114 clock control functions */

#ifndef _TEGRA114_CLOCK_H_
#define _TEGRA114_CLOCK_H_

#include <asm/arch-tegra/clock.h>

/* CLK_RST_CONTROLLER_OSC_CTRL_0 */
#define OSC_FREQ_SHIFT          28
#define OSC_FREQ_MASK           (0xF << OSC_FREQ_SHIFT)

/* CLK_RST_CONTROLLER_PLLC_MISC_0 */
#define PLLC_IDDQ		(1 << 26)

#endif	/* _TEGRA114_CLOCK_H_ */
