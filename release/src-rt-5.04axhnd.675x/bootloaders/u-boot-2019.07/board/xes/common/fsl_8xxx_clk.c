// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008 Extreme Engineering Solutions, Inc.
 */

#include <common.h>
#include <asm/io.h>

/*
 * Return SYSCLK input frequency - 50 MHz or 66 MHz depending on POR config
 */
unsigned long get_board_sys_clk(ulong dummy)
{
#if defined(CONFIG_MPC85xx)
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
#elif defined(CONFIG_MPC86xx)
	immap_t *immap = (immap_t *)CONFIG_SYS_IMMR;
	volatile ccsr_gur_t *gur = &immap->im_gur;
#endif

	if (in_be32(&gur->gpporcr) & 0x10000)
		return 66666666;
	else
#ifdef CONFIG_ARCH_P2020
		return 100000000;
#else
		return 50000000;
#endif
}

#ifdef CONFIG_MPC85xx
/*
 * Return DDR input clock - synchronous with SYSCLK or 66 MHz
 * Note: 86xx doesn't support asynchronous DDR clk
 */
unsigned long get_board_ddr_clk(ulong dummy)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 ddr_ratio = (in_be32(&gur->porpllsr) & 0x00003e00) >> 9;

	if (ddr_ratio == 0x7)
		return get_board_sys_clk(dummy);

#ifdef CONFIG_ARCH_P2020
	if (in_be32(&gur->gpporcr) & 0x20000)
		return 66666666;
	else
		return 100000000;
#else
	return 66666666;
#endif
}
#endif
