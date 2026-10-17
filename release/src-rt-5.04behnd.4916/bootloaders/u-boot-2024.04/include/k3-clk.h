/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2020-2021 Texas Instruments Incorporated - https://www.ti.com
 *      Tero Kristo <t-kristo@ti.com>
 */

#ifndef __K3_CLK_H__
#define __K3_CLK_H__

#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/clk-provider.h>
#include <linux/types.h>
#include <stdint.h>

struct dev_clk {
	int dev_id;
	int clk_id;
	const char *clk_name;
};

#define DEV_CLK(_dev_id, _clk_id, _clk_name) { .dev_id = _dev_id,		\
					.clk_id = _clk_id, .clk_name = _clk_name, }

#define CLK_TYPE_MUX		0x01
#define CLK_TYPE_DIV		0x02
#define CLK_TYPE_PLL		0x03
#define CLK_TYPE_HFOSC		0x04
#define CLK_TYPE_POSTDIV	0x05
#define CLK_TYPE_MUX_PLLCTRL	0x06
#define CLK_TYPE_FIXED_RATE	0x07

struct pll_data {
	u32 reg;
	const char *name;
	const char *parent;
	u32 flags;
};

struct mux_data {
	u32 reg;
	const char *name;
	const char * const *parents;
	int num_parents;
	u32 flags;
	int shift;
	int width;
};

struct div_data {
	u32 reg;
	const char *name;
	const char *parent;
	u32 flags;
	int shift;
	int width;
	u32 div_flags;
};

struct hfosc_data {
	const char *name;
	u32 flags;
};

struct fixed_rate_data {
	const char *name;
	u64 rate;
	u32 flags;
};

struct postdiv_data {
	const char *name;
	const char *parent;
	int width;
	u32 flags;
};

struct mux_pllctrl_data {
	u32 reg;
	const char *name;
	const char * const *parents;
	int num_parents;
	u32 flags;
};

struct clk_data {
	int type;
	u32 default_freq;
	union {
		struct pll_data pll;
		struct mux_data mux;
		struct div_data div;
		struct hfosc_data hfosc;
		struct postdiv_data postdiv;
		struct mux_pllctrl_data mux_pllctrl;
		struct fixed_rate_data fixed_rate;
	} clk;
};

#define CLK_MUX(_name, _parents, _num_parents, _reg, _shift, _width, _flags) \
	{								\
		.type = CLK_TYPE_MUX,					\
		.clk.mux = { .name = _name, .parents = _parents,	\
		.reg = _reg,						\
		.num_parents = _num_parents, .shift = _shift,		\
		.width = _width, .flags = _flags }			\
	}

#define CLK_DIV(_name, _parent, _reg, _shift, _width, _flags, _div_flags)	\
	{								\
		.type = CLK_TYPE_DIV,					\
		.clk.div = {						\
			.name = _name, .parent = _parent, .reg = _reg,	\
			.shift = _shift, .width = _width,		\
			.flags = _flags, .div_flags = _div_flags }	\
	}

#define CLK_DIV_DEFFREQ(_name, _parent, _reg, _shift, _width, _flags, _div_flags, _freq) \
	{								\
		.type = CLK_TYPE_DIV,					\
		.default_freq = _freq,					\
		.clk.div = {						\
			.name = _name, .parent = _parent, .reg = _reg,	\
			.shift = _shift, .width = _width,		\
			.flags = _flags, .div_flags = _div_flags }	\
	}

#define CLK_PLL(_name, _parent, _reg,  _flags)	\
	{					\
		.type = CLK_TYPE_PLL,		\
		.clk.pll = {.name = _name, .parent = _parent, .reg = _reg, .flags = _flags } \
	}

#define CLK_PLL_DEFFREQ(_name, _parent, _reg, _flags, _freq)	\
	{							\
		.type = CLK_TYPE_PLL,				\
		.default_freq = _freq,				\
		.clk.pll = { .name = _name, .parent = _parent,	\
				.reg = _reg, .flags = _flags }	\
	}

#define CLK_HFOSC(_name, _flags)		\
	{					\
		.type = CLK_TYPE_HFOSC,		\
		.clk.hfosc = { .name = _name, .flags = _flags } \
	}

#define CLK_FIXED_RATE(_name, _rate, _flags)		\
	{						\
		.type = CLK_TYPE_FIXED_RATE,		\
		.clk.fixed_rate = { .name = _name, .rate = _rate, .flags = _flags } \
	}

#define CLK_POSTDIV(_name, _parent, _width, _flags)	\
	{						\
		.type = CLK_TYPE_POSTDIV,		\
		.clk.postdiv = {.name = _name, .parent = _parent, .width = _width, .flags = _flags } \
	}

#define CLK_MUX_PLLCTRL(_name, _parents, _num_parents, _reg, _flags)	\
	{								\
		.type = CLK_TYPE_MUX,					\
		.clk.mux_pllctrl = { .name = _name, .parents = _parents,\
		.num_parents = _num_parents, .flags = _flags }		\
	}

struct ti_k3_clk_platdata {
	const struct clk_data *clk_list;
	int clk_list_cnt;
	const struct dev_clk *soc_dev_clk_data;
	int soc_dev_clk_data_cnt;
};

extern const struct ti_k3_clk_platdata j721e_clk_platdata;
extern const struct ti_k3_clk_platdata j7200_clk_platdata;
extern const struct ti_k3_clk_platdata j721s2_clk_platdata;
extern const struct ti_k3_clk_platdata am62x_clk_platdata;
extern const struct ti_k3_clk_platdata am62ax_clk_platdata;

struct clk *clk_register_ti_pll(const char *name, const char *parent_name,
				void __iomem *reg);

#endif /* __K3_CLK_H__ */
