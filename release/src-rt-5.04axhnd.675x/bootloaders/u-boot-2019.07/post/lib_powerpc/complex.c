// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>

/*
 * CPU test
 * Complex calculations
 *
 * The calculations in this test are just a combination of simpler
 * calculations, but probably under different timing conditions, etc.
 */

#include <post.h>
#include "cpu_asm.h"

#if CONFIG_POST & CONFIG_SYS_POST_CPU

extern int cpu_post_complex_1_asm (int a1, int a2, int a3, int a4, int n);
extern int cpu_post_complex_2_asm (int x, int n);

  /*
   *     n
   *	SUM (a1 * a2 - a3) / a4 = n * result
   *    i=1
   */
static int cpu_post_test_complex_1 (void)
{
    int a1 = 666;
    int a2 = 667;
    int a3 = 668;
    int a4 = 66;
    int n = 100;
    int result = 6720; /* (a1 * a2 - a3) / a4 */

    if (cpu_post_complex_1_asm(a1, a2, a3, a4, n) != n * result)
    {
	return -1;
    }

    return 0;
}

  /*	(1 + x + x^2 + ... + x^n) * (1 - x) = 1 - x^(n+1)
   */
static int cpu_post_test_complex_2 (void)
{
    int ret = -1;
    int x;
    int n;
    int k;
    int left;
    int right;

    for (x = -8; x <= 8; x ++)
    {
	n = 9;

	left = cpu_post_complex_2_asm(x, n);
	left *= 1 - x;

	right = 1;
	for (k = 0; k <= n; k ++)
	{
	    right *= x;
	}
	right = 1 - right;

	if (left != right)
	{
	    goto Done;
	}
    }

    ret = 0;
    Done:

    return ret;
}

int cpu_post_test_complex (void)
{
    int ret = 0;
    int flag = disable_interrupts();

    if (ret == 0)
    {
	ret = cpu_post_test_complex_1();
    }

    if (ret == 0)
    {
	ret = cpu_post_test_complex_2();
    }

    if (ret != 0)
    {
	post_log ("Error at complex test !\n");
    }

    if (flag)
	enable_interrupts();

    return ret;
}

#endif
