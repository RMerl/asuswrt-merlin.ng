/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 */

#ifndef __ASM_ARCH_LS102XA_CLOCK_H_
#define __ASM_ARCH_LS102XA_CLOCK_H_

#include <common.h>

enum mxc_clock {
	MXC_ARM_CLK = 0,
	MXC_UART_CLK,
	MXC_ESDHC_CLK,
	MXC_I2C_CLK,
	MXC_DSPI_CLK,
};

unsigned int mxc_get_clock(enum mxc_clock clk);
ulong get_ddr_freq(ulong);
uint get_svr(void);

#endif /* __ASM_ARCH_LS102XA_CLOCK_H_ */
