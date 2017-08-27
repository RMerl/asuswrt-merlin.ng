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
	list = linked_list_create_with_items((void*)1, (void*)2, (void*)3, (void*)4,
									 (void*)5, NULL);
	ck_assert_int_eq(list->get_count(list), 5);
}
END_SETUP

START_TEARDOWN(teardown_list)
{
	list->destroy(list);
}
END_TEARDOWN

/*******************************************************************************
 * enumeration
 */

START_TEST(test_enumerate)
{
	enumerator_t *enumerator;
	intptr_t x;
	int round;

	round = 1;
	enumerator = list->create_enumerator(list);
	while (enumerator->enumerate(enumerator, &x))
	{
		ck_assert_int_eq(round, x);
		round++;
	}
	ck_assert_int_eq(round, 6);
	enumerator->destroy(enumerator);
}
END_TEST

START_TEST(test_enumerate_null)
{
	enumerator_t *enumerator;
	int round;

	round = 1;
	enumerator = list->create_enumerator(list);
	while (enumerator->enumerate(enumerator, NULL))
	{
		round++;
	}
	ck_assert_int_eq(round, 6);
	enumerator->destroy(enumerator);
}
END_TEST

START_TEST(test_reset_enumerator)
{
	enumerator_t *enumerator;
	intptr_t x;
	int round;

	enumerator = list->create_enumerator(list);
	while (enumerator->enumerate(enumerator, &x))
	{
	}
	list->reset_enumerator(list, enumerator);
	round = 1;
	while (enumerator->enumerate(enumerator, &x))
	{
		ck_assert_int_eq(round, x);
		round++;
	}
	ck_assert_int_eq(round, 6);
	enumerator->destroy(enumerator);
}
END_TEST

/*******************************************************************************
 * insert before
 */

START_TEST(test_insert_before)
{
	enumerator_t *enumerator;
	intptr_t x;
	int round;

	round = 1;
	enumerator = list->create_enumerator(list);
	while (enumerator->enumerate(enumerator, &x))
	{
		ck_assert_int_eq(round, x);
		round++;
		if (x == _i)
		{
			list->insert_before(list, enumerator, (void*)6);
		}
	}
	ck_assert_int_eq(list->get_count(list), 6);
	list->reset_enumerator(list, enumerator);
	round = 1;
	while (enumerator->enumerate(enumerator, &x))
	{
		if (round == _i && x != _i)
		{
			ck_assert_int_eq(6, x);
		}
		else
		{
			ck_assert_int_eq(round, x);
			round++;
		}
	}
	enumerator->destroy(enumerator);
}
END_TEST

START_TEST(test_insert_before_ends)
{
	enumerator_t *enumerator;
	intptr_t x;
	int round;

	enumerator = list->create_enumerator(list);
	list->insert_before(list, enumerator, (void*)0);
	ck_assert_int_eq(list->get_count(list), 6);
	ck_assert(list->get_first(list, (void*)&x) == SUCCESS);
	ck_assert_int_eq(x, 0);
	round = 0;
	while (enumerator->enumerate(enumerator, &x))
	{
		ck_assert_int_eq(round, x);
		round++;
	}
	list->insert_before(list, enumerator, (void*)6);
	ck_assert_int_eq(list->get_count(list), 7);
	ck_assert(list->get_last(list, (void*)&x) == SUCCESS);
	ck_assert_int_eq(x, 6);
	ck_assert(!enumerator->enumerate(enumerator, &x));
	enumerator->destroy(enumerator);
}
END_TEST

START_TEST(test_insert_before_empty)
{
	enumerator_t *enumerator;
	intptr_t x;

	list->destroy(list);
	list = linked_list_create();
	enumerator = list->create_enumerator(list);
	list->insert_before(list, enumerator, (void*)1);
	ck_assert_int_eq(list->get_count(list), 1);
	ck_assert(list->get_first(list, (void*)&x) == SUCCESS);
	ck_assert_int_eq(x, 1);
	ck_assert(list->get_last(list, (void*)&x) == SUCCESS);
	ck_assert_int_eq(x, 1);
	ck_assert(enumerator->enumerate(enumerator, &x));
	ck_assert_int_eq(x, 1);
	ck_assert(!enumerator->enumerate(enumerator, NULL));
	enumerator->destroy(enumerator);
}
END_TEST

/*******************************************************************************
 * remove_at
 */

START_TEST(test_remove_at)
{
	enumerator_t *enumerator;
	intptr_t x;
	int round;

	round = 1;
	enumerator = list->create_enumerator(list);
	while (enumerator->enumerate(enumerator, &x))
	{
		ck_assert_int_eq(round, x);
		if (round == 2)
		{
			list->remove_at(list, enumerator);
		}
		round++;
	}
	ck_assert_int_eq(list->get_count(list), 4);
	list->reset_enumerator(list, enumerator);
	round = 1;
	while (enumerator->enumerate(enumerator, &x))
	{
		if (round == 2)
		{	/* skip removed item */
			round++;
		}
		ck_assert_int_eq(round, x);
		round++;
	}
	enumerator->destroy(enumerator);
}
END_TEST

START_TEST(test_remove_at_ends)
{
	enumerator_t *enumerator;
	intptr_t x;

	enumerator = list->create_enumerator(list);
	list->remove_at(list, enumerator);
	ck_assert_int_eq(list->get_count(list), 5);
	ck_assert(list->get_first(list, (void*)&x) == SUCCESS);
	ck_assert_int_eq(x, 1);
	while (enumerator->enumerate(enumerator, &x))
	{
	}
	list->remove_at(list, enumerator);
	ck_assert_int_eq(list->get_count(list), 5);
	ck_assert(list->get_last(list, (void*)&x) == SUCCESS);
	ck_assert_int_eq(x, 5);
	enumerator->destroy(enumerator);
}
END_TEST

START_TEST(test_insert_before_remove_at)
{
	enumerator_t *enumerator;
	intptr_t x;
	int round;

	round = 1;
	enumerator = list->create_enumerator(list);
	while (enumerator->enumerate(enumerator, &x))
	{
		ck_assert_int_eq(round, x);
		if (round == 2)
		{	/* this replaces the current item, as insert_before does not change
			 * the enumerator position */
			list->insert_before(list, enumerator, (void*)42);
			list->remove_at(list, enumerator);
		}
		else if (round == 4)
		{	/* this does not replace the item, as remove_at moves the enumerator
			 * position to the previous item */
			list->remove_at(list, enumerator);
			list->insert_before(list, enumerator, (void*)21);
		}
		round++;
	}
	ck_assert_int_eq(list->get_count(list), 5);
	list->reset_enumerator(list, enumerator);
	round = 1;
	while (enumerator->enumerate(enumerator, &x))
	{
		if (round == 2)
		{	/* check replaced item */
			ck_assert_int_eq(42, x);
		}
		else if (round == 3)
		{	/* check misplaced item */
			ck_assert_int_eq(21, x);
		}
		else if (round == 4)
		{	/* check misplaced item */
			ck_assert_int_eq(3, x);
		}
		else
		{
			ck_assert_int_eq(round, x);
		}
		round++;
	}
	enumerator->destroy(enumerator);
}
END_TEST

/*******************************************************************************
 * create list from enumerator
 */

START_TEST(test_create_from_enumerator)
{
	enumerator_t *enumerator, *enumerator_other;
	linked_list_t *other;
	intptr_t x, y;
	int count = 0;

	enumerator = list->create_enumerator(list);
	other = linked_list_create_from_enumerator(enumerator);
	ck_assert_int_eq(other->get_count(list), 5);

	enumerator = list->create_enumerator(list);
	enumerator_other = other->create_enumerator(other);
	while (enumerator->enumerate(enumerator, &x) &&
		   enumerator_other->enumerate(enumerator_other, &y))
	{
		ck_assert_int_eq(x, y);
		count++;
	}
	ck_assert_int_eq(count, 5);
	enumerator_other->destroy(enumerator_other);
	enumerator->destroy(enumerator);
	other->destroy(other);
}
END_TEST

Suite *linked_list_enumerator_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("linked list and enumerators");

	tc = tcase_create("enumerate");
	tcase_add_checked_fixture(tc, setup_list, teardown_list);
	tcase_add_test(tc, test_enumerate);
	tcase_add_test(tc, test_enumerate_null);
	tcase_add_test(tc, test_reset_enumerator);
	suite_add_tcase(s, tc);

	tc = tcase_create("insert_before()");
	tcase_add_checked_fixture(tc, setup_list, teardown_list);
	tcase_add_loop_test(tc, test_insert_before, 1, 5);
	tcase_add_test(tc, test_insert_before_ends);
	tcase_add_test(tc, test_insert_before_empty);
	suite_add_tcase(s, tc);

	tc = tcase_create("modify");
	tcase_add_checked_fixture(tc, setup_list, teardown_list);
	tcase_add_test(tc, test_remove_at);
	tcase_add_test(tc, test_remove_at_ends);
	tcase_add_test(tc, test_insert_before_remove_at);
	suite_add_tcase(s, tc);

	tc = tcase_create("create_from_enumerator");
	tcase_add_checked_fixture(tc, setup_list, teardown_list);
	tcase_add_test(tc, test_create_from_enumerator);
	suite_add_tcase(s, tc);

	return s;
}
