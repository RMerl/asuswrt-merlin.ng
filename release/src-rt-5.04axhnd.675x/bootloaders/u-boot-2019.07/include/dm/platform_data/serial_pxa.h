/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2016 Marcel Ziswiler <marcel.ziswiler@toradex.com>
 */

#ifndef __SERIAL_PXA_H
#define __SERIAL_PXA_H

/*
 * The numbering scheme differs here for PXA25x, PXA27x and PXA3xx so we can
 * easily handle enabling of clock.
 */
#ifdef CONFIG_CPU_MONAHANS
#define UART_CLK_BASE	CKENA_21_BTUART
#define UART_CLK_REG	CKENA
#define BTUART_INDEX	0
#define FFUART_INDEX	1
#define STUART_INDEX	2
#elif CONFIG_CPU_PXA25X
#define UART_CLK_BASE	(1 << 4)	/* HWUART */
#define UART_CLK_REG	CKEN
#define HWUART_INDEX	0
#define STUART_INDEX	1
#define FFUART_INDEX	2
#define BTUART_INDEX	3
#else /* PXA27x */
#define UART_CLK_BASE	CKEN5_STUART
#define UART_CLK_REG	CKEN
#define STUART_INDEX	0
#define FFUART_INDEX	1
#define BTUART_INDEX	2
#endif

/*
 * Only PXA250 has HWUART, to avoid poluting the code with more macros,
 * artificially introduce this.
 */
#ifndef CONFIG_CPU_PXA25X
#define HWUART_INDEX	0xff
#endif

/*
 * struct pxa_serial_platdata - information about a PXA port
 *
 * @base:               Uart port base register address
 * @port:               Uart port index, for cpu with pinmux for uart / gpio
 * baudrtatre:          Uart port baudrate
 */
struct pxa_serial_platdata {
	struct pxa_uart_regs *base;
	int port;
	int baudrate;
};

#endif /* __SERIAL_PXA_H */
