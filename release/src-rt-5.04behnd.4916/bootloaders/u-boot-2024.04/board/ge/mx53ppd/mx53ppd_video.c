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
#include <dm.h>
#include <linux/list.h>
#include <asm/arch/iomux-mx53.h>
#include <asm/mach-imx/video.h>
#include <linux/fb.h>
#include <ipu_pixfmt.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/io.h>
#include <panel.h>

static int detect_lcd(struct display_info_t const *dev)
{
	return 1;
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
}

static void do_enable_backlight(struct display_info_t const *dev)
{
	struct udevice *panel;
	int ret;

	lcd_enable();

	ret = uclass_get_device(UCLASS_PANEL, 0, &panel);
	if (ret) {
		printf("Could not find panel: %d\n", ret);
		return;
	}

	panel_set_backlight(panel, 100);
	panel_enable_backlight(panel);
}

struct display_info_t const displays[] = {
	{
		.bus	= -1,
		.addr	= -1,
		.pixfmt	= IPU_PIX_FMT_RGB24,
		.detect	= detect_lcd,
		.enable	= do_enable_backlight,
		.mode = {
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
		}
	}
};

size_t display_count = ARRAY_SIZE(displays);
