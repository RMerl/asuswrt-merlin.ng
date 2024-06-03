/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 * Copyright 2019 NXP Semiconductors
 *
 */

#ifndef __ASM_ARCH_FSL_LAYERSCAPE_CLOCK_H_
#define __ASM_ARCH_FSL_LAYERSCAPE_CLOCK_H_

#include <common.h>

enum mxc_clock {
	MXC_ARM_CLK = 0,
	MXC_BUS_CLK,
	MXC_UART_CLK,
	MXC_ESDHC_CLK,
	MXC_ESDHC2_CLK,
	MXC_I2C_CLK,
	MXC_DSPI_CLK,
};

unsigned int mxc_get_clock(enum mxc_clock clk);
ulong get_ddr_freq(ulong);
uint get_svr(void);

#endif /* __ASM_ARCH_FSL_LAYERSCAPE_CLOCK_H_ */
