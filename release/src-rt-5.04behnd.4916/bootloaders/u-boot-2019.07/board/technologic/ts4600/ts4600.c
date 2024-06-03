// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Savoir-faire Linux Inc.
 *
 * Author: Sebastien Bourdelin <sebastien.bourdelin@savoirfairelinux.com>
 *
 * Based on work from TS7680 code by:
 *   Kris Bahnsen <kris@embeddedarm.com>
 *   Mark Featherston <mark@embeddedarm.com>
 *   https://github.com/embeddedarm/u-boot/tree/master/board/technologic/ts7680
 *
 * Derived from MX28EVK code by
 *   Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux-mx28.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <linux/mii.h>
#include <miiphy.h>
#include <netdev.h>
#include <errno.h>

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	/* IO0 clock at 480MHz */
	mxs_set_ioclk(MXC_IOCLK0, 480000);
	/* IO1 clock at 480MHz */
	mxs_set_ioclk(MXC_IOCLK1, 480000);

	/* SSP0 clocks at 96MHz */
	mxs_set_sspclk(MXC_SSPCLK0, 96000, 0);

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

#ifdef CONFIG_CMD_MMC
static int ts4600_mmc_cd(int id)
{
	return 1;
}

int board_mmc_init(bd_t *bis)
{
	int ret;

	mxs_iomux_setup_pad(MX28_PAD_PWM3__GPIO_3_28);

	/* Power-on SD */
	gpio_direction_output(MX28_PAD_PWM3__GPIO_3_28, 1);
	udelay(1000);
	gpio_direction_output(MX28_PAD_PWM3__GPIO_3_28, 0);

	/* SD card */
	ret = mxsmmc_initialize(bis, 0, NULL, ts4600_mmc_cd);
	if(ret != 0) {
		printf("SD controller initialized with %d\n", ret);
	}

	return ret;
}
#endif

int checkboard(void)
{
	puts("Board: TS4600\n");

	return 0;
}
