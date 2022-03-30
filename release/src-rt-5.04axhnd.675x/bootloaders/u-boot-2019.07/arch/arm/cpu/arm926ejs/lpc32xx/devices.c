// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 by Vladimir Zapolskiy <vz@mleia.com>
 */

#include <common.h>
#include <dm.h>
#include <ns16550.h>
#include <dm/platform_data/lpc32xx_hsuart.h>

#include <asm/arch/clk.h>
#include <asm/arch/uart.h>
#include <asm/arch/mux.h>
#include <asm/io.h>

static struct clk_pm_regs    *clk  = (struct clk_pm_regs *)CLK_PM_BASE;
static struct uart_ctrl_regs *ctrl = (struct uart_ctrl_regs *)UART_CTRL_BASE;
static struct mux_regs *mux = (struct mux_regs *)MUX_BASE;

void lpc32xx_uart_init(unsigned int uart_id)
{
	if (uart_id < 1 || uart_id > 7)
		return;

	/* Disable loopback mode, if it is set by S1L bootloader */
	clrbits_le32(&ctrl->loop,
		     UART_LOOPBACK(CONFIG_SYS_LPC32XX_UART));

	if (uart_id < 3 || uart_id > 6)
		return;

	/* Enable UART system clock */
	setbits_le32(&clk->uartclk_ctrl, CLK_UART(uart_id));

	/* Set UART into autoclock mode */
	clrsetbits_le32(&ctrl->clkmode,
			UART_CLKMODE_MASK(uart_id),
			UART_CLKMODE_AUTO(uart_id));

	/* Bypass pre-divider of UART clock */
	writel(CLK_UART_X_DIV(1) | CLK_UART_Y_DIV(1),
	       &clk->u3clk + (uart_id - 3));
}

#if !CONFIG_IS_ENABLED(OF_CONTROL)
static const struct ns16550_platdata lpc32xx_uart[] = {
	{ .base = UART3_BASE, .reg_shift = 2,
	  .clock = CONFIG_SYS_NS16550_CLK, .fcr = UART_FCR_DEFVAL, },
	{ .base = UART4_BASE, .reg_shift = 2,
	  .clock = CONFIG_SYS_NS16550_CLK, .fcr = UART_FCR_DEFVAL, },
	{ .base = UART5_BASE, .reg_shift = 2,
	  .clock = CONFIG_SYS_NS16550_CLK, .fcr = UART_FCR_DEFVAL, },
	{ .base = UART6_BASE, .reg_shift = 2,
	  .clock = CONFIG_SYS_NS16550_CLK, .fcr = UART_FCR_DEFVAL, },
};

#if defined(CONFIG_LPC32XX_HSUART)
static const struct lpc32xx_hsuart_platdata lpc32xx_hsuart[] = {
	{ HS_UART1_BASE, },
	{ HS_UART2_BASE, },
	{ HS_UART7_BASE, },
};
#endif

U_BOOT_DEVICES(lpc32xx_uarts) = {
#if defined(CONFIG_LPC32XX_HSUART)
	{ "lpc32xx_hsuart", &lpc32xx_hsuart[0], },
	{ "lpc32xx_hsuart", &lpc32xx_hsuart[1], },
#endif
	{ "ns16550_serial", &lpc32xx_uart[0], },
	{ "ns16550_serial", &lpc32xx_uart[1], },
	{ "ns16550_serial", &lpc32xx_uart[2], },
	{ "ns16550_serial", &lpc32xx_uart[3], },
#if defined(CONFIG_LPC32XX_HSUART)
	{ "lpc32xx_hsuart", &lpc32xx_hsuart[2], },
#endif
};
#endif

void lpc32xx_dma_init(void)
{
	/* Enable DMA interface */
	writel(CLK_DMA_ENABLE, &clk->dmaclk_ctrl);
}

void lpc32xx_mac_init(void)
{
	/* Enable MAC interface */
	writel(CLK_MAC_REG | CLK_MAC_SLAVE | CLK_MAC_MASTER
#if defined(CONFIG_RMII)
		| CLK_MAC_RMII,
#else
		| CLK_MAC_MII,
#endif
		&clk->macclk_ctrl);
}

void lpc32xx_mlc_nand_init(void)
{
	/* Enable NAND interface */
	writel(CLK_NAND_MLC | CLK_NAND_MLC_INT, &clk->flashclk_ctrl);
}

void lpc32xx_slc_nand_init(void)
{
	/* Enable SLC NAND interface */
	writel(CLK_NAND_SLC | CLK_NAND_SLC_SELECT, &clk->flashclk_ctrl);
}

void lpc32xx_usb_init(void)
{
	/* Do not route the UART 5 Tx/Rx pins to the USB D+ and USB D- pins. */
	clrbits_le32(&ctrl->ctrl, UART_CTRL_UART5_USB_MODE);
}

void lpc32xx_i2c_init(unsigned int devnum)
{
	/* Enable I2C interface */
	uint32_t ctrl = readl(&clk->i2cclk_ctrl);
	if (devnum == 1)
		ctrl |= CLK_I2C1_ENABLE;
	if (devnum == 2)
		ctrl |= CLK_I2C2_ENABLE;
	writel(ctrl, &clk->i2cclk_ctrl);
}

U_BOOT_DEVICE(lpc32xx_gpios) = {
	.name = "gpio_lpc32xx"
};

/* Mux for SCK0, MISO0, MOSI0. We do not use SSEL0. */

#define P_MUX_SET_SSP0 0x1600

void lpc32xx_ssp_init(void)
{
	/* Enable SSP0 interface */
	writel(CLK_SSP0_ENABLE_CLOCK, &clk->ssp_ctrl);
	/* Mux SSP0 pins */
	writel(P_MUX_SET_SSP0, &mux->p_mux_set);
}
