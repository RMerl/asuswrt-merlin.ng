/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#ifndef _MACH_DEBUG_UART_H
#define _MACH_DEBUG_UART_H

unsigned int uniphier_ld4_debug_uart_init(void);
unsigned int uniphier_pro4_debug_uart_init(void);
unsigned int uniphier_sld8_debug_uart_init(void);
unsigned int uniphier_pro5_debug_uart_init(void);
unsigned int uniphier_pxs2_debug_uart_init(void);
unsigned int uniphier_ld6b_debug_uart_init(void);

void sg_set_pinsel(unsigned int pin, unsigned int muxval,
		   unsigned int mux_bits, unsigned int reg_stride);
void sg_set_iectrl(unsigned int pin);

#endif /* _MACH_DEBUG_UART_H */
