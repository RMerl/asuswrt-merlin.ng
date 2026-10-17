/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 *
 * Copyright (c) 2010-2011 Jeremy Kerr <jeremy.kerr@canonical.com>
 * Copyright (C) 2011-2012 Linaro Ltd <mturquette@linaro.org>
 */
#ifndef __LINUX_CLK_PROVIDER_H
#define __LINUX_CLK_PROVIDER_H

#include <linux/bitops.h>
#include <linux/err.h>
#include <clk-uclass.h>

struct udevice;

static inline void clk_dm(ulong id, struct clk *clk)
{
	if (!IS_ERR(clk))
		clk->id = id;
}

/*
 * flags used across common struct clk.  these flags should only affect the
 * top-level framework.  custom flags for dealing with hardware specifics
 * belong in struct clk_foo
 *
 * Please update clk_flags[] in drivers/clk/clk.c when making changes here!
 */
#define CLK_SET_RATE_GATE	BIT(0) /* must be gated across rate change */
#define CLK_SET_PARENT_GATE	BIT(1) /* must be gated across re-parent */
#define CLK_SET_RATE_PARENT	BIT(2) /* propagate rate change up one level */
#define CLK_IGNORE_UNUSED	BIT(3) /* do not gate even if unused */
				/* unused */
#define CLK_IS_BASIC		BIT(5) /* Basic clk, can't do a to_clk_foo() */
#define CLK_GET_RATE_NOCACHE	BIT(6) /* do not use the cached clk rate */
#define CLK_SET_RATE_NO_REPARENT BIT(7) /* don't re-parent on rate change */
#define CLK_GET_ACCURACY_NOCACHE BIT(8) /* do not use the cached clk accuracy */
#define CLK_RECALC_NEW_RATES	BIT(9) /* recalc rates after notifications */
#define CLK_SET_RATE_UNGATE	BIT(10) /* clock needs to run to set rate */
#define CLK_IS_CRITICAL		BIT(11) /* do not gate, ever */
/* parents need enable during gate/ungate, set rate and re-parent */
#define CLK_OPS_PARENT_ENABLE	BIT(12)
/* duty cycle call may be forwarded to the parent clock */
#define CLK_DUTY_CYCLE_PARENT	BIT(13)

#define CLK_MUX_INDEX_ONE		BIT(0)
#define CLK_MUX_INDEX_BIT		BIT(1)
#define CLK_MUX_HIWORD_MASK		BIT(2)
#define CLK_MUX_READ_ONLY		BIT(3) /* mux can't be changed */
#define CLK_MUX_ROUND_CLOSEST		BIT(4)

struct clk_mux {
	struct clk	clk;
	void __iomem	*reg;
	u32		*table;
	u32		mask;
	u8		shift;
	u8		flags;

	/*
	 * Fields from struct clk_init_data - this struct has been
	 * omitted to avoid too deep level of CCF for bootloader
	 */
	const char	* const *parent_names;
	u8		num_parents;
#if IS_ENABLED(CONFIG_SANDBOX_CLK_CCF)
	u32             io_mux_val;
#endif

};

#define to_clk_mux(_clk) container_of(_clk, struct clk_mux, clk)
extern const struct clk_ops clk_mux_ops;
u8 clk_mux_get_parent(struct clk *clk);

/**
 * clk_mux_index_to_val() - Convert the parent index to the register value
 *
 * It returns the value to write in the hardware register to output the selected
 * input clock parent.
 *
 * @table: array of register values corresponding to the parent index (optional)
 * @flags: hardware-specific flags
 * @index: parent clock index
 * Return: the register value
 */
unsigned int clk_mux_index_to_val(u32 *table, unsigned int flags, u8 index);

struct clk_gate {
	struct clk	clk;
	void __iomem	*reg;
	u8		bit_idx;
	u8		flags;
#if IS_ENABLED(CONFIG_SANDBOX_CLK_CCF)
	u32		io_gate_val;
#endif
};

#define to_clk_gate(_clk) container_of(_clk, struct clk_gate, clk)

#define CLK_GATE_SET_TO_DISABLE		BIT(0)
#define CLK_GATE_HIWORD_MASK		BIT(1)

extern const struct clk_ops clk_gate_ops;
struct clk *clk_register_gate(struct device *dev, const char *name,
			      const char *parent_name, unsigned long flags,
			      void __iomem *reg, u8 bit_idx,
			      u8 clk_gate_flags, spinlock_t *lock);

struct clk_div_table {
	unsigned int	val;
	unsigned int	div;
};

struct clk_divider {
	struct clk	clk;
	void __iomem	*reg;
	u8		shift;
	u8		width;
	u8		flags;
	const struct clk_div_table	*table;
#if IS_ENABLED(CONFIG_SANDBOX_CLK_CCF)
	u32             io_divider_val;
#endif
};

#define clk_div_mask(width)	((1 << (width)) - 1)
#define to_clk_divider(_clk) container_of(_clk, struct clk_divider, clk)

#define CLK_DIVIDER_ONE_BASED		BIT(0)
#define CLK_DIVIDER_POWER_OF_TWO	BIT(1)
#define CLK_DIVIDER_ALLOW_ZERO		BIT(2)
#define CLK_DIVIDER_HIWORD_MASK		BIT(3)
#define CLK_DIVIDER_ROUND_CLOSEST	BIT(4)
#define CLK_DIVIDER_READ_ONLY		BIT(5)
#define CLK_DIVIDER_MAX_AT_ZERO		BIT(6)
extern const struct clk_ops clk_divider_ops;

/**
 * clk_divider_get_table_div() - convert the register value to the divider
 *
 * @table:  array of register values corresponding to valid dividers
 * @val: value to convert
 * Return: the divider
 */
unsigned int clk_divider_get_table_div(const struct clk_div_table *table,
				       unsigned int val);

/**
 * clk_divider_get_table_val() - convert the divider to the register value
 *
 * It returns the value to write in the hardware register to divide the input
 * clock rate by @div.
 *
 * @table: array of register values corresponding to valid dividers
 * @div: requested divider
 * Return: the register value
 */
unsigned int clk_divider_get_table_val(const struct clk_div_table *table,
				       unsigned int div);

/**
 * clk_divider_is_valid_div() - check if the divider is valid
 *
 * @table: array of valid dividers (optional)
 * @div: divider to check
 * @flags: hardware-specific flags
 * Return: true if the divider is valid, false otherwise
 */
bool clk_divider_is_valid_div(const struct clk_div_table *table,
			      unsigned int div, unsigned long flags);

/**
 * clk_divider_is_valid_table_div - check if the divider is in the @table array
 *
 * @table: array of valid dividers
 * @div: divider to check
 * Return: true if the divider is found in the @table array, false otherwise
 */
bool clk_divider_is_valid_table_div(const struct clk_div_table *table,
				    unsigned int div);
unsigned long divider_recalc_rate(struct clk *hw, unsigned long parent_rate,
				  unsigned int val,
				  const struct clk_div_table *table,
				  unsigned long flags, unsigned long width);

struct clk_fixed_factor {
	struct clk	clk;
	unsigned int	mult;
	unsigned int	div;
};

extern const struct clk_ops clk_fixed_rate_ops;

#define to_clk_fixed_factor(_clk) container_of(_clk, struct clk_fixed_factor,\
					       clk)

struct clk_fixed_rate {
	struct clk clk;
	unsigned long fixed_rate;
};

#define to_clk_fixed_rate(dev)	((struct clk_fixed_rate *)dev_get_plat(dev))

void clk_fixed_rate_ofdata_to_plat_(struct udevice *dev,
				    struct clk_fixed_rate *plat);

struct clk_composite {
	struct clk	clk;
	struct clk_ops	ops;

	struct clk	*mux;
	struct clk	*rate;
	struct clk	*gate;

	const struct clk_ops	*mux_ops;
	const struct clk_ops	*rate_ops;
	const struct clk_ops	*gate_ops;
};

#define to_clk_composite(_clk) container_of(_clk, struct clk_composite, clk)

struct clk *clk_register_composite(struct device *dev, const char *name,
		const char * const *parent_names, int num_parents,
		struct clk *mux_clk, const struct clk_ops *mux_ops,
		struct clk *rate_clk, const struct clk_ops *rate_ops,
		struct clk *gate_clk, const struct clk_ops *gate_ops,
		unsigned long flags);

int clk_register(struct clk *clk, const char *drv_name, const char *name,
		 const char *parent_name);

struct clk *clk_register_fixed_factor(struct device *dev, const char *name,
		const char *parent_name, unsigned long flags,
		unsigned int mult, unsigned int div);

struct clk *clk_register_divider(struct device *dev, const char *name,
		const char *parent_name, unsigned long flags,
		void __iomem *reg, u8 shift, u8 width,
		u8 clk_divider_flags);

struct clk *clk_register_mux(struct device *dev, const char *name,
		const char * const *parent_names, u8 num_parents,
		unsigned long flags,
		void __iomem *reg, u8 shift, u8 width,
		u8 clk_mux_flags);

struct clk *clk_register_fixed_rate(struct device *dev, const char *name,
				    ulong rate);

const char *clk_hw_get_name(const struct clk *hw);
ulong clk_generic_get_rate(struct clk *clk);

struct clk *dev_get_clk_ptr(struct udevice *dev);

ulong ccf_clk_get_rate(struct clk *clk);
ulong ccf_clk_set_rate(struct clk *clk, unsigned long rate);
int ccf_clk_set_parent(struct clk *clk, struct clk *parent);
int ccf_clk_enable(struct clk *clk);
int ccf_clk_disable(struct clk *clk);
extern const struct clk_ops ccf_clk_ops;

#endif /* __LINUX_CLK_PROVIDER_H */
