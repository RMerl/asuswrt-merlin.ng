// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 * Fabio Estevam <fabio.estevam@freescale.com>
 */

#include <common.h>
#include <linux/list.h>
#include <asm/gpio.h>
#include <asm/arch/iomux-mx53.h>
#include <linux/fb.h>
#include <ipu_pixfmt.h>

#define MX53LOCO_LCD_POWER		IMX_GPIO_NR(3, 24)

static struct fb_videomode const claa_wvga = {
	.name		= "CLAA07LC0ACW",
	.refresh	= 57,
	.xres		= 800,
	.yres		= 480,
	.pixclock	= 37037,
	.left_margin	= 40,
	.right_margin	= 60,
	.upper_margin	= 10,
	.lower_margin	= 10,
	.hsync_len	= 20,
	.vsync_len	= 10,
	.sync		= 0,
	.vmode		= FB_VMODE_NONINTERLACED
};

static struct fb_videomode const seiko_wvga = {
	.name		= "Seiko-43WVF1G",
	.refresh	= 60,
	.xres		= 800,
	.yres		= 480,
	.pixclock	= 29851, /* picosecond (33.5 MHz) */
	.left_margin	= 89,
	.right_margin	= 164,
	.upper_margin	= 23,
	.lower_margin	= 10,
	.hsync_len	= 10,
	.vsync_len	= 10,
	.sync		= 0,
};

void setup_iomux_lcd(void)
{
	static const iomux_v3_cfg_t lcd_pads[] = {
		MX53_PAD_DI0_DISP_CLK__IPU_DI0_DISP_CLK,
		MX53_PAD_DI0_PIN15__IPU_DI0_PIN15,
		MX53_PAD_DI0_PIN2__IPU_DI0_PIN2,
		MX53_PAD_DI0_PIN3__IPU_DI0_PIN3,
		MX53_PAD_DISP0_DAT0__IPU_DISP0_DAT_0,
		MX53_PAD_DISP0_DAT1__IPU_DISP0_DAT_1,
		MX53_PAD_DISP0_DAT2__IPU_DISP0_DAT_2,
		MX53_PAD_DISP0_DAT3__IPU_DISP0_DAT_3,
		MX53_PAD_DISP0_DAT4__IPU_DISP0_DAT_4,
		MX53_PAD_DISP0_DAT5__IPU_DISP0_DAT_5,
		MX53_PAD_DISP0_DAT6__IPU_DISP0_DAT_6,
		MX53_PAD_DISP0_DAT7__IPU_DISP0_DAT_7,
		MX53_PAD_DISP0_DAT8__IPU_DISP0_DAT_8,
		MX53_PAD_DISP0_DAT9__IPU_DISP0_DAT_9,
		MX53_PAD_DISP0_DAT10__IPU_DISP0_DAT_10,
		MX53_PAD_DISP0_DAT11__IPU_DISP0_DAT_11,
		MX53_PAD_DISP0_DAT12__IPU_DISP0_DAT_12,
		MX53_PAD_DISP0_DAT13__IPU_DISP0_DAT_13,
		MX53_PAD_DISP0_DAT14__IPU_DISP0_DAT_14,
		MX53_PAD_DISP0_DAT15__IPU_DISP0_DAT_15,
		MX53_PAD_DISP0_DAT16__IPU_DISP0_DAT_16,
		MX53_PAD_DISP0_DAT17__IPU_DISP0_DAT_17,
		MX53_PAD_DISP0_DAT18__IPU_DISP0_DAT_18,
		MX53_PAD_DISP0_DAT19__IPU_DISP0_DAT_19,
		MX53_PAD_DISP0_DAT20__IPU_DISP0_DAT_20,
		MX53_PAD_DISP0_DAT21__IPU_DISP0_DAT_21,
		MX53_PAD_DISP0_DAT22__IPU_DISP0_DAT_22,
		MX53_PAD_DISP0_DAT23__IPU_DISP0_DAT_23,
	};

	imx_iomux_v3_setup_multiple_pads(lcd_pads, ARRAY_SIZE(lcd_pads));

	/* Turn on GPIO backlight */
	imx_iomux_v3_setup_pad(MX53_PAD_EIM_D24__GPIO3_24);
	gpio_direction_output(MX53LOCO_LCD_POWER, 1);

	/* Turn on display contrast */
	imx_iomux_v3_setup_pad(MX53_PAD_GPIO_1__GPIO1_1);
	gpio_direction_output(IMX_GPIO_NR(1, 1), 1);
}

int board_video_skip(void)
{
	int ret;
	char const *e = env_get("panel");

	if (e) {
		if (strcmp(e, "seiko") == 0) {
			ret = ipuv3_fb_init(&seiko_wvga, 0, IPU_PIX_FMT_RGB24);
			if (ret)
				printf("Seiko cannot be configured: %d\n", ret);
			return ret;
		}
	}

	/*
	 * 'panel' env variable not found or has different value than 'seiko'
	 *  Defaulting to claa lcd.
	 */
	ret = ipuv3_fb_init(&claa_wvga, 0, IPU_PIX_FMT_RGB565);
	if (ret)
		printf("CLAA cannot be configured: %d\n", ret);
	return ret;
}
