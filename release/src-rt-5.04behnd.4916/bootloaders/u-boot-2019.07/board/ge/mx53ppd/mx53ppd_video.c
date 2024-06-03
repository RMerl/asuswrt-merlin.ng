// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 General Electric Company
 *
 * Based on board/freescale/mx53loco/mx53loco_video.c:
 *
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 * Fabio Estevam <fabio.estevam@freescale.com>
 */

#include <common.h>
#include <linux/list.h>
#include <asm/gpio.h>
#include <asm/arch/iomux-mx53.h>
#include <linux/fb.h>
#include <ipu_pixfmt.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/io.h>
#include <pwm.h>
#include "ppd_gpio.h"

#define MX53PPD_LCD_POWER		IMX_GPIO_NR(3, 24)

static struct fb_videomode const nv_spwg = {
	.name		= "NV-SPWGRGB888",
	.refresh	= 60,
	.xres		= 800,
	.yres		= 480,
	.pixclock	= 15384,
	.left_margin	= 16,
	.right_margin	= 210,
	.upper_margin	= 10,
	.lower_margin	= 22,
	.hsync_len	= 30,
	.vsync_len	= 13,
	.sync		= FB_SYNC_EXT,
	.vmode		= FB_VMODE_NONINTERLACED
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
}

static void lcd_enable(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;

	/* Set LDB_DI0 as clock source for IPU_DI0 */
	clrsetbits_le32(&mxc_ccm->cscmr2,
			MXC_CCM_CSCMR2_DI0_CLK_SEL_MASK,
			MXC_CCM_CSCMR2_DI0_CLK_SEL(
				MXC_CCM_CSCMR2_DI0_CLK_SEL_LDB_DI0_CLK));

	/* Turn on IPU LDB DI0 clocks */
	setbits_le32(&mxc_ccm->CCGR6, MXC_CCM_CCGR6_LDB_DI0(3));

	/* Turn on IPU DI0 clocks */
	setbits_le32(&mxc_ccm->CCGR6, MXC_CCM_CCGR6_IPU_DI0(3));

	/* Configure LDB */
	writel(IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG |
		IOMUXC_GPR2_DATA_WIDTH_CH0_24BIT |
		IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI0,
		&iomux->gpr[2]);

	/* Enable backlights  */
	pwm_init(1, 0, 0);

	/* duty cycle 5000000ns, period: 5000000ns */
	pwm_config(1, 5000000, 5000000);

	/* Backlight Power */
	gpio_direction_output(BACKLIGHT_ENABLE, 1);

	pwm_enable(1);
}

static int do_lcd_enable(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{
	lcd_enable();
	return 0;
}

U_BOOT_CMD(
	ppd_lcd_enable,	1,	1,	do_lcd_enable,
	"enable PPD LCD",
	"no parameters"
);

int board_video_skip(void)
{
	int ret;

	ret = ipuv3_fb_init(&nv_spwg, 0, IPU_PIX_FMT_RGB24);
	if (ret)
		printf("Display cannot be configured: %d\n", ret);

	return ret;
}
