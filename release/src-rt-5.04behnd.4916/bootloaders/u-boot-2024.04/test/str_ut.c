// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 Google LLC
 */

#include <common.h>
#include <vsprintf.h>
#include <test/suites.h>
#include <test/test.h>
#include <test/ut.h>

/* This is large enough for any of the test strings */
#define TEST_STR_SIZE	200

static const char str1[] = "I'm sorry I'm late.";
static const char str2[] = "1099abNo, don't bother apologising.";
static const char str3[] = "0xbI'm sorry you're alive.";
static const char str4[] = "1234567890123 I lost closer friends";
static const char str5[] = "0x9876543210the last time I was deloused";
static const char str6[] = "0778octal is seldom used";
static const char str7[] = "707it is a piece of computing history";

/* Declare a new str test */
#define STR_TEST(_name, _flags)		UNIT_TEST(_name, _flags, str_test)

static int str_upper(struct unit_test_state *uts)
{
	char out[TEST_STR_SIZE];

	/* Make sure it adds a terminator */
	out[strlen(str1)] = 'a';
	str_to_upper(str1, out, SIZE_MAX);
	ut_asserteq_str("I'M SORRY I'M LATE.", out);

	/* In-place operation */
	strcpy(out, str2);
	str_to_upper(out, out, SIZE_MAX);
	ut_asserteq_str("1099ABNO, DON'T BOTHER APOLOGISING.", out);

	/* Limited length */
	str_to_upper(str1, out, 7);
	ut_asserteq_str("I'M SORO, DON'T BOTHER APOLOGISING.", out);

	/* In-place with limited length */
	strcpy(out, str2);
	str_to_upper(out, out, 7);
	ut_asserteq_str("1099ABNo, don't bother apologising.", out);

	/* Copy an empty string to a buffer with space*/
	out[1] = 0x7f;
	str_to_upper("", out, SIZE_MAX);
	ut_asserteq('\0', *out);
	ut_asserteq(0x7f, out[1]);

	/* Copy an empty string to a buffer with no space*/
	out[0] = 0x7f;
	str_to_upper("", out, 0);
	ut_asserteq(0x7f, out[0]);

	return 0;
}
STR_TEST(str_upper, 0);

static int run_strtoul(struct unit_test_state *uts, const char *str, int base,
		       ulong expect_val, int expect_endp_offset, bool upper)
{
	char out[TEST_STR_SIZE];
	char *endp;
	ulong val;

	strcpy(out, str);
	if (upper)
		str_to_upper(out, out, -1);

	val = simple_strtoul(out, &endp, base);
	ut_asserteq(expect_val, val);
	ut_asserteq(expect_endp_offset, endp - out);

	return 0;
}

static int str_simple_strtoul(struct unit_test_state *uts)
{
	int upper;

	/* Check that it is case-insentive */
	for (upper = 0; upper < 2; upper++) {
		/* Base 10 and base 16 */
		ut_assertok(run_strtoul(uts, str2, 10, 1099, 4, upper));
		ut_assertok(run_strtoul(uts, str2, 16, 0x1099ab, 6, upper));
		ut_assertok(run_strtoul(uts, str3, 16, 0xb, 3, upper));
		ut_assertok(run_strtoul(uts, str3, 10, 0xb, 3, upper));

		/* Octal */
		ut_assertok(run_strtoul(uts, str6, 0, 63, 3, upper));
		ut_assertok(run_strtoul(uts, str7, 8, 0x1c7, 3, upper));

		/* Invalid string */
		ut_assertok(run_strtoul(uts, str1, 10, 0, 0, upper));

		/* Base 0 */
		ut_assertok(run_strtoul(uts, str1, 0, 0, 0, upper));
		ut_assertok(run_strtoul(uts, str2, 0, 1099, 4, upper));
		ut_assertok(run_strtoul(uts, str3, 0, 0xb, 3, upper));

		/* Base 2 */
		ut_assertok(run_strtoul(uts, str1, 2, 0, 0, upper));
		ut_assertok(run_strtoul(uts, str2, 2, 2, 2, upper));
	}

	/* Check endp being NULL */
	ut_asserteq(1099, simple_strtoul(str2, NULL, 0));

	return 0;
}
STR_TEST(str_simple_strtoul, 0);

static int run_strtoull(struct unit_test_state *uts, const char *str, int base,
			unsigned long long expect_val, int expect_endp_offset,
			bool upper)
{
	char out[TEST_STR_SIZE];
	char *endp;
	unsigned long long val;

	strcpy(out, str);
	if (upper)
		str_to_upper(out, out, -1);

	val = simple_strtoull(out, &endp, base);
	ut_asserteq(expect_val, val);
	ut_asserteq(expect_endp_offset, endp - out);

	return 0;
}

static int str_simple_strtoull(struct unit_test_state *uts)
{
	int upper;

	/* Check that it is case-insentive */
	for (upper = 0; upper < 2; upper++) {
		/* Base 10 and base 16 */
		ut_assertok(run_strtoull(uts, str2, 10, 1099, 4, upper));
		ut_assertok(run_strtoull(uts, str2, 16, 0x1099ab, 6, upper));
		ut_assertok(run_strtoull(uts, str3, 16, 0xb, 3, upper));
		ut_assertok(run_strtoull(uts, str3, 10, 0xb, 3, upper));

		/* Octal */
		ut_assertok(run_strtoull(uts, str6, 0, 63, 3, upper));
		ut_assertok(run_strtoull(uts, str7, 8, 0x1c7, 3, upper));

		/* Large values */
		ut_assertok(run_strtoull(uts, str4, 10, 1234567890123, 13,
					 upper));
		ut_assertok(run_strtoull(uts, str4, 16, 0x1234567890123, 13,
					 upper));
		ut_assertok(run_strtoull(uts, str5, 0, 0x9876543210, 12,
					 upper));

		/* Invalid string */
		ut_assertok(run_strtoull(uts, str1, 10, 0, 0, upper));

		/* Base 0 */
		ut_assertok(run_strtoull(uts, str1, 0, 0, 0, upper));
		ut_assertok(run_strtoull(uts, str2, 0, 1099, 4, upper));
		ut_assertok(run_strtoull(uts, str3, 0, 0xb, 3, upper));

		/* Base 2 */
		ut_assertok(run_strtoull(uts, str1, 2, 0, 0, upper));
		ut_assertok(run_strtoull(uts, str2, 2, 2, 2, upper));
	}

	/* Check endp being NULL */
	ut_asserteq(1099, simple_strtoull(str2, NULL, 0));

	return 0;
}
STR_TEST(str_simple_strtoull, 0);

static int str_hextoul(struct unit_test_state *uts)
{
	char *endp;

	/* Just a simple test, since we know this uses simple_strtoul() */
	ut_asserteq(0x1099ab, hextoul(str2, &endp));
	ut_asserteq(6, endp - str2);

	return 0;
}
STR_TEST(str_hextoul, 0);

static int str_dectoul(struct unit_test_state *uts)
{
	char *endp;

	/* Just a simple test, since we know this uses simple_strtoul() */
	ut_asserteq(1099, dectoul(str2, &endp));
	ut_asserteq(4, endp - str2);

	return 0;
}
STR_TEST(str_dectoul, 0);

static int str_itoa(struct unit_test_state *uts)
{
	ut_asserteq_str("123", simple_itoa(123));
	ut_asserteq_str("0", simple_itoa(0));
	ut_asserteq_str("2147483647", simple_itoa(0x7fffffff));
	ut_asserteq_str("4294967295", simple_itoa(0xffffffff));

	/* Use #ifdef here to avoid a compiler warning on 32-bit machines */
#ifdef CONFIG_PHYS_64BIT
	if (sizeof(ulong) == 8) {
		ut_asserteq_str("9223372036854775807",
				simple_itoa((1UL << 63) - 1));
		ut_asserteq_str("18446744073709551615", simple_itoa(-1));
	}
#endif /* CONFIG_PHYS_64BIT */

	return 0;
}
STR_TEST(str_itoa, 0);

static int str_xtoa(struct unit_test_state *uts)
{
	ut_asserteq_str("7f", simple_xtoa(127));
	ut_asserteq_str("00", simple_xtoa(0));
	ut_asserteq_str("7fffffff", simple_xtoa(0x7fffffff));
	ut_asserteq_str("ffffffff", simple_xtoa(0xffffffff));

	/* Use #ifdef here to avoid a compiler warning on 32-bit machines */
#ifdef CONFIG_PHYS_64BIT
	if (sizeof(ulong) == 8) {
		ut_asserteq_str("7fffffffffffffff",
				simple_xtoa((1UL << 63) - 1));
		ut_asserteq_str("ffffffffffffffff", simple_xtoa(-1));
	}
#endif /* CONFIG_PHYS_64BIT */

	return 0;
}
STR_TEST(str_xtoa, 0);

static int str_trailing(struct unit_test_state *uts)
{
	const char str1[] = "abc123def";
	const char str2[] = "abc123def456";
	const char *end;

	ut_asserteq(-1, trailing_strtol(""));
	ut_asserteq(-1, trailing_strtol("123"));
	ut_asserteq(123, trailing_strtol("abc123"));
	ut_asserteq(4, trailing_strtol("12c4"));
	ut_asserteq(-1, trailing_strtol("abd"));
	ut_asserteq(-1, trailing_strtol("abc123def"));

	ut_asserteq(-1, trailing_strtoln(str1, NULL));
	ut_asserteq(123, trailing_strtoln(str1, str1 + 6));
	ut_asserteq(-1, trailing_strtoln(str1, str1 + 9));

	ut_asserteq(3, trailing_strtol("a3"));

	ut_asserteq(123, trailing_strtoln_end(str1, str1 + 6, &end));
	ut_asserteq(3, end - str1);

	ut_asserteq(-1, trailing_strtoln_end(str1, str1 + 7, &end));
	ut_asserteq(7, end - str1);

	ut_asserteq(456, trailing_strtoln_end(str2, NULL, &end));
	ut_asserteq(9, end - str2);

	return 0;
}
STR_TEST(str_trailing, 0);

static int test_str_to_list(struct unit_test_state *uts)
{
	const char **ptr;
	ulong start;

	/* check out of memory */
	start = ut_check_delta(0);
	malloc_enable_testing(0);
	ut_assertnull(str_to_list(""));
	ut_assertok(ut_check_delta(start));

	ut_assertnull(str_to_list("this is a test"));
	ut_assertok(ut_check_delta(start));

	malloc_enable_testing(1);
	ut_assertnull(str_to_list("this is a test"));
	ut_assertok(ut_check_delta(start));

	/* for an empty string, only one nalloc is needed */
	malloc_enable_testing(1);
	ptr = str_to_list("");
	ut_assertnonnull(ptr);
	ut_assertnull(ptr[0]);
	str_free_list(ptr);
	ut_assertok(ut_check_delta(start));

	malloc_disable_testing();

	/* test the same again, without any nalloc restrictions */
	ptr = str_to_list("");
	ut_assertnonnull(ptr);
	ut_assertnull(ptr[0]);
	str_free_list(ptr);
	ut_assertok(ut_check_delta(start));

	/* test a single string */
	start = ut_check_delta(0);
	ptr = str_to_list("hi");
	ut_assertnonnull(ptr);
	ut_assertnonnull(ptr[0]);
	ut_asserteq_str("hi", ptr[0]);
	ut_assertnull(ptr[1]);
	str_free_list(ptr);
	ut_assertok(ut_check_delta(start));

	/* test two strings */
	ptr = str_to_list("hi there");
	ut_assertnonnull(ptr);
	ut_assertnonnull(ptr[0]);
	ut_asserteq_str("hi", ptr[0]);
	ut_assertnonnull(ptr[1]);
	ut_asserteq_str("there", ptr[1]);
	ut_assertnull(ptr[2]);
	str_free_list(ptr);
	ut_assertok(ut_check_delta(start));

	/* test leading, trailing and multiple spaces */
	ptr = str_to_list(" more  space  ");
	ut_assertnonnull(ptr);
	ut_assertnonnull(ptr[0]);
	ut_asserteq_str("", ptr[0]);
	ut_assertnonnull(ptr[1]);
	ut_asserteq_str("more", ptr[1]);
	ut_assertnonnull(ptr[2]);
	ut_asserteq_str("", ptr[2]);
	ut_assertnonnull(ptr[3]);
	ut_asserteq_str("space", ptr[3]);
	ut_assertnonnull(ptr[4]);
	ut_asserteq_str("", ptr[4]);
	ut_assertnonnull(ptr[5]);
	ut_asserteq_str("", ptr[5]);
	ut_assertnull(ptr[6]);
	str_free_list(ptr);
	ut_assertok(ut_check_delta(start));

	/* test freeing a NULL pointer */
	str_free_list(NULL);

	return 0;
}
STR_TEST(test_str_to_list, 0);

int do_ut_str(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = UNIT_TEST_SUITE_START(str_test);
	const int n_ents = UNIT_TEST_SUITE_COUNT(str_test);

	return cmd_ut_category("str", "str_", tests, n_ents, argc, argv);
}
