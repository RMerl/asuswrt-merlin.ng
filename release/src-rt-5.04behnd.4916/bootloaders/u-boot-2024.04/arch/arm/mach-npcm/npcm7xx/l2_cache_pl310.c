// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Nuvoton Technology Corp.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/pl310.h>

void l2_pl310_init(void);

void set_pl310_ctrl(u32 enable)
{
	struct pl310_regs *const pl310 = (struct pl310_regs *)CFG_SYS_PL310_BASE;

	writel(enable, &pl310->pl310_ctrl);
}

void v7_outer_cache_enable(void)
{
	l2_pl310_init();

	set_pl310_ctrl(1);
}

void v7_outer_cache_disable(void)
{
	set_pl310_ctrl(0);
}
