/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010-2013
 * NVIDIA Corporation <www.nvidia.com>
 */

/* Tegra124 clock control definitions */

#ifndef _TEGRA124_CLOCK_H_
#define _TEGRA124_CLOCK_H_

#include <asm/arch-tegra/clock.h>

/* CLK_RST_CONTROLLER_OSC_CTRL_0 */
#define OSC_FREQ_SHIFT          28
#define OSC_FREQ_MASK           (0xF << OSC_FREQ_SHIFT)

/* CLK_RST_CONTROLLER_PLLC_MISC_0 */
#define PLLC_IDDQ			(1 << 26)

/* CLK_RST_CONTROLLER_CLK_SOURCE_SOR0_0 */
#define SOR0_CLK_SEL0			(1 << 14)
#define SOR0_CLK_SEL1			(1 << 15)

int tegra_plle_enable(void);

void clock_sor_enable_edp_clock(void);

/**
 * clock_set_display_rate() - Set the display clock rate
 *
 * @frequency: the requested PLLD frequency
 *
 * Return the PLLD frequenc (which may not quite what was requested), or 0
 * on failure
 */
u32 clock_set_display_rate(u32 frequency);

/**
 * clock_set_up_plldp() - Set up the EDP clock ready for use
 */
void clock_set_up_plldp(void);

#endif	/* _TEGRA124_CLOCK_H_ */
