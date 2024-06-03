// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * (C) Copyright 2009 Magnus Lilja <lilja.magnus@gmail.com>
 *
 * (c) 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
 */

#include <common.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>

void mx31_uart1_hw_init(void)
{
	/* setup pins for UART1 */
	mx31_gpio_mux(MUX_RXD1__UART1_RXD_MUX);
	mx31_gpio_mux(MUX_TXD1__UART1_TXD_MUX);
	mx31_gpio_mux(MUX_RTS1__UART1_RTS_B);
	mx31_gpio_mux(MUX_CTS1__UART1_CTS_B);
}

void mx31_uart2_hw_init(void)
{
	/* setup pins for UART2 */
	mx31_gpio_mux(MUX_RXD2__UART2_RXD_MUX);
	mx31_gpio_mux(MUX_TXD2__UART2_TXD_MUX);
	mx31_gpio_mux(MUX_RTS2__UART2_RTS_B);
	mx31_gpio_mux(MUX_CTS2__UART2_CTS_B);
}

#ifdef CONFIG_MXC_SPI
/*
 * Note: putting several spi setups here makes no sense as they may differ
 * at board level (physical pin SS0 of CSPI2 may aswell be used as SS0 of CSPI3)
 */
void mx31_spi2_hw_init(void)
{
	/* SPI2 */
	mx31_gpio_mux(MUX_CSPI2_SS2__CSPI2_SS2_B);
	mx31_gpio_mux(MUX_CSPI2_SCLK__CSPI2_CLK);
	mx31_gpio_mux(MUX_CSPI2_SPI_RDY__CSPI2_DATAREADY_B);
	mx31_gpio_mux(MUX_CSPI2_MOSI__CSPI2_MOSI);
	mx31_gpio_mux(MUX_CSPI2_MISO__CSPI2_MISO);
	mx31_gpio_mux(MUX_CSPI2_SS0__CSPI2_SS0_B);
	mx31_gpio_mux(MUX_CSPI2_SS1__CSPI2_SS1_B);

	/* start SPI2 clock */
	__REG(CCM_CGR2) = __REG(CCM_CGR2) | (3 << 4);
}
#endif
