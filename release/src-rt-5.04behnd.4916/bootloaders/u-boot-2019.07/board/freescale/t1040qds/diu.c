// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Author: Priyanka Jain <Priyanka.Jain@freescale.com>
 */

#include <common.h>
#include <command.h>
#include <linux/ctype.h>
#include <asm/io.h>
#include <stdio_dev.h>
#include <video_fb.h>
#include <fsl_diu_fb.h>
#include "../common/qixis.h"
#include "../common/diu_ch7301.h"
#include "t1040qds.h"
#include "t1040qds_qixis.h"

/*
 * DIU Area Descriptor
 *
 * Note that we need to byte-swap the value before it's written to the AD
 * register.  So even though the registers don't look like they're in the same
 * bit positions as they are on the MPC8610, the same value is written to the
 * AD register on the MPC8610 and on the P1022.
 */
#define AD_BYTE_F		0x10000000
#define AD_ALPHA_C_SHIFT	25
#define AD_BLUE_C_SHIFT		23
#define AD_GREEN_C_SHIFT	21
#define AD_RED_C_SHIFT		19
#define AD_PIXEL_S_SHIFT	16
#define AD_COMP_3_SHIFT		12
#define AD_COMP_2_SHIFT		8
#define AD_COMP_1_SHIFT		4
#define AD_COMP_0_SHIFT		0

void diu_set_pixel_clock(unsigned int pixclock)
{
	unsigned long speed_ccb, temp;
	u32 pixval;
	int ret = 0;
	speed_ccb = get_bus_freq(0);
	temp = 1000000000 / pixclock;
	temp *= 1000;
	pixval = speed_ccb / temp;

	/* Program HDMI encoder */
	/* Switch channel to DIU */
	select_i2c_ch_pca9547(I2C_MUX_CH_DIU);

	/* Set dispaly encoder */
	ret = diu_set_dvi_encoder(temp);
	if (ret) {
		puts("Failed to set DVI encoder\n");
		return;
	}

	/* Switch channel to default */
	select_i2c_ch_pca9547(I2C_MUX_CH_DEFAULT);

	/* Program pixel clock */
	out_be32((unsigned *)CONFIG_SYS_FSL_SCFG_PIXCLK_ADDR,
		 ((pixval << PXCK_BITS_START) & PXCK_MASK));
	/* enable clock*/
	out_be32((unsigned *)CONFIG_SYS_FSL_SCFG_PIXCLK_ADDR, PXCKEN_MASK |
		 ((pixval << PXCK_BITS_START) & PXCK_MASK));
}

int platform_diu_init(unsigned int xres, unsigned int yres, const char *port)
{
	u32 pixel_format;
	u8 sw;

	/*Route I2C4 to DIU system as HSYNC/VSYNC*/
	sw = QIXIS_READ(brdcfg[5]);
	QIXIS_WRITE(brdcfg[5],
		    ((sw & ~(BRDCFG5_IMX_MASK)) | (BRDCFG5_IMX_DIU)));

	/*Configure Display ouput port as HDMI*/
	sw = QIXIS_READ(brdcfg[15]);
	QIXIS_WRITE(brdcfg[15],
		    ((sw & ~(BRDCFG15_LCDPD_MASK | BRDCFG15_DIUSEL_MASK))
		      | (BRDCFG15_LCDPD_ENABLED | BRDCFG15_DIUSEL_HDMI)));

	pixel_format = cpu_to_le32(AD_BYTE_F | (3 << AD_ALPHA_C_SHIFT) |
		(0 << AD_BLUE_C_SHIFT) | (1 << AD_GREEN_C_SHIFT) |
		(2 << AD_RED_C_SHIFT) | (8 << AD_COMP_3_SHIFT) |
		(8 << AD_COMP_2_SHIFT) | (8 << AD_COMP_1_SHIFT) |
		(8 << AD_COMP_0_SHIFT) | (3 << AD_PIXEL_S_SHIFT));

	printf("DIU:   Switching to monitor @ %ux%u\n",  xres, yres);


	return fsl_diu_init(xres, yres, pixel_format, 0);
}
