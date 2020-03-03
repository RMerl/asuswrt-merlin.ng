/*
 * TI clock drivers support
 *
 * Copyright (C) 2013 Texas Instruments, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef __LINUX_CLK_TI_H__
#define __LINUX_CLK_TI_H__

#include <linux/clk-provider.h>
#include <linux/clkdev.h>

/**
 * struct dpll_data - DPLL registers and integration data
 * @mult_div1_reg: register containing the DPLL M and N bitfields
 * @mult_mask: mask of the DPLL M bitfield in @mult_div1_reg
 * @div1_mask: mask of the DPLL N bitfield in @mult_div1_reg
 * @clk_bypass: struct clk pointer to the clock's bypass clock input
 * @clk_ref: struct clk pointer to the clock's reference clock input
 * @control_reg: register containing the DPLL mode bitfield
 * @enable_mask: mask of the DPLL mode bitfield in @control_reg
 * @last_rounded_rate: cache of the last rate result of omap2_dpll_round_rate()
 * @last_rounded_m: cache of the last M result of omap2_dpll_round_rate()
 * @last_rounded_m4xen: cache of the last M4X result of
 *			omap4_dpll_regm4xen_round_rate()
 * @last_rounded_lpmode: cache of the last lpmode result of
 *			 omap4_dpll_lpmode_recalc()
 * @max_multiplier: maximum valid non-bypass multiplier value (actual)
 * @last_rounded_n: cache of the last N result of omap2_dpll_round_rate()
 * @min_divider: minimum valid non-bypass divider value (actual)
 * @max_divider: maximum valid non-bypass divider value (actual)
 * @modes: possible values of @enable_mask
 * @autoidle_reg: register containing the DPLL autoidle mode bitfield
 * @idlest_reg: register containing the DPLL idle status bitfield
 * @autoidle_mask: mask of the DPLL autoidle mode bitfield in @autoidle_reg
 * @freqsel_mask: mask of the DPLL jitter correction bitfield in @control_reg
 * @dcc_mask: mask of the DPLL DCC correction bitfield @mult_div1_reg
 * @dcc_rate: rate atleast which DCC @dcc_mask must be set
 * @idlest_mask: mask of the DPLL idle status bitfield in @idlest_reg
 * @lpmode_mask: mask of the DPLL low-power mode bitfield in @control_reg
 * @m4xen_mask: mask of the DPLL M4X multiplier bitfield in @control_reg
 * @auto_recal_bit: bitshift of the driftguard enable bit in @control_reg
 * @recal_en_bit: bitshift of the PRM_IRQENABLE_* bit for recalibration IRQs
 * @recal_st_bit: bitshift of the PRM_IRQSTATUS_* bit for recalibration IRQs
 * @flags: DPLL type/features (see below)
 *
 * Possible values for @flags:
 * DPLL_J_TYPE: "J-type DPLL" (only some 36xx, 4xxx DPLLs)
 *
 * @freqsel_mask is only used on the OMAP34xx family and AM35xx.
 *
 * XXX Some DPLLs have multiple bypass inputs, so it's not technically
 * correct to only have one @clk_bypass pointer.
 *
 * XXX The runtime-variable fields (@last_rounded_rate, @last_rounded_m,
 * @last_rounded_n) should be separated from the runtime-fixed fields
 * and placed into a different structure, so that the runtime-fixed data
 * can be placed into read-only space.
 */
struct dpll_data {
	void __iomem		*mult_div1_reg;
	u32			mult_mask;
	u32			div1_mask;
	struct clk		*clk_bypass;
	struct clk		*clk_ref;
	void __iomem		*control_reg;
	u32			enable_mask;
	unsigned long		last_rounded_rate;
	u16			last_rounded_m;
	u8			last_rounded_m4xen;
	u8			last_rounded_lpmode;
	u16			max_multiplier;
	u8			last_rounded_n;
	u8			min_divider;
	u16			max_divider;
	u8			modes;
	void __iomem		*autoidle_reg;
	void __iomem		*idlest_reg;
	u32			autoidle_mask;
	u32			freqsel_mask;
	u32			idlest_mask;
	u32			dco_mask;
	u32			sddiv_mask;
	u32			dcc_mask;
	unsigned long		dcc_rate;
	u32			lpmode_mask;
	u32			m4xen_mask;
	u8			auto_recal_bit;
	u8			recal_en_bit;
	u8			recal_st_bit;
	u8			flags;
};

struct clk_hw_omap;

/**
 * struct clk_hw_omap_ops - OMAP clk ops
 * @find_idlest: find idlest register information for a clock
 * @find_companion: find companion clock register information for a clock,
 *		    basically converts CM_ICLKEN* <-> CM_FCLKEN*
 * @allow_idle: enables autoidle hardware functionality for a clock
 * @deny_idle: prevent autoidle hardware functionality for a clock
 */
struct clk_hw_omap_ops {
	void	(*find_idlest)(struct clk_hw_omap *oclk,
			       void __iomem **idlest_reg,
			       u8 *idlest_bit, u8 *idlest_val);
	void	(*find_companion)(struct clk_hw_omap *oclk,
				  void __iomem **other_reg,
				  u8 *other_bit);
	void	(*allow_idle)(struct clk_hw_omap *oclk);
	void	(*deny_idle)(struct clk_hw_omap *oclk);
};

/**
 * struct clk_hw_omap - OMAP struct clk
 * @node: list_head connecting this clock into the full clock list
 * @enable_reg: register to write to enable the clock (see @enable_bit)
 * @enable_bit: bitshift to write to enable/disable the clock (see @enable_reg)
 * @flags: see "struct clk.flags possibilities" above
 * @clksel_reg: for clksel clks, register va containing src/divisor select
 * @clksel_mask: bitmask in @clksel_reg for the src/divisor selector
 * @clksel: for clksel clks, pointer to struct clksel for this clock
 * @dpll_data: for DPLLs, pointer to struct dpll_data for this clock
 * @clkdm_name: clockdomain name that this clock is contained in
 * @clkdm: pointer to struct clockdomain, resolved from @clkdm_name at runtime
 * @ops: clock ops for this clock
 */
struct clk_hw_omap {
	struct clk_hw		hw;
	struct list_head	node;
	unsigned long		fixed_rate;
	u8			fixed_div;
	void __iomem		*enable_reg;
	u8			enable_bit;
	u8			flags;
	void __iomem		*clksel_reg;
	u32			clksel_mask;
	const struct clksel	*clksel;
	struct dpll_data	*dpll_data;
	const char		*clkdm_name;
	struct clockdomain	*clkdm;
	const struct clk_hw_omap_ops	*ops;
};

/*
 * struct clk_hw_omap.flags possibilities
 *
 * XXX document the rest of the clock flags here
 *
 * ENABLE_REG_32BIT: (OMAP1 only) clock control register must be accessed
 *     with 32bit ops, by default OMAP1 uses 16bit ops.
 * CLOCK_IDLE_CONTROL: (OMAP1 only) clock has autoidle support.
 * CLOCK_NO_IDLE_PARENT: (OMAP1 only) when clock is enabled, its parent
 *     clock is put to no-idle mode.
 * ENABLE_ON_INIT: Clock is enabled on init.
 * INVERT_ENABLE: By default, clock enable bit behavior is '1' enable, '0'
 *     disable. This inverts the behavior making '0' enable and '1' disable.
 * CLOCK_CLKOUTX2: (OMAP4 only) DPLL CLKOUT and CLKOUTX2 GATE_CTRL
 *     bits share the same register.  This flag allows the
 *     omap4_dpllmx*() code to determine which GATE_CTRL bit field
 *     should be used.  This is a temporary solution - a better approach
 *     would be to associate clock type-specific data with the clock,
 *     similar to the struct dpll_data approach.
 * MEMMAP_ADDRESSING: Use memmap addressing to access clock registers.
 */
#define ENABLE_REG_32BIT	(1 << 0)	/* Use 32-bit access */
#define CLOCK_IDLE_CONTROL	(1 << 1)
#define CLOCK_NO_IDLE_PARENT	(1 << 2)
#define ENABLE_ON_INIT		(1 << 3)	/* Enable upon framework init */
#define INVERT_ENABLE		(1 << 4)	/* 0 enables, 1 disables */
#define CLOCK_CLKOUTX2		(1 << 5)
#define MEMMAP_ADDRESSING	(1 << 6)

/* CM_CLKEN_PLL*.EN* bit values - not all are available for every DPLL */
#define DPLL_LOW_POWER_STOP	0x1
#define DPLL_LOW_POWER_BYPASS	0x5
#define DPLL_LOCKED		0x7

/* DPLL Type and DCO Selection Flags */
#define DPLL_J_TYPE		0x1

/* Composite clock component types */
enum {
	CLK_COMPONENT_TYPE_GATE = 0,
	CLK_COMPONENT_TYPE_DIVIDER,
	CLK_COMPONENT_TYPE_MUX,
	CLK_COMPONENT_TYPE_MAX,
};

/**
 * struct ti_dt_clk - OMAP DT clock alias declarations
 * @lk: clock lookup definition
 * @node_name: clock DT node to map to
 */
struct ti_dt_clk {
	struct clk_lookup		lk;
	char				*node_name;
};

#define DT_CLK(dev, con, name)		\
	{				\
		.lk = {			\
			.dev_id = dev,	\
			.con_id = con,	\
		},			\
		.node_name = name,	\
	}

/* Static memmap indices */
enum {
	TI_CLKM_CM = 0,
	TI_CLKM_CM2,
	TI_CLKM_PRM,
	TI_CLKM_SCRM,
	TI_CLKM_CTRL,
	CLK_MAX_MEMMAPS
};

typedef void (*ti_of_clk_init_cb_t)(struct clk_hw *, struct device_node *);

/**
 * struct clk_omap_reg - OMAP register declaration
 * @offset: offset from the master IP module base address
 * @index: index of the master IP module
 */
struct clk_omap_reg {
	u16 offset;
	u16 index;
};

/**
 * struct ti_clk_ll_ops - low-level register access ops for a clock
 * @clk_readl: pointer to register read function
 * @clk_writel: pointer to register write function
 *
 * Low-level register access ops are generally used by the basic clock types
 * (clk-gate, clk-mux, clk-divider etc.) to provide support for various
 * low-level hardware interfaces (direct MMIO, regmap etc.), but can also be
 * used by other hardware-specific clock drivers if needed.
 */
struct ti_clk_ll_ops {
	u32	(*clk_readl)(void __iomem *reg);
	void	(*clk_writel)(u32 val, void __iomem *reg);
};

extern struct ti_clk_ll_ops *ti_clk_ll_ops;

extern const struct clk_ops ti_clk_divider_ops;
extern const struct clk_ops ti_clk_mux_ops;

#define to_clk_hw_omap(_hw) container_of(_hw, struct clk_hw_omap, hw)

void omap2_init_clk_hw_omap_clocks(struct clk *clk);
int omap3_noncore_dpll_enable(struct clk_hw *hw);
void omap3_noncore_dpll_disable(struct clk_hw *hw);
int omap3_noncore_dpll_set_parent(struct clk_hw *hw, u8 index);
int omap3_noncore_dpll_set_rate(struct clk_hw *hw, unsigned long rate,
				unsigned long parent_rate);
int omap3_noncore_dpll_set_rate_and_parent(struct clk_hw *hw,
					   unsigned long rate,
					   unsigned long parent_rate,
					   u8 index);
long omap3_noncore_dpll_determine_rate(struct clk_hw *hw,
				       unsigned long rate,
				       unsigned long min_rate,
				       unsigned long max_rate,
				       unsigned long *best_parent_rate,
				       struct clk_hw **best_parent_clk);
unsigned long omap4_dpll_regm4xen_recalc(struct clk_hw *hw,
					 unsigned long parent_rate);
long omap4_dpll_regm4xen_round_rate(struct clk_hw *hw,
				    unsigned long target_rate,
				    unsigned long *parent_rate);
long omap4_dpll_regm4xen_determine_rate(struct clk_hw *hw,
					unsigned long rate,
					unsigned long min_rate,
					unsigned long max_rate,
					unsigned long *best_parent_rate,
					struct clk_hw **best_parent_clk);
u8 omap2_init_dpll_parent(struct clk_hw *hw);
unsigned long omap3_dpll_recalc(struct clk_hw *hw, unsigned long parent_rate);
long omap2_dpll_round_rate(struct clk_hw *hw, unsigned long target_rate,
			   unsigned long *parent_rate);
void omap2_init_clk_clkdm(struct clk_hw *clk);
unsigned long omap3_clkoutx2_recalc(struct clk_hw *hw,
				    unsigned long parent_rate);
int omap3_clkoutx2_set_rate(struct clk_hw *hw, unsigned long rate,
					unsigned long parent_rate);
long omap3_clkoutx2_round_rate(struct clk_hw *hw, unsigned long rate,
		unsigned long *prate);
int omap2_clkops_enable_clkdm(struct clk_hw *hw);
void omap2_clkops_disable_clkdm(struct clk_hw *hw);
int omap2_clk_disable_autoidle_all(void);
void omap2_clk_enable_init_clocks(const char **clk_names, u8 num_clocks);
int omap3_dpll4_set_rate(struct clk_hw *clk, unsigned long rate,
			 unsigned long parent_rate);
int omap3_dpll4_set_rate_and_parent(struct clk_hw *hw, unsigned long rate,
				    unsigned long parent_rate, u8 index);
int omap2_dflt_clk_enable(struct clk_hw *hw);
void omap2_dflt_clk_disable(struct clk_hw *hw);
int omap2_dflt_clk_is_enabled(struct clk_hw *hw);
void omap3_clk_lock_dpll5(void);
unsigned long omap2_dpllcore_recalc(struct clk_hw *hw,
				    unsigned long parent_rate);
int omap2_reprogram_dpllcore(struct clk_hw *clk, unsigned long rate,
			     unsigned long parent_rate);
void omap2xxx_clkt_dpllcore_init(struct clk_hw *hw);
void omap2xxx_clkt_vps_init(void);

void __iomem *ti_clk_get_reg_addr(struct device_node *node, int index);
void ti_dt_clocks_register(struct ti_dt_clk *oclks);
void ti_dt_clk_init_provider(struct device_node *np, int index);
void ti_dt_clk_init_retry_clks(void);
void ti_dt_clockdomains_setup(void);
int ti_clk_retry_init(struct device_node *node, struct clk_hw *hw,
		      ti_of_clk_init_cb_t func);
int of_ti_clk_autoidle_setup(struct device_node *node);
int ti_clk_add_component(struct device_node *node, struct clk_hw *hw, int type);

int omap3430_dt_clk_init(void);
int omap3630_dt_clk_init(void);
int am35xx_dt_clk_init(void);
int ti81xx_dt_clk_init(void);
int omap4xxx_dt_clk_init(void);
int omap5xxx_dt_clk_init(void);
int dra7xx_dt_clk_init(void);
int am33xx_dt_clk_init(void);
int am43xx_dt_clk_init(void);
int omap2420_dt_clk_init(void);
int omap2430_dt_clk_init(void);

#ifdef CONFIG_OF
void of_ti_clk_allow_autoidle_all(void);
void of_ti_clk_deny_autoidle_all(void);
#else
static inline void of_ti_clk_allow_autoidle_all(void) { }
static inline void of_ti_clk_deny_autoidle_all(void) { }
#endif

extern const struct clk_hw_omap_ops clkhwops_omap2xxx_dpll;
extern const struct clk_hw_omap_ops clkhwops_omap2430_i2chs_wait;
extern const struct clk_hw_omap_ops clkhwops_omap3_dpll;
extern const struct clk_hw_omap_ops clkhwops_omap4_dpllmx;
extern const struct clk_hw_omap_ops clkhwops_wait;
extern const struct clk_hw_omap_ops clkhwops_omap3430es2_dss_usbhost_wait;
extern const struct clk_hw_omap_ops clkhwops_am35xx_ipss_module_wait;
extern const struct clk_hw_omap_ops clkhwops_am35xx_ipss_wait;
extern const struct clk_hw_omap_ops clkhwops_iclk;
extern const struct clk_hw_omap_ops clkhwops_iclk_wait;
extern const struct clk_hw_omap_ops clkhwops_omap3430es2_iclk_ssi_wait;
extern const struct clk_hw_omap_ops clkhwops_omap3430es2_iclk_dss_usbhost_wait;
extern const struct clk_hw_omap_ops clkhwops_omap3430es2_iclk_hsotgusb_wait;

#ifdef CONFIG_ATAGS
int omap3430_clk_legacy_init(void);
int omap3430es1_clk_legacy_init(void);
int omap36xx_clk_legacy_init(void);
int am35xx_clk_legacy_init(void);
#else
static inline int omap3430_clk_legacy_init(void) { return -ENXIO; }
static inline int omap3430es1_clk_legacy_init(void) { return -ENXIO; }
static inline int omap36xx_clk_legacy_init(void) { return -ENXIO; }
static inline int am35xx_clk_legacy_init(void) { return -ENXIO; }
#endif


#endif
