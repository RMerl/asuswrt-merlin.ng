// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Armadeus Systems
 */

#include <common.h>
#include <init.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/io.h>

#ifdef CONFIG_VIDEO_MXS
int setup_lcd(void)
{
	struct gpio_desc backlight;
	int ret;

	/* Set Brightness to high */
	ret = dm_gpio_lookup_name("GPIO4_10", &backlight);
	if (ret) {
		printf("Cannot get GPIO4_10\n");
		return ret;
	}

	ret = dm_gpio_request(&backlight, "backlight");
	if (ret) {
		printf("Cannot request GPIO4_10\n");
		return ret;
	}

	dm_gpio_set_dir_flags(&backlight, GPIOD_IS_OUT);
	dm_gpio_set_value(&backlight, 1);

	return 0;
}
#endif

int opos6ul_board_late_init(void)
{
#ifdef CONFIG_VIDEO_MXS
	setup_lcd();
#endif

	return 0;
}
