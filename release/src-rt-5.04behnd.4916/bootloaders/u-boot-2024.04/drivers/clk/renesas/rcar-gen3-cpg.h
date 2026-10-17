/* SPDX-License-Identifier: GPL-2.0 */
/*
 * R-Car Gen3 Clock Pulse Generator
 *
 * Copyright (C) 2015-2018 Glider bvba
 * Copyright (C) 2018 Renesas Electronics Corp.
 *
 */

#ifndef __CLK_RENESAS_RCAR_GEN3_CPG_H__
#define __CLK_RENESAS_RCAR_GEN3_CPG_H__

enum rcar_gen3_clk_types {
	CLK_TYPE_GEN3_MAIN = CLK_TYPE_CUSTOM,
	CLK_TYPE_GEN3_PLL0,
	CLK_TYPE_GEN3_PLL1,
	CLK_TYPE_GEN3_PLL2,
	CLK_TYPE_GEN3_PLL3,
	CLK_TYPE_GEN3_PLL4,
	CLK_TYPE_GEN3_SDH,
	CLK_TYPE_R8A77970_SD0H,
	CLK_TYPE_GEN3_SD,
	CLK_TYPE_R8A77970_SD0,
	CLK_TYPE_GEN3_R,
	CLK_TYPE_GEN3_MDSEL,	/* Select parent/divider using mode pin */
	CLK_TYPE_GEN3_Z,
	CLK_TYPE_GEN3_ZG,
	CLK_TYPE_GEN3_OSC,	/* OSC EXTAL predivider and fixed divider */
	CLK_TYPE_GEN3_RCKSEL,	/* Select parent/divider using RCKCR.CKSEL */
	CLK_TYPE_GEN3_RPCSRC,
	CLK_TYPE_GEN3_D3_RPCSRC,
	CLK_TYPE_GEN3_E3_RPCSRC,
	CLK_TYPE_GEN3_RPC,
	CLK_TYPE_GEN3_RPCD2,

	CLK_TYPE_GEN4_MAIN,
	CLK_TYPE_GEN4_PLL1,
	CLK_TYPE_GEN4_PLL2,
	CLK_TYPE_GEN4_PLL2X_3X,	/* R8A779A0 only */
	CLK_TYPE_GEN4_PLL3,
	CLK_TYPE_GEN4_PLL5,
	CLK_TYPE_GEN4_PLL4,
	CLK_TYPE_GEN4_PLL6,
	CLK_TYPE_GEN4_PLL7,
	CLK_TYPE_GEN4_SDSRC,
	CLK_TYPE_GEN4_SDH,
	CLK_TYPE_GEN4_SD,
	CLK_TYPE_GEN4_MDSEL,	/* Select parent/divider using mode pin */
	CLK_TYPE_GEN4_Z,
	CLK_TYPE_GEN4_OSC,	/* OSC EXTAL predivider and fixed divider */
	CLK_TYPE_GEN4_RPCSRC,
	CLK_TYPE_GEN4_RPC,
	CLK_TYPE_GEN4_RPCD2,

	/* SoC specific definitions start here */
	CLK_TYPE_GEN3_SOC_BASE,
};

#define DEF_GEN3_SDH(_name, _id, _parent, _offset)	\
	DEF_BASE(_name, _id, CLK_TYPE_GEN3_SDH, _parent, .offset = _offset)

#define DEF_GEN3_SD(_name, _id, _parent, _offset)	\
	DEF_BASE(_name, _id, CLK_TYPE_GEN3_SD, _parent, .offset = _offset)

#define DEF_GEN3_MDSEL(_name, _id, _md, _parent0, _div0, _parent1, _div1) \
	DEF_BASE(_name, _id, CLK_TYPE_GEN3_MDSEL,	\
		 (_parent0) << 16 | (_parent1),		\
		 .div = (_div0) << 16 | (_div1), .offset = _md)

#define DEF_GEN3_PE(_name, _id, _parent_sscg, _div_sscg, _parent_clean, \
		    _div_clean) \
	DEF_GEN3_MDSEL(_name, _id, 12, _parent_sscg, _div_sscg,	\
		       _parent_clean, _div_clean)

#define DEF_GEN3_OSC(_name, _id, _parent, _div)		\
	DEF_BASE(_name, _id, CLK_TYPE_GEN3_OSC, _parent, .div = _div)

#define DEF_GEN3_RCKSEL(_name, _id, _parent0, _div0, _parent1, _div1) \
	DEF_BASE(_name, _id, CLK_TYPE_GEN3_RCKSEL,	\
		 (_parent0) << 16 | (_parent1),	.div = (_div0) << 16 | (_div1))

#define DEF_GEN3_Z(_name, _id, _type, _parent, _div, _offset)	\
	DEF_BASE(_name, _id, _type, _parent, .div = _div, .offset = _offset)

#define DEF_FIXED_RPCSRC_D3(_name, _id, _parent0, _parent1)	\
	DEF_BASE(_name, _id, CLK_TYPE_GEN3_D3_RPCSRC,	\
		 (_parent0) << 16 | (_parent1), .div = 5)

#define DEF_FIXED_RPCSRC_E3(_name, _id, _parent0, _parent1)	\
	DEF_BASE(_name, _id, CLK_TYPE_GEN3_E3_RPCSRC,	\
		 (_parent0) << 16 | (_parent1), .div = 8)

#define DEF_GEN4_SDH(_name, _id, _parent, _offset)	\
	DEF_BASE(_name, _id, CLK_TYPE_GEN4_SDH, _parent, .offset = _offset)

#define DEF_GEN4_SD(_name, _id, _parent, _offset)	\
	DEF_BASE(_name, _id, CLK_TYPE_GEN4_SD, _parent, .offset = _offset)

#define DEF_GEN4_MDSEL(_name, _id, _md, _parent0, _div0, _parent1, _div1) \
	DEF_BASE(_name, _id, CLK_TYPE_GEN4_MDSEL,	\
		 (_parent0) << 16 | (_parent1),		\
		 .div = (_div0) << 16 | (_div1), .offset = _md)

#define DEF_GEN4_OSC(_name, _id, _parent, _div)		\
	DEF_BASE(_name, _id, CLK_TYPE_GEN4_OSC, _parent, .div = _div)

#define DEF_GEN4_Z(_name, _id, _type, _parent, _div, _offset)	\
	DEF_BASE(_name, _id, _type, _parent, .div = _div, .offset = _offset)

struct rcar_gen3_cpg_pll_config {
	u8 extal_div;
	u8 pll1_mult;
	u8 pll1_div;
	u8 pll3_mult;
	u8 pll3_div;
	u8 osc_prediv;
};

struct rcar_gen4_cpg_pll_config {
	u8 extal_div;
	u8 pll1_mult;
	u8 pll1_div;
	u8 pll2_mult;
	u8 pll2_div;
	u8 pll3_mult;
	u8 pll3_div;
	u8 pll4_mult;
	u8 pll4_div;
	u8 pll5_mult;
	u8 pll5_div;
	u8 pll6_mult;
	u8 pll6_div;
	u8 osc_prediv;
	u8 pll7_mult;
	u8 pll7_div;
};

#define CPG_RST_MODEMR	0x060
#define CPG_RST_MODEMR0	0x000

#define CPG_SDCKCR_STPnHCK		BIT(9)
#define CPG_SDCKCR_STPnCK		BIT(8)
#define CPG_SDCKCR_SRCFC_MASK		GENMASK(4, 2)
#define CPG_SDCKCR_FC_MASK		GENMASK(1, 0)
/* V3M specifics */
#define CPG_SDCKCR_SDHFC_MASK		GENMASK(11, 8)
#define CPG_SDCKCR_SD0FC_MASK		GENMASK(7, 4)

#define CPG_RPCCKCR	0x238
#define CPG_RPCCKCR_DIV_POST_MASK	GENMASK(4, 3)
#define CPG_RPCCKCR_DIV_PRE_MASK	GENMASK(2, 0)

#define CPG_RCKCR	0x240

struct gen3_clk_priv {
	void __iomem		*base;
	struct cpg_mssr_info	*info;
	struct clk		clk_extal;
	struct clk		clk_extalr;
	u32			cpg_mode;
	union {
		const struct rcar_gen3_cpg_pll_config *gen3_cpg_pll_config;
		const struct rcar_gen4_cpg_pll_config *gen4_cpg_pll_config;
	};
};

int gen3_cpg_bind(struct udevice *parent);

extern const struct clk_ops gen3_clk_ops;

#endif
