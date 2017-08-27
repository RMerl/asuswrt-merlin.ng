/*
 * Copyright (C) 2013 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
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

#include <collections/linked_list.h>

/*******************************************************************************
 * test fixture
 */

static linked_list_t *list;

START_SETUP(setup_list)
{
	void *x = NULL;

	list = linked_list_create();
	ck_assert_int_eq(list->get_count(list), 0);
	ck_assert(list->get_first(list, &x) == NOT_FOUND);
	ck_assert(list->get_last(list, &x) == NOT_FOUND);
}
END_SETUP

START_TEARDOWN(teardown_list)
{
	list->destroy(list);
}
END_TEARDOWN

/*******************************************************************************
 * insert first/last
 */

START_TEST(test_insert_first)
{
	void *a = (void*)1, *b = (void*)2, *x = NULL;

	list->insert_first(list, a);
	ck_assert_int_eq(list->get_count(list), 1);
	ck_assert(list->get_first(list, &x) == SUCCESS);
	ck_assert(x == a);
	ck_assert(list->get_last(list, &x) == SUCCESS);
	ck_assert(x == a);

	list->insert_first(list, b);
	ck_assert_int_eq(list->get_count(list), 2);
	ck_assert(list->get_first(list, &x) == SUCCESS);
	ck_assert(x == b);
	ck_assert(list->get_last(list, &x) == SUCCESS);
	ck_assert(x == a);
}
END_TEST

START_TEST(test_insert_last)
{
	void *a = (void*)1, *b = (void*)2, *x = NULL;

	list->insert_last(list, a);
	ck_assert_int_eq(list->get_count(list), 1);
	ck_assert(list->get_first(list, &x) == SUCCESS);
	ck_assert(x == a);
	ck_assert(list->get_last(list, &x) == SUCCESS);
	ck_assert(x == a);

	list->insert_last(list, b);
	ck_assert_int_eq(list->get_count(list), 2);
	ck_assert(list->get_first(list, &x) == SUCCESS);
	ck_assert(x == a);
	ck_assert(list->get_last(list, &x) == SUCCESS);
	ck_assert(x == b);
}
END_TEST

/*******************************************************************************
 * remove first/last
 */

START_TEST(test_remove_first)
{
	void *a = (void*)1, *b = (void*)2, *x = NULL;

	list->insert_first(list, a);
	list->insert_first(list, b);
	ck_assert(list->remove_first(list, &x) == SUCCESS);
	ck_assert_int_eq(list->get_count(list), 1);
	ck_assert(x == b);
	ck_assert(list->remove_first(list, &x) == SUCCESS);
	ck_assert_int_eq(list->get_count(list), 0);
	ck_assert(x == a);
	ck_assert(list->remove_first(list, &x) == NOT_FOUND);
	ck_assert(list->remove_last(list, &x) == NOT_FOUND);
}
END_TEST

START_TEST(test_remove_last)
{
	void *a = (void*)1, *b = (void*)2, *x = NULL;

	list->insert_first(list, a);
	list->insert_first(list, b);
	ck_assert(list->remove_last(list, &x) == SUCCESS);
	ck_assert_int_eq(list->get_count(list), 1);
	ck_assert(x == a);
	ck_assert(list->remove_last(list, &x) == SUCCESS);
	ck_assert_int_eq(list->get_count(list), 0);
	ck_assert(x == b);
	ck_assert(list->remove_first(list, &x) == NOT_FOUND);
	ck_assert(list->remove_last(list, &x) == NOT_FOUND);
}
END_TEST

/*******************************************************************************
 * helper function for remove and find tests
 */

static bool match_a(void *item, void *a)
{
	ck_assert(a == (void*)1);
	return item == a;
}

static bool match_b(void *item, void *b)
{
	ck_assert(b == (void*)2);
	return item == b;
}

/*******************************************************************************
 * remove
 */

START_TEST(test_remove)
{
	void *a = (void*)1, *b = (void*)2;

	list->insert_first(list, a);
	ck_assert(list->remove(list, a, NULL) == 1);
	ck_assert_int_eq(list->get_count(list), 0);

	list->insert_last(list, a);
	list->insert_last(list, a);
	list->insert_last(list, a);
	list->insert_last(list, b);
	ck_assert(list->remove(list, a, NULL) == 3);
	ck_assert(list->remove(list, a, NULL) == 0);
	ck_assert_int_eq(list->get_count(list), 1);
	ck_assert(list->remove(list, b, NULL) == 1);
	ck_assert(list->remove(list, b, NULL) == 0);
}
END_TEST

START_TEST(test_remove_callback)
{
	void *a = (void*)1, *b = (void*)2;

	list->insert_last(list, a);
	list->insert_last(list, b);
	list->insert_last(list, a);
	list->insert_last(list, b);
	ck_assert(list->remove(list, a, match_a) == 2);
	ck_assert(list->remove(list, a, match_a) == 0);
	ck_assert_int_eq(list->get_count(list), 2);
	ck_assert(list->remove(list, b, match_b) == 2);
	ck_assert(list->remove(list, b, match_b) == 0);
	ck_assert_int_eq(list->get_count(list), 0);
}
END_TEST

/*******************************************************************************
 * find
 */

static bool match_a_b(void *item, void *a, void *b)
{
	ck_assert(a == (void*)1);
	ck_assert(b == (void*)2);
	return item == a || item == b;
}

START_TEST(test_find)
{
	void *a = (void*)1, *b = (void*)2;

	ck_assert(list->find_first(list, NULL, &a) == NOT_FOUND);
	list->insert_last(list, a);
	ck_assert(list->find_first(list, NULL, &a) == SUCCESS);
	ck_assert(list->find_first(list, NULL, &b) == NOT_FOUND);
	list->insert_last(list, b);
	ck_assert(list->find_first(list, NULL, &a) == SUCCESS);
	ck_assert(list->find_first(list, NULL, &b) == SUCCESS);

	ck_assert(list->find_first(list, NULL, NULL) == NOT_FOUND);
}
END_TEST

START_TEST(test_find_callback)
{
	void *a = (void*)1, *b = (void*)2, *x = NULL;

	ck_assert(list->find_first(list, (linked_list_match_t)match_a_b, &x, a, b) == NOT_FOUND);
	list->insert_last(list, a);
	ck_assert(list->find_first(list, (linked_list_match_t)match_a, NULL, a) == SUCCESS);
	x = NULL;
	ck_assert(list->find_first(list, (linked_list_match_t)match_a, &x, a) == SUCCESS);
	ck_assert(a == x);
	ck_assert(list->find_first(list, (linked_list_match_t)match_b, &x, b) == NOT_FOUND);
	ck_assert(a == x);
	x = NULL;
	ck_assert(list->find_first(list, (linked_list_match_t)match_a_b, &x, a, b) == SUCCESS);
	ck_assert(a == x);

	list->insert_last(list, b);
	ck_assert(list->find_first(list, (linked_list_match_t)match_a, &x, a) == SUCCESS);
	ck_assert(a == x);
	ck_assert(list->find_first(list, (linked_list_match_t)match_b, &x, b) == SUCCESS);
	ck_assert(b == x);
	x = NULL;
	ck_assert(list->find_first(list, (linked_list_match_t)match_a_b, &x, a, b) == SUCCESS);
	ck_assert(a == x);
}
END_TEST

/*******************************************************************************
 * invoke
 */

typedef struct invoke_t invoke_t;

struct invoke_t {
	int val;
	void (*invoke)(invoke_t *item, void *a, void *b, void *c, void *d, int *sum);
};

static void invoke(intptr_t item, void *a, void *b, void *c, void *d, int *sum)
{
	ck_assert_int_eq((uintptr_t)a, 1);
	ck_assert_int_eq((uintptr_t)b, 2);
	ck_assert_int_eq((uintptr_t)c, 3);
	ck_assert_int_eq((uintptr_t)d, 4);
	*sum += item;
}

static void invoke_offset(invoke_t *item, void *a, void *b, void *c, void *d, int *sum)
{
	invoke(item->val, a, b, c, d, sum);
}

START_TEST(test_invoke_function)
{
	int sum = 0;

	list->insert_last(list, (void*)1);
	list->insert_last(list, (void*)2);
	list->insert_last(list, (void*)3);
	list->insert_last(list, (void*)4);
	list->insert_last(list, (void*)5);
	list->invoke_function(list, (linked_list_invoke_t)invoke,
						  (uintptr_t)1, (uintptr_t)2,
						  (uintptr_t)3, (uintptr_t)4, &sum);
	ck_assert_int_eq(sum, 15);
}
END_TEST

START_TEST(test_invoke_offset)
{
	invoke_t items[] = {
		{ .val = 1, .invoke = invoke_offset, },
		{ .val = 2, .invoke = invoke_offset, },
		{ .val = 3, .invoke = invoke_offset, },
		{ .val = 4, .invoke = invoke_offset, },
		{ .val = 5, .invoke = invoke_offset, },
	};
	int i, sum = 0;

	for (i = 0; i < countof(items); i++)
	{
		list->insert_last(list, &items[i]);
	}
	list->invoke_offset(list, offsetof(invoke_t, invoke),
						(uintptr_t)1, (uintptr_t)2,
						(uintptr_t)3, (uintptr_t)4, &sum);
	ck_assert_int_eq(sum, 15);
}
END_TEST

/*******************************************************************************
 * clone
 */

typedef struct clone_t clone_t;

struct clone_t {
	void *val;
	void *(*clone)(clone_t *item);
};

static void *clonefn(clone_t *item)
{
	return item->val;
}

static void test_clone(linked_list_t *list)
{
	intptr_t x;
	int round = 1;

	ck_assert_int_eq(list->get_count(list), 5);
	while (list->remove_first(list, (void*)&x) == SUCCESS)
	{
		ck_assert_int_eq(round, x);
		round++;
	}
	ck_assert_int_eq(round, 6);
}

START_TEST(test_clone_offset)
{
	linked_list_t *other;
	clone_t items[] = {
		{ .val = (void*)1, .clone = clonefn, },
		{ .val = (void*)2, .clone = clonefn, },
		{ .val = (void*)3, .clone = clonefn, },
		{ .val = (void*)4, .clone = clonefn, },
		{ .val = (void*)5, .clone = clonefn, },
	};
	int i;

	for (i = 0; i < countof(items); i++)
	{
		list->insert_last(list, &items[i]);
	}
	other = list->clone_offset(list, offsetof(clone_t, clone));
	test_clone(other);
	other->destroy(other);
}
END_TEST

Suite *linked_list_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("linked list");

	tc = tcase_create("insert/get");
	tcase_add_checked_fixture(tc, setup_list, teardown_list);
	tcase_add_test(tc, test_insert_first);
	tcase_add_test(tc, test_insert_last);
	suite_add_tcase(s, tc);

	tc = tcase_create("remove");
	tcase_add_checked_fixture(tc, setup_list, teardown_list);
	tcase_add_test(tc, test_remove_first);
	tcase_add_test(tc, test_remove_last);
	tcase_add_test(tc, test_remove);
	tcase_add_test(tc, test_remove_callback);
	suite_add_tcase(s, tc);

	tc = tcase_create("find");
	tcase_add_checked_fixture(tc, setup_list, teardown_list);
	tcase_add_test(tc, test_find);
	tcase_add_test(tc, test_find_callback);
	suite_add_tcase(s, tc);

	tc = tcase_create("invoke");
	tcase_add_checked_fixture(tc, setup_list, teardown_list);
	tcase_add_test(tc, test_invoke_function);
	tcase_add_test(tc, test_invoke_offset);
	suite_add_tcase(s, tc);

	tc = tcase_create("clone");
	tcase_add_checked_fixture(tc, setup_list, teardown_list);
	tcase_add_test(tc, test_clone_offset);
	suite_add_tcase(s, tc);

	return s;
}
