/*
 * Copyright (C) 2013 Tobias Brunner
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

#include <utils/utils.h>

/*******************************************************************************
 * continuous enum
 */
enum test_enum_cont {
	CONT1,
	CONT2,
	CONT3,
	CONT4,
	CONT5,
};

ENUM_BEGIN(test_enum_cont_names, CONT1, CONT5,
	"CONT1", "CONT2", "CONT3", "CONT4", "CONT5");
ENUM_END(test_enum_cont_names, CONT5);

/*******************************************************************************
 * split enum
 */
enum test_enum_split {
	SPLIT1 = 1,
	SPLIT2,
	SPLIT3 = 5,
	SPLIT4,
	SPLIT5 = 255,
};

ENUM_BEGIN(test_enum_split_names, SPLIT1, SPLIT2,
	"SPLIT1", "SPLIT2");
ENUM_NEXT(test_enum_split_names, SPLIT3, SPLIT4, SPLIT2,
	"SPLIT3", "SPLIT4");
ENUM_NEXT(test_enum_split_names, SPLIT5, SPLIT5, SPLIT4,
	"SPLIT5");
ENUM_END(test_enum_split_names, SPLIT5);

/*******************************************************************************
 * enum flags
 */
enum test_enum_flags {
	FLAG1 = (1 << 0),
	FLAG2 = (1 << 1),
	FLAG3 = (1 << 2),
	FLAG4 = (1 << 3),
	FLAG5 = (1 << 4),
	FLAG6 = (1 << 5),
	FLAG7 = (1 << 6),
	FLAG8 = (1 << 7),
	FLAG9 = (1 << 8),
	FLAG10 = (1 << 9),
	FLAG11 = (1 << 10),
	FLAG12 = (1 << 11),
};

ENUM_FLAGS(test_enum_flags_names, FLAG1, FLAG5,
	"(unset)", "FLAG1", "FLAG2", "FLAG3", "FLAG4", "FLAG5");

ENUM_FLAGS(test_enum_flags_incomplete_names, FLAG3, FLAG4,
	"(unset)", "FLAG3", "FLAG4");

ENUM_FLAGS(test_enum_flags_null_names, FLAG1, FLAG4,
	"(unset)", "FLAG1", NULL, "FLAG3", NULL);

ENUM_FLAGS(test_enum_flags_overflow_names, FLAG1, FLAG12, "(unset)",
	"OVERFLOWFLAGLONGNAME1",  "OVERFLOWFLAGLONGNAME2",  "OVERFLOWFLAGLONGNAME3",
	"OVERFLOWFLAGLONGNAME4",  "OVERFLOWFLAGLONGNAME5",  "OVERFLOWFLAGLONGNAME6",
	"OVERFLOWFLAGLONGNAME7",  "OVERFLOWFLAGLONGNAME8",  "OVERFLOWFLAGLONGNAME9",
	"OVERFLOWFLAGLONGNAME10", "OVERFLOWFLAGLONGNAME11", "OVERFLOWFLAGLONGNAME12");

/*******************************************************************************
 * add_enum_names
 */

ENUM_EXT(e1, 65000, 65001, "CONT65000", "CONT65001");
ENUM_EXT(e2, 62000, 62001, "CONT62000", "CONT62001");

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
}, enum_tests_ext[] = {
	{TRUE, CONT1, "CONT1"},
	{TRUE, 62000, "CONT62000"},
	{TRUE, 62001, "CONT62001"},
	{TRUE, 65000, "CONT65000"},
	{TRUE, 65001, "CONT65001"},
	{FALSE, 0, "CONT64000"},
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

START_TEST(test_enum_from_name_ext)
{
	int val = 0;
	bool found;

	enum_add_enum_names(test_enum_cont_names, e1);
	enum_add_enum_names(test_enum_cont_names, e2);

	found = enum_from_name(test_enum_cont_names, enum_tests_ext[_i].str, &val);
	ck_assert(enum_tests_ext[_i].found == found);
	ck_assert_int_eq(val, enum_tests_ext[_i].val);

	enum_remove_enum_names(test_enum_cont_names, e1);
	enum_remove_enum_names(test_enum_cont_names, e2);
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

/*******************************************************************************
 * flag_to_name
 */

static struct {
	int val;
	char *str;
} printf_tests_flags[] = {
	{0, "(unset)"},
	{FLAG1, "FLAG1"},
	{FLAG2, "FLAG2"},
	{FLAG3, "FLAG3"},
	{FLAG4, "FLAG4"},
	{FLAG5, "FLAG5"},
	{FLAG1 | FLAG3, "FLAG1 | FLAG3"},
	{FLAG1 | FLAG3 | 32, "FLAG1 | FLAG3 | (0x20)"},
	{FLAG1 | FLAG3 | 32 | 64, "FLAG1 | FLAG3 | (0x20) | (0x40)"},
	{0x20, "(0x20)"},
	{0x80000000, "(0x80000000)"},
	{0xFFFFF, "FLAG1 | FLAG2 | FLAG3 | FLAG4 | "
			  "FLAG5 | (0x20) | (0x40) | (0x80) | "
			 "(0x100) | (0x200) | (0x400) | (0x800) | "
			 "(0x1000) | (0x2000) | (0x4000) | (0x8000) | "
			 "(0x10000) | (0x20000) | (0x40000) | (0x80000)"},
}, printf_tests_flags_incomplete[] = {
	{FLAG1, "(0x1)"},
	{FLAG1 | FLAG2 | FLAG3, "(0x1) | (0x2) | FLAG3"},
	{FLAG3 | FLAG4 | FLAG5, "FLAG3 | FLAG4 | (0x10)"},
}, printf_tests_flags_null[] = {
	{FLAG1 | FLAG2 | FLAG3 | FLAG4, "FLAG1 | FLAG3"},
}, printf_tests_flags_overflow[] = {
	{0xFFFFFFFF, "(0xFFFFFFFF)"},
}, printf_tests_flags_noflagenum[] = {
	{-1, "(-1)"},
	{6435, "(6435)"},
}, enum_flags_to_string_tests[] = {
	{-1, NULL},
	{6435, NULL},
}, enum_flags_from_string_tests[] = {
	{0, NULL},
	{0, ""},
	{0, "(unset)"},
	{FLAG1, "FLAG1"},
	{FLAG2, "flag2"},
	{FLAG3, "fLaG3"},
	{FLAG4, "FLAG4"},
	{FLAG5, "FLAG5"},
	{FLAG1 | FLAG3, "FLAG1 | FLAG3"},
	{FLAG1 | FLAG3, "flag3|flag1"},
	{FLAG1 | FLAG3, "flag1|flag3 | (unset)"},
	{FLAG1 | FLAG2 | FLAG3 | FLAG4 | FLAG5, "flag1|flag2|flag3|flag4|flag5"},
	{FLAG1 | FLAG2 | FLAG3 | FLAG4 | FLAG5, "flag3|flag4|flag5|flag2|flag1"},
	{FLAG5, "(unset)|flag5"},
	{FLAG1, "FLAG1 | flag1 | flAg1"},
	{-1, "FLAG6"},
	{-1, "flag1 | asdf"},
}, enum_flags_from_string_noflagenum_tests[] = {
	{0, NULL},
	{0, ""},
	{CONT2, "CONT2"},
	{CONT5, "CONT5"},
	{-1, "asdf"},
};

START_TEST(test_enum_printf_hook_cont)
{
	char buf[128];

	snprintf(buf, sizeof(buf), "%N",
			 test_enum_cont_names, printf_tests_cont[_i].val);
	ck_assert_str_eq(printf_tests_cont[_i].str, buf);
}
END_TEST

START_TEST(test_enum_printf_hook_split)
{
	char buf[128];

	snprintf(buf, sizeof(buf), "%N",
			 test_enum_split_names, printf_tests_split[_i].val);
	ck_assert_str_eq(printf_tests_split[_i].str, buf);
}
END_TEST

START_TEST(test_enum_printf_hook_null)
{
	char buf[16];

	snprintf(buf, sizeof(buf), "%N", NULL, 7);
	ck_assert_str_eq("(7)", buf);
}
END_TEST

START_TEST(test_enum_printf_hook_flags)
{
	char buf[1024];

	snprintf(buf, sizeof(buf), "%N", test_enum_flags_names,
			 printf_tests_flags[_i].val);
	ck_assert_str_eq(printf_tests_flags[_i].str, buf);
}
END_TEST

START_TEST(test_enum_printf_hook_flags_incomplete)
{
	char buf[1024];

	snprintf(buf, sizeof(buf), "%N", test_enum_flags_incomplete_names,
			 printf_tests_flags_incomplete[_i].val);
	ck_assert_str_eq(printf_tests_flags_incomplete[_i].str, buf);
}
END_TEST

START_TEST(test_enum_printf_hook_flags_null)
{
	char buf[1024];

	snprintf(buf, sizeof(buf), "%N", test_enum_flags_null_names,
			 printf_tests_flags_null[_i].val);
	ck_assert_str_eq(printf_tests_flags_null[_i].str, buf);
}
END_TEST

START_TEST(test_enum_printf_hook_flags_overflow)
{
	char buf[1024];

	snprintf(buf, sizeof(buf), "%N", test_enum_flags_overflow_names,
			 printf_tests_flags_overflow[_i].val);
	ck_assert_str_eq(printf_tests_flags_overflow[_i].str, buf);
}
END_TEST

START_TEST(test_enum_printf_hook_flags_noflagenum)
{
	char buf[1024];

	snprintf(buf, sizeof(buf), "%N", test_enum_cont_names,
			 printf_tests_flags_noflagenum[_i].val);
	ck_assert_str_eq(printf_tests_flags_noflagenum[_i].str, buf);
}
END_TEST

START_TEST(test_enum_flags_to_string)
{
	char buf[1], *str;

	str = enum_flags_to_string(test_enum_flags_names,
			enum_flags_to_string_tests[_i].val, buf, sizeof(buf));
	if (str)
	{
		ck_assert_str_eq(enum_flags_to_string_tests[_i].str, str);
	}
	else
	{
		ck_assert(str == enum_flags_to_string_tests[_i].str);
	}
}
END_TEST

START_TEST(test_enum_flags_to_string_noflagenum)
{
	char buf[1024];

	enum_flags_to_string(test_enum_cont_names,
			printf_tests_flags_noflagenum[_i].val, buf, sizeof(buf));
	ck_assert_str_eq(printf_tests_flags_noflagenum[_i].str, buf);
}
END_TEST

START_TEST(test_enum_flags_from_string)
{
	enum test_enum_flags val;

	if (enum_flags_from_string(test_enum_flags_names,
							   enum_flags_from_string_tests[_i].str, &val))
	{
		ck_assert_int_eq(enum_flags_from_string_tests[_i].val, val);
	}
	else
	{
		ck_assert_int_eq(enum_flags_from_string_tests[_i].val, -1);
	}
}
END_TEST

START_TEST(test_enum_flags_from_string_noflagenum)
{
	enum test_enum_cont val;

	if (enum_flags_from_string(test_enum_cont_names,
						enum_flags_from_string_noflagenum_tests[_i].str, &val))
	{
		ck_assert_int_eq(enum_flags_from_string_noflagenum_tests[_i].val, val);
	}
	else
	{
		ck_assert_int_eq(enum_flags_from_string_noflagenum_tests[_i].val, -1);
	}
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

START_TEST(test_enum_printf_hook_add_enum_names)
{
	char buf[128];

	enum_add_enum_names(test_enum_cont_names, e1);
	snprintf(buf, sizeof(buf), "%N", test_enum_cont_names, 65001);
	ck_assert_str_eq("CONT65001", buf);

	enum_add_enum_names(test_enum_cont_names, e2);
	snprintf(buf, sizeof(buf), "%N", test_enum_cont_names, 62001);
	ck_assert_str_eq("CONT62001", buf);

	/* adding the same list repeatedly should not result in an infinite loop */
	enum_add_enum_names(test_enum_cont_names, e2);
	snprintf(buf, sizeof(buf), "%N", test_enum_cont_names, 62001);
	ck_assert_str_eq("CONT62001", buf);

	/* can also be defined inside a function as long as the same function is
	 * adding and removing it */
	ENUM_EXT(e3, 64000, 64001, "CONT64000", "CONT64001");
	enum_add_enum_names(test_enum_cont_names, e3);
	snprintf(buf, sizeof(buf), "%N", test_enum_cont_names, 64000);
	ck_assert_str_eq("CONT64000", buf);

	snprintf(buf, sizeof(buf), "%N, %N, %N", test_enum_cont_names, 62001,
			test_enum_cont_names, 65000, test_enum_cont_names, 64000);
	ck_assert_str_eq("CONT62001, CONT65000, CONT64000", buf);

	enum_remove_enum_names(test_enum_cont_names, e2);
	snprintf(buf, sizeof(buf), "%N, %N, %N", test_enum_cont_names, 62001,
			test_enum_cont_names, 65000, test_enum_cont_names, 64000);
	ck_assert_str_eq("(62001), CONT65000, CONT64000", buf);

	enum_remove_enum_names(test_enum_cont_names, e3);
	snprintf(buf, sizeof(buf), "%N, %N, %N", test_enum_cont_names, 62001,
			test_enum_cont_names, 65000, test_enum_cont_names, 64000);
	ck_assert_str_eq("(62001), CONT65000, (64000)", buf);

	enum_remove_enum_names(test_enum_cont_names, e1);
	snprintf(buf, sizeof(buf), "%N, %N, %N", test_enum_cont_names, 62001,
			test_enum_cont_names, 65000, test_enum_cont_names, 64000);
	ck_assert_str_eq("(62001), (65000), (64000)", buf);
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
	tcase_add_loop_test(tc, test_enum_from_name_ext, 0, countof(enum_tests_ext));
	suite_add_tcase(s, tc);

	tc = tcase_create("enum_flags_to_string");
	tcase_add_loop_test(tc, test_enum_flags_to_string, 0, countof(enum_flags_to_string_tests));
	tcase_add_loop_test(tc, test_enum_flags_to_string_noflagenum, 0, countof(printf_tests_flags_noflagenum));
	suite_add_tcase(s, tc);

	tc = tcase_create("enum_flags_from_string");
	tcase_add_loop_test(tc, test_enum_flags_from_string, 0, countof(enum_flags_from_string_tests));
	tcase_add_loop_test(tc, test_enum_flags_from_string_noflagenum, 0, countof(enum_flags_from_string_noflagenum_tests));
	suite_add_tcase(s, tc);

	tc = tcase_create("enum_printf_hook");
	tcase_add_loop_test(tc, test_enum_printf_hook_cont, 0, countof(printf_tests_cont));
	tcase_add_loop_test(tc, test_enum_printf_hook_split, 0, countof(printf_tests_split));
	tcase_add_test(tc, test_enum_printf_hook_null);
	tcase_add_loop_test(tc, test_enum_printf_hook_flags, 0, countof(printf_tests_flags));
	tcase_add_loop_test(tc, test_enum_printf_hook_flags_incomplete, 0, countof(printf_tests_flags_incomplete));
	tcase_add_loop_test(tc, test_enum_printf_hook_flags_null, 0, countof(printf_tests_flags_null));
	tcase_add_loop_test(tc, test_enum_printf_hook_flags_overflow, 0, countof(printf_tests_flags_overflow));
	tcase_add_loop_test(tc, test_enum_printf_hook_flags_noflagenum, 0, countof(printf_tests_flags_noflagenum));
	tcase_add_test(tc, test_enum_printf_hook_width);
	tcase_add_test(tc, test_enum_printf_hook_add_enum_names);
	suite_add_tcase(s, tc);

	return s;
}
