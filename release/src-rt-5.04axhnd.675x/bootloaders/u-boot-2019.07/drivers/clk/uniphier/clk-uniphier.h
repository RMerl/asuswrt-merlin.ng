/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#ifndef __CLK_UNIPHIER_H__
#define __CLK_UNIPHIER_H__

#include <linux/kernel.h>
#include <linux/types.h>

#define UNIPHIER_CLK_MUX_MAX_PARENTS		8

#define UNIPHIER_CLK_TYPE_END			0
#define UNIPHIER_CLK_TYPE_FIXED_RATE		2
#define UNIPHIER_CLK_TYPE_GATE			3
#define UNIPHIER_CLK_TYPE_MUX			4

#define UNIPHIER_CLK_ID_INVALID			(U8_MAX)

struct uniphier_clk_fixed_rate_data {
	unsigned long fixed_rate;
};

struct uniphier_clk_gate_data {
	u8 parent_id;
	u16 reg;
	u8 bit;
};

struct uniphier_clk_mux_data {
	u8 parent_ids[UNIPHIER_CLK_MUX_MAX_PARENTS];
	u8 num_parents;
	u16 reg;
	u32 masks[UNIPHIER_CLK_MUX_MAX_PARENTS];
	u32 vals[UNIPHIER_CLK_MUX_MAX_PARENTS];
};

struct uniphier_clk_data {
	u8 type;
	u8 id;
	union {
		struct uniphier_clk_fixed_rate_data rate;
		struct uniphier_clk_gate_data gate;
		struct uniphier_clk_mux_data mux;
	} data;
};

#define UNIPHIER_CLK_RATE(_id, _rate)				\
	{							\
		.type = UNIPHIER_CLK_TYPE_FIXED_RATE,		\
		.id = (_id),					\
		.data.rate = {					\
			.fixed_rate = (_rate),			\
		},						\
	}

#define UNIPHIER_CLK_GATE(_id, _parent, _reg, _bit)		\
	{							\
		.type = UNIPHIER_CLK_TYPE_GATE,			\
		.id = (_id),					\
		.data.gate = {					\
			.parent_id = (_parent),			\
			.reg = (_reg),				\
			.bit = (_bit),				\
		},						\
	}

#define UNIPHIER_CLK_GATE_SIMPLE(_id, _reg, _bit)		\
	UNIPHIER_CLK_GATE(_id, UNIPHIER_CLK_ID_INVALID, _reg, _bit)

extern const struct uniphier_clk_data uniphier_pxs2_sys_clk_data[];
extern const struct uniphier_clk_data uniphier_ld20_sys_clk_data[];
extern const struct uniphier_clk_data uniphier_pxs3_sys_clk_data[];
extern const struct uniphier_clk_data uniphier_mio_clk_data[];

#endif /* __CLK_UNIPHIER_H__ */
