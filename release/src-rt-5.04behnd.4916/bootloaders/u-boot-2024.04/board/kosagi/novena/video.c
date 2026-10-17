// SPDX-License-Identifier: GPL-2.0+
/*
 * Novena video output support
 *
 * IT6251 code based on code Copyright (C) 2014 Sean Cross
 * from https://github.com/xobs/novena-linux.git commit
 * 3d85836ee1377d445531928361809612aa0a18db
 *
 * Copyright (C) 2014 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <log.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/mach-imx/video.h>
#include <i2c.h>
#include <input.h>
#include <ipu_pixfmt.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <malloc.h>
#include <stdio_dev.h>

#include "novena.h"

#define IT6251_VENDOR_ID_LOW				0x00
#define IT6251_VENDOR_ID_HIGH				0x01
#define IT6251_DEVICE_ID_LOW				0x02
#define IT6251_DEVICE_ID_HIGH				0x03
#define IT6251_SYSTEM_STATUS				0x0d
#define IT6251_SYSTEM_STATUS_RINTSTATUS			(1 << 0)
#define IT6251_SYSTEM_STATUS_RHPDSTATUS			(1 << 1)
#define IT6251_SYSTEM_STATUS_RVIDEOSTABLE		(1 << 2)
#define IT6251_SYSTEM_STATUS_RPLL_IOLOCK		(1 << 3)
#define IT6251_SYSTEM_STATUS_RPLL_XPLOCK		(1 << 4)
#define IT6251_SYSTEM_STATUS_RPLL_SPLOCK		(1 << 5)
#define IT6251_SYSTEM_STATUS_RAUXFREQ_LOCK		(1 << 6)
#define IT6251_REF_STATE				0x0e
#define IT6251_REF_STATE_MAIN_LINK_DISABLED		(1 << 0)
#define IT6251_REF_STATE_AUX_CHANNEL_READ		(1 << 1)
#define IT6251_REF_STATE_CR_PATTERN			(1 << 2)
#define IT6251_REF_STATE_EQ_PATTERN			(1 << 3)
#define IT6251_REF_STATE_NORMAL_OPERATION		(1 << 4)
#define IT6251_REF_STATE_MUTED				(1 << 5)

#define IT6251_REG_PCLK_CNT_LOW				0x57
#define IT6251_REG_PCLK_CNT_HIGH			0x58

#define IT6521_RETRY_MAX				20

static int it6251_is_stable(void)
{
	const unsigned int caddr = NOVENA_IT6251_CHIPADDR;
	const unsigned int laddr = NOVENA_IT6251_LVDSADDR;
	int status;
	int clkcnt;
	int rpclkcnt;
	int refstate;

	rpclkcnt = (i2c_reg_read(caddr, 0x13) & 0xff) |
		   ((i2c_reg_read(caddr, 0x14) << 8) & 0x0f00);
	debug("RPCLKCnt: %d\n", rpclkcnt);

	status = i2c_reg_read(caddr, IT6251_SYSTEM_STATUS);
	debug("System status: 0x%02x\n", status);

	clkcnt = (i2c_reg_read(laddr, IT6251_REG_PCLK_CNT_LOW) & 0xff) |
		 ((i2c_reg_read(laddr, IT6251_REG_PCLK_CNT_HIGH) << 8) &
		  0x0f00);
	debug("Clock: 0x%02x\n", clkcnt);

	refstate = i2c_reg_read(laddr, IT6251_REF_STATE);
	debug("Ref Link State: 0x%02x\n", refstate);

	if ((refstate & 0x1f) != 0)
		return 0;

	/* If video is muted, that's a failure */
	if (refstate & IT6251_REF_STATE_MUTED)
		return 0;

	if (!(status & IT6251_SYSTEM_STATUS_RVIDEOSTABLE))
		return 0;

	return 1;
}

static int it6251_ready(void)
{
	const unsigned int caddr = NOVENA_IT6251_CHIPADDR;

	/* Test if the IT6251 came out of reset by reading ID regs. */
	if (i2c_reg_read(caddr, IT6251_VENDOR_ID_LOW) != 0x15)
		return 0;
	if (i2c_reg_read(caddr, IT6251_VENDOR_ID_HIGH) != 0xca)
		return 0;
	if (i2c_reg_read(caddr, IT6251_DEVICE_ID_LOW) != 0x51)
		return 0;
	if (i2c_reg_read(caddr, IT6251_DEVICE_ID_HIGH) != 0x62)
		return 0;

	return 1;
}

static void it6251_program_regs(void)
{
	const unsigned int caddr = NOVENA_IT6251_CHIPADDR;
	const unsigned int laddr = NOVENA_IT6251_LVDSADDR;

	i2c_reg_write(caddr, 0x05, 0x00);
	mdelay(1);

	/* set LVDSRX address, and enable */
	i2c_reg_write(caddr, 0xfd, 0xbc);
	i2c_reg_write(caddr, 0xfe, 0x01);

	/*
	 * LVDSRX
	 */
	/* This write always fails, because the chip goes into reset */
	/* reset LVDSRX */
	i2c_reg_write(laddr, 0x05, 0xff);
	i2c_reg_write(laddr, 0x05, 0x00);

	/* reset LVDSRX PLL */
	i2c_reg_write(laddr, 0x3b, 0x42);
	i2c_reg_write(laddr, 0x3b, 0x43);

	/* something with SSC PLL */
	i2c_reg_write(laddr, 0x3c, 0x08);
	/* don't swap links, but writing reserved registers */
	i2c_reg_write(laddr, 0x0b, 0x88);

	/* JEIDA, 8-bit depth  0x11, orig 0x42 */
	i2c_reg_write(laddr, 0x2c, 0x01);
	/* "reserved" */
	i2c_reg_write(laddr, 0x32, 0x04);
	/* "reserved" */
	i2c_reg_write(laddr, 0x35, 0xe0);
	/* "reserved" + clock delay */
	i2c_reg_write(laddr, 0x2b, 0x24);

	/* reset LVDSRX pix clock */
	i2c_reg_write(laddr, 0x05, 0x02);
	i2c_reg_write(laddr, 0x05, 0x00);

	/*
	 * DPTX
	 */
	/* set for two lane mode, normal op, no swapping, no downspread */
	i2c_reg_write(caddr, 0x16, 0x02);

	/* some AUX channel EDID magic */
	i2c_reg_write(caddr, 0x23, 0x40);

	/* power down lanes 3-0 */
	i2c_reg_write(caddr, 0x5c, 0xf3);

	/* enable DP scrambling, change EQ CR phase */
	i2c_reg_write(caddr, 0x5f, 0x06);

	/* color mode RGB, pclk/2 */
	i2c_reg_write(caddr, 0x60, 0x02);
	/* dual pixel input mode, no EO swap, no RGB swap */
	i2c_reg_write(caddr, 0x61, 0x04);
	/* M444B24 video format */
	i2c_reg_write(caddr, 0x62, 0x01);

	/* vesa range / not interlace / vsync high / hsync high */
	i2c_reg_write(caddr, 0xa0, 0x0F);

	/* hpd event timer set to 1.6-ish ms */
	i2c_reg_write(caddr, 0xc9, 0xf5);

	/* more reserved magic */
	i2c_reg_write(caddr, 0xca, 0x4d);
	i2c_reg_write(caddr, 0xcb, 0x37);

	/* enhanced framing mode, auto video fifo reset, video mute disable */
	i2c_reg_write(caddr, 0xd3, 0x03);

	/* "vidstmp" and some reserved stuff */
	i2c_reg_write(caddr, 0xd4, 0x45);

	/* queue number -- reserved */
	i2c_reg_write(caddr, 0xe7, 0xa0);
	/* info frame packets  and reserved */
	i2c_reg_write(caddr, 0xe8, 0x33);
	/* more AVI stuff */
	i2c_reg_write(caddr, 0xec, 0x00);

	/* select PC master reg for aux channel? */
	i2c_reg_write(caddr, 0x23, 0x42);

	/* send PC request commands */
	i2c_reg_write(caddr, 0x24, 0x00);
	i2c_reg_write(caddr, 0x25, 0x00);
	i2c_reg_write(caddr, 0x26, 0x00);

	/* native aux read */
	i2c_reg_write(caddr, 0x2b, 0x00);
	/* back to internal */
	i2c_reg_write(caddr, 0x23, 0x40);

	/* voltage swing level 3 */
	i2c_reg_write(caddr, 0x19, 0xff);
	/* pre-emphasis level 3 */
	i2c_reg_write(caddr, 0x1a, 0xff);

	/* start link training */
	i2c_reg_write(caddr, 0x17, 0x01);
}

static int it6251_init(void)
{
	const unsigned int caddr = NOVENA_IT6251_CHIPADDR;
	int reg;
	int tries, retries = 0;

	for (retries = 0; retries < IT6521_RETRY_MAX; retries++) {
		/* Program the chip. */
		it6251_program_regs();

		/* Wait for video stable. */
		for (tries = 0; tries < 100; tries++) {
			reg = i2c_reg_read(caddr, 0x17);
			/* Test Link CFG, STS, LCS read done. */
			if ((reg & 0xe0) != 0xe0) {
				/* Not yet, wait a bit more. */
				mdelay(2);
				continue;
			}

			/* Test if the video input is stable. */
			if (it6251_is_stable())
				return 0;
		}
		/*
		 * If we couldn't stabilize, requeue and try again,
		 * because it means that the LVDS channel isn't
		 * stable yet.
		 */
		printf("Display didn't stabilize.\n");
		printf("This may be because the LVDS port is still in powersave mode.\n");
		mdelay(50);
	}

	return -EINVAL;
}

static void enable_hdmi(struct display_info_t const *dev)
{
	imx_enable_hdmi_phy();
}

static int lvds_enabled;

static void enable_lvds(struct display_info_t const *dev)
{
	if (lvds_enabled)
		return;

	/* ITE IT6251 power enable. */
	gpio_request(NOVENA_ITE6251_PWR_GPIO, "ite6251-power");
	gpio_direction_output(NOVENA_ITE6251_PWR_GPIO, 0);
	mdelay(10);
	gpio_direction_output(NOVENA_ITE6251_PWR_GPIO, 1);
	mdelay(20);
	lvds_enabled = 1;
}

static int detect_lvds(struct display_info_t const *dev)
{
	int ret, loops = 250;

	enable_lvds(dev);

	ret = i2c_set_bus_num(NOVENA_IT6251_I2C_BUS);
	if (ret) {
		puts("Cannot select IT6251 I2C bus.\n");
		return 0;
	}

	/* Wait up-to ~250 mS for the LVDS to come up. */
	while (--loops) {
		ret = it6251_ready();
		if (ret)
			return ret;

		mdelay(1);
	}

	return 0;
}

struct display_info_t const displays[] = {
	{
		/* HDMI Output */
		.bus	= -1,
		.addr	= 0,
		.pixfmt	= IPU_PIX_FMT_RGB24,
		.detect	= detect_hdmi,
		.enable	= enable_hdmi,
		.mode	= {
			.name		= "HDMI",
			.refresh	= 60,
			.xres		= 1024,
			.yres		= 768,
			.pixclock	= 15384,
			.left_margin	= 220,
			.right_margin	= 40,
			.upper_margin	= 21,
			.lower_margin	= 7,
			.hsync_len	= 60,
			.vsync_len	= 10,
			.sync		= FB_SYNC_EXT,
			.vmode		= FB_VMODE_NONINTERLACED
		},
	}, {
		/* LVDS Output: N133HSE-EA1 Rev. C1 */
		.bus	= -1,
		.pixfmt	= IPU_PIX_FMT_RGB24,
		.detect	= detect_lvds,
		.enable	= enable_lvds,
		.mode	= {
			.name		= "Chimei-FHD",
			.refresh	= 60,
			.xres		= 1920,
			.yres		= 1080,
			.pixclock	= 15384,
			.left_margin	= 148,
			.right_margin	= 88,
			.upper_margin	= 36,
			.lower_margin	= 4,
			.hsync_len	= 44,
			.vsync_len	= 5,
			.sync		= FB_SYNC_HOR_HIGH_ACT |
					  FB_SYNC_VERT_HIGH_ACT |
					  FB_SYNC_EXT,
			.vmode		= FB_VMODE_NONINTERLACED,
		},
	},
};

size_t display_count = ARRAY_SIZE(displays);

static void enable_vpll(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	int timeout = 100000;

	setbits_le32(&ccm->analog_pll_video, BM_ANADIG_PLL_VIDEO_POWERDOWN);

	clrsetbits_le32(&ccm->analog_pll_video,
			BM_ANADIG_PLL_VIDEO_DIV_SELECT |
			BM_ANADIG_PLL_VIDEO_POST_DIV_SELECT,
			BF_ANADIG_PLL_VIDEO_DIV_SELECT(37) |
			BF_ANADIG_PLL_VIDEO_POST_DIV_SELECT(1));

	writel(BF_ANADIG_PLL_VIDEO_NUM_A(11), &ccm->analog_pll_video_num);
	writel(BF_ANADIG_PLL_VIDEO_DENOM_B(12), &ccm->analog_pll_video_denom);

	clrbits_le32(&ccm->analog_pll_video, BM_ANADIG_PLL_VIDEO_POWERDOWN);

	while (timeout--)
		if (readl(&ccm->analog_pll_video) & BM_ANADIG_PLL_VIDEO_LOCK)
			break;
	if (timeout < 0)
		printf("Warning: video pll lock timeout!\n");

	clrsetbits_le32(&ccm->analog_pll_video,
			BM_ANADIG_PLL_VIDEO_BYPASS,
			BM_ANADIG_PLL_VIDEO_ENABLE);
}

void setup_display_clock(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;

	enable_ipu_clock();
	enable_vpll();
	imx_setup_hdmi();

	/* Turn on IPU LDB DI0 clocks */
	setbits_le32(&mxc_ccm->CCGR3, MXC_CCM_CCGR3_LDB_DI0_MASK);

	/* Switch LDB DI0 to PLL5 (Video PLL) */
	clrsetbits_le32(&mxc_ccm->cs2cdr,
			MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK,
			(0 << MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET));

	/* LDB clock div by 3.5 */
	clrbits_le32(&mxc_ccm->cscmr2, MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV);

	/* DI0 clock derived from ldb_di0_clk */
	clrsetbits_le32(&mxc_ccm->chsccdr,
			MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_MASK,
			(CHSCCDR_CLK_SEL_LDB_DI0 <<
			 MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET)
			);

	/* Enable both LVDS channels, both connected to DI0. */
	writel(IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_HIGH |
	       IOMUXC_GPR2_BIT_MAPPING_CH1_JEIDA |
	       IOMUXC_GPR2_DATA_WIDTH_CH1_24BIT |
	       IOMUXC_GPR2_BIT_MAPPING_CH0_JEIDA |
	       IOMUXC_GPR2_DATA_WIDTH_CH0_24BIT |
	       IOMUXC_GPR2_SPLIT_MODE_EN_MASK |
	       IOMUXC_GPR2_LVDS_CH1_MODE_ENABLED_DI0 |
	       IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI0,
	       &iomux->gpr[2]);

	clrsetbits_le32(&iomux->gpr[3],
			IOMUXC_GPR3_LVDS0_MUX_CTL_MASK |
			IOMUXC_GPR3_LVDS1_MUX_CTL_MASK,
			(IOMUXC_GPR3_MUX_SRC_IPU1_DI0 <<
			 IOMUXC_GPR3_LVDS0_MUX_CTL_OFFSET) |
			(IOMUXC_GPR3_MUX_SRC_IPU1_DI0 <<
			 IOMUXC_GPR3_LVDS1_MUX_CTL_OFFSET)
			);
}

void setup_display_lvds(void)
{
	int ret;

	ret = i2c_set_bus_num(NOVENA_IT6251_I2C_BUS);
	if (ret) {
		puts("Cannot select LVDS-to-eDP I2C bus.\n");
		return;
	}

	/* The IT6251 should be ready now, if it's not, it's not connected. */
	ret = it6251_ready();
	if (!ret)
		return;

	/* Init the LVDS-to-eDP chip and if it succeeded, enable backlight. */
	ret = it6251_init();
	if (!ret) {
		gpio_request(NOVENA_BACKLIGHT_PWR_GPIO, "backlight-power");
		gpio_request(NOVENA_BACKLIGHT_PWM_GPIO, "backlight-pwm");
		/* Backlight power enable. */
		gpio_direction_output(NOVENA_BACKLIGHT_PWR_GPIO, 1);
		/* PWM backlight pin, always on for full brightness. */
		gpio_direction_output(NOVENA_BACKLIGHT_PWM_GPIO, 1);
	}
}
