/*
 * Copyright (C) 2014 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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

#include <collections/array.h>

START_TEST(test_append_ptr)
{
	array_t *array;
	uintptr_t x;
	int i;

	array = array_create(0, 0);

	for (i = 0; i < 4; i++)
	{
		ck_assert_int_eq(array_count(array), 0);

		array_insert(array, ARRAY_HEAD, (void*)(uintptr_t)3);
		array_insert(array, ARRAY_TAIL, (void*)(uintptr_t)4);
		ck_assert_int_eq(array_count(array), 2);

		/* 3, 4 */

		ck_assert(array_get(array, ARRAY_HEAD, &x));
		ck_assert_int_eq(x, 3);
		ck_assert(array_get(array, 1, &x));
		ck_assert_int_eq(x, 4);
		ck_assert(array_get(array, ARRAY_TAIL, &x));
		ck_assert_int_eq(x, 4);
		ck_assert(!array_get(array, 3, &x));

		array_insert(array, ARRAY_HEAD, (void*)(uintptr_t)1);
		array_insert(array, 1, (void*)(uintptr_t)2);
		ck_assert_int_eq(array_count(array), 4);

		/* 1, 2, 3, 4 */

		array_insert(array, ARRAY_TAIL, (void*)(uintptr_t)5);
		array_insert(array, ARRAY_HEAD, (void*)(uintptr_t)0);
		ck_assert_int_eq(array_count(array), 6);

		/* 0, 1, 2, 3, 4, 5 */

		ck_assert(array_remove(array, ARRAY_TAIL, &x));
		ck_assert_int_eq(x, 5);
		ck_assert(array_remove(array, 4, &x));
		ck_assert_int_eq(x, 4);

		if (i < 3)
		{
			array_compress(array);
		}

		/* 0, 1, 2, 3 */

		ck_assert(array_remove(array, 1, &x));
		ck_assert_int_eq(x, 1);
		ck_assert(array_remove(array, ARRAY_HEAD, &x));
		ck_assert_int_eq(x, 0);

		if (i < 2)
		{
			array_compress(array);
		}

		/* 2, 3 */

		ck_assert(array_remove(array, ARRAY_TAIL, &x));
		ck_assert_int_eq(x, 3);
		ck_assert(array_remove(array, ARRAY_TAIL, &x));
		ck_assert_int_eq(x, 2);

		if (i < 1)
		{
			array_compress(array);
		}

		ck_assert_int_eq(array_count(array), 0);

		ck_assert(array_remove(array, ARRAY_HEAD, NULL) == FALSE);
		ck_assert(array_remove(array, ARRAY_TAIL, NULL) == FALSE);
	}

	array_destroy(array);
}
END_TEST

START_TEST(test_append_obj)
{
	array_t *array;
	int i, x, y[6] = {0, 1, 2, 3, 4, 5};

	array = array_create(sizeof(y[0]), 0);

	for (i = 0; i < 4; i++)
	{
		ck_assert_int_eq(array_count(array), 0);

		array_insert(array, ARRAY_HEAD, &y[3]);
		array_insert(array, ARRAY_TAIL, &y[4]);
		ck_assert_int_eq(array_count(array), 2);;

		/* 3, 4 */

		ck_assert(array_get(array, ARRAY_HEAD, &x));
		ck_assert_int_eq(x, 3);
		ck_assert(array_get(array, 1, &x));
		ck_assert_int_eq(x, 4);
		ck_assert(array_get(array, ARRAY_TAIL, &x));
		ck_assert_int_eq(x, 4);
		ck_assert(!array_get(array, 3, &x));

		array_insert(array, ARRAY_HEAD, &y[1]);
		array_insert(array, 1, &y[2]);
		ck_assert_int_eq(array_count(array), 4);

		/* 1, 2, 3, 4 */

		array_insert(array, ARRAY_TAIL, &y[5]);
		array_insert(array, ARRAY_HEAD, &y[0]);
		ck_assert_int_eq(array_count(array), 6);

		/* 0, 1, 2, 3, 4, 5 */

		ck_assert(array_remove(array, ARRAY_TAIL, &x));
		ck_assert_int_eq(x, 5);
		ck_assert(array_remove(array, 4, &x));
		ck_assert_int_eq(x, 4);

		if (i < 3)
		{
			array_compress(array);
		}

		/* 0, 1, 2, 3 */

		ck_assert(array_remove(array, ARRAY_HEAD, &x));
		ck_assert_int_eq(x, 0);
		ck_assert(array_remove(array, ARRAY_HEAD, &x));
		ck_assert_int_eq(x, 1);

		if (i < 2)
		{
			array_compress(array);
		}

		/* 2, 3 */

		ck_assert(array_remove(array, ARRAY_TAIL, &x));
		ck_assert_int_eq(x, 3);
		ck_assert(array_remove(array, ARRAY_HEAD, &x));
		ck_assert_int_eq(x, 2);

		if (i < 1)
		{
			array_compress(array);
		}

		ck_assert_int_eq(array_count(array), 0);

		ck_assert(array_remove(array, ARRAY_HEAD, NULL) == FALSE);
		ck_assert(array_remove(array, ARRAY_TAIL, NULL) == FALSE);
	}

	array_destroy(array);
}
END_TEST

START_TEST(test_enumerate)
{
	array_t *array;
	int i, *x, y[6] = {0, 1, 2, 3, 4, 5};
	enumerator_t *enumerator;

	array = array_create(sizeof(y[0]), 0);

	array_insert(array, ARRAY_TAIL, &y[0]);
	array_insert(array, ARRAY_TAIL, &y[1]);
	array_insert(array, ARRAY_TAIL, &y[2]);
	array_insert(array, ARRAY_TAIL, &y[3]);
	array_insert(array, ARRAY_TAIL, &y[4]);
	array_insert(array, ARRAY_TAIL, &y[5]);

	ck_assert_int_eq(array_count(array), 6);

	/* 0, 1, 2, 3, 4, 5 */

	i = 0;
	enumerator = array_create_enumerator(array);
	while (enumerator->enumerate(enumerator, &x))
	{
		ck_assert_int_eq(*x, y[i]);
		i++;
	}
	enumerator->destroy(enumerator);
	ck_assert_int_eq(i, 6);

	i = 0;
	enumerator = array_create_enumerator(array);
	while (enumerator->enumerate(enumerator, &x))
	{
		ck_assert_int_eq(*x, y[i]);
		if (i == 0 || i == 3 || i == 5)
		{
			array_remove_at(array, enumerator);
		}
		i++;
	}
	enumerator->destroy(enumerator);
	ck_assert_int_eq(i, 6);
	ck_assert_int_eq(array_count(array), 3);

	/* 1, 2, 4 */

	i = 0;
	enumerator = array_create_enumerator(array);
	while (enumerator->enumerate(enumerator, &x))
	{
		switch (i++)
		{
			case 0:
				ck_assert_int_eq(*x, y[1]);
				break;
			case 1:
				ck_assert_int_eq(*x, y[2]);
				break;
			case 2:
				ck_assert_int_eq(*x, y[4]);
				break;
			default:
				ck_assert(0);
		}
	}
	enumerator->destroy(enumerator);

	array_compress(array);

	i = 0;
	enumerator = array_create_enumerator(array);
	while (enumerator->enumerate(enumerator, &x))
	{
		switch (i++)
		{
			case 0:
				ck_assert_int_eq(*x, y[1]);
				break;
			case 1:
				ck_assert_int_eq(*x, y[2]);
				break;
			case 2:
				ck_assert_int_eq(*x, y[4]);
				break;
			default:
				ck_assert(0);
		}
	}
	enumerator->destroy(enumerator);

	array_destroy(array);
}
END_TEST

static int comp_obj(const void *a, const void *b, void *arg)
{
	ck_assert_str_eq(arg, "arg");
	return *(int*)a - *(int*)b;
}

START_TEST(test_sort_obj)
{
	array_t *array;
	int x[][3] = {
		{1, 2, 3},
		{1, 3, 2},
		{2, 1, 3},
		{2, 3, 1},
		{3, 1, 2},
		{3, 2, 1},
	};
	char *arg = "arg";
	int i, v;

	for (i = 0; i < countof(x); i++)
	{
		array = array_create(sizeof(x[i][0]), 0);
		array_insert(array, ARRAY_TAIL, &x[i][0]);
		array_insert(array, ARRAY_TAIL, &x[i][1]);
		array_insert(array, ARRAY_TAIL, &x[i][2]);

		array_sort(array, comp_obj, arg);

		ck_assert(array_get(array, 0, &v));
		ck_assert_int_eq(v, 1);
		ck_assert(array_get(array, 1, &v));
		ck_assert_int_eq(v, 2);
		ck_assert(array_get(array, 2, &v));
		ck_assert_int_eq(v, 3);

		array_destroy(array);
	}
}
END_TEST

static int comp_ptr(const void *a, const void *b, void *arg)
{
	ck_assert_str_eq(arg, "arg");
	return strcmp(a, b);
}

START_TEST(test_sort_ptr)
{
	array_t *array;
	char *x[][3] = {
		{"a", "b", "c"},
		{"a", "c", "b"},
		{"b", "a", "c"},
		{"b", "c", "a"},
		{"c", "a", "b"},
		{"c", "b", "a"},
	};
	char *v, *arg = "arg";
	int i;

	for (i = 0; i < countof(x); i++)
	{
		array = array_create(0, 0);
		array_insert(array, ARRAY_TAIL, x[i][0]);
		array_insert(array, ARRAY_TAIL, x[i][1]);
		array_insert(array, ARRAY_TAIL, x[i][2]);

		array_sort(array, comp_ptr, arg);

		ck_assert(array_get(array, 0, &v));
		ck_assert_str_eq(v, "a");
		ck_assert(array_get(array, 1, &v));
		ck_assert_str_eq(v, "b");
		ck_assert(array_get(array, 2, &v));
		ck_assert_str_eq(v, "c");

		array_destroy(array);
	}
}
END_TEST

static int comp_search_obj(const void *a, const void *b)
{
	return *(int*)a - *(int*)b;
}

START_TEST(test_bsearch_obj)
{
	array_t *array;
	int x[] = { 3, 2, 1 };
	int k, v;

	array = array_create(sizeof(x[0]), 0);
	array_insert(array, ARRAY_TAIL, &x[0]);
	array_insert(array, ARRAY_TAIL, &x[1]);
	array_insert(array, ARRAY_TAIL, &x[2]);

	array_sort(array, (void*)comp_search_obj, NULL);

	k = 0;
	ck_assert_int_eq(array_bsearch(array, &k, comp_search_obj, &v), -1);
	for (k = 1; k < 4; k++)
	{
		ck_assert_int_eq(array_bsearch(array, &k, comp_search_obj, &v), k-1);
		ck_assert_int_eq(v, k);
	}
	k = 4;
	ck_assert_int_eq(array_bsearch(array, &k, comp_search_obj, &v), -1);
	array_destroy(array);
}
END_TEST

static int comp_search_ptr(const void *a, const void *b)
{
	return strcmp(a, b);
}

START_TEST(test_bsearch_ptr)
{
	array_t *array;
	char *x[] = {"c", "b", "a"};
	char *v;

	array = array_create(0, 0);
	array_insert(array, ARRAY_TAIL, x[0]);
	array_insert(array, ARRAY_TAIL, x[1]);
	array_insert(array, ARRAY_TAIL, x[2]);

	array_sort(array, (void*)comp_search_ptr, NULL);

	ck_assert_int_eq(array_bsearch(array, "abc", comp_search_ptr, &v), -1);
	ck_assert_int_eq(array_bsearch(array, "a", comp_search_ptr, &v), 0);
	ck_assert_str_eq(v, "a");
	ck_assert_int_eq(array_bsearch(array, "b", comp_search_ptr, &v), 1);
	ck_assert_str_eq(v, "b");
	ck_assert_int_eq(array_bsearch(array, "c", comp_search_ptr, &v), 2);
	ck_assert_str_eq(v, "c");

	array_destroy(array);
}
END_TEST

static void invoke(void *data, int idx, void *user)
{
	int *y = user, *x = data;

	ck_assert(idx < 3);

	ck_assert_int_eq(y[idx], *x);
	y[idx] = 0;
}

START_TEST(test_invoke)
{
	array_t *array;
	int y[] = {1, 2, 3};

	array = array_create(sizeof(y[0]), 0);

	array_insert(array, ARRAY_TAIL, &y[0]);
	array_insert(array, ARRAY_TAIL, &y[1]);
	array_insert(array, ARRAY_TAIL, &y[2]);

	array_invoke(array, invoke, y);

	ck_assert_int_eq(y[0], 0);
	ck_assert_int_eq(y[0], 0);
	ck_assert_int_eq(y[0], 0);

	array_destroy(array);
}
END_TEST

typedef struct obj_t obj_t;

struct obj_t {
	void (*fun)(obj_t *obj);
	int x;
	int *counter;
};

static void fun(obj_t *obj)
{
	ck_assert(obj->x == (*obj->counter)++);
}

START_TEST(test_invoke_offset)
{
	array_t *array;
	obj_t objs[5];
	int i, counter = 0;

	array = array_create(0, 0);

	for (i = 0; i < countof(objs); i++)
	{
		objs[i].x = i;
		objs[i].counter = &counter;
		objs[i].fun = fun;

		array_insert(array, ARRAY_TAIL, &objs[i]);
	}

	ck_assert_int_eq(countof(objs), array_count(array));

	array_invoke_offset(array, offsetof(obj_t, fun));

	ck_assert_int_eq(counter, countof(objs));

	array_destroy(array);
}
END_TEST

Suite *array_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("array");

	tc = tcase_create("add/get/remove ptr");
	tcase_add_test(tc, test_append_ptr);
	suite_add_tcase(s, tc);

	tc = tcase_create("add/get/remove obj");
	tcase_add_test(tc, test_append_obj);
	suite_add_tcase(s, tc);

	tc = tcase_create("enumerate");
	tcase_add_test(tc, test_enumerate);
	suite_add_tcase(s, tc);

	tc = tcase_create("sort");
	tcase_add_test(tc, test_sort_obj);
	tcase_add_test(tc, test_sort_ptr);
	suite_add_tcase(s, tc);

	tc = tcase_create("bsearch");
	tcase_add_test(tc, test_bsearch_obj);
	tcase_add_test(tc, test_bsearch_ptr);
	suite_add_tcase(s, tc);

	tc = tcase_create("invoke");
	tcase_add_test(tc, test_invoke);
	suite_add_tcase(s, tc);

	tc = tcase_create("invoke offset");
	tcase_add_test(tc, test_invoke_offset);
	suite_add_tcase(s, tc);

	return s;
}
