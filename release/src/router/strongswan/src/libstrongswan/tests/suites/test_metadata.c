/*
 * Copyright (C) 2021 Tobias Brunner, codelabs GmbH
 * Copyright (C) 2021 Thomas Egerer, secunet AG
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
