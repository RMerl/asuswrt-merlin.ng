// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011-2015 Panasonic Corporation
 * Copyright (C) 2015-2017 Socionext Inc.
 */

#include <linux/io.h>

#include "../init.h"
#include "sbc-regs.h"

void uniphier_ld4_sbc_init(void)
{
	u32 tmp;

	uniphier_sbc_init_savepin();

	/* system bus output enable */
	tmp = readl(PC0CTRL);
	tmp &= 0xfffffcff;
	writel(tmp, PC0CTRL);
}
