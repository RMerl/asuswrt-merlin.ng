// SPDX-License-Identifier: GPL-2.0+
/*
 *  (C) Copyright 2010-2012
 *  NVIDIA Corporation <www.nvidia.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/tegra.h>
#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/pinmux.h>
#include <asm/gpio.h>
#include <i2c.h>

void pin_mux_usb(void)
{
	/*
	 * USB1 internal/external mux GPIO, which masquerades as a VBUS GPIO
	 * in the current device tree.
	 */
	pinmux_tristate_disable(PMUX_PINGRP_UAC);
}

void pin_mux_spi(void)
{
	funcmux_select(PERIPH_ID_SPI1, FUNCMUX_SPI1_GMC_GMD);
}

/*
 * Routine: pin_mux_mmc
 * Description: setup the pin muxes/tristate values for the SDMMC(s)
 */
void pin_mux_mmc(void)
{
	funcmux_select(PERIPH_ID_SDMMC1, FUNCMUX_SDMMC1_SDIO1_4BIT);
	funcmux_select(PERIPH_ID_SDMMC4, FUNCMUX_SDMMC4_ATB_GMA_4_BIT);

	/* For CD GPIO PP1 */
	pinmux_tristate_disable(PMUX_PINGRP_DAP3);
}
