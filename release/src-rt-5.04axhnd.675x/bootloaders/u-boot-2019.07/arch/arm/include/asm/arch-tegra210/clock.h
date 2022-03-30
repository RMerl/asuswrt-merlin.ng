/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010-2015
 * NVIDIA Corporation <www.nvidia.com>
 */

/* Tegra210 clock control definitions */

#ifndef _TEGRA210_CLOCK_H_
#define _TEGRA210_CLOCK_H_

#include <asm/arch-tegra/clock.h>

/* CLK_RST_CONTROLLER_OSC_CTRL_0 */
#define OSC_FREQ_SHIFT          28
#define OSC_FREQ_MASK           (0xF << OSC_FREQ_SHIFT)

/* PLL bits that differ from generic clk_rst.h */
#define PLLC_RESET		30
#define PLLC_IDDQ		27
#define PLLD_ENABLE_CLK		21
#define PLLD_EN_LCKDET		28

int tegra_plle_enable(void);

#endif	/* _TEGRA210_CLOCK_H_ */
