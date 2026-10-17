// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Unit tests for memory functions
 *
 * The architecture dependent implementations run through different lines of
 * code depending on the alignment and length of memory regions copied or set.
 * This has to be considered in testing.
 */

#include <common.h>
#include <command.h>
#include <errno.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>

/**
 * lib_errno_str() - unit test for errno_str()
 *
 * Test errno_str() with varied alignment and length of the copied buffer.
 *
 * @uts:	unit test state
 * Return:	0 = success, 1 = failure
 */
static int lib_errno_str(struct unit_test_state *uts)
{
	const char *msg;

	msg = errno_str(1);
	ut_asserteq_str("Success", msg);

	msg = errno_str(0);
	ut_asserteq_str("Success", msg);

	msg = errno_str(-ENOMEM);
	ut_asserteq_str("Out of memory", msg);

	msg = errno_str(-99999);
	ut_asserteq_str("Unknown error", msg);

	return 0;
}

LIB_TEST(lib_errno_str, 0);
