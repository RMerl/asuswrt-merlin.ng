// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2019
 * Roman Kapl, SYSGO, rka@sysgo.com
 */

#include <common.h>
#include <command.h>
#include <log.h>
#include <search.h>
#include <stdio.h>
#include <test/env.h>
#include <test/ut.h>

#define SIZE 32
#define ITERATIONS 10000

static int htab_fill(struct unit_test_state *uts,
		     struct hsearch_data *htab, size_t size)
{
	size_t i;
	struct env_entry item;
	struct env_entry *ritem;
	char key[20];

	for (i = 0; i < size; i++) {
		sprintf(key, "%d", (int)i);
		item.callback = NULL;
		item.data = key;
		item.flags = 0;
		item.key = key;
		ut_asserteq(1, hsearch_r(item, ENV_ENTER, &ritem, htab, 0));
	}

	return 0;
}

static int htab_check_fill(struct unit_test_state *uts,
			   struct hsearch_data *htab, size_t size)
{
	size_t i;
	struct env_entry item;
	struct env_entry *ritem;
	char key[20];

	for (i = 0; i < size; i++) {
		sprintf(key, "%d", (int)i);
		item.callback = NULL;
		item.flags = 0;
		item.data = key;
		item.key = key;
		hsearch_r(item, ENV_FIND, &ritem, htab, 0);
		ut_assert(ritem);
		ut_asserteq_str(key, ritem->key);
		ut_asserteq_str(key, ritem->data);
	}

	return 0;
}

static int htab_create_delete(struct unit_test_state *uts,
			      struct hsearch_data *htab, size_t iterations)
{
	size_t i;
	struct env_entry item;
	struct env_entry *ritem;
	char key[20];

	for (i = 0; i < iterations; i++) {
		sprintf(key, "cd-%d", (int)i);
		item.callback = NULL;
		item.flags = 0;
		item.data = key;
		item.key = key;
		hsearch_r(item, ENV_ENTER, &ritem, htab, 0);
		ritem = NULL;

		hsearch_r(item, ENV_FIND, &ritem, htab, 0);
		ut_assert(ritem);
		ut_asserteq_str(key, ritem->key);
		ut_asserteq_str(key, ritem->data);

		ut_asserteq(0, hdelete_r(key, htab, 0));
	}

	return 0;
}

/* Completely fill up the hash table */
static int env_test_htab_fill(struct unit_test_state *uts)
{
	struct hsearch_data htab;

	memset(&htab, 0, sizeof(htab));
	ut_asserteq(1, hcreate_r(SIZE, &htab));

	ut_assertok(htab_fill(uts, &htab, SIZE));
	ut_assertok(htab_check_fill(uts, &htab, SIZE));
	ut_asserteq(SIZE, htab.filled);

	hdestroy_r(&htab);
	return 0;
}

ENV_TEST(env_test_htab_fill, 0);

/* Fill the hashtable up halfway an repeateadly delete/create elements
 * and check for corruption
 */
static int env_test_htab_deletes(struct unit_test_state *uts)
{
	struct hsearch_data htab;

	memset(&htab, 0, sizeof(htab));
	ut_asserteq(1, hcreate_r(SIZE, &htab));

	ut_assertok(htab_fill(uts, &htab, SIZE / 2));
	ut_assertok(htab_create_delete(uts, &htab, ITERATIONS));
	ut_assertok(htab_check_fill(uts, &htab, SIZE / 2));
	ut_asserteq(SIZE / 2, htab.filled);

	hdestroy_r(&htab);
	return 0;
}

ENV_TEST(env_test_htab_deletes, 0);
