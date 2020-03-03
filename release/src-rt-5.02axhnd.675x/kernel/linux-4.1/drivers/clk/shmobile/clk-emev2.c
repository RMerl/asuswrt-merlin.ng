/*
 * EMMA Mobile EV2 common clock framework support
 *
 * Copyright (C) 2013 Takashi Yoshii <takashi.yoshii.ze@renesas.com>
 * Copyright (C) 2012 Magnus Damm
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>

/* EMEV2 SMU registers */
#define USIAU0_RSTCTRL 0x094
#define USIBU1_RSTCTRL 0x0ac
#define USIBU2_RSTCTRL 0x0b0
#define USIBU3_RSTCTRL 0x0b4
#define STI_RSTCTRL 0x124
#define STI_CLKSEL 0x688

static DEFINE_SPINLOCK(lock);

/* not pretty, but hey */
void __iomem *smu_base;

static void __init emev2_smu_write(unsigned long value, int offs)
{
	BUG_ON(!smu_base || (offs >= PAGE_SIZE));
	writel_relaxed(value, smu_base + offs);
}

static const struct of_device_id smu_id[] __initconst = {
	{ .compatible = "renesas,emev2-smu", },
	{},
};

static void __init emev2_smu_init(void)
{
	struct device_node *np;

	np = of_find_matching_node(NULL, smu_id);
	BUG_ON(!np);
	smu_base = of_iomap(np, 0);
	BUG_ON(!smu_base);
	of_node_put(np);

	/* setup STI timer to run on 32.768 kHz and deassert reset */
	emev2_smu_write(0, STI_CLKSEL);
	emev2_smu_write(1, STI_RSTCTRL);

	/* deassert reset for UART0->UART3 */
	emev2_smu_write(2, USIAU0_RSTCTRL);
	emev2_smu_write(2, USIBU1_RSTCTRL);
	emev2_smu_write(2, USIBU2_RSTCTRL);
	emev2_smu_write(2, USIBU3_RSTCTRL);
}

static void __init emev2_smu_clkdiv_init(struct device_node *np)
{
	u32 reg[2];
	struct clk *clk;
	const char *parent_name = of_clk_get_parent_name(np, 0);
	if (WARN_ON(of_property_read_u32_array(np, "reg", reg, 2)))
		return;
	if (!smu_base)
		emev2_smu_init();
	clk = clk_register_divider(NULL, np->name, parent_name, 0,
				   smu_base + reg[0], reg[1], 8, 0, &lock);
	of_clk_add_provider(np, of_clk_src_simple_get, clk);
	clk_register_clkdev(clk, np->name, NULL);
	pr_debug("## %s %s %p\n", __func__, np->name, clk);
}
CLK_OF_DECLARE(emev2_smu_clkdiv, "renesas,emev2-smu-clkdiv",
		emev2_smu_clkdiv_init);

static void __init emev2_smu_gclk_init(struct device_node *np)
{
	u32 reg[2];
	struct clk *clk;
	const char *parent_name = of_clk_get_parent_name(np, 0);
	if (WARN_ON(of_property_read_u32_array(np, "reg", reg, 2)))
		return;
	if (!smu_base)
		emev2_smu_init();
	clk = clk_register_gate(NULL, np->name, parent_name, 0,
				smu_base + reg[0], reg[1], 0, &lock);
	of_clk_add_provider(np, of_clk_src_simple_get, clk);
	clk_register_clkdev(clk, np->name, NULL);
	pr_debug("## %s %s %p\n", __func__, np->name, clk);
}
CLK_OF_DECLARE(emev2_smu_gclk, "renesas,emev2-smu-gclk", emev2_smu_gclk_init);
