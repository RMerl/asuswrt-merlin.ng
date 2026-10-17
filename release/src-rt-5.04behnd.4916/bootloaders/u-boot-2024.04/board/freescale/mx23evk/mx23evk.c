// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale MX23EVK board
 *
 * (C) Copyright 2013 O.S. Systems Software LTDA.
 *
 * Author: Otavio Salvador <otavio@ossystems.com.br>
 *
 * Based on m28evk.c:
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 */

#include <common.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/iomux-mx23.h>
#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Functions
 */
int board_early_init_f(void)
{
	/* IO0 clock at 480MHz */
	mxs_set_ioclk(MXC_IOCLK0, 480000);

	/* SSP0 clock at 96MHz */
	mxs_set_sspclk(MXC_SSPCLK0, 96000, 0);

	/* Power on LCD */
	gpio_direction_output(MX23_PAD_LCD_RESET__GPIO_1_18, 1);

	/* Set contrast to maximum */
	gpio_direction_output(MX23_PAD_PWM2__GPIO_1_28, 1);

	return 0;
}

int dram_init(void)
{
	return mxs_dram_init();
}

int board_init(void)
{
	/* Adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}
