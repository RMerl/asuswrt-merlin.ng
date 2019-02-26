/*
 * Copyright (C) 2013 Andreas Steffen
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

#include <pen/pen.h>

/*******************************************************************************
 * create
 */

START_TEST(test_pen_type_create)
{
	pen_type_t ita_1 = pen_type_create(PEN_ITA, 100);

	ck_assert(ita_1.vendor_id == PEN_ITA);
	ck_assert(ita_1.type == 100);
}
END_TEST

/*******************************************************************************
 * equals
 */

START_TEST(test_pen_type_equals)
{
	pen_type_t ita_1 = pen_type_create(PEN_ITA, 100);
	pen_type_t ita_2 = pen_type_create(PEN_ITA, 200);
	pen_type_t fhh_1 = pen_type_create(PEN_FHH, 100);
	pen_type_t fhh_2 = pen_type_create(PEN_FHH, 200);

	ck_assert( pen_type_equals(ita_1, ita_1));
	ck_assert(!pen_type_equals(ita_1, ita_2));
	ck_assert(!pen_type_equals(ita_1, fhh_1));
	ck_assert(!pen_type_equals(ita_1, fhh_2));
}
END_TEST

/*******************************************************************************
 * is
 */

START_TEST(test_pen_type_is)
{
	pen_type_t ita_1 = pen_type_create(PEN_ITA, 100);

	ck_assert( pen_type_is(ita_1, PEN_ITA, 100));
	ck_assert(!pen_type_is(ita_1, PEN_ITA, 200));
	ck_assert(!pen_type_is(ita_1, PEN_FHH, 100));
	ck_assert(!pen_type_is(ita_1, PEN_FHH, 200));
}
END_TEST

Suite *pen_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("pen");

	tc = tcase_create("create");
	tcase_add_test(tc, test_pen_type_create);
	suite_add_tcase(s, tc);

	tc = tcase_create("equals");
	tcase_add_test(tc, test_pen_type_equals);
	suite_add_tcase(s, tc);

	tc = tcase_create("is");
	tcase_add_test(tc, test_pen_type_is);
	suite_add_tcase(s, tc);

	return s;
}
