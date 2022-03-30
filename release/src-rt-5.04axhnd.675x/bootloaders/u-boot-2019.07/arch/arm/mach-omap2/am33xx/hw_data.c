// SPDX-License-Identifier: GPL-2.0+
/*
 * HW data initialization for AM33xx.
 *
 * (C) Copyright 2017 Linaro Ltd.
 * Sam Protsenko <semen.protsenko@linaro.org>
 */

#include <asm/arch/omap.h>
#include <asm/omap_common.h>

struct omap_sys_ctrl_regs const **ctrl =
	(struct omap_sys_ctrl_regs const **)OMAP_SRAM_SCRATCH_SYS_CTRL;

void hw_data_init(void)
{
	*ctrl = &am33xx_ctrl;
}
