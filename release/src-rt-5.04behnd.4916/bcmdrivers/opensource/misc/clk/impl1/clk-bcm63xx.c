/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/clk-provider.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/of_address.h>
#include <linux/of.h>
#include <linux/module.h>

#include "clk-bcm63xx.h"
extern int __init init_strap_register(void);

/*
 * dummy implementation. must implement these functions in chip specific 
 * file if it supports cpuclk
 */
int __weak get_arm_core_ratio(struct bcm63xx_cpuclk *cpuclk, int *ratio, int *ratio_base, int *mdiv)
{
	return 0;
}

long __weak round_arm_core_rate(struct bcm63xx_cpuclk *cpuclk, unsigned long rate)
{
	return 0;
}

int __weak set_arm_core_clock(struct bcm63xx_cpuclk *cpuclk, unsigned long parent_rate, unsigned long rate)
{
	return 0;
}

int __weak init_arm_core_pll(struct bcm63xx_cpuclk *cpuclk)
{
	return 0;
}

/*
 * Initialize the cpu default pll frequency
 */
static int bcm63xx_cpuclk_init_default(struct device_node *node, struct bcm63xx_cpuclk *cpuclk)
{
	return init_arm_core_pll(cpuclk);
}

static unsigned long bcm63xx_cpuclk_recalc_rate(struct clk_hw *hwclk,
					 unsigned long parent_rate)
{
	struct bcm63xx_cpuclk *cpuclk  = to_bcm63xx_cpuclk(hwclk);
	u32 ratio, base;
	unsigned long rate;

	ratio = base = 0;
	get_arm_core_ratio(cpuclk, &ratio, &base, &cpuclk->mdiv);
	cpuclk->ratio = ratio;
	cpuclk->ratio_base = base;

	rate = (cpuclk->pllclk/cpuclk->mdiv/base)*ratio;

	CLK_BCM_LOG("ratio %d base %d mdiv %d\n", ratio, base, cpuclk->mdiv);
	CLK_BCM_LOG("parent rate %ld return rate %ld\n", parent_rate, rate);

	return rate;
}

static long bcm63xx_cpuclk_round_rate(struct clk_hw *hwclk, unsigned long rate,
			       unsigned long *parent_rate)
{
	long rate_round;
	struct bcm63xx_cpuclk *cpuclk = to_bcm63xx_cpuclk(hwclk);

	rate_round = round_arm_core_rate(cpuclk, rate);

	CLK_BCM_LOG("parent rate %ld rate %ld round rate %ld\n", *parent_rate, rate, (unsigned long)rate_round);


	return rate_round;
}

static int bcm63xx_cpuclk_set_rate(struct clk_hw *hwclk, unsigned long rate,
			    unsigned long parent_rate)
{

	struct bcm63xx_cpuclk *cpuclk = to_bcm63xx_cpuclk(hwclk);
	CLK_BCM_LOG("parent rate %ld rate %ld\n", parent_rate, rate);

	return set_arm_core_clock(cpuclk, parent_rate, rate);
}

static const struct clk_ops bcm63xx_cpuclk_ops = {
	.recalc_rate = bcm63xx_cpuclk_recalc_rate,
	.round_rate = bcm63xx_cpuclk_round_rate,
	.set_rate = bcm63xx_cpuclk_set_rate,
};

/**
 * bcm63xx_cpuclk_init - initialize bcm63xx cpu clock through DT
 * @node: device tree node for this clock
 */
static void __init bcm63xx_cpuclk_init(struct device_node *node)
{
	struct clk *clk;
	const char *clk_name = node->name;
	struct clk_init_data init;
	struct bcm63xx_cpuclk *cpuclk;

	cpuclk = kmalloc(sizeof(struct bcm63xx_cpuclk), GFP_KERNEL);
	if (!cpuclk) {
		pr_err("%s: could not allocate bcm63xx cpuclk\n", __func__);
		return;
	}

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
	cpuclk->reg = of_iomap(node, 0);
	if (IS_ERR_OR_NULL(cpuclk->reg)) {
		pr_err("%s: Failed to map clk reg in node %pOF!\n", __func__, node);
		return;
	}
#endif

	bcm63xx_cpuclk_init_default(node, cpuclk);

	of_property_read_string(node, "clock-output-names", &clk_name);

	cpuclk->hw.init = &init;
	init.name = clk_name;
	init.ops = &bcm63xx_cpuclk_ops;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
	init.flags = CLK_IS_ROOT;
#endif
	init.parent_names = NULL;
	init.num_parents = 0;

	clk = clk_register(NULL, &cpuclk->hw);
	if (IS_ERR(clk))
		kfree(cpuclk);
	else
		of_clk_add_provider(node, of_clk_src_simple_get, clk);

	return;
}

/**
 * of_bcm63xx_cpuclk_init - initialize bcm63xx cpu clock through DT
 * @node: device tree node for this clock
 */
static void __init of_bcm63xx_cpuclk_init(struct device_node *node)
{
	printk("clk-bcm63xx: BCM63XX CPU Clock driver\n");
	init_strap_register();
	bcm63xx_cpuclk_init(node);
}
CLK_OF_DECLARE(bcm63xx_cpuclk, "brcm,63xx_cpuclk", of_bcm63xx_cpuclk_init);
