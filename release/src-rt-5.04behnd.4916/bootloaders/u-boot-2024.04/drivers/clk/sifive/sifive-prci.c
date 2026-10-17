// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2021 SiFive, Inc.
 * Wesley Terpstra
 * Paul Walmsley
 * Zong Li
 * Pragnesh Patel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * The PRCI implements clock and reset control for the SiFive chip.
 * This driver assumes that it has sole control over all PRCI resources.
 *
 * This driver is based on the PRCI driver written by Wesley Terpstra:
 * https://github.com/riscv/riscv-linux/commit/999529edf517ed75b56659d456d221b2ee56bb60
 */

#include <common.h>
#include <clk-uclass.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <reset.h>
#include <asm/io.h>
#include <asm/arch/reset.h>
#include <linux/delay.h>
#include <linux/math64.h>
#include <dt-bindings/clock/sifive-fu740-prci.h>

#include "fu540-prci.h"
#include "fu740-prci.h"

/*
 * Private functions
 */

/**
 * __prci_readl() - read from a PRCI register
 * @pd: PRCI context
 * @offs: register offset to read from (in bytes, from PRCI base address)
 *
 * Read the register located at offset @offs from the base virtual
 * address of the PRCI register target described by @pd, and return
 * the value to the caller.
 *
 * Context: Any context.
 *
 * Return: the contents of the register described by @pd and @offs.
 */
static u32 __prci_readl(struct __prci_data *pd, u32 offs)
{
	return readl(pd->va + offs);
}

static void __prci_writel(u32 v, u32 offs, struct __prci_data *pd)
{
	writel(v, pd->va + offs);
}

/* WRPLL-related private functions */

/**
 * __prci_wrpll_unpack() - unpack WRPLL configuration registers into parameters
 * @c: ptr to a struct wrpll_cfg record to write config into
 * @r: value read from the PRCI PLL configuration register
 *
 * Given a value @r read from an FU540 PRCI PLL configuration register,
 * split it into fields and populate it into the WRPLL configuration record
 * pointed to by @c.
 *
 * The COREPLLCFG0 macros are used below, but the other *PLLCFG0 macros
 * have the same register layout.
 *
 * Context: Any context.
 */
static void __prci_wrpll_unpack(struct wrpll_cfg *c, u32 r)
{
	u32 v;

	v = r & PRCI_COREPLLCFG0_DIVR_MASK;
	v >>= PRCI_COREPLLCFG0_DIVR_SHIFT;
	c->divr = v;

	v = r & PRCI_COREPLLCFG0_DIVF_MASK;
	v >>= PRCI_COREPLLCFG0_DIVF_SHIFT;
	c->divf = v;

	v = r & PRCI_COREPLLCFG0_DIVQ_MASK;
	v >>= PRCI_COREPLLCFG0_DIVQ_SHIFT;
	c->divq = v;

	v = r & PRCI_COREPLLCFG0_RANGE_MASK;
	v >>= PRCI_COREPLLCFG0_RANGE_SHIFT;
	c->range = v;

	c->flags &= (WRPLL_FLAGS_INT_FEEDBACK_MASK |
		     WRPLL_FLAGS_EXT_FEEDBACK_MASK);

	/* external feedback mode not supported */
	c->flags |= WRPLL_FLAGS_INT_FEEDBACK_MASK;
}

/**
 * __prci_wrpll_pack() - pack PLL configuration parameters into a register value
 * @c: pointer to a struct wrpll_cfg record containing the PLL's cfg
 *
 * Using a set of WRPLL configuration values pointed to by @c,
 * assemble a PRCI PLL configuration register value, and return it to
 * the caller.
 *
 * Context: Any context.  Caller must ensure that the contents of the
 *          record pointed to by @c do not change during the execution
 *          of this function.
 *
 * Returns: a value suitable for writing into a PRCI PLL configuration
 *          register
 */
static u32 __prci_wrpll_pack(const struct wrpll_cfg *c)
{
	u32 r = 0;

	r |= c->divr << PRCI_COREPLLCFG0_DIVR_SHIFT;
	r |= c->divf << PRCI_COREPLLCFG0_DIVF_SHIFT;
	r |= c->divq << PRCI_COREPLLCFG0_DIVQ_SHIFT;
	r |= c->range << PRCI_COREPLLCFG0_RANGE_SHIFT;

	/* external feedback mode not supported */
	r |= PRCI_COREPLLCFG0_FSE_MASK;

	return r;
}

/**
 * __prci_wrpll_read_cfg0() - read the WRPLL configuration from the PRCI
 * @pd: PRCI context
 * @pwd: PRCI WRPLL metadata
 *
 * Read the current configuration of the PLL identified by @pwd from
 * the PRCI identified by @pd, and store it into the local configuration
 * cache in @pwd.
 *
 * Context: Any context.  Caller must prevent the records pointed to by
 *          @pd and @pwd from changing during execution.
 */
static void __prci_wrpll_read_cfg0(struct __prci_data *pd,
				   struct __prci_wrpll_data *pwd)
{
	__prci_wrpll_unpack(&pwd->c, __prci_readl(pd, pwd->cfg0_offs));
}

/**
 * __prci_wrpll_write_cfg0() - write WRPLL configuration into the PRCI
 * @pd: PRCI context
 * @pwd: PRCI WRPLL metadata
 * @c: WRPLL configuration record to write
 *
 * Write the WRPLL configuration described by @c into the WRPLL
 * configuration register identified by @pwd in the PRCI instance
 * described by @c.  Make a cached copy of the WRPLL's current
 * configuration so it can be used by other code.
 *
 * Context: Any context.  Caller must prevent the records pointed to by
 *          @pd and @pwd from changing during execution.
 */
static void __prci_wrpll_write_cfg0(struct __prci_data *pd,
				    struct __prci_wrpll_data *pwd,
				    struct wrpll_cfg *c)
{
	__prci_writel(__prci_wrpll_pack(c), pwd->cfg0_offs, pd);

	memcpy(&pwd->c, c, sizeof(*c));
}

/**
 * __prci_wrpll_write_cfg1() - write Clock enable/disable configuration
 * into the PRCI
 * @pd: PRCI context
 * @pwd: PRCI WRPLL metadata
 * @enable: Clock enable or disable value
 */
static void __prci_wrpll_write_cfg1(struct __prci_data *pd,
				    struct __prci_wrpll_data *pwd,
				    u32 enable)
{
	__prci_writel(enable, pwd->cfg1_offs, pd);
}

unsigned long sifive_prci_wrpll_recalc_rate(struct __prci_clock *pc,
					    unsigned long parent_rate)
{
	struct __prci_wrpll_data *pwd = pc->pwd;

	return wrpll_calc_output_rate(&pwd->c, parent_rate);
}

unsigned long sifive_prci_wrpll_round_rate(struct __prci_clock *pc,
					   unsigned long rate,
					   unsigned long *parent_rate)
{
	struct __prci_wrpll_data *pwd = pc->pwd;
	struct wrpll_cfg c;

	memcpy(&c, &pwd->c, sizeof(c));

	wrpll_configure_for_rate(&c, rate, *parent_rate);

	return wrpll_calc_output_rate(&c, *parent_rate);
}

int sifive_prci_wrpll_set_rate(struct __prci_clock *pc,
			       unsigned long rate,
			       unsigned long parent_rate)
{
	struct __prci_wrpll_data *pwd = pc->pwd;
	struct __prci_data *pd = pc->pd;
	int r;

	r = wrpll_configure_for_rate(&pwd->c, rate, parent_rate);
	if (r)
		return r;

	if (pwd->enable_bypass)
		pwd->enable_bypass(pd);

	__prci_wrpll_write_cfg0(pd, pwd, &pwd->c);

	udelay(wrpll_calc_max_lock_us(&pwd->c));

	return 0;
}

int sifive_prci_clock_enable(struct __prci_clock *pc, bool enable)
{
	struct __prci_wrpll_data *pwd = pc->pwd;
	struct __prci_data *pd = pc->pd;

	if (enable) {
		__prci_wrpll_write_cfg1(pd, pwd, PRCI_COREPLLCFG1_CKE_MASK);

		if (pwd->disable_bypass)
			pwd->disable_bypass(pd);

		if (pwd->release_reset)
			pwd->release_reset(pd);
	} else {
		u32 r;

		if (pwd->enable_bypass)
			pwd->enable_bypass(pd);

		r = __prci_readl(pd, pwd->cfg1_offs);
		r &= ~PRCI_COREPLLCFG1_CKE_MASK;

		__prci_wrpll_write_cfg1(pd, pwd, r);
	}

	return 0;
}

/* TLCLKSEL clock integration */

unsigned long sifive_prci_tlclksel_recalc_rate(struct __prci_clock *pc,
					       unsigned long parent_rate)
{
	struct __prci_data *pd = pc->pd;
	u32 v;
	u8 div;

	v = __prci_readl(pd, PRCI_CLKMUXSTATUSREG_OFFSET);
	v &= PRCI_CLKMUXSTATUSREG_TLCLKSEL_STATUS_MASK;
	div = v ? 1 : 2;

	return div_u64(parent_rate, div);
}

/* HFPCLK clock integration */

unsigned long sifive_prci_hfpclkplldiv_recalc_rate(struct __prci_clock *pc,
						   unsigned long parent_rate)
{
	struct __prci_data *pd = pc->pd;
	u32 div = __prci_readl(pd, PRCI_HFPCLKPLLDIV_OFFSET);

	return div_u64(parent_rate, div + 2);
}

/**
 * sifive_prci_coreclksel_use_final_corepll() - switch the CORECLK mux to output
 * FINAL_COREPLL
 * @pd: struct __prci_data * for the PRCI containing the CORECLK mux reg
 *
 * Switch the CORECLK mux to the final COREPLL output clock; return once
 * complete.
 *
 * Context: Any context.  Caller must prevent concurrent changes to the
 *          PRCI_CORECLKSEL_OFFSET register.
 */
void sifive_prci_coreclksel_use_final_corepll(struct __prci_data *pd)
{
	u32 r;

	r = __prci_readl(pd, PRCI_CORECLKSEL_OFFSET);
	r &= ~PRCI_CORECLKSEL_CORECLKSEL_MASK;
	__prci_writel(r, PRCI_CORECLKSEL_OFFSET, pd);

	r = __prci_readl(pd, PRCI_CORECLKSEL_OFFSET);   /* barrier */
}

/**
 * sifive_prci_corepllsel_use_dvfscorepll() - switch the COREPLL mux to
 * output DVFS_COREPLL
 * @pd: struct __prci_data * for the PRCI containing the COREPLL mux reg
 *
 * Switch the COREPLL mux to the DVFSCOREPLL output clock; return once complete.
 *
 * Context: Any context.  Caller must prevent concurrent changes to the
 *          PRCI_COREPLLSEL_OFFSET register.
 */
void sifive_prci_corepllsel_use_dvfscorepll(struct __prci_data *pd)
{
	u32 r;

	r = __prci_readl(pd, PRCI_COREPLLSEL_OFFSET);
	r |= PRCI_COREPLLSEL_COREPLLSEL_MASK;
	__prci_writel(r, PRCI_COREPLLSEL_OFFSET, pd);

	r = __prci_readl(pd, PRCI_COREPLLSEL_OFFSET);   /* barrier */
}

/**
 * sifive_prci_corepllsel_use_corepll() - switch the COREPLL mux to
 * output COREPLL
 * @pd: struct __prci_data * for the PRCI containing the COREPLL mux reg
 *
 * Switch the COREPLL mux to the COREPLL output clock; return once complete.
 *
 * Context: Any context.  Caller must prevent concurrent changes to the
 *          PRCI_COREPLLSEL_OFFSET register.
 */
void sifive_prci_corepllsel_use_corepll(struct __prci_data *pd)
{
	u32 r;

	r = __prci_readl(pd, PRCI_COREPLLSEL_OFFSET);
	r &= ~PRCI_COREPLLSEL_COREPLLSEL_MASK;
	__prci_writel(r, PRCI_COREPLLSEL_OFFSET, pd);

	r = __prci_readl(pd, PRCI_COREPLLSEL_OFFSET);   /* barrier */
}

/**
 * sifive_prci_hfpclkpllsel_use_hfclk() - switch the HFPCLKPLL mux to
 * output HFCLK
 * @pd: struct __prci_data * for the PRCI containing the HFPCLKPLL mux reg
 *
 * Switch the HFPCLKPLL mux to the HFCLK input source; return once complete.
 *
 * Context: Any context.  Caller must prevent concurrent changes to the
 *          PRCI_HFPCLKPLLSEL_OFFSET register.
 */
void sifive_prci_hfpclkpllsel_use_hfclk(struct __prci_data *pd)
{
	u32 r;

	r = __prci_readl(pd, PRCI_HFPCLKPLLSEL_OFFSET);
	r |= PRCI_HFPCLKPLLSEL_HFPCLKPLLSEL_MASK;
	__prci_writel(r, PRCI_HFPCLKPLLSEL_OFFSET, pd);

	r = __prci_readl(pd, PRCI_HFPCLKPLLSEL_OFFSET); /* barrier */
}

/**
 * sifive_prci_hfpclkpllsel_use_hfpclkpll() - switch the HFPCLKPLL mux to
 * output HFPCLKPLL
 * @pd: struct __prci_data * for the PRCI containing the HFPCLKPLL mux reg
 *
 * Switch the HFPCLKPLL mux to the HFPCLKPLL output clock; return once complete.
 *
 * Context: Any context.  Caller must prevent concurrent changes to the
 *          PRCI_HFPCLKPLLSEL_OFFSET register.
 */
void sifive_prci_hfpclkpllsel_use_hfpclkpll(struct __prci_data *pd)
{
	u32 r;

	r = __prci_readl(pd, PRCI_HFPCLKPLLSEL_OFFSET);
	r &= ~PRCI_HFPCLKPLLSEL_HFPCLKPLLSEL_MASK;
	__prci_writel(r, PRCI_HFPCLKPLLSEL_OFFSET, pd);

	r = __prci_readl(pd, PRCI_HFPCLKPLLSEL_OFFSET); /* barrier */
}

static int __prci_consumer_reset(const char *rst_name, bool trigger)
{
	struct udevice *dev;
	struct reset_ctl rst_sig;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_RESET,
					  DM_DRIVER_GET(sifive_reset),
					  &dev);
	if (ret) {
		dev_err(dev, "Reset driver not found: %d\n", ret);
		return ret;
	}

	ret = reset_get_by_name(dev, rst_name, &rst_sig);
	if (ret) {
		dev_err(dev, "failed to get %s reset\n", rst_name);
		return ret;
	}

	if (reset_valid(&rst_sig)) {
		if (trigger)
			ret = reset_deassert(&rst_sig);
		else
			ret = reset_assert(&rst_sig);
		if (ret) {
			dev_err(dev, "failed to trigger reset id = %ld\n",
				rst_sig.id);
			return ret;
		}
	}

	return ret;
}

/**
 * sifive_prci_ddr_release_reset() - Release DDR reset
 * @pd: struct __prci_data * for the PRCI containing the DDRCLK mux reg
 *
 */
void sifive_prci_ddr_release_reset(struct __prci_data *pd)
{
	/* Release DDR ctrl reset */
	__prci_consumer_reset("ddr_ctrl", true);

	/* HACK to get the '1 full controller clock cycle'. */
	asm volatile ("fence");

	/* Release DDR AXI reset */
	__prci_consumer_reset("ddr_axi", true);

	/* Release DDR AHB reset */
	__prci_consumer_reset("ddr_ahb", true);

	/* Release DDR PHY reset */
	__prci_consumer_reset("ddr_phy", true);

	/* HACK to get the '1 full controller clock cycle'. */
	asm volatile ("fence");

	/*
	 * These take like 16 cycles to actually propagate. We can't go sending
	 * stuff before they come out of reset. So wait.
	 */
	for (int i = 0; i < 256; i++)
		asm volatile ("nop");
}

/**
 * sifive_prci_ethernet_release_reset() - Release ethernet reset
 * @pd: struct __prci_data * for the PRCI containing the Ethernet CLK mux reg
 *
 */
void sifive_prci_ethernet_release_reset(struct __prci_data *pd)
{
	/* Release GEMGXL reset */
	__prci_consumer_reset("gemgxl_reset", true);

	/* Procmon => core clock */
	__prci_writel(PRCI_PROCMONCFG_CORE_CLOCK_MASK, PRCI_PROCMONCFG_OFFSET,
		      pd);

	/* Release Chiplink reset */
	__prci_consumer_reset("cltx_reset", true);
}

/**
 * sifive_prci_cltx_release_reset() - Release cltx reset
 * @pd: struct __prci_data * for the PRCI containing the Ethernet CLK mux reg
 *
 */
void sifive_prci_cltx_release_reset(struct __prci_data *pd)
{
	/* Release CLTX reset */
	__prci_consumer_reset("cltx_reset", true);
}

/* Core clock mux control */

/**
 * sifive_prci_coreclksel_use_hfclk() - switch the CORECLK mux to output HFCLK
 * @pd: struct __prci_data * for the PRCI containing the CORECLK mux reg
 *
 * Switch the CORECLK mux to the HFCLK input source; return once complete.
 *
 * Context: Any context.  Caller must prevent concurrent changes to the
 *          PRCI_CORECLKSEL_OFFSET register.
 */
void sifive_prci_coreclksel_use_hfclk(struct __prci_data *pd)
{
	u32 r;

	r = __prci_readl(pd, PRCI_CORECLKSEL_OFFSET);
	r |= PRCI_CORECLKSEL_CORECLKSEL_MASK;
	__prci_writel(r, PRCI_CORECLKSEL_OFFSET, pd);

	r = __prci_readl(pd, PRCI_CORECLKSEL_OFFSET); /* barrier */
}

/**
 * sifive_prci_coreclksel_use_corepll() - switch the CORECLK mux to output COREPLL
 * @pd: struct __prci_data * for the PRCI containing the CORECLK mux reg
 *
 * Switch the CORECLK mux to the PLL output clock; return once complete.
 *
 * Context: Any context.  Caller must prevent concurrent changes to the
 *          PRCI_CORECLKSEL_OFFSET register.
 */
void sifive_prci_coreclksel_use_corepll(struct __prci_data *pd)
{
	u32 r;

	r = __prci_readl(pd, PRCI_CORECLKSEL_OFFSET);
	r &= ~PRCI_CORECLKSEL_CORECLKSEL_MASK;
	__prci_writel(r, PRCI_CORECLKSEL_OFFSET, pd);

	r = __prci_readl(pd, PRCI_CORECLKSEL_OFFSET); /* barrier */
}

static ulong sifive_prci_parent_rate(struct __prci_clock *pc, struct prci_clk_desc *data)
{
	ulong parent_rate;
	ulong i;
	struct __prci_clock *p;

	if (strcmp(pc->parent_name, "corepll") == 0 ||
	    strcmp(pc->parent_name, "hfpclkpll") == 0) {
		for (i = 0; i < data->num_clks; i++) {
			if (strcmp(pc->parent_name, data->clks[i].name) == 0)
				break;
		}

		if (i >= data->num_clks)
			return -ENXIO;

		p = &data->clks[i];
		if (!p->pd || !p->ops->recalc_rate)
			return -ENXIO;

		return p->ops->recalc_rate(p, sifive_prci_parent_rate(p, data));
	}

	if (strcmp(pc->parent_name, "rtcclk") == 0)
		parent_rate = clk_get_rate(&pc->pd->parent_rtcclk);
	else
		parent_rate = clk_get_rate(&pc->pd->parent_hfclk);

	return parent_rate;
}

static ulong sifive_prci_get_rate(struct clk *clk)
{
	struct __prci_clock *pc;
	struct prci_clk_desc *data =
		(struct prci_clk_desc *)dev_get_driver_data(clk->dev);

	if (data->num_clks <= clk->id)
		return -ENXIO;

	pc = &data->clks[clk->id];
	if (!pc->pd || !pc->ops->recalc_rate)
		return -ENXIO;

	return pc->ops->recalc_rate(pc, sifive_prci_parent_rate(pc, data));
}

static ulong sifive_prci_set_rate(struct clk *clk, ulong rate)
{
	int err;
	struct __prci_clock *pc;
	struct prci_clk_desc *data =
		(struct prci_clk_desc *)dev_get_driver_data(clk->dev);

	if (data->num_clks <= clk->id)
		return -ENXIO;

	pc = &data->clks[clk->id];
	if (!pc->pd || !pc->ops->set_rate)
		return -ENXIO;

	err = pc->ops->set_rate(pc, rate, sifive_prci_parent_rate(pc, data));
	if (err)
		return err;

	return rate;
}

static int sifive_prci_enable(struct clk *clk)
{
	struct __prci_clock *pc;
	int ret = 0;
	struct prci_clk_desc *data =
		(struct prci_clk_desc *)dev_get_driver_data(clk->dev);

	if (data->num_clks <= clk->id)
		return -ENXIO;

	pc = &data->clks[clk->id];
	if (!pc->pd)
		return -ENXIO;

	if (pc->ops->enable_clk)
		ret = pc->ops->enable_clk(pc, 1);

	return ret;
}

static int sifive_prci_disable(struct clk *clk)
{
	struct __prci_clock *pc;
	int ret = 0;
	struct prci_clk_desc *data =
		(struct prci_clk_desc *)dev_get_driver_data(clk->dev);

	if (data->num_clks <= clk->id)
		return -ENXIO;

	pc = &data->clks[clk->id];
	if (!pc->pd)
		return -ENXIO;

	if (pc->ops->enable_clk)
		ret = pc->ops->enable_clk(pc, 0);

	return ret;
}

static int sifive_prci_probe(struct udevice *dev)
{
	int i, err;
	struct __prci_clock *pc;
	struct __prci_data *pd = dev_get_priv(dev);

	struct prci_clk_desc *data =
		(struct prci_clk_desc *)dev_get_driver_data(dev);

	pd->va = dev_read_addr_ptr(dev);
	if (!pd->va)
		return -EINVAL;

	err = clk_get_by_index(dev, 0, &pd->parent_hfclk);
	if (err)
		return err;

	err = clk_get_by_index(dev, 1, &pd->parent_rtcclk);
	if (err)
		return err;

	for (i = 0; i < data->num_clks; ++i) {
		pc = &data->clks[i];
		pc->pd = pd;
		if (pc->pwd)
			__prci_wrpll_read_cfg0(pd, pc->pwd);
	}

	if (IS_ENABLED(CONFIG_SPL_BUILD)) {
		if (device_is_compatible(dev, "sifive,fu740-c000-prci")) {
			u32 prci_pll_reg;
			unsigned long parent_rate;

			prci_pll_reg = readl(pd->va + PRCI_PRCIPLL_OFFSET);

			if (prci_pll_reg & PRCI_PRCIPLL_HFPCLKPLL) {
				/*
				 * Only initialize the HFPCLK PLL. In this
				 * case the design uses hfpclk to drive
				 * Chiplink
				 */
				pc = &data->clks[FU740_PRCI_CLK_HFPCLKPLL];
				parent_rate = sifive_prci_parent_rate(pc, data);
				sifive_prci_wrpll_set_rate(pc, 260000000,
							   parent_rate);
				pc->ops->enable_clk(pc, 1);
			} else if (prci_pll_reg & PRCI_PRCIPLL_CLTXPLL) {
				/* CLTX pll init */
				pc = &data->clks[FU740_PRCI_CLK_CLTXPLL];
				parent_rate = sifive_prci_parent_rate(pc, data);
				sifive_prci_wrpll_set_rate(pc, 260000000,
							   parent_rate);
				pc->ops->enable_clk(pc, 1);
			}
		}
	}

	return 0;
}

static struct clk_ops sifive_prci_ops = {
	.set_rate = sifive_prci_set_rate,
	.get_rate = sifive_prci_get_rate,
	.enable = sifive_prci_enable,
	.disable = sifive_prci_disable,
};

static int sifive_clk_bind(struct udevice *dev)
{
	return sifive_reset_bind(dev, PRCI_DEVICERESETCNT);
}

static const struct udevice_id sifive_prci_ids[] = {
	{ .compatible = "sifive,fu540-c000-prci", .data = (ulong)&prci_clk_fu540 },
	{ .compatible = "sifive,fu740-c000-prci", .data = (ulong)&prci_clk_fu740 },
	{ }
};

U_BOOT_DRIVER(sifive_prci) = {
	.name = "sifive-prci",
	.id = UCLASS_CLK,
	.of_match = sifive_prci_ids,
	.probe = sifive_prci_probe,
	.ops = &sifive_prci_ops,
	.priv_auto = sizeof(struct __prci_data),
	.bind = sifive_clk_bind,
};
