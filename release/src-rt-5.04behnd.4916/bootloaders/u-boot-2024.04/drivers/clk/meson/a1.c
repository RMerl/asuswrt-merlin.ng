// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2023 SberDevices, Inc.
 * Author: Igor Prusov <ivprusov@salutedevices.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <regmap.h>
#include <asm/arch/clock-a1.h>
#include <dt-bindings/clock/amlogic,a1-pll-clkc.h>
#include <dt-bindings/clock/amlogic,a1-peripherals-clkc.h>
#include "clk_meson.h"

/*
 * This driver supports both PLL and peripherals clock sources.
 * Following operations are supported:
 * - calculating clock frequency on a limited tree
 * - reading muxes and dividers
 * - enabling/disabling gates without propagation
 * - reparenting without rate propagation, only on muxes
 * - setting rates with limited reparenting, only on dividers with mux parent
 */

#define NR_CLKS				154
#define NR_PLL_CLKS			11

/* External clock IDs. Those should not overlap with regular IDs */
#define EXTERNAL_XTAL			(NR_CLKS + 0)
#define EXTERNAL_FCLK_DIV2		(NR_CLKS + 1)
#define EXTERNAL_FCLK_DIV3		(NR_CLKS + 2)
#define EXTERNAL_FCLK_DIV5		(NR_CLKS + 3)
#define EXTERNAL_FCLK_DIV7		(NR_CLKS + 4)

#define EXTERNAL_FIXPLL_IN		(NR_PLL_CLKS + 1)

#define SET_PARM_VALUE(_priv, _parm, _val)				\
	regmap_update_bits((_priv)->map, (_parm)->reg_off,		\
			   SETPMASK((_parm)->width, (_parm)->shift),	\
			   (_val) << (_parm)->shift)

#define GET_PARM_VALUE(_priv, _parm)					\
({									\
	uint _reg;							\
	regmap_read((_priv)->map, (_parm)->reg_off, &_reg);		\
	PARM_GET((_parm)->width, (_parm)->shift, _reg);			\
})

struct meson_clk {
	struct regmap *map;
};

/**
 * enum meson_clk_type - The type of clock
 * @MESON_CLK_ANY: Special value that matches any clock type
 * @MESON_CLK_GATE: This clock is a gate
 * @MESON_CLK_MUX: This clock is a multiplexer
 * @MESON_CLK_DIV: This clock is a configurable divider
 * @MESON_CLK_FIXED_DIV: This clock is a configurable divider
 * @MESON_CLK_EXTERNAL: This is an external clock from different clock provider
 * @MESON_CLK_PLL: This is a PLL
 */
enum meson_clk_type {
	MESON_CLK_ANY = 0,
	MESON_CLK_GATE,
	MESON_CLK_MUX,
	MESON_CLK_DIV,
	MESON_CLK_FIXED_DIV,
	MESON_CLK_EXTERNAL,
	MESON_CLK_PLL,
};

/**
 * struct meson_clk_info - The parameters defining a clock
 * @name: Name of the clock
 * @parm: Register bits description for muxes and dividers
 * @div: Fixed divider value
 * @parents: List of parent clock IDs
 * @type: Clock type
 */
struct meson_clk_info {
	const char *name;
	union {
		const struct parm *parm;
		u8 div;
	};
	const unsigned int *parents;
	const enum meson_clk_type type;
};

/**
 * struct meson_clk_data - Clocks supported by clock provider
 * @num_clocks: Number of clocks
 * @clocks: Array of clock descriptions
 *
 */
struct meson_clk_data {
	const u8 num_clocks;
	const struct meson_clk_info **clocks;
};

/* Clock description initialization macros */

/* A multiplexer */
#define CLK_MUX(_name, _reg, _shift, _width, ...)			\
	(&(struct meson_clk_info){					\
		.parents = (const unsigned int[])__VA_ARGS__,		\
		.parm = &(struct parm) {				\
			.reg_off = (_reg),				\
			.shift = (_shift),				\
			.width = (_width),				\
		},							\
		.name = (_name),					\
		.type = MESON_CLK_MUX,					\
	})

/* A divider with an integral divisor */
#define CLK_DIV(_name, _reg, _shift, _width, _parent)			\
	(&(struct meson_clk_info){					\
		.parents = (const unsigned int[]) { (_parent) },	\
		.parm = &(struct parm) {				\
			.reg_off = (_reg),				\
			.shift = (_shift),				\
			.width = (_width),				\
		},							\
		.name = (_name),					\
		.type = MESON_CLK_DIV,					\
	})

/* A fixed divider */
#define CLK_DIV_FIXED(_name, _div, _parent)				\
	(&(struct meson_clk_info){					\
		.parents = (const unsigned int[]) { (_parent) },	\
		.div = (_div),						\
		.name = (_name),					\
		.type = MESON_CLK_FIXED_DIV,				\
	})

/* An external clock */
#define CLK_EXTERNAL(_name)						\
	(&(struct meson_clk_info){					\
		.name = (_name),					\
		.parents = (const unsigned int[]) { -ENOENT },		\
		.type = MESON_CLK_EXTERNAL,				\
	})

/* A clock gate */
#define CLK_GATE(_name, _reg, _shift, _parent)				\
	(&(struct meson_clk_info){					\
		.parents = (const unsigned int[]) { (_parent) },	\
		.parm = &(struct parm) {				\
			.reg_off = (_reg),				\
			.shift = (_shift),				\
			.width = 1,					\
		},							\
		.name = (_name),					\
		.type = MESON_CLK_GATE,					\
	})

/* A PLL clock */
#define CLK_PLL(_name, _parent, ...)					\
	(&(struct meson_clk_info){					\
		.name = (_name),					\
		.parents = (const unsigned int[]) { (_parent) },	\
		.parm = (const struct parm[])__VA_ARGS__,		\
		.type = MESON_CLK_PLL,					\
	})

/* A1 peripherals clocks */
static const struct meson_clk_info *meson_clocks[] = {
	[CLKID_SPIFC_SEL] = CLK_MUX("spifc_sel", A1_SPIFC_CLK_CTRL, 9, 2, {
		EXTERNAL_FCLK_DIV2,
		EXTERNAL_FCLK_DIV3,
		EXTERNAL_FCLK_DIV5,
		-ENOENT,
	}),
	[CLKID_SPIFC_SEL2] = CLK_MUX("spifc_sel2", A1_SPIFC_CLK_CTRL, 15, 1, {
		CLKID_SPIFC_DIV,
		EXTERNAL_XTAL,
	}),
	[CLKID_USB_BUS_SEL] = CLK_MUX("usb_bus_sel", A1_USB_BUSCLK_CTRL, 9, 2, {
		-ENOENT,
		CLKID_SYS,
		EXTERNAL_FCLK_DIV3,
		EXTERNAL_FCLK_DIV5,
	}),
	[CLKID_SYS] = CLK_MUX("sys", A1_SYS_CLK_CTRL0, 31, 1, {
		CLKID_SYS_A,
		CLKID_SYS_B,
	}),
	[CLKID_SYS_A_SEL] = CLK_MUX("sys_a_sel", A1_SYS_CLK_CTRL0, 10, 3, {
		-ENOENT,
		EXTERNAL_FCLK_DIV2,
		EXTERNAL_FCLK_DIV3,
		EXTERNAL_FCLK_DIV5,
		-ENOENT,
		-ENOENT,
		-ENOENT,
		-ENOENT,
	}),
	[CLKID_SYS_B_SEL] = CLK_MUX("sys_b_sel", A1_SYS_CLK_CTRL0, 26, 3, {
		-ENOENT,
		EXTERNAL_FCLK_DIV2,
		EXTERNAL_FCLK_DIV3,
		EXTERNAL_FCLK_DIV5,
		-ENOENT,
		-ENOENT,
		-ENOENT,
		-ENOENT,
	}),

	[CLKID_SPIFC_DIV] = CLK_DIV("spifc_div", A1_SPIFC_CLK_CTRL, 0, 8,
		CLKID_SPIFC_SEL
	),
	[CLKID_USB_BUS_DIV] = CLK_DIV("usb_bus_div", A1_USB_BUSCLK_CTRL, 0, 8,
		CLKID_USB_BUS_SEL
	),
	[CLKID_SYS_A_DIV] = CLK_DIV("sys_a_div", A1_SYS_CLK_CTRL0, 0, 10,
		CLKID_SYS_A_SEL
	),
	[CLKID_SYS_B_DIV] = CLK_DIV("sys_b_div", A1_SYS_CLK_CTRL0, 16, 10,
		CLKID_SYS_B_SEL
	),

	[CLKID_SPIFC] = CLK_GATE("spifc", A1_SPIFC_CLK_CTRL, 8,
		CLKID_SPIFC_SEL2
	),
	[CLKID_USB_BUS] = CLK_GATE("usb_bus", A1_USB_BUSCLK_CTRL, 8,
		CLKID_USB_BUS_DIV
	),
	[CLKID_SYS_A] = CLK_GATE("sys_a", A1_SYS_CLK_CTRL0, 13,
		CLKID_SYS_A_DIV
	),
	[CLKID_SYS_B] = CLK_GATE("sys_b", A1_SYS_CLK_CTRL0, 29,
		CLKID_SYS_B_DIV
	),
	[CLKID_FIXPLL_IN] = CLK_GATE("fixpll_in", A1_SYS_OSCIN_CTRL, 1,
		EXTERNAL_XTAL
	),
	[CLKID_USB_PHY_IN] = CLK_GATE("usb_phy_in", A1_SYS_OSCIN_CTRL, 2,
		EXTERNAL_XTAL
	),
	[CLKID_USB_CTRL_IN] = CLK_GATE("usb_ctrl_in", A1_SYS_OSCIN_CTRL, 3,
		EXTERNAL_XTAL
	),
	[CLKID_USB_CTRL] = CLK_GATE("usb_ctrl", A1_SYS_CLK_EN0, 28,
		CLKID_SYS
	),
	[CLKID_USB_PHY] = CLK_GATE("usb_phy", A1_SYS_CLK_EN0, 27,
		CLKID_SYS
	),
	[CLKID_SARADC] = CLK_GATE("saradc", A1_SAR_ADC_CLK_CTR, 8,
		-ENOENT
	),
	[CLKID_SARADC_EN] = CLK_GATE("saradc_en", A1_SYS_CLK_EN0, 13,
		CLKID_SYS
	),

	[EXTERNAL_XTAL] = CLK_EXTERNAL("xtal"),
	[EXTERNAL_FCLK_DIV2] = CLK_EXTERNAL("fclk_div2"),
	[EXTERNAL_FCLK_DIV3] = CLK_EXTERNAL("fclk_div3"),
	[EXTERNAL_FCLK_DIV5] = CLK_EXTERNAL("fclk_div5"),
	[EXTERNAL_FCLK_DIV7] = CLK_EXTERNAL("fclk_div7"),
};

/* A1 PLL clocks */
static const struct meson_clk_info *meson_pll_clocks[] = {
	[EXTERNAL_FIXPLL_IN] = CLK_EXTERNAL("fixpll_in"),

	[CLKID_FIXED_PLL_DCO] = CLK_PLL("fixed_pll_dco", EXTERNAL_FIXPLL_IN, {
			{A1_ANACTRL_FIXPLL_CTRL0, 0, 8},
			{A1_ANACTRL_FIXPLL_CTRL0, 10, 5},
	}),

	[CLKID_FCLK_DIV2_DIV] = CLK_DIV_FIXED("fclk_div2_div", 2,
		CLKID_FIXED_PLL
	),
	[CLKID_FCLK_DIV3_DIV] = CLK_DIV_FIXED("fclk_div3_div", 3,
		CLKID_FIXED_PLL
	),
	[CLKID_FCLK_DIV5_DIV] = CLK_DIV_FIXED("fclk_div5_div", 5,
		CLKID_FIXED_PLL
	),
	[CLKID_FCLK_DIV7_DIV] = CLK_DIV_FIXED("fclk_div7_div", 7,
		CLKID_FIXED_PLL
	),

	[CLKID_FIXED_PLL] = CLK_GATE("fixed_pll", A1_ANACTRL_FIXPLL_CTRL0, 20,
		CLKID_FIXED_PLL_DCO
	),
	[CLKID_FCLK_DIV2] = CLK_GATE("fclk_div2", A1_ANACTRL_FIXPLL_CTRL0, 21,
		CLKID_FCLK_DIV2_DIV
	),
	[CLKID_FCLK_DIV3] = CLK_GATE("fclk_div3", A1_ANACTRL_FIXPLL_CTRL0, 22,
		CLKID_FCLK_DIV3_DIV
	),
	[CLKID_FCLK_DIV5] = CLK_GATE("fclk_div5", A1_ANACTRL_FIXPLL_CTRL0, 23,
		CLKID_FCLK_DIV5_DIV
	),
	[CLKID_FCLK_DIV7] = CLK_GATE("fclk_div7", A1_ANACTRL_FIXPLL_CTRL0, 24,
		CLKID_FCLK_DIV7_DIV
	),
};

static const struct meson_clk_info *meson_clk_get_info(struct clk *clk, ulong id,
						       enum meson_clk_type type)
{
	struct meson_clk_data *data;
	const struct meson_clk_info *info;

	data = (struct meson_clk_data *)dev_get_driver_data(clk->dev);
	if (id >= data->num_clocks)
		return ERR_PTR(-EINVAL);

	info = data->clocks[id];
	if (!info)
		return ERR_PTR(-ENOENT);

	if (type != MESON_CLK_ANY && type != info->type)
		return ERR_PTR(-EINVAL);

	return info;
}

static ulong meson_clk_get_rate_by_id(struct clk *clk, unsigned long id);

static int meson_set_gate(struct clk *clk, bool on)
{
	struct meson_clk *priv = dev_get_priv(clk->dev);
	const struct meson_clk_info *info;

	debug("%s: %sabling %lu\n", __func__, on ? "en" : "dis", clk->id);

	info = meson_clk_get_info(clk, clk->id, MESON_CLK_ANY);
	if (IS_ERR(info))
		return PTR_ERR(info);

	SET_PARM_VALUE(priv, info->parm, on);

	return 0;
}

static int meson_clk_enable(struct clk *clk)
{
	return meson_set_gate(clk, true);
}

static int meson_clk_disable(struct clk *clk)
{
	return meson_set_gate(clk, false);
}

static ulong meson_div_get_rate(struct clk *clk, unsigned long id)
{
	struct meson_clk *priv = dev_get_priv(clk->dev);
	u16 n;
	ulong rate;
	const struct meson_clk_info *info;

	info = meson_clk_get_info(clk, id, MESON_CLK_DIV);
	if (IS_ERR(info))
		return PTR_ERR(info);

	/* Actual divider value is (field value + 1), hence the increment */
	n = GET_PARM_VALUE(priv, info->parm) + 1;

	rate = meson_clk_get_rate_by_id(clk, info->parents[0]);

	return rate / n;
}

static int meson_clk_get_parent(struct clk *clk, unsigned long id)
{
	uint reg = 0;
	struct meson_clk *priv = dev_get_priv(clk->dev);
	const struct meson_clk_info *info;

	info = meson_clk_get_info(clk, id, MESON_CLK_ANY);
	if (IS_ERR(info))
		return PTR_ERR(info);

	/* For muxes we read currently selected parent from register,
	 * for other types there is always only one element in parents array.
	 */
	if (info->type == MESON_CLK_MUX) {
		reg = GET_PARM_VALUE(priv, info->parm);
		if (IS_ERR_VALUE(reg))
			return reg;
	}

	return info->parents[reg];
}

static ulong meson_pll_get_rate(struct clk *clk, unsigned long id)
{
	struct meson_clk *priv = dev_get_priv(clk->dev);
	const struct meson_clk_info *info;
	const struct parm *pm, *pn;
	ulong parent_rate_mhz;
	unsigned int parent;
	u16 n, m;

	info = meson_clk_get_info(clk, id, MESON_CLK_ANY);
	if (IS_ERR(info))
		return PTR_ERR(info);

	pm = &info->parm[0];
	pn = &info->parm[1];

	n = GET_PARM_VALUE(priv, pn);
	m = GET_PARM_VALUE(priv, pm);

	if (n == 0)
		return -EINVAL;

	parent = info->parents[0];
	parent_rate_mhz = meson_clk_get_rate_by_id(clk, parent) / 1000000;

	return parent_rate_mhz * m / n * 1000000;
}

static ulong meson_clk_get_rate_by_id(struct clk *clk, unsigned long id)
{
	ulong rate, parent;
	const struct meson_clk_info *info;

	if (IS_ERR_VALUE(id))
		return id;

	info = meson_clk_get_info(clk, id, MESON_CLK_ANY);
	if (IS_ERR(info))
		return PTR_ERR(info);

	switch (info->type) {
	case MESON_CLK_PLL:
		rate = meson_pll_get_rate(clk, id);
		break;
	case MESON_CLK_GATE:
	case MESON_CLK_MUX:
		parent = meson_clk_get_parent(clk, id);
		rate = meson_clk_get_rate_by_id(clk, parent);
		break;
	case MESON_CLK_DIV:
		rate = meson_div_get_rate(clk, id);
		break;
	case MESON_CLK_FIXED_DIV:
		parent = meson_clk_get_parent(clk, id);
		rate = meson_clk_get_rate_by_id(clk, parent) / info->div;
		break;
	case MESON_CLK_EXTERNAL: {
		int ret;
		struct clk external_clk;

		ret = clk_get_by_name(clk->dev, info->name, &external_clk);
		if (ret)
			return ret;

		rate = clk_get_rate(&external_clk);
		break;
	}
	default:
		rate = -EINVAL;
		break;
	}

	return rate;
}

static ulong meson_clk_get_rate(struct clk *clk)
{
	return meson_clk_get_rate_by_id(clk, clk->id);
}

/* This implements rate propagation for dividers placed after multiplexer:
 *  ---------|\
 *     ..... | |---DIV--
 *  ---------|/
 */
static ulong meson_composite_set_rate(struct clk *clk, ulong id, ulong rate)
{
	unsigned int i, best_div_val;
	unsigned long best_delta, best_parent;
	const struct meson_clk_info *div;
	const struct meson_clk_info *mux;
	struct meson_clk *priv = dev_get_priv(clk->dev);

	div = meson_clk_get_info(clk, id, MESON_CLK_DIV);
	if (IS_ERR(div))
		return PTR_ERR(div);

	mux = meson_clk_get_info(clk, div->parents[0], MESON_CLK_MUX);
	if (IS_ERR(mux))
		return PTR_ERR(mux);

	best_parent = -EINVAL;
	best_delta = ULONG_MAX;
	for (i = 0; i < (1 << mux->parm->width); i++) {
		unsigned long parent_rate, delta;
		unsigned int div_val;

		parent_rate = meson_clk_get_rate_by_id(clk, mux->parents[i]);
		if (IS_ERR_VALUE(parent_rate))
			continue;

		/* If overflow, try to use max divider value */
		div_val = min(DIV_ROUND_CLOSEST(parent_rate, rate),
			      (1UL << div->parm->width));

		delta = abs(rate - (parent_rate / div_val));
		if (delta < best_delta) {
			best_delta = delta;
			best_div_val = div_val;
			best_parent = i;
		}
	}

	if (IS_ERR_VALUE(best_parent))
		return best_parent;

	SET_PARM_VALUE(priv, mux->parm, best_parent);
	/* Divider is set to (field value + 1), hence the decrement */
	SET_PARM_VALUE(priv, div->parm, best_div_val - 1);

	return 0;
}

static ulong meson_clk_set_rate_by_id(struct clk *clk, unsigned int id, ulong rate);

static ulong meson_mux_set_rate(struct clk *clk, unsigned long id, ulong rate)
{
	int i;
	ulong ret = -EINVAL;
	struct meson_clk *priv = dev_get_priv(clk->dev);
	const struct meson_clk_info *info;

	info = meson_clk_get_info(clk, id, MESON_CLK_MUX);
	if (IS_ERR(info))
		return PTR_ERR(info);

	for (i = 0; i < (1 << info->parm->width); i++) {
		ret = meson_clk_set_rate_by_id(clk, info->parents[i], rate);
		if (!ret) {
			SET_PARM_VALUE(priv, info->parm, i);
			break;
		}
	}

	return ret;
}

/* Rate propagation is implemented for a subcection of a clock tree, that is
 * required at boot stage.
 */
static ulong meson_clk_set_rate_by_id(struct clk *clk, unsigned int id, ulong rate)
{
	switch (id) {
	case CLKID_SPIFC_DIV:
	case CLKID_USB_BUS_DIV:
		return meson_composite_set_rate(clk, id, rate);
	case CLKID_SPIFC:
	case CLKID_USB_BUS: {
		unsigned long parent = meson_clk_get_parent(clk, id);

		return meson_clk_set_rate_by_id(clk, parent, rate);
	}
	case CLKID_SPIFC_SEL2:
		return meson_mux_set_rate(clk, id, rate);
	}

	return -EINVAL;
}

static ulong meson_clk_set_rate(struct clk *clk, ulong rate)
{
	return meson_clk_set_rate_by_id(clk, clk->id, rate);
}

static int meson_mux_set_parent_by_id(struct clk *clk, unsigned int parent_id)
{
	unsigned int i, parent_index;
	struct meson_clk *priv = dev_get_priv(clk->dev);
	const struct meson_clk_info *info;

	info = meson_clk_get_info(clk, clk->id, MESON_CLK_MUX);
	if (IS_ERR(info))
		return PTR_ERR(info);

	parent_index = -EINVAL;
	for (i = 0; i < (1 << info->parm->width); i++) {
		if (parent_id == info->parents[i]) {
			parent_index = i;
			break;
		}
	}

	if (IS_ERR_VALUE(parent_index))
		return parent_index;

	SET_PARM_VALUE(priv, info->parm, parent_index);

	return 0;
}

static int meson_clk_set_parent(struct clk *clk, struct clk *parent_clk)
{
	return meson_mux_set_parent_by_id(clk, parent_clk->id);
}

static int meson_clk_probe(struct udevice *dev)
{
	struct meson_clk *priv = dev_get_priv(dev);

	return regmap_init_mem(dev_ofnode(dev), &priv->map);
}

struct meson_clk_data meson_a1_peripherals_info = {
	.clocks = meson_clocks,
	.num_clocks = ARRAY_SIZE(meson_clocks),
};

struct meson_clk_data meson_a1_pll_info = {
	.clocks = meson_pll_clocks,
	.num_clocks = ARRAY_SIZE(meson_pll_clocks),
};

static const struct udevice_id meson_clk_ids[] = {
	{
		.compatible = "amlogic,a1-peripherals-clkc",
		.data = (ulong)&meson_a1_peripherals_info,
	},
	{
		.compatible = "amlogic,a1-pll-clkc",
		.data = (ulong)&meson_a1_pll_info,
	},
	{ }
};

#if IS_ENABLED(CONFIG_CMD_CLK)
static const char *meson_clk_get_name(struct clk *clk, int id)
{
	const struct meson_clk_info *info;

	info = meson_clk_get_info(clk, id, MESON_CLK_ANY);

	return IS_ERR(info) ? "unknown" : info->name;
}

static int meson_clk_dump_single(struct clk *clk)
{
	const struct meson_clk_info *info;
	struct meson_clk *priv;
	unsigned long rate;
	char *state, frequency[80];
	int parent;

	priv = dev_get_priv(clk->dev);

	info = meson_clk_get_info(clk, clk->id, MESON_CLK_ANY);
	if (IS_ERR(info) || !info->name)
		return -EINVAL;

	rate = clk_get_rate(clk);
	if (IS_ERR_VALUE(rate))
		sprintf(frequency, "unknown");
	else
		sprintf(frequency, "%lu", rate);

	if (info->type == MESON_CLK_GATE)
		state = GET_PARM_VALUE(priv, info->parm) ? "enabled" : "disabled";
	else
		state = "N/A";

	parent = meson_clk_get_parent(clk, clk->id);
	printf("%15s%20s%20s%15s\n",
	       info->name,
	       frequency,
	       meson_clk_get_name(clk, parent),
	       state);

	return 0;
}

static void meson_clk_dump(struct udevice *dev)
{
	int i;
	struct meson_clk_data *data;
	const char *sep = "--------------------";

	printf("%s:\n", dev->name);
	printf("%.15s%s%s%.15s\n", sep, sep, sep, sep);
	printf("%15s%20s%20s%15s\n", "clk", "frequency", "parent", "state");
	printf("%.15s%s%s%.15s\n", sep, sep, sep, sep);

	data = (struct meson_clk_data *)dev_get_driver_data(dev);
	for (i = 0; i < data->num_clocks; i++) {
		meson_clk_dump_single(&(struct clk){
			.dev = dev,
			.id = i
		});
	}
}
#endif

static struct clk_ops meson_clk_ops = {
	.disable	= meson_clk_disable,
	.enable		= meson_clk_enable,
	.get_rate	= meson_clk_get_rate,
	.set_rate	= meson_clk_set_rate,
	.set_parent	= meson_clk_set_parent,
#if IS_ENABLED(CONFIG_CMD_CLK)
	.dump		= meson_clk_dump,
#endif
};

U_BOOT_DRIVER(meson_clk) = {
	.name		= "meson-clk-a1",
	.id		= UCLASS_CLK,
	.of_match	= meson_clk_ids,
	.priv_auto	= sizeof(struct meson_clk),
	.ops		= &meson_clk_ops,
	.probe		= meson_clk_probe,
};
