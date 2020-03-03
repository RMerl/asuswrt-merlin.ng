/*
 * Copyright (C) 2011 Sascha Hauer, Pengutronix <s.hauer@pengutronix.de>
 * Copyright (C) 2011 Richard Zhao, Linaro <richard.zhao@linaro.org>
 * Copyright (C) 2011-2012 Mike Turquette, Linaro Ltd <mturquette@linaro.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Adjustable divider clock implementation
 */

#include <linux/clk-provider.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/log2.h>

/*
 * DOC: basic adjustable divider clock that cannot gate
 *
 * Traits of this clock:
 * prepare - clk_prepare only ensures that parents are prepared
 * enable - clk_enable only ensures that parents are enabled
 * rate - rate is adjustable.  clk->rate = DIV_ROUND_UP(parent->rate / divisor)
 * parent - fixed parent.  No clk_set_parent support
 */

#define to_clk_divider(_hw) container_of(_hw, struct clk_divider, hw)

#define div_mask(width)	((1 << (width)) - 1)

static unsigned int _get_table_maxdiv(const struct clk_div_table *table)
{
	unsigned int maxdiv = 0;
	const struct clk_div_table *clkt;

	for (clkt = table; clkt->div; clkt++)
		if (clkt->div > maxdiv)
			maxdiv = clkt->div;
	return maxdiv;
}

static unsigned int _get_table_mindiv(const struct clk_div_table *table)
{
	unsigned int mindiv = UINT_MAX;
	const struct clk_div_table *clkt;

	for (clkt = table; clkt->div; clkt++)
		if (clkt->div < mindiv)
			mindiv = clkt->div;
	return mindiv;
}

static unsigned int _get_maxdiv(const struct clk_div_table *table, u8 width,
				unsigned long flags)
{
	if (flags & CLK_DIVIDER_ONE_BASED)
		return div_mask(width);
	if (flags & CLK_DIVIDER_POWER_OF_TWO)
		return 1 << div_mask(width);
	if (table)
		return _get_table_maxdiv(table);
	return div_mask(width) + 1;
}

static unsigned int _get_table_div(const struct clk_div_table *table,
							unsigned int val)
{
	const struct clk_div_table *clkt;

	for (clkt = table; clkt->div; clkt++)
		if (clkt->val == val)
			return clkt->div;
	return 0;
}

static unsigned int _get_div(const struct clk_div_table *table,
			     unsigned int val, unsigned long flags)
{
	if (flags & CLK_DIVIDER_ONE_BASED)
		return val;
	if (flags & CLK_DIVIDER_POWER_OF_TWO)
		return 1 << val;
	if (table)
		return _get_table_div(table, val);
	return val + 1;
}

static unsigned int _get_table_val(const struct clk_div_table *table,
							unsigned int div)
{
	const struct clk_div_table *clkt;

	for (clkt = table; clkt->div; clkt++)
		if (clkt->div == div)
			return clkt->val;
	return 0;
}

static unsigned int _get_val(const struct clk_div_table *table,
			     unsigned int div, unsigned long flags)
{
	if (flags & CLK_DIVIDER_ONE_BASED)
		return div;
	if (flags & CLK_DIVIDER_POWER_OF_TWO)
		return __ffs(div);
	if (table)
		return  _get_table_val(table, div);
	return div - 1;
}

unsigned long divider_recalc_rate(struct clk_hw *hw, unsigned long parent_rate,
				  unsigned int val,
				  const struct clk_div_table *table,
				  unsigned long flags)
{
	unsigned int div;

	div = _get_div(table, val, flags);
	if (!div) {
		WARN(!(flags & CLK_DIVIDER_ALLOW_ZERO),
			"%s: Zero divisor and CLK_DIVIDER_ALLOW_ZERO not set\n",
			__clk_get_name(hw->clk));
		return parent_rate;
	}

	return DIV_ROUND_UP(parent_rate, div);
}
EXPORT_SYMBOL_GPL(divider_recalc_rate);

static unsigned long clk_divider_recalc_rate(struct clk_hw *hw,
		unsigned long parent_rate)
{
	struct clk_divider *divider = to_clk_divider(hw);
	unsigned int val;

	val = clk_readl(divider->reg) >> divider->shift;
	val &= div_mask(divider->width);

	return divider_recalc_rate(hw, parent_rate, val, divider->table,
				   divider->flags);
}

static bool _is_valid_table_div(const struct clk_div_table *table,
							 unsigned int div)
{
	const struct clk_div_table *clkt;

	for (clkt = table; clkt->div; clkt++)
		if (clkt->div == div)
			return true;
	return false;
}

static bool _is_valid_div(const struct clk_div_table *table, unsigned int div,
			  unsigned long flags)
{
	if (flags & CLK_DIVIDER_POWER_OF_TWO)
		return is_power_of_2(div);
	if (table)
		return _is_valid_table_div(table, div);
	return true;
}

static int _round_up_table(const struct clk_div_table *table, int div)
{
	const struct clk_div_table *clkt;
	int up = INT_MAX;

	for (clkt = table; clkt->div; clkt++) {
		if (clkt->div == div)
			return clkt->div;
		else if (clkt->div < div)
			continue;

		if ((clkt->div - div) < (up - div))
			up = clkt->div;
	}

	return up;
}

static int _round_down_table(const struct clk_div_table *table, int div)
{
	const struct clk_div_table *clkt;
	int down = _get_table_mindiv(table);

	for (clkt = table; clkt->div; clkt++) {
		if (clkt->div == div)
			return clkt->div;
		else if (clkt->div > div)
			continue;

		if ((div - clkt->div) < (div - down))
			down = clkt->div;
	}

	return down;
}

static int _div_round_up(const struct clk_div_table *table,
			 unsigned long parent_rate, unsigned long rate,
			 unsigned long flags)
{
	int div = DIV_ROUND_UP(parent_rate, rate);

	if (flags & CLK_DIVIDER_POWER_OF_TWO)
		div = __roundup_pow_of_two(div);
	if (table)
		div = _round_up_table(table, div);

	return div;
}

static int _div_round_closest(const struct clk_div_table *table,
			      unsigned long parent_rate, unsigned long rate,
			      unsigned long flags)
{
	int up, down;
	unsigned long up_rate, down_rate;

	up = DIV_ROUND_UP(parent_rate, rate);
	down = parent_rate / rate;

	if (flags & CLK_DIVIDER_POWER_OF_TWO) {
		up = __roundup_pow_of_two(up);
		down = __rounddown_pow_of_two(down);
	} else if (table) {
		up = _round_up_table(table, up);
		down = _round_down_table(table, down);
	}

	up_rate = DIV_ROUND_UP(parent_rate, up);
	down_rate = DIV_ROUND_UP(parent_rate, down);

	return (rate - up_rate) <= (down_rate - rate) ? up : down;
}

static int _div_round(const struct clk_div_table *table,
		      unsigned long parent_rate, unsigned long rate,
		      unsigned long flags)
{
	if (flags & CLK_DIVIDER_ROUND_CLOSEST)
		return _div_round_closest(table, parent_rate, rate, flags);

	return _div_round_up(table, parent_rate, rate, flags);
}

static bool _is_best_div(unsigned long rate, unsigned long now,
			 unsigned long best, unsigned long flags)
{
	if (flags & CLK_DIVIDER_ROUND_CLOSEST)
		return abs(rate - now) < abs(rate - best);

	return now <= rate && now > best;
}

static int _next_div(const struct clk_div_table *table, int div,
		     unsigned long flags)
{
	div++;

	if (flags & CLK_DIVIDER_POWER_OF_TWO)
		return __roundup_pow_of_two(div);
	if (table)
		return _round_up_table(table, div);

	return div;
}

static int clk_divider_bestdiv(struct clk_hw *hw, unsigned long rate,
			       unsigned long *best_parent_rate,
			       const struct clk_div_table *table, u8 width,
			       unsigned long flags)
{
	int i, bestdiv = 0;
	unsigned long parent_rate, best = 0, now, maxdiv;
	unsigned long parent_rate_saved = *best_parent_rate;

	if (!rate)
		rate = 1;

	maxdiv = _get_maxdiv(table, width, flags);

	if (!(__clk_get_flags(hw->clk) & CLK_SET_RATE_PARENT)) {
		parent_rate = *best_parent_rate;
		bestdiv = _div_round(table, parent_rate, rate, flags);
		bestdiv = bestdiv == 0 ? 1 : bestdiv;
		bestdiv = bestdiv > maxdiv ? maxdiv : bestdiv;
		return bestdiv;
	}

	/*
	 * The maximum divider we can use without overflowing
	 * unsigned long in rate * i below
	 */
	maxdiv = min(ULONG_MAX / rate, maxdiv);

	for (i = 1; i <= maxdiv; i = _next_div(table, i, flags)) {
		if (!_is_valid_div(table, i, flags))
			continue;
		if (rate * i == parent_rate_saved) {
			/*
			 * It's the most ideal case if the requested rate can be
			 * divided from parent clock without needing to change
			 * parent rate, so return the divider immediately.
			 */
			*best_parent_rate = parent_rate_saved;
			return i;
		}
		parent_rate = __clk_round_rate(__clk_get_parent(hw->clk),
					       rate * i);
		now = DIV_ROUND_UP(parent_rate, i);
		if (_is_best_div(rate, now, best, flags)) {
			bestdiv = i;
			best = now;
			*best_parent_rate = parent_rate;
		}
	}

	if (!bestdiv) {
		bestdiv = _get_maxdiv(table, width, flags);
		*best_parent_rate = __clk_round_rate(__clk_get_parent(hw->clk), 1);
	}

	return bestdiv;
}

long divider_round_rate(struct clk_hw *hw, unsigned long rate,
			unsigned long *prate, const struct clk_div_table *table,
			u8 width, unsigned long flags)
{
	int div;

	div = clk_divider_bestdiv(hw, rate, prate, table, width, flags);

	return DIV_ROUND_UP(*prate, div);
}
EXPORT_SYMBOL_GPL(divider_round_rate);

static long clk_divider_round_rate(struct clk_hw *hw, unsigned long rate,
				unsigned long *prate)
{
	struct clk_divider *divider = to_clk_divider(hw);
	int bestdiv;

	/* if read only, just return current value */
	if (divider->flags & CLK_DIVIDER_READ_ONLY) {
		bestdiv = readl(divider->reg) >> divider->shift;
		bestdiv &= div_mask(divider->width);
		bestdiv = _get_div(divider->table, bestdiv, divider->flags);
		return DIV_ROUND_UP(*prate, bestdiv);
	}

	return divider_round_rate(hw, rate, prate, divider->table,
				  divider->width, divider->flags);
}

int divider_get_val(unsigned long rate, unsigned long parent_rate,
		    const struct clk_div_table *table, u8 width,
		    unsigned long flags)
{
	unsigned int div, value;

	div = DIV_ROUND_UP(parent_rate, rate);

	if (!_is_valid_div(table, div, flags))
		return -EINVAL;

	value = _get_val(table, div, flags);

	return min_t(unsigned int, value, div_mask(width));
}
EXPORT_SYMBOL_GPL(divider_get_val);

static int clk_divider_set_rate(struct clk_hw *hw, unsigned long rate,
				unsigned long parent_rate)
{
	struct clk_divider *divider = to_clk_divider(hw);
	unsigned int value;
	unsigned long flags = 0;
	u32 val;

	value = divider_get_val(rate, parent_rate, divider->table,
				divider->width, divider->flags);

	if (divider->lock)
		spin_lock_irqsave(divider->lock, flags);

	if (divider->flags & CLK_DIVIDER_HIWORD_MASK) {
		val = div_mask(divider->width) << (divider->shift + 16);
	} else {
		val = clk_readl(divider->reg);
		val &= ~(div_mask(divider->width) << divider->shift);
	}
	val |= value << divider->shift;
	clk_writel(val, divider->reg);

	if (divider->lock)
		spin_unlock_irqrestore(divider->lock, flags);

	return 0;
}

const struct clk_ops clk_divider_ops = {
	.recalc_rate = clk_divider_recalc_rate,
	.round_rate = clk_divider_round_rate,
	.set_rate = clk_divider_set_rate,
};
EXPORT_SYMBOL_GPL(clk_divider_ops);

static struct clk *_register_divider(struct device *dev, const char *name,
		const char *parent_name, unsigned long flags,
		void __iomem *reg, u8 shift, u8 width,
		u8 clk_divider_flags, const struct clk_div_table *table,
		spinlock_t *lock)
{
	struct clk_divider *div;
	struct clk *clk;
	struct clk_init_data init;

	if (clk_divider_flags & CLK_DIVIDER_HIWORD_MASK) {
		if (width + shift > 16) {
			pr_warn("divider value exceeds LOWORD field\n");
			return ERR_PTR(-EINVAL);
		}
	}

	/* allocate the divider */
	div = kzalloc(sizeof(struct clk_divider), GFP_KERNEL);
	if (!div) {
		pr_err("%s: could not allocate divider clk\n", __func__);
		return ERR_PTR(-ENOMEM);
	}

	init.name = name;
	init.ops = &clk_divider_ops;
	init.flags = flags | CLK_IS_BASIC;
	init.parent_names = (parent_name ? &parent_name: NULL);
	init.num_parents = (parent_name ? 1 : 0);

	/* struct clk_divider assignments */
	div->reg = reg;
	div->shift = shift;
	div->width = width;
	div->flags = clk_divider_flags;
	div->lock = lock;
	div->hw.init = &init;
	div->table = table;

	/* register the clock */
	clk = clk_register(dev, &div->hw);

	if (IS_ERR(clk))
		kfree(div);

	return clk;
}

/**
 * clk_register_divider - register a divider clock with the clock framework
 * @dev: device registering this clock
 * @name: name of this clock
 * @parent_name: name of clock's parent
 * @flags: framework-specific flags
 * @reg: register address to adjust divider
 * @shift: number of bits to shift the bitfield
 * @width: width of the bitfield
 * @clk_divider_flags: divider-specific flags for this clock
 * @lock: shared register lock for this clock
 */
struct clk *clk_register_divider(struct device *dev, const char *name,
		const char *parent_name, unsigned long flags,
		void __iomem *reg, u8 shift, u8 width,
		u8 clk_divider_flags, spinlock_t *lock)
{
	return _register_divider(dev, name, parent_name, flags, reg, shift,
			width, clk_divider_flags, NULL, lock);
}
EXPORT_SYMBOL_GPL(clk_register_divider);

/**
 * clk_register_divider_table - register a table based divider clock with
 * the clock framework
 * @dev: device registering this clock
 * @name: name of this clock
 * @parent_name: name of clock's parent
 * @flags: framework-specific flags
 * @reg: register address to adjust divider
 * @shift: number of bits to shift the bitfield
 * @width: width of the bitfield
 * @clk_divider_flags: divider-specific flags for this clock
 * @table: array of divider/value pairs ending with a div set to 0
 * @lock: shared register lock for this clock
 */
struct clk *clk_register_divider_table(struct device *dev, const char *name,
		const char *parent_name, unsigned long flags,
		void __iomem *reg, u8 shift, u8 width,
		u8 clk_divider_flags, const struct clk_div_table *table,
		spinlock_t *lock)
{
	return _register_divider(dev, name, parent_name, flags, reg, shift,
			width, clk_divider_flags, table, lock);
}
EXPORT_SYMBOL_GPL(clk_register_divider_table);

void clk_unregister_divider(struct clk *clk)
{
	struct clk_divider *div;
	struct clk_hw *hw;

	hw = __clk_get_hw(clk);
	if (!hw)
		return;

	div = to_clk_divider(hw);

	clk_unregister(clk);
	kfree(div);
}
EXPORT_SYMBOL_GPL(clk_unregister_divider);
