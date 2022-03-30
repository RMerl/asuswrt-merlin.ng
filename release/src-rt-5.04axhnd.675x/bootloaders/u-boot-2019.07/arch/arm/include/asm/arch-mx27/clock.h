/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *
 * (c) 2009 Ilya Yanok, Emcraft Systems <yanok@emcraft.com>
 */

#ifndef __ASM_ARCH_CLOCK_H
#define __ASM_ARCH_CLOCK_H

enum mxc_clock {
	MXC_ARM_CLK,
	MXC_I2C_CLK,
	MXC_UART_CLK,
	MXC_ESDHC_CLK,
	MXC_FEC_CLK,
};

unsigned int mxc_get_clock(enum mxc_clock clk);
#define imx_get_uartclk() mxc_get_clock(MXC_UART_CLK)
#define imx_get_fecclk() mxc_get_clock(MXC_FEC_CLK)

#endif /* __ASM_ARCH_CLOCK_H */
