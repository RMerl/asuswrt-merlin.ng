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

int fpu_post_test_math1 (void)
{
	volatile double a;
	double c, d;
	volatile double b;

	d = 1.0;

	do
	{
		c = d;
		d = c * 0.5;
		b = 1 + d;
	} while (b != 1.0);

	a = 1.0 + c;

	if (a == 1.0) {
		post_log ("Error in FPU math1 test\n");
		return -1;
	}

	return 0;
}

#endif /* CONFIG_POST & CONFIG_SYS_POST_FPU */
