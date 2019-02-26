/*
 * Copyright (C) 2015 Andreas Steffen
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

#include <bliss_huffman_coder.h>

static chunk_t data = chunk_from_chars(0x5f, 0x71, 0x9e, 0x4c);

START_TEST(test_bliss_huffman_encode)
{
	bliss_bitpacker_t *packer;
	bliss_huffman_code_t *code;
	bliss_huffman_coder_t *coder;
	chunk_t encoding;

	packer = bliss_bitpacker_create(32);
	ck_assert(packer);

	code = bliss_huffman_code_get_by_id(BLISS_B_I);
	ck_assert(code);

	coder = bliss_huffman_coder_create(code, packer);
	ck_assert(coder);

	ck_assert( coder->encode(coder, 0,  0));	/* 0 */
	ck_assert( coder->encode(coder, 1,  0));	/* 10 */
	ck_assert( coder->encode(coder, 2,  0));	/* 111 */
	ck_assert( coder->encode(coder, 0,  1));	/* 1101 */
	ck_assert( coder->encode(coder, 0, -1));	/* 11000 */
	ck_assert( coder->encode(coder, 1,  1));	/* 110011 */
	ck_assert( coder->encode(coder, 1, -1));	/* 1100100 */
	ck_assert(!coder->encode(coder, 3,  0));	/* 11001010 */
	ck_assert(!coder->encode(coder, 8,  0));	/* - */

	encoding = packer->extract_buf(packer);
	ck_assert(chunk_equals(encoding, data));

	chunk_free(&encoding);
	coder->destroy(coder);
	packer->destroy(packer);
}
END_TEST

START_TEST(test_bliss_huffman_decode)
{
	bliss_bitpacker_t *packer;
	bliss_huffman_code_t *code;
	bliss_huffman_coder_t *coder;
	int32_t z1;
	int16_t z2;

	packer = bliss_bitpacker_create_from_data(data);
	ck_assert(packer);

	code = bliss_huffman_code_get_by_id(BLISS_II);
	ck_assert(!code);
	code = bliss_huffman_code_get_by_id(BLISS_B_II);
	ck_assert(!code);
	code = bliss_huffman_code_get_by_id(BLISS_B_I);
	ck_assert(code);

	coder = bliss_huffman_coder_create(code, packer);
	ck_assert(coder);

	ck_assert(coder->decode(coder, &z1, &z2));	/* 0 */
	ck_assert(z1 == 0 && z2 == 0);

	ck_assert(coder->decode(coder, &z1, &z2));	/* 10 */
	ck_assert(z1 == 1 && z2 == 0);

	ck_assert(coder->decode(coder, &z1, &z2));	/* 111 */
	ck_assert(z1 == 2 && z2 == 0);

	ck_assert(coder->decode(coder, &z1, &z2));	/* 1101 */
	ck_assert(z1 == 0 && z2 == 1);

	ck_assert(coder->decode(coder, &z1, &z2));	/* 11000 */
	ck_assert(z1 == 0 && z2 == -1);

	ck_assert(coder->decode(coder, &z1, &z2));	/* 110011 */
	ck_assert(z1 == 1 && z2 == 1);

	ck_assert(coder->decode(coder, &z1, &z2));	/* 1100100 */
	ck_assert(z1 == 1 && z2 == -1);

	ck_assert(!coder->decode(coder, &z1, &z2));	/* 11001010 */

	coder->destroy(coder);
	packer->destroy(packer);
}
END_TEST

Suite *bliss_huffman_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("bliss_huffman");

	tc = tcase_create("huffman_encode");
	tcase_add_test(tc, test_bliss_huffman_encode);
	suite_add_tcase(s, tc);

	tc = tcase_create("huffman_decode");
	tcase_add_test(tc, test_bliss_huffman_decode);
	suite_add_tcase(s, tc);

	return s;
}
