// SPDX-License-Identifier: GPL-2.0+
/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
 */

#include <common.h>
#include <lcd.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/tegra.h>
#include <asm/gpio.h>

#ifdef CONFIG_MMC_SDHCI_TEGRA
/*
 * Routine: pin_mux_mmc
 * Description: setup the pin muxes/tristate values for the SDMMC(s)
 */
void pin_mux_mmc(void)
{
	funcmux_select(PERIPH_ID_SDMMC4, FUNCMUX_SDMMC4_ATB_GMA_GME_8_BIT);
	funcmux_select(PERIPH_ID_SDMMC2, FUNCMUX_SDMMC2_DTA_DTD_8BIT);

	/* For power GPIO PI6 */
	pinmux_tristate_disable(PMUX_PINGRP_ATA);
	/* For CD GPIO PH2 */
	pinmux_tristate_disable(PMUX_PINGRP_ATD);

	/* For power GPIO PT3 */
	pinmux_tristate_disable(PMUX_PINGRP_DTB);
	/* For CD GPIO PI5 */
	pinmux_tristate_disable(PMUX_PINGRP_ATC);
}
#endif

void pin_mux_usb(void)
{
	funcmux_select(PERIPH_ID_USB2, FUNCMUX_USB2_ULPI);
	pinmux_set_func(PMUX_PINGRP_CDEV2, PMUX_FUNC_PLLP_OUT4);
	pinmux_tristate_disable(PMUX_PINGRP_CDEV2);
	/* USB2 PHY reset GPIO */
	pinmux_tristate_disable(PMUX_PINGRP_UAC);
}

void pin_mux_display(void)
{
	pinmux_set_func(PMUX_PINGRP_SDC, PMUX_FUNC_PWM);
	pinmux_tristate_disable(PMUX_PINGRP_SDC);
}
