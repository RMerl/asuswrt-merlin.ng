// SPDX-License-Identifier: GPL-2.0+
/*
 * SPL/U-Boot common functions for CompuLab CL-SOM-iMX7 module
 *
 * (C) Copyright 2017 CompuLab, Ltd. http://www.compulab.com
 *
 * Author: Uri Mashiach <uri.mashiach@compulab.co.il>
 */

#include <common.h>
#include <fsl_esdhc_imx.h>
#include <asm-generic/gpio.h>
#include "common.h"

#ifdef CONFIG_SPI

#define CL_SOM_IMX7_GPIO_SPI_CS	IMX_GPIO_NR(4, 19)

int board_spi_cs_gpio(unsigned int bus, unsigned int cs)
{
	return CL_SOM_IMX7_GPIO_SPI_CS;
}

#endif /* CONFIG_SPI */

#ifdef CONFIG_FSL_ESDHC_IMX

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC1_BASE_ADDR:
		ret = !gpio_get_value(CL_SOM_IMX7_GPIO_USDHC1_CD);
		break;
	case USDHC3_BASE_ADDR:
		ret = 1; /* Assume uSDHC3 emmc is always present */
		break;
	}

	return ret;
}

#endif /* CONFIG_FSL_ESDHC_IMX */
