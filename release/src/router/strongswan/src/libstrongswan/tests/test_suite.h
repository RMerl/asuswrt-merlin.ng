/*
 * Copyright (C) 2013 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup test_suite test_suite
 * @{ @ingroup libtest
 */

#ifndef TEST_SUITE_H_
#define TEST_SUITE_H_

#define _GNU_SOURCE
#include <setjmp.h>

#include <library.h>
#include <utils/debug.h>
#include <utils/backtrace.h>
#include <collections/array.h>

typedef struct test_suite_t test_suite_t;
typedef struct test_case_t test_case_t;
typedef struct test_function_t test_function_t;
typedef struct test_fixture_t test_fixture_t;

/**
 * Default timeout for a single test function
 */
#ifndef TEST_FUNCTION_DEFAULT_TIMEOUT
#define TEST_FUNCTION_DEFAULT_TIMEOUT 2
#endif

/**
 * Test function implementation
 */
typedef void (*test_function_cb_t)(int);

/**
 * Fixture for a test case.
 */
typedef void (*test_fixture_cb_t)(void);

/**
 * A test suite; a collection of test cases with fixtures
 */
struct test_suite_t {
	/** name of the test suite */
	const char *name;
	/** test cases registered, as test_case_t* */
	array_t *tcases;
};

/**
 * A test case; multiple test functions using the same fixtures
 */
struct test_case_t {
	/** name of the test case */
	const char *name;
	/** tests registered, as test_function_t */
	array_t *functions;
	/** fixture for tests, as test_fixture_t */
	array_t *fixtures;
	/** timeout for each function, in s */
	int timeout;
};

/**
 * A test function, with optional loop setup
 */
struct test_function_t {
	/** name of test function */
	char *name;
	/** tests function registered, test_function_t* */
	test_function_cb_t cb;
	/** start for loop test */
	int start;
	/** end for loop test */
	int end;
};

/**
 * Registered fixture for a test case
 */
struct test_fixture_t {
	test_fixture_cb_t setup;
	test_fixture_cb_t teardown;
};

/**
 * Create a new test suite
 *
 * @param name		name of the test suite
 * @return			test suite
 */
test_suite_t* test_suite_create(const char *name);

/**
 * Create a new test case
 *
 * @param name		name of test case
 * @return			test case
 */
test_case_t* test_case_create(const char *name);

/**
 * Add a setup/teardown function to the test case
 *
 * @param tcase		test case to add a fixture to
 * @param setup		setup function called before each test
 * @param teardown	cleanup function called after each test
 */
void test_case_add_checked_fixture(test_case_t *tcase, test_fixture_cb_t setup,
								   test_fixture_cb_t teardown);

/**
 * Add a test function to a test case, with a name, looped several times
 *
 * @param name		name of the test case
 * @param tcase		test case to add test function to
 * @param cb		callback function to invoke for test
 * @param start		start of loop counter
 * @param end		end of loop counter
 */
void test_case_add_test_name(test_case_t *tcase, char *name,
							 test_function_cb_t cb, int start, int end);

/**
 * Add a test function to a test case
 *
 * @param tcase		test case to add test function to
 * @param cb		callback function to invoke for test
 */
#define test_case_add_test(tcase, cb) \
	test_case_add_test_name(tcase, #cb, cb, 0, 1)

/**
 * Add a test function to a test case, looped several times
 *
 * @param tcase		test case to add test function to
 * @param cb		callback function to invoke for test
 * @param start		start of loop counter
 * @param end		end of loop counter
 */
#define test_case_add_loop_test(tcase, cb, start, end) \
	test_case_add_test_name(tcase, #cb, cb, start, end)

/**
 * Set a custom timeout for test functions in a test case
 *
 * @param tcase		test case to set timeout for
 * @param s			test timeout in s
 */
void test_case_set_timeout(test_case_t *tcase, int s);

/**
 * Add a test function to a test case, looped several times
 *
 * @param suite		test suite to add test case to
 * @param tcase		test case to add
 */
void test_suite_add_case(test_suite_t *suite, test_case_t *tcase);

/**
 * sigjmp restore point used by test_restore_point
 */
#ifdef WIN32
extern jmp_buf test_restore_point_env;
#else
extern sigjmp_buf test_restore_point_env;
#endif

/**
 * Set or return from an execution restore point
 *
 * This call sets a restore execution point and returns TRUE after it has
 * been set up. On test failure, the execution is returned to the restore point
 * and FALSE is returned to indicate test failure.
 *
 * @return			TRUE if restore point set, FALSE when restored
 */
#ifdef WIN32
# define test_restore_point() (setjmp(test_restore_point_env) == 0)
#else
# define test_restore_point() (sigsetjmp(test_restore_point_env, 1) == 0)
#endif

/**
 * Set up signal handlers for test cases
 */
void test_setup_handler();

/**
 * Set up a timeout to let a test fail
 *
 * @param s			timeout, 0 to disable timeout
 */
void test_setup_timeout(int s);

/**
 * Get info about a test failure
 *
 * @param msg		buffer receiving failure info
 * @param len		size of msg buffer
 * @param file		pointer receiving source code file
 * @return			source code line number
 */
int test_failure_get(char *msg, int len, const char **file);

/**
 * Get info about a warning if one was issued during the test. Resets the
 * warning state.
 *
 * @param msg		buffer receiving warning
 * @param len		size of msg buffer
 * @param file		pointer receiving source code file
 * @return			source code line number, 0 if no warning issued
 */
int test_warning_get(char *msg, int len, const char **file);

/**
 * Get a backtrace for a failure.
 *
 * @return			allocated backtrace of test failure, if any
 */
backtrace_t *test_failure_backtrace();

/**
 * Let a test fail and set a message using vprintf style arguments.
 *
 * @param file		source code file name
 * @param line		source code line number
 * @param fmt		printf format string
 * @param args		argument list for fmt
 */
void test_fail_vmsg(const char *file, int line, char *fmt, va_list args);

/**
 * Let a test fail and set a message using printf style arguments.
 *
 * @param file		source code file name
 * @param line		source code line number
 * @param fmt		printf format string
 * @param ...		arguments for fmt
 */
void test_fail_msg(const char *file, int line, char *fmt, ...);

/**
 * Issue a warning for a particular test with a message using printf style
 * arguments. This does not fail the test, and only the last warning for each
 * test is kept.
 *
 * @param file		source code file name
 * @param line		source code line number
 * @param fmt		printf format string
 * @param ...		arguments for fmt
 */
void test_warn_msg(const char *file, int line, char *fmt, ...);

/**
 * Let a test fail if one of the worker threads has failed (only if called from
 * the main thread).
 */
void test_fail_if_worker_failed();

/**
 * Check if two integers equal, fail test if not
 *
 * @param a			first integer
 * @param b			second integer
 */
#define test_int_eq(a, b) \
({ \
	typeof(a) _a = a; \
	typeof(b) _b = b; \
	test_fail_if_worker_failed(); \
	if (_a != _b) \
	{ \
		test_fail_msg(__FILE__, __LINE__, #a " != " #b " (%d != %d)", _a, _b); \
	} \
})

/**
 * Check if two strings equal, fail test if not
 *
 * @param a			first string
 * @param b			second string
 */
#define test_str_eq(a, b) \
({ \
	char* _a = (char*)a; \
	char* _b = (char*)b; \
	test_fail_if_worker_failed(); \
	if (!_a || !_b || !streq(_a, _b)) \
	{ \
		test_fail_msg(__FILE__, __LINE__, \
					  #a " != " #b " (\"%s\" != \"%s\")", _a, _b); \
	} \
})

/**
 * Check if two chunks are equal, fail test if not
 *
 * @param a			first chunk
 * @param b			second chunk
 */
#define test_chunk_eq(a, b) \
({ \
	chunk_t _a = (chunk_t)a; \
	chunk_t _b = (chunk_t)b; \
	test_fail_if_worker_failed(); \
	if (_a.len != _b.len || !memeq(_a.ptr, _b.ptr, _a.len)) \
	{ \
		test_fail_msg(__FILE__, __LINE__, \
					  #a " != " #b " (\"%#B\" != \"%#B\")", &_a, &_b); \
	} \
})

/**
 * Check if a statement evaluates to TRUE, fail test if not
 *
 * @param x			statement to evaluate
 */
#define test_assert(x) \
({ \
	test_fail_if_worker_failed(); \
	if (!(x)) \
	{ \
		test_fail_msg(__FILE__, __LINE__, "%s", #x); \
	} \
})

/**
 * Check if a statement evaluates to TRUE, fail and print a message if not
 *
 * @param x			statement to evaluate
 * @param fmt		message format string
 * @param ...		fmt printf arguments
 */
#define test_assert_msg(x, fmt, ...) \
({ \
	test_fail_if_worker_failed(); \
	if (!(x)) \
	{ \
		test_fail_msg(__FILE__, __LINE__, "%s: " fmt, #x, ##__VA_ARGS__); \
	} \
})



/* "check unit testing" compatibility */
#define Suite test_suite_t
#define TCase test_case_t
#define ck_assert_int_eq test_int_eq
#define ck_assert test_assert
#define ck_assert_msg test_assert_msg
#define ck_assert_str_eq test_str_eq
#define ck_assert_chunk_eq test_chunk_eq
#define warn(fmt, ...) test_warn_msg(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define fail(fmt, ...) test_fail_msg(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define fail_if(x, fmt, ...) \
({ \
	test_fail_if_worker_failed(); \
	if (x) \
	{ \
		test_fail_msg(__FILE__, __LINE__, "%s : " fmt, #x, ##__VA_ARGS__); \
	} \
})
#define fail_unless test_assert_msg
#define suite_create test_suite_create
#define tcase_create test_case_create
#define tcase_add_checked_fixture test_case_add_checked_fixture
#define tcase_add_test test_case_add_test
#define tcase_add_loop_test test_case_add_loop_test
#define tcase_set_timeout test_case_set_timeout
#define suite_add_tcase test_suite_add_case
#define START_TEST(name) static void name (int _i) {
#define END_TEST test_fail_if_worker_failed(); }
#define START_SETUP(name) static void name() {
#define END_SETUP test_fail_if_worker_failed(); }
#define START_TEARDOWN(name) static void name() {
#define END_TEARDOWN test_fail_if_worker_failed(); }

#endif /** TEST_SUITE_H_ @}*/
