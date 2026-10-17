// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/grf_rk3066.h>

#define GRF_BASE	0x20008000

const char * const boot_devices[BROM_LAST_BOOTSOURCE + 1] = {
	[BROM_BOOTSOURCE_EMMC] = "/mmc@1021c000",
	[BROM_BOOTSOURCE_SD] = "/mmc@10214000",
};

void board_debug_uart_init(void)
{
	struct rk3066_grf * const grf = (void *)GRF_BASE;

	/* Enable early UART on the RK3066 */
	rk_clrsetreg(&grf->gpio1b_iomux,
		     GPIO1B1_MASK | GPIO1B0_MASK,
		     GPIO1B1_UART2_SOUT << GPIO1B1_SHIFT |
		     GPIO1B0_UART2_SIN << GPIO1B0_SHIFT);
}

void spl_board_init(void)
{
	if (!IS_ENABLED(CONFIG_SPL_BUILD))
		return;

	if (IS_ENABLED(CONFIG_SPL_DM_MMC)) {
		struct rk3066_grf * const grf = (void *)GRF_BASE;

		rk_clrsetreg(&grf->gpio3b_iomux,
			     GPIO3B0_MASK | GPIO3B1_MASK | GPIO3B2_MASK |
			     GPIO3B3_MASK | GPIO3B4_MASK | GPIO3B5_MASK |
			     GPIO3B6_MASK,
			     GPIO3B0_SDMMC0_CLKOUT << GPIO3B0_SHIFT |
			     GPIO3B1_SDMMC0_CMD    << GPIO3B1_SHIFT |
			     GPIO3B2_SDMMC0_DATA0  << GPIO3B2_SHIFT |
			     GPIO3B3_SDMMC0_DATA1  << GPIO3B3_SHIFT |
			     GPIO3B4_SDMMC0_DATA2  << GPIO3B4_SHIFT |
			     GPIO3B5_SDMMC0_DATA3  << GPIO3B5_SHIFT |
			     GPIO3B6_SDMMC0_DECTN  << GPIO3B6_SHIFT);
	}
}
