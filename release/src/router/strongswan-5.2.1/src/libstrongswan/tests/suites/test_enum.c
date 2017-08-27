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

#include <utils/utils.h>

/*******************************************************************************
 * continuous enum
 */
enum {
	CONT1,
	CONT2,
	CONT3,
	CONT4,
	CONT5,
} test_enum_cont;

/* can't be static */
enum_name_t *test_enum_cont_names;

ENUM_BEGIN(test_enum_cont_names, CONT1, CONT5,
	"CONT1", "CONT2", "CONT3", "CONT4", "CONT5");
ENUM_END(test_enum_cont_names, CONT5);

/*******************************************************************************
 * split enum
 */
enum {
	SPLIT1 = 1,
	SPLIT2,
	SPLIT3 = 5,
	SPLIT4,
	SPLIT5 = 255,
} test_enum_split;

/* can't be static */
enum_name_t *test_enum_split_names;

ENUM_BEGIN(test_enum_split_names, SPLIT1, SPLIT2,
	"SPLIT1", "SPLIT2");
ENUM_NEXT(test_enum_split_names, SPLIT3, SPLIT4, SPLIT2,
	"SPLIT3", "SPLIT4");
ENUM_NEXT(test_enum_split_names, SPLIT5, SPLIT5, SPLIT4,
	"SPLIT5");
ENUM_END(test_enum_split_names, SPLIT5);

/*******************************************************************************
 * enum_to_name
 */

static struct {
	int val;
	char *str;
} name_tests_cont[] = {
	{-1, NULL},
	{CONT1, "CONT1"},
	{CONT2, "CONT2"},
	{CONT3, "CONT3"},
	{CONT4, "CONT4"},
	{CONT5, "CONT5"},
	{5, NULL},
}, name_tests_split[] = {
	{-1, NULL},
	{0, NULL},
	{SPLIT1, "SPLIT1"},
	{SPLIT2, "SPLIT2"},
	{3, NULL},
	{4, NULL},
	{SPLIT3, "SPLIT3"},
	{SPLIT4, "SPLIT4"},
	{7, NULL},
	{254, NULL},
	{SPLIT5, "SPLIT5"},
	{256, NULL},
};

START_TEST(test_enum_to_name_cont)
{
	char *str = enum_to_name(test_enum_cont_names, name_tests_cont[_i].val);
	if (str)
	{
		ck_assert_str_eq(str, name_tests_cont[_i].str);
	}
	else
	{
		ck_assert(str == name_tests_cont[_i].str);
	}
}
END_TEST

START_TEST(test_enum_to_name_split)
{
	char *str = enum_to_name(test_enum_split_names, name_tests_split[_i].val);
	if (str)
	{
		ck_assert_str_eq(str, name_tests_split[_i].str);
	}
	else
	{
		ck_assert(str == name_tests_split[_i].str);
	}
}
END_TEST

/*******************************************************************************
 * enum_from_name
 */

static struct {
	bool found;
	int val;
	char *str;
} enum_tests_cont[] = {
	{TRUE, CONT1, "CONT1"},
	{TRUE, CONT2, "CONT2"},
	{TRUE, CONT2, "CoNt2"},
	{TRUE, CONT3, "CONT3"},
	{TRUE, CONT4, "CONT4"},
	{TRUE, CONT5, "CONT5"},
	{FALSE, 0, "asdf"},
	{FALSE, 0, ""},
	{FALSE, 0, NULL},
}, enum_tests_split[] = {
	{TRUE, SPLIT1, "SPLIT1"},
	{TRUE, SPLIT1, "split1"},
	{TRUE, SPLIT2, "SPLIT2"},
	{TRUE, SPLIT2, "SpLiT2"},
	{TRUE, SPLIT3, "SPLIT3"},
	{TRUE, SPLIT4, "SPLIT4"},
	{TRUE, SPLIT5, "SPLIT5"},
	{FALSE, 0, "asdf"},
	{FALSE, 0, ""},
	{FALSE, 0, NULL},
};

START_TEST(test_enum_from_name_cont)
{
	int val = 0;
	bool found;

	found = enum_from_name(test_enum_cont_names, enum_tests_cont[_i].str, &val);
	ck_assert(enum_tests_cont[_i].found == found);
	ck_assert_int_eq(val, enum_tests_cont[_i].val);
}
END_TEST

START_TEST(test_enum_from_name_split)
{
	int val = 0;
	bool found;

	found = enum_from_name(test_enum_split_names, enum_tests_split[_i].str, &val);
	ck_assert(enum_tests_split[_i].found == found);
	ck_assert_int_eq(val, enum_tests_split[_i].val);
}
END_TEST

/*******************************************************************************
 * enum_printf_hook
 */

static struct {
	int val;
	char *str;
} printf_tests_cont[] = {
	{-1, "(-1)"},
	{CONT1, "CONT1"},
	{CONT2, "CONT2"},
	{CONT3, "CONT3"},
	{CONT4, "CONT4"},
	{CONT5, "CONT5"},
	{5, "(5)"},
}, printf_tests_split[] = {
	{-1, "(-1)"},
	{0, "(0)"},
	{SPLIT1, "SPLIT1"},
	{SPLIT2, "SPLIT2"},
	{3, "(3)"},
	{4, "(4)"},
	{SPLIT3, "SPLIT3"},
	{SPLIT4, "SPLIT4"},
	{7, "(7)"},
	{254, "(254)"},
	{SPLIT5, "SPLIT5"},
	{256, "(256)"},
};

START_TEST(test_enum_printf_hook_cont)
{
	char buf[128];

	snprintf(buf, sizeof(buf), "%N", test_enum_cont_names, printf_tests_cont[_i].val);
	ck_assert_str_eq(printf_tests_cont[_i].str, buf);
}
END_TEST

START_TEST(test_enum_printf_hook_split)
{
	char buf[128];

	snprintf(buf, sizeof(buf), "%N", test_enum_split_names, printf_tests_split[_i].val);
	ck_assert_str_eq(printf_tests_split[_i].str, buf);
}
END_TEST

START_TEST(test_enum_printf_hook_width)
{
	char buf[128];

	snprintf(buf, sizeof(buf), "%10N", test_enum_cont_names, CONT1);
	ck_assert_str_eq("     CONT1", buf);
	snprintf(buf, sizeof(buf), "%-*N", 10, test_enum_cont_names, CONT2);
	ck_assert_str_eq("CONT2     ", buf);
	snprintf(buf, sizeof(buf), "%3N", test_enum_cont_names, CONT3);
	ck_assert_str_eq("CONT3", buf);
}
END_TEST

Suite *enum_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("enum");

	tc = tcase_create("enum_to_name");
	tcase_add_loop_test(tc, test_enum_to_name_cont, 0, countof(name_tests_cont));
	tcase_add_loop_test(tc, test_enum_to_name_split, 0, countof(name_tests_split));
	suite_add_tcase(s, tc);

	tc = tcase_create("enum_from_name");
	tcase_add_loop_test(tc, test_enum_from_name_cont, 0, countof(enum_tests_cont));
	tcase_add_loop_test(tc, test_enum_from_name_split, 0, countof(enum_tests_split));
	suite_add_tcase(s, tc);

	tc = tcase_create("enum_printf_hook");
	tcase_add_loop_test(tc, test_enum_printf_hook_cont, 0, countof(printf_tests_cont));
	tcase_add_loop_test(tc, test_enum_printf_hook_split, 0, countof(printf_tests_split));
	tcase_add_test(tc, test_enum_printf_hook_width);
	suite_add_tcase(s, tc);

	return s;
}
