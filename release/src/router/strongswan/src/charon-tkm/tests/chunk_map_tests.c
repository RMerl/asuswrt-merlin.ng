/*
 * Copyright (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
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

#include <tests/test_suite.h>

#include "tkm_chunk_map.h"

START_TEST(test_chunk_map_creation)
{
	tkm_chunk_map_t *map = NULL;

	map = tkm_chunk_map_create();
	fail_if(map == NULL, "Error creating chunk map");

	map->destroy(map);
}
END_TEST

START_TEST(test_chunk_map_handling)
{
	tkm_chunk_map_t *map = NULL;
	const int ref = 35;
	chunk_t data = chunk_from_thing(ref);

	map = tkm_chunk_map_create();
	fail_if(map == NULL, "Error creating chunk map");

	map->insert(map, &data, 24);
	fail_if(map->get_id(map, &data) != 24, "Id mismatch");

	fail_unless(map->remove(map, &data), "Unable to remove mapping");
	fail_unless(!map->get_id(map, &data), "Error removing mapping");

	map->destroy(map);
}
END_TEST

Suite *make_chunk_map_tests()
{
	Suite *s;
	TCase *tc;

	s = suite_create("chunk map");

	tc = tcase_create("creating");
	tcase_add_test(tc, test_chunk_map_creation);
	suite_add_tcase(s, tc);

	tc = tcase_create("handling");
	tcase_add_test(tc, test_chunk_map_handling);
	suite_add_tcase(s, tc);

	return s;
}
