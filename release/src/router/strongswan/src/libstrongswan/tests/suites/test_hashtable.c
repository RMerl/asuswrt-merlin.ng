/*
 * Copyright (C) 2010-2020 Tobias Brunner
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
 * hash table functions
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

/*******************************************************************************
 * test fixture
 */

static hashtable_t *ht;

typedef enum {
	/* regular string hash table */
	HASHTABLE_REGULAR,
	/* regular string hash list */
	HASHLIST_REGULAR,
	/* sorted string hash list */
	HASHLIST_REGULAR_SORTED,
	REGULAR_MAX,
	/* hash table with only 4 characters hashed -> one bucket tests */
	HASHTABLE_FUZZY = REGULAR_MAX,
	/* hash list with only 4 characters hashed */
	HASHLIST_FUZZY,
	/* sorted string hash list with only 4 characters hashed */
	HASHLIST_FUZZY_SORTED,
	HASHTABLE_MAX,
} hashtable_type_t;

/**
 * Create a specific hash table/list
 */
static hashtable_t *create_hashtable(int i)
{
	hashlist_t *hl = NULL;

	DESTROY_IF(ht);

	switch (i)
	{
		case HASHTABLE_REGULAR:
			ht = hashtable_create(hashtable_hash_str,
								  hashtable_equals_str, 0);
			break;
		case HASHLIST_REGULAR:
			hl = hashlist_create(hashtable_hash_str,
								 hashtable_equals_str, 0);
			break;
		case HASHLIST_REGULAR_SORTED:
			hl = hashlist_create_sorted(hashtable_hash_str,
									   (hashtable_cmp_t)strcmp, 0);
			break;
		case HASHTABLE_FUZZY:
			ht = hashtable_create((hashtable_hash_t)hash_match,
								  hashtable_equals_str, 0);
			break;
		case HASHLIST_FUZZY:
			hl = hashlist_create((hashtable_hash_t)hash_match,
								 hashtable_equals_str, 0);
			break;
		case HASHLIST_FUZZY_SORTED:
			hl = hashlist_create_sorted((hashtable_hash_t)hash_match,
										(hashtable_cmp_t)strcmp, 0);
			break;
	}
	if (hl)
	{
		ht = &hl->ht;
	}
	ck_assert_int_eq(ht->get_count(ht), 0);
	return ht;
}

START_SETUP(setup_ht)
{
	create_hashtable(_i);
}
END_SETUP

START_TEARDOWN(teardown_ht)
{
	ht->destroy(ht);
	ht = NULL;
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

START_TEST(test_get_match)
{
	hashlist_t *hl;
	char *k1 = "key1_a", *k2 = "key2", *k3 = "key1_b", *k4 = "key1_c";
	char *v1 = "val1", *v2 = "val2", *v3 = "val3", *value;

	hl = (hashlist_t*)create_hashtable(HASHLIST_FUZZY);

	ht->put(ht, k1, v1);
	ht->put(ht, k2, v2);
	value = ht->put(ht, k3, v3);
	ck_assert_int_eq(ht->get_count(ht), 3);
	ck_assert(streq(ht->get(ht, k1), v1));
	ck_assert(streq(ht->get(ht, k2), v2));
	ck_assert(streq(ht->get(ht, k3), v3));
	ck_assert(value == NULL);

	value = hl->get_match(hl, k1, (hashtable_equals_t)equal_match);
	ck_assert(value != NULL);
	ck_assert(streq(value, v1));
	value = hl->get_match(hl, k2, (hashtable_equals_t)equal_match);
	ck_assert(value != NULL);
	ck_assert(streq(value, v2));
	value = hl->get_match(hl, k3, (hashtable_equals_t)equal_match);
	ck_assert(value != NULL);
	ck_assert(streq(value, v1));
	value = hl->get_match(hl, k4, (hashtable_equals_t)equal_match);
	ck_assert(value != NULL);
	ck_assert(streq(value, v1));
}
END_TEST

START_TEST(test_get_match_remove)
{
	hashlist_t *hl;
	char *k1 = "key1_a", *k2 = "key2", *k3 = "key1_b", *k4 = "key1_c";
	char *v1 = "val1", *v2 = "val2", *v3 = "val3", *value;

	hl = (hashlist_t*)create_hashtable(HASHLIST_FUZZY);

	/* by removing and reinserting the first item we verify that insertion
	 * order is adhered */
	ht->put(ht, k1, v1);
	ht->put(ht, k2, v2);
	ht->put(ht, k3, v3);
	ht->remove(ht, k1);
	ht->put(ht, k1, v1);
	ck_assert_int_eq(ht->get_count(ht), 3);
	ck_assert(streq(ht->get(ht, k1), v1));
	ck_assert(streq(ht->get(ht, k2), v2));
	ck_assert(streq(ht->get(ht, k3), v3));

	value = hl->get_match(hl, k1, (hashtable_equals_t)equal_match);
	ck_assert(value != NULL);
	ck_assert(streq(value, v1));
	value = hl->get_match(hl, k2, (hashtable_equals_t)equal_match);
	ck_assert(value != NULL);
	ck_assert(streq(value, v2));
	value = hl->get_match(hl, k3, (hashtable_equals_t)equal_match);
	ck_assert(value != NULL);
	ck_assert(streq(value, v3));
	value = hl->get_match(hl, k4, (hashtable_equals_t)equal_match);
	ck_assert(value != NULL);
	ck_assert(streq(value, v3));
}
END_TEST

START_TEST(test_get_match_sorted)
{
	hashlist_t *hl;
	char *k1 = "key1_a", *k2 = "key2", *k3 = "key1_b", *k4 = "key1_c";
	char *v1 = "val1", *v2 = "val2", *v3 = "val3", *value;

	hl = (hashlist_t*)create_hashtable(HASHLIST_FUZZY_SORTED);

	/* since the keys are sorted, the insertion order doesn't matter */
	ht->put(ht, k3, v3);
	ht->put(ht, k2, v2);
	ht->put(ht, k1, v1);
	ht->put(ht, k4, v1);
	ht->remove(ht, k1);
	ht->put(ht, k1, v1);
	ck_assert_int_eq(ht->get_count(ht), 4);
	ck_assert(streq(ht->get(ht, k1), v1));
	ck_assert(streq(ht->get(ht, k2), v2));
	ck_assert(streq(ht->get(ht, k3), v3));
	ck_assert(streq(ht->get(ht, k4), v1));

	value = hl->get_match(hl, k1, (hashtable_equals_t)equal_match);
	ck_assert(value != NULL);
	ck_assert(streq(value, v1));
	value = hl->get_match(hl, k2, (hashtable_equals_t)equal_match);
	ck_assert(value != NULL);
	ck_assert(streq(value, v2));
	value = hl->get_match(hl, k3, (hashtable_equals_t)equal_match);
	ck_assert(value != NULL);
	ck_assert(streq(value, v1));
	value = hl->get_match(hl, k4, (hashtable_equals_t)equal_match);
	ck_assert(value != NULL);
	ck_assert(streq(value, v1));
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
	do_remove(k3, k2, k1);
	do_remove(k1, k3, k2);
}
END_TEST

START_TEST(test_remove_one_bucket)
{
	char *k1 = "key1_a", *k2 = "key1_b", *k3 = "key1_c";

	do_remove(k1, k2, k3);
	do_remove(k3, k2, k1);
	do_remove(k1, k3, k2);
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

START_TEST(test_enumerator_order)
{
	char *k1 = "key1", *k2 = "key2", *k3 = "key3", *key;
	char *v1 = "val1", *v2 = "val2", *v3 = "val3", *v4 = "val4", *value;
	enumerator_t *enumerator;
	int count;

	ht->put(ht, k1, v1);
	ht->put(ht, k2, v2);
	ht->put(ht, k3, v3);

	count = 0;
	enumerator = ht->create_enumerator(ht);
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		switch (count)
		{
			case 0:
				ck_assert(streq(key, k1) && streq(value, v1));
				break;
			case 1:
				ck_assert(streq(key, k2) && streq(value, v2));
				break;
			case 2:
				ck_assert(streq(key, k3) && streq(value, v3));
				break;
		}
		count++;
	}
	enumerator->destroy(enumerator);
	ck_assert_int_eq(count, 3);

	value = ht->remove(ht, k2);
	ht->put(ht, k2, v2);
	ht->put(ht, k1, v4);

	count = 0;
	enumerator = ht->create_enumerator(ht);
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		switch (count)
		{
			case 0:
				ck_assert(streq(key, k1) && streq(value, v4));
				break;
			case 1:
				ck_assert(streq(key, k3) && streq(value, v3));
				break;
			case 2:
				ck_assert(streq(key, k2) && streq(value, v2));
				break;
		}
		count++;
	}
	enumerator->destroy(enumerator);
	ck_assert_int_eq(count, 3);
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

	do_remove_at(k1, k2, k3);
}
END_TEST

/*******************************************************************************
 * many items
 */

static u_int hash_int(int *key)
{
	return chunk_hash(chunk_create((u_char*)key, sizeof(int)));
}

static bool equals_int(int *key1, int *key2)
{
	return *key1 == *key2;
}

static int cmp_int(int *key1, int *key2)
{
	return *key1 - *key2;
}

/**
 * Create a specific hash table with integers as keys.
 */
static hashtable_t *create_int_hashtable(int i)
{
	hashlist_t *hl = NULL;

	DESTROY_IF(ht);

	switch (i)
	{
		case HASHTABLE_REGULAR:
			ht = hashtable_create((hashtable_hash_t)hash_int,
								  (hashtable_equals_t)equals_int, 0);
			break;
		case HASHLIST_REGULAR:
			hl = hashlist_create((hashtable_hash_t)hash_int,
								 (hashtable_equals_t)equals_int, 0);
			break;
		case HASHLIST_REGULAR_SORTED:
			hl = hashlist_create_sorted((hashtable_hash_t)hash_int,
										(hashtable_cmp_t)cmp_int, 0);
			break;
	}
	if (hl)
	{
		ht = &hl->ht;
	}
	ck_assert_int_eq(ht->get_count(ht), 0);
	return ht;
}

START_SETUP(setup_ht_many)
{
	create_int_hashtable(_i >> 1);
}
END_SETUP

START_SETUP(setup_ht_lookups)
{
	create_int_hashtable(_i);
}
END_SETUP

START_TEARDOWN(teardown_ht_many)
{
	ht->destroy_function(ht, (void*)free);
	ht = NULL;
}
END_TEARDOWN

START_TEST(test_many_items)
{
	u_int count = 100000;
	int i, *val, r;

#define GET_VALUE(i) ({ (_i % 2) == 0 ? i : (count-1-i); })

	for (i = 0; i < count; i++)
	{
		val = malloc_thing(int);
		*val = GET_VALUE(i);
		ht->put(ht, val, val);
	}
	for (i = 0; i < count; i++)
	{
		r = GET_VALUE(i);
		val = ht->get(ht, &r);
		ck_assert_int_eq(GET_VALUE(i), *val);
	}
	ck_assert_int_eq(count, ht->get_count(ht));
	for (i = 0; i < count; i++)
	{
		r = GET_VALUE(i);
		free(ht->remove(ht, &r));
	}
	ck_assert_int_eq(0, ht->get_count(ht));
	for (i = 0; i < count; i++)
	{
		val = malloc_thing(int);
		*val = GET_VALUE(i);
		ht->put(ht, val, val);
	}
	for (i = 0; i < count/2; i++)
	{
		free(ht->remove(ht, &i));
	}
	ck_assert_int_eq(count/2, ht->get_count(ht));
	for (i = 0; i < count; i++)
	{
		val = malloc_thing(int);
		*val = GET_VALUE(i);
		free(ht->put(ht, val, val));
	}
	srandom(666);
	for (i = 0; i < count; i++)
	{
		r = random() % count;
		ht->get(ht, &r);
	}
	for (i = 0; i < count; i++)
	{
		free(ht->remove(ht, &i));
	}
	ck_assert_int_eq(0, ht->get_count(ht));
	for (i = 0; i < 2*count; i++)
	{
		val = malloc_thing(int);
		*val = i;
		ht->put(ht, val, val);
		free(ht->remove(ht, val));
	}
}
END_TEST

START_TEST(test_many_lookups_success)
{
	u_int count = 25000, lookups = 1000000;
	int i, *val, r;

	for (i = 0; i < count; i++)
	{
		val = malloc_thing(int);
		*val = i;
		ht->put(ht, val, val);
	}
	srandom(666);
	for (i = 0; i < lookups; i++)
	{
		r = random() % count;
		ht->get(ht, &r);
	}
}
END_TEST

START_TEST(test_many_lookups_failure_larger)
{
	u_int count = 25000, lookups = 1000000;
	int i, *val, r;

	for (i = 0; i < count; i++)
	{
		val = malloc_thing(int);
		*val = i;
		ht->put(ht, val, val);
	}
	srandom(666);
	for (i = 0; i < lookups; i++)
	{
		r = random() % count + count;
		ht->get(ht, &r);
	}
}
END_TEST

START_TEST(test_many_lookups_failure_smaller)
{
	u_int count = 25000, lookups = 1000000;
	int i, *val, r;

	for (i = 0; i < count; i++)
	{
		val = malloc_thing(int);
		*val = i + count;
		ht->put(ht, val, val);
	}
	srandom(666);
	for (i = 0; i < lookups; i++)
	{
		r = random() % count;
		ht->get(ht, &r);
	}
}
END_TEST

Suite *hashtable_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("hashtable");

	tc = tcase_create("put/get");
	tcase_add_checked_fixture(tc, setup_ht, teardown_ht);
	tcase_add_loop_test(tc, test_put_get, 0, HASHTABLE_MAX);
	suite_add_tcase(s, tc);

	tc = tcase_create("get_match");
	tcase_add_checked_fixture(tc, NULL, teardown_ht);
	tcase_add_test(tc, test_get_match);
	tcase_add_test(tc, test_get_match_remove);
	tcase_add_test(tc, test_get_match_sorted);
	suite_add_tcase(s, tc);

	tc = tcase_create("remove");
	tcase_add_checked_fixture(tc, setup_ht, teardown_ht);
	tcase_add_loop_test(tc, test_remove, 0, REGULAR_MAX);
	tcase_add_loop_test(tc, test_remove_one_bucket, HASHTABLE_FUZZY, HASHTABLE_MAX);
	suite_add_tcase(s, tc);

	tc = tcase_create("enumerator");
	tcase_add_checked_fixture(tc, setup_ht, teardown_ht);
	tcase_add_loop_test(tc, test_enumerator, 0, HASHTABLE_MAX);
	tcase_add_test(tc, test_enumerator_order);
	suite_add_tcase(s, tc);

	tc = tcase_create("remove_at");
	tcase_add_checked_fixture(tc, setup_ht, teardown_ht);
	tcase_add_loop_test(tc, test_remove_at, 0, REGULAR_MAX);
	tcase_add_loop_test(tc, test_remove_at_one_bucket, HASHTABLE_FUZZY, HASHTABLE_MAX);
	suite_add_tcase(s, tc);

	tc = tcase_create("many items");
	tcase_add_checked_fixture(tc, setup_ht_many, teardown_ht_many);
	tcase_set_timeout(tc, 10);
	tcase_add_loop_test(tc, test_many_items, 0, REGULAR_MAX << 1);
	suite_add_tcase(s, tc);

	tc = tcase_create("many lookups");
	tcase_add_checked_fixture(tc, setup_ht_lookups, teardown_ht_many);
	tcase_add_loop_test(tc, test_many_lookups_success, 0, REGULAR_MAX);
	tcase_add_loop_test(tc, test_many_lookups_failure_larger, 0, REGULAR_MAX);
	tcase_add_loop_test(tc, test_many_lookups_failure_smaller, 0, REGULAR_MAX);
	suite_add_tcase(s, tc);

	return s;
}
