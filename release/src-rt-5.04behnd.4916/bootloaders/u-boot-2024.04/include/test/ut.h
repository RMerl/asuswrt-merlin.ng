/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Simple unit test library
 *
 * Copyright (c) 2013 Google, Inc
 */

#ifndef __TEST_UT_H
#define __TEST_UT_H

#include <command.h>
#include <hexdump.h>
#include <linux/err.h>
#include <test/test.h>

struct unit_test_state;

/**
 * ut_fail() - Record failure of a unit test
 *
 * @uts: Test state
 * @fname: Filename where the error occurred
 * @line: Line number where the error occurred
 * @func: Function name where the error occurred
 * @cond: The condition that failed
 */
void ut_fail(struct unit_test_state *uts, const char *fname, int line,
	     const char *func, const char *cond);

/**
 * ut_failf() - Record failure of a unit test
 *
 * @uts: Test state
 * @fname: Filename where the error occurred
 * @line: Line number where the error occurred
 * @func: Function name where the error occurred
 * @cond: The condition that failed
 * @fmt: printf() format string for the error, followed by args
 */
void ut_failf(struct unit_test_state *uts, const char *fname, int line,
	      const char *func, const char *cond, const char *fmt, ...)
			__attribute__ ((format (__printf__, 6, 7)));

/**
 * ut_check_console_line() - Check the next console line against expectations
 *
 * This creates a string and then checks it against the next line of console
 * output obtained with console_record_readline().
 *
 * After the function returns, uts->expect_str holds the expected string and
 * uts->actual_str holds the actual string read from the console.
 *
 * @uts: Test state
 * @fmt: printf() format string for the error, followed by args
 * Return: 0 if OK, other value on error
 */
int ut_check_console_line(struct unit_test_state *uts, const char *fmt, ...)
			__attribute__ ((format (__printf__, 2, 3)));

/**
 * ut_check_console_linen() - Check part of the next console line
 *
 * This creates a string and then checks it against the next line of console
 * output obtained with console_record_readline(). Only the length of the
 * string is checked
 *
 * After the function returns, uts->expect_str holds the expected string and
 * uts->actual_str holds the actual string read from the console.
 *
 * @uts: Test state
 * @fmt: printf() format string for the error, followed by args
 * Return: 0 if OK, other value on error
 */
int ut_check_console_linen(struct unit_test_state *uts, const char *fmt, ...)
			__attribute__ ((format (__printf__, 2, 3)));

/**
 * ut_check_skipline() - Check that the next console line exists and skip it
 *
 * @uts: Test state
 * Return: 0 if OK, other value on error
 */
int ut_check_skipline(struct unit_test_state *uts);

/**
 * ut_check_skip_to_line() - skip output until a line is found
 *
 * This creates a string and then checks it against the following lines of
 * console output obtained with console_record_readline() until it is found.
 *
 * After the function returns, uts->expect_str holds the expected string and
 * uts->actual_str holds the actual string read from the console.
 *
 * @uts: Test state
 * @fmt: printf() format string to look for, followed by args
 * Return: 0 if OK, -ENOENT if not found, other value on error
 */
int ut_check_skip_to_line(struct unit_test_state *uts, const char *fmt, ...);

/**
 * ut_check_skip_to_linen() - skip output until a partial line is found
 *
 * This creates a string and then checks it against the following lines of
 * console output obtained with console_record_readline() until it is found.
 * Only the characters up to the length of the string are checked, so the line
 * may extend further
 *
 * After the function returns, uts->expect_str holds the expected string and
 * uts->actual_str holds the actual string read from the console.
 *
 * @uts: Test state
 * @fmt: printf() format string to look for, followed by args
 * Return: 0 if OK, -ENOENT if not found, other value on error
 */
int ut_check_skip_to_linen(struct unit_test_state *uts, const char *fmt, ...);

/**
 * ut_check_console_end() - Check there is no more console output
 *
 * After the function returns, uts->actual_str holds the actual string read
 * from the console
 *
 * @uts: Test state
 * Return: 0 if OK (console has no output), other value on error
 */
int ut_check_console_end(struct unit_test_state *uts);

/**
 * ut_check_console_dump() - Check that next lines have a print_buffer() dump
 *
 * This only supports a byte dump.
 *
 * @total_bytes: Size of the expected dump in bytes`
 * Return: 0 if OK (looks like a dump and the length matches), other value on
 *	error
 */
int ut_check_console_dump(struct unit_test_state *uts, int total_bytes);

/* Report a failure, with printf() string */
#define ut_reportf(fmt, args...)					\
	ut_failf(uts, __FILE__, __LINE__, __func__, "report",		\
		 fmt, ##args)

/* Assert that a condition is non-zero */
#define ut_assert(cond) ({						\
	int __ret = 0;							\
									\
	if (!(cond)) {							\
		ut_fail(uts, __FILE__, __LINE__, __func__, #cond);	\
		return CMD_RET_FAILURE;					\
	}								\
	__ret;								\
})

/* Assert that a condition is non-zero, with printf() string */
#define ut_assertf(cond, fmt, args...) ({				\
	int __ret = 0;							\
									\
	if (!(cond)) {							\
		ut_failf(uts, __FILE__, __LINE__, __func__, #cond,	\
			 fmt, ##args);					\
		return CMD_RET_FAILURE;					\
	}								\
	__ret;								\
})

/* Assert that two int expressions are equal */
#define ut_asserteq(expr1, expr2) ({					\
	unsigned int _val1 = (expr1), _val2 = (expr2);			\
	int __ret = 0;							\
									\
	if (_val1 != _val2) {						\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 #expr1 " == " #expr2,				\
			 "Expected %#x (%d), got %#x (%d)",		\
			 _val1, _val1, _val2, _val2);			\
		return CMD_RET_FAILURE;					\
	}								\
	__ret;								\
})

/* Assert that two 64 int expressions are equal */
#define ut_asserteq_64(expr1, expr2) ({					\
	u64 _val1 = (expr1), _val2 = (expr2);				\
	int __ret = 0;							\
									\
	if (_val1 != _val2) {						\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 #expr1 " == " #expr2,				\
			 "Expected %#llx (%lld), got %#llx (%lld)",	\
			 (unsigned long long)_val1,			\
			 (unsigned long long)_val1,			\
			 (unsigned long long)_val2,			\
			 (unsigned long long)_val2);			\
		return CMD_RET_FAILURE;					\
	}								\
	__ret;								\
})

/* Assert that two string expressions are equal */
#define ut_asserteq_str(expr1, expr2) ({				\
	const char *_val1 = (expr1), *_val2 = (expr2);			\
	int __ret = 0;							\
									\
	if (strcmp(_val1, _val2)) {					\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 #expr1 " = " #expr2,				\
			 "Expected \"%s\", got \"%s\"", _val1, _val2);	\
		return CMD_RET_FAILURE;					\
	}								\
	__ret;								\
})

/*
 * Assert that two string expressions are equal, up to length of the
 * first
 */
#define ut_asserteq_strn(expr1, expr2) ({				\
	const char *_val1 = (expr1), *_val2 = (expr2);			\
	int _len = strlen(_val1);					\
	int __ret = 0;							\
									\
	if (memcmp(_val1, _val2, _len)) {				\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 #expr1 " = " #expr2,				\
			 "Expected \"%.*s\", got \"%.*s\"",		\
			 _len, _val1, _len, _val2);			\
		return CMD_RET_FAILURE;					\
	}								\
	__ret;								\
})

/* Assert that two memory areas are equal */
#define ut_asserteq_mem(expr1, expr2, len) ({				\
	const u8 *_val1 = (u8 *)(expr1), *_val2 = (u8 *)(expr2);	\
	const uint __len = len;						\
	int __ret = 0;							\
									\
	if (memcmp(_val1, _val2, __len)) {				\
		char __buf1[64 + 1] = "\0";				\
		char __buf2[64 + 1] = "\0";				\
		bin2hex(__buf1, _val1, min(__len, (uint)32));		\
		bin2hex(__buf2, _val2, min(__len, (uint)32));		\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 #expr1 " = " #expr2,				\
			 "Expected \"%s\", got \"%s\"",			\
			 __buf1, __buf2);				\
		return CMD_RET_FAILURE;					\
	}								\
	__ret;								\
})

/* Assert that two pointers are equal */
#define ut_asserteq_ptr(expr1, expr2) ({				\
	const void *_val1 = (expr1), *_val2 = (expr2);			\
	int __ret = 0;							\
									\
	if (_val1 != _val2) {						\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 #expr1 " = " #expr2,				\
			 "Expected %p, got %p", _val1, _val2);		\
		return CMD_RET_FAILURE;					\
	}								\
	__ret;								\
})

/* Assert that two addresses (converted from pointers) are equal */
#define ut_asserteq_addr(expr1, expr2) ({				\
	ulong _val1 = map_to_sysmem(expr1);				\
	ulong _val2 = map_to_sysmem(expr2);				\
	int __ret = 0;							\
									\
	if (_val1 != _val2) {						\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 #expr1 " = " #expr2,				\
			 "Expected %lx, got %lx", _val1, _val2);	\
		return CMD_RET_FAILURE;					\
	}								\
	__ret;								\
})

/* Assert that a pointer is NULL */
#define ut_assertnull(expr) ({						\
	const void *_val = (expr);					\
	int __ret = 0;							\
									\
	if (_val) {							\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 #expr " != NULL",				\
			 "Expected NULL, got %p", _val);		\
		return CMD_RET_FAILURE;					\
	}								\
	__ret;								\
})

/* Assert that a pointer is not NULL */
#define ut_assertnonnull(expr) ({					\
	const void *_val = (expr);					\
	int __ret = 0;							\
									\
	if (!_val) {							\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 #expr " = NULL",				\
			 "Expected non-null, got NULL");		\
		return CMD_RET_FAILURE;					\
	}								\
	__ret;								\
})

/* Assert that a pointer is not an error pointer */
#define ut_assertok_ptr(expr) ({					\
	const void *_val = (expr);					\
	int __ret = 0;							\
									\
	if (IS_ERR(_val)) {						\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 #expr " = NULL",				\
			 "Expected pointer, got error %ld",		\
			 PTR_ERR(_val));				\
		return CMD_RET_FAILURE;					\
	}								\
	__ret;								\
})

/* Assert that an operation succeeds (returns 0) */
#define ut_assertok(cond)	ut_asserteq(0, cond)

/* Assert that the next console output line matches */
#define ut_assert_nextline(fmt, args...) ({				\
	int __ret = 0;							\
									\
	if (ut_check_console_line(uts, fmt, ##args)) {			\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 "console", "\nExpected '%s',\n     got '%s'",	\
			 uts->expect_str, uts->actual_str);		\
		return CMD_RET_FAILURE;					\
	}								\
	__ret;								\
})

/* Assert that the next console output line matches up to the length */
#define ut_assert_nextlinen(fmt, args...) ({				\
	int __ret = 0;							\
									\
	if (ut_check_console_linen(uts, fmt, ##args)) {			\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 "console", "\nExpected '%s',\n     got '%s'",	\
			 uts->expect_str, uts->actual_str);		\
		return CMD_RET_FAILURE;					\
	}								\
	__ret;								\
})

/* Assert that there is a 'next' console output line, and skip it */
#define ut_assert_skipline() ({						\
	int __ret = 0;							\
									\
	if (ut_check_skipline(uts)) {					\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 "console", "\nExpected a line, got end");	\
		return CMD_RET_FAILURE;					\
	}								\
	__ret;								\
})

/* Assert that a following console output line matches */
#define ut_assert_skip_to_line(fmt, args...) ({				\
	int __ret = 0;							\
									\
	if (ut_check_skip_to_line(uts, fmt, ##args)) {			\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 "console", "\nExpected '%s',\n     got to '%s'", \
			 uts->expect_str, uts->actual_str);		\
		return CMD_RET_FAILURE;					\
	}								\
	__ret;								\
})

/* Assert that a following console output line matches */
#define ut_assert_skip_to_linen(fmt, args...) ({				\
	int __ret = 0;							\
									\
	if (ut_check_skip_to_linen(uts, fmt, ##args)) {			\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 "console", "\nExpected '%s',\n     got to '%s'", \
			 uts->expect_str, uts->actual_str);		\
		return CMD_RET_FAILURE;					\
	}								\
	__ret;								\
})

/* Assert that there is no more console output */
#define ut_assert_console_end() ({					\
	int __ret = 0;							\
									\
	if (ut_check_console_end(uts)) {				\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 "console", "Expected no more output, got '%s'",\
			 uts->actual_str);				\
		return CMD_RET_FAILURE;					\
	}								\
	__ret;								\
})

/* Assert that the next lines are print_buffer() dump at an address */
#define ut_assert_nextlines_are_dump(total_bytes) ({			\
	int __ret = 0;							\
									\
	if (ut_check_console_dump(uts, total_bytes)) {			\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 "console",					\
			"Expected dump of length %x bytes, got '%s'",	\
			 total_bytes, uts->actual_str);			\
		return CMD_RET_FAILURE;					\
	}								\
	__ret;								\
})

/* Assert that the next console output line is empty */
#define ut_assert_nextline_empty()					\
	ut_assert_nextline("%s", "")

/**
 * ut_check_free() - Return the number of bytes free in the malloc() pool
 *
 * Return: bytes free
 */
ulong ut_check_free(void);

/**
 * ut_check_delta() - Return the number of bytes allocated/freed
 *
 * @last: Last value from ut_check_free
 * Return: free memory delta from @last; positive means more memory has been
 *	allocated, negative means less has been allocated (i.e. some is freed)
 */
long ut_check_delta(ulong last);

/**
 * ut_silence_console() - Silence the console if requested by the user
 *
 * This stops test output from appear on the console. It is the default on
 * sandbox, unless the -v flag is given. For other boards, this does nothing.
 *
 * @uts: Test state (in case in future we want to keep state here)
 */
void ut_silence_console(struct unit_test_state *uts);

/**
 * ut_unsilence_console() - Unsilence the console after a test
 *
 * This restarts console output again and turns off console recording. This
 * happens on all boards, including sandbox.
 */
void ut_unsilence_console(struct unit_test_state *uts);

/**
 * ut_set_skip_delays() - Sets whether delays should be skipped
 *
 * Normally functions like mdelay() cause U-Boot to wait for a while. This
 * allows all such delays to be skipped on sandbox, to speed up tests
 *
 * @uts: Test state (in case in future we want to keep state here)
 * @skip_delays: true to skip delays, false to process them normally
 */
void ut_set_skip_delays(struct unit_test_state *uts, bool skip_delays);

/**
 * test_get_state() - Get the active test state
 *
 * Return: the currently active test state, or NULL if none
 */
struct unit_test_state *test_get_state(void);

/**
 * test_set_state() - Set the active test state
 *
 * @uts: Test state to use as currently active test state, or NULL if none
 */
void test_set_state(struct unit_test_state *uts);

/**
 * ut_run_tests() - Run a set of tests
 *
 * This runs the test, handling any preparation and clean-up needed. It prints
 * the name of each test before running it.
 *
 * @category: Category of these tests. This is a string printed at the start to
 *	announce the the number of tests
 * @prefix: String prefix for the tests. Any tests that have this prefix will be
 *	printed without the prefix, so that it is easier to see the unique part
 *	of the test name. If NULL, no prefix processing is done
 * @tests: List of tests to run
 * @count: Number of tests to run
 * @select_name: Name of a single test to run (from the list provided). If NULL
 *	then all tests are run
 * @runs_per_test: Number of times to run each test (typically 1)
 * @force_run: Run tests that are marked as manual-only (UT_TESTF_MANUAL)
 * @test_insert: String describing a test to run after n other tests run, in the
 * format n:name where n is the number of tests to run before this one and
 * name is the name of the test to run. This is used to find which test causes
 * another test to fail. If the one test fails, testing stops immediately.
 * Pass NULL to disable this
 * Return: 0 if all tests passed, -1 if any failed
 */
int ut_run_list(const char *name, const char *prefix, struct unit_test *tests,
		int count, const char *select_name, int runs_per_test,
		bool force_run, const char *test_insert);

#endif
