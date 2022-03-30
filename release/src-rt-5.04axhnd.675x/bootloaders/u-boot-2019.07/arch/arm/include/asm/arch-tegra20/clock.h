/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

/* Tegra20 clock control functions */

#ifndef _TEGRA20_CLOCK_H
#define _TEGRA20_CLOCK_H

#include <asm/arch-tegra/clock.h>

/* CLK_RST_CONTROLLER_OSC_CTRL_0 */
#define OSC_FREQ_SHIFT          30
#define OSC_FREQ_MASK           (3U << OSC_FREQ_SHIFT)

int tegra_plle_enable(void);

#endif	/* _TEGRA20_CLOCK_H */
