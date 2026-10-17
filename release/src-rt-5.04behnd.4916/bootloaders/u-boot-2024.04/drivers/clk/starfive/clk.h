/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022 StarFive Technology Co., Ltd.
 * Author:	Yanhong Wang <yanhong.wang@starfivetech.com>
 */

#ifndef __CLK_STARFIVE_H
#define __CLK_STARFIVE_H

/* the number of fixed clocks in DTS */
#define JH7110_EXTCLK_END	12

#define _JH7110_CLK_OPS(_name)					\
static const struct clk_ops jh7110_##_name##_clk_ops = {	\
	.set_rate = ccf_clk_set_rate,				\
	.get_rate = ccf_clk_get_rate,				\
	.set_parent = ccf_clk_set_parent,			\
	.enable = ccf_clk_enable,				\
	.disable = ccf_clk_disable,				\
	.of_xlate = jh7110_##_name##_clk_of_xlate,		\
}

#define JH7110_CLK_OPS(name) _JH7110_CLK_OPS(name)

#endif
