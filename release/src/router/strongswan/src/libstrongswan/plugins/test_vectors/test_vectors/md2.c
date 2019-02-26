/*
 * Copyright (C) 2009 Martin Willi
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
 * MD2 vectors from RFC 1319
 */
hasher_test_vector_t md2_1 = {
	.alg = HASH_MD2, .len = 0,
	.data	= "",
	.hash	= "\x83\x50\xe5\xa3\xe2\x4c\x15\x3d\xf2\x27\x5c\x9f\x80\x69\x27\x73"
};

hasher_test_vector_t md2_2 = {
	.alg = HASH_MD2, .len = 1,
	.data	= "a",
	.hash	= "\x32\xec\x01\xec\x4a\x6d\xac\x72\xc0\xab\x96\xfb\x34\xc0\xb5\xd1"
};

hasher_test_vector_t md2_3 = {
	.alg = HASH_MD2, .len = 3,
	.data	= "abc",
	.hash	= "\xda\x85\x3b\x0d\x3f\x88\xd9\x9b\x30\x28\x3a\x69\xe6\xde\xd6\xbb"
};

hasher_test_vector_t md2_4 = {
	.alg = HASH_MD2, .len = 14,
	.data	= "message digest",
	.hash	= "\xab\x4f\x49\x6b\xfb\x2a\x53\x0b\x21\x9f\xf3\x30\x31\xfe\x06\xb0"
};

hasher_test_vector_t md2_5 = {
	.alg = HASH_MD2, .len = 26,
	.data	= "abcdefghijklmnopqrstuvwxyz",
	.hash	= "\x4e\x8d\xdf\xf3\x65\x02\x92\xab\x5a\x41\x08\xc3\xaa\x47\x94\x0b"
};

hasher_test_vector_t md2_6 = {
	.alg = HASH_MD2, .len = 62,
	.data	= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
	.hash	= "\xda\x33\xde\xf2\xa4\x2d\xf1\x39\x75\x35\x28\x46\xc3\x03\x38\xcd"
};

hasher_test_vector_t md2_7 = {
	.alg = HASH_MD2, .len = 80,
	.data	= "1234567890123456789012345678901234567890"
			  "1234567890123456789012345678901234567890",
	.hash	= "\xd5\x97\x6f\x79\xd8\x3d\x3a\x0d\xc9\x80\x6c\x3c\x66\xf3\xef\xd8"
};

