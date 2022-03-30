// SPDX-License-Identifier: GPL-2.0+
/*
 * Microchip PIC32MZ[DA] Starter Kit board
 *
 * Copyright (C) 2015, Microchip Technology Inc.
 * Purna Chandra Mandal <purna.mandal@microchip.com>
 *
 */

#include <common.h>
#include <dm.h>
#include <clk.h>
#include <dt-bindings/clock/microchip,clock.h>
#include <mach/pic32.h>

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	ulong rate;
	struct udevice *dev;
	struct clk clk;
	int ret;

	printf("Core: %s\n", get_core_name());

	if (uclass_get_device(UCLASS_CLK, 0, &dev))
		return 0;

	clk.id = PB7CLK;
	ret = clk_request(dev, &clk);
	if (ret < 0)
		return 0;

	rate = clk_get_rate(&clk);
	printf("CPU Speed: %lu MHz\n", rate / 1000000);

	clk_free(&clk);

	return 0;
}
#endif
