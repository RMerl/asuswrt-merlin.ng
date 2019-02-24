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
 * FIPS PRF known value test
 */
prf_test_vector_t fips_prf_1 = {
	.alg = PRF_FIPS_SHA1_160, .stateful = TRUE, .key_size = 20, .len = 1,
	.key	= "\xbd\x02\x9b\xbe\x7f\x51\x96\x0b\xcf\x9e\xdb\x2b\x61\xf0\x6f\x0f"
			  "\xeb\x5a\x38\xb6",
	.seed	= "\x00",
	.out	= "\x20\x70\xb3\x22\x3d\xba\x37\x2f\xde\x1c\x0f\xfc\x7b\x2e\x3b\x49"
			  "\x8b\x26\x06\x14\x3c\x6c\x18\xba\xcb\x0f\x6c\x55\xba\xbb\x13\x78"
			  "\x8e\x20\xd7\x37\xa3\x27\x51\x16"
};

