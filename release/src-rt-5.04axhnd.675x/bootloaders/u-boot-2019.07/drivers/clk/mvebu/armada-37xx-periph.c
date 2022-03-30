// SPDX-License-Identifier: GPL-2.0+
/*
 * Marvell Armada 37xx SoC Peripheral clocks
 *
 * Marek Behun <marek.behun@nic.cz>
 *
 * Based on Linux driver by:
 *   Gregory CLEMENT <gregory.clement@free-electrons.com>
 */

#include <common.h>
#include <malloc.h>
#include <clk-uclass.h>
#include <clk.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>

#define TBG_SEL		0x0
#define DIV_SEL0	0x4
#define DIV_SEL1	0x8
#define DIV_SEL2	0xC
#define CLK_SEL		0x10
#define CLK_DIS		0x14

enum a37xx_periph_parent {
	TBG_A_P		= 0,
	TBG_B_P		= 1,
	TBG_A_S		= 2,
	TBG_B_S		= 3,
	MAX_TBG_PARENTS	= 4,
	XTAL		= 4,
	MAX_PARENTS	= 5,
};

static const struct {
	const char *name;
	enum a37xx_periph_parent parent;
} a37xx_periph_parent_names[] = {
	{ "TBG-A-P", TBG_A_P },
	{ "TBG-B-P", TBG_B_P },
	{ "TBG-A-S", TBG_A_S },
	{ "TBG-B-S", TBG_B_S },
	{ "xtal",    XTAL    },
};

struct clk_periph;

struct a37xx_periphclk {
	void __iomem *reg;

	ulong parents[MAX_PARENTS];

	const struct clk_periph *clks;
	bool clk_has_periph_parent[16];
	int clk_parent[16];

	int count;
};

struct clk_div_table {
	u32 div;
	u32 val;
};

struct clk_periph {
	const char *name;

	const char *parent_name;

	u32 disable_bit;
	int mux_shift;

	const struct clk_div_table *div_table[2];
	s32 div_reg_off[2];
	u32 div_mask[2];
	int div_shift[2];

	unsigned can_gate : 1;
	unsigned can_mux : 1;
	unsigned dividers : 2;
};

static const struct clk_div_table div_table1[] = {
	{ 1, 1 },
	{ 2, 2 },
	{ 0, 0 },
};

static const struct clk_div_table div_table2[] = {
	{ 2, 1 },
	{ 4, 2 },
	{ 0, 0 },
};

static const struct clk_div_table div_table6[] = {
	{ 1, 1 },
	{ 2, 2 },
	{ 3, 3 },
	{ 4, 4 },
	{ 5, 5 },
	{ 6, 6 },
	{ 0, 0 },
};

#define CLK_FULL_DD(_n, _d, _mux, _r0, _r1, _s0, _s1)	\
	{						\
		.name = #_n,				\
		.disable_bit = BIT(_d),			\
		.mux_shift = _mux,			\
		.div_table[0] = div_table6,		\
		.div_table[1] = div_table6,		\
		.div_reg_off[0] = _r0,			\
		.div_reg_off[1] = _r1,			\
		.div_shift[0] = _s0,			\
		.div_shift[1] = _s1,			\
		.div_mask[0] = 7,			\
		.div_mask[1] = 7,			\
		.can_gate = 1,				\
		.can_mux = 1,				\
		.dividers = 2,				\
	}

#define CLK_FULL(_n, _d, _mux, _r, _s, _m, _t)	\
	{					\
		.name = #_n,			\
		.disable_bit = BIT(_d),		\
		.mux_shift = _mux,		\
		.div_table[0] = _t,		\
		.div_reg_off[0] = _r,		\
		.div_shift[0] = _s,		\
		.div_mask[0] = _m,		\
		.can_gate = 1,			\
		.can_mux = 1,			\
		.dividers = 1,			\
	}

#define CLK_GATE_DIV(_n, _d, _r, _s, _m, _t, _p)	\
	{						\
		.name = #_n,				\
		.parent_name = _p,			\
		.disable_bit = BIT(_d),			\
		.div_table[0] = _t,			\
		.div_reg_off[0] = _r,			\
		.div_shift[0] = _s,			\
		.div_mask[0] = _m,			\
		.can_gate = 1,				\
		.dividers = 1,				\
	}

#define CLK_GATE(_n, _d, _p)		\
	{				\
		.name = #_n,		\
		.parent_name = _p,	\
		.disable_bit = BIT(_d),	\
		.can_gate = 1,		\
	}

#define CLK_MUX_DIV(_n, _mux, _r, _s, _m, _t)	\
	{					\
		.name = #_n,			\
		.mux_shift = _mux,		\
		.div_table[0] = _t,		\
		.div_reg_off[0] = _r,		\
		.div_shift[0] = _s,		\
		.div_mask[0] = _m,		\
		.can_mux = 1,			\
		.dividers = 1,			\
	}

#define CLK_MUX_DD(_n, _mux, _r0, _r1, _s0, _s1)	\
	{						\
		.name = #_n,				\
		.mux_shift = _mux,			\
		.div_table[0] = div_table6,		\
		.div_table[1] = div_table6,		\
		.div_reg_off[0] = _r0,			\
		.div_reg_off[1] = _r1,			\
		.div_shift[0] = _s0,			\
		.div_shift[1] = _s1,			\
		.div_mask[0] = 7,			\
		.div_mask[1] = 7,			\
		.can_mux = 1,				\
		.dividers = 2,				\
	}

/* NB periph clocks */
static const struct clk_periph clks_nb[] = {
	CLK_FULL_DD(mmc, 2, 0, DIV_SEL2, DIV_SEL2, 16, 13),
	CLK_FULL_DD(sata_host, 3, 2, DIV_SEL2, DIV_SEL2, 10, 7),
	CLK_FULL_DD(sec_at, 6, 4, DIV_SEL1, DIV_SEL1, 3, 0),
	CLK_FULL_DD(sec_dap, 7, 6, DIV_SEL1, DIV_SEL1, 9, 6),
	CLK_FULL_DD(tscem, 8, 8, DIV_SEL1, DIV_SEL1, 15, 12),
	CLK_FULL(tscem_tmx, 10, 10, DIV_SEL1, 18, 7, div_table6),
	CLK_GATE(avs, 11, "xtal"),
	CLK_FULL_DD(sqf, 12, 12, DIV_SEL1, DIV_SEL1, 27, 24),
	CLK_FULL_DD(pwm, 13, 14, DIV_SEL0, DIV_SEL0, 3, 0),
	CLK_GATE(i2c_2, 16, "xtal"),
	CLK_GATE(i2c_1, 17, "xtal"),
	CLK_GATE_DIV(ddr_phy, 19, DIV_SEL0, 18, 1, div_table2, "TBG-A-S"),
	CLK_FULL_DD(ddr_fclk, 21, 16, DIV_SEL0, DIV_SEL0, 15, 12),
	CLK_FULL(trace, 22, 18, DIV_SEL0, 20, 7, div_table6),
	CLK_FULL(counter, 23, 20, DIV_SEL0, 23, 7, div_table6),
	CLK_FULL_DD(eip97, 24, 24, DIV_SEL2, DIV_SEL2, 22, 19),
	CLK_MUX_DIV(cpu, 22, DIV_SEL0, 28, 7, div_table6),
	{ },
};

/* SB periph clocks */
static const struct clk_periph clks_sb[] = {
	CLK_MUX_DD(gbe_50, 6, DIV_SEL2, DIV_SEL2, 6, 9),
	CLK_MUX_DD(gbe_core, 8, DIV_SEL1, DIV_SEL1, 18, 21),
	CLK_MUX_DD(gbe_125, 10, DIV_SEL1, DIV_SEL1, 6, 9),
	CLK_GATE(gbe1_50, 0, "gbe_50"),
	CLK_GATE(gbe0_50, 1, "gbe_50"),
	CLK_GATE(gbe1_125, 2, "gbe_125"),
	CLK_GATE(gbe0_125, 3, "gbe_125"),
	CLK_GATE_DIV(gbe1_core, 4, DIV_SEL1, 13, 1, div_table1, "gbe_core"),
	CLK_GATE_DIV(gbe0_core, 5, DIV_SEL1, 14, 1, div_table1, "gbe_core"),
	CLK_GATE_DIV(gbe_bm, 12, DIV_SEL1, 0, 1, div_table1, "gbe_core"),
	CLK_FULL_DD(sdio, 11, 14, DIV_SEL0, DIV_SEL0, 3, 6),
	CLK_FULL_DD(usb32_usb2_sys, 16, 16, DIV_SEL0, DIV_SEL0, 9, 12),
	CLK_FULL_DD(usb32_ss_sys, 17, 18, DIV_SEL0, DIV_SEL0, 15, 18),
	{ },
};

static int get_mux(struct a37xx_periphclk *priv, int shift)
{
	return (readl(priv->reg + TBG_SEL) >> shift) & 3;
}

static void set_mux(struct a37xx_periphclk *priv, int shift, int val)
{
	u32 reg;

	reg = readl(priv->reg + TBG_SEL);
	reg &= ~(3 << shift);
	reg |= (val & 3) << shift;
	writel(reg, priv->reg + TBG_SEL);
}

static ulong periph_clk_get_rate(struct a37xx_periphclk *priv, int id);

static ulong get_parent_rate(struct a37xx_periphclk *priv, int id)
{
	const struct clk_periph *clk = &priv->clks[id];
	ulong res;

	if (clk->can_mux) {
		/* parent is one of TBG clocks */
		int tbg = get_mux(priv, clk->mux_shift);

		res = priv->parents[tbg];
	} else if (priv->clk_has_periph_parent[id]) {
		/* parent is one of other periph clocks */

		if (priv->clk_parent[id] >= priv->count)
			return -EINVAL;

		res = periph_clk_get_rate(priv, priv->clk_parent[id]);
	} else {
		/* otherwise parent is one of TBGs or XTAL */

		if (priv->clk_parent[id] >= MAX_PARENTS)
			return -EINVAL;

		res = priv->parents[priv->clk_parent[id]];
	}

	return res;
}

static ulong get_div(struct a37xx_periphclk *priv,
		     const struct clk_periph *clk, int idx)
{
	const struct clk_div_table *i;
	u32 reg;

	reg = readl(priv->reg + clk->div_reg_off[idx]);
	reg = (reg >> clk->div_shift[idx]) & clk->div_mask[idx];

	/* find divisor for register value val */
	for (i = clk->div_table[idx]; i && i->div != 0; ++i)
		if (i->val == reg)
			return i->div;

	return 0;
}

static void set_div_val(struct a37xx_periphclk *priv,
			const struct clk_periph *clk, int idx, int val)
{
	u32 reg;

	reg = readl(priv->reg + clk->div_reg_off[idx]);
	reg &= ~(clk->div_mask[idx] << clk->div_shift[idx]);
	reg |= (val & clk->div_mask[idx]) << clk->div_shift[idx];
	writel(reg, priv->reg + clk->div_reg_off[idx]);
}

static ulong periph_clk_get_rate(struct a37xx_periphclk *priv, int id)
{
	const struct clk_periph *clk = &priv->clks[id];
	ulong rate, div;
	int i;

	rate = get_parent_rate(priv, id);
	if (rate == -EINVAL)
		return -EINVAL;

	/* divide the parent rate by dividers */
	div = 1;
	for (i = 0; i < clk->dividers; ++i)
		div *= get_div(priv, clk, i);

	if (!div)
		return 0;

	return DIV_ROUND_UP(rate, div);
}

static ulong armada_37xx_periph_clk_get_rate(struct clk *clk)
{
	struct a37xx_periphclk *priv = dev_get_priv(clk->dev);

	if (clk->id >= priv->count)
		return -EINVAL;

	return periph_clk_get_rate(priv, clk->id);
}

static int periph_clk_enable(struct clk *clk, int enable)
{
	struct a37xx_periphclk *priv = dev_get_priv(clk->dev);
	const struct clk_periph *periph_clk = &priv->clks[clk->id];

	if (clk->id >= priv->count)
		return -EINVAL;

	if (!periph_clk->can_gate)
		return -ENOTSUPP;

	if (enable)
		clrbits_le32(priv->reg + CLK_DIS, periph_clk->disable_bit);
	else
		setbits_le32(priv->reg + CLK_DIS, periph_clk->disable_bit);

	return 0;
}

static int armada_37xx_periph_clk_enable(struct clk *clk)
{
	return periph_clk_enable(clk, 1);
}

static int armada_37xx_periph_clk_disable(struct clk *clk)
{
	return periph_clk_enable(clk, 0);
}

#define diff(a, b) abs((long)(a) - (long)(b))

static ulong find_best_div(const struct clk_div_table *t0,
			   const struct clk_div_table *t1, ulong parent_rate,
			   ulong req_rate, int *v0, int *v1)
{
	const struct clk_div_table *i, *j;
	ulong rate, best_rate = 0;

	for (i = t0; i && i->div; ++i) {
		for (j = t1; j && j->div; ++j) {
			rate = DIV_ROUND_UP(parent_rate, i->div * j->div);

			if (!best_rate ||
			    diff(rate, req_rate) < diff(best_rate, req_rate)) {
				best_rate = rate;
				*v0 = i->val;
				*v1 = j->val;
			}
		}
	}

	return best_rate;
}

static ulong armada_37xx_periph_clk_set_rate(struct clk *clk, ulong req_rate)
{
	struct a37xx_periphclk *priv = dev_get_priv(clk->dev);
	const struct clk_periph *periph_clk = &priv->clks[clk->id];
	ulong rate, old_rate, parent_rate;
	int div_val0 = 0, div_val1 = 0;
	const struct clk_div_table *t1;
	static const struct clk_div_table empty_table[2] = {
		{ 1, 0 },
		{ 0, 0 }
	};

	if (clk->id > priv->count)
		return -EINVAL;

	old_rate = periph_clk_get_rate(priv, clk->id);
	if (old_rate == -EINVAL)
		return -EINVAL;

	if (old_rate == req_rate)
		return old_rate;

	if (!periph_clk->can_gate || !periph_clk->dividers)
		return -ENOTSUPP;

	parent_rate = get_parent_rate(priv, clk->id);
	if (parent_rate == -EINVAL)
		return -EINVAL;

	t1 = empty_table;
	if (periph_clk->dividers > 1)
		t1 = periph_clk->div_table[1];

	rate = find_best_div(periph_clk->div_table[0], t1, parent_rate,
			     req_rate, &div_val0, &div_val1);

	periph_clk_enable(clk, 0);

	set_div_val(priv, periph_clk, 0, div_val0);
	if (periph_clk->dividers > 1)
		set_div_val(priv, periph_clk, 1, div_val1);

	periph_clk_enable(clk, 1);

	return rate;
}

static int armada_37xx_periph_clk_set_parent(struct clk *clk,
					     struct clk *parent)
{
	struct a37xx_periphclk *priv = dev_get_priv(clk->dev);
	const struct clk_periph *periph_clk = &priv->clks[clk->id];
	struct clk check_parent;
	int ret;

	/* We also check if parent is our TBG clock */
	if (clk->id > priv->count || parent->id >= MAX_TBG_PARENTS)
		return -EINVAL;

	if (!periph_clk->can_mux || !periph_clk->can_gate)
		return -ENOTSUPP;

	ret = clk_get_by_index(clk->dev, 0, &check_parent);
	if (ret < 0)
		return ret;

	if (parent->dev != check_parent.dev)
		ret = -EINVAL;

	clk_free(&check_parent);
	if (ret < 0)
		return ret;

	periph_clk_enable(clk, 0);
	set_mux(priv, periph_clk->mux_shift, parent->id);
	periph_clk_enable(clk, 1);

	return 0;
}

#if defined(CONFIG_CMD_CLK) && defined(CONFIG_CLK_ARMADA_3720)
static int armada_37xx_periph_clk_dump(struct udevice *dev)
{
	struct a37xx_periphclk *priv = dev_get_priv(dev);
	const struct clk_periph *clks;
	int i;

	if (!priv)
		return -ENODEV;

	clks = priv->clks;

	for (i = 0; i < priv->count; ++i)
		printf("  %s at %lu Hz\n", clks[i].name,
		       periph_clk_get_rate(priv, i));
	printf("\n");

	return 0;
}

static int clk_dump(const char *name, int (*func)(struct udevice *))
{
	struct udevice *dev;

	if (uclass_get_device_by_name(UCLASS_CLK, name, &dev)) {
		printf("Cannot find device %s\n", name);
		return -ENODEV;
	}

	return func(dev);
}

int armada_37xx_tbg_clk_dump(struct udevice *);

int soc_clk_dump(void)
{
	printf("  xtal at %u000000 Hz\n\n", get_ref_clk());

	if (clk_dump("tbg@13200", armada_37xx_tbg_clk_dump))
		return 1;

	if (clk_dump("nb-periph-clk@13000",
		     armada_37xx_periph_clk_dump))
		return 1;

	if (clk_dump("sb-periph-clk@18000",
		     armada_37xx_periph_clk_dump))
		return 1;

	return 0;
}
#endif

static int armada_37xx_periph_clk_probe(struct udevice *dev)
{
	struct a37xx_periphclk *priv = dev_get_priv(dev);
	const struct clk_periph *clks;
	int ret, i;

	clks = (const struct clk_periph *)dev_get_driver_data(dev);
	if (!clks)
		return -ENODEV;

	priv->reg = dev_read_addr_ptr(dev);
	if (!priv->reg) {
		dev_err(dev, "no io address\n");
		return -ENODEV;
	}

	/* count clk_periph nodes */
	priv->count = 0;
	while (clks[priv->count].name)
		priv->count++;

	priv->clks = clks;

	/* assign parent IDs to nodes which have non-NULL parent_name */
	for (i = 0; i < priv->count; ++i) {
		int j;

		if (!clks[i].parent_name)
			continue;

		/* first try if parent_name is one of TBGs or XTAL */
		for (j = 0; j < MAX_PARENTS; ++j)
			if (!strcmp(clks[i].parent_name,
				    a37xx_periph_parent_names[j].name))
				break;

		if (j < MAX_PARENTS) {
			priv->clk_has_periph_parent[i] = false;
			priv->clk_parent[i] =
				a37xx_periph_parent_names[j].parent;
			continue;
		}

		/* else parent_name should be one of other periph clocks */
		for (j = 0; j < priv->count; ++j) {
			if (!strcmp(clks[i].parent_name, clks[j].name))
				break;
		}

		if (j < priv->count) {
			priv->clk_has_periph_parent[i] = true;
			priv->clk_parent[i] = j;
			continue;
		}

		dev_err(dev, "undefined parent %s\n", clks[i].parent_name);
		return -EINVAL;
	}

	for (i = 0; i < MAX_PARENTS; ++i) {
		struct clk clk;

		if (i == XTAL) {
			priv->parents[i] = get_ref_clk() * 1000000;
			continue;
		}

		ret = clk_get_by_index(dev, i, &clk);
		if (ret) {
			dev_err(dev, "one of parent clocks (%i) missing: %i\n",
				i, ret);
			return -ENODEV;
		}

		priv->parents[i] = clk_get_rate(&clk);
		clk_free(&clk);
	}

	return 0;
}

static const struct clk_ops armada_37xx_periph_clk_ops = {
	.get_rate = armada_37xx_periph_clk_get_rate,
	.set_rate = armada_37xx_periph_clk_set_rate,
	.set_parent = armada_37xx_periph_clk_set_parent,
	.enable = armada_37xx_periph_clk_enable,
	.disable = armada_37xx_periph_clk_disable,
};

static const struct udevice_id armada_37xx_periph_clk_ids[] = {
	{
		.compatible = "marvell,armada-3700-periph-clock-nb",
		.data = (ulong)clks_nb,
	},
	{
		.compatible = "marvell,armada-3700-periph-clock-sb",
		.data = (ulong)clks_sb,
	},
	{}
};

U_BOOT_DRIVER(armada_37xx_periph_clk) = {
	.name		= "armada_37xx_periph_clk",
	.id		= UCLASS_CLK,
	.of_match	= armada_37xx_periph_clk_ids,
	.ops		= &armada_37xx_periph_clk_ops,
	.priv_auto_alloc_size = sizeof(struct a37xx_periphclk),
	.probe		= armada_37xx_periph_clk_probe,
};
