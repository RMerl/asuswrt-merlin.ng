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

static float rintf (float x)
{
	volatile float TWO23 = 8388608.0;

	if (__builtin_fabs (x) < TWO23)
	{
		if (x > 0.0)
		{
			x += TWO23;
			x -= TWO23;
		}
		else if (x < 0.0)
		{
			x = TWO23 - x;
			x = -(x - TWO23);
		}
	}

	return x;
}

int fpu_post_test_math2 (void)
{
	if (rintf (-1.5) != -2.0) {
		post_log ("Error in FPU math2 test\n");
		return -1;
	}
	return 0;
}

#endif /* CONFIG_POST & CONFIG_SYS_POST_FPU */
