// SPDX-License-Identifier: GPL-2.0+
/*
 * Test setjmp(), longjmp()
 *
 * Copyright (c) 2021, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#include <common.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>
#include <asm/setjmp.h>

struct test_jmp_buf {
	jmp_buf env;
	int val;
};

/**
 * test_longjmp() - test longjmp function
 *
 * @i is passed to longjmp.
 * @i << 8 is set in the environment structure.
 *
 * @env:	environment
 * @i:		value passed to longjmp()
 */
static noinline void test_longjmp(struct test_jmp_buf *env, int i)
{
	env->val = i << 8;
	longjmp(env->env, i);
}

/**
 * test_setjmp() - test setjmp function
 *
 * setjmp() will return the value @i passed to longjmp() if @i is non-zero.
 * For @i == 0 we expect return value 1.
 *
 * @i << 8 will be set by test_longjmp in the environment structure.
 * This value can be used to check that the stack frame is restored.
 *
 * We return the XORed values to allow simply check both at once.
 *
 * @i:		value passed to longjmp()
 * Return:	values return by longjmp()
 */
static int test_setjmp(int i)
{
	struct test_jmp_buf env;
	int ret;

	env.val = -1;
	ret = setjmp(env.env);
	if (ret)
		return ret ^ env.val;
	test_longjmp(&env, i);
	/* We should not arrive here */
	return 0x1000;
}

static int lib_test_longjmp(struct unit_test_state *uts)
{
	int i;

	for (i = -3; i < 0; ++i)
		ut_asserteq(i ^ (i << 8), test_setjmp(i));
	ut_asserteq(1, test_setjmp(0));
	for (i = 1; i < 4; ++i)
		ut_asserteq(i ^ (i << 8), test_setjmp(i));
	return 0;
}
LIB_TEST(lib_test_longjmp, 0);
