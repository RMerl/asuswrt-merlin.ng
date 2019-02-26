/*
 * Copyright (C) 2013 Tobias Brunner
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

CALLBACK(find_a_b, bool,
	void *item, va_list args)
{
	void *a, *b;

	VA_ARGS_VGET(args, a, b);
	ck_assert(a == (void*)1);
	ck_assert(b == (void*)2);
	return item == a || item == b;
}

CALLBACK(find_a, bool,
	void *item, va_list args)
{
	void *a;

	VA_ARGS_VGET(args, a);
	return match_a(item, a);
}

CALLBACK(find_b, bool,
	void *item, va_list args)
{
	void *b;

	VA_ARGS_VGET(args, b);
	return match_b(item, b);
}

START_TEST(test_find)
{
	void *a = (void*)1, *b = (void*)2;

	ck_assert(!list->find_first(list, NULL, &a));
	list->insert_last(list, a);
	ck_assert(list->find_first(list, NULL, &a));
	ck_assert(!list->find_first(list, NULL, &b));
	list->insert_last(list, b);
	ck_assert(list->find_first(list, NULL, &a));
	ck_assert(list->find_first(list, NULL, &b));

	ck_assert(!list->find_first(list, NULL, NULL));
}
END_TEST

START_TEST(test_find_callback)
{
	void *a = (void*)1, *b = (void*)2, *x = NULL;

	ck_assert(!list->find_first(list, find_a_b, &x, a, b));
	list->insert_last(list, a);
	ck_assert(list->find_first(list, find_a, NULL, a));
	x = NULL;
	ck_assert(list->find_first(list, find_a, &x, a));
	ck_assert(a == x);
	ck_assert(!list->find_first(list, find_b, &x, b));
	ck_assert(a == x);
	x = NULL;
	ck_assert(list->find_first(list, find_a_b, &x, a, b));
	ck_assert(a == x);

	list->insert_last(list, b);
	ck_assert(list->find_first(list, find_a, &x, a));
	ck_assert(a == x);
	ck_assert(list->find_first(list, find_b, &x, b));
	ck_assert(b == x);
	x = NULL;
	ck_assert(list->find_first(list, find_a_b, &x, a, b));
	ck_assert(a == x);
}
END_TEST

CALLBACK(find_args, bool,
	void *item, va_list args)
{
	uint64_t d, e;
	level_t c;
	int *a, b;

	VA_ARGS_VGET(args, a, b, c, d, e);
	ck_assert_int_eq(*a, 1);
	ck_assert_int_eq(b, 2);
	ck_assert_int_eq(c, LEVEL_PRIVATE);
	ck_assert_int_eq(d, UINT64_MAX);
	ck_assert_int_eq(e, UINT64_MAX-1);
	return item == a;
}

START_TEST(test_find_callback_args)
{
	int a = 1, b = 2, *x;
	uint64_t d = UINT64_MAX;

	list->insert_last(list, &a);
	ck_assert(list->find_first(list, find_args, (void**)&x, &a, b,
							   LEVEL_PRIVATE, d, UINT64_MAX-1));
	ck_assert_int_eq(a, *x);
}
END_TEST

/*******************************************************************************
 * invoke
 */

typedef struct invoke_t invoke_t;

struct invoke_t {
	int val;
	void (*invoke)(invoke_t *item);
};

CALLBACK(invoke, void,
	intptr_t item, va_list args)
{
	void *a, *b, *c, *d;
	int *sum;

	VA_ARGS_VGET(args, a, b, c, d, sum);
	ck_assert_int_eq((uintptr_t)a, 1);
	ck_assert_int_eq((uintptr_t)b, 2);
	ck_assert_int_eq((uintptr_t)c, 3);
	ck_assert_int_eq((uintptr_t)d, 4);
	*sum += item;
}

static void invoke_offset(invoke_t *item)
{
	item->val++;
}

START_TEST(test_invoke_function)
{
	int sum = 0;

	list->insert_last(list, (void*)1);
	list->insert_last(list, (void*)2);
	list->insert_last(list, (void*)3);
	list->insert_last(list, (void*)4);
	list->insert_last(list, (void*)5);
	list->invoke_function(list, invoke, (uintptr_t)1, (uintptr_t)2,
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
	}, *item;
	int i;

	for (i = 0; i < countof(items); i++)
	{
		list->insert_last(list, &items[i]);
	}
	list->invoke_offset(list, offsetof(invoke_t, invoke));
	i = 2;
	while (list->remove_first(list, (void**)&item) == SUCCESS)
	{
		ck_assert_int_eq(item->val, i++);
	}
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


/*******************************************************************************
 * equals
 */

typedef struct equals_t equals_t;

struct equals_t {
	int val;
	bool (*equals)(equals_t *a, equals_t *b);
};

static bool equalsfn(equals_t *a, equals_t *b)
{
	return a->val == b->val;
}

START_TEST(test_equals_offset)
{
	linked_list_t *other;
	equals_t *x, items[] = {
		{ .val = 1, .equals = equalsfn, },
		{ .val = 2, .equals = equalsfn, },
		{ .val = 3, .equals = equalsfn, },
		{ .val = 4, .equals = equalsfn, },
		{ .val = 5, .equals = equalsfn, },
	};
	int i;

	for (i = 0; i < countof(items); i++)
	{
		list->insert_last(list, &items[i]);
	}
	ck_assert(list->equals_offset(list, list, offsetof(equals_t, equals)));
	other = linked_list_create_from_enumerator(list->create_enumerator(list));
	ck_assert(list->equals_offset(list, other, offsetof(equals_t, equals)));
	other->remove_last(other, (void**)&x);
	ck_assert(!list->equals_offset(list, other, offsetof(equals_t, equals)));
	list->remove_last(list, (void**)&x);
	ck_assert(list->equals_offset(list, other, offsetof(equals_t, equals)));
	other->remove_first(other, (void**)&x);
	ck_assert(!list->equals_offset(list, other, offsetof(equals_t, equals)));
	list->remove_first(list, (void**)&x);
	ck_assert(list->equals_offset(list, other, offsetof(equals_t, equals)));
	while (list->remove_first(list, (void**)&x) == SUCCESS);
	while (other->remove_first(other, (void**)&x) == SUCCESS);
	ck_assert(list->equals_offset(list, other, offsetof(equals_t, equals)));
	other->destroy(other);
}
END_TEST

START_TEST(test_equals_function)
{
	linked_list_t *other;
	equals_t *x, items[] = {
		{ .val = 1, },
		{ .val = 2, },
		{ .val = 3, },
		{ .val = 4, },
		{ .val = 5, },
	};
	int i;

	for (i = 0; i < countof(items); i++)
	{
		list->insert_last(list, &items[i]);
	}
	ck_assert(list->equals_function(list, list, (void*)equalsfn));
	other = linked_list_create_from_enumerator(list->create_enumerator(list));
	ck_assert(list->equals_function(list, other, (void*)equalsfn));
	other->remove_last(other, (void**)&x);
	ck_assert(!list->equals_function(list, other, (void*)equalsfn));
	list->remove_last(list, (void**)&x);
	ck_assert(list->equals_function(list, other, (void*)equalsfn));
	other->remove_first(other, (void**)&x);
	ck_assert(!list->equals_function(list, other, (void*)equalsfn));
	list->remove_first(list, (void**)&x);
	ck_assert(list->equals_function(list, other, (void*)equalsfn));
	while (list->remove_first(list, (void**)&x) == SUCCESS);
	while (other->remove_first(other, (void**)&x) == SUCCESS);
	ck_assert(list->equals_function(list, other, (void*)equalsfn));
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
	tcase_add_test(tc, test_find_callback_args);
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

	tc = tcase_create("equals");
	tcase_add_checked_fixture(tc, setup_list, teardown_list);
	tcase_add_test(tc, test_equals_offset);
	tcase_add_test(tc, test_equals_function);
	suite_add_tcase(s, tc);

	return s;
}
