// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 */
#ifndef __MACH_IMX_CLK_H
#define __MACH_IMX_CLK_H

#include <linux/clk-provider.h>

enum imx_pllv3_type {
	IMX_PLLV3_GENERIC,
	IMX_PLLV3_GENERICV2,
	IMX_PLLV3_SYS,
	IMX_PLLV3_USB,
	IMX_PLLV3_USB_VF610,
	IMX_PLLV3_AV,
	IMX_PLLV3_ENET,
	IMX_PLLV3_ENET_IMX7,
	IMX_PLLV3_SYS_VF610,
	IMX_PLLV3_DDR_IMX7,
};

enum imx_pll14xx_type {
	PLL_1416X,
	PLL_1443X,
};

/* NOTE: Rate table should be kept sorted in descending order. */
struct imx_pll14xx_rate_table {
	unsigned int rate;
	unsigned int pdiv;
	unsigned int mdiv;
	unsigned int sdiv;
	unsigned int kdiv;
};

struct imx_pll14xx_clk {
	enum imx_pll14xx_type type;
	const struct imx_pll14xx_rate_table *rate_table;
	int rate_count;
	int flags;
};

extern struct imx_pll14xx_clk imx_1416x_pll;
extern struct imx_pll14xx_clk imx_1443x_pll;
extern struct imx_pll14xx_clk imx_1443x_dram_pll;

#define CLK_FRACN_GPPLL_INTEGER	BIT(0)
#define CLK_FRACN_GPPLL_FRACN	BIT(1)

/* NOTE: Rate table should be kept sorted in descending order. */
struct imx_fracn_gppll_rate_table {
	unsigned int rate;
	unsigned int mfi;
	unsigned int mfn;
	unsigned int mfd;
	unsigned int rdiv;
	unsigned int odiv;
};

struct imx_fracn_gppll_clk {
	const struct imx_fracn_gppll_rate_table *rate_table;
	int rate_count;
	int flags;
};

struct clk *imx_clk_fracn_gppll(const char *name, const char *parent_name, void __iomem *base,
				const struct imx_fracn_gppll_clk *pll_clk);
struct clk *imx_clk_fracn_gppll_integer(const char *name, const char *parent_name,
					void __iomem *base,
					const struct imx_fracn_gppll_clk *pll_clk);

extern struct imx_fracn_gppll_clk imx_fracn_gppll;
extern struct imx_fracn_gppll_clk imx_fracn_gppll_integer;

struct clk *imx_clk_pll14xx(const char *name, const char *parent_name,
			    void __iomem *base,
			    const struct imx_pll14xx_clk *pll_clk);

struct clk *clk_register_gate2(struct device *dev, const char *name,
		const char *parent_name, unsigned long flags,
		void __iomem *reg, u8 bit_idx, u8 cgr_val,
		u8 clk_gate_flags, unsigned int *share_count);

struct clk *imx_clk_pllv3(enum imx_pllv3_type type, const char *name,
			  const char *parent_name, void __iomem *base,
			  u32 div_mask);

static inline struct clk *imx_clk_gate2(const char *name, const char *parent,
					void __iomem *reg, u8 shift)
{
	return clk_register_gate2(NULL, name, parent, CLK_SET_RATE_PARENT, reg,
			shift, 0x3, 0, NULL);
}

static inline struct clk *imx_clk_gate2_shared(const char *name,
					       const char *parent,
					       void __iomem *reg, u8 shift,
					       unsigned int *share_count)
{
	return clk_register_gate2(NULL, name, parent, CLK_SET_RATE_PARENT, reg,
				  shift, 0x3, 0, share_count);
}

static inline struct clk *imx_clk_gate2_shared2(const char *name,
						const char *parent,
						void __iomem *reg, u8 shift,
						unsigned int *share_count)
{
	return clk_register_gate2(NULL, name, parent, CLK_SET_RATE_PARENT |
				  CLK_OPS_PARENT_ENABLE, reg, shift, 0x3, 0,
				  share_count);
}

static inline struct clk *imx_clk_gate4(const char *name, const char *parent,
		void __iomem *reg, u8 shift)
{
	return clk_register_gate2(NULL, name, parent,
			CLK_SET_RATE_PARENT | CLK_OPS_PARENT_ENABLE,
			reg, shift, 0x3, 0, NULL);
}

static inline struct clk *imx_clk_gate4_flags(const char *name,
		const char *parent, void __iomem *reg, u8 shift,
		unsigned long flags)
{
	return clk_register_gate2(NULL, name, parent,
			flags | CLK_SET_RATE_PARENT | CLK_OPS_PARENT_ENABLE,
			reg, shift, 0x3, 0, NULL);
}

static inline struct clk *imx_clk_fixed_factor(const char *name,
		const char *parent, unsigned int mult, unsigned int div)
{
	return clk_register_fixed_factor(NULL, name, parent,
			CLK_SET_RATE_PARENT, mult, div);
}

static inline struct clk *imx_clk_divider(const char *name, const char *parent,
		void __iomem *reg, u8 shift, u8 width)
{
	return clk_register_divider(NULL, name, parent, CLK_SET_RATE_PARENT,
			reg, shift, width, 0);
}

static inline struct clk *
imx_clk_busy_divider(const char *name, const char *parent, void __iomem *reg,
		     u8 shift, u8 width, void __iomem *busy_reg, u8 busy_shift)
{
	return clk_register_divider(NULL, name, parent, CLK_SET_RATE_PARENT,
			reg, shift, width, 0);
}

static inline struct clk *imx_clk_divider2(const char *name, const char *parent,
		void __iomem *reg, u8 shift, u8 width)
{
	return clk_register_divider(NULL, name, parent,
			CLK_SET_RATE_PARENT | CLK_OPS_PARENT_ENABLE,
			reg, shift, width, 0);
}

struct clk *imx_clk_pfd(const char *name, const char *parent_name,
			void __iomem *reg, u8 idx);

struct clk *imx_clk_fixup_mux(const char *name, void __iomem *reg,
			      u8 shift, u8 width, const char * const *parents,
			      int num_parents, void (*fixup)(u32 *val));

static inline struct clk *imx_clk_mux_flags(const char *name,
			void __iomem *reg, u8 shift, u8 width,
			const char * const *parents, int num_parents,
			unsigned long flags)
{
	return clk_register_mux(NULL, name, parents, num_parents,
				flags | CLK_SET_RATE_NO_REPARENT, reg, shift,
				width, 0);
}

static inline struct clk *imx_clk_mux2_flags(const char *name,
		void __iomem *reg, u8 shift, u8 width,
		const char * const *parents,
		int num_parents, unsigned long flags)
{
	return clk_register_mux(NULL, name, parents, num_parents,
			flags | CLK_SET_RATE_NO_REPARENT | CLK_OPS_PARENT_ENABLE,
			reg, shift, width, 0);
}

static inline struct clk *imx_clk_mux(const char *name, void __iomem *reg,
			u8 shift, u8 width, const char * const *parents,
			int num_parents)
{
	return clk_register_mux(NULL, name, parents, num_parents,
			CLK_SET_RATE_NO_REPARENT, reg, shift,
			width, 0);
}

static inline struct clk *
imx_clk_busy_mux(const char *name, void __iomem *reg, u8 shift, u8 width,
		 void __iomem *busy_reg, u8 busy_shift,
		 const char * const *parents, int num_parents)
{
	return clk_register_mux(NULL, name, parents, num_parents,
			CLK_SET_RATE_NO_REPARENT, reg, shift,
			width, 0);
}

static inline struct clk *imx_clk_mux2(const char *name, void __iomem *reg,
			u8 shift, u8 width, const char * const *parents,
			int num_parents)
{
	return clk_register_mux(NULL, name, parents, num_parents,
			CLK_SET_RATE_NO_REPARENT | CLK_OPS_PARENT_ENABLE,
			reg, shift, width, 0);
}

static inline struct clk *imx_clk_gate(const char *name, const char *parent,
		void __iomem *reg, u8 shift)
{
	return clk_register_gate(NULL, name, parent, CLK_SET_RATE_PARENT, reg,
			shift, 0, NULL);
}

static inline struct clk *imx_clk_gate_flags(const char *name, const char *parent,
		void __iomem *reg, u8 shift, unsigned long flags)
{
	return clk_register_gate(NULL, name, parent, flags | CLK_SET_RATE_PARENT, reg,
			shift, 0, NULL);
}

static inline struct clk *imx_clk_gate3(const char *name, const char *parent,
		void __iomem *reg, u8 shift)
{
	return clk_register_gate(NULL, name, parent,
			CLK_SET_RATE_PARENT | CLK_OPS_PARENT_ENABLE,
			reg, shift, 0, NULL);
}

struct clk *imx8m_clk_composite_flags(const char *name,
		const char * const *parent_names,
		int num_parents, void __iomem *reg, unsigned long flags);

#define __imx8m_clk_composite(name, parent_names, reg, flags) \
	imx8m_clk_composite_flags(name, parent_names, \
		ARRAY_SIZE(parent_names), reg, \
		flags | CLK_SET_RATE_NO_REPARENT | CLK_OPS_PARENT_ENABLE)

#define imx8m_clk_composite(name, parent_names, reg) \
	__imx8m_clk_composite(name, parent_names, reg, 0)

#define imx8m_clk_composite_critical(name, parent_names, reg) \
	__imx8m_clk_composite(name, parent_names, reg, CLK_IS_CRITICAL)

struct clk *imx93_clk_composite_flags(const char *name,
				      const char * const *parent_names,
				      int num_parents,
				      void __iomem *reg,
				      u32 domain_id,
				      unsigned long flags);
#define imx93_clk_composite(name, parent_names, num_parents, reg, domain_id) \
	imx93_clk_composite_flags(name, parent_names, num_parents, reg, domain_id \
				  CLK_SET_RATE_NO_REPARENT | CLK_OPS_PARENT_ENABLE)

struct clk *imx93_clk_gate(struct device *dev, const char *name, const char *parent_name,
			   unsigned long flags, void __iomem *reg, u32 bit_idx, u32 val,
			   u32 mask, u32 domain_id, unsigned int *share_count);

#endif /* __MACH_IMX_CLK_H */
