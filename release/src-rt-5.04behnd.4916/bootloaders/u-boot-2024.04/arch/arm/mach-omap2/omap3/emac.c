// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * DaVinci EMAC initialization.
 *
 * (C) Copyright 2011, Ilya Yanok, Emcraft Systems
 */

#include <common.h>
#include <net.h>
#include <asm/io.h>
#include <asm/arch/am35x_def.h>

/*
 * Initializes on-chip ethernet controllers.
 * to override, implement board_eth_init()
 */
int cpu_eth_init(struct bd_info *bis)
{
	u32 reset;

	/* ensure that the module is out of reset */
	reset = readl(&am35x_scm_general_regs->ip_sw_reset);
	reset &= ~CPGMACSS_SW_RST;
	writel(reset, &am35x_scm_general_regs->ip_sw_reset);

	return 0;
}
