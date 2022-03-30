/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *
 * (c) 2009 Ilya Yanok, Emcraft Systems <yanok@emcraft.com>
 *
 * Modified for mx25 by John Rigby <jrigby@gmail.com>
 */

#ifndef __ASM_ARCH_CLOCK_H
#define __ASM_ARCH_CLOCK_H

#include <common.h>

#ifdef CONFIG_MX25_HCLK_FREQ
#define MXC_HCLK	CONFIG_MX25_HCLK_FREQ
#else
#define MXC_HCLK	24000000
#endif

#ifdef CONFIG_MX25_CLK32
#define MXC_CLK32	CONFIG_MX25_CLK32
#else
#define MXC_CLK32	32768
#endif

enum mxc_clock {
	/* PER clocks (do not change order) */
	MXC_CSI_CLK,
	MXC_EPIT_CLK,
	MXC_ESAI_CLK,
	MXC_ESDHC1_CLK,
	MXC_ESDHC2_CLK,
	MXC_GPT_CLK,
	MXC_I2C_CLK,
	MXC_LCDC_CLK,
	MXC_NFC_CLK,
	MXC_OWIRE_CLK,
	MXC_PWM_CLK,
	MXC_SIM1_CLK,
	MXC_SIM2_CLK,
	MXC_SSI1_CLK,
	MXC_SSI2_CLK,
	MXC_UART_CLK,
	/* Other clocks */
	MXC_ARM_CLK,
	MXC_AHB_CLK,
	MXC_IPG_CLK,
	MXC_CSPI_CLK,
	MXC_FEC_CLK,
	MXC_CLK_NUM
};

int imx_set_perclk(enum mxc_clock clk, bool from_upll, unsigned int freq);
unsigned int mxc_get_clock(enum mxc_clock clk);

#define imx_get_uartclk()	mxc_get_clock(MXC_UART_CLK)
#define imx_get_fecclk()	mxc_get_clock(MXC_FEC_CLK)

#endif /* __ASM_ARCH_CLOCK_H */
