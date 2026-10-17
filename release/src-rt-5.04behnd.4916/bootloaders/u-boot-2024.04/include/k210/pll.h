/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019-20 Sean Anderson <seanga2@gmail.com>
 */
#ifndef K210_PLL_H
#define K210_PLL_H

#include <test/export.h>

struct k210_pll_config {
	u8 r;
	u8 f;
	u8 od;
};

#ifdef CONFIG_UNIT_TEST
TEST_STATIC int k210_pll_calc_config(u32 rate, u32 rate_in,
				     struct k210_pll_config *best);

#endif
#endif /* K210_PLL_H */
