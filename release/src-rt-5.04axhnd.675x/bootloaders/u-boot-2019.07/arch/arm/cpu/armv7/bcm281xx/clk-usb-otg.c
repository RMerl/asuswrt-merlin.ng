// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Broadcom Corporation.
 */

#include <common.h>
#include <linux/errno.h>
#include <asm/arch/sysmap.h>
#include "clk-core.h"

/* Enable appropriate clocks for the USB OTG port */
int clk_usb_otg_enable(void *base)
{
	char *ahbstr;

	switch ((u32) base) {
	case HSOTG_BASE_ADDR:
		ahbstr = "usb_otg_ahb_clk";
		break;
	default:
		printf("%s: base 0x%p not found\n", __func__, base);
		return -EINVAL;
	}

	return clk_get_and_enable(ahbstr);
}
