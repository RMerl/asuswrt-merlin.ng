// SPDX-License-Identifier: GPL-2.1+
/*
 * Copyright (C) 2021 Sean Anderson <seanga2@gmail.com>
 * Copyright (C) 2011-2021 Free Software Foundation, Inc.
 *
 * These tests adapted from glibc's string/test-strncat.c
 */

#include <common.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>

#define BUF_SIZE 4096
char buf1[BUF_SIZE], buf2[BUF_SIZE];

static int do_test_strlcat(struct unit_test_state *uts, int line, size_t align1,
			   size_t align2, size_t len1, size_t len2, size_t n)
{
	char *s1, *s2;
	size_t i, len, expected, actual;

	align1 &= 7;
	if (align1 + len1 >= BUF_SIZE)
		return 0;
	if (align1 + n > BUF_SIZE)
		return 0;

	align2 &= 7;
	if (align2 + len1 + len2 >= BUF_SIZE)
		return 0;
	if (align2 + len1 + n > BUF_SIZE)
		return 0;

	s1 = buf1 + align1;
	s2 = buf2 + align2;

	for (i = 0; i < len1 - 1; i++)
		s1[i] = 32 + 23 * i % (127 - 32);
	s1[len1 - 1] = '\0';

	for (i = 0; i < len2 - 1; i++)
		s2[i] = 32 + 23 * i % (127 - 32);
	s2[len2 - 1] = '\0';

	expected = min(strlen(s2), n) + strlen(s1);
	actual = strlcat(s2, s1, n);
	if (expected != actual) {
		ut_failf(uts, __FILE__, line, __func__,
			 "strlcat(s2, s1, n) == min(len2, n) + len1",
			 "Expected %#zx (%zd), got %#zx (%zd)",
			 expected, expected, actual, actual);
		return CMD_RET_FAILURE;
	}

	len = min3(len1, n - len2, (size_t)0);
	if (memcmp(s2 + len2, s1, len)) {
		ut_failf(uts, __FILE__, line, __func__,
			 "s2 + len1 == s1",
			 "Expected \"%.*s\", got \"%.*s\"",
			 (int)len, s1, (int)len, s2 + len2);
		return CMD_RET_FAILURE;
	}

	i = min(n, len1 + len2 - 1) - 1;
	if (len2 < n && s2[i] != '\0') {
		ut_failf(uts, __FILE__, line, __func__,
			 "n < len1 && s2[len2 + n] == '\\0'",
			 "Expected s2[%zd] = '\\0', got %d ('%c')",
			 i, s2[i], s2[i]);
		return CMD_RET_FAILURE;
	}

	return 0;
}

#define test_strlcat(align1, align2, len1, len2, n) do { \
	int ret = do_test_strlcat(uts, __LINE__, align1, align2, len1, len2, \
				  n); \
	if (ret) \
		return ret; \
} while (0)

static int lib_test_strlcat(struct unit_test_state *uts)
{
	size_t i, n;

	test_strlcat(0, 2, 2, 2, SIZE_MAX);
	test_strlcat(0, 0, 4, 4, SIZE_MAX);
	test_strlcat(4, 0, 4, 4, SIZE_MAX);
	test_strlcat(0, 0, 8, 8, SIZE_MAX);
	test_strlcat(0, 8, 8, 8, SIZE_MAX);

	for (i = 1; i < 8; i++) {
		test_strlcat(0, 0, 8 << i, 8 << i, SIZE_MAX);
		test_strlcat(8 - i, 2 * i, 8 << i, 8 << i, SIZE_MAX);
		test_strlcat(0, 0, 8 << i, 2 << i, SIZE_MAX);
		test_strlcat(8 - i, 2 * i, 8 << i, 2 << i, SIZE_MAX);

		test_strlcat(i, 2 * i, 8 << i, 1, SIZE_MAX);
		test_strlcat(2 * i, i, 8 << i, 1, SIZE_MAX);
		test_strlcat(i, i, 8 << i, 10, SIZE_MAX);
	}

	for (n = 2; n <= 2048; n *= 4) {
		test_strlcat(0, 2, 2, 2, n);
		test_strlcat(0, 0, 4, 4, n);
		test_strlcat(4, 0, 4, 4, n);
		test_strlcat(0, 0, 8, 8, n);
		test_strlcat(0, 8, 8, 8, n);

		for (i = 1; i < 8; i++) {
			test_strlcat(0, 0, 8 << i, 8 << i, n);
			test_strlcat(8 - i, 2 * i, 8 << i, 8 << i, n);
			test_strlcat(0, 0, 8 << i, 2 << i, n);
			test_strlcat(8 - i, 2 * i, 8 << i, 2 << i, n);

			test_strlcat(i, 2 * i, 8 << i, 1, n);
			test_strlcat(2 * i, i, 8 << i, 1, n);
			test_strlcat(i, i, 8 << i, 10, n);
		}
	}

	return 0;
}
LIB_TEST(lib_test_strlcat, 0);
