// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2023, Heinrich Schuchardt <heinrich.schuchardt@canonical.com>
 *
 * Unit test for crc8
 */

#include <test/lib.h>
#include <test/ut.h>
#include <u-boot/crc.h>

static int lib_crc8(struct unit_test_state *uts) {
	const char str[] = {0x20, 0xf4, 0xd8, 0x24, 0x6f, 0x41, 0x91, 0xae,
			    0x46, 0x61, 0xf6, 0x55, 0xeb, 0x38, 0x47, 0x0f,
			    0xec, 0xd8};
	int actual1, actual2, actual3;
	int expected1 = 0x47, expected2 = 0xea, expected3 = expected1;

	actual1 = crc8(0, str, sizeof(str));
	ut_asserteq(expected1, actual1);
	actual2 = crc8(0, str, 7);
	ut_asserteq(expected2, actual2);
	actual3 = crc8(actual2, str + 7, sizeof(str) - 7);
	ut_asserteq(expected3, actual3);

	return 0;
}

LIB_TEST(lib_crc8, 0);
