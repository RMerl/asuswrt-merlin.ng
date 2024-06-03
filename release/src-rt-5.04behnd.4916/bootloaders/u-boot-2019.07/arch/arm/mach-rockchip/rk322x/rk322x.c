// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 Rockchip Electronics Co., Ltd
 */
#include <asm/io.h>
#include <asm/arch-rockchip/grf_rk322x.h>
#include <asm/arch-rockchip/hardware.h>

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
#define GRF_BASE	0x11000000
	static struct rk322x_grf * const grf = (void *)GRF_BASE;
	enum {
		GPIO1B2_SHIFT		= 4,
		GPIO1B2_MASK		= 3 << GPIO1B2_SHIFT,
		GPIO1B2_GPIO            = 0,
		GPIO1B2_UART1_SIN,
		GPIO1B2_UART21_SIN,

		GPIO1B1_SHIFT		= 2,
		GPIO1B1_MASK		= 3 << GPIO1B1_SHIFT,
		GPIO1B1_GPIO            = 0,
		GPIO1B1_UART1_SOUT,
		GPIO1B1_UART21_SOUT,
	};
	enum {
		CON_IOMUX_UART2SEL_SHIFT = 8,
		CON_IOMUX_UART2SEL_MASK	= 1 << CON_IOMUX_UART2SEL_SHIFT,
		CON_IOMUX_UART2SEL_2	= 0,
		CON_IOMUX_UART2SEL_21,
	};

	/* Enable early UART2 channel 1 on the RK322x */
	rk_clrsetreg(&grf->gpio1b_iomux,
		     GPIO1B1_MASK | GPIO1B2_MASK,
		     GPIO1B2_UART21_SIN << GPIO1B2_SHIFT |
		     GPIO1B1_UART21_SOUT << GPIO1B1_SHIFT);
	/* Set channel C as UART2 input */
	rk_clrsetreg(&grf->con_iomux,
		     CON_IOMUX_UART2SEL_MASK,
		     CON_IOMUX_UART2SEL_21 << CON_IOMUX_UART2SEL_SHIFT);
}
#endif
