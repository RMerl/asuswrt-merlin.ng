/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Simple unit test library
 *
 * Copyright (c) 2013 Google, Inc
 */

#ifndef __TEST_UT_H
#define __TEST_UT_H

#include <linux/err.h>

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


/* Assert that a condition is non-zero */
#define ut_assert(cond)							\
	if (!(cond)) {							\
		ut_fail(uts, __FILE__, __LINE__, __func__, #cond);	\
		return CMD_RET_FAILURE;					\
	}

/* Assert that a condition is non-zero, with printf() string */
#define ut_assertf(cond, fmt, args...)					\
	if (!(cond)) {							\
		ut_failf(uts, __FILE__, __LINE__, __func__, #cond,	\
			 fmt, ##args);					\
		return CMD_RET_FAILURE;					\
	}

/* Assert that two int expressions are equal */
#define ut_asserteq(expr1, expr2) {					\
	unsigned int val1 = (expr1), val2 = (expr2);			\
									\
	if (val1 != val2) {						\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 #expr1 " == " #expr2,				\
			 "Expected %d, got %d", val1, val2);		\
		return CMD_RET_FAILURE;					\
	}								\
}

/* Assert that two string expressions are equal */
#define ut_asserteq_str(expr1, expr2) {					\
	const char *val1 = (expr1), *val2 = (expr2);			\
									\
	if (strcmp(val1, val2)) {					\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 #expr1 " = " #expr2,				\
			 "Expected \"%s\", got \"%s\"", val1, val2);	\
		return CMD_RET_FAILURE;					\
	}								\
}

/* Assert that two memory areas are equal */
#define ut_asserteq_mem(expr1, expr2, len) {				\
	const u8 *val1 = (u8 *)(expr1), *val2 = (u8 *)(expr2);		\
	const uint __len = len;						\
									\
	if (memcmp(val1, val2, __len)) {				\
		char __buf1[64 + 1] = "\0";				\
		char __buf2[64 + 1] = "\0";				\
		bin2hex(__buf1, val1, min(__len, (uint)32));		\
		bin2hex(__buf2, val2, min(__len, (uint)32));		\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 #expr1 " = " #expr2,				\
			 "Expected \"%s\", got \"%s\"",			\
			 __buf1, __buf2);				\
		return CMD_RET_FAILURE;					\
	}								\
}

/* Assert that two pointers are equal */
#define ut_asserteq_ptr(expr1, expr2) {					\
	const void *val1 = (expr1), *val2 = (expr2);			\
									\
	if (val1 != val2) {						\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 #expr1 " = " #expr2,				\
			 "Expected %p, got %p", val1, val2);		\
		return CMD_RET_FAILURE;					\
	}								\
}

/* Assert that a pointer is NULL */
#define ut_assertnull(expr) {					\
	const void *val = (expr);					\
									\
	if (val != NULL) {						\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 #expr " != NULL",				\
			 "Expected NULL, got %p", val);		\
		return CMD_RET_FAILURE;					\
	}								\
}

/* Assert that a pointer is not NULL */
#define ut_assertnonnull(expr) {					\
	const void *val = (expr);					\
									\
	if (val == NULL) {						\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 #expr " = NULL",				\
			 "Expected non-null, got NULL");		\
		return CMD_RET_FAILURE;					\
	}								\
}

/* Assert that a pointer is not an error pointer */
#define ut_assertok_ptr(expr) {						\
	const void *val = (expr);					\
									\
	if (IS_ERR(val)) {						\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 #expr " = NULL",				\
			 "Expected pointer, got error %ld",		\
			 PTR_ERR(val));					\
		return CMD_RET_FAILURE;					\
	}								\
}

/* Assert that an operation succeeds (returns 0) */
#define ut_assertok(cond)	ut_asserteq(0, cond)

#endif
