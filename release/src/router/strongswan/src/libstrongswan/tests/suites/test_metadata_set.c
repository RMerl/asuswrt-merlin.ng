/*
 * Copyright (C) 2021 Tobias Brunner
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

#include <metadata/metadata_set.h>

START_TEST(test_destroy_null)
{
	metadata_set_t *set = NULL;
	metadata_set_destroy(set);
}
END_TEST

START_TEST(test_destroy_empty)
{
	metadata_set_t *set = metadata_set_create();
	metadata_set_destroy(set);
}
END_TEST

START_TEST(test_put_null)
{
	metadata_set_t *set = NULL;
	metadata_t *metadata;

	metadata = lib->metadata->create(lib->metadata, "int", 42);
	metadata_set_put(set, "key", metadata);
	metadata_set_put(set, "other", NULL);
}
END_TEST

START_TEST(test_put)
{
	metadata_set_t *set = metadata_set_create();
	metadata_t *metadata;

	metadata = lib->metadata->create(lib->metadata, "int", 42);
	metadata_set_put(set, "key", metadata);
	metadata_set_destroy(set);
}
END_TEST

START_TEST(test_get_null)
{
	metadata_set_t *set = NULL;
	metadata_t *metadata;

	metadata = metadata_set_get(set, "key");
	ck_assert(!metadata);
}
END_TEST

/**
 * Assert that the given int metadata value is found with the given key.
 */
static void assert_int_value(metadata_set_t *set, const char *key, int expected)
{
	metadata_t *metadata;
	int value;

	metadata = metadata_set_get(set, key);
	ck_assert(metadata);
	metadata->get(metadata, &value);
	ck_assert_int_eq(expected, value);
}

START_TEST(test_get)
{
	metadata_set_t *set = metadata_set_create();
	metadata_t *metadata;

	metadata = lib->metadata->create(lib->metadata, "int", 42);
	metadata_set_put(set, "key", metadata);
	assert_int_value(set, "key", 42);
	metadata_set_destroy(set);
}
END_TEST

START_TEST(test_get_missing)
{
	metadata_set_t *set = metadata_set_create();
	metadata_t *metadata;

	metadata = metadata_set_get(set, "key");
	ck_assert(!metadata);

	metadata = lib->metadata->create(lib->metadata, "int", 42);
	metadata_set_put(set, "other", metadata);
	metadata = metadata_set_get(set, "key");
	ck_assert(!metadata);
	metadata_set_destroy(set);
}
END_TEST

START_TEST(test_get_multi)
{
	metadata_set_t *set = metadata_set_create();
	metadata_t *metadata;

	metadata = lib->metadata->create(lib->metadata, "int", 42);
	metadata_set_put(set, "key", metadata);
	metadata = lib->metadata->create(lib->metadata, "int", 0);
	metadata_set_put(set, "other", metadata);
	assert_int_value(set, "key", 42);
	assert_int_value(set, "other", 0);
	metadata_set_destroy(set);
}
END_TEST

START_TEST(test_put_replace)
{
	metadata_set_t *set = metadata_set_create();
	metadata_t *metadata;

	metadata = lib->metadata->create(lib->metadata, "int", 42);
	metadata_set_put(set, "whatever", metadata);
	metadata = lib->metadata->create(lib->metadata, "int", 0);
	metadata_set_put(set, "other", metadata);
	metadata = lib->metadata->create(lib->metadata, "int", 666);
	metadata_set_put(set, "other", metadata);
	assert_int_value(set, "whatever", 42);
	assert_int_value(set, "other", 666);
	metadata_set_destroy(set);
}
END_TEST

START_TEST(test_put_remove)
{
	metadata_set_t *set = metadata_set_create();
	metadata_t *metadata;

	metadata = lib->metadata->create(lib->metadata, "int", 42);
	metadata_set_put(set, "key", metadata);
	metadata = lib->metadata->create(lib->metadata, "int", 0);
	metadata_set_put(set, "other", metadata);
	metadata_set_put(set, "other", NULL);
	assert_int_value(set, "key", 42);
	metadata = metadata_set_get(set, "other");
	ck_assert(!metadata);
	metadata_set_put(set, "key", NULL);
	metadata = metadata_set_get(set, "key");
	ck_assert(!metadata);
	metadata_set_destroy(set);
}
END_TEST

START_TEST(test_put_remove_missing)
{
	metadata_set_t *set = metadata_set_create();
	metadata_t *metadata;

	metadata_set_put(set, "key", NULL);
	metadata = lib->metadata->create(lib->metadata, "int", 42);
	metadata_set_put(set, "key", metadata);
	assert_int_value(set, "key", 42);
	metadata_set_put(set, "key", NULL);
	metadata = metadata_set_get(set, "key");
	ck_assert(!metadata);
	metadata_set_put(set, "key", NULL);
	metadata_set_destroy(set);
}
END_TEST

START_TEST(test_clone_null)
{
	metadata_set_t *set = NULL, *clone;

	clone = metadata_set_clone(set);
	ck_assert(!clone);
}
END_TEST

START_TEST(test_clone_empty)
{
	metadata_set_t *set = metadata_set_create(), *clone;

	clone = metadata_set_clone(set);
	ck_assert(clone != set);

	metadata_set_destroy(clone);
	metadata_set_destroy(set);
}
END_TEST

START_TEST(test_clone)
{
	metadata_set_t *set = metadata_set_create(), *clone;
	metadata_t *metadata;
	struct {
		const char *key;
		int value;
	}  expected[] = {
		{ "key", 42, },
		{ "other", 666, },
		{ "abc", 4500, },
	};
	int i;

	for (i = 0; i < countof(expected); i++)
	{
		metadata = lib->metadata->create(lib->metadata, "int", expected[i].value);
		metadata_set_put(set, expected[i].key, metadata);
	}

	clone = metadata_set_clone(set);
	ck_assert(clone != set);

	for (i = 0; i < countof(expected); i++)
	{
		assert_int_value(set, expected[i].key, expected[i].value);
		assert_int_value(clone, expected[i].key, expected[i].value);
	}

	metadata_set_put(set, expected[0].key, NULL);
	assert_int_value(clone, expected[0].key, expected[0].value);

	metadata_set_destroy(clone);
	metadata_set_destroy(set);
}
END_TEST

Suite *metadata_set_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("metadata_set");

	tc = tcase_create("create/destroy");
	tcase_add_test(tc, test_destroy_null);
	tcase_add_test(tc, test_destroy_empty);
	suite_add_tcase(s, tc);

	tc = tcase_create("put/get");
	tcase_add_test(tc, test_put_null);
	tcase_add_test(tc, test_put);
	tcase_add_test(tc, test_get_null);
	tcase_add_test(tc, test_get);
	tcase_add_test(tc, test_get_missing);
	tcase_add_test(tc, test_get_multi);
	tcase_add_test(tc, test_put_replace);
	tcase_add_test(tc, test_put_remove);
	tcase_add_test(tc, test_put_remove_missing);
	suite_add_tcase(s, tc);

	tc = tcase_create("clone");
	tcase_add_test(tc, test_clone_null);
	tcase_add_test(tc, test_clone_empty);
	tcase_add_test(tc, test_clone);
	suite_add_tcase(s, tc);

	return s;
}
