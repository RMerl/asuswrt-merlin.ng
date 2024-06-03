// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/addrspace.h>
#include <asm/types.h>
#include <mach/ath79.h>
#include <mach/ar71xx_regs.h>
#include <mach/ddr.h>
#include <debug_uart.h>

#ifdef CONFIG_USB
static void wdr4300_usb_start(void)
{
	void __iomem *gpio_regs = map_physmem(AR71XX_GPIO_BASE,
					      AR71XX_GPIO_SIZE, MAP_NOCACHE);
	if (!gpio_regs)
		return;

	/* Power up the USB HUB. */
	clrbits_be32(gpio_regs + AR71XX_GPIO_REG_OE, BIT(21) | BIT(22));
	writel(BIT(21) | BIT(22), gpio_regs + AR71XX_GPIO_REG_SET);
	mdelay(1);

	ath79_usb_reset();
}
#else
static inline void wdr4300_usb_start(void) {}
#endif

void wdr4300_pinmux_config(void)
{
	void __iomem *regs;

	regs = map_physmem(AR71XX_GPIO_BASE, AR71XX_GPIO_SIZE,
			   MAP_NOCACHE);

	/* Assure JTAG is not disconnected. */
	writel(0x40, regs + AR934X_GPIO_REG_FUNC);

	/* Configure default GPIO input/output regs. */
	writel(0x3031b, regs + AR71XX_GPIO_REG_OE);
	writel(0x0f804, regs + AR71XX_GPIO_REG_OUT);

	/* Configure pin multiplexing. */
	writel(0x00000000, regs + AR934X_GPIO_REG_OUT_FUNC0);
	writel(0x0b0a0980, regs + AR934X_GPIO_REG_OUT_FUNC1);
	writel(0x00180000, regs + AR934X_GPIO_REG_OUT_FUNC2);
	writel(0x00000000, regs + AR934X_GPIO_REG_OUT_FUNC3);
	writel(0x0000004d, regs + AR934X_GPIO_REG_OUT_FUNC4);
	writel(0x00000000, regs + AR934X_GPIO_REG_OUT_FUNC5);
}

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
	wdr4300_pinmux_config();
}
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
#ifndef CONFIG_DEBUG_UART_BOARD_INIT
	wdr4300_pinmux_config();
#endif

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	ar934x_pll_init(560, 480, 240);
	ar934x_ddr_init(560, 480, 240);
#endif

	wdr4300_usb_start();
	ath79_eth_reset();

	return 0;
}
#endif
