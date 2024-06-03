// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Author: Priyanka Jain <Priyanka.Jain@freescale.com>
 */

#include <asm/io.h>
#include <common.h>
#include <command.h>
#include <fsl_diu_fb.h>
#include <linux/ctype.h>
#include <video_fb.h>

#include "../common/diu_ch7301.h"

#include "cpld.h"
#include "t104xrdb.h"

/*
 * DIU Area Descriptor
 *
 * Note that we need to byte-swap the value before it's written to the AD
 * register. So even though the registers don't look like they're in the same
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
	int ret;

	speed_ccb = get_bus_freq(0);
	temp = 1000000000 / pixclock;
	temp *= 1000;
	pixval = speed_ccb / temp;

	/* Program HDMI encoder */
	ret = diu_set_dvi_encoder(temp);
	if (ret) {
		puts("Failed to set DVI encoder\n");
		return;
	}

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

	/*Configure Display ouput port as HDMI*/
	sw = CPLD_READ(sfp_ctl_status);
	CPLD_WRITE(sfp_ctl_status , sw & ~(CPLD_DIU_SEL_DFP));

	pixel_format = cpu_to_le32(AD_BYTE_F | (3 << AD_ALPHA_C_SHIFT) |
		(0 << AD_BLUE_C_SHIFT) | (1 << AD_GREEN_C_SHIFT) |
		(2 << AD_RED_C_SHIFT) | (8 << AD_COMP_3_SHIFT) |
		(8 << AD_COMP_2_SHIFT) | (8 << AD_COMP_1_SHIFT) |
		(8 << AD_COMP_0_SHIFT) | (3 << AD_PIXEL_S_SHIFT));

	printf("DIU: Switching to monitor DVI @ %ux%u\n",  xres, yres);

	return fsl_diu_init(xres, yres, pixel_format, 0);
}
