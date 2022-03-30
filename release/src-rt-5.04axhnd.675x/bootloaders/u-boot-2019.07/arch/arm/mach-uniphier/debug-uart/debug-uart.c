// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <debug_uart.h>
#include <linux/io.h>
#include <linux/serial_reg.h>

#include "../sg-regs.h"
#include "../soc-info.h"
#include "debug-uart.h"

#define UNIPHIER_UART_TX		0x00
#define UNIPHIER_UART_LCR_MCR		0x10
#define UNIPHIER_UART_LSR		0x14
#define UNIPHIER_UART_LDR		0x24

static void _debug_uart_putc(int c)
{
	void __iomem *base = (void __iomem *)CONFIG_DEBUG_UART_BASE;

	while (!(readl(base + UNIPHIER_UART_LSR) & UART_LSR_THRE))
		;

	writel(c, base + UNIPHIER_UART_TX);
}

#ifdef CONFIG_SPL_BUILD
void sg_set_pinsel(unsigned int pin, unsigned int muxval,
		   unsigned int mux_bits, unsigned int reg_stride)
{
	unsigned int shift = pin * mux_bits % 32;
	unsigned long reg = SG_PINCTRL_BASE + pin * mux_bits / 32 * reg_stride;
	u32 mask = (1U << mux_bits) - 1;
	u32 tmp;

	tmp = readl(reg);
	tmp &= ~(mask << shift);
	tmp |= (mask & muxval) << shift;
	writel(tmp, reg);
}

void sg_set_iectrl(unsigned int pin)
{
	unsigned int bit = pin % 32;
	unsigned long reg = SG_IECTRL + pin / 32 * 4;
	u32 tmp;

	tmp = readl(reg);
	tmp |= 1 << bit;
	writel(tmp, reg);
}
#endif

void _debug_uart_init(void)
{
#ifdef CONFIG_SPL_BUILD
	void __iomem *base = (void __iomem *)CONFIG_DEBUG_UART_BASE;
	unsigned int divisor;

	switch (uniphier_get_soc_id()) {
#if defined(CONFIG_ARCH_UNIPHIER_LD4)
	case UNIPHIER_LD4_ID:
		divisor = uniphier_ld4_debug_uart_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO4)
	case UNIPHIER_PRO4_ID:
		divisor = uniphier_pro4_debug_uart_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_SLD8)
	case UNIPHIER_SLD8_ID:
		divisor = uniphier_sld8_debug_uart_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO5)
	case UNIPHIER_PRO5_ID:
		divisor = uniphier_pro5_debug_uart_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PXS2)
	case UNIPHIER_PXS2_ID:
		divisor = uniphier_pxs2_debug_uart_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD6B)
	case UNIPHIER_LD6B_ID:
		divisor = uniphier_ld6b_debug_uart_init();
		break;
#endif
	default:
		return;
	}

	writel(UART_LCR_WLEN8 << 8, base + UNIPHIER_UART_LCR_MCR);

	writel(divisor, base + UNIPHIER_UART_LDR);
#endif
}
DEBUG_UART_FUNCS
