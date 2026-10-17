/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 */

#ifndef __SANDBOX_CLK_H__
#define __SANDBOX_CLK_H__

#include <linux/clk-provider.h>

enum {
	SANDBOX_CLK_PLL2 = 1,
	SANDBOX_CLK_PLL3,
	SANDBOX_CLK_PLL3_60M,
	SANDBOX_CLK_PLL3_80M,
	SANDBOX_CLK_ECSPI_ROOT,
	SANDBOX_CLK_ECSPI0,
	SANDBOX_CLK_ECSPI1,
	SANDBOX_CLK_USDHC1_SEL,
	SANDBOX_CLK_USDHC2_SEL,
	SANDBOX_CLK_I2C,
	SANDBOX_CLK_I2C_ROOT,
};

enum sandbox_pllv3_type {
	SANDBOX_PLLV3_GENERIC,
	SANDBOX_PLLV3_USB,
};

struct clk *sandbox_clk_pllv3(enum sandbox_pllv3_type type, const char *name,
			      const char *parent_name, void __iomem *base,
			      u32 div_mask);

static inline struct clk *sandbox_clk_fixed_factor(const char *name,
						   const char *parent,
						   unsigned int mult,
						   unsigned int div)
{
	return clk_register_fixed_factor(NULL, name, parent,
			CLK_SET_RATE_PARENT, mult, div);
}

static inline struct clk *sandbox_clk_divider(const char *name,
					      const char *parent,
					      void __iomem *reg, u8 shift,
					      u8 width)
{
	return clk_register_divider(NULL, name, parent, CLK_SET_RATE_PARENT,
			reg, shift, width, 0);
}

static inline struct clk *sandbox_clk_gate(const char *name, const char *parent,
					   void __iomem *reg, u8 bit_idx,
					   u8 clk_gate_flags)
{
	return clk_register_gate(NULL, name, parent, CLK_SET_RATE_PARENT,
				 reg, bit_idx, clk_gate_flags, NULL);
}

struct clk *sandbox_clk_register_gate2(struct device *dev, const char *name,
				       const char *parent_name,
				       unsigned long flags,
				       void __iomem *reg, u8 bit_idx,
				       u8 cgr_val, u8 clk_gate_flags);

static inline struct clk *sandbox_clk_gate2(const char *name,
					    const char *parent,
					    void __iomem *reg, u8 shift)
{
	return sandbox_clk_register_gate2(NULL, name, parent,
					  CLK_SET_RATE_PARENT, reg, shift,
					  0x3, 0);
}

static inline struct clk *sandbox_clk_mux(const char *name, void __iomem *reg,
					  u8 shift, u8 width,
					  const char * const *parents,
					  int num_parents)
{
	return clk_register_mux(NULL, name, parents, num_parents,
				CLK_SET_RATE_NO_REPARENT, reg, shift,
				width, 0);
}

int sandbox_clk_enable_count(struct clk *clk);

#endif /* __SANDBOX_CLK_H__ */
