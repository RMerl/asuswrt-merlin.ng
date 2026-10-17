// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015  Beckhoff Automation GmbH & Co. KG
 * Patrick Bruenn <p.bruenn@beckhoff.com>
 *
 * Based on <u-boot>/board/freescale/mx53loco/mx53loco_video.c
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/arch/iomux-mx53.h>
#include <asm/gpio.h>
#include <asm/mach-imx/video.h>

#define CX9020_DVI_PWD	IMX_GPIO_NR(6, 1)

struct display_info_t const displays[] = {{
	.bus	= -1,
	.addr	= 0,
	.pixfmt	= IPU_PIX_FMT_RGB24,
	.detect	= NULL,
	.enable	= NULL,
	.mode	= {
		.name           = "DVI",
		.refresh = 60,
		.xres = 640,
		.yres = 480,
		.pixclock = 39721,	/* picosecond (25.175 MHz) */
		.left_margin = 40,
		.right_margin = 60,
		.upper_margin = 10,
		.lower_margin = 10,
		.hsync_len = 20,
		.vsync_len = 10,
		.sync = 0,
		.vmode = FB_VMODE_NONINTERLACED
} } };
size_t display_count = ARRAY_SIZE(displays);

void setup_iomux_lcd(void)
{
	/* Turn on DVI_PWD */
	imx_iomux_v3_setup_pad(MX53_PAD_CSI0_DAT15__GPIO6_1);
	gpio_request(CX9020_DVI_PWD, "CX9020_DVI_PWD");
	gpio_direction_output(CX9020_DVI_PWD, 1);
}
