// SPDX-License-Identifier: GPL-2.0+
/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/arch/tegra.h>
#include <asm/arch-tegra/board.h>
#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pinmux.h>
#include <asm/gpio.h>

/* TODO: Remove this code when the SPI switch is working */
#ifndef CONFIG_TARGET_VENTANA
void gpio_early_init_uart(void)
{
	/* Enable UART via GPIO_PI3 (port 8, bit 3) so serial console works */
	gpio_request(TEGRA_GPIO(I, 3), "uart_en");
	gpio_direction_output(TEGRA_GPIO(I, 3), 0);
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
	funcmux_select(PERIPH_ID_SDMMC3, FUNCMUX_SDMMC3_SDB_4BIT);

	/* For power GPIO PI6 */
	pinmux_tristate_disable(PMUX_PINGRP_ATA);
	/* For CD GPIO PI5 */
	pinmux_tristate_disable(PMUX_PINGRP_ATC);
}
#endif

void pin_mux_usb(void)
{
	/* For USB0's GPIO PD0. For now, since we have no pinmux in fdt */
	pinmux_tristate_disable(PMUX_PINGRP_SLXK);
	/* For USB1's ULPI signals */
	funcmux_select(PERIPH_ID_USB2, FUNCMUX_USB2_ULPI);
	pinmux_set_func(PMUX_PINGRP_CDEV2, PMUX_FUNC_PLLP_OUT4);
	pinmux_tristate_disable(PMUX_PINGRP_CDEV2);
	/* USB1 PHY reset GPIO */
	pinmux_tristate_disable(PMUX_PINGRP_UAC);
}
