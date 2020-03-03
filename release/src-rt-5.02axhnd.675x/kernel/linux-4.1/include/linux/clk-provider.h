/*
 *  linux/include/linux/clk-provider.h
 *
 *  Copyright (c) 2010-2011 Jeremy Kerr <jeremy.kerr@canonical.com>
 *  Copyright (C) 2011-2012 Linaro Ltd <mturquette@linaro.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __LINUX_CLK_PROVIDER_H
#define __LINUX_CLK_PROVIDER_H

#include <linux/clk.h>
#include <linux/io.h>
#include <linux/of.h>

#ifdef CONFIG_COMMON_CLK

/*
 * flags used across common struct clk.  these flags should only affect the
 * top-level framework.  custom flags for dealing with hardware specifics
 * belong in struct clk_foo
 */
#define CLK_SET_RATE_GATE	BIT(0) /* must be gated across rate change */
#define CLK_SET_PARENT_GATE	BIT(1) /* must be gated across re-parent */
#define CLK_SET_RATE_PARENT	BIT(2) /* propagate rate change up one level */
#define CLK_IGNORE_UNUSED	BIT(3) /* do not gate even if unused */
#define CLK_IS_ROOT		BIT(4) /* root clk, has no parent */
#define CLK_IS_BASIC		BIT(5) /* Basic clk, can't do a to_clk_foo() */
#define CLK_GET_RATE_NOCACHE	BIT(6) /* do not use the cached clk rate */
#define CLK_SET_RATE_NO_REPARENT BIT(7) /* don't re-parent on rate change */
#define CLK_GET_ACCURACY_NOCACHE BIT(8) /* do not use the cached clk accuracy */

struct clk_hw;
struct clk_core;
struct dentry;

/**
 * struct clk_ops -  Callback operations for hardware clocks; these are to
 * be provided by the clock implementation, and will be called by drivers
 * through the clk_* api.
 *
 * @prepare:	Prepare the clock for enabling. This must not return until
 *		the clock is fully prepared, and it's safe to call clk_enable.
 *		This callback is intended to allow clock implementations to
 *		do any initialisation that may sleep. Called with
 *		prepare_lock held.
 *
 * @unprepare:	Release the clock from its prepared state. This will typically
 *		undo any work done in the @prepare callback. Called with
 *		prepare_lock held.
 *
 * @is_prepared: Queries the hardware to determine if the clock is prepared.
 *		This function is allowed to sleep. Optional, if this op is not
 *		set then the prepare count will be used.
 *
 * @unprepare_unused: Unprepare the clock atomically.  Only called from
 *		clk_disable_unused for prepare clocks with special needs.
 *		Called with prepare mutex held. This function may sleep.
 *
 * @enable:	Enable the clock atomically. This must not return until the
 *		clock is generating a valid clock signal, usable by consumer
 *		devices. Called with enable_lock held. This function must not
 *		sleep.
 *
 * @disable:	Disable the clock atomically. Called with enable_lock held.
 *		This function must not sleep.
 *
 * @is_enabled:	Queries the hardware to determine if the clock is enabled.
 *		This function must not sleep. Optional, if this op is not
 *		set then the enable count will be used.
 *
 * @disable_unused: Disable the clock atomically.  Only called from
 *		clk_disable_unused for gate clocks with special needs.
 *		Called with enable_lock held.  This function must not
 *		sleep.
 *
 * @recalc_rate	Recalculate the rate of this clock, by querying hardware. The
 *		parent rate is an input parameter.  It is up to the caller to
 *		ensure that the prepare_mutex is held across this call.
 *		Returns the calculated rate.  Optional, but recommended - if
 *		this op is not set then clock rate will be initialized to 0.
 *
 * @round_rate:	Given a target rate as input, returns the closest rate actually
 *		supported by the clock. The parent rate is an input/output
 *		parameter.
 *
 * @determine_rate: Given a target rate as input, returns the closest rate
 *		actually supported by the clock, and optionally the parent clock
 *		that should be used to provide the clock rate.
 *
 * @set_parent:	Change the input source of this clock; for clocks with multiple
 *		possible parents specify a new parent by passing in the index
 *		as a u8 corresponding to the parent in either the .parent_names
 *		or .parents arrays.  This function in affect translates an
 *		array index into the value programmed into the hardware.
 *		Returns 0 on success, -EERROR otherwise.
 *
 * @get_parent:	Queries the hardware to determine the parent of a clock.  The
 *		return value is a u8 which specifies the index corresponding to
 *		the parent clock.  This index can be applied to either the
 *		.parent_names or .parents arrays.  In short, this function
 *		translates the parent value read from hardware into an array
 *		index.  Currently only called when the clock is initialized by
 *		__clk_init.  This callback is mandatory for clocks with
 *		multiple parents.  It is optional (and unnecessary) for clocks
 *		with 0 or 1 parents.
 *
 * @set_rate:	Change the rate of this clock. The requested rate is specified
 *		by the second argument, which should typically be the return
 *		of .round_rate call.  The third argument gives the parent rate
 *		which is likely helpful for most .set_rate implementation.
 *		Returns 0 on success, -EERROR otherwise.
 *
 * @set_rate_and_parent: Change the rate and the parent of this clock. The
 *		requested rate is specified by the second argument, which
 *		should typically be the return of .round_rate call.  The
 *		third argument gives the parent rate which is likely helpful
 *		for most .set_rate_and_parent implementation. The fourth
 *		argument gives the parent index. This callback is optional (and
 *		unnecessary) for clocks with 0 or 1 parents as well as
 *		for clocks that can tolerate switching the rate and the parent
 *		separately via calls to .set_parent and .set_rate.
 *		Returns 0 on success, -EERROR otherwise.
 *
 * @recalc_accuracy: Recalculate the accuracy of this clock. The clock accuracy
 *		is expressed in ppb (parts per billion). The parent accuracy is
 *		an input parameter.
 *		Returns the calculated accuracy.  Optional - if	this op is not
 *		set then clock accuracy will be initialized to parent accuracy
 *		or 0 (perfect clock) if clock has no parent.
 *
 * @get_phase:	Queries the hardware to get the current phase of a clock.
 *		Returned values are 0-359 degrees on success, negative
 *		error codes on failure.
 *
 * @set_phase:	Shift the phase this clock signal in degrees specified
 *		by the second argument. Valid values for degrees are
 *		0-359. Return 0 on success, otherwise -EERROR.
 *
 * @init:	Perform platform-specific initialization magic.
 *		This is not not used by any of the basic clock types.
 *		Please consider other ways of solving initialization problems
 *		before using this callback, as its use is discouraged.
 *
 * @debug_init:	Set up type-specific debugfs entries for this clock.  This
 *		is called once, after the debugfs directory entry for this
 *		clock has been created.  The dentry pointer representing that
 *		directory is provided as an argument.  Called with
 *		prepare_lock held.  Returns 0 on success, -EERROR otherwise.
 *
 *
 * The clk_enable/clk_disable and clk_prepare/clk_unprepare pairs allow
 * implementations to split any work between atomic (enable) and sleepable
 * (prepare) contexts.  If enabling a clock requires code that might sleep,
 * this must be done in clk_prepare.  Clock enable code that will never be
 * called in a sleepable context may be implemented in clk_enable.
 *
 * Typically, drivers will call clk_prepare when a clock may be needed later
 * (eg. when a device is opened), and clk_enable when the clock is actually
 * required (eg. from an interrupt). Note that clk_prepare MUST have been
 * called before clk_enable.
 */
struct clk_ops {
	int		(*prepare)(struct clk_hw *hw);
	void		(*unprepare)(struct clk_hw *hw);
	int		(*is_prepared)(struct clk_hw *hw);
	void		(*unprepare_unused)(struct clk_hw *hw);
	int		(*enable)(struct clk_hw *hw);
	void		(*disable)(struct clk_hw *hw);
	int		(*is_enabled)(struct clk_hw *hw);
	void		(*disable_unused)(struct clk_hw *hw);
	unsigned long	(*recalc_rate)(struct clk_hw *hw,
					unsigned long parent_rate);
	long		(*round_rate)(struct clk_hw *hw, unsigned long rate,
					unsigned long *parent_rate);
	long		(*determine_rate)(struct clk_hw *hw,
					  unsigned long rate,
					  unsigned long min_rate,
					  unsigned long max_rate,
					  unsigned long *best_parent_rate,
					  struct clk_hw **best_parent_hw);
	int		(*set_parent)(struct clk_hw *hw, u8 index);
	u8		(*get_parent)(struct clk_hw *hw);
	int		(*set_rate)(struct clk_hw *hw, unsigned long rate,
				    unsigned long parent_rate);
	int		(*set_rate_and_parent)(struct clk_hw *hw,
				    unsigned long rate,
				    unsigned long parent_rate, u8 index);
	unsigned long	(*recalc_accuracy)(struct clk_hw *hw,
					   unsigned long parent_accuracy);
	int		(*get_phase)(struct clk_hw *hw);
	int		(*set_phase)(struct clk_hw *hw, int degrees);
	void		(*init)(struct clk_hw *hw);
	int		(*debug_init)(struct clk_hw *hw, struct dentry *dentry);
};

/**
 * struct clk_init_data - holds init data that's common to all clocks and is
 * shared between the clock provider and the common clock framework.
 *
 * @name: clock name
 * @ops: operations this clock supports
 * @parent_names: array of string names for all possible parents
 * @num_parents: number of possible parents
 * @flags: framework-level hints and quirks
 */
struct clk_init_data {
	const char		*name;
	const struct clk_ops	*ops;
	const char		**parent_names;
	u8			num_parents;
	unsigned long		flags;
};

/**
 * struct clk_hw - handle for traversing from a struct clk to its corresponding
 * hardware-specific structure.  struct clk_hw should be declared within struct
 * clk_foo and then referenced by the struct clk instance that uses struct
 * clk_foo's clk_ops
 *
 * @core: pointer to the struct clk_core instance that points back to this
 * struct clk_hw instance
 *
 * @clk: pointer to the per-user struct clk instance that can be used to call
 * into the clk API
 *
 * @init: pointer to struct clk_init_data that contains the init data shared
 * with the common clock framework.
 */
struct clk_hw {
	struct clk_core *core;
	struct clk *clk;
	const struct clk_init_data *init;
};

/*
 * DOC: Basic clock implementations common to many platforms
 *
 * Each basic clock hardware type is comprised of a structure describing the
 * clock hardware, implementations of the relevant callbacks in struct clk_ops,
 * unique flags for that hardware type, a registration function and an
 * alternative macro for static initialization
 */

/**
 * struct clk_fixed_rate - fixed-rate clock
 * @hw:		handle between common and hardware-specific interfaces
 * @fixed_rate:	constant frequency of clock
 */
struct clk_fixed_rate {
	struct		clk_hw hw;
	unsigned long	fixed_rate;
	unsigned long	fixed_accuracy;
	u8		flags;
};

extern const struct clk_ops clk_fixed_rate_ops;
struct clk *clk_register_fixed_rate(struct device *dev, const char *name,
		const char *parent_name, unsigned long flags,
		unsigned long fixed_rate);
struct clk *clk_register_fixed_rate_with_accuracy(struct device *dev,
		const char *name, const char *parent_name, unsigned long flags,
		unsigned long fixed_rate, unsigned long fixed_accuracy);

void of_fixed_clk_setup(struct device_node *np);

/**
 * struct clk_gate - gating clock
 *
 * @hw:		handle between common and hardware-specific interfaces
 * @reg:	register controlling gate
 * @bit_idx:	single bit controlling gate
 * @flags:	hardware-specific flags
 * @lock:	register lock
 *
 * Clock which can gate its output.  Implements .enable & .disable
 *
 * Flags:
 * CLK_GATE_SET_TO_DISABLE - by default this clock sets the bit at bit_idx to
 *	enable the clock.  Setting this flag does the opposite: setting the bit
 *	disable the clock and clearing it enables the clock
 * CLK_GATE_HIWORD_MASK - The gate settings are only in lower 16-bit
 *	of this register, and mask of gate bits are in higher 16-bit of this
 *	register.  While setting the gate bits, higher 16-bit should also be
 *	updated to indicate changing gate bits.
 */
struct clk_gate {
	struct clk_hw hw;
	void __iomem	*reg;
	u8		bit_idx;
	u8		flags;
	spinlock_t	*lock;
};

#define CLK_GATE_SET_TO_DISABLE		BIT(0)
#define CLK_GATE_HIWORD_MASK		BIT(1)

extern const struct clk_ops clk_gate_ops;
struct clk *clk_register_gate(struct device *dev, const char *name,
		const char *parent_name, unsigned long flags,
		void __iomem *reg, u8 bit_idx,
		u8 clk_gate_flags, spinlock_t *lock);
void clk_unregister_gate(struct clk *clk);

struct clk_div_table {
	unsigned int	val;
	unsigned int	div;
};

/**
 * struct clk_divider - adjustable divider clock
 *
 * @hw:		handle between common and hardware-specific interfaces
 * @reg:	register containing the divider
 * @shift:	shift to the divider bit field
 * @width:	width of the divider bit field
 * @table:	array of value/divider pairs, last entry should have div = 0
 * @lock:	register lock
 *
 * Clock with an adjustable divider affecting its output frequency.  Implements
 * .recalc_rate, .set_rate and .round_rate
 *
 * Flags:
 * CLK_DIVIDER_ONE_BASED - by default the divisor is the value read from the
 *	register plus one.  If CLK_DIVIDER_ONE_BASED is set then the divider is
 *	the raw value read from the register, with the value of zero considered
 *	invalid, unless CLK_DIVIDER_ALLOW_ZERO is set.
 * CLK_DIVIDER_POWER_OF_TWO - clock divisor is 2 raised to the value read from
 *	the hardware register
 * CLK_DIVIDER_ALLOW_ZERO - Allow zero divisors.  For dividers which have
 *	CLK_DIVIDER_ONE_BASED set, it is possible to end up with a zero divisor.
 *	Some hardware implementations gracefully handle this case and allow a
 *	zero divisor by not modifying their input clock
 *	(divide by one / bypass).
 * CLK_DIVIDER_HIWORD_MASK - The divider settings are only in lower 16-bit
 *	of this register, and mask of divider bits are in higher 16-bit of this
 *	register.  While setting the divider bits, higher 16-bit should also be
 *	updated to indicate changing divider bits.
 * CLK_DIVIDER_ROUND_CLOSEST - Makes the best calculated divider to be rounded
 *	to the closest integer instead of the up one.
 * CLK_DIVIDER_READ_ONLY - The divider settings are preconfigured and should
 *	not be changed by the clock framework.
 */
struct clk_divider {
	struct clk_hw	hw;
	void __iomem	*reg;
	u8		shift;
	u8		width;
	u8		flags;
	const struct clk_div_table	*table;
	spinlock_t	*lock;
};

#define CLK_DIVIDER_ONE_BASED		BIT(0)
#define CLK_DIVIDER_POWER_OF_TWO	BIT(1)
#define CLK_DIVIDER_ALLOW_ZERO		BIT(2)
#define CLK_DIVIDER_HIWORD_MASK		BIT(3)
#define CLK_DIVIDER_ROUND_CLOSEST	BIT(4)
#define CLK_DIVIDER_READ_ONLY		BIT(5)

extern const struct clk_ops clk_divider_ops;

unsigned long divider_recalc_rate(struct clk_hw *hw, unsigned long parent_rate,
		unsigned int val, const struct clk_div_table *table,
		unsigned long flags);
long divider_round_rate(struct clk_hw *hw, unsigned long rate,
		unsigned long *prate, const struct clk_div_table *table,
		u8 width, unsigned long flags);
int divider_get_val(unsigned long rate, unsigned long parent_rate,
		const struct clk_div_table *table, u8 width,
		unsigned long flags);

struct clk *clk_register_divider(struct device *dev, const char *name,
		const char *parent_name, unsigned long flags,
		void __iomem *reg, u8 shift, u8 width,
		u8 clk_divider_flags, spinlock_t *lock);
struct clk *clk_register_divider_table(struct device *dev, const char *name,
		const char *parent_name, unsigned long flags,
		void __iomem *reg, u8 shift, u8 width,
		u8 clk_divider_flags, const struct clk_div_table *table,
		spinlock_t *lock);
void clk_unregister_divider(struct clk *clk);

/**
 * struct clk_mux - multiplexer clock
 *
 * @hw:		handle between common and hardware-specific interfaces
 * @reg:	register controlling multiplexer
 * @shift:	shift to multiplexer bit field
 * @width:	width of mutliplexer bit field
 * @flags:	hardware-specific flags
 * @lock:	register lock
 *
 * Clock with multiple selectable parents.  Implements .get_parent, .set_parent
 * and .recalc_rate
 *
 * Flags:
 * CLK_MUX_INDEX_ONE - register index starts at 1, not 0
 * CLK_MUX_INDEX_BIT - register index is a single bit (power of two)
 * CLK_MUX_HIWORD_MASK - The mux settings are only in lower 16-bit of this
 *	register, and mask of mux bits are in higher 16-bit of this register.
 *	While setting the mux bits, higher 16-bit should also be updated to
 *	indicate changing mux bits.
 * CLK_MUX_ROUND_CLOSEST - Use the parent rate that is closest to the desired
 *	frequency.
 */
struct clk_mux {
	struct clk_hw	hw;
	void __iomem	*reg;
	u32		*table;
	u32		mask;
	u8		shift;
	u8		flags;
	spinlock_t	*lock;
};

#define CLK_MUX_INDEX_ONE		BIT(0)
#define CLK_MUX_INDEX_BIT		BIT(1)
#define CLK_MUX_HIWORD_MASK		BIT(2)
#define CLK_MUX_READ_ONLY		BIT(3) /* mux can't be changed */
#define CLK_MUX_ROUND_CLOSEST		BIT(4)

extern const struct clk_ops clk_mux_ops;
extern const struct clk_ops clk_mux_ro_ops;

struct clk *clk_register_mux(struct device *dev, const char *name,
		const char **parent_names, u8 num_parents, unsigned long flags,
		void __iomem *reg, u8 shift, u8 width,
		u8 clk_mux_flags, spinlock_t *lock);

struct clk *clk_register_mux_table(struct device *dev, const char *name,
		const char **parent_names, u8 num_parents, unsigned long flags,
		void __iomem *reg, u8 shift, u32 mask,
		u8 clk_mux_flags, u32 *table, spinlock_t *lock);

void clk_unregister_mux(struct clk *clk);

void of_fixed_factor_clk_setup(struct device_node *node);

/**
 * struct clk_fixed_factor - fixed multiplier and divider clock
 *
 * @hw:		handle between common and hardware-specific interfaces
 * @mult:	multiplier
 * @div:	divider
 *
 * Clock with a fixed multiplier and divider. The output frequency is the
 * parent clock rate divided by div and multiplied by mult.
 * Implements .recalc_rate, .set_rate and .round_rate
 */

struct clk_fixed_factor {
	struct clk_hw	hw;
	unsigned int	mult;
	unsigned int	div;
};

extern struct clk_ops clk_fixed_factor_ops;
struct clk *clk_register_fixed_factor(struct device *dev, const char *name,
		const char *parent_name, unsigned long flags,
		unsigned int mult, unsigned int div);

/**
 * struct clk_fractional_divider - adjustable fractional divider clock
 *
 * @hw:		handle between common and hardware-specific interfaces
 * @reg:	register containing the divider
 * @mshift:	shift to the numerator bit field
 * @mwidth:	width of the numerator bit field
 * @nshift:	shift to the denominator bit field
 * @nwidth:	width of the denominator bit field
 * @lock:	register lock
 *
 * Clock with adjustable fractional divider affecting its output frequency.
 */

struct clk_fractional_divider {
	struct clk_hw	hw;
	void __iomem	*reg;
	u8		mshift;
	u32		mmask;
	u8		nshift;
	u32		nmask;
	u8		flags;
	spinlock_t	*lock;
};

extern const struct clk_ops clk_fractional_divider_ops;
struct clk *clk_register_fractional_divider(struct device *dev,
		const char *name, const char *parent_name, unsigned long flags,
		void __iomem *reg, u8 mshift, u8 mwidth, u8 nshift, u8 nwidth,
		u8 clk_divider_flags, spinlock_t *lock);

/***
 * struct clk_composite - aggregate clock of mux, divider and gate clocks
 *
 * @hw:		handle between common and hardware-specific interfaces
 * @mux_hw:	handle between composite and hardware-specific mux clock
 * @rate_hw:	handle between composite and hardware-specific rate clock
 * @gate_hw:	handle between composite and hardware-specific gate clock
 * @mux_ops:	clock ops for mux
 * @rate_ops:	clock ops for rate
 * @gate_ops:	clock ops for gate
 */
struct clk_composite {
	struct clk_hw	hw;
	struct clk_ops	ops;

	struct clk_hw	*mux_hw;
	struct clk_hw	*rate_hw;
	struct clk_hw	*gate_hw;

	const struct clk_ops	*mux_ops;
	const struct clk_ops	*rate_ops;
	const struct clk_ops	*gate_ops;
};

struct clk *clk_register_composite(struct device *dev, const char *name,
		const char **parent_names, int num_parents,
		struct clk_hw *mux_hw, const struct clk_ops *mux_ops,
		struct clk_hw *rate_hw, const struct clk_ops *rate_ops,
		struct clk_hw *gate_hw, const struct clk_ops *gate_ops,
		unsigned long flags);

/***
 * struct clk_gpio_gate - gpio gated clock
 *
 * @hw:		handle between common and hardware-specific interfaces
 * @gpiod:	gpio descriptor
 *
 * Clock with a gpio control for enabling and disabling the parent clock.
 * Implements .enable, .disable and .is_enabled
 */

struct clk_gpio {
	struct clk_hw	hw;
	struct gpio_desc *gpiod;
};

extern const struct clk_ops clk_gpio_gate_ops;
struct clk *clk_register_gpio_gate(struct device *dev, const char *name,
		const char *parent_name, unsigned gpio, bool active_low,
		unsigned long flags);

void of_gpio_clk_gate_setup(struct device_node *node);

/**
 * clk_register - allocate a new clock, register it and return an opaque cookie
 * @dev: device that is registering this clock
 * @hw: link to hardware-specific clock data
 *
 * clk_register is the primary interface for populating the clock tree with new
 * clock nodes.  It returns a pointer to the newly allocated struct clk which
 * cannot be dereferenced by driver code but may be used in conjuction with the
 * rest of the clock API.  In the event of an error clk_register will return an
 * error code; drivers must test for an error code after calling clk_register.
 */
struct clk *clk_register(struct device *dev, struct clk_hw *hw);
struct clk *devm_clk_register(struct device *dev, struct clk_hw *hw);

void clk_unregister(struct clk *clk);
void devm_clk_unregister(struct device *dev, struct clk *clk);

/* helper functions */
const char *__clk_get_name(struct clk *clk);
struct clk_hw *__clk_get_hw(struct clk *clk);
u8 __clk_get_num_parents(struct clk *clk);
struct clk *__clk_get_parent(struct clk *clk);
struct clk *clk_get_parent_by_index(struct clk *clk, u8 index);
unsigned int __clk_get_enable_count(struct clk *clk);
unsigned long __clk_get_rate(struct clk *clk);
unsigned long __clk_get_flags(struct clk *clk);
bool __clk_is_prepared(struct clk *clk);
bool __clk_is_enabled(struct clk *clk);
struct clk *__clk_lookup(const char *name);
long __clk_mux_determine_rate(struct clk_hw *hw, unsigned long rate,
			      unsigned long min_rate,
			      unsigned long max_rate,
			      unsigned long *best_parent_rate,
			      struct clk_hw **best_parent_p);
unsigned long __clk_determine_rate(struct clk_hw *core,
				   unsigned long rate,
				   unsigned long min_rate,
				   unsigned long max_rate);
long __clk_mux_determine_rate_closest(struct clk_hw *hw, unsigned long rate,
			      unsigned long min_rate,
			      unsigned long max_rate,
			      unsigned long *best_parent_rate,
			      struct clk_hw **best_parent_p);

static inline void __clk_hw_set_clk(struct clk_hw *dst, struct clk_hw *src)
{
	dst->clk = src->clk;
	dst->core = src->core;
}

/*
 * FIXME clock api without lock protection
 */
unsigned long __clk_round_rate(struct clk *clk, unsigned long rate);

struct of_device_id;

typedef void (*of_clk_init_cb_t)(struct device_node *);

struct clk_onecell_data {
	struct clk **clks;
	unsigned int clk_num;
};

extern struct of_device_id __clk_of_table;

#define CLK_OF_DECLARE(name, compat, fn) OF_DECLARE_1(clk, name, compat, fn)

#ifdef CONFIG_OF
int of_clk_add_provider(struct device_node *np,
			struct clk *(*clk_src_get)(struct of_phandle_args *args,
						   void *data),
			void *data);
void of_clk_del_provider(struct device_node *np);
struct clk *of_clk_src_simple_get(struct of_phandle_args *clkspec,
				  void *data);
struct clk *of_clk_src_onecell_get(struct of_phandle_args *clkspec, void *data);
int of_clk_get_parent_count(struct device_node *np);
const char *of_clk_get_parent_name(struct device_node *np, int index);

void of_clk_init(const struct of_device_id *matches);

#else /* !CONFIG_OF */

static inline int of_clk_add_provider(struct device_node *np,
			struct clk *(*clk_src_get)(struct of_phandle_args *args,
						   void *data),
			void *data)
{
	return 0;
}
#define of_clk_del_provider(np) \
	{ while (0); }
static inline struct clk *of_clk_src_simple_get(
	struct of_phandle_args *clkspec, void *data)
{
	return ERR_PTR(-ENOENT);
}
static inline struct clk *of_clk_src_onecell_get(
	struct of_phandle_args *clkspec, void *data)
{
	return ERR_PTR(-ENOENT);
}
static inline const char *of_clk_get_parent_name(struct device_node *np,
						 int index)
{
	return NULL;
}
#define of_clk_init(matches) \
	{ while (0); }
#endif /* CONFIG_OF */

/*
 * wrap access to peripherals in accessor routines
 * for improved portability across platforms
 */

#if IS_ENABLED(CONFIG_PPC)

static inline u32 clk_readl(u32 __iomem *reg)
{
	return ioread32be(reg);
}

static inline void clk_writel(u32 val, u32 __iomem *reg)
{
	iowrite32be(val, reg);
}

#else	/* platform dependent I/O accessors */

static inline u32 clk_readl(u32 __iomem *reg)
{
	return readl(reg);
}

static inline void clk_writel(u32 val, u32 __iomem *reg)
{
	writel(val, reg);
}

#endif	/* platform dependent I/O accessors */

#ifdef CONFIG_DEBUG_FS
struct dentry *clk_debugfs_add_file(struct clk_hw *hw, char *name, umode_t mode,
				void *data, const struct file_operations *fops);
#endif

#endif /* CONFIG_COMMON_CLK */
#endif /* CLK_PROVIDER_H */
