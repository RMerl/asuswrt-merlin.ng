/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_POWERPC_CLOCK_H
#define __ASM_POWERPC_CLOCK_H

/* Make fsl_esdhc driver happy */
enum mxc_clock {
	MXC_ESDHC_CLK,
};

DECLARE_GLOBAL_DATA_PTR;

uint mxc_get_clock(int clk)
{
	return gd->arch.sdhc_clk;
}
#endif /* __ASM_POWERPC_CLOCK_H */
