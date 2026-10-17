// SPDX-License-Identifier: GPL-2.0+
/*
 * Olimex MX23 Olinuxino board
 *
 * Copyright (C) 2013 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/iomux-mx23.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#ifdef CONFIG_LED_STATUS
#include <status_led.h>
#endif
#include <linux/delay.h>

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

	return 0;
}

#ifdef CONFIG_CMD_USB
int board_ehci_hcd_init(int port)
{
	/* Enable LAN9512 (Maxi) or GL850G (Mini) USB HUB power. */
	gpio_direction_output(MX23_PAD_GPMI_ALE__GPIO_0_17, 1);
	udelay(100);
	return 0;
}

int board_ehci_hcd_exit(int port)
{
	/* Enable LAN9512 (Maxi) or GL850G (Mini) USB HUB power. */
	gpio_direction_output(MX23_PAD_GPMI_ALE__GPIO_0_17, 0);
	return 0;
}
#endif

int dram_init(void)
{
	return mxs_dram_init();
}

int board_init(void)
{
	/* Adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

#if defined(CONFIG_LED_STATUS) && defined(CONFIG_LED_STATUS_BOOT_ENABLE)
	status_led_set(CONFIG_LED_STATUS_BOOT, CONFIG_LED_STATUS_STATE);
#endif

	return 0;
}
