/*
 * Copyright (C) 2021 Tobias Brunner
 * Copyright (C) 2021 Thomas Egerer
 *
 * Copyright (C) secunet Security Networks AG
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

#include "test_suite.h"

static struct {
	const char *type;
	int value;
	short short_value;
} test_data_int[] = {
	{ "int", 3, 3 },
	{ "int", SHRT_MAX, SHRT_MAX },
	{ "int", INT_MAX, (short)INT_MAX },
};

/**
 * Compare the int value stored in the metadata object.
 */
static void equals_int(metadata_t *metadata, int _i)
{
	int got;

	ck_assert(metadata);
	ck_assert(metadata->equals(metadata, test_data_int[_i].value));
	ck_assert_str_eq(test_data_int[_i].type, metadata->get_type(metadata));
	metadata->get(metadata, &got);
	ck_assert_int_eq(test_data_int[_i].value, got);

	if (test_data_int[_i].value == test_data_int[_i].short_value)
	{
		ck_assert(metadata->equals(metadata, test_data_int[_i].short_value));
		ck_assert_int_eq(test_data_int[_i].short_value, got);
	}
	else
	{
		ck_assert(!metadata->equals(metadata, test_data_int[_i].short_value));
		ck_assert(test_data_int[_i].short_value != got);
	}
}

START_TEST(test_create_equals_int)
{
	metadata_t *metadata, *clone;

	metadata = lib->metadata->create(lib->metadata, test_data_int[_i].type,
									 test_data_int[_i].value);

	equals_int(metadata, _i);
	clone = metadata->clone(metadata);
	equals_int(clone, _i);

	clone->destroy(clone);
	metadata->destroy(metadata);
}
END_TEST

static struct {
	const char *type;
	uint64_t value;
	short short_value;
} test_data_uint64[] = {
	{ "uint64",	 3, 3 },
	{ "uint64",	 SHRT_MAX, SHRT_MAX },
	{ "uint64",	 UINT64_MAX, (short)UINT64_MAX },
};

/**
 * Compare the uint64_t value stored in the metadata object.
 */
static void equals_uint64(metadata_t *metadata, int _i)
{
	uint64_t got;

	ck_assert(metadata);
	ck_assert(metadata->equals(metadata, test_data_uint64[_i].value));
	ck_assert_str_eq(test_data_uint64[_i].type, metadata->get_type(metadata));
	metadata->get(metadata, &got);
	ck_assert_int_eq(test_data_uint64[_i].value, got);

	if (test_data_uint64[_i].value == test_data_uint64[_i].short_value)
	{
		ck_assert(metadata->equals(metadata, (uint64_t)test_data_uint64[_i].short_value));
		ck_assert_int_eq(test_data_uint64[_i].short_value, got);
	}
	else
	{
		ck_assert(!metadata->equals(metadata, (uint64_t)test_data_uint64[_i].short_value));
		ck_assert(test_data_uint64[_i].short_value != got);
	}
}

START_TEST(test_create_equals_uint64)
{
	metadata_t *metadata, *clone;

	metadata = lib->metadata->create(lib->metadata, test_data_uint64[_i].type,
									 test_data_uint64[_i].value);

	equals_uint64(metadata, _i);
	clone = metadata->clone(metadata);
	equals_uint64(clone, _i);

	clone->destroy(clone);
	metadata->destroy(metadata);
}
END_TEST

Suite *metadata_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("metadata");

	tc = tcase_create("integer types");
	tcase_add_loop_test(tc, test_create_equals_int, 0, countof(test_data_int));
	tcase_add_loop_test(tc, test_create_equals_uint64, 0, countof(test_data_uint64));
	suite_add_tcase(s, tc);

	return s;
}
