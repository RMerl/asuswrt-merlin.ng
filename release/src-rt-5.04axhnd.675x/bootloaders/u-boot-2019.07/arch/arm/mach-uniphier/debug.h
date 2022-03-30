/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <linux/io.h>
#include <linux/serial_reg.h>

#define DEBUG_UART_BASE		0x54006800
#define UART_SHIFT 2

#define UNIPHIER_UART_TX		0
#define UNIPHIER_UART_LSR		(5 * 4)

/* All functions are inline so that they can be called from .secure section. */

#ifdef DEBUG
static inline void debug_putc(int c)
{
	void __iomem *base = (void __iomem *)DEBUG_UART_BASE;

	while (!(readl(base + UNIPHIER_UART_LSR) & UART_LSR_THRE))
		;

	writel(c, base + UNIPHIER_UART_TX);
}

static inline void debug_puts(const char *s)
{
	while (*s) {
		if (*s == '\n')
			debug_putc('\r');

		debug_putc(*s++);
	}
}

static inline void debug_puth(unsigned long val)
{
	int i;
	unsigned char c;

	for (i = 8; i--; ) {
		c = ((val >> (i * 4)) & 0xf);
		c += (c >= 10) ? 'a' - 10 : '0';
		debug_putc(c);
	}
}
#else
static inline void debug_putc(int c)
{
}

static inline void debug_puts(const char *s)
{
}

static inline void debug_puth(unsigned long val)
{
}
#endif

#endif /* __DEBUG_H__ */
