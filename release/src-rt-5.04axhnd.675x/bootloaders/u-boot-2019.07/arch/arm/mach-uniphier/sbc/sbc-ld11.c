// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016-2017 Socionext Inc.
 */

#include <common.h>
#include <spl.h>
#include <linux/io.h>

#include "../init.h"
#include "sbc-regs.h"

void uniphier_ld11_sbc_init(void)
{
	uniphier_sbc_init_savepin();

	/* necessary for ROM boot ?? */
	/* system bus output enable */
	writel(0x17, PC0CTRL);

	/* pins for NAND and System Bus are multiplexed */
	if (spl_boot_device() != BOOT_DEVICE_NAND)
		uniphier_pin_init("system-bus");
}
