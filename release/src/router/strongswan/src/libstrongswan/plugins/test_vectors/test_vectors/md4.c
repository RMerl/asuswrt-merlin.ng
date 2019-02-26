/*
 * Copyright (C) 2009 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
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
 * MD4 vectors from RFC 1320
 */
hasher_test_vector_t md4_1 = {
	.alg = HASH_MD4, .len = 0,
	.data	= "",
	.hash	= "\x31\xd6\xcf\xe0\xd1\x6a\xe9\x31\xb7\x3c\x59\xd7\xe0\xc0\x89\xc0"
};

hasher_test_vector_t md4_2 = {
	.alg = HASH_MD4, .len = 1,
	.data	= "a",
	.hash	= "\xbd\xe5\x2c\xb3\x1d\xe3\x3e\x46\x24\x5e\x05\xfb\xdb\xd6\xfb\x24"
};

hasher_test_vector_t md4_3 = {
	.alg = HASH_MD4, .len = 3,
	.data	= "abc",
	.hash	= "\xa4\x48\x01\x7a\xaf\x21\xd8\x52\x5f\xc1\x0a\xe8\x7a\xa6\x72\x9d"
};

hasher_test_vector_t md4_4 = {
	.alg = HASH_MD4, .len = 14,
	.data	= "message digest",
	.hash	= "\xd9\x13\x0a\x81\x64\x54\x9f\xe8\x18\x87\x48\x06\xe1\xc7\x01\x4b"
};

hasher_test_vector_t md4_5 = {
	.alg = HASH_MD4, .len = 26,
	.data	= "abcdefghijklmnopqrstuvwxyz",
	.hash	= "\xd7\x9e\x1c\x30\x8a\xa5\xbb\xcd\xee\xa8\xed\x63\xdf\x41\x2d\xa9"
};

hasher_test_vector_t md4_6 = {
	.alg = HASH_MD4, .len = 62,
	.data	= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
	.hash	= "\x04\x3f\x85\x82\xf2\x41\xdb\x35\x1c\xe6\x27\xe1\x53\xe7\xf0\xe4"
};

hasher_test_vector_t md4_7 = {
	.alg = HASH_MD4, .len = 80,
	.data	= "1234567890123456789012345678901234567890"
			  "1234567890123456789012345678901234567890",
	.hash	= "\xe3\x3b\x4d\xdc\x9c\x38\xf2\x19\x9c\x3e\x7b\x16\x4f\xcc\x05\x36"
};

