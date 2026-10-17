// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <init.h>
#include <log.h>
#include <nuvoton_nct6102d.h>
#include <asm/gpio.h>
#include <asm/ibmpc.h>
#include <asm/pnp_def.h>
#include <linux/delay.h>

int board_early_init_f(void)
{
#ifdef CONFIG_INTERNAL_UART
	/* Disable the legacy UART which is enabled per default */
	nct6102d_uarta_disable();
#else
	/*
	 * The FSP enables the BayTrail internal legacy UART (again).
	 * Disable it again, so that the Nuvoton one can be used.
	 */
	setup_internal_uart(0);
#endif

	/* Disable the watchdog which is enabled per default */
	nct6102d_wdt_disable();

	return 0;
}

int board_late_init(void)
{
	struct gpio_desc desc;
	int ret;

	ret = dm_gpio_lookup_name("F10", &desc);
	if (ret)
		debug("gpio ret=%d\n", ret);
	ret = dm_gpio_request(&desc, "xhci_hub_reset");
	if (ret)
		debug("gpio_request ret=%d\n", ret);
	ret = dm_gpio_set_dir_flags(&desc, GPIOD_IS_OUT);
	if (ret)
		debug("gpio dir ret=%d\n", ret);

	/* Pull xHCI hub reset to low (active low) */
	dm_gpio_set_value(&desc, 0);

	/* Wait at least 5 ms, so lets choose 10 to be safe */
	mdelay(10);

	/* Pull xHCI hub reset to high (active low) */
	dm_gpio_set_value(&desc, 1);

	return 0;
}
