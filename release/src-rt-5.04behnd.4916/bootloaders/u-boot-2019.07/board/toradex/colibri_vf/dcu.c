// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 Toradex AG
 *
 * FSL DCU platform driver
 */

#include <asm/arch/crm_regs.h>
#include <asm/io.h>
#include <common.h>
#include <fsl_dcu_fb.h>
#include "div64.h"

unsigned int dcu_set_pixel_clock(unsigned int pixclock)
{
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;
	unsigned long long div;

	clrbits_le32(&ccm->cscmr1, CCM_CSCMR1_DCU0_CLK_SEL);
	clrsetbits_le32(&ccm->cscdr3,
			CCM_CSCDR3_DCU0_DIV_MASK | CCM_CSCDR3_DCU0_EN,
			CCM_CSCDR3_DCU0_DIV(0) | CCM_CSCDR3_DCU0_EN);
	div = (unsigned long long)(PLL1_PFD2_FREQ / 1000);
	do_div(div, pixclock);

	return div;
}

int platform_dcu_init(unsigned int xres, unsigned int yres,
		      const char *port,
		      struct fb_videomode *dcu_fb_videomode)
{
	fsl_dcu_init(xres, yres, 32);

	return 0;
}
