// SPDX-License-Identifier: GPL-2.0+
/*
 * HW regs data for OMAP3.
 *
 * (C) Copyright 2017 Linaro Ltd.
 * Sam Protsenko <semen.protsenko@linaro.org>
 */

#include <asm/arch/omap.h>
#include <asm/omap_common.h>

struct omap_sys_ctrl_regs const omap3_ctrl = {
	.control_status = OMAP34XX_CTRL_BASE + 0x2F0,
};
