// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Broadcom Corporation.
 */

/*
 *
 * bcm235xx architecture clock framework
 *
 */

#include <common.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <bitfield.h>
#include <asm/arch/sysmap.h>
#include <asm/kona-common/clk.h>
#include "clk-core.h"

#define CLK_WR_ACCESS_PASSWORD	0x00a5a501
#define WR_ACCESS_OFFSET	0	/* common to all clock blocks */
#define POLICY_CTL_GO		1	/* Load and refresh policy masks */
#define POLICY_CTL_GO_ATL	4	/* Active Load */

/* Helper function */
int clk_get_and_enable(char *clkstr)
{
	int ret = 0;
	struct clk *c;

	debug("%s: %s\n", __func__, clkstr);

	c = clk_get(clkstr);
	if (c) {
		ret = clk_enable(c);
		if (ret)
			return ret;
	} else {
		printf("%s: Couldn't find %s\n", __func__, clkstr);
		return -EINVAL;
	}
	return ret;
}

/*
 * Poll a register in a CCU's address space, returning when the
 * specified bit in that register's value is set (or clear). Delay
 * a microsecond after each read of the register. Returns true if
 * successful, or false if we gave up trying.
 *
 * Caller must ensure the CCU lock is held.
 */
#define CLK_GATE_DELAY_USEC 2000
static inline int wait_bit(void *base, u32 offset, u32 bit, bool want)
{
	unsigned int tries;
	u32 bit_mask = 1 << bit;

	for (tries = 0; tries < CLK_GATE_DELAY_USEC; tries++) {
		u32 val;
		bool bit_val;

		val = readl(base + offset);
		bit_val = (val & bit_mask) ? 1 : 0;
		if (bit_val == want)
			return 0;	/* success */
		udelay(1);
	}

	debug("%s: timeout on addr 0x%p, waiting for bit %d to go to %d\n",
	      __func__, base + offset, bit, want);

	return -ETIMEDOUT;
}

/* Enable a peripheral clock */
static int peri_clk_enable(struct clk *c, int enable)
{
	int ret = 0;
	u32 reg;
	struct peri_clock *peri_clk = to_peri_clk(c);
	struct peri_clk_data *cd = peri_clk->data;
	struct bcm_clk_gate *gate = &cd->gate;
	void *base = (void *)c->ccu_clk_mgr_base;


	debug("%s: %s\n", __func__, c->name);

	clk_get_rate(c);	/* Make sure rate and sel are filled in */

	/* enable access */
	writel(CLK_WR_ACCESS_PASSWORD, base + WR_ACCESS_OFFSET);

	if (enable) {
		debug("%s %s set rate %lu div %lu sel %d parent %lu\n",
		      __func__, c->name, c->rate, c->div, c->sel,
		      c->parent->rate);

		/*
		 * clkgate - only software controllable gates are
		 * supported by u-boot which includes all clocks
		 * that matter. This avoids bringing in a lot of extra
		 * complexity as done in the kernel framework.
		 */
		if (gate_exists(gate)) {
			reg = readl(base + cd->gate.offset);
			reg |= (1 << cd->gate.en_bit);
			writel(reg, base + cd->gate.offset);
		}

		/* div and pll select */
		if (divider_exists(&cd->div)) {
			reg = readl(base + cd->div.offset);
			bitfield_replace(reg, cd->div.shift, cd->div.width,
					 c->div - 1);
			writel(reg, base + cd->div.offset);
		}

		/* frequency selector */
		if (selector_exists(&cd->sel)) {
			reg = readl(base + cd->sel.offset);
			bitfield_replace(reg, cd->sel.shift, cd->sel.width,
					 c->sel);
			writel(reg, base + cd->sel.offset);
		}

		/* trigger */
		if (trigger_exists(&cd->trig)) {
			writel((1 << cd->trig.bit), base + cd->trig.offset);

			/* wait for trigger status bit to go to 0 */
			ret = wait_bit(base, cd->trig.offset, cd->trig.bit, 0);
			if (ret)
				return ret;
		}

		/* wait for running (status_bit = 1) */
		ret = wait_bit(base, cd->gate.offset, cd->gate.status_bit, 1);
		if (ret)
			return ret;
	} else {
		debug("%s disable clock %s\n", __func__, c->name);

		/* clkgate */
		reg = readl(base + cd->gate.offset);
		reg &= ~(1 << cd->gate.en_bit);
		writel(reg, base + cd->gate.offset);

		/* wait for stop (status_bit = 0) */
		ret = wait_bit(base, cd->gate.offset, cd->gate.status_bit, 0);
	}

	/* disable access */
	writel(0, base + WR_ACCESS_OFFSET);

	return ret;
}

/* Set the rate of a peripheral clock */
static int peri_clk_set_rate(struct clk *c, unsigned long rate)
{
	int ret = 0;
	int i;
	unsigned long diff;
	unsigned long new_rate = 0, div = 1;
	struct peri_clock *peri_clk = to_peri_clk(c);
	struct peri_clk_data *cd = peri_clk->data;
	const char **clock;

	debug("%s: %s\n", __func__, c->name);
	diff = rate;

	i = 0;
	for (clock = cd->clocks; *clock; clock++, i++) {
		struct refclk *ref = refclk_str_to_clk(*clock);
		if (!ref) {
			printf("%s: Lookup of %s failed\n", __func__, *clock);
			return -EINVAL;
		}

		/* round to the new rate */
		div = ref->clk.rate / rate;
		if (div == 0)
			div = 1;

		new_rate = ref->clk.rate / div;

		/* get the min diff */
		if (abs(new_rate - rate) < diff) {
			diff = abs(new_rate - rate);
			c->sel = i;
			c->parent = &ref->clk;
			c->rate = new_rate;
			c->div = div;
		}
	}

	debug("%s %s set rate %lu div %lu sel %d parent %lu\n", __func__,
	      c->name, c->rate, c->div, c->sel, c->parent->rate);
	return ret;
}

/* Get the rate of a peripheral clock */
static unsigned long peri_clk_get_rate(struct clk *c)
{
	struct peri_clock *peri_clk = to_peri_clk(c);
	struct peri_clk_data *cd = peri_clk->data;
	void *base = (void *)c->ccu_clk_mgr_base;
	int div = 1;
	const char **clock;
	struct refclk *ref;
	u32 reg;

	debug("%s: %s\n", __func__, c->name);
	if (selector_exists(&cd->sel)) {
		reg = readl(base + cd->sel.offset);
		c->sel = bitfield_extract(reg, cd->sel.shift, cd->sel.width);
	} else {
		/*
		 * For peri clocks that don't have a selector, the single
		 * reference clock will always exist at index 0.
		 */
		c->sel = 0;
	}

	if (divider_exists(&cd->div)) {
		reg = readl(base + cd->div.offset);
		div = bitfield_extract(reg, cd->div.shift, cd->div.width);
		div += 1;
	}

	clock = cd->clocks;
	ref = refclk_str_to_clk(clock[c->sel]);
	if (!ref) {
		printf("%s: Can't lookup %s\n", __func__, clock[c->sel]);
		return 0;
	}

	c->parent = &ref->clk;
	c->div = div;
	c->rate = c->parent->rate / c->div;
	debug("%s parent rate %lu div %d sel %d rate %lu\n", __func__,
	      c->parent->rate, div, c->sel, c->rate);

	return c->rate;
}

/* Peripheral clock operations */
struct clk_ops peri_clk_ops = {
	.enable = peri_clk_enable,
	.set_rate = peri_clk_set_rate,
	.get_rate = peri_clk_get_rate,
};

/* Enable a CCU clock */
static int ccu_clk_enable(struct clk *c, int enable)
{
	struct ccu_clock *ccu_clk = to_ccu_clk(c);
	void *base = (void *)c->ccu_clk_mgr_base;
	int ret = 0;
	u32 reg;

	debug("%s: %s\n", __func__, c->name);
	if (!enable)
		return -EINVAL;	/* CCU clock cannot shutdown */

	/* enable access */
	writel(CLK_WR_ACCESS_PASSWORD, base + WR_ACCESS_OFFSET);

	/* config enable for policy engine */
	writel(1, base + ccu_clk->lvm_en_offset);

	/* wait for bit to go to 0 */
	ret = wait_bit(base, ccu_clk->lvm_en_offset, 0, 0);
	if (ret)
		return ret;

	/* freq ID */
	if (!ccu_clk->freq_bit_shift)
		ccu_clk->freq_bit_shift = 8;

	/* Set frequency id for each of the 4 policies */
	reg = ccu_clk->freq_id |
	    (ccu_clk->freq_id << (ccu_clk->freq_bit_shift)) |
	    (ccu_clk->freq_id << (ccu_clk->freq_bit_shift * 2)) |
	    (ccu_clk->freq_id << (ccu_clk->freq_bit_shift * 3));
	writel(reg, base + ccu_clk->policy_freq_offset);

	/* enable all clock mask */
	writel(0x7fffffff, base + ccu_clk->policy0_mask_offset);
	writel(0x7fffffff, base + ccu_clk->policy1_mask_offset);
	writel(0x7fffffff, base + ccu_clk->policy2_mask_offset);
	writel(0x7fffffff, base + ccu_clk->policy3_mask_offset);

	if (ccu_clk->num_policy_masks == 2) {
		writel(0x7fffffff, base + ccu_clk->policy0_mask2_offset);
		writel(0x7fffffff, base + ccu_clk->policy1_mask2_offset);
		writel(0x7fffffff, base + ccu_clk->policy2_mask2_offset);
		writel(0x7fffffff, base + ccu_clk->policy3_mask2_offset);
	}

	/* start policy engine */
	reg = readl(base + ccu_clk->policy_ctl_offset);
	reg |= (POLICY_CTL_GO + POLICY_CTL_GO_ATL);
	writel(reg, base + ccu_clk->policy_ctl_offset);

	/* wait till started */
	ret = wait_bit(base, ccu_clk->policy_ctl_offset, 0, 0);
	if (ret)
		return ret;

	/* disable access */
	writel(0, base + WR_ACCESS_OFFSET);

	return ret;
}

/* Get the CCU clock rate */
static unsigned long ccu_clk_get_rate(struct clk *c)
{
	struct ccu_clock *ccu_clk = to_ccu_clk(c);
	debug("%s: %s\n", __func__, c->name);
	c->rate = ccu_clk->freq_tbl[ccu_clk->freq_id];
	return c->rate;
}

/* CCU clock operations */
struct clk_ops ccu_clk_ops = {
	.enable = ccu_clk_enable,
	.get_rate = ccu_clk_get_rate,
};

/* Enable a bus clock */
static int bus_clk_enable(struct clk *c, int enable)
{
	struct bus_clock *bus_clk = to_bus_clk(c);
	struct bus_clk_data *cd = bus_clk->data;
	void *base = (void *)c->ccu_clk_mgr_base;
	int ret = 0;
	u32 reg;

	debug("%s: %s\n", __func__, c->name);
	/* enable access */
	writel(CLK_WR_ACCESS_PASSWORD, base + WR_ACCESS_OFFSET);

	/* enable gating */
	reg = readl(base + cd->gate.offset);
	if (!!(reg & (1 << cd->gate.status_bit)) == !!enable)
		debug("%s already %s\n", c->name,
		      enable ? "enabled" : "disabled");
	else {
		int want = (enable) ? 1 : 0;
		reg |= (1 << cd->gate.hw_sw_sel_bit);

		if (enable)
			reg |= (1 << cd->gate.en_bit);
		else
			reg &= ~(1 << cd->gate.en_bit);

		writel(reg, base + cd->gate.offset);
		ret = wait_bit(base, cd->gate.offset, cd->gate.status_bit,
			       want);
		if (ret)
			return ret;
	}

	/* disable access */
	writel(0, base + WR_ACCESS_OFFSET);

	return ret;
}

/* Get the rate of a bus clock */
static unsigned long bus_clk_get_rate(struct clk *c)
{
	struct bus_clock *bus_clk = to_bus_clk(c);
	struct ccu_clock *ccu_clk;

	debug("%s: %s\n", __func__, c->name);
	ccu_clk = to_ccu_clk(c->parent);

	c->rate = bus_clk->freq_tbl[ccu_clk->freq_id];
	c->div = ccu_clk->freq_tbl[ccu_clk->freq_id] / c->rate;
	return c->rate;
}

/* Bus clock operations */
struct clk_ops bus_clk_ops = {
	.enable = bus_clk_enable,
	.get_rate = bus_clk_get_rate,
};

/* Enable a reference clock */
static int ref_clk_enable(struct clk *c, int enable)
{
	debug("%s: %s\n", __func__, c->name);
	return 0;
}

/* Reference clock operations */
struct clk_ops ref_clk_ops = {
	.enable = ref_clk_enable,
};

/*
 * clk.h implementation follows
 */

/* Initialize the clock framework */
int clk_init(void)
{
	debug("%s:\n", __func__);
	return 0;
}

/* Get a clock handle, give a name string */
struct clk *clk_get(const char *con_id)
{
	int i;
	struct clk_lookup *clk_tblp;

	debug("%s: %s\n", __func__, con_id);

	clk_tblp = arch_clk_tbl;
	for (i = 0; i < arch_clk_tbl_array_size; i++, clk_tblp++) {
		if (clk_tblp->con_id) {
			if (!con_id || strcmp(clk_tblp->con_id, con_id))
				continue;
			return clk_tblp->clk;
		}
	}
	return NULL;
}

/* Enable a clock */
int clk_enable(struct clk *c)
{
	int ret = 0;

	debug("%s: %s\n", __func__, c->name);
	if (!c->ops || !c->ops->enable)
		return -1;

	/* enable parent clock first */
	if (c->parent)
		ret = clk_enable(c->parent);

	if (ret)
		return ret;

	if (!c->use_cnt)
		ret = c->ops->enable(c, 1);
	c->use_cnt++;

	return ret;
}

/* Disable a clock */
void clk_disable(struct clk *c)
{
	debug("%s: %s\n", __func__, c->name);
	if (!c->ops || !c->ops->enable)
		return;

	if (c->use_cnt > 0) {
		c->use_cnt--;
		if (c->use_cnt == 0)
			c->ops->enable(c, 0);
	}

	/* disable parent */
	if (c->parent)
		clk_disable(c->parent);
}

/* Get the clock rate */
unsigned long clk_get_rate(struct clk *c)
{
	unsigned long rate;

	if (!c || !c->ops || !c->ops->get_rate)
		return 0;
	debug("%s: %s\n", __func__, c->name);

	rate = c->ops->get_rate(c);
	debug("%s: rate = %ld\n", __func__, rate);
	return rate;
}

/* Set the clock rate */
int clk_set_rate(struct clk *c, unsigned long rate)
{
	int ret;

	if (!c || !c->ops || !c->ops->set_rate)
		return -EINVAL;
	debug("%s: %s rate=%ld\n", __func__, c->name, rate);

	if (c->use_cnt)
		return -EINVAL;

	ret = c->ops->set_rate(c, rate);

	return ret;
}

/* Not required for this arch */
/*
long clk_round_rate(struct clk *clk, unsigned long rate);
int clk_set_parent(struct clk *clk, struct clk *parent);
struct clk *clk_get_parent(struct clk *clk);
*/
