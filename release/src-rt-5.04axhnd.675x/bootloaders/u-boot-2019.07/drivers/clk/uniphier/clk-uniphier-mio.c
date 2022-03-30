// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include "clk-uniphier.h"

#define UNIPHIER_MIO_CLK_SD_FIXED					\
	UNIPHIER_CLK_RATE(128, 44444444),				\
	UNIPHIER_CLK_RATE(129, 33333333),				\
	UNIPHIER_CLK_RATE(130, 50000000),				\
	UNIPHIER_CLK_RATE(131, 66666667),				\
	UNIPHIER_CLK_RATE(132, 100000000),				\
	UNIPHIER_CLK_RATE(133, 40000000),				\
	UNIPHIER_CLK_RATE(134, 25000000),				\
	UNIPHIER_CLK_RATE(135, 22222222)

#define UNIPHIER_MIO_CLK_SD(_id, ch)					\
	{								\
		.type = UNIPHIER_CLK_TYPE_MUX,				\
		.id = (_id) + 32,					\
		.data.mux = {						\
			.parent_ids = {					\
				128,					\
				129,					\
				130,					\
				131,					\
				132,					\
				133,					\
				134,					\
				135,					\
			},						\
			.num_parents = 8,				\
			.reg = 0x30 + 0x200 * (ch),			\
			.masks = {					\
				0x00031000,				\
				0x00031000,				\
				0x00031000,				\
				0x00031000,				\
				0x00001300,				\
				0x00001300,				\
				0x00001300,				\
				0x00001300,				\
			},						\
			.vals = {					\
				0x00000000,				\
				0x00010000,				\
				0x00020000,				\
				0x00030000,				\
				0x00001000,				\
				0x00001100,				\
				0x00001200,				\
				0x00001300,				\
			},						\
		},							\
	},								\
	UNIPHIER_CLK_GATE((_id), (_id) + 32, 0x20 + 0x200 * (ch), 8)

#define UNIPHIER_MIO_CLK_USB2(id, ch)					\
	UNIPHIER_CLK_GATE_SIMPLE((id), 0x20 + 0x200 * (ch), 28)

#define UNIPHIER_MIO_CLK_USB2_PHY(id, ch)				\
	UNIPHIER_CLK_GATE_SIMPLE((id), 0x20 + 0x200 * (ch), 29)

#define UNIPHIER_MIO_CLK_DMAC(id)					\
	UNIPHIER_CLK_GATE_SIMPLE((id), 0x20, 25)

const struct uniphier_clk_data uniphier_mio_clk_data[] = {
	UNIPHIER_MIO_CLK_SD_FIXED,
	UNIPHIER_MIO_CLK_SD(0, 0),
	UNIPHIER_MIO_CLK_SD(1, 1),
	UNIPHIER_MIO_CLK_SD(2, 2),
	UNIPHIER_MIO_CLK_DMAC(7),
	UNIPHIER_MIO_CLK_USB2(8, 0),
	UNIPHIER_MIO_CLK_USB2(9, 1),
	UNIPHIER_MIO_CLK_USB2(10, 2),
	UNIPHIER_MIO_CLK_USB2_PHY(12, 0),
	UNIPHIER_MIO_CLK_USB2_PHY(13, 1),
	UNIPHIER_MIO_CLK_USB2_PHY(14, 2),
	{ /* sentinel */ }
};
