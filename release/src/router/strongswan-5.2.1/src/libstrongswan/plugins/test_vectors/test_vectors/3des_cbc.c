/*
 * Copyright (C) 2009 Andreas Steffen
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
 * Example 1 from NIST 3DES MMT
 */
crypter_test_vector_t des3_cbc1 = {
	.alg = ENCR_3DES, .key_size = 24, .len = 8,
	.key    = "\x62\x7f\x46\x0e\x08\x10\x4a\x10"
			  "\x43\xcd\x26\x5d\x58\x40\xea\xf1"
			  "\x31\x3e\xdf\x97\xdf\x2a\x8a\x8c",
	.iv		= "\x8e\x29\xf7\x5e\xa7\x7e\x54\x75",
	.plain	= "\x32\x6a\x49\x4c\xd3\x3f\xe7\x56",
	.cipher	= "\xb2\x2b\x8d\x66\xde\x97\x06\x92"
};

/**
 * Example 2 from NIST 3DES MMT
 */
crypter_test_vector_t des3_cbc2 = {
	.alg = ENCR_3DES, .key_size = 24, .len = 16,
	.key    = "\x37\xae\x5e\xbf\x46\xdf\xf2\xdc"
			  "\x07\x54\xb9\x4f\x31\xcb\xb3\x85"
			  "\x5e\x7f\xd3\x6d\xc8\x70\xbf\xae",
	.iv		= "\x3d\x1d\xe3\xcc\x13\x2e\x3b\x65",
	.plain	= "\x84\x40\x1f\x78\xfe\x6c\x10\x87\x6d\x8e\xa2\x30\x94\xea\x53\x09",
	.cipher	= "\x7b\x1f\x7c\x7e\x3b\x1c\x94\x8e\xbd\x04\xa7\x5f\xfb\xa7\xd2\xf5"
};

