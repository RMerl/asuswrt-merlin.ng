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

static double func (const double *array)
{
	double d = *array;

	if (d == 0.0)
		return d;
	else
		return d + func (array + 1);
}

int fpu_post_test_math5 (void)
{
	double values[] = { 0.1e-100, 1.0, -1.0, 0.0 };

	if (func (values) != 0.1e-100) {
		post_log ("Error in FPU math5 test\n");
		return -1;
	}
	return 0;
}

#endif /* CONFIG_POST & CONFIG_SYS_POST_FPU */
