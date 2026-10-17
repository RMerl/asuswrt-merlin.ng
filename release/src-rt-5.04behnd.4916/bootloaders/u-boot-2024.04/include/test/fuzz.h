/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2022 Google, Inc.
 * Written by Andrew Scull <ascull@google.com>
 */

#ifndef __TEST_FUZZ_H
#define __TEST_FUZZ_H

#include <linker_lists.h>
#include <linux/types.h>

/**
 * struct fuzz_test - Information about a fuzz test
 *
 * @name: Name of fuzz test
 * @func: Function to call to perform fuzz test on an input
 * @flags: Flags indicate pre-conditions for fuzz test
 */
struct fuzz_test {
	const char *name;
	int (*func)(const uint8_t * data, size_t size);
	int flags;
};

/**
 * FUZZ_TEST() - register a fuzz test
 *
 * The fuzz test function must return 0 as other values are reserved for future
 * use.
 *
 * @_name:	the name of the fuzz test function
 * @_flags:	an integer field that can be evaluated by the fuzzer
 * 		implementation
 */
#define FUZZ_TEST(_name, _flags)					\
	ll_entry_declare(struct fuzz_test, _name, fuzz_tests) = {	\
		.name = #_name,						\
		.func = _name,						\
		.flags = _flags,					\
	}

/** Get the start of the list of fuzz tests */
#define FUZZ_TEST_START() \
	ll_entry_start(struct fuzz_test, fuzz_tests)

/** Get the number of elements in the list of fuzz tests */
#define FUZZ_TEST_COUNT() \
	ll_entry_count(struct fuzz_test, fuzz_tests)

#endif /* __TEST_FUZZ_H */
