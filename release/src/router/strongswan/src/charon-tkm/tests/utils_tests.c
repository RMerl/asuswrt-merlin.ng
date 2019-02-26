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

#include <tkm/types.h>

#include "tkm_utils.h"

START_TEST(test_sequence_to_chunk)
{
	key_type key = {5, {0, 1, 2, 3, 4}};
	chunk_t chunk = chunk_empty;

	sequence_to_chunk(key.data, key.size, &chunk);
	fail_if(chunk.len != key.size, "Chunk size mismatch");

	uint32_t i;
	for (i = 0; i < key.size; i++)
	{
		fail_if(chunk.ptr[i] != i, "Data mismatch");
	}
	chunk_free(&chunk);
}
END_TEST

START_TEST(test_chunk_to_sequence)
{
	chunk_t chunk = chunk_from_thing("ABCDEFGH");
	key_type key;

	chunk_to_sequence(&chunk, &key, sizeof(key_type));
	fail_if(key.size != chunk.len, "Seq size mismatch");

	uint32_t i;
	for (i = 0; i < key.size - 1; i++)
	{
		fail_if(key.data[i] != 65 + i, "Data mismatch (1)");
	}
	fail_if(key.data[key.size - 1] != 0, "Data mismatch (2)");
}
END_TEST

Suite *make_utility_tests()
{
	Suite *s;
	TCase *tc;

	s = suite_create("utility tests");

	tc = tcase_create("chunk<->sequence");
	tcase_add_test(tc, test_sequence_to_chunk);
	tcase_add_test(tc, test_chunk_to_sequence);
	suite_add_tcase(s, tc);

	return s;
}
