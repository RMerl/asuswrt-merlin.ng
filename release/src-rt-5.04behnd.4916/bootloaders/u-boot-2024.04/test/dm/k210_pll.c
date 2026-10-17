// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Sean Anderson <seanga2@gmail.com>
 */

#include <common.h>
/* For DIV_ROUND_DOWN_ULL, defined in linux/kernel.h */
#include <div64.h>
#include <dm/test.h>
#include <k210/pll.h>
#include <test/ut.h>

static int dm_test_k210_pll_calc_config(u32 rate, u32 rate_in,
					struct k210_pll_config *best)
{
	u64 f, r, od, max_r, inv_ratio;
	s64 error, best_error;

	best_error = S64_MAX;
	error = best_error;
	max_r = min(16ULL, DIV_ROUND_DOWN_ULL(rate_in, 13300000));
	inv_ratio = DIV_ROUND_CLOSEST_ULL((u64)rate_in << 32, rate);

	/* Brute force it */
	for (r = 1; r <= max_r; r++) {
		for (f = 1; f <= 64; f++) {
			for (od = 1; od <= 16; od++) {
				u64 vco = DIV_ROUND_CLOSEST_ULL(rate_in * f, r);

				if (vco > 1750000000 || vco < 340000000)
					continue;

				error = DIV_ROUND_CLOSEST_ULL(f * inv_ratio,
							      r * od);
				/* The lower 16 bits are spurious */
				error = abs64((error - BIT_ULL(32))) >> 16;
				if (error < best_error) {
					best->r = r;
					best->f = f;
					best->od = od;
					best_error = error;
				}
			}
		}
	}

	if (best_error == S64_MAX)
		return -EINVAL;
	return 0;
}

static int dm_test_k210_pll_compare(struct k210_pll_config *ours,
				    struct k210_pll_config *theirs)
{
	return (u32)ours->f * theirs->r * theirs->od !=
	       (u32)theirs->f * ours->r * ours->od;
}

static int dm_test_k210_pll(struct unit_test_state *uts)
{
	struct k210_pll_config ours, theirs;

	/* General range checks */
	ut_asserteq(-EINVAL, k210_pll_calc_config(0, 26000000, &theirs));
	ut_asserteq(-EINVAL, k210_pll_calc_config(390000000, 0, &theirs));
	ut_asserteq(-EINVAL, k210_pll_calc_config(2000000000, 26000000,
						  &theirs));
	ut_asserteq(-EINVAL, k210_pll_calc_config(390000000, 2000000000,
						  &theirs));
	ut_asserteq(-EINVAL, k210_pll_calc_config(1500000000, 20000000,
						  &theirs));
	ut_asserteq(-EINVAL, k210_pll_calc_config(1750000000, 13300000,
						  &theirs));

	/* Verify we get the same output with brute-force */
#define compare(rate, rate_in) do { \
	ut_assertok(dm_test_k210_pll_calc_config(rate, rate_in, &ours)); \
	ut_assertok(k210_pll_calc_config(rate, rate_in, &theirs)); \
	ut_assertok(dm_test_k210_pll_compare(&ours, &theirs)); \
} while (0)

	compare(390000000, 26000000);
	compare(26000000, 390000000);
	compare(400000000, 26000000);
	compare(27000000, 26000000);
	compare(26000000, 27000000);
	compare(13300000 * 64, 13300000);
	compare(21250000, 21250000 * 70);
	compare(21250000, 1750000000);
	compare(1750000000, 1750000000);

	return 0;
}
DM_TEST(dm_test_k210_pll, 0);
