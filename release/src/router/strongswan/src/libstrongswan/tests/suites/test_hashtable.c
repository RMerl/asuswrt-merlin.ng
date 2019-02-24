/*
 * Copyright (C) 2010-2013 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
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

#include <collections/hashtable.h>
#include <utils/chunk.h>

/*******************************************************************************
 * string hash table functions
 */

static u_int hash(char *key)
{
	return chunk_hash(chunk_from_str(key));
}

static bool equals(char *key1, char *key2)
{
	return streq(key1, key2);
}

/*******************************************************************************
 * test fixture
 */

static hashtable_t *ht;

START_SETUP(setup_ht)
{
	ht = hashtable_create((hashtable_hash_t)hash,
						  (hashtable_equals_t)equals, 0);
	ck_assert_int_eq(ht->get_count(ht), 0);
}
END_SETUP

START_TEARDOWN(teardown_ht)
{
	ht->destroy(ht);
}
END_TEARDOWN

/*******************************************************************************
 * put/get
 */

START_TEST(test_put_get)
{
	char *k1 = "key1", *k2 = "key2", *k3 = "key3";
	char *v1 = "val1", *v2 = "val2", *v3 = "val3", *value;

	value = ht->put(ht, k1, v1);
	ck_assert_int_eq(ht->get_count(ht), 1);
	ck_assert(streq(ht->get(ht, k1), v1));
	ck_assert(ht->get(ht, k2) == NULL);
	ck_assert(ht->get(ht, k3) == NULL);
	ck_assert(value == NULL);

	ht->put(ht, k2, v2);
	ht->put(ht, k3, v3);
	ck_assert_int_eq(ht->get_count(ht), 3);
	ck_assert(streq(ht->get(ht, k1), v1));
	ck_assert(streq(ht->get(ht, k2), v2));
	ck_assert(streq(ht->get(ht, k3), v3));

	value = ht->put(ht, k2, v1);
	ck_assert_int_eq(ht->get_count(ht), 3);
	ck_assert(streq(value, v2));
	ck_assert(streq(ht->get(ht, k2), v1));
}
END_TEST

/*******************************************************************************
 * get_match
 */

static u_int hash_match(char *key)
{
	return chunk_hash(chunk_create(key, 4));
}

static bool equal_match(char *key1, char *key2)
{
	if (!strneq(key1, key2, 4))
	{
		return FALSE;
	}
	/* look for an item with a key < than what we look for */
	return strcmp(key1, key2) >= 0;
}

START_TEST(test_get_match)
{
	char *k1 = "key1_a", *k2 = "key2", *k3 = "key1_b", *k4 = "key1_c";
	char *v1 = "val1", *v2 = "val2", *v3 = "val3", *value;

	ht = hashtable_create((hashtable_hash_t)hash_match,
						  (hashtable_equals_t)equals, 0);

	ht->put(ht, k1, v1);
	ht->put(ht, k2, v2);
	value = ht->put(ht, k3, v3);
	ck_assert_int_eq(ht->get_count(ht), 3);
	ck_assert(streq(ht->get(ht, k1), v1));
	ck_assert(streq(ht->get(ht, k2), v2));
	ck_assert(streq(ht->get(ht, k3), v3));
	ck_assert(value == NULL);

	value = ht->get_match(ht, k1, (hashtable_equals_t)equal_match);
	ck_assert(value != NULL);
	ck_assert(streq(value, v1));
	value = ht->get_match(ht, k2, (hashtable_equals_t)equal_match);
	ck_assert(value != NULL);
	ck_assert(streq(value, v2));
	value = ht->get_match(ht, k3, (hashtable_equals_t)equal_match);
	ck_assert(value != NULL);
	ck_assert(streq(value, v1));
	value = ht->get_match(ht, k4, (hashtable_equals_t)equal_match);
	ck_assert(value != NULL);
	ck_assert(streq(value, v1));

	ht->destroy(ht);
}
END_TEST

/*******************************************************************************
 * remove
 */

static void do_remove(char *k1, char *k2, char *k3)
{
	char *v1 = "val1", *v2 = "val2", *v3 = "val3", *value;

	ht->put(ht, k1, v1);
	ht->put(ht, k2, v2);
	ht->put(ht, k3, v3);

	value = ht->remove(ht, k2);
	ck_assert_int_eq(ht->get_count(ht), 2);
	ck_assert(streq(ht->get(ht, k1), v1));
	ck_assert(streq(ht->get(ht, k3), v3));
	ck_assert(streq(value, v2));
	ck_assert(ht->get(ht, k2) == NULL);

	value = ht->remove(ht, k2);
	ck_assert_int_eq(ht->get_count(ht), 2);
	ck_assert(value == NULL);

	value = ht->remove(ht, k1);
	value = ht->remove(ht, k3);
	ck_assert_int_eq(ht->get_count(ht), 0);
	ck_assert(ht->get(ht, k1) == NULL);
	ck_assert(ht->get(ht, k2) == NULL);
	ck_assert(ht->get(ht, k3) == NULL);
}

START_TEST(test_remove)
{
	char *k1 = "key1", *k2 = "key2", *k3 = "key3";

	do_remove(k1, k2, k3);
}
END_TEST

START_TEST(test_remove_one_bucket)
{
	char *k1 = "key1_a", *k2 = "key1_b", *k3 = "key1_c";

	ht->destroy(ht);
	/* set a capacity to avoid rehashing, which would change the items' order */
	ht = hashtable_create((hashtable_hash_t)hash_match,
						  (hashtable_equals_t)equals, 8);

	do_remove(k1, k2, k3);
}
END_TEST

/*******************************************************************************
 * enumerator
 */

START_TEST(test_enumerator)
{
	char *k1 = "key1", *k2 = "key2", *k3 = "key3", *key;
	char *v1 = "val1", *v2 = "val2", *v3 = "val3", *value;
	enumerator_t *enumerator;
	int count;

	ht->put(ht, k1, v1);
	ht->put(ht, k2, v2);
	ht->put(ht, k3, v3);

	count = 0;
	enumerator = ht->create_enumerator(ht);
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		ck_assert(streq(key, k1) || streq(key, k2) || streq(key, k3));
		ck_assert(streq(value, v1) || streq(value, v2) || streq(value, v3));
		ck_assert(!streq(key, k1) || streq(value, v1));
		ck_assert(!streq(key, k2) || streq(value, v2));
		ck_assert(!streq(key, k3) || streq(value, v3));
		count++;
	}
	enumerator->destroy(enumerator);
	ck_assert_int_eq(count, 3);

	count = 0;
	enumerator = ht->create_enumerator(ht);
	while (enumerator->enumerate(enumerator, NULL, NULL))
	{
		count++;
	}
	enumerator->destroy(enumerator);
	ck_assert_int_eq(count, 3);

	value = ht->remove(ht, k1);
	value = ht->remove(ht, k2);
	value = ht->remove(ht, k3);

	count = 0;
	enumerator = ht->create_enumerator(ht);
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		count++;
	}
	enumerator->destroy(enumerator);
	ck_assert_int_eq(count, 0);
}
END_TEST

/*******************************************************************************
 * remove_at
 */

static void do_remove_at(char *k1, char *k2, char *k3)
{
	char *v1 = "val1", *v2 = "val2", *v3 = "val3", *value, *key;
	enumerator_t *enumerator;

	ht->put(ht, k1, v1);
	ht->put(ht, k2, v2);
	ht->put(ht, k3, v3);

	enumerator = ht->create_enumerator(ht);
	ht->remove_at(ht, enumerator);
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		if (streq(key, k2))
		{
			ht->remove_at(ht, enumerator);
		}
	}
	enumerator->destroy(enumerator);

	ck_assert_int_eq(ht->get_count(ht), 2);
	ck_assert(ht->get(ht, k1) != NULL);
	ck_assert(ht->get(ht, k3) != NULL);
	ck_assert(ht->get(ht, k2) == NULL);

	ht->put(ht, k2, v2);

	ck_assert_int_eq(ht->get_count(ht), 3);
	ck_assert(ht->get(ht, k1) != NULL);
	ck_assert(ht->get(ht, k2) != NULL);
	ck_assert(ht->get(ht, k3) != NULL);

	enumerator = ht->create_enumerator(ht);
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		ht->remove_at(ht, enumerator);
	}
	enumerator->destroy(enumerator);

	ck_assert_int_eq(ht->get_count(ht), 0);
	ck_assert(ht->get(ht, k1) == NULL);
	ck_assert(ht->get(ht, k2) == NULL);
	ck_assert(ht->get(ht, k3) == NULL);
}

START_TEST(test_remove_at)
{
	char *k1 = "key1", *k2 = "key2", *k3 = "key3";

	do_remove_at(k1, k2, k3);
}
END_TEST

START_TEST(test_remove_at_one_bucket)
{
	char *k1 = "key1_a", *k2 = "key1_b", *k3 = "key1_c";

	ht->destroy(ht);
	/* set a capacity to avoid rehashing, which would change the items' order */
	ht = hashtable_create((hashtable_hash_t)hash_match,
						  (hashtable_equals_t)equals, 8);
	do_remove_at(k1, k2, k3);
}
END_TEST

Suite *hashtable_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("hashtable");

	tc = tcase_create("put/get");
	tcase_add_checked_fixture(tc, setup_ht, teardown_ht);
	tcase_add_test(tc, test_put_get);
	suite_add_tcase(s, tc);

	tc = tcase_create("get_match");
	tcase_add_test(tc, test_get_match);
	suite_add_tcase(s, tc);

	tc = tcase_create("remove");
	tcase_add_checked_fixture(tc, setup_ht, teardown_ht);
	tcase_add_test(tc, test_remove);
	tcase_add_test(tc, test_remove_one_bucket);
	suite_add_tcase(s, tc);

	tc = tcase_create("enumerator");
	tcase_add_checked_fixture(tc, setup_ht, teardown_ht);
	tcase_add_test(tc, test_enumerator);
	suite_add_tcase(s, tc);

	tc = tcase_create("remove_at");
	tcase_add_checked_fixture(tc, setup_ht, teardown_ht);
	tcase_add_test(tc, test_remove_at);
	tcase_add_test(tc, test_remove_at_one_bucket);
	suite_add_tcase(s, tc);

	return s;
}
