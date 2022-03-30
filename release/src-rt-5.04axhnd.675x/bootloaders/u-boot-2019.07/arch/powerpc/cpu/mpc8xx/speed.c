// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <mpc8xx.h>
#include <asm/processor.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * get_clocks() fills in gd->cpu_clock depending on CONFIG_8xx_GCLK_FREQ
 */
int get_clocks(void)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;
	uint sccr = in_be32(&immap->im_clkrst.car_sccr);
	uint divider = 1 << (((sccr & SCCR_DFBRG11) >> 11) * 2);

	/*
	 * If for some reason measuring the gclk frequency won't
	 * work, we return the hardwired value.
	 * (For example, the cogent CMA286-60 CPU module has no
	 * separate oscillator for PITRTCLK)
	 */
	gd->cpu_clk = CONFIG_8xx_GCLK_FREQ;

	if ((sccr & SCCR_EBDF11) == 0) {
		/* No Bus Divider active */
		gd->bus_clk = gd->cpu_clk;
	} else {
		/* The MPC8xx has only one BDF: half clock speed */
		gd->bus_clk = gd->cpu_clk / 2;
	}

	gd->arch.brg_clk = gd->cpu_clk / divider;

	return 0;
}
