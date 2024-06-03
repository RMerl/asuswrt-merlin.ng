/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Copyright (C) 2018 SiFive, Inc.
 * Wesley Terpstra
 * Paul Walmsley
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __LINUX_CLK_ANALOGBITS_WRPLL_CLN28HPC_H
#define __LINUX_CLK_ANALOGBITS_WRPLL_CLN28HPC_H

#include <linux/types.h>

/* DIVQ_VALUES: number of valid DIVQ values */
#define DIVQ_VALUES				6

/*
 * Bit definitions for struct analogbits_wrpll_cfg.flags
 *
 * WRPLL_FLAGS_BYPASS_FLAG: if set, the PLL is either in bypass, or should be
 *	programmed to enter bypass
 * WRPLL_FLAGS_RESET_FLAG: if set, the PLL is in reset
 * WRPLL_FLAGS_INT_FEEDBACK_FLAG: if set, the PLL is configured for internal
 *	feedback mode
 * WRPLL_FLAGS_EXT_FEEDBACK_FLAG: if set, the PLL is configured for external
 *	feedback mode (not yet supported by this driver)
 *
 * The flags WRPLL_FLAGS_INT_FEEDBACK_FLAG and WRPLL_FLAGS_EXT_FEEDBACK_FLAG are
 * mutually exclusive.  If both bits are set, or both are zero, the struct
 * analogbits_wrpll_cfg record is uninitialized or corrupt.
 */
#define WRPLL_FLAGS_BYPASS_SHIFT		0
#define WRPLL_FLAGS_BYPASS_MASK		BIT(WRPLL_FLAGS_BYPASS_SHIFT)
#define WRPLL_FLAGS_RESET_SHIFT		1
#define WRPLL_FLAGS_RESET_MASK		BIT(WRPLL_FLAGS_RESET_SHIFT)
#define WRPLL_FLAGS_INT_FEEDBACK_SHIFT	2
#define WRPLL_FLAGS_INT_FEEDBACK_MASK	BIT(WRPLL_FLAGS_INT_FEEDBACK_SHIFT)
#define WRPLL_FLAGS_EXT_FEEDBACK_SHIFT	3
#define WRPLL_FLAGS_EXT_FEEDBACK_MASK	BIT(WRPLL_FLAGS_EXT_FEEDBACK_SHIFT)

/**
 * struct analogbits_wrpll_cfg - WRPLL configuration values
 * @divr: reference divider value (6 bits), as presented to the PLL signals.
 * @divf: feedback divider value (9 bits), as presented to the PLL signals.
 * @divq: output divider value (3 bits), as presented to the PLL signals.
 * @flags: PLL configuration flags.  See above for more information.
 * @range: PLL loop filter range.  See below for more information.
 * @_output_rate_cache: cached output rates, swept across DIVQ.
 * @_parent_rate: PLL refclk rate for which values are valid
 * @_max_r: maximum possible R divider value, given @parent_rate
 * @_init_r: initial R divider value to start the search from
 *
 * @divr, @divq, @divq, @range represent what the PLL expects to see
 * on its input signals.  Thus @divr and @divf are the actual divisors
 * minus one.  @divq is a power-of-two divider; for example, 1 =
 * divide-by-2 and 6 = divide-by-64.  0 is an invalid @divq value.
 *
 * When initially passing a struct analogbits_wrpll_cfg record, the
 * record should be zero-initialized with the exception of the @flags
 * field.  The only flag bits that need to be set are either
 * WRPLL_FLAGS_INT_FEEDBACK or WRPLL_FLAGS_EXT_FEEDBACK.
 *
 * Field names beginning with an underscore should be considered
 * private to the wrpll-cln28hpc.c code.
 */
struct analogbits_wrpll_cfg {
	u8 divr;
	u8 divq;
	u8 range;
	u8 flags;
	u16 divf;
	u32 _output_rate_cache[DIVQ_VALUES];
	unsigned long _parent_rate;
	u8 _max_r;
	u8 _init_r;
};

/*
 * Function prototypes
 */

int analogbits_wrpll_configure_for_rate(struct analogbits_wrpll_cfg *c,
					u32 target_rate,
					unsigned long parent_rate);

unsigned int analogbits_wrpll_calc_max_lock_us(struct analogbits_wrpll_cfg *c);

unsigned long analogbits_wrpll_calc_output_rate(struct analogbits_wrpll_cfg *c,
						unsigned long parent_rate);

#endif /* __LINUX_CLK_ANALOGBITS_WRPLL_CLN28HPC_H */
