/*
 * Copyright (C) 2009 Martin Willi
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
 * MD5 vectors from RFC1321
 */
hasher_test_vector_t md5_1 = {
	.alg = HASH_MD5, .len = 0,
	.data	= "",
	.hash	= "\xd4\x1d\x8c\xd9\x8f\x00\xb2\x04\xe9\x80\x09\x98\xec\xf8\x42\x7e"
};

hasher_test_vector_t md5_2 = {
	.alg = HASH_MD5, .len = 1,
	.data	= "a",
	.hash	= "\x0c\xc1\x75\xb9\xc0\xf1\xb6\xa8\x31\xc3\x99\xe2\x69\x77\x26\x61"
};

hasher_test_vector_t md5_3 = {
	.alg = HASH_MD5, .len = 3,
	.data	= "abc",
	.hash	= "\x90\x01\x50\x98\x3c\xd2\x4f\xb0\xd6\x96\x3f\x7d\x28\xe1\x7f\x72"
};

hasher_test_vector_t md5_4 = {
	.alg = HASH_MD5, .len = 14,
	.data	= "message digest",
	.hash	= "\xf9\x6b\x69\x7d\x7c\xb7\x93\x8d\x52\x5a\x2f\x31\xaa\xf1\x61\xd0"
};

hasher_test_vector_t md5_5 = {
	.alg = HASH_MD5, .len = 26,
	.data	= "abcdefghijklmnopqrstuvwxyz",
	.hash	= "\xc3\xfc\xd3\xd7\x61\x92\xe4\x00\x7d\xfb\x49\x6c\xca\x67\xe1\x3b"
};

hasher_test_vector_t md5_6 = {
	.alg = HASH_MD5, .len = 62,
	.data	= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
	.hash	= "\xd1\x74\xab\x98\xd2\x77\xd9\xf5\xa5\x61\x1c\x2c\x9f\x41\x9d\x9f"
};

hasher_test_vector_t md5_7 = {
	.alg = HASH_MD5, .len = 80,
	.data	= "1234567890123456789012345678901234567890"
			  "1234567890123456789012345678901234567890",
	.hash	= "\x57\xed\xf4\xa2\x2b\xe3\xc9\x55\xac\x49\xda\x2e\x21\x07\xb6\x7a"
};

