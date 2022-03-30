// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 *
 * FSL DCU Framebuffer driver
 */

#include <common.h>
#include <fsl_dcu_fb.h>
#include "div64.h"
#include "../common/dcu_sii9022a.h"

DECLARE_GLOBAL_DATA_PTR;

unsigned int dcu_set_pixel_clock(unsigned int pixclock)
{
	unsigned long long div;

	div = (unsigned long long)(gd->bus_clk / 1000);
	div *= (unsigned long long)pixclock;
	do_div(div, 1000000000);

	return div;
}

int platform_dcu_init(unsigned int xres, unsigned int yres,
		const char *port,
		struct fb_videomode *dcu_fb_videomode)
{
	const char *name;
	unsigned int pixel_format;

	if (strncmp(port, "twr_lcd", 4) == 0) {
		name = "TWR_LCD_RGB card";
	} else {
		name = "HDMI";
		dcu_set_dvi_encoder(dcu_fb_videomode);
	}

	printf("DCU: Switching to %s monitor @ %ux%u\n", name, xres, yres);

	pixel_format = 32;
	fsl_dcu_init(xres, yres, pixel_format);

	return 0;
}
