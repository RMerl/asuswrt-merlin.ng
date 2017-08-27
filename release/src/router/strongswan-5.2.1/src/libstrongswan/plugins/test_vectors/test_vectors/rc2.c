/*
 * Copyright (C) 2013 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the Licenseor (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be usefulbut
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <crypto/crypto_tester.h>

/**
 * Test vectors from RFC 2268
 */

/**
 * RC2 key length 8 bytes, effective key length 63 bits
 */
crypter_test_vector_t rc2_1 = {
	.alg = ENCR_RC2_CBC, .key_size = RC2_KEY_SIZE(8, 63), .len = 8,
	.key	= "\x00\x00\x00\x00\x00\x00\x00\x00",
	.iv		= "\x00\x00\x00\x00\x00\x00\x00\x00",
	.plain	= "\x00\x00\x00\x00\x00\x00\x00\x00",
	.cipher	= "\xeb\xb7\x73\xf9\x93\x27\x8e\xff",
};

/**
 * RC2 key length 8 bytes, effective key length 64 bits
 */
crypter_test_vector_t rc2_2 = {
	.alg = ENCR_RC2_CBC, .key_size = RC2_KEY_SIZE(8, 64), .len = 8,
	.key	= "\xff\xff\xff\xff\xff\xff\xff\xff",
	.iv		= "\x00\x00\x00\x00\x00\x00\x00\x00",
	.plain	= "\xff\xff\xff\xff\xff\xff\xff\xff",
	.cipher	= "\x27\x8b\x27\xe4\x2e\x2f\x0d\x49",
};

/**
 * RC2 key length 8 bytes, effective key length 64 bits
 */
crypter_test_vector_t rc2_3 = {
	.alg = ENCR_RC2_CBC, .key_size = RC2_KEY_SIZE(8, 64), .len = 8,
	.key	= "\x30\x00\x00\x00\x00\x00\x00\x00",
	.iv		= "\x00\x00\x00\x00\x00\x00\x00\x00",
	.plain	= "\x10\x00\x00\x00\x00\x00\x00\x01",
	.cipher	= "\x30\x64\x9e\xdf\x9b\xe7\xd2\xc2",
};

/**
 * RC2 key length 1 byte, effective key length 64 bits
 */
crypter_test_vector_t rc2_4 = {
	.alg = ENCR_RC2_CBC, .key_size = RC2_KEY_SIZE(1, 64), .len = 8,
	.key	= "\x88",
	.iv		= "\x00\x00\x00\x00\x00\x00\x00\x00",
	.plain	= "\x00\x00\x00\x00\x00\x00\x00\x00",
	.cipher	= "\x61\xa8\xa2\x44\xad\xac\xcc\xf0",
};

/**
 * RC2 key length 7 bytes, effective key length 64 bits
 */
crypter_test_vector_t rc2_5 = {
	.alg = ENCR_RC2_CBC, .key_size = RC2_KEY_SIZE(7, 64), .len = 8,
	.key	= "\x88\xbc\xa9\x0e\x90\x87\x5a",
	.iv		= "\x00\x00\x00\x00\x00\x00\x00\x00",
	.plain	= "\x00\x00\x00\x00\x00\x00\x00\x00",
	.cipher	= "\x6c\xcf\x43\x08\x97\x4c\x26\x7f",
};

/**
 * RC2 key length 16 bytes, effective key length 64 bits
 */
crypter_test_vector_t rc2_6 = {
	.alg = ENCR_RC2_CBC, .key_size = RC2_KEY_SIZE(16, 64), .len = 8,
	.key	= "\x88\xbc\xa9\x0e\x90\x87\x5a\x7f\x0f\x79\xc3\x84\x62\x7b\xaf\xb2",
	.iv		= "\x00\x00\x00\x00\x00\x00\x00\x00",
	.plain	= "\x00\x00\x00\x00\x00\x00\x00\x00",
	.cipher	= "\x1a\x80\x7d\x27\x2b\xbe\x5d\xb1",
};

/**
 * RC2 key length 16 bytes, effective key length 128 bits
 */
crypter_test_vector_t rc2_7 = {
	.alg = ENCR_RC2_CBC, .key_size = RC2_KEY_SIZE(16, 128), .len = 8,
	.key	= "\x88\xbc\xa9\x0e\x90\x87\x5a\x7f\x0f\x79\xc3\x84\x62\x7b\xaf\xb2",
	.iv		= "\x00\x00\x00\x00\x00\x00\x00\x00",
	.plain	= "\x00\x00\x00\x00\x00\x00\x00\x00",
	.cipher	= "\x22\x69\x55\x2a\xb0\xf8\x5c\xa6",
};

/**
 * RC2 key length 33 bytes, effective key length 129 bits
 */
crypter_test_vector_t rc2_8 = {
	.alg = ENCR_RC2_CBC, .key_size = RC2_KEY_SIZE(33, 129), .len = 8,
	.key	= "\x88\xbc\xa9\x0e\x90\x87\x5a\x7f\x0f\x79\xc3\x84\x62\x7b\xaf\xb2"
			  "\x16\xf8\x0a\x6f\x85\x92\x05\x84\xc4\x2f\xce\xb0\xbe\x25\x5d\xaf\x1e",
	.iv		= "\x00\x00\x00\x00\x00\x00\x00\x00",
	.plain	= "\x00\x00\x00\x00\x00\x00\x00\x00",
	.cipher	= "\x5b\x78\xd3\xa4\x3d\xff\xf1\xf1",
};
