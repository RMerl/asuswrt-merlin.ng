// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#include <common.h>
#include <asm/io.h>

void mscc_gpio_set_alternate(int gpio, int mode)
{
	u32 mask = BIT(gpio);
	u32 val0, val1;

	val0 = readl(BASE_DEVCPU_GCB + GPIO_ALT(0));
	val1 = readl(BASE_DEVCPU_GCB + GPIO_ALT(1));

	if (mode == 1) {
		val0 |= mask;
		val1 &= ~mask;
	} else if (mode == 2) {
		val0 &= ~mask;
		val1 |= mask;
	} else if (mode == 3) {
		val0 |= mask;
		val1 |= mask;
	} else {
		val0 &= ~mask;
		val1 &= ~mask;
	}

	writel(val0, BASE_DEVCPU_GCB + GPIO_ALT(0));
	writel(val1, BASE_DEVCPU_GCB + GPIO_ALT(1));
}
