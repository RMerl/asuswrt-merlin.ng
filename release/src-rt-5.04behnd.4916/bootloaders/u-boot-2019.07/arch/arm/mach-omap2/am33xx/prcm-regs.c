// SPDX-License-Identifier: GPL-2.0+
/*
 * HW regs data for AM33xx.
 *
 * (C) Copyright 2017 Linaro Ltd.
 * Sam Protsenko <semen.protsenko@linaro.org>
 */

#include <asm/arch/hardware.h>
#include <asm/omap_common.h>

struct omap_sys_ctrl_regs const am33xx_ctrl = {
	.control_status = CTRL_BASE + 0x40,
};
