/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#ifndef __ASM_ARCH_CLOCK_H
#define __ASM_ARCH_CLOCK_H

#include <common.h>

enum mxc_clock {
	MXC_ARM_CLK = 0,
	MXC_BUS_CLK,
	MXC_IPG_CLK,
	MXC_UART_CLK,
	MXC_ESDHC_CLK,
	MXC_FEC_CLK,
	MXC_I2C_CLK,
	MXC_DSPI_CLK,
};

void enable_ocotp_clk(unsigned char enable);
unsigned int mxc_get_clock(enum mxc_clock clk);
u32 get_lpuart_clk(void);
#ifdef CONFIG_SYS_I2C_MXC
int enable_i2c_clk(unsigned char enable, unsigned int i2c_num);
#endif

#define imx_get_fecclk() mxc_get_clock(MXC_FEC_CLK)

#endif /* __ASM_ARCH_CLOCK_H */
