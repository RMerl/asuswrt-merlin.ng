// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Rosy Song <rosysong@rosinson.com>
 */

#include <common.h>
#include <init.h>
#include <asm/io.h>
#include <asm/addrspace.h>
#include <asm/types.h>
#include <mach/ar71xx_regs.h>
#include <mach/ddr.h>
#include <mach/ath79.h>
#include <debug_uart.h>

#define RST_RESET_RTC_RESET_LSB 27
#define RST_RESET_RTC_RESET_MASK 0x08000000
#define RST_RESET_RTC_RESET_SET(x) \
	(((x) << RST_RESET_RTC_RESET_LSB) & RST_RESET_RTC_RESET_MASK)

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
	void __iomem *regs;
	u32 val;

	regs = map_physmem(AR71XX_GPIO_BASE, AR71XX_GPIO_SIZE,
			   MAP_NOCACHE);

	/* UART : RX18, TX22 done
	 * GPIO18 as input, GPIO22 as output
	 */
	val = readl(regs + AR71XX_GPIO_REG_OE);
	val |= QCA956X_GPIO(18);
	val &= ~QCA956X_GPIO(22);
	writel(val, regs + AR71XX_GPIO_REG_OE);

	/*
	 * Enable GPIO22 as UART0_SOUT
	 */
	val = readl(regs + QCA956X_GPIO_REG_OUT_FUNC5);
	val &= ~QCA956X_GPIO_MUX_MASK(16);
	val |= QCA956X_GPIO_OUT_MUX_UART0_SOUT << 16;
	writel(val, regs + QCA956X_GPIO_REG_OUT_FUNC5);

	/*
	 * Enable GPIO18 as UART0_SIN
	 */
	val = readl(regs + QCA956X_GPIO_REG_IN_ENABLE0);
	val &= ~QCA956X_GPIO_MUX_MASK(8);
	val |= QCA956X_GPIO_IN_MUX_UART0_SIN << 8;
	writel(val, regs + QCA956X_GPIO_REG_IN_ENABLE0);

	/*
	 * Enable GPIO22 output
	 */
	val = readl(regs + AR71XX_GPIO_REG_OUT);
	val |= QCA956X_GPIO(22);
	writel(val, regs + AR71XX_GPIO_REG_OUT);
}
#endif

int board_early_init_f(void)
{
	u32 reg;
	void __iomem *rst_regs = map_physmem(AR71XX_RESET_BASE,
							 AR71XX_RESET_SIZE, MAP_NOCACHE);

#if !CONFIG_IS_ENABLED(SKIP_LOWLEVEL_INIT)
	/* CPU:775, DDR:650, AHB:258 */
	qca956x_pll_init();
	qca956x_ddr_init();
#endif

	/* Take WMAC out of reset */
	reg = readl(rst_regs + QCA956X_RESET_REG_RESET_MODULE);
	reg &= (~RST_RESET_RTC_RESET_SET(1));
	writel(reg, rst_regs + QCA956X_RESET_REG_RESET_MODULE);

	ath79_eth_reset();
	return 0;
}
