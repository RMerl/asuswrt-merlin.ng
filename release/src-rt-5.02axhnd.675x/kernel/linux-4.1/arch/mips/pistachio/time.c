/*
 * Pistachio clocksource/timer setup
 *
 * Copyright (C) 2014 Google, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/clocksource.h>
#include <linux/init.h>
#include <linux/irqchip/mips-gic.h>
#include <linux/of.h>

#include <asm/time.h>

unsigned int get_c0_compare_int(void)
{
	return gic_get_c0_compare_int();
}

int get_c0_perfcount_int(void)
{
	return gic_get_c0_perfcount_int();
}
EXPORT_SYMBOL_GPL(get_c0_perfcount_int);

void __init plat_time_init(void)
{
	struct device_node *np;
	struct clk *clk;

	of_clk_init(NULL);
	clocksource_of_init();

	np = of_get_cpu_node(0, NULL);
	if (!np) {
		pr_err("Failed to get CPU node\n");
		return;
	}

	clk = of_clk_get(np, 0);
	if (IS_ERR(clk)) {
		pr_err("Failed to get CPU clock: %ld\n", PTR_ERR(clk));
		return;
	}

	mips_hpt_frequency = clk_get_rate(clk) / 2;
	clk_put(clk);
}
