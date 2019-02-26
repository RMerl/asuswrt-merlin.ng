/*
 * Copyright (C) 2013 Tobias Brunner
 * Copyright (C) 2007 Martin Willi
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

#include <collections/enumerator.h>
#include <collections/linked_list.h>

/*******************************************************************************
 * token test
 */

static const char *token_results1[] = { "abc", "cde", "efg" };
static const char *token_results2[] = { "a", "b", "c" };

static struct {
	char *string;
	char *sep;
	char *trim;
	const char **results;
} token_tests[] = {
	{"abc, cde, efg", ",", " ", token_results1},
	{" abc 1:2 cde;3  4efg5.  ", ":;.,", " 12345", token_results1},
	{"abc.cde,efg", ",.", "", token_results1},
	{"  abc   cde  efg  ", " ", " ", token_results1},
	{"a'abc' c 'cde' cefg", " ", " abcd", token_results1},
	{"'abc' abc 'cde'd 'efg'", " ", " abcd", token_results1},

	{"a, b, c", ",", " ", token_results2},
	{"a,b,c", ",", " ", token_results2},
	{" a 1:2 b;3  4c5.  ", ":;.,", " 12345", token_results2},
	{"a.b,c", ",.", "", token_results2},
	{"  a   b  c  ", " ", " ", token_results2},
};

START_TEST(test_token)
{
	enumerator_t *enumerator;
	const char **results;
	char *token;
	int tok = 0;

	enumerator = enumerator_create_token(token_tests[_i].string,
									token_tests[_i].sep, token_tests[_i].trim);
	results = token_tests[_i].results;
	while (enumerator->enumerate(enumerator, &token))
	{
		switch (tok)
		{
			case 0:
			case 1:
			case 2:
				ck_assert_str_eq(token, results[tok]);
				break;
			default:
				fail("unexpected token '%s'", token);
		}
		tok++;
	}
	fail_if(tok != 3, "not enough tokens (%d) extracted from '%s'",
			tok, token_tests[_i].string);
	enumerator->destroy(enumerator);
}
END_TEST

/*******************************************************************************
 * utilities for filtered, nested and cleaner tests
 */

static int destroy_data_called;

START_SETUP(setup_destroy_data)
{
	destroy_data_called = 0;
}
END_SETUP

START_TEARDOWN(teardown_destroy_data)
{
	ck_assert_int_eq(destroy_data_called, 1);
}
END_TEARDOWN

static void destroy_data(void *data)
{
	fail_if(data != (void*)101, "data does not match '101' in destructor");
	destroy_data_called++;
}

/*******************************************************************************
 * filtered test
 */

CALLBACK(filter, bool,
	int *data, enumerator_t *orig, va_list args)
{
	int *item, *vo, *wo, *xo, *yo, *zo;

	VA_ARGS_VGET(args, vo, wo, xo, yo, zo);

	if (orig->enumerate(orig, &item))
	{
		int val = *item;
		*vo = val++;
		*wo = val++;
		*xo = val++;
		*yo = val++;
		*zo = val++;
		fail_if(data != (void*)101, "data does not match '101' in filter function");
		return TRUE;
	}
	return FALSE;
}

CALLBACK(filter_odd, bool,
	void *data, enumerator_t *orig, va_list args)
{
	int *item, *out;

	VA_ARGS_VGET(args, out);

	fail_if(data != (void*)101, "data does not match '101' in filter function");

	while (orig->enumerate(orig, &item))
	{
		if (*item % 2 == 0)
		{
			*out = *item;
			return TRUE;
		}
	}
	return FALSE;
}

START_TEST(test_filtered)
{
	int data[5] = {1,2,3,4,5}, round, v, w, x, y, z;
	linked_list_t *list;
	enumerator_t *enumerator;

	list = linked_list_create_with_items(&data[0], &data[1], &data[2], &data[3],
										 &data[4], NULL);

	round = 1;
	enumerator = enumerator_create_filter(list->create_enumerator(list),
										  filter, (void*)101, destroy_data);
	while (enumerator->enumerate(enumerator, &v, &w, &x, &y, &z))
	{
		ck_assert_int_eq(v, round);
		ck_assert_int_eq(w, round + 1);
		ck_assert_int_eq(x, round + 2);
		ck_assert_int_eq(y, round + 3);
		ck_assert_int_eq(z, round + 4);
		round++;
	}
	enumerator->destroy(enumerator);
	ck_assert_int_eq(round, 6);

	list->destroy(list);
}
END_TEST

START_TEST(test_filtered_filter)
{
	int data[5] = {1,2,3,4,5}, count, x;
	linked_list_t *list;
	enumerator_t *enumerator;

	list = linked_list_create_with_items(&data[0], &data[1], &data[2], &data[3],
										 &data[4], NULL);

	count = 0;
	/* should also work without destructor, so set this manually */
	destroy_data_called = 1;
	enumerator = enumerator_create_filter(list->create_enumerator(list),
										  filter_odd, (void*)101, NULL);
	while (enumerator->enumerate(enumerator, &x))
	{
		ck_assert(x % 2 == 0);
		count++;
	}
	enumerator->destroy(enumerator);
	ck_assert_int_eq(count, 2);

	list->destroy(list);
}
END_TEST

/*******************************************************************************
 * nested test
 */

static enumerator_t* create_inner(linked_list_t *outer, void *data)
{
	fail_if(data != (void*)101, "data does not match '101' in nested constr.");
	return outer->create_enumerator(outer);
}

static enumerator_t* create_inner_null(void *outer, void *data)
{
	ck_assert(outer == (void*)1);
	fail_if(data != (void*)101, "data does not match '101' in nested constr.");
	return NULL;
}

START_TEST(test_nested)
{
	linked_list_t *list, *l1, *l2, *l3;
	enumerator_t *enumerator;
	intptr_t x;
	int round;

	l1 = linked_list_create_with_items((void*)1, (void*)2, NULL);
	l2 = linked_list_create();
	l3 = linked_list_create_with_items((void*)3, (void*)4, (void*)5, NULL);
	list = linked_list_create_with_items(l1, l2, l3, NULL);

	round = 1;
	enumerator = enumerator_create_nested(list->create_enumerator(list),
								(void*)create_inner, (void*)101, destroy_data);
	while (enumerator->enumerate(enumerator, &x))
	{
		ck_assert_int_eq(round, x);
		round++;
	}
	enumerator->destroy(enumerator);
	ck_assert_int_eq(round, 6);

	list->destroy(list);
	l1->destroy(l1);
	l2->destroy(l2);
	l3->destroy(l3);
}
END_TEST

START_TEST(test_nested_reset)
{
	linked_list_t *list, *l1, *l2, *l3;
	enumerator_t *outer, *enumerator;
	intptr_t x;
	int count = 0;

	l1 = linked_list_create_with_items((void*)1, (void*)2, NULL);
	l2 = linked_list_create();
	l3 = linked_list_create_with_items((void*)3, (void*)4, (void*)5, NULL);
	list = linked_list_create_with_items(l1, l2, l3, NULL);

	outer = list->create_enumerator(list);
	enumerator = enumerator_create_nested(outer, (void*)create_inner,
										 (void*)101, destroy_data);
	while (enumerator->enumerate(enumerator, &x))
	{
		count++;
	}
	ck_assert_int_eq(count, 5);

	list->reset_enumerator(list, outer);
	ck_assert(enumerator->enumerate(enumerator, &x));
	ck_assert_int_eq(x, 1);
	enumerator->destroy(enumerator);

	list->destroy(list);
	l1->destroy(l1);
	l2->destroy(l2);
	l3->destroy(l3);
}
END_TEST

START_TEST(test_nested_empty)
{
	linked_list_t *list;
	enumerator_t *enumerator;
	intptr_t x;
	int count;

	list = linked_list_create();
	count = 0;
	enumerator = enumerator_create_nested(list->create_enumerator(list),
								(void*)create_inner, (void*)101, destroy_data);
	while (enumerator->enumerate(enumerator, &x))
	{
		count++;
	}
	enumerator->destroy(enumerator);
	ck_assert_int_eq(count, 0);

	list->destroy(list);
}
END_TEST

START_TEST(test_nested_null)
{
	linked_list_t *list;
	enumerator_t *enumerator;
	intptr_t x;
	int count;

	list = linked_list_create_with_items((void*)1, NULL);

	count = 0;
	/* should also work without destructor, so set this manually */
	destroy_data_called = 1;
	enumerator = enumerator_create_nested(list->create_enumerator(list),
									(void*)create_inner_null, (void*)101, NULL);
	while (enumerator->enumerate(enumerator, &x))
	{
		count++;
	}
	enumerator->destroy(enumerator);
	ck_assert_int_eq(count, 0);

	list->destroy(list);
}
END_TEST

/*******************************************************************************
 * cleaner test
 */

START_TEST(test_cleaner)
{
	enumerator_t *enumerator;
	linked_list_t *list;
	intptr_t x;
	int round;

	list = linked_list_create_with_items((void*)1, (void*)2, NULL);

	round = 1;
	enumerator = enumerator_create_cleaner(list->create_enumerator(list),
										   destroy_data, (void*)101);
	while (enumerator->enumerate(enumerator, &x))
	{
		ck_assert_int_eq(round, x);
		round++;
	}
	ck_assert_int_eq(round, 3);
	enumerator->destroy(enumerator);
	list->destroy(list);
}
END_TEST

/*******************************************************************************
 * single test
 */

static void single_cleanup(void *data)
{
	ck_assert_int_eq((intptr_t)data, 1);
}

static void do_test_single(enumerator_t *enumerator)
{
	intptr_t x;

	ck_assert(enumerator->enumerate(enumerator, &x));
	ck_assert_int_eq(x, 1);
	ck_assert(!enumerator->enumerate(enumerator, &x));
	enumerator->destroy(enumerator);
}

START_TEST(test_single)
{
	enumerator_t *enumerator;

	enumerator = enumerator_create_single((void*)1, NULL);
	do_test_single(enumerator);
}
END_TEST

START_TEST(test_single_cleanup)
{
	enumerator_t *enumerator;

	enumerator = enumerator_create_single((void*)1, single_cleanup);
	do_test_single(enumerator);
}
END_TEST

Suite *enumerator_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("enumerator");

	tc = tcase_create("tokens");
	tcase_add_loop_test(tc, test_token, 0, countof(token_tests));
	suite_add_tcase(s, tc);

	tc = tcase_create("filtered");
	tcase_add_checked_fixture(tc, setup_destroy_data, teardown_destroy_data);
	tcase_add_test(tc, test_filtered);
	tcase_add_test(tc, test_filtered_filter);
	suite_add_tcase(s, tc);

	tc = tcase_create("nested");
	tcase_add_checked_fixture(tc, setup_destroy_data, teardown_destroy_data);
	tcase_add_test(tc, test_nested);
	tcase_add_test(tc, test_nested_reset);
	tcase_add_test(tc, test_nested_empty);
	tcase_add_test(tc, test_nested_null);
	suite_add_tcase(s, tc);

	tc = tcase_create("cleaner");
	tcase_add_checked_fixture(tc, setup_destroy_data, teardown_destroy_data);
	tcase_add_test(tc, test_cleaner);
	suite_add_tcase(s, tc);

	tc = tcase_create("single");
	tcase_add_test(tc, test_single);
	tcase_add_test(tc, test_single_cleanup);
	suite_add_tcase(s, tc);

	return s;
}
