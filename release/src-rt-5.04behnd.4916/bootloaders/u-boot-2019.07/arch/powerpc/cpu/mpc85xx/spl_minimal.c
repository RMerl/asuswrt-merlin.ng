// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2009 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/global_data.h>
#include <fsl_ifc.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

ulong cpu_init_f(void)
{
#ifdef CONFIG_SYS_INIT_L2_ADDR
	ccsr_l2cache_t *l2cache = (void *)CONFIG_SYS_MPC85xx_L2_ADDR;

	out_be32(&l2cache->l2srbar0, CONFIG_SYS_INIT_L2_ADDR);

	/* set MBECCDIS=1, SBECCDIS=1 */
	out_be32(&l2cache->l2errdis,
		(MPC85xx_L2ERRDIS_MBECC | MPC85xx_L2ERRDIS_SBECC));

	/* set L2E=1 & L2SRAM=001 */
	out_be32(&l2cache->l2ctl,
		(MPC85xx_L2CTL_L2E | MPC85xx_L2CTL_L2SRAM_ENTIRE));
#endif

	return 0;
}

#ifndef CONFIG_SYS_FSL_TBCLK_DIV
#define CONFIG_SYS_FSL_TBCLK_DIV 8
#endif

void udelay(unsigned long usec)
{
	u32 ticks_per_usec = gd->bus_clk / (CONFIG_SYS_FSL_TBCLK_DIV * 1000000);
	u32 ticks = ticks_per_usec * usec;
	u32 s = mfspr(SPRN_TBRL);

	while ((mfspr(SPRN_TBRL) - s) < ticks);
}
