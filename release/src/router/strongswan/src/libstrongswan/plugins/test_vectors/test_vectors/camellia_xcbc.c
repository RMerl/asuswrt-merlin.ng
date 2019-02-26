/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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
 * draft-kanno-ipsecme-camellia-xcbc Test Case #1
 */
signer_test_vector_t camellia_xcbc_s1 = {
	.alg = AUTH_CAMELLIA_XCBC_96, .len = 20,
	.key	= "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f",
	.data	= "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
			  "\x10\x11\x12\x13",
	.mac	= "\x3d\x04\x2d\xd4\xe7\xbc\x79\x1c\xee\x32\x04\x15",
};

prf_test_vector_t camellia_xcbc_p1 = {
	.alg = PRF_CAMELLIA128_XCBC, .key_size = 16, .len = 20,
	.key	= "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f",
	.seed	= "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
			  "\x10\x11\x12\x13",
	.out	= "\x3d\x04\x2d\xd4\xe7\xbc\x79\x1c\xee\x32\x04\x15\xc5\xe3\x26\xd6",
};

/**
 * draft-kanno-ipsecme-camellia-xcbc Test Case #2
 */
prf_test_vector_t camellia_xcbc_p2 = {
	.alg = PRF_CAMELLIA128_XCBC, .key_size = 10, .len = 20,
	.key	= "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09",
	.seed	= "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
			  "\x10\x11\x12\x13",
	.out	= "\xb9\x16\xb4\x23\x42\x0a\x90\x6c\xd7\xd7\xb6\x72\xa2\x4e\x97\x6f",
};

/**
 * draft-kanno-ipsecme-camellia-xcbc Test #3
 */
prf_test_vector_t camellia_xcbc_p3 = {
	.alg = PRF_CAMELLIA128_XCBC, .key_size = 18, .len = 20,
	.key	= "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
			  "\xed\xcb",
	.seed	= "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
			  "\x10\x11\x12\x13",
	.out	= "\xb9\x71\x46\x36\x9d\x31\x94\x0f\xf5\x7a\x0d\xdf\x22\x33\xc1\xd2",
};
