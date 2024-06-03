// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 * Fabio Estevam <fabio.estevam@freescale.com>
 */

#include <common.h>
#include <linux/list.h>
#include <asm/gpio.h>
#include <asm/arch/iomux-mx51.h>
#include <linux/fb.h>
#include <ipu_pixfmt.h>

#define MX51EVK_LCD_3V3		IMX_GPIO_NR(4, 9)
#define MX51EVK_LCD_5V		IMX_GPIO_NR(4, 10)
#define MX51EVK_LCD_BACKLIGHT	IMX_GPIO_NR(3, 4)

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

static struct fb_videomode const dvi = {
	.name		= "DVI panel",
	.refresh	= 60,
	.xres		= 1024,
	.yres		= 768,
	.pixclock	= 15385,
	.left_margin	= 220,
	.right_margin	= 40,
	.upper_margin	= 21,
	.lower_margin	= 7,
	.hsync_len	= 60,
	.vsync_len	= 10,
	.sync		= 0,
	.vmode		= FB_VMODE_NONINTERLACED
};

void setup_iomux_lcd(void)
{
	/* DI2_PIN15 */
	imx_iomux_v3_setup_pad(MX51_PAD_DI_GP4__DI2_PIN15);

	/* Pad settings for DI2_DISP_CLK */
	imx_iomux_v3_setup_pad(NEW_PAD_CTRL(MX51_PAD_DI2_DISP_CLK__DI2_DISP_CLK,
			    PAD_CTL_PKE | PAD_CTL_DSE_MAX | PAD_CTL_SRE_SLOW));

	/* Turn on 3.3V voltage for LCD */
	imx_iomux_v3_setup_pad(NEW_PAD_CTRL(MX51_PAD_CSI2_D12__GPIO4_9,
						NO_PAD_CTRL));
	gpio_direction_output(MX51EVK_LCD_3V3, 1);

	/* Turn on 5V voltage for LCD */
	imx_iomux_v3_setup_pad(NEW_PAD_CTRL(MX51_PAD_CSI2_D13__GPIO4_10,
						NO_PAD_CTRL));
	gpio_direction_output(MX51EVK_LCD_5V, 1);

	/* Turn on GPIO backlight */
	imx_iomux_v3_setup_pad(NEW_PAD_CTRL(MX51_PAD_DI1_D1_CS__GPIO3_4,
						NO_PAD_CTRL));
	gpio_direction_output(MX51EVK_LCD_BACKLIGHT, 1);
}

int board_video_skip(void)
{
	int ret;
	char const *e = env_get("panel");

	if (e) {
		if (strcmp(e, "claa") == 0) {
			ret = ipuv3_fb_init(&claa_wvga, 1, IPU_PIX_FMT_RGB565);
			if (ret)
				printf("claa cannot be configured: %d\n", ret);
			return ret;
		}
	}

	/*
	 * 'panel' env variable not found or has different value than 'claa'
	 *  Defaulting to dvi output.
	 */
	ret = ipuv3_fb_init(&dvi, 0, IPU_PIX_FMT_RGB24);
	if (ret)
		printf("dvi cannot be configured: %d\n", ret);
	return ret;
}
