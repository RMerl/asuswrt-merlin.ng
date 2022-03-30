// SPDX-License-Identifier: GPL-2.0+
/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
 *  (C) Copyright 2011-2012
 *  Avionic Design GmbH <www.avionic-design.de>
 */

#include <common.h>
#include <ns16550.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/tegra.h>
#include <asm/arch-tegra/board.h>
#include <asm/arch-tegra/clk_rst.h>
#include <asm/arch-tegra/sys_proto.h>
#include <asm/arch-tegra/uart.h>

#ifdef CONFIG_BOARD_EARLY_INIT_F
void gpio_early_init(void)
{
	gpio_request(TEGRA_GPIO(I, 4), NULL);
	gpio_direction_output(TEGRA_GPIO(I, 4), 1);
}
#endif

#ifdef CONFIG_MMC_SDHCI_TEGRA
/*
 * Routine: pin_mux_mmc
 * Description: setup the pin muxes/tristate values for the SDMMC(s)
 */
void pin_mux_mmc(void)
{
	funcmux_select(PERIPH_ID_SDMMC4, FUNCMUX_SDMMC4_ATB_GMA_GME_8_BIT);
	/* for write-protect GPIO PI6 */
	pinmux_tristate_disable(PMUX_PINGRP_ATA);
	/* for CD GPIO PH2 */
	pinmux_tristate_disable(PMUX_PINGRP_ATD);
}
#endif
