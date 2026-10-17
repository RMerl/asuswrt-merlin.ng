// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <adc.h>
#include <asm/io.h>
#include <asm/arch/grf_rk3308.h>
#include <asm/arch-rockchip/hardware.h>
#include <linux/bitops.h>

#if defined(CONFIG_DEBUG_UART)
#define GRF_BASE	0xff000000

enum {
	GPIO1C7_SHIFT		= 8,
	GPIO1C7_MASK		= GENMASK(11, 8),
	GPIO1C7_GPIO		= 0,
	GPIO1C7_UART1_RTSN,
	GPIO1C7_UART2_TX_M0,
	GPIO1C7_SPI2_MOSI,
	GPIO1C7_JTAG_TMS,

	GPIO1C6_SHIFT		= 4,
	GPIO1C6_MASK		= GENMASK(7, 4),
	GPIO1C6_GPIO		= 0,
	GPIO1C6_UART1_CTSN,
	GPIO1C6_UART2_RX_M0,
	GPIO1C6_SPI2_MISO,
	GPIO1C6_JTAG_TCLK,

	GPIO4D3_SHIFT           = 6,
	GPIO4D3_MASK            = GENMASK(7, 6),
	GPIO4D3_GPIO            = 0,
	GPIO4D3_SDMMC_D3,
	GPIO4D3_UART2_TX_M1,

	GPIO4D2_SHIFT           = 4,
	GPIO4D2_MASK            = GENMASK(5, 4),
	GPIO4D2_GPIO            = 0,
	GPIO4D2_SDMMC_D2,
	GPIO4D2_UART2_RX_M1,

	UART2_IO_SEL_SHIFT	= 2,
	UART2_IO_SEL_MASK	= GENMASK(3, 2),
	UART2_IO_SEL_M0		= 0,
	UART2_IO_SEL_M1,
	UART2_IO_SEL_USB,
};

void board_debug_uart_init(void)
{
	static struct rk3308_grf * const grf = (void *)GRF_BASE;

	/* Enable early UART2 channel m0 on the rk3308 */
	rk_clrsetreg(&grf->soc_con5, UART2_IO_SEL_MASK,
		     UART2_IO_SEL_M0 << UART2_IO_SEL_SHIFT);
	rk_clrsetreg(&grf->gpio1ch_iomux,
		     GPIO1C6_MASK | GPIO1C7_MASK,
		     GPIO1C6_UART2_RX_M0 << GPIO1C6_SHIFT |
		     GPIO1C7_UART2_TX_M0 << GPIO1C7_SHIFT);
}
#endif

#define KEY_DOWN_MIN_VAL        0
#define KEY_DOWN_MAX_VAL        30

int rockchip_dnl_key_pressed(void)
{
	unsigned int val;

	if (adc_channel_single_shot("saradc@ff1e0000", 1, &val)) {
		printf("%s read adc key val failed\n", __func__);
		return false;
	}

	if (val >= KEY_DOWN_MIN_VAL && val <= KEY_DOWN_MAX_VAL)
		return true;
	else
		return false;
}
