/*
 * Board support file for Nokia N900 (aka RX-51).
 *
 * Copyright (C) 2007, 2008 Nokia
 * Copyright (C) 2012 Ivaylo Dimitrov <freemangordon@abv.bg>
 * Copyright (C) 2013 Pali Rohár <pali.rohar@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/leds.h>
#include <linux/usb/phy.h>
#include <linux/usb/musb.h>
#include <linux/platform_data/spi-omap2-mcspi.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <linux/omap-dma.h>

#include "common.h"
#include "mux.h"
#include "gpmc.h"
#include "pm.h"
#include "soc.h"
#include "sdram-nokia.h"
#include "omap-secure.h"

#define RX51_GPIO_SLEEP_IND 162

static struct gpio_led gpio_leds[] = {
	{
		.name	= "sleep_ind",
		.gpio	= RX51_GPIO_SLEEP_IND,
	},
};

static struct gpio_led_platform_data gpio_led_info = {
	.leds		= gpio_leds,
	.num_leds	= ARRAY_SIZE(gpio_leds),
};

static struct platform_device leds_gpio = {
	.name	= "leds-gpio",
	.id	= -1,
	.dev	= {
		.platform_data	= &gpio_led_info,
	},
};

/*
 * cpuidle C-states definition for rx51.
 *
 * The 'exit_latency' field is the sum of sleep
 * and wake-up latencies.

    ---------------------------------------------
   | state |  exit_latency  |  target_residency  |
    ---------------------------------------------
   |  C1   |    110 + 162   |            5       |
   |  C2   |    106 + 180   |          309       |
   |  C3   |    107 + 410   |        46057       |
   |  C4   |    121 + 3374  |        46057       |
   |  C5   |    855 + 1146  |        46057       |
   |  C6   |   7580 + 4134  |       484329       |
   |  C7   |   7505 + 15274 |       484329       |
    ---------------------------------------------

*/

extern void __init rx51_peripherals_init(void);

#ifdef CONFIG_OMAP_MUX
static struct omap_board_mux board_mux[] __initdata = {
	{ .reg_offset = OMAP_MUX_TERMINATOR },
};
#endif

static struct omap_musb_board_data musb_board_data = {
	.interface_type		= MUSB_INTERFACE_ULPI,
	.mode			= MUSB_OTG,
	.power			= 0,
};

static void __init rx51_init(void)
{
	struct omap_sdrc_params *sdrc_params;

	omap3_mux_init(board_mux, OMAP_PACKAGE_CBB);
	omap_serial_init();

	sdrc_params = nokia_get_sdram_timings();
	omap_sdrc_init(sdrc_params, sdrc_params);

	usb_bind_phy("musb-hdrc.0.auto", 0, "twl4030_usb");
	usb_musb_init(&musb_board_data);
	rx51_peripherals_init();

	if (omap_type() == OMAP2_DEVICE_TYPE_SEC) {
#ifdef CONFIG_ARM_ERRATA_430973
		pr_info("RX-51: Enabling ARM errata 430973 workaround\n");
		/* set IBE to 1 */
		rx51_secure_update_aux_cr(BIT(6), 0);
#endif
	}

	/* Ensure SDRC pins are mux'd for self-refresh */
	omap_mux_init_signal("sdrc_cke0", OMAP_PIN_OUTPUT);
	omap_mux_init_signal("sdrc_cke1", OMAP_PIN_OUTPUT);

	platform_device_register(&leds_gpio);
}

static void __init rx51_reserve(void)
{
	omap_reserve();
}

MACHINE_START(NOKIA_RX51, "Nokia RX-51 board")
	/* Maintainer: Lauri Leukkunen <lauri.leukkunen@nokia.com> */
	.atag_offset	= 0x100,
	.reserve	= rx51_reserve,
	.map_io		= omap3_map_io,
	.init_early	= omap3430_init_early,
	.init_irq	= omap3_init_irq,
	.init_machine	= rx51_init,
	.init_late	= omap3430_init_late,
	.init_time	= omap3_sync32k_timer_init,
	.restart	= omap3xxx_restart,
MACHINE_END
