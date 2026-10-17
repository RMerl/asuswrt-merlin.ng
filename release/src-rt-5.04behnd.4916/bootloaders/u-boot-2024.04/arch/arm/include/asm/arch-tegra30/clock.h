/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010-2012, NVIDIA CORPORATION.  All rights reserved.
 */

/* Tegra30 clock control functions */

#ifndef _TEGRA30_CLOCK_H_
#define _TEGRA30_CLOCK_H_

#include <asm/arch-tegra/clock.h>

/* CLK_RST_CONTROLLER_OSC_CTRL_0 */
#define OSC_FREQ_SHIFT          28
#define OSC_FREQ_MASK           (0xF << OSC_FREQ_SHIFT)

int tegra_plle_enable(void);

#endif	/* _TEGRA30_CLOCK_H_ */
