// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 * Authors: Timur Tabi <timur@freescale.com>
 *
 * FSL DIU Framebuffer driver
 */

#include <common.h>
#include <command.h>
#include <linux/ctype.h>
#include <asm/io.h>
#include <stdio_dev.h>
#include <video_fb.h>
#include <fsl_diu_fb.h>

#define PMUXCR_ELBCDIU_MASK	0xc0000000
#define PMUXCR_ELBCDIU_NOR16	0x80000000
#define PMUXCR_ELBCDIU_DIU	0x40000000

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

/*
 * Variables used by the DIU/LBC switching code.  It's safe to makes these
 * global, because the DIU requires DDR, so we'll only run this code after
 * relocation.
 */
static u32 pmuxcr;

void diu_set_pixel_clock(unsigned int pixclock)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	unsigned long speed_ccb, temp;
	u32 pixval;

	speed_ccb = get_bus_freq(0);
	temp = 1000000000 / pixclock;
	temp *= 1000;
	pixval = speed_ccb / temp;
	debug("DIU pixval = %u\n", pixval);

	/* Modify PXCLK in GUTS CLKDVDR */
	temp = in_be32(&gur->clkdvdr) & 0x2000FFFF;
	out_be32(&gur->clkdvdr, temp);			/* turn off clock */
	out_be32(&gur->clkdvdr, temp | 0x80000000 | ((pixval & 0x1F) << 16));
}

int platform_diu_init(unsigned int xres, unsigned int yres, const char *port)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 pixel_format;

	pixel_format = cpu_to_le32(AD_BYTE_F | (3 << AD_ALPHA_C_SHIFT) |
		(0 << AD_BLUE_C_SHIFT) | (1 << AD_GREEN_C_SHIFT) |
		(2 << AD_RED_C_SHIFT) | (8 << AD_COMP_3_SHIFT) |
		(8 << AD_COMP_2_SHIFT) | (8 << AD_COMP_1_SHIFT) |
		(8 << AD_COMP_0_SHIFT) | (3 << AD_PIXEL_S_SHIFT));

	printf("DIU:   Switching to %ux%u\n", xres, yres);

	/* Set PMUXCR to switch the muxed pins from the LBC to the DIU */
	clrsetbits_be32(&gur->pmuxcr, PMUXCR_ELBCDIU_MASK, PMUXCR_ELBCDIU_DIU);
	pmuxcr = in_be32(&gur->pmuxcr);

	return fsl_diu_init(xres, yres, pixel_format, 0);
}
