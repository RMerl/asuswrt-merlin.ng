/*
 * Copyright (C) 2014 Andreas Steffen
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

#include <bliss_bitpacker.h>

static uint32_t bits[] = { 0, 1, 2, 3, 4, 7, 1, 14, 2, 29, 3, 28, 67, 0x2fe3a9c1};

static chunk_t packed_bits = chunk_from_chars(0x6e, 0x71, 0xe1, 0x74,
											  0x37, 0x21, 0x97, 0xf1,
											  0xd4, 0xe0, 0x80);

START_TEST(test_bliss_sign_bitpacker_write)
{
	chunk_t buf;
	bliss_bitpacker_t *packer;
	int i;

	packer = bliss_bitpacker_create(81);

	for (i = 0; i < 13; i++)
	{
		ck_assert(packer->write_bits(packer, bits[i], 1 + i/2));
	}
	ck_assert(packer->write_bits(packer, bits[13], 32));

	buf = packer->extract_buf(packer);
	ck_assert_int_eq(packer->get_bits(packer), 81);
	ck_assert_chunk_eq(buf, packed_bits);

	packer->destroy(packer);
	free(buf.ptr);
}
END_TEST

START_TEST(test_bliss_sign_bitpacker_read)
{
	uint32_t value;
	bliss_bitpacker_t *packer;
	int i;

	packer = bliss_bitpacker_create_from_data(packed_bits);

	ck_assert(!packer->read_bits(packer, &value, 33));

	for (i = 0; i < 13; i++)
	{
		ck_assert(packer->read_bits(packer, &value, 1 + i/2));
		ck_assert_int_eq(value, bits[i]);
	}
	ck_assert(packer->read_bits(packer, &value, 32));
	ck_assert_int_eq(value, bits[13]);

	packer->destroy(packer);
}
END_TEST

START_TEST(test_bliss_sign_bitpacker_fail)
{
	bliss_bitpacker_t *packer;
	uint32_t value;

	packer = bliss_bitpacker_create(32);
	ck_assert( packer->write_bits(packer, 0xff, 0));
	ck_assert(!packer->write_bits(packer, 0, 33));
	ck_assert( packer->write_bits(packer, 0x7f2a3b01, 31));
	ck_assert(!packer->write_bits(packer, 3,  2));
	packer->destroy(packer);

	packer = bliss_bitpacker_create_from_data(
							chunk_from_chars(0x7f, 0x2a, 0x3b, 0x01));
	ck_assert(!packer->read_bits(packer, &value, 33));
	ck_assert( packer->read_bits(packer, &value, 31));
	ck_assert(!packer->read_bits(packer, &value,  2));
	packer->destroy(packer);
}
END_TEST

Suite *bliss_bitpacker_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("bliss_bitpacker");

	tc = tcase_create("bitpacker_write");
	tcase_add_test(tc, test_bliss_sign_bitpacker_write);
	suite_add_tcase(s, tc);

	tc = tcase_create("bitpacker_read");
	tcase_add_test(tc, test_bliss_sign_bitpacker_read);
	suite_add_tcase(s, tc);

	tc = tcase_create("bitpacker_fail");
	tcase_add_test(tc, test_bliss_sign_bitpacker_fail);
	suite_add_tcase(s, tc);

	return s;
}
