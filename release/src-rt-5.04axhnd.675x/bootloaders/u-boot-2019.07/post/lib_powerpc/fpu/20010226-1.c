// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2007
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */
/*
 * This file is originally a part of the GCC testsuite.
 */

#include <common.h>

#include <post.h>

GNU_FPOST_ATTR

#if CONFIG_POST & CONFIG_SYS_POST_FPU

int fpu_post_test_math3 (void)
{
	volatile long double dfrom = 1.1;
	volatile long double m1;
	volatile long double m2;
	volatile unsigned long mant_long;

	m1 = dfrom / 2.0;
	m2 = m1 * 4294967296.0;
	mant_long = ((unsigned long) m2) & 0xffffffff;

	if (mant_long != 0x8ccccccc) {
		post_log ("Error in FPU math3 test\n");
		return -1;
	}
	return 0;
}

#endif /* CONFIG_POST & CONFIG_SYS_POST_FPU */
